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

A Processor imlementation that does not use concurrency. This was mainly
used for debugging and is supplied as an example of another processor implementation.
*/

#pragma once

#include "processors/processor.hpp"
#include <tbb/concurrent_queue.h>


namespace campers {

/// Single threaded processor implementation
class SinglethreadProcessor : public Processor {
    /// All indexes
    Indexes indexes;
    // Keeps all currently available results that has not been returned yet
    tbb::concurrent_queue<Document> docs;

public:
	/// Constructor
    SinglethreadProcessor();

    /// Adds a query to the system
    ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist) override;
    /// Removes an existing query from the system
    ErrorCode EndQuery(QueryID query_id) override;
    /// Matches a document
    ErrorCode MatchDocument(DocID doc_id, const char* doc_str) override;
    /// Retrieves one result from the system
    ErrorCode GetNextAvailRes(DocID* p_doc_id,unsigned int* p_num_res,QueryID** p_query_ids) override;
};

}
