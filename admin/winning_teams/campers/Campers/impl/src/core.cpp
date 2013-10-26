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

Main starting point of the api impementation. This transforms API calls to calls to a processor
which in turn implements the api. This file is build like a state automaton to allow for callbacks
to be executed whenever a block of intructions is over.
*/

#include "core.h"
#include "processors/stats_processor.hpp"
#include "processors/singlethread_processor.hpp"
#include "processors/tbb_processor.hpp"
#include <memory>

using namespace std;
using namespace campers;


/// Possible states
enum class State { StartQuery, EndQuery, InitializeIndex, MatchDocument, GetNextAvailRes };
/// The state variable
static State state=State::InitializeIndex;

/// Allows for easy processor switching
using ProcessorType=TbbProcessor;
/// The actual processor instance
static std::unique_ptr<ProcessorType> processor;

/// Implementation of initialize index, we create a processor instance here
ErrorCode InitializeIndex(){
    state=State::InitializeIndex;
    processor=std::unique_ptr<ProcessorType>(new ProcessorType());
    return EC_SUCCESS;
}

/// Implementation of destroy index, we destroy the processor here
ErrorCode DestroyIndex(){
    processor.reset(nullptr);
    return EC_SUCCESS;
}

/// Implementation of start query
ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist)
{
    if (state!=State::StartQuery) {
        switch (state) {
            case State::EndQuery: processor->afterEndQueryBlock(); break;
            case State::GetNextAvailRes: processor->afterGetNextAvailResBlock(); break;
            case State::InitializeIndex: processor->afterInitializeIndex(); break;
            case State::MatchDocument: processor->afterMatchDocumentBlock(); break;
            default: throw;
        };
        processor->beforeStartQueryBlock();
        state=State::StartQuery;
    }
    if (match_dist==0) match_type=MT_EXACT_MATCH;
    return processor->StartQuery(query_id,query_str,match_type,match_dist);
}

/// Implementation of end query
ErrorCode EndQuery(QueryID query_id)
{
    if (state!=State::EndQuery) {
        switch (state) {
            case State::GetNextAvailRes: processor->afterGetNextAvailResBlock(); break;
            case State::InitializeIndex: processor->afterInitializeIndex(); break;
            case State::MatchDocument: processor->afterMatchDocumentBlock(); break;
            case State::StartQuery: processor->afterStartQueryBlock(); break;
            default: throw;
        };
        processor->beforeEndQueryBlock();
        state=State::EndQuery;
    }
    return processor->EndQuery(query_id);
}

/// Implementation of match document
ErrorCode MatchDocument(DocID doc_id, const char* doc_str)
{
    if (state!=State::MatchDocument) {
        switch (state) {
            case State::EndQuery: processor->afterEndQueryBlock(); break;
            case State::GetNextAvailRes: processor->afterGetNextAvailResBlock(); break;
            case State::InitializeIndex: processor->afterInitializeIndex(); break;
            case State::StartQuery: processor->afterStartQueryBlock(); break;
            default: throw;
        };
        processor->beforeMatchDocumentBlock();
        state=State::MatchDocument;
    }
    return processor->MatchDocument(doc_id,doc_str);
}

/// Implementation of get next res
ErrorCode GetNextAvailRes(DocID* p_doc_id,unsigned int* p_num_res,QueryID** p_query_ids) {
    if (state!=State::GetNextAvailRes) {
        switch (state) {
            case State::EndQuery: processor->afterEndQueryBlock(); break;
            case State::InitializeIndex: processor->afterInitializeIndex(); break;
            case State::MatchDocument: processor->afterMatchDocumentBlock(); break;
            case State::StartQuery: processor->afterStartQueryBlock(); break;
            default: throw;
        };
        processor->beforeGetNextAvailResBlock();
        state=State::GetNextAvailRes;
    }
    return processor->GetNextAvailRes(p_doc_id,p_num_res,p_query_ids);
}
