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

The Matcher implementation for exact match queries.
*/

#pragma once

#include "core.h"
#include "matcher.hpp"


namespace campers {

/// Matcher based index for exact match queries
class ExactMatch : public Matcher<NoCover> {
public:
    /// Matches a document
    std::vector<QueryID> matchDocument(const Tokenizer& index) override;
};

}
