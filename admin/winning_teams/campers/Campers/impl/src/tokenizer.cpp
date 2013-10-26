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

#include "tokenizer.hpp"
#include <algorithm>

namespace campers {

/// Applies a permutation given in the last parameter to all vectors given in the preceeding parameters. Used to resort all vectors by permutation.
template<class T1,class T2,class T3,class T4>
static void applyPermutation(T1& vec1,std::vector<T2>& vec2,std::vector<T3>& vec3,std::vector<T4>& vec4,std::vector<uint64_t> permutation) {
    for (uint64_t index=0,limit=permutation.size();index<limit;) {
        vec1.swapIndexes(permutation[index],permutation[permutation[index]]);
        std::swap(vec2[permutation[index]],vec2[permutation[permutation[index]]]);
        std::swap(vec3[permutation[index]],vec3[permutation[permutation[index]]]);
        std::swap(vec4[permutation[index]],vec4[permutation[permutation[index]]]);
        std::swap(permutation[index],permutation[permutation[index]]);
        if (permutation[index]==index) ++index;
    }
}

/// Inserts a word into the meta data store and computes all metadata
void Tokenizer::insert(boost::string_ref word) {
    boost::string_ref* refPtr=h.tryInsert(word);
    if (refPtr!=nullptr) {
        words[word.length()].insert(word); // The word itself
        freq[word.length()].emplace_back(word); // The frequency filter
        freqFast[word.length()].emplace_back(word); // The fast frequency filter
        coefficient[word.length()].push_back(freq[word.length()].back().getCoefficient()); // The Haar-Wavelet coefficient
        perm[word.length()].push_back(perm[word.length()].size()); // The permutation
        ++size_; // Increment size
    }
}

/// Parses a document using the parser mixing and then reorders all data
uint64_t Tokenizer::parse(const char* str,uint64_t len) {
    // Parse
    h.hintSize(len>>4);
    auto res=ParserMixin<Tokenizer>::parse(str,len);

    // Reorder metadata and words by coefficient
    for (uint32_t len=MIN_WORD_LENGTH;len<=MAX_WORD_LENGTH;++len) {
        // Sort coefficient vector
    	std::sort(perm[len].begin(),perm[len].end(),[&](uint64_t i1,uint64_t i2) {return coefficient[len][i1]<coefficient[len][i2];});

        // Apply resulting permutation to all other vectors
    	applyPermutation(words[len],freq[len],freqFast[len],coefficient[len],perm[len]);

        // Compute indexes inside the word vector where all words with a given coefficient start
        uint64_t currentCoef=0;
        offsets[len][0]=0;
        for (uint64_t index=0;index<coefficient[len].size();++index) {
            while (currentCoef<coefficient[len][index]) {
                offsets[len][++currentCoef]=index;
            }
        }
        // Fix coefficients for which there are no words
        for (++currentCoef;currentCoef<offsets[len].size();++currentCoef) {
            offsets[len][currentCoef]=coefficient[len].size();
        }
    }

    return res;
}

}
