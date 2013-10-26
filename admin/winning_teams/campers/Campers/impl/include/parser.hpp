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

Parser mixing which efficiently parses a space separated string into separate words. The
parser is build using sse 4.2 string instructions.
*/

#pragma once

#include <boost/utility/string_ref.hpp>
#include <cassert>
#include <iostream>
#include <x86intrin.h>


/// Parser which can be added to an existing class. insert() in that class is called for each word
/// parse() finds. See tokenizer.hpp for an example.
template<class T>
class ParserMixin {
public:
    /// Parses a space separated string into words.
    uint64_t parse(const char* str,uint64_t length);
};

/// Parses a space separated string into words.
template<class T>
uint64_t ParserMixin<T>::parse(const char* str,uint64_t length) {
    // Determine bounds
    const char* cur = str;
    const char* finalSSE=(length>=16) ? str+length-16 : cur;
    uint8_t len = 0;
    // Determine set of characters we search for with sse
    const char* searchChars = " \0";
    __m128i _set = _mm_loadu_si128((__m128i*)(searchChars));

    // While we can apply sse parsing
    while (cur<finalSSE) {
        // Find break characters
        __m128i _cur = _mm_loadu_si128((__m128i*)(cur));
        uint64_t index = _mm_cmpestri(_set, 2, _cur, 16, _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_LEAST_SIGNIFICANT);
        if(index < 16) {
            // Extract next word and continue
            if (len+index > 0) reinterpret_cast<T*>(this)->insert(boost::string_ref(cur-len,len+index));
            cur+=index;
            if(*cur == '\0')
                break;
            len = 0;
            ++cur;
        } else {
            // No word within 16 bytes
            len += 16;
            cur += 16;
        }
    }

    // Parse final part using regular for loop parsing so that we do not run over
    // a segment boundary.
    for (const char* begin=cur-len,*iter=cur;;++iter) {
        if (*iter == ' '||*iter==0) {
            if (iter!=begin) reinterpret_cast<T*>(this)->insert(boost::string_ref(begin,iter-begin));
            begin=iter+1;
            if (*iter==0) break;
        }
    }

    return cur-str;
};
