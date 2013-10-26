#pragma once

#include "config.h"
#include "structs.h"
#include "utils/vector.h"
#include <cstdio>

START_NAMESPACE

class UQWHashset {
  public:
	Vector<UniqueQueryWord> warehouse;
	Vector<int> next;

	unsigned int Size() const { return warehouse.size; }

	DEBUG_NOINLINE UQWHashset() {
	}

	DEBUG_NOINLINE void Init() {
		table_size = INITIAL_UQW_HASHSET_TABLE_SIZE;
		table = (int*)malloc(table_size * sizeof(int));
		memset(table, -1, table_size * sizeof(int));

		warehouse.reserve(INITIAL_UQW_HASHSET_WAREHOUSE_SIZE);
		next.reserve(INITIAL_UQW_HASHSET_WAREHOUSE_SIZE);
#ifdef PRINT_STATISTICS
		bucketCounter = 0;
#endif
	}

	DEBUG_NOINLINE ~UQWHashset() {
#ifdef PRINT_STATISTICS
		printf("uqw hashset: %f\n", (double)size / bucketCounter);
#endif
		free(table);
	}

	DEBUG_NOINLINE UniqueQueryWord* Insert(const char* str, const unsigned int hash, unsigned int length,  bool& found, unsigned int& resultIndex) {
		if (Size() >= table_size) {
			Resize(Size() * 2);
		}

		unsigned int index = hash % table_size;
		if (table[index] == -1) {
#ifdef PRINT_STATISTICS
		++bucketCounter;
#endif
			table[index] = Size();
			
			resultIndex = Size();
			UniqueQueryWord& uqw = warehouse.push_back();
			next.push_back(-1);
			return &uqw;
		}

		int cur = table[index];
		int prev;
		while (cur != -1) {
			if (hash == warehouse[cur].word.hash &&
				memcmp(str, warehouse[cur].word.string, length) == 0) {
				
				found = true;
				resultIndex = cur;
				return &warehouse[cur];
			}
			prev = cur;
			cur = next[cur];
		}

		next[prev] = Size();
		resultIndex = Size();
		UniqueQueryWord& uqw = warehouse.push_back();
		next.push_back(-1);
		return &uqw;
	}

	DEBUG_NOINLINE UniqueQueryWord* Find(const Word& word) {
		int index = word.hash % table_size;
		int cur = table[index];
		while (cur != -1) {
			if (warehouse[cur].word == word) {
				return &warehouse[cur];
			}
			cur = next[cur];
		}
		return NULL;
	}

	private:
	int* table;
	unsigned int table_size;
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
