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

#ifndef QUERY_HPP_8KXD4XMM
#define QUERY_HPP_8KXD4XMM

#include "core.h"


namespace campers {

// Keeps all information related to an active query
struct Query
{
	/// The externally supplied id of this query
	QueryID query_id;
	/// The type of this query
	MatchType match_type;
	/// The distance of this query
	unsigned int match_dist;
};

}

#endif /* end of include guard: QUERY_HPP_8KXD4XMM */
