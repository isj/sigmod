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

This file contains the implementation of the edit distance and hamming distance metrics.
*/

#pragma once

#include "core.h"
#include "utils.hpp"
#include <boost/utility/string_ref.hpp>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <x86intrin.h>


namespace campers {

// [1] Helpful description: Section 3.3 in http://www.cs.uta.fi/~helmu/pubs/spire03a.pdf
// [2] Other helpful description: http://www.cs.uta.fi/~helmu/pubs/A2001-10.pdf, Figure 5
// [3] http://apps.topcoder.com/forums/;jsessionid=2E79B293F6EA2F509795A6C303F74CAA?module=Thread&threadID=639500&start=0&mc=8#1095148

/// Custom levenshtein distance. This is essentially the algorithm given in [1], Figure 5 which
/// was originally developed by Myeres et. al (G. Myers. A fast bit-vector algorithm for approximate
/// string matching based on dynamic progamming. Journal of the ACM, 46(3): 395-415, 1999.).
/// An example implementation is given at [2]. We found [3] helpful in understanding the original
/// paper better.
///
/// We improved the algorithm by replacing the inner loop with sse instructions effectively
/// removing it
///
/// [1] http://www.cs.uta.fi/~helmu/pubs/A2001-10.pdf
/// [2] http://apps.topcoder.com/forums/?module=Thread&threadID=639500&start=0&mc=8#1095148
/// [3] http://www.cs.uta.fi/~helmu/pubs/A2001-10.pdf (especially Figure 5)
static inline uint64_t similarity_levenshtein(boost::string_ref a,boost::string_ref b)
{
    unsigned qlen=a.length();
    unsigned blen=b.length();
    const char* qptr=a.data();
    const char* bptr=b.data();
    if (qlen>blen) {
        std::swap(qlen,blen);
        std::swap(qptr,bptr);
    }
    __m128i brep1,brep2;
    uint64_t bmask;
    if (blen<=16) {
        brep1=_mm_lddqu_si128(reinterpret_cast<const __m128i*>(bptr));
        bmask=(1<<blen)-1;
    } else {
        brep1=_mm_lddqu_si128(reinterpret_cast<const __m128i*>(bptr));
        brep2=_mm_lddqu_si128(reinterpret_cast<const __m128i*>(bptr)+1);
        bmask=(blen==32)?0xFFFFFFFF:((1<<blen)-1);
    }

    uint64_t dmj=blen;
    uint64_t vp = (uint64_t)-1;
    uint64_t vn = 0;
    uint64_t hp = 0;
    uint64_t hn = 0;
    for (unsigned j = 0; j < qlen; j++) {
        uint64_t pm;
        __m128i c=_mm_set1_epi8(qptr[j]);
        if (blen<=16) {
            pm=_mm_movemask_epi8(_mm_cmpeq_epi8(c,brep1))&bmask;
        } else {
            pm=(_mm_movemask_epi8(_mm_cmpeq_epi8(c,brep1))|(_mm_movemask_epi8(_mm_cmpeq_epi8(c,brep2))<<16))&bmask;
        }
        uint64_t d = ((((pm & vp) + vp)) ^ vp) | pm | vn;
        hp = (vn | ~(d | vp));
        uint64_t hpw = (hp << 1) | 1;
        hn = d & vp;

        int m=blen-1;
        if (hp&(1<<m)) ++dmj;
        if (hn&(1<<m)) --dmj;

        vp = (hn << 1) | ~(d | hpw);
        vn = d & hpw;
    }
    return dmj;
}

/// Hamming distance using sse. Both words must be padded and aligned to 16 bytes and loaded
/// into sse registers a and b. Then, we compute the xor of the two registers and use saturation
/// arithmetics to propagte differences from the differing bit to the while byte.
static inline unsigned similarity_hamming(__m128i a,__m128i b)
{
    __m128i mask=_mm_set1_epi8(254);
    union { __m128i a; uint64_t b[2]; } x;
    x.a=_mm_adds_epu8(_mm_xor_si128(a,b),mask);
    return (_mm_popcnt_u64(x.b[0])+_mm_popcnt_u64(x.b[1]))-(128-16);
}

}
