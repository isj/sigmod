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

#ifndef TBB_PROCESSOR_HPP
#define TBB_PROCESSOR_HPP

#include "processors/processor.hpp"
#include <iostream>
#include <memory>
#include <tbb/concurrent_queue.h>
#include <tbb/task_group.h>
#include <tbb/task_scheduler_init.h>


namespace campers {

class TbbProcessor : public Processor {
    std::unique_ptr<tbb::task_scheduler_init> init;
    // Keeps all currently available results that has not been returned yet
    tbb::concurrent_queue<Document> docs;
    // Main task groups
    tbb::task_group matchingTg;
    tbb::task_group coverTg;

public:
    TbbProcessor();
    ~TbbProcessor() override { matchingTg.wait(); }
    ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist);
    ErrorCode EndQuery(QueryID query_id);
    ErrorCode MatchDocument(DocID doc_id, const char* doc_str);
    ErrorCode GetNextAvailRes(DocID* p_doc_id,unsigned int* p_num_res,QueryID** p_query_ids);
    void beforeMatchDocumentBlock() override;
    void afterGetNextAvailResBlock() override;
};

}

#endif /* end of include guard: TBB_PROCESSOR_HPP */
