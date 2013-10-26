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

Multiple datastructres used all over the system. See comments for each structure.
*/

#pragma once

#include "core.h"
#include <array>
#include <boost/utility/string_ref.hpp>
#include <cstring>
#include <stack>
#include <string>
#include <vector>

namespace campers {

/// A map from key to value based on a vector. Only words as expected for dense, zero based keys.
template<class K,class V>
class DenseMap {
    /// The underlying vector
    std::vector<V> data;

public:
    /// An iterator for the dense map
    class iterator {
        /// Reference to the underlying data vector
        std::vector<V>& data;
        /// Index inside the vector
        size_t index;

    public:
        /// Constructor
        iterator(std::vector<V>& data,size_t index) : data(data),index(index) {}
        /// Dereferenciation
        const std::pair<const K,V> operator*() { return std::make_pair(index,data[index]); }
        /// Not equal based on index compare
        bool operator!=(const iterator& other) const { return index!=other.index; }
        /// Increments the iterator to the next value
        iterator& operator++() { ++index; return *this; }
        /// Operator minus
        friend size_t operator-(const iterator& i1, const iterator& i2) { return i1.index-i2.index; }
    };

    /// A const version of the above iterator
    class const_iterator {
        /// Reference to the underlying data vector
        const std::vector<V>& data;
        /// Index inside the vector
        size_t index;

    public:
        /// Constructor
        const_iterator(const std::vector<V>& data,size_t index) : data(data),index(index) {}
        /// Dereferenciation
        const std::pair<const K,V> operator*() const { return std::make_pair(index,data[index]); }
        /// Not equal based on index compare
        bool operator!=(const const_iterator& other) const { return index!=other.index; }
        /// Increments the iterator to the next value
        const_iterator& operator++() { ++index; return *this; }
        /// Operator minus
        friend size_t operator-(const const_iterator& i1, const const_iterator& i2) { return i1.index-i2.index; }
    };

    /// Constructor
    DenseMap() {}
    /// Constructor
    DenseMap(size_t size) : data(size) {}
    /// Index operator with map semantics
    V& operator[](const K& index) {
        if (data.size()<=(size_t)index) data.resize(index+1);
        return data[index];
    }
    /// Const version of the index operator with bounds check
    const V& operator[](const K& index) const {
        return data[index];
    }
    /// Returns an iterator to the first element
    iterator begin() { return iterator(data,0); }
    /// Returns an iterator to the one-past-last element
    iterator end() { return iterator(data,data.size()); }
    /// Returns a const iterator to the first element
    const_iterator begin() const { return const_iterator(data,0); }
    /// Returns a const iterator to the one-past-last element
    const_iterator end() const { return const_iterator(data,data.size()); }
    /// Returns the number of elements inside the dense map
    size_t size() const { return data.size(); }
    /// Returns the last element inside the dense map
	const V& back() const { return data.back(); }
    /// Resizes the dense map to a given size
    void resize(size_t newSize) {data.resize(newSize, V());}
    /// Removes all elements from the dense map
    void clear() { data.clear(); }
};


/// Datastructure allowing unique id reuse
class UniqueIdStore {
    /// Compile time constant disabling reuse for debugging purposes
    static const bool disableReuse=0;
    /// The value of the next id we will assign fresh (if there is none to reuse)
    int64_t nextId=0;
    /// The stack of ids to be reused
    std::stack<int64_t> freeIds;

public:
    /// Gets a unique id which is not currently in use
    int64_t allocateId() {
        if (freeIds.size()) {
            int64_t result=freeIds.top();
            freeIds.pop();
            return result;
        } else {
            return nextId++;
        }
    }

    /// Marks a unique id as no longer in active use and ready for reuse
    void deallocateId(int64_t id) {
        if (!disableReuse)
            freeIds.push(id);
    }
    /// Return the first id that has not been used (for < iteration)
    uint64_t endIndex() const { return nextId; }
    /// Clear the id supply
    void clear() { nextId=0; while(freeIds.size()) freeIds.pop(); }
};

// Tokenizes a cstring into a vector of c++ strings
inline std::vector<std::string> tokenize(const char* str,bool removeDups=true) {
    std::vector<std::string> result;
    char wordBuffer[MAX_WORD_LENGTH+1];
    uint64_t pos=0;

    for(const char* c=str;;++c) {
        if (*c == ' ' || *c == 0) {
            // End of word (possibly end of string)
            wordBuffer[pos]=0;
            if (pos!=0) result.emplace_back(wordBuffer);
            pos=0;
            if (*c == 0) break;
        } else
            wordBuffer[pos++]=*c;
    }

    if (removeDups) {
        std::sort(result.begin(),result.end());
        result.erase(std::unique(result.begin(),result.end()),result.end());
    }

    return std::move(result);
}

}

/// A vector for words of the same length, words are padded to either 16 or 32 bytes with 0x00 to make sse handling easier
class FixedLengthWordVector {
    /// Length of the words inside this vector
    uint64_t length;
    /// Length of the padded word, either 16 or 32
    uint64_t increment;
    /// Where the words are stored
    std::vector<char> data;

public:
    /// An iterator for the words inside this vector
    struct const_iterator {
        /// Back reference to the actual vector
        const FixedLengthWordVector& wv;
        /// Index inside the vector
        uint64_t index;

        /// Constructor
        const_iterator(const FixedLengthWordVector& wv,uint64_t index) : wv(wv),index(index) {}
        /// Increment to next word
        const_iterator& operator++() { index+=wv.increment; return *this; }
        /// Iterator comparison
        bool operator==(const const_iterator& other) const { return index==other.index; }
        bool operator!=(const const_iterator& other) const { return index!=other.index; }
        /// Dereferenciation
        const boost::string_ref operator*() const { return boost::string_ref(&wv.data[index],wv.length); }
    };

    /// Constructor
    FixedLengthWordVector() : length(-1) {}
    /// Constructor
    FixedLengthWordVector(uint64_t length) : length(length),increment(((length+15)/16)*16) {}
    /// Move Constructor
    FixedLengthWordVector(FixedLengthWordVector&& other) : length(other.length),increment(other.increment),data(std::move(other.data)) { other.length=0; other.increment=0; }
    /// Move assignment
    FixedLengthWordVector& operator=(FixedLengthWordVector&& other) { length=other.length; increment=other.increment; other.length=0; other.increment=0; data=std::move(other.data); return *this; }
    /// Sets the length of words contained in this vector
    void setLength(uint64_t length) { this->length=length; this->increment=((length+15)/16)*16; }
    /// Iterator to first element inside the vector
    const_iterator begin() const { return const_iterator(*this,0); }
    /// Iterator to one-past-last element inside the vector
    const_iterator end() const { return const_iterator(*this,data.size()); }
    /// Returns the number of elements inside the vector
    size_t size() const { return data.size()>>(increment==16?4:5); }
    /// Returns the underlying data
    const char* getRawData() const { return data.data(); }
    /// Returns the underlying data size
    uint64_t getRawSize() const { return data.size(); }
    /// Index access to vector elements
    boost::string_ref operator[](uint64_t index) const {
        return boost::string_ref(data.data()+(index<<(increment==16?4:5)),length);
    }
    /// Inserts a word into the vector padding it with 0x00
    void insert(boost::string_ref word) {
        uint64_t size=data.size();
        data.resize(size+increment);
        memcpy(&data[size],word.data(),length);
        memset(&data[size+length],0x00,increment-length);
    }
    /// Swaps two words inside the vector
    inline void swapIndexes(uint64_t index1,uint64_t index2) {
        char* pos1=data.data()+(index1<<(increment==16?4:5));
        char* pos2=data.data()+(index2<<(increment==16?4:5));
        if (pos1==pos2) return;
        std::array<char,MAX_WORD_LENGTH+1> mem;
        memcpy(mem.data(),pos1,increment);
        memcpy(pos1,pos2,increment);
        memcpy(pos2,mem.data(),increment);
    }
};
