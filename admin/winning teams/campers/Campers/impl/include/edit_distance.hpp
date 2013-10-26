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

#pragma once

#include "core.h"
#include "frequency.hpp"
#include "hash.hpp"
#include "matcher.hpp"
#include "tokenizer.hpp"
#include <atomic>
#include <iostream>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_vector.h>
#include <tbb/task_group.h>
#include <unordered_map>
#include <unordered_set>
#include <map>


namespace campers {


struct EditDistanceData {
    static const uint64_t coverRefreshLimit=500;

    /// A frequency filter
    Frequency freq;
    /// The coefficient
    uint8_t coefficient;
    /// Another frequency filter which can be evaluated faster but filters less
    FrequencyFast freqFast;
    /// Last matches cache, one for each edit distance
    std::array<Matcher<>::MatchCache,3> cache;
    /// Constructor
    EditDistanceData() {}
    /// Constructor
    EditDistanceData(boost::string_ref word) {
        freq=Frequency(word);
        freqFast=FrequencyFast(word);
        coefficient=freq.getCoefficient();

        // Set last match
        for (unsigned i=0; i<3; ++i)
            cache[i].set(word);
    }

    // Compute cover?
    static const bool cover=true;
};

struct EditDistance : public Matcher<EditDistanceData> {
    /// Matches a document against this matcher
    std::vector<QueryID> matchDocument(const Tokenizer& index) override;

    /// Destructor for statistics
    ~EditDistance() {}
};

}

