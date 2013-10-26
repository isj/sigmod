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

Struct holding the results for a single document. This struct is put into a queue
which is used to deliver one result when GetNextAvailRes is called.
*/

#pragma once

#include "core.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>


namespace campers {

/// Keeps all query id results associated with a document, internally sorts them
struct Document
{
	/// The document id
	DocID doc_id;
	/// The number of queries that match this document
    uint32_t query_count;
    /// A malloced stretch of memory holding all query ids
    QueryID* query_ids;

    /// Constructor
    Document() : doc_id(0),query_count(0),query_ids(nullptr) {}
    /// Constructor
    Document(DocID docId,std::vector<QueryID>& queryIds) : doc_id(docId),query_count(queryIds.size()) {
    	// Sort query ids
        std::sort(queryIds.begin(),queryIds.end());
    	// Copy all document ids into memory which has been allocated using
    	// malloc (as it is later free'ed using free() by the driver)
        query_ids=reinterpret_cast<QueryID*>(malloc(query_count*sizeof(QueryID)));
        memcpy(query_ids,queryIds.data(),query_count*sizeof(QueryID));
    }
};

}
