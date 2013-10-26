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

This implements a functor class which wraps the intrinsic crc32 instructions
inside modern cpus and exposes them to compute hashes. Since for this application
we have a known length limit on strings, we do not need a loop to compute a
string has but instead only a switch between different sizes classes.
*/

#pragma once

#include <cstdint>
#include <boost/utility/string_ref.hpp>
#include <string>
#include <x86intrin.h>


/// Adds hashing for boost string references to std::hash
namespace std {
template<>
struct hash<boost::string_ref> {
    size_t operator()(const boost::string_ref& word) const {
        return std::hash<std::string>()((std::string)word);
    }
};
}

namespace campers {

/// Wraps crc32 cpu instructions so that they can be used with unordered_map or our custom hash
class Crc32Hash {
public:
    /// Integers
    inline size_t operator()(uint64_t x) const {
        return _mm_crc32_u64(0,x);
    }
    inline size_t operator()(int64_t x) const {
        return _mm_crc32_u64(0,x);
    }
    inline size_t operator()(uint32_t x) const {
        return _mm_crc32_u32(0,x);
    }
    inline size_t operator()(int32_t x) const {
        return _mm_crc32_u32(0,x);
    }

    /// Pointers
    inline size_t operator()(void* ptr) const {
        return std::hash<void*>()(ptr);
    }

    /// Boost string references
    inline unsigned operator()(const boost::string_ref& s) const {
        unsigned l=s.length();
        const char* from = s.data();
        if (l<=8) {
            return _mm_crc32_u64(0,(reinterpret_cast<const uint64_t*>(from)[0])<<(64-8*l));
        } else if (l<=16) {
            return _mm_crc32_u64(_mm_crc32_u64(0,reinterpret_cast<const uint64_t*>(from)[0]),(reinterpret_cast<const  uint64_t*>(from)[1])<<(128-8*l));
        } else if (l<=24) {
            return _mm_crc32_u64(_mm_crc32_u64(_mm_crc32_u64(0,reinterpret_cast<const uint64_t*>(from)[0]),reinterpret_cast<const  uint64_t*>(from)[1]),(reinterpret_cast<const uint64_t*>(from)[2])<<(128+64-8*l));
        } else {
            return _mm_crc32_u64(_mm_crc32_u64(_mm_crc32_u64(_mm_crc32_u64(0,reinterpret_cast<const uint64_t*>(from)[0]),reinterpret_cast<const uint64_t*>(from)[1]),reinterpret_cast<const uint64_t*>(from)[2]),(reinterpret_cast<const uint64_t*>(from)[3])<<(256-8*l));
        }
    }

    /// Strings
    inline unsigned operator()(const std::string& s) const {
        unsigned l=s.length();
        const char* from = s.data();
        if (l<=8) {
            return _mm_crc32_u64(0,(reinterpret_cast<const uint64_t*>(from)[0])<<(64-8*l));
        } else if (l<=16) {
            return _mm_crc32_u64(_mm_crc32_u64(0,reinterpret_cast<const uint64_t*>(from)[0]),(reinterpret_cast<const  uint64_t*>(from)[1])<<(128-8*l));
        } else if (l<=24) {
            return _mm_crc32_u64(_mm_crc32_u64(_mm_crc32_u64(0,reinterpret_cast<const uint64_t*>(from)[0]),reinterpret_cast<const  uint64_t*>(from)[1]),(reinterpret_cast<const uint64_t*>(from)[2])<<(128+64-8*l));
        } else {
            return _mm_crc32_u64(_mm_crc32_u64(_mm_crc32_u64(_mm_crc32_u64(0,reinterpret_cast<const uint64_t*>(from)[0]),reinterpret_cast<const uint64_t*>(from)[1]),reinterpret_cast<const uint64_t*>(from)[2]),(reinterpret_cast<const uint64_t*>(from)[3])<<(256-8*l));
        }
    }
};

}
