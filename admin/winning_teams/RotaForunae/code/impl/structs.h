#pragma once

#include "core.h"
#include "config.h"

#include <string.h>

#include "utils/concurrent_vector.h"
#include <utils/vector.h>

START_NAMESPACE

struct Word {
	char string[MAX_WORD_LENGTH + 1];
	unsigned int length;
	hash_type hash;

#ifdef USE_BLOOM_FILTER
	uint64_t bloomFilter;
#endif

	bool operator==(const Word& word) const {
		return hash == word.hash && length == word.length && memcmp(word.string, this->string, word.length) == 0;
	}
};

struct ExactWord {
	Word word;
	ConcurrentVector<UniqueQueryWordID>* dependencies;
};

struct UniqueQueryWord {
	Word word;
	unsigned int wordID;
	unsigned int cost;
	// TODO pointer!
#ifdef USE_LEVENSHTEIN_MASKS
	unsigned int masks[5][MAX_WORD_LENGTH + 1];
#endif
};

// Keeps all information related to an active query
struct Query {
	unsigned int wordIDs[MAX_QUERY_WORDS];
	unsigned int words[MAX_QUERY_WORDS];
	unsigned int words_length[MAX_QUERY_WORDS];

	unsigned int num_words;

	QueryID query_id;

	MatchType match_type;
	unsigned int match_dist;

	unsigned int prev;
	unsigned int next;
};

struct Document {
	DocID documentID;
	Vector<unsigned int> words[MAX_WORD_LENGTH + 1];
	unsigned int resultCount;
	QueryID* queryIDs;
};

END_NAMESPACE
