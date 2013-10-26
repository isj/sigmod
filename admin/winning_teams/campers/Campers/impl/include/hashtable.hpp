/*
Copyright 2013 Henrik Mühe and Florian Funke

This file is part of CampersCoreBurner.

CampersCoreBurner is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CampersCoreBurner is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with CampersCoreBurner.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
COMMENT SUMMARY FOR THIS FILE:

This file contains an implementation of an optimized chaining hashset and hashmap.
Depending on the use case, these hashes perform up to 4x better than std::unordered_map.
*/

#pragma once

#include "hash.hpp"
#include "pool.hpp"
#include <boost/utility/string_ref.hpp>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>


namespace campers {

/// A fast, chaining hash set
template<class T,class HashFunc=Crc32Hash>
class HashSet {
    /// An entry inside the chain
    struct Entry {
        /// The key
        T word;
        /// The hash value
        uint32_t hashValue;
        /// Chain to collisions
        Entry* next;
        /// Constructor
        Entry () : hashValue(0),next(nullptr) {}
        /// Constructor
        Entry (const T& word,uint32_t hashValue) : word(word),hashValue(hashValue),next(nullptr) {}
    };

    /// An instance of the hash funtion functor
    HashFunc hashFunc;
    /// Hashmap of pointers to actual entries
    std::vector<Entry*> entries;
    /// A custom pool allocator for fast allocation of hash entries
    Pool<Entry> entryAllocator;
    /// The exponent used in this hashmap, to avoid modulus calculation, the hash grows in powers of two, this is the exponent
    uint64_t exp;
    /// This is the mask such that if you compute number&mask you get the index inside the entries table
    uint64_t mask;
    /// Current size in elements of the hashtable
    uint64_t size_=0;

public:
    /// Insert an element if it doesn't exist. Return pointer to element if new, nullptr otherwise
    T* tryInsert(const T& word);
    /// Find an element inside the hash table, return true or false depending on whether it exists
    bool find(const T& word) const;
    /// Find an element inside the hash table, return true or false depending on whether it exists. Hash is known in advance.
    bool find(const T& word, uint32_t hashValue) const;
    /// Pre allocate for size many elements. Can not be called when the hash already contains elements, see grow.
    void hintSize(uint32_t size);
    /// Grow the hash table to twice its current size
    void grow() { grow(2*capacity()); };
    /// Grow the hash table to a specific size
    void grow(uint32_t newSize);
    /// Remove all elements from the hashtable
    void clear() { size_=0; entries.clear(); entries.resize(1<<exp); }
    /// Return the current size of the hashtable
    size_t size() const { return size_; };
    /// Return the capacity of the hash table
    size_t capacity() const { return entries.size(); };
};

/// Grow the hash table to a specific size
template<class T,class HashFunc>
void HashSet<T,HashFunc>::grow(uint32_t newSize) {
    // Grow
    for (exp=1;(uint32_t)(1<<exp)<newSize;++exp);
    std::vector<Entry*> newEntries(1<<exp);
    mask=(1ull<<exp)-1ull;

    // Copy entries over
    for (Entry* entryChain : entries) {
        for  (Entry *e=entryChain, *next; e!=nullptr; e=next) {
            Entry*& entry = newEntries[e->hashValue&mask];
            if (entry==nullptr) { // immediate hit: use slot for e
                entry=e;
                next=e->next;
                e->next=nullptr;
            } else { // slot already in use: prepend e to chain
                next=e->next;
                e->next=entry;
                entry=e;
            }
        }
    }

    // Switch to new entry vector
    entries=std::move(newEntries);
}

/// Insert an element if it doesn't exist. Return pointer to element if new, nullptr otherwise
template<class T,class HashFunc>
T* HashSet<T,HashFunc>::tryInsert(const T& word) {
    uint32_t hashValue=hashFunc(word);
    Entry*& entry=entries[hashValue&mask];
    // Unroll empty slot branch
    if (entry==nullptr) {
        ++size_;
        entry=entryAllocator.allocate();
        new (entry) Entry(word,hashValue);
        return &entry->word;
    } else {
        // Unroll immediate hit branch
        if (entry->hashValue==hashValue&&entry->word==word) return nullptr;

        // Iterate to the end of the chain
        Entry* entryIter;
        for (entryIter=entry;entryIter->next!=nullptr;entryIter=entryIter->next) if (entryIter->next->hashValue==hashValue&&entryIter->next->word==word) return nullptr;

        // Word is not present, insert and return reference for backpatching
        ++size_;
        entryIter->next=entryAllocator.allocate();
        new (entryIter->next) Entry(word,hashValue);
        return &entryIter->next->word;
    }
}

/// Find an element inside the hash table, return true or false depending on whether it exists.
template<class T,class HashFunc>
bool HashSet<T,HashFunc>::find(const T& word) const {
    uint32_t hashValue=(uint32_t)hashFunc(word);
    return find(word, hashValue);
}

/// Find an element inside the hash table, return true or false depending on whether it exists. Hash is known in advance.
template<class T,class HashFunc>
bool HashSet<T,HashFunc>::find(const T& word, uint32_t hashValue) const {
    const Entry* entry=entries[hashValue&mask];
    // Unroll empty slot branch
    if (entry==nullptr) {
        return false;
    } else {
        // Unroll immediate hit branch
        if (entry->hashValue==hashValue&&entry->word==word) return true;

        // Iterate to the end of the chain
        for (;entry->next!=nullptr;entry=entry->next) if (entry->next->hashValue==hashValue&&entry->next->word==word) return true;

        return false;
    }
}

/// Pre allocate for size many elements. Can not be called when the hash already contains elements, see grow.
template<class T,class HashFunc>
void HashSet<T,HashFunc>::hintSize(uint32_t size) {
    for (exp=1;(uint32_t)(1<<exp)<size;++exp);
    entries.resize(1<<exp);
    mask=(1ull<<exp)-1ull;
}




/// A fast, chaining HashMap
template<class T,class Value,uint64_t PartitionCount=1,class HashFunc=Crc32Hash>
class HashMap {
    /// An entry inside the hash map
    struct Entry {
        /// The key
        T word;
        /// The value for this key
        Value value;
        /// The hash value
        uint32_t hashValue;
        /// Pointer to chain of entries for collisions
        Entry* next;

        /// Constructor
        Entry () : hashValue(0),next(nullptr) {}
        /// Constructor
        Entry (const T& word,uint32_t hashValue) : word(word),hashValue(hashValue),next(nullptr) {}
    };

    /// An instance of the hash funtion functor
    HashFunc hashFunc;
    /// The hashmap of pointers to actual entries
    std::vector<Entry*> entries;
    /// The pool allocator. Since this hashmap can be partitioned for concurrent access (but never is), there is one pool allocator per partition
    std::array<Pool<Entry>,PartitionCount> entryAllocator;
    /// The exponent, see HashSet and summary for details
    uint64_t exp;
    /// The mask, see HashSet and summary for details
    uint64_t mask;
    /// The number of elements inside the hash, again partitioned into PartitionCount partitions for potential concurrency (unused)
    std::array<uint64_t,PartitionCount> size_;
    /// The size this hash map was allocated with
    uint64_t allocSize;
    /// The mask used to determine the partition for an element (unused)
    uint64_t partitionMask;


public:
    /// Constructor
    HashMap() { size_.fill(0); } // XXX remove and fix errors
    /// Constructor
    HashMap(uint64_t size) : HashMap() { hintSize(size); }
    /// Inserts an element into the hashtable. Returns the value for that key, regardless if an insertion happened or not. Hash value supplied externally
    Value* tryInsert(const T& word,uint32_t hashValue);
    /// Inserts an element into the hashtable. Returns the value for that key, regardless if an insertion happened or not.
    Value* tryInsert(const T& word) { return tryInsert(word,hashFunc(word)); }
    /// Finds an element inside the hashmap, returns the value if found or nullptr otherwise. Hash value supplied externally
    Value* find(const T& word,uint32_t hashValue);
    /// Finds an element inside the hashmap, returns the value if found or nullptr otherwise
    Value* find(const T& word) { return find(word,hashFunc(word)); }
    /// Alternative find: uses a type that can differ from T, e.g. T=string, T2=string_ref
    template <class T2> Value* altFind(const T2& word);
    /// Pre allocates the hash to be able to contain size elements efficiently. Also, see summary and HashSet
    void hintSize(uint32_t size);
    /// Returns the current number of elements inside the hash
    size_t size() const { return std::accumulate(size_.begin(),size_.end(),0); }
    /// Grows the hash to twice its capacity
    void grow() { grow(2*capacity()); };
    /// Grows the hash to be able to efficiently contain 2* capacity elements
    void grow(uint32_t newSize);
    /// Retrieves the mask used to determine an elements partition (unused)
    uint64_t getPartitionMask() const { return partitionMask; }
    /// Returns the current capacity of the hash
    size_t capacity() const { return entries.size(); };
};


/// Grows the hash to be able to efficiently contain 2* capacity elements
template<class T,class Value,uint64_t PartitionCount,class HashFunc>
void HashMap<T,Value,PartitionCount,HashFunc>::grow(uint32_t newSize) {
    allocSize=newSize;
    // Grow
    for (exp=1;(uint32_t)(1<<exp)<newSize;++exp);
    std::vector<Entry*> newEntries(1<<exp);
    mask=(1ull<<exp)-1ull;

    // Copy entries over
    for (Entry* entryChain : entries) {
        for  (Entry *e=entryChain, *next; e!=nullptr; e=next) {
            Entry*& entry = newEntries[e->hashValue&mask];
            if (entry==nullptr) { // immediate hit: use slot for e
                entry=e;
                next=e->next;
                e->next=nullptr;
            } else { // slot already in use: prepend e to chain
                next=e->next;
                e->next=entry;
                entry=e;
            }
        }
    }

    // Switch to new entry vector
    entries=std::move(newEntries);
}

/// Inserts an element into the hashtable. Returns the value for that key, regardless if an insertion happened or not.
template<class T,class Value,uint64_t PartitionCount,class HashFunc>
Value* HashMap<T,Value,PartitionCount,HashFunc>::tryInsert(const T& word,uint32_t hashValue) {
    uint64_t index=hashValue&partitionMask;
    hashValue>>=8;
    Entry*& entry=entries[hashValue&mask];
    // Unroll empty slot branch
    if (entry==nullptr) {
        ++size_[index];
        entry=entryAllocator[index].allocate();
        new (entry) Entry(word,hashValue);
        return &entry->value;
    } else {
        // Unroll immediate hit branch
        if (entry->hashValue==hashValue&&entry->word==word) return &entry->value;

        // Iterate to the end of the chain
        Entry* entryIter;
        for (entryIter=entry;entryIter->next!=nullptr;entryIter=entryIter->next) if (entryIter->next->hashValue==hashValue&&entryIter->next->word==word) return &entryIter->next->value;

        // Word is not present, insert and return reference for backpatching
        ++size_[index];
        entryIter->next=entryAllocator[index].allocate();
        new (entryIter->next) Entry(word,hashValue);
        return &entryIter->next->value;
    }
}

/// Finds an element inside the hashmap, returns the value if found or nullptr otherwise. Hash value supplied externally
template<class T,class Value,uint64_t PartitionCount,class HashFunc>
Value* HashMap<T,Value,PartitionCount,HashFunc>::find(const T& word,uint32_t hashValue) {
    hashValue>>=8;
    Entry* entry=entries[hashValue&mask];
    // Unroll empty slot branch
    if (entry==nullptr) {
        return nullptr;
    } else {
        // Unroll immediate hit branch
        if (entry->hashValue==hashValue&&entry->word==word) return &entry->value;

        // Iterate to the end of the chain
        for (;entry->next!=nullptr;entry=entry->next) if (entry->next->hashValue==hashValue&&entry->next->word==word) return &entry->next->value;

        return nullptr;
    }
}

/// Alternative find: uses a type that can differ from T, e.g. T=string, T2=string_ref
template<class T,class Value,uint64_t PartitionCount,class HashFunc>
template<class T2>
Value* HashMap<T,Value,PartitionCount,HashFunc>::altFind(const T2& word) {
    uint32_t hashValue=hashFunc(word);
    hashValue>>=8;
    Entry* entry=entries[hashValue&mask];
    // Unroll empty slot branch
    if (entry==nullptr) {
        return nullptr;
    } else {
        // Unroll immediate hit branch
        if (entry->hashValue==hashValue&&entry->word==word) return &entry->value;

        // Iterate to the end of the chain
        for (;entry->next!=nullptr;entry=entry->next) if (entry->next->hashValue==hashValue&&entry->next->word==word) return &entry->next->value;

        return nullptr;
    }
}

/// Pre allocates the hash to be able to contain size elements efficiently. Also, see summary and HashSet
template<class T,class Value,uint64_t PartitionCount,class HashFunc>
void HashMap<T,Value,PartitionCount,HashFunc>::hintSize(uint32_t size) {
    this->allocSize=size;
    for (exp=1;(uint32_t)(1<<exp)<size;++exp);
    entries.resize(1<<exp);
    mask=(1ull<<exp)-1ull;

    if (PartitionCount<=1) { partitionMask=0; return; }
    uint64_t partitionExp;
    for (partitionExp=1;(uint32_t)(1<<partitionExp)<PartitionCount;++partitionExp);
    partitionMask=(1ull<<partitionExp)-1ull;
}



}
