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

This implements two histogram (character frequency) based filters. The histogram of a
word is an array that gives the number of times each possible letter in the alphabet
occurs inside a certain word. For the word "food", the array would be

a b c d e f g h ... o ... z
0 0 0 1 0 1 0 0 ... 2 ... 0

We define the delta d between two such histogram arrays as the sum of all absolute
differences of the counts of each pair of characters. Shortened example, see deltaSlow()
for the algorithm:

vec a 0 0 1 3 2 1
vec b 0 1 0 3 2 0
delta 0+1+1+0+0+1

The filter can be used for exact matches, hamming distance and edit distance although
it is only beneficial for edit distnace as matching exact words and hamming distance if
comparatively cheap anyway. For two words to be within edit distance t, their frequency
delta must be <= 2*t. This can be improved by looking at the possible edit operations. If
the length of the two words differs, that can only be caused by insert or delete operations.
These operations change the delta by 1, not 2 like substitution. Therefore, the criterion can
be improved to <= 2*t-lengthdiff where lengthdiff is the absolute length difference
between the two words.

While not overly costly, the naive way of executing the filter does not yield a sufficient
result. We therefore use SSE instructions to impove the filter by about a factor of 3. Also,
we have a shortened version of the filter which combines some characters. Its pruning
capability is not as good as the full version of the filter but it can be executed on one
sse register instead of two.
*/

#pragma once

#include <boost/utility/string_ref.hpp>
#include <cassert>
#include <x86intrin.h>


namespace campers {

/// The full 26 character frequency filter
struct Frequency {
    /// Maps each letter in the alphabet to a slot inside two sse 16 byte registers. A good mapping for english words
    /// was obtained by randomized search while monitoring the prune rate using a script such as:
    /// #!/bin/bash
    /// while true; do
    ///     seq 0 31 | sort -R | head -n 26 | xargs | tr ' ' ',' > include/mapping.hpp
    ///     make -j >/dev/null 2>&1 && ./driver test_data/inter* | grep prune | awk '{ print $2 }' | tr '\n' ' ' | cat - include/mapping.hpp
    /// done
    ///
    /// Alternatively, a good but not ideal mapping is simply mapping a-m to the first register and n-z to the second.
    static constexpr unsigned char charMap[26]={ 24,26,22,2,13,20,14,1,30,27,23,16,5,6,8,4,3,9,25,7,29,28,31,17,21,15 };
    /// Union trick. We use two sse 128 bit registers to represent 32 char values for 26 characters
    union { uint64_t l[4]; unsigned char f[32]; __m128i p[2]; };

    /// Comparison for adding Frequency to maps
    bool operator<(const Frequency& other) const {
        return std::lexicographical_compare(l,l+4,other.l,other.l+4);
    }

    /// Comparison for adding Frequency to hash maps
    bool operator==(const Frequency& other) const {
        return l[0]==other.l[0]&&l[1]==other.l[1]&&l[2]==other.l[2]&&l[3]==other.l[3];
    }

    /// Constructor
    Frequency() {}

    /// Constructor which builds the frequency vector for a given word
    explicit Frequency(const boost::string_ref& word) {
        for (unsigned index=0;index!=32;++index) f[index]=0;
        for (unsigned index=0;index<word.length();++index)
            ++f[charMap[word[index]-'a']];
    }

    /// Delta computation using the naive mechanism adding the absolute of each character count difference
    unsigned deltaSlow(const Frequency& f2) const {
        unsigned result=0; for (unsigned index=0;index!=26;++index)
        if (f[index]<f2.f[index])
            result+=f2.f[index]-f[index];
        else
            result+=f[index]-f2.f[index];
        return result;
    }

    /// Get coefficient
    unsigned getCoefficient() const {
        auto sum1=_mm_sad_epu8(p[0],_mm_setzero_si128());
        auto fsum1=_mm_extract_epi64(_mm_add_epi8(sum1,_mm_srli_si128(sum1,8)),0);
        auto sum2=_mm_sad_epu8(p[1],_mm_setzero_si128());
        auto fsum2=_mm_extract_epi64(_mm_add_epi8(sum2,_mm_srli_si128(sum2,8)),0);
        return 32+fsum1-fsum2;
    }

    /// Delta computation using sse, the sse instruction compute the same result as the naive approach but
    /// in less cycles.
    unsigned delta(const Frequency& f2) const {
        __m128i half=_mm_add_epi8(_mm_sad_epu8(p[0],f2.p[0]),_mm_sad_epu8(p[1],f2.p[1]));
        return _mm_extract_epi64(_mm_add_epi8(half,_mm_srli_si128(half,8)),0);
    }
};

/// The frequency filter which maps 26 characters on 16 slots so that it can be evaluated on one sse
/// register instead of two. The mapping is constructed from a dictionary such that letters which do
/// not occur together frequently are mapped to the same slot.
struct FrequencyFast {
    /// A mapping compacting the 26 characters of the alphabet into 16 slots of one sse register. The mapping
    /// was extracted similarly to that of the regular Frequency filter.
    static constexpr unsigned char charMap[26]={13,0,1,14,9,2,3,4,5,5,6,0,7,15,2,8,9,12,11,6,10,11,12,13,14,15};
    /// Union trick. We use two sse 128 bit registers to represent 32 char values for 26 characters
    union { uint64_t l[2]; unsigned char f[16]; __m128i p; };

    /// Comparison for adding Frequency to maps
    bool operator<(const FrequencyFast& other) const {
        return _mm_extract_epi64(p,1) < _mm_extract_epi64(other.p,1) ||
        (_mm_extract_epi64(p,1) == _mm_extract_epi64(other.p,1) && _mm_extract_epi64(p,0) < _mm_extract_epi64(other.p,0));
    }

    /// Comparison for adding Frequency to hash maps
    bool operator==(const FrequencyFast& other) const {
        return l[0]==other.l[0]&&l[1]==other.l[1];
    }

    /// Constructor
    FrequencyFast() {}

    /// Constructor which builds the frequency vector for a given word
    explicit FrequencyFast(const boost::string_ref& word) {
        for (unsigned index=0;index!=16.;++index) f[index]=0;
        for (unsigned index=0;index<word.length();++index)
            ++f[charMap[word[index]-'a']];
    }

    /// Delta computation using the naive mechanism adding the absolute of each character count difference
    unsigned deltaSlow(const FrequencyFast& f2) const {
        unsigned result=0; for (unsigned index=0;index!=16;++index)
        if (f[index]<f2.f[index])
            result+=f2.f[index]-f[index];
        else
            result+=f[index]-f2.f[index];
        return result;
    }

    /// Delta computation using sse, the sse instruction compute the same result as the naive approach but
    /// in less cycles.
    unsigned delta(const FrequencyFast& f2) const {
        __m128i half=_mm_sad_epu8(p,f2.p);
        return _mm_extract_epi64(_mm_add_epi8(half,_mm_srli_si128(half,8)),0);
    }
};

}

namespace std {

/// Hash for FrequencyFast crc
template<>
struct hash<campers::FrequencyFast> {
    size_t operator()(const campers::FrequencyFast& f) const {
        return _mm_crc32_u64(_mm_crc32_u64(0,reinterpret_cast<const uint64_t*>(&f)[0]),reinterpret_cast<const uint64_t*>(&f)[1]);
    }
};

/// Hash for Frequency using crc
template<>
struct hash<campers::Frequency> {
    size_t operator()(const campers::Frequency& f) const {
        return _mm_crc32_u64(_mm_crc32_u64(_mm_crc32_u64(_mm_crc32_u64(0,reinterpret_cast<const uint64_t*>(&f)[0]),
            reinterpret_cast<const uint64_t*>(&f)[1]),reinterpret_cast<const uint64_t*>(&f)[2]),(reinterpret_cast<const uint64_t*>(&f)[3]));
    }
};
}
