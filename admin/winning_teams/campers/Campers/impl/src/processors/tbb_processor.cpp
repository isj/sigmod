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

#define _GLIBCXX_USE_SCHED_YIELD
#include "processors/tbb_processor.hpp"
#include <thread>


namespace campers {
using namespace tbb;

TbbProcessor::TbbProcessor() : init(new tbb::task_scheduler_init(task_scheduler_init::default_num_threads()>>1)) {}

ErrorCode TbbProcessor::StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist)
{
    auto tokens=tokenize(query_str);

    if (match_type==MT_EDIT_DIST) {
        indexes.edIndex.addQuery(query_id,match_dist,tokens);
    } else if (match_type==MT_HAMMING_DIST) {
        indexes.hammingIndex.addQuery(query_id,match_dist,tokens);
    } else if (match_type==MT_EXACT_MATCH) {
        indexes.exactIndex.addQuery(query_id,0,tokens);
    }

	return EC_SUCCESS;
}

ErrorCode TbbProcessor::EndQuery(QueryID query_id)
{
    // Remove from index
    indexes.edIndex.removeQuery(query_id);
    indexes.hammingIndex.removeQuery(query_id);
    indexes.exactIndex.removeQuery(query_id);

	return EC_SUCCESS;
}

ErrorCode TbbProcessor::MatchDocument(DocID doc_id, const char* doc_str)
{
    uint64_t len=strlen(doc_str);
    std::shared_ptr<char> str(reinterpret_cast<char*>(operator new(len+1)));
    memcpy(str.get(),doc_str,len+1);

    matchingTg.run([&,str,len,doc_id,this]() {
        Tokenizer tokenizer;
        tokenizer.parse(str.get(),len);

        std::vector<uint32_t> edQueries,hammingIds,exactQueries;

        tbb::task_group tg;
        tg.run([&,this](){ exactQueries=indexes.exactIndex.matchDocument(tokenizer); });
        tg.run([&,this](){ hammingIds=indexes.hammingIndex.matchDocument(tokenizer); });
        tg.run([&,this](){ edQueries=indexes.edIndex.matchDocument(tokenizer); });
        tg.wait();

        std::vector<QueryID> query_ids; query_ids.reserve(exactQueries.size()+hammingIds.size()+edQueries.size());
        query_ids.insert(query_ids.end(),edQueries.begin(),edQueries.end());
        query_ids.insert(query_ids.end(),hammingIds.begin(),hammingIds.end());
        query_ids.insert(query_ids.end(),exactQueries.begin(),exactQueries.end());

        docs.push(Document(doc_id,query_ids));
    });

	return EC_SUCCESS;
}

ErrorCode TbbProcessor::GetNextAvailRes(DocID* p_doc_id,unsigned int* p_num_res,QueryID** p_query_ids) {
    Document doc;
    while (!docs.try_pop(doc)) std::this_thread::yield();

    *p_doc_id=doc.doc_id;
    *p_num_res=doc.query_count;
    *p_query_ids=doc.query_ids;

    return EC_SUCCESS;
}

void TbbProcessor::beforeMatchDocumentBlock() {
    // Active hyperthreads if useful
    static bool hyperActive=false;
    if (!hyperActive&&indexes.edIndex.getQueryWordSize()>200) {
        init->terminate();
        init.reset(new tbb::task_scheduler_init(task_scheduler_init::default_num_threads()));
        hyperActive=true;
    }

    indexes.edIndex.computeCover();
    indexes.hammingIndex.computeCover();
}

void TbbProcessor::afterGetNextAvailResBlock() {
    indexes.edIndex.applyCover();
    indexes.hammingIndex.applyCover();
}

}
