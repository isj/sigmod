#pragma once

#include "config.h"
#include "structs.h"
#include "config.h"
#include "utils/vector.h"

#include <cstdio>

START_NAMESPACE

struct HashsetElement {
	hash_type hash;
	const char* str;
	int length;
};

class Hashset {
	public:
	Vector<HashsetElement> warehouse;
	Vector<int> next;

	unsigned int Size() const { return warehouse.size; }

	DEBUG_NOINLINE Hashset() {
		table_size = INITIAL_HASHSET_TABLE_SIZE;
		table = (int*)malloc(table_size * sizeof(int));
		warehouse.reserve(INITIAL_HASHSET_WAREHOUSE_SIZE);
		next.reserve(INITIAL_HASHSET_WAREHOUSE_SIZE);
	}

	DEBUG_NOINLINE ~Hashset() {
#ifdef PRINT_STATISTICS
		printf("hash_type hashset: %f\n", (double)size / bucketCounter);
#endif
		free(table);
	}

	DEBUG_NOINLINE void Init() {
		memset(table, -1, table_size * sizeof(int));
		next.size = 0;
		warehouse.size = 0;

#ifdef PRINT_STATISTICS
		bucketCounter = 0;
#endif
	}

	// true if element was not in the hashset
	DEBUG_NOINLINE bool Insert(hash_type key, const char* str, int length) {
		if (Size() >= table_size) {
			Resize(Size() * 2);
		}

		unsigned int index = key % table_size;
		if (table[index] == -1) {
#ifdef PRINT_STATISTICS
		++bucketCounter;
#endif
			table[index] = Size();

			HashsetElement& new_el = warehouse.push_back();
			new_el.hash = key;
			new_el.str = str;
			new_el.length = length;
			next.push_back(-1);

			return true;
		}

		int prev;
		int cur = table[index];
		while (cur != -1) {
			if (warehouse[cur].hash == key && 
				warehouse[cur].length == length &&
				memcmp(warehouse[cur].str, str, length) == 0) {
				return false;
			}
			prev = cur;
			cur = next[cur];
		}

		next[prev] = Size();

		HashsetElement& new_el = warehouse.push_back();
		new_el.hash = key;
		new_el.str = str;
		new_el.length = length;
		next.push_back(-1);

		return true;
	}

  private:
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
			unsigned int index = warehouse[i].hash % table_size;
#ifdef PRINT_STATISTICS
			if (table[index] == -1) {
				++bucketCounter;
			}
#endif
			next[i] = table[index];
			table[index] = i;
		}
	}

	int* table;
	unsigned int table_size;
#ifdef PRINT_STATISTICS
	unsigned int bucketCounter;
#endif
};

END_NAMESPACE
