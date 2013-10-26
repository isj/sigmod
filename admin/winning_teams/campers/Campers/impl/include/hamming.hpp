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

The Matcher implementation for hamming distance queries.
*/

#pragma once

#include "core.h"
#include "matcher.hpp"


namespace campers {

struct HammingData {
    static const uint64_t coverRefreshLimit=500;
    // Compute cover?
    static const bool cover=true;

    /// A frequency filter
    Frequency freq;
    /// The coefficient
    uint8_t coefficient;
    /// Constructor
    HammingData() {}
    /// Constructor
    HammingData(boost::string_ref word) {
        freq=Frequency(word);
        coefficient=freq.getCoefficient();
    }
};

/// Matcher based index for exact match queries
class Hamming : public Matcher<HammingData> {
public:
    /// Matches a document
    std::vector<QueryID> matchDocument(const Tokenizer& index) override;
};

}

