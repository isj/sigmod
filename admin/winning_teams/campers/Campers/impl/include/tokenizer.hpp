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

Tokenizer is used to hold the metadata for a document. We first parse the document using ParserMixin and then
compute all filters for each word that we've found. All meta data is ordered by the second coefficient inside
the Haar-Wavelet transformation of the Frequency filter such that we can easily exclude filters during matching.
*/

#pragma once

#include "core.h"
#include "frequency.hpp"
#include "hashtable.hpp"
#include "parser.hpp"
#include "utils.hpp"
#include <array>
#include <boost/utility/string_ref.hpp>
#include <cassert>
#include <cstring>
#include <list>
#include <iostream>
#include <vector>


namespace campers {

/// The tokenizer and meta data store
struct Tokenizer : public ParserMixin<Tokenizer> {
    /// All unique words inside the document by length
    std::array<FixedLengthWordVector,MAX_WORD_LENGTH+1> words;
    /// Frequencies matching the words, also by length
    std::array<std::vector<Frequency>,MAX_WORD_LENGTH+1> freq;
    /// FastFrequencies matching the words, also by length
    std::array<std::vector<FrequencyFast>,MAX_WORD_LENGTH+1> freqFast;
    /// Permutation which is applied inside parse, matching the words, also by length
    std::array<std::vector<uint64_t>,MAX_WORD_LENGTH+1> perm;
    /// The Haar-Wavelet coefficients (see summary and README), also by length
    std::array<std::vector<uint8_t>,MAX_WORD_LENGTH+1> coefficient;
    /// The offsets for each Haar-Wavelet coefficient value such that we can iterate over all filters which might actually match, also by length.
    std::array<std::array<unsigned,66>,MAX_WORD_LENGTH+1> offsets;
    /// Hash set used to deduplicate the document words
    HashSet<boost::string_ref> h;
    /// The number of unique words the document contains in total
    uint64_t size_;

public:
    /// Constructor
    Tokenizer() : size_(0) {
        for (uint64_t length=MIN_WORD_LENGTH;length<=MAX_WORD_LENGTH;++length)
            words[length].setLength(length);
    }
    /// Called from the parser to insert a new word
    void insert(boost::string_ref word);
    /// Wrapper for ParserMixing<>::parse
    uint64_t parse(const char* str,uint64_t len);
    /// Allows finding a match inside the deduplication hash (used for exact matching)
    bool find(boost::string_ref word) const { return h.find(word); }
    /// Allows finding a match inside the deduplication hash (used for exact matching), precomputed hash
    bool find(boost::string_ref word, uint32_t hashValue) const { return h.find(word, hashValue); }
    /// Allows access to the underlying deduplication hashset
    const HashSet<boost::string_ref>& getHash() const { return h; }
    /// Returns number of unique words inside the document
    uint64_t size() const { return size_; }
    /// Allows retrieving the words vector
    const FixedLengthWordVector& getWords(uint64_t length) const { return words[length]; }
};

}
