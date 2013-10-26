#pragma once

#include "config.h"
#include "thread_pool/mutex.h"
#include "structs.h"
#include "utils/vector.h"
#include "utils/statistics.h"
#include "../thread_pool/thread_pools.h"

#include <cassert>
#include <cstdio>

START_NAMESPACE

class ExactWordHashset {
  public:
	Vector<ExactWord> warehouse;
	Vector<int> next;

	DEBUG_NOINLINE ExactWordHashset() {

	}

	DEBUG_NOINLINE void Init() {
		table_size = INITIAL_EXACT_WORD_HASHSET_SIZE;

		table = (int*)malloc(table_size * sizeof(int));
		memset(table, -1, table_size * sizeof(int));

		warehouse.reserve(INITIAL_EXACT_WORD_HASHSET_SIZE);
		next.reserve(INITIAL_EXACT_WORD_HASHSET_SIZE);
#ifdef PRINT_STATISTICS
		bucketCounter = 0;
#endif
	}

	DEBUG_NOINLINE ~ExactWordHashset() {
		for (unsigned int i = 0; i < warehouse.size; ++i) {
			delete warehouse[i].dependencies;
		}
		free(table);
#ifdef PRINT_STATISTICS
		printf("exact: %f\n", (double)Size() / bucketCounter);
#endif
	}

	DEBUG_NOINLINE int Insert(const char* string, unsigned int length, unsigned int hash, bool& found) {
		mutex.Lock();

		if (Size() >= table_size) {
			ThreadPools::queryThreadPool.WaitAll();
			Resize(Size() * 2);
		}

		unsigned int index = hash % table_size;

		if (table[index] == -1) {
#ifdef PRINT_STATISTICS
			bucketCounter++;
#endif
			int pos = Size();

			int exactWordIndex = warehouse.push_back_return_index();
			next.push_back(-1);

			found = false;

			Word& word = warehouse[exactWordIndex].word;

			memcpy(word.string, string, length);
			word.string[length] = 0;
			word.length = length;
			word.hash = hash;

			warehouse[exactWordIndex].dependencies = new ConcurrentVector<UniqueQueryWordID>();

			table[index] = pos;

			mutex.Unlock();
			return exactWordIndex;
		}

		int cur = table[index];
		int prev;
		while (cur != -1) {
			Word& word = warehouse[cur].word;
			if (word.hash == hash &&
				memcmp(word.string, string, length) == 0) {

				found = true;
				mutex.Unlock();
				return cur;
			}
			prev = cur;
			cur = next[cur];
		}

		index = Size();
		int exactWordIndex = warehouse.push_back_return_index();
		next.push_back(-1);

		found = false;

		Word& word = warehouse[exactWordIndex].word;

		memcpy(word.string, string, length);
		word.string[length] = 0;
		word.length = length;
		word.hash = hash;

		warehouse[exactWordIndex].dependencies = new ConcurrentVector<UniqueQueryWordID>();
		next.warehouse[prev] = index;

		mutex.Unlock();
		return exactWordIndex;
	}

	DEBUG_NOINLINE int Find(const char* string, unsigned int length, unsigned int hash) {
		mutex.Lock();
		int index = hash % table_size;
		int cur = table[index];
		while (cur != -1) {
			Word& word = warehouse[cur].word;
			if (word.hash == hash &&
				memcmp(word.string, string, length) == 0) {
				mutex.Unlock();
				return cur;
			}
			cur = next.warehouse[cur];
		}
		mutex.Unlock();
		return -1;
	}

	DEBUG_NOINLINE unsigned int Size() const {
		return warehouse.size;
	}

private:
	int* table;
	unsigned int table_size;
	Mutex mutex;
	Mutex readMutex;
#ifdef PRINT_STATISTICS
	unsigned int bucketCounter;
#endif

	DEBUG_NOINLINE void Resize(unsigned int new_size) {
#ifdef PRINT_STATISTICS
		bucketCounter = 0;
#endif
		int i = 0;
		while (PRIME_LIST[i] < new_size) {
			++i;
		}
		table_size = PRIME_LIST[i];
		warehouse.Resize(table_size);
		next.Resize(table_size);
		free(table);
		table = (int*)malloc(table_size * sizeof(int));
		memset(table, -1, table_size * sizeof(int));

		for (unsigned int i = 0; i < Size(); ++i) {
			unsigned int index = warehouse[i].word.hash % table_size;
#ifdef PRINT_STATISTICS
			if (table[index] == -1) {
				++bucketCounter;
			}
#endif
			next[i] = table[index];
			table[index] = i;
		}
	}
};

END_NAMESPACE
