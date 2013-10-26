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
IMPL_O=./impl/libs/tbb41_20130116oss/src/rml/server/rml_server.cpp ./impl/libs/tbb41_20130116oss/src/rml/client/rml_omp.cpp ./impl/libs/tbb41_20130116oss/src/rml/client/rml_tbb.cpp ./impl/libs/tbb41_20130116oss/src/rml/client/omp_dynamic_link.cpp ./impl/libs/tbb41_20130116oss/src/old/spin_rw_mutex_v2.cpp ./impl/libs/tbb41_20130116oss/src/old/task_v2.cpp ./impl/libs/tbb41_20130116oss/src/old/concurrent_vector_v2.cpp ./impl/libs/tbb41_20130116oss/src/old/concurrent_queue_v2.cpp ./impl/libs/tbb41_20130116oss/src/tbb/arena.cpp ./impl/libs/tbb41_20130116oss/src/tbb/cache_aligned_allocator.cpp ./impl/libs/tbb41_20130116oss/src/tbb/queuing_mutex.cpp ./impl/libs/tbb41_20130116oss/src/tbb/private_server.cpp ./impl/libs/tbb41_20130116oss/src/tbb/tbb_misc_ex.cpp ./impl/libs/tbb41_20130116oss/src/tbb/concurrent_queue.cpp ./impl/libs/tbb41_20130116oss/src/tbb/tbb_thread.cpp ./impl/libs/tbb41_20130116oss/src/tbb/reader_writer_lock.cpp ./impl/libs/tbb41_20130116oss/src/tbb/critical_section.cpp ./impl/libs/tbb41_20130116oss/src/tbb/concurrent_hash_map.cpp ./impl/libs/tbb41_20130116oss/src/tbb/tbb_statistics.cpp ./impl/libs/tbb41_20130116oss/src/tbb/concurrent_vector.cpp ./impl/libs/tbb41_20130116oss/src/tbb/dynamic_link.cpp ./impl/libs/tbb41_20130116oss/src/tbb/tbb_misc.cpp ./impl/libs/tbb41_20130116oss/src/tbb/task_group_context.cpp ./impl/libs/tbb41_20130116oss/src/tbb/market.cpp ./impl/libs/tbb41_20130116oss/src/tbb/task.cpp ./impl/libs/tbb41_20130116oss/src/tbb/concurrent_monitor.cpp ./impl/libs/tbb41_20130116oss/src/tbb/semaphore.cpp ./impl/libs/tbb41_20130116oss/src/tbb/observer_proxy.cpp ./impl/libs/tbb41_20130116oss/src/tbb/recursive_mutex.cpp ./impl/libs/tbb41_20130116oss/src/tbb/mutex.cpp ./impl/libs/tbb41_20130116oss/src/tbb/itt_notify.cpp ./impl/libs/tbb41_20130116oss/src/tbb/condition_variable.cpp ./impl/libs/tbb41_20130116oss/src/tbb/tbb_main.cpp ./impl/libs/tbb41_20130116oss/src/tbb/queuing_rw_mutex.cpp ./impl/libs/tbb41_20130116oss/src/tbb/spin_mutex.cpp ./impl/libs/tbb41_20130116oss/src/tbb/spin_rw_mutex.cpp ./impl/libs/tbb41_20130116oss/src/tbb/governor.cpp ./impl/libs/tbb41_20130116oss/src/tbb/scheduler.cpp ./impl/libs/tbb41_20130116oss/src/tbb/pipeline.cpp ./impl/libs/tbb41_20130116oss/src/dlopen.c ./impl/libs/lockless_allocator/ll_alloc.c ./impl/src/tokenizer.cpp ./impl/src/exactmatch.cpp ./impl/src/frequency.cpp ./impl/src/core.cpp ./impl/src/edit_distance.cpp ./impl/src/processors/tbb_processor.cpp ./impl/src/processors/singlethread_processor.cpp ./impl/src/hamming.cpp

USER_FLAGS=-march=native 
USER_LIBS=

# Compiler flags
CC  = gcc
CXX = g++
CFLAGS=$(USER_FLAGS) -O3 -fPIC -Wall -std=c++11 -fopenmp -g -I. -I./. -I././impl -I././impl/include -I././impl/include/boost -I././impl/include/boost/utility -I././impl/include/boost/detail -I././impl/include/boost/config -I././impl/include/boost/config/stdlib -I././impl/include/boost/config/no_tr1 -I././impl/include/boost/config/compiler -I././impl/include/boost/config/platform -I././impl/include/boost/config/abi -I././impl/include/processors -I././impl/libs -I././impl/libs/tbb41_20130116oss -I././impl/libs/tbb41_20130116oss/build -I././impl/libs/tbb41_20130116oss/build/vsproject -I././impl/libs/tbb41_20130116oss/examples -I././impl/libs/tbb41_20130116oss/examples/common -I././impl/libs/tbb41_20130116oss/examples/common/gui -I././impl/libs/tbb41_20130116oss/examples/common/gui/xcode -I././impl/libs/tbb41_20130116oss/examples/common/gui/xcode/tbbExample -I././impl/libs/tbb41_20130116oss/examples/common/gui/xcode/tbbExample/en.lproj -I././impl/libs/tbb41_20130116oss/examples/parallel_for -I././impl/libs/tbb41_20130116oss/examples/parallel_for/seismic -I././impl/libs/tbb41_20130116oss/examples/parallel_for/seismic/msvs -I././impl/libs/tbb41_20130116oss/examples/parallel_for/game_of_life -I././impl/libs/tbb41_20130116oss/examples/parallel_for/game_of_life/msvs -I././impl/libs/tbb41_20130116oss/examples/parallel_for/polygon_overlay -I././impl/libs/tbb41_20130116oss/examples/parallel_for/tachyon -I././impl/libs/tbb41_20130116oss/examples/parallel_for/tachyon/msvs -I././impl/libs/tbb41_20130116oss/examples/task_priority -I././impl/libs/tbb41_20130116oss/examples/task_priority/fractal -I././impl/libs/tbb41_20130116oss/examples/task_priority/fractal/msvs -I././impl/libs/tbb41_20130116oss/include -I././impl/libs/tbb41_20130116oss/include/serial -I././impl/libs/tbb41_20130116oss/include/serial/tbb -I././impl/libs/tbb41_20130116oss/include/tbb -I././impl/libs/tbb41_20130116oss/include/tbb/machine -I././impl/libs/tbb41_20130116oss/include/tbb/internal -I././impl/libs/tbb41_20130116oss/include/tbb/compat -I././impl/libs/tbb41_20130116oss/doc -I././impl/libs/tbb41_20130116oss/doc/html -I././impl/libs/tbb41_20130116oss/src -I././impl/libs/tbb41_20130116oss/src/tbbmalloc -I././impl/libs/tbb41_20130116oss/src/rml -I././impl/libs/tbb41_20130116oss/src/rml/server -I././impl/libs/tbb41_20130116oss/src/rml/client -I././impl/libs/tbb41_20130116oss/src/rml/include -I././impl/libs/tbb41_20130116oss/src/perf -I././impl/libs/tbb41_20130116oss/src/old -I././impl/libs/tbb41_20130116oss/src/tbb -I././impl/libs/tbb41_20130116oss/src/tbb/intel64-masm -I././impl/libs/tbb41_20130116oss/src/tbb/ia32-masm -I././impl/libs/tbb41_20130116oss/src/tbb/tools_api -I././impl/libs/tbb41_20130116oss/src/tbb/tools_api/prototype -I././impl/libs/tbb41_20130116oss/src/tbb/tools_api/legacy -I././impl/libs/tbb41_20130116oss/src/tbb/tools_api/internal -I././impl/libs/tbb41_20130116oss/src/tbb/ia64-gas -I././impl/libs/tbb41_20130116oss/src/tbbproxy -I././impl/libs/lockless_allocator -I././impl/src -I././impl/src/processors
CXXFLAGS=$(CFLAGS)
LDFLAGS=-lpthread

# The name of the library that will be built
LIBRARY=core

lib: $(IMPL_O)
	$(CXX) $(CXXFLAGS) -shared -pthread $(LDFLAGS) -o lib$(LIBRARY).so $(IMPL_O) $(USER_LIBS)

clean:
	rm -f lib$(LIBRARY).so
	find . -name '*.o' -print | xargs rm -f
