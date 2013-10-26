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

This class is the base for all three types of matching. EditDistance, Hamming and ExactMatch. It
encapsulates the methods required for adding and removing queries as well as some of the algorithms
used to improve matching performance (also see README).

The main speedup algorithm implemented here is coverage pruning. Coverage pruning uses the fact that if
there is two queries with four query words in the system like this:

q1 a b c
q1 a b e

then if b does not match, a, c and e do not need to be checked since there's no query which would benefit
from a, c or e matching now that b did not match.
*/

#pragma once

#include "core.h"
#include "frequency.hpp"
#include "hash.hpp"
#include "tokenizer.hpp"
#include <atomic>
#include <iostream>
#include <tbb/concurrent_vector.h>
#include <tbb/spin_mutex.h>
#include <tbb/task_group.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace campers {

struct NoData {
    static const uint64_t coverRefreshLimit=500;
    /// Constructor
    NoData() {}
    /// Constructor
    NoData(boost::string_ref word) {}
    /// Compute cover?
    static const bool cover=true;
};

struct NoCover {
    static const uint64_t coverRefreshLimit=500;
    /// Constructor
    NoCover() {}
    /// Constructor
    NoCover(boost::string_ref word) {}
    /// Compute cover?
    static const bool cover=false;
};

template<class Data=NoData>
struct Matcher {
    struct Query;

    /// A cache for the last words which successfully matched a query word
    struct MatchCache {
        /// Lightweight mutex to prevent an update/read race condition
        tbb::spin_mutex mutex;
        /// The word which last matched the query word
        std::array<char,MAX_WORD_LENGTH+1> lastMatch __attribute__ ((aligned (16)));
        /// The hash of that word
        uint32_t hash;
        /// The length of the word
        int8_t len;

        /// Set the word inside the cache
        inline void set(const boost::string_ref& word) { set(word,Crc32Hash()(word)); }
        /// Set the word inside the cache supllying the hash value to be used
        inline void set(const boost::string_ref& word, uint32_t hashKey) {
            len=word.size();
            memcpy(lastMatch.data(),word.data(), len);
            hash=hashKey;
        }
        /// Set the word inside the cache using the word saved in another cache
        inline void set(const MatchCache& cache) {
            memcpy(lastMatch.data(),cache.lastMatch.data(), cache.len);
            len=cache.len;
            hash=cache.hash;
        }
        /// Constructor
        MatchCache() : len(0) {}
        /// Copy constructor
        MatchCache(const MatchCache& cache) { set(cache); }
        /// Copy assigment
        MatchCache& operator=(const MatchCache& other) { set(other); return *this; }
        /// Convert cache to boost::string_ref
        const boost::string_ref toStrRef() const { return boost::string_ref(lastMatch.data(), len); }
    };

    /// A query word which is defined as a unique word which is found in multiple queries and with multiple distances
    struct QueryWord : public Data {
        struct QueryInfo {
            uint32_t id;
            uint32_t wordCount;
            bool operator==(const QueryInfo& other) const { return id==other.id; }
            QueryInfo() : id(-1u),wordCount(-1u) {}
            QueryInfo(uint32_t id,uint32_t wordCount) : id(id),wordCount(wordCount) {}
        };

        /// The query word id
        uint32_t id;
        /// Index inside the run vector
        uint32_t compactIndex;
        /// The number of queries containing this word
        uint32_t totalCount;
        /// The maximum distance at which this word exists inside a query
        int8_t maxDistance;
        /// The minimum distance at which this word exists inside a query
        int8_t minDistance;
        /// The word buffer
        std::array<char,MAX_WORD_LENGTH+1> wordMemory __attribute__ ((aligned (16)));
        /// The word
        boost::string_ref word;
        /// All queries for which this word is a match provided that it is within distance or closer
        DenseMap<int8_t,std::vector<QueryInfo>> queriesByDistance;
        /// Wheather the set has been initialized or not
        bool combiSetInitialized;
        /// The combination set
        std::vector<QueryWord*> combiSet;

        QueryWord() : id(-1u) {}
        QueryWord(boost::string_ref word) : Data(word),totalCount(0),maxDistance(0),minDistance(99),combiSetInitialized(0) {}
    };

    /// A more compact representation of the QueryWord to be used inside a vector for iteration. Improves memory locality
    struct CompactQW : public Data {
        /// The query word id
        uint32_t id;
        /// The word length
        int8_t len;
        /// The maximum distance at which this word exists inside a query
        int8_t maxDistance;
        /// The minimum distance at which this word exists inside a query
        int8_t minDistance;
        /// A pointer back to the actual full query word struct
        QueryWord* qw;
        /// The word buffer
        std::array<char,MAX_WORD_LENGTH+1> wordMemory __attribute__ ((aligned (16)));
        /// Constructor
        CompactQW(QueryWord& qw) : id(qw.id),len(qw.word.length()),maxDistance(qw.maxDistance),minDistance(qw.minDistance),qw(&qw),wordMemory(qw.wordMemory) { Data::operator=(qw); }
    };

    /// A query as received from a library call
    struct Query {
        /// The external id of this query
        uint32_t id;
        /// The distance at which a matching is still valid
        int8_t distance;
        /// The number of words
        int8_t wordCount;
        /// Query words
        std::vector<QueryWord*> queryWords;

        Query() : id(-1u) {}
        Query(uint32_t id,int8_t distance,const std::vector<std::string>& words) : id(id),distance(distance) {}
    };

    /// IDs for QueryWords
    UniqueIdStore queryWordIds;
    /// Where we store all queries (so we can use pointers to queries)
    std::unordered_map<QueryID,Query> queries;
    /// All query words, these are not necessarily active right now
    HashMap<std::string,QueryWord> words;
    /// List of query words that are excluded by another query word not matching
    std::vector<std::vector<uint32_t>> deathList,newDeathList;
    /// Vector of compact words to iterate over
    std::vector<CompactQW> runVector;
    /// Highest known query id
    int32_t maxQueryId;
    /// Count of inactive qws inside the runvector
    uint64_t inactive;
    /// Remove count
    uint64_t removeCount;
    /// Internal task group
    tbb::task_group tg;
    /// Indicator to swap death lists which have been computed asynchoniously
    bool swapDeathLists;
    // Vector used to iterate over all query words
    std::vector<QueryWord*> allQws;


public:
    /// Constructor
    Matcher() : maxQueryId(-1),inactive(0),removeCount(0),swapDeathLists(0) { words.hintSize(500); };
    /// Destructor
    virtual ~Matcher() { /* wait for all thread to finish */ tg.wait(); };
    /// Returns the number of active unique words
    size_t getQueryWordSize() const { return words.size(); }
    /// Adds a query to this matcher
    void addQuery(QueryID id,int8_t distance,const std::vector<std::string>& newWords);
    /// Removes a query from this matcher
    void removeQuery(QueryID id);
    /// Computes a pruning cover
    void computeCover();
    /// Applies the newly computed pruning cover
    void applyCover();
    /// Matches a document against this matcher
    virtual std::vector<QueryID> matchDocument(const Tokenizer& index) = 0;
};

/// Adds a query to this matcher
template<class Data>
void Matcher<Data>::addQuery(QueryID id,int8_t distance,const std::vector<std::string>& newWords) {
    // Add meta data of the query itself
    Query& query=queries[id];
    query.id=id;
    query.distance=distance;
    query.wordCount=newWords.size();
    // Compute highest query id for iterations
    maxQueryId=std::max<int32_t>(id,maxQueryId);

    // Add query words
    for (auto& word : newWords) {
        // Find or insert the word into the queryword hashtable
        QueryWord& qw=*words.tryInsert(word);
        // Work around for our hashtable not being able to call a constructor
        // with parameters, we in-place construct new words a second time
        if (qw.id==-1u) {
            new (&qw) QueryWord(word); allQws.push_back(&qw);
        }

        // Determine min and max distance for the word
        qw.minDistance=std::min(qw.minDistance,distance);
        qw.maxDistance=std::max(qw.maxDistance,distance);

        // Increment number of queries this word is part of
        ++qw.totalCount;

        // Initialize new words
        if (qw.id==-1u) {
            // Allocate a query word id
            qw.id=queryWordIds.allocateId();

            // Zero fill memory for the string and copy the word
            memset(qw.wordMemory.data(),0x00,qw.wordMemory.size());
            memcpy(qw.wordMemory.data(),word.data(), word.size());
            qw.word=boost::string_ref(qw.wordMemory.data(), word.size());

            // Add to runvector
            qw.compactIndex=runVector.size();
            runVector.push_back(CompactQW(qw));
        } else if (qw.totalCount==1) {
            // If we just increased total count to 1 this word just became active, either set it to active
            // in the runVector or readd it if it's not inside the runVector
            if (qw.compactIndex!=-1u) {
                --inactive;
                runVector[qw.compactIndex].id=qw.id;
            } else {
                qw.compactIndex=runVector.size();
                runVector.push_back(CompactQW(qw));
            }
        }

        // Add the query word to the new queries meta data
        query.queryWords.push_back(&qw);

        // Update min/max inside runVector
        runVector[qw.compactIndex].minDistance=qw.minDistance;
        runVector[qw.compactIndex].maxDistance=qw.maxDistance;

        // Add query to the list of queries this query word exists in
        qw.queriesByDistance[distance].emplace_back(query.id,query.wordCount);
    }

    // Check if we should grow the query word hash table
    if (words.size()>= words.capacity()*0.8) {
        unsigned sz=words.capacity();
        // Default: double in size
        unsigned newSize=2*sz;
        // If the hash table is really small we instantly jump to 50000
        if (sz<50000) newSize=50000; else
        // If the hash table has been grown once, agressively grow it again
        if (sz<500000) newSize=500000;
        // Actually grow the hash table
        words.grow(newSize);
    }

    // Compute cover and death lists, see computeCover() for a commented, slightly different version of this algorithm.
    if (Data::cover) {
        deathList.resize(queryWordIds.endIndex());
        for (QueryWord* qw1 : query.queryWords) {
            if (!qw1->combiSetInitialized) {
                qw1->combiSetInitialized=1;
                auto& set=qw1->combiSet;
                for (auto* qw2 : query.queryWords)
                    if (qw1!=qw2) {
                        set.push_back(qw2);
                        deathList[qw2->id].push_back(qw1->id);
                    }
            } else {
                std::vector<QueryWord*> intersection;
                auto& existingSet=qw1->combiSet;
                for (QueryWord* qw2 : query.queryWords) {
                    if (qw1==qw2) continue;

                    if (std::find(existingSet.begin(),existingSet.end(),qw2)!=existingSet.end()) {
                        intersection.push_back(qw2);
                    }
                }

                for (auto* qw2 : existingSet) {
                    auto iter=std::find(deathList[qw2->id].begin(),deathList[qw2->id].end(),qw1->id);
                    deathList[qw2->id].erase(iter);
                }
                for (auto* qw2 : intersection) {
                    deathList[qw2->id].push_back(qw1->id);
                }

                qw1->combiSet=std::move(intersection);
            }
        }
    }
}

/// Removes a query from this matcher
template<class Data>
void Matcher<Data>::removeQuery(QueryID id) {
    // Find query, do not complain if it doesn't exist as removes are send to all query indexes
    auto iter=queries.find(id);
    if (iter==queries.end()) return;
    Query& q=iter->second;

    // Iterate over all words inside the query which is being removed
    for (QueryWord* qwPtr : q.queryWords) {
        QueryWord& qw=*qwPtr;

        // Decrement number of queries this query word is used in
        --qw.totalCount;

        // Recompute min and max distance
        qw.minDistance=std::min<int8_t>(std::min<int8_t>(qw.queriesByDistance[1].size()?1:99,qw.queriesByDistance[2].size()?2:99),qw.queriesByDistance[3].size()?3:99);
        qw.maxDistance=std::max<int8_t>(std::max<int8_t>(qw.queriesByDistance[1].size()?1:0,qw.queriesByDistance[2].size()?2:0),qw.queriesByDistance[3].size()?3:0);

        if (qw.totalCount==0) {
            // If we decremented totalCount to zero, we have to deactivate this query word as it is no longer used in any queries
            ++inactive;
            runVector[qw.compactIndex].id=-1u;
        } else {
            // Otherwise, we update min and max inside the run vector
            runVector[qw.compactIndex].minDistance=qw.minDistance;
            runVector[qw.compactIndex].maxDistance=qw.maxDistance;
        }

        // Remove query from the query words list of queries it is used in
        auto& slot=qw.queriesByDistance[q.distance];
        auto iter=std::find(slot.begin(),slot.end(),typename QueryWord::QueryInfo(q.id,q.wordCount));
        uint64_t index=std::distance(slot.begin(),iter);
        if (index!=slot.size()-1) *iter=slot.back();
        slot.resize(slot.size()-1);
    }

    queries.erase(id);
    ++removeCount;
}

/// Computes a pruning cover
template<class Data>
void Matcher<Data>::computeCover() {
    if (Data::cover&&words.size()>200&&removeCount>Data::coverRefreshLimit) {
        Matcher<Data>& m=*this; // workaround for gcc 4.7.2 where this can only be captured const for templates
        tg.run([&m]()->void {
            // Reset combinations inside all query words
            for (auto* qw : m.allQws) {
                qw->combiSet.clear();
                qw->combiSetInitialized=0;
            }

            // Recompute combinations and death lists
            m.newDeathList.clear();
            for (auto& queryKv : m.queries) {
                /// For each query
                auto& query=queryKv.second;
                m.newDeathList.resize(m.queryWordIds.endIndex());

                // For each word inside the query
                for (QueryWord* qw1 : query.queryWords) {
                    // There are two cases: Either, the query word has been checked before, or not.
                    if (!qw1->combiSetInitialized) {
                        qw1->combiSetInitialized=1;
                        auto& set=qw1->combiSet;
                        // Previously unknown query word: For every other query word w that occurs with this
                        // query word remember that if w does not match, this word can not possible match since
                        // it only occurs in combination with w.
                        for (auto* qw2 : query.queryWords)
                            if (qw1!=qw2) {
                                set.push_back(qw2);
                                m.newDeathList[qw2->id].push_back(qw1->id);
                            }
                    } else {
                        // This query word has been seen before. Change the pruning scheme such that words which
                        // previously pruned this query word because this query word only occured in combination with
                        // those words and no longer occur only with this query word are removed from the pruning vector.

                        // Compute intersection of words that occur together and this query
                        std::vector<QueryWord*> intersection;
                        auto& existingSet=qw1->combiSet;
                        for (QueryWord* qw2 : query.queryWords) {
                            if (qw1==qw2) continue;

                            if (std::find(existingSet.begin(),existingSet.end(),qw2)!=existingSet.end()) {
                                intersection.push_back(qw2);
                            }
                        }

                        // Update pruning vectors (deathLists)
                        for (auto* qw2 : existingSet) {
                            m.newDeathList[qw2->id].erase(std::find(m.newDeathList[qw2->id].begin(),m.newDeathList[qw2->id].end(),qw1->id));
                        }
                        for (auto* qw2 : intersection) {
                            m.newDeathList[qw2->id].push_back(qw1->id);
                        }

                        // Save update
                        qw1->combiSet=std::move(intersection);
                    }
                }
            }
            // Reset counter and set swapDeathLists such that after the matching run occuring in parallel, the new death list is activated
            m.removeCount=0;
            m.swapDeathLists=1;
        });
    }

    // Defrag run vector. We copy all query words into a vector so that locality is improved. Inside this vector,
    // we do not delete in-place but instead mark words as deleted. From time to time (whenever the number of inactive
    // words is larger than 2% of the total words) we actually remove all inactive words from the runvector.
    if (inactive > words.size()*0.02) {
        uint64_t index=0;
        // Iterate over vector and "fill holes", see std::remove_if. Fix indexes inside the big query words at the same time.
        runVector.erase(std::remove_if(runVector.begin(),runVector.end(),[&index](CompactQW& cqw)->bool {
            if (cqw.id==-1u) {
                cqw.qw->compactIndex=-1u;
                return true;
            } else {
                cqw.qw->compactIndex=index++;
                return false;
            }
        }),runVector.end());
        // Reset inactive word counter
        inactive=0;
    }
}

/// Applies the newly computed pruning cover, swaps pruning vectors with newly computed vectors, see computeCover
template<class Data>
void Matcher<Data>::applyCover() {
    if (!Data::cover) return;
    tg.wait();
    if (swapDeathLists) {
        swapDeathLists=0;
        swap(deathList,newDeathList);
    }
}

}
