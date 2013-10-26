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

Processor is an abtract class which provides an interface for specific
processing strategies. It is used so that multiple ways of handling incoming
API calls can be implemented easily. For instance, a processor implementation
could be based on a queue of tasks, another could be single threaded and yet
another could collect statistics of the underlying benchmark.
*/

#pragma once

#include "document.hpp"
#include "exactmatch.hpp"
#include "hamming.hpp"
#include "edit_distance.hpp"
#include "query.hpp"
#include <algorithm>
#include <cstdint>
#include <unordered_map>


namespace campers {

/// An abstract processor which implements all API calls
class Processor {
public:
    /// Struct holding one index for each distance metric
    struct Indexes {
        /// Exact Match index
        ExactMatch exactIndex;
        /// Hamming index
        Hamming hammingIndex;
        /// Edit Distance index
        EditDistance edIndex;
    };

protected:
    /// All indexes used for matching
    Indexes indexes;

    /// Constructor
    Processor() {}
    /// Destructor
    virtual ~Processor() {}

public:
    /// Adds a query to the system
    virtual ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist) = 0;
    /// Removes an existing query from the system
    virtual ErrorCode EndQuery(QueryID query_id) = 0;
    /// Matches a document
    virtual ErrorCode MatchDocument(DocID doc_id, const char* doc_str) = 0;
    /// Retrieves one result from the system
    virtual ErrorCode GetNextAvailRes(DocID* p_doc_id,unsigned int* p_num_res,QueryID** p_query_ids) = 0;

    /// Called right after InitializeIndex is called
    virtual void afterInitializeIndex() {}
    /// Called right after one or more calls to GetNextAvailRes when the next call is different to GetNextAvailRes
    virtual void afterGetNextAvailResBlock() {}
    /// Called right after one or more calls to MatchDocument when the next call is different to MatchDocument
    virtual void afterMatchDocumentBlock() {}
    /// Called right after one or more calls to EndQuery when the next call is different to EndQuery
    virtual void afterEndQueryBlock() {}
    /// Called right after one or more calls to StartQuery when the next call is different to StartQuery
    virtual void afterStartQueryBlock() {}

    /// Called right before one or more calls to GetNextAvailRes when the previous call was different than GetNextAvailRes
    virtual void beforeGetNextAvailResBlock() {}
    /// Called right before one or more calls to MatchDocument when the previous call was different than MatchDocument
    virtual void beforeMatchDocumentBlock() {}
    /// Called right before one or more calls to EndQuery when the previous call was different than EndQuery
    virtual void beforeEndQueryBlock() {}
    /// Called right before one or more calls to StartQuery when the previous call was different than StartQuery
    virtual void beforeStartQueryBlock() {}
};

}
