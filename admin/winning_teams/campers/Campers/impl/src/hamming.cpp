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

#include "hamming.hpp"
#include "metrics.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <set>
#include <unordered_set>
#include <tbb/parallel_for.h>

namespace campers {

/// Matches a document with all hamming distance words and queries
std::vector<QueryID> Hamming::matchDocument(const Tokenizer& index) {
    // Results are stored and later used to find all matching queries
    struct ResultEntry { int8_t dist; const QueryWord* qw; };
    tbb::concurrent_vector<ResultEntry> resEntries;

    // Vector to mark query words as not needed any more (due to cover pruning).
    std::vector<int8_t> dead(queryWordIds.endIndex());

    // Functor body which matches a range of query words inside the run vector against the document contained in the index tokenizer / meta store
    auto theLambda=[&dead,&resEntries,&index,this](const tbb::blocked_range<size_t>& range)->void {
        for (size_t i=range.begin();i!=range.end();++i) {
            auto& qw=runVector[i];
            if (qw.id!=-1u&&!dead[qw.id]) {
                // Compact the filter boundary. A word can not match if its Haar-Wavelet coefficient is within filterDiff from the query words coefficient.
                uint32_t filterDiff=2*qw.maxDistance;

                // Find best distance
                int8_t bestDistance=99;
                if(qw.len <= 16) {
                    // Short word
                    __m128i d1 = _mm_loadu_si128((__m128i *) (qw.wordMemory.data()));
                    unsigned minCoeff=(qw.coefficient>filterDiff)?(qw.coefficient-filterDiff):0,maxCoeff=(qw.coefficient+filterDiff+1>65)?65:qw.coefficient+filterDiff+1;
                    // Compute the range inside the word vector  which can match at all because of Haar-Wavelet pruning
                    for (auto iter=index.words[qw.len].getRawData()+(index.offsets[qw.len][minCoeff]<<4),limit=index.words[qw.len].getRawData()+(index.offsets[qw.len][maxCoeff]<<4);iter!=limit;iter+=16) {
                        __m128i d2 = _mm_loadu_si128((__m128i *) (iter));
                        bestDistance=std::min(bestDistance,(int8_t)similarity_hamming(d1,d2));
                    }
                } else {
                    // Long word
                    __m128i d1_1 = _mm_loadu_si128((__m128i *) (qw.wordMemory.data()));
                    __m128i d1_2 = _mm_loadu_si128((__m128i *) (qw.wordMemory.data()+16));
                    unsigned minCoeff=(qw.coefficient>filterDiff)?(qw.coefficient-filterDiff):0,maxCoeff=(qw.coefficient+filterDiff+1>65)?65:qw.coefficient+filterDiff+1;
                    // Compute the range inside the word vector  which can match at all because of Haar-Wavelet pruning
                    for (auto iter=index.words[qw.len].getRawData()+(index.offsets[qw.len][minCoeff]<<5),limit=index.words[qw.len].getRawData()+(index.offsets[qw.len][maxCoeff]<<5);iter!=limit;iter+=32) {
                        __m128i d2_1 = _mm_loadu_si128((__m128i *) (iter));
                        __m128i d2_2 = _mm_loadu_si128((__m128i *) (iter+16));
                        bestDistance=std::min<int8_t>(bestDistance,(int8_t)similarity_hamming(d1_1,d2_1)+(int8_t)similarity_hamming(d1_2,d2_2));
                    }
                }

                // Distance bound is good enough so that all queries with this query word match, abort matching loop
                if (bestDistance<=qw.maxDistance) {
                    resEntries.push_back(ResultEntry{bestDistance,qw.qw});
                } else {
                    // This word did not match at all, prune all words which we do not need to check since they would only have
                    // helped a query if this word also matches
                    for (auto& id : deathList[qw.id]) { dead[id]=1; }
                }
            }
        }
    };

    // Execute in parallel in two waves to maximize the effect of pruning
    tbb::parallel_for(tbb::blocked_range<size_t>(0,runVector.size()/2,100),theLambda);
    tbb::parallel_for(tbb::blocked_range<size_t>(runVector.size()/2,runVector.size(),100),theLambda);

    // Compute resulting queries from matching query word
    std::vector<QueryID> results;
    std::vector<uint8_t> queryMatchCounts(maxQueryId+1);
    for (auto& re : resEntries) {
        const QueryWord* qw=re.qw;
        int8_t bestDistance=re.dist;

        // Adjust number of matching query words for all queries containing this query word
        for (int8_t dist=bestDistance; dist<=qw->maxDistance; ++dist) {
            for (auto qinfo : qw->queriesByDistance[dist]) {
                auto& matchCount=queryMatchCounts[qinfo.id];
                if (matchCount==0) {
                    matchCount=qinfo.wordCount;
                }
                if (matchCount==1) {
                    results.push_back(qinfo.id);
                } else {
                    --matchCount;
                }
            }
        }
    }

    return std::move(results);
}

}

