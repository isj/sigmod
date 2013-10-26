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

#include "edit_distance.hpp"
#include "metrics.hpp"
#include <array>
#include <atomic>
#include <cassert>
#include <unordered_set>
#include <signal.h>
#include <set>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/parallel_for_each.h>
#include <tbb/parallel_for.h>
#include <tbb/task_group.h>
#include <x86intrin.h>

using namespace std;


namespace campers {

/// Matches a document against this matcher
std::vector<QueryID> EditDistance::matchDocument(const Tokenizer& index) {
    // Collect results to map them to queries later
    struct ResultEntry { int8_t dist; QueryWord* qw; };
    tbb::concurrent_vector<ResultEntry> resEntries;

    // Vector to mark query words as not needed any more (due to cover pruning).
    std::vector<int8_t> dead(queryWordIds.endIndex());

    // Functor body which matches a range of query words inside the run vector against the document contained in the index tokenizer / meta store
    auto theLambda=[&dead,&resEntries,&index,this](const tbb::blocked_range<size_t>& range)->void {
        for (size_t i=range.begin();i!=range.end();++i) {
        auto& qw=runVector[i];
        if (qw.id!=-1u&&!dead[qw.id]) {
            int8_t bestDistance=99;

            // Probe cache for last word that matched the query word and check if the document contains that word
            qw.cache[0].mutex.lock();
            if (index.find(qw.cache[0].toStrRef(),qw.cache[0].hash)) {
                bestDistance=1;
            } else if (index.find(qw.cache[1].toStrRef(),qw.cache[1].hash)) {
                 bestDistance=2;
            } else if (index.find(qw.cache[2].toStrRef(),qw.cache[2].hash)) {
                 bestDistance=3;
            }
            qw.cache[0].mutex.unlock();

            // Regular matching
            boost::string_ref best;

            // Necessary only when we haven't found a great match using the cache
            if (bestDistance>qw.minDistance) {
                // We only need to find matches even tighter than those from the cache
                uint64_t bound=std::min<uint64_t>(qw.maxDistance, bestDistance-1);

                // Iterate over all possible word lengths inside the tokenizer
                for (int64_t length=std::max<unsigned>(MIN_WORD_LENGTH,qw.len-bound),limit=std::min<unsigned>(MAX_WORD_LENGTH,qw.len+bound);length<=limit;++length) {
                    // Comoact the filter boundary. A word can not match if the differency between the two frequency filters is > filterDiff
                    uint64_t diff=std::abs(length - qw.len);
                    uint32_t filterDiff=2*bound-diff;

                    // Compute the offset between fast filters and normal filters. <<1 is required because normal filters are larger
                    intptr_t filterOffset=reinterpret_cast<uintptr_t>(index.freq[length].data())-(reinterpret_cast<uintptr_t>(index.freqFast[length].data())<<1);

                    // Compute the range inside the filter vector of filters which can match at all because of Haar-Wavelet pruning
                    unsigned minCoeff=(qw.coefficient>filterDiff)?(qw.coefficient-filterDiff):0,maxCoeff=(qw.coefficient+filterDiff+1>65)?65:qw.coefficient+filterDiff+1;
                    for (auto iter=index.freqFast[length].data()+index.offsets[length][minCoeff],limit=index.freqFast[length].data()+index.offsets[length][maxCoeff];iter!=limit;++iter) {
                        // Check fast filter and skip if the word can not match
                        if ((*iter).delta(qw.freqFast)>filterDiff) continue;

                        // Check normal frequency filter and skip if the word can not match
                        Frequency& normalFilter=*reinterpret_cast<Frequency*>((reinterpret_cast<uintptr_t>(iter)<<1)+filterOffset);
                        if (normalFilter.delta(qw.freq)>filterDiff) continue;

                        // Match the word against the edit distance
                        uint64_t windex=iter-index.freqFast[length].data();
                        auto word=index.getWords(length)[windex];

                        int8_t d = similarity_levenshtein(word,boost::string_ref(qw.wordMemory.data(),qw.len));
                        if (d<bestDistance) {
                            // Good hit, remember it
                            bestDistance=d;
                            best=word;
                        }
                        // Distance bound is good enough so that all queries with this query word match, abort matching loop
                        if (bestDistance<=qw.minDistance) {
                            length=99; break;
                        }
                    }

                }
            }

            // Update cache and save result if we matched
            if (bestDistance<=qw.maxDistance) {
                if (best.length()) {
                    // Update query word inside the cache
                    qw.cache[0].mutex.lock();
                    QueryWord& queryWord=*(qw.qw);
                    auto index=std::max<int8_t>(0,bestDistance-1);
                    queryWord.cache[index].set(best);
                    qw.cache[index].set(best,queryWord.cache[index].hash);
                    qw.cache[0].mutex.unlock();
                }

                // Remember match
                resEntries.push_back(ResultEntry{bestDistance,qw.qw});
            } else {
                // This word did not match at all, prune all words which we do not need to check since they would only have
                // helped a query if this word also matches
                for (auto& id : deathList[qw.id]) dead[id]=1;
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
        QueryWord* qw=re.qw;
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

    // Return result
    return std::move(results);
}

}

