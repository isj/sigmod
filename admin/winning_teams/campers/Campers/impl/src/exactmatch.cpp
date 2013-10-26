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

#include "exactmatch.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <unordered_set>
#include <set>
#include <utility>

namespace campers {

// Operator for HashMap::altFind
bool operator==(const std::string& s1, const boost::string_ref& s2) {
    auto sz1=s1.size();
    return s2.size()==sz1 && memcmp(s1.data(),s2.data(),sz1)==0;
}

// Match document against exact words
std::vector<QueryID> ExactMatch::matchDocument(const Tokenizer& index) {
    std::vector<QueryID> results;
    std::vector<uint8_t> queryMatchCounts(maxQueryId+1);

    // Depending on the size of the document and the number of query words,
    // either probe the document's hash table with each query word or vice versa.
    if (index.size() > runVector.size()) {
        for (const CompactQW& qw : runVector) {
            boost::string_ref word(qw.wordMemory.data(),qw.len);
            if (index.find(word)) {
                // Result computation, adjust matching word count for all queries containing this query word
                for (auto qinfo : qw.qw->queriesByDistance[0]) {
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
    } else {
        for (uint64_t len=MIN_WORD_LENGTH; len<=MAX_WORD_LENGTH; ++len) {
            for (const auto& word : index.getWords(len)) {
                const QueryWord* qw=words.altFind(word);
                if(qw) {
                    // Result computation, adjust matching word count for all queries containing this query word
                    for (auto qinfo : qw->queriesByDistance[0]) {
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
        }
    }

    return std::move(results);
}

}
