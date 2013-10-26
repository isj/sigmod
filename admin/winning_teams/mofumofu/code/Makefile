# Copyright (c) 2013 KAUST - InfoCloud Group (All Rights Reserved)
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# Authors: Amin Allam  <amin.allam@kaust.edu.sa>,
#          Fuad Jamour <fuad.jamour@kaust.edu.sa>
#
# Current version: 1.0 (initial release)
	

# Build targets (your implementation targets should go in IMPL_O)
IMPL_O=./impl/ref_impl/core.cpp

USER_FLAGS=-O3 -march=corei7 -mcx16 -msahf -maes -mpclmul -mpopcnt -msse4.1 -msse4.2 -mtune=corei7 -std=c++0x -m64 -fPIC -I. -DSSE4_2 -DNDEBUG 
USER_LIBS=

# Compiler flags
CC  = gcc
CXX = g++
CFLAGS=$(USER_FLAGS) -O3 -fPIC -Wall -std=c++11 -fopenmp -g -I. -I./. -I././impl -I././impl/ref_impl -I././impl/cpp-btree-1.0.1 -I././impl/include
CXXFLAGS=$(CFLAGS)
LDFLAGS=-lpthread

# The name of the library that will be built
LIBRARY=core

lib: $(IMPL_O)
	$(CXX) $(CXXFLAGS) -shared -pthread $(LDFLAGS) -o lib$(LIBRARY).so $(IMPL_O) $(USER_LIBS)

clean:
	rm -f lib$(LIBRARY).so
	find . -name '*.o' -print | xargs rm -f
