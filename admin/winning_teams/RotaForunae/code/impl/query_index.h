#pragma once

#include "config.h"
#include "structs.h"
#include "hash_structs/uqw_hashset.h"
#include "hash_structs/exact_word_hashset.h"
#include "thread_pool/query_thread_pool.h"
#include "thread_pool/thread_pools.h"

#include <stack>
#include <unordered_map>

START_NAMESPACE

class QueryIndex {
  public:
	static void AddQuery(const char* queryText, QueryID query_id, MatchType match_type, unsigned int match_dist);
	static void RemoveQuery(QueryID query_id);

	static UniqueQueryWord* AddWord(const char* str, hash_type hash, unsigned int length, MatchType& match_type, unsigned int& match_dist, unsigned int& resultIndex);

	static void ProcessExactWordQueue();

	static Query* Queries;
	static unsigned int last_query_index;

	static std::stack<unsigned int> emptyIDs;
	static std::unordered_map<unsigned int, unsigned int> positions;

	// 7 types
	static UQWHashset exactUQWSet;

	static UQWHashset hamming1UQWSet[HASHSET_SIZE];
	static UQWHashset hamming2UQWSet[HASHSET_SIZE];
	static UQWHashset hamming3UQWSet[HASHSET_SIZE];

	static UQWHashset edit1UQWSet[HASHSET_SIZE];
	static UQWHashset edit2UQWSet[HASHSET_SIZE];
	static UQWHashset edit3UQWSet[HASHSET_SIZE];

	static unsigned int QueriesAmount;
	static unsigned int IDCounter;
	static unsigned int QueryIDCounter;

	static Vector<bool> activeUniqueQueryWords;
};

END_NAMESPACE
