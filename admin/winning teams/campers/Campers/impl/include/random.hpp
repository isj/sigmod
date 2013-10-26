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

#include <cstdint>

namespace campers {

/// A fast, custom user space random number generator
class Random
{
   /// The state
   uint64_t state;

   public:
   /// Constructor
   explicit Random(uint64_t seed=88172645463325252ull)  : state(seed) {}


   /// Get the next value
   inline uint64_t next() {
       state^=(state<<13);
       state^=(state>>7);
       return (state^=(state<<17));
   }
};

}
