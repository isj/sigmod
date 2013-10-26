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

#include "processors/singlethread_processor.hpp"


namespace campers {

SinglethreadProcessor::SinglethreadProcessor() {}

ErrorCode SinglethreadProcessor::StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist)
{
    auto tokens=tokenize(query_str);

    if (match_type==MT_EDIT_DIST)
        indexes.edIndex.addQuery(query_id,match_dist,tokens);
    else if (match_type==MT_HAMMING_DIST)
        indexes.hammingIndex.addQuery(query_id,match_dist,tokens);
    else if (match_type==MT_EXACT_MATCH)
        indexes.exactIndex.addQuery(query_id,0,tokens);

	return EC_SUCCESS;
}

ErrorCode SinglethreadProcessor::EndQuery(QueryID query_id)
{
    // Remove from index
    indexes.edIndex.removeQuery(query_id);
    indexes.hammingIndex.removeQuery(query_id);
    indexes.exactIndex.removeQuery(query_id);

	return EC_SUCCESS;
}

ErrorCode SinglethreadProcessor::MatchDocument(DocID doc_id, const char* doc_str)
{
    uint64_t len=strlen(doc_str);

    Tokenizer tokenizer;
    tokenizer.parse(doc_str,len);

    std::vector<uint32_t> edQueries,hammingIds,exactQueries;

    exactQueries=indexes.exactIndex.matchDocument(tokenizer);
    hammingIds=indexes.hammingIndex.matchDocument(tokenizer);
    edQueries=indexes.edIndex.matchDocument(tokenizer);

    std::vector<QueryID> query_ids; query_ids.reserve(exactQueries.size()+hammingIds.size()+edQueries.size());
    query_ids.insert(query_ids.end(),edQueries.begin(),edQueries.end());
    query_ids.insert(query_ids.end(),hammingIds.begin(),hammingIds.end());
    query_ids.insert(query_ids.end(),exactQueries.begin(),exactQueries.end());

    docs.push(Document(doc_id,query_ids));

	return EC_SUCCESS;
}

ErrorCode SinglethreadProcessor::GetNextAvailRes(DocID* p_doc_id,unsigned int* p_num_res,QueryID** p_query_ids) {
    Document doc;
    while (!docs.try_pop(doc)) usleep(10);

    *p_doc_id=doc.doc_id;
    *p_num_res=doc.query_count;
    *p_query_ids=doc.query_ids;

    return EC_SUCCESS;
}

}
