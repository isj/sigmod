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

#ifndef STATS_PROCESSOR_HPP
#define STATS_PROCESSOR_HPP

#include "utils.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <set>
#include <unordered_set>

namespace campers {

template<class Base>
class StatsProcessor : public Base {
    /// Document stats
    uint64_t documentCount = 0;
    uint64_t minDocumentLength = std::numeric_limits<uint64_t>::max();
    uint64_t maxDocumentLength = 0;
    std::vector<uint64_t> documentLengths;
    std::vector<uint64_t> uniqueWordsPerDocument;
    std::vector<uint64_t> wordsPerDocument;
    uint32_t documentBlocks = 0;
    std::unordered_set<std::string> docwords;

    /// Query stats
    std::vector<uint64_t> queriesByType = std::vector<uint64_t>(3,0);
    std::vector<std::set<std::string>> queryWordsByType = std::vector<std::set<std::string>>(3);
    std::vector<uint64_t> wordsPerQuery;
    uint32_t currentlyActiveQueries = 0;
    std::vector<uint32_t> activeQueries;
    std::array<std::array<uint32_t,4>,3> distanceHistogram;

public:

    StatsProcessor()
    {
        for (auto& d : distanceHistogram)
            d.fill(0);
    }

    ~StatsProcessor()
    {
        std::cout << "Document Stats ----------" << std::endl;
        std::cout << "Count:             " << documentCount << std::endl;
        std::cout << "Min:               " << minDocumentLength << std::endl;
        std::cout << "Max:               " << maxDocumentLength << std::endl;
        std::cout << "Avg:               " << std::accumulate(documentLengths.begin(),documentLengths.end(),0)*1.0f/documentLengths.size() << std::endl;
        std::cout << "Med:               " << documentLengths[documentLengths.size()/2] << std::endl;
        std::cout << "unique words/doc:  " << std::accumulate(uniqueWordsPerDocument.begin(),uniqueWordsPerDocument.end(),0)*1.0f/uniqueWordsPerDocument.size() << std::endl;
        std::cout << "words/doc:         " << std::accumulate(wordsPerDocument.begin(),wordsPerDocument.end(),0)*1.0f/wordsPerDocument.size() << std::endl;
        std::cout << "count doc blocks:  " << documentBlocks << std::endl;
        std::cout << "unique total words:" << docwords.size() << std::endl;

        std::cout << "Query Stats -------------" << std::endl;
        std::cout << "Count:             " << std::accumulate(queriesByType.begin(),queriesByType.end(),0) << std::endl;
        std::cout << "Exact:             " << queriesByType[0] << std::endl;
        std::cout << "Hamming:           " << queriesByType[1] << std::endl;
        std::cout << "Levenshtein:       " << queriesByType[2] << std::endl;
        std::cout << "Unique Ex words    " << queryWordsByType[0].size() << std::endl;
        std::cout << "Unique Ha words    " << queryWordsByType[1].size() << std::endl;
        std::cout << "Unique Le words    " << queryWordsByType[2].size() << std::endl;
        std::cout << "words/query        " << std::accumulate(wordsPerQuery.begin(),wordsPerQuery.end(),0)*1.0f/wordsPerQuery.size() << std::endl;
        std::cout << "avg active queries " << std::accumulate(activeQueries.begin(),activeQueries.end(),0)*1.0f/activeQueries.size() << std::endl;

        for (auto type : {1,2}) {
            for (auto distance : {1,2,3}) {
                std::cout << "type " << type << " dist " << distance << " " << distanceHistogram[type][distance] << std::endl;
            }
        }
    }

    ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist)
    {
        // Keep stats
        currentlyActiveQueries++;
        queriesByType[match_type]++;
        auto words = tokenize(query_str,false);
        wordsPerQuery.push_back(words.size());
        for(auto& iter : words)
            queryWordsByType[match_type].insert(iter);
        ++distanceHistogram[match_type][match_dist];

        // Execute
        return Base::StartQuery(query_id,query_str,match_type,match_dist);
    }

    ErrorCode EndQuery(QueryID query_id)
    {
        currentlyActiveQueries--;
        return Base::EndQuery(query_id);
    }

    ErrorCode MatchDocument(DocID doc_id, const char* doc_str)
    {
        // Keep stats
        documentCount++;
        uint64_t docLen=strlen(doc_str);
        minDocumentLength=std::min(minDocumentLength,docLen);
        maxDocumentLength=std::max(maxDocumentLength,docLen);
        documentLengths.push_back(docLen);
        uniqueWordsPerDocument.push_back(tokenize(doc_str,true).size());
        wordsPerDocument.push_back(tokenize(doc_str,false).size());

        for (auto& word : tokenize(doc_str,true))
            docwords.insert(word);

        // Execute
        return Base::MatchDocument(doc_id,doc_str);
    }

    virtual void afterMatchDocumentBlock()
    {
        activeQueries.push_back(currentlyActiveQueries);
        documentBlocks++;
    }
};

}

#endif /* end of include guard: STATS_PROCESSOR_HPP */
