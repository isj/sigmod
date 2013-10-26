#include "config.h"
#include "query_index.h"
#include "exact_word_index.h"

#include <cstdlib>
#include <cstdio>
#include <cassert>

#include <stdint.h>

START_NAMESPACE

UQWHashset QueryIndex::exactUQWSet;

UQWHashset QueryIndex::hamming1UQWSet[HASHSET_SIZE];
UQWHashset QueryIndex::hamming2UQWSet[HASHSET_SIZE];
UQWHashset QueryIndex::hamming3UQWSet[HASHSET_SIZE];

UQWHashset QueryIndex::edit1UQWSet[HASHSET_SIZE];
UQWHashset QueryIndex::edit2UQWSet[HASHSET_SIZE];
UQWHashset QueryIndex::edit3UQWSet[HASHSET_SIZE];

Query* QueryIndex::Queries;

std::stack<unsigned int> QueryIndex::emptyIDs;
std::unordered_map<unsigned int, unsigned int> QueryIndex::positions;

unsigned int QueryIndex::QueriesAmount = 0;
unsigned int QueryIndex::IDCounter = 0;
unsigned int QueryIndex::QueryIDCounter = 1;
unsigned int QueryIndex::last_query_index = 0;

Vector<bool> QueryIndex::activeUniqueQueryWords;

void QueryIndex::AddQuery(const char* queryText, QueryID query_id, MatchType match_type, unsigned int match_dist) {
	QueriesAmount++;

	unsigned int index;
	if (!emptyIDs.empty()) {
		index = emptyIDs.top();
		emptyIDs.pop();
	} else {
		index = QueryIDCounter++;
	}

	positions[query_id] = index;
	Query& query = Queries[index];

	query.match_type = match_type;
	query.match_dist = match_dist;
	query.query_id = query_id;

	query.prev = last_query_index;
	Queries[last_query_index].next = index;
	last_query_index = index;

	const char* cur = queryText;
	int iq = 0;

	while (*cur) {
		const char* start = cur;

		hash_type hash = 0;
		while (*cur != ' ' && *cur != 0) {
			hash = hash * 31 + (*cur - 'a' + 1);
			cur++;
		}

		unsigned int length = cur - start;

		unsigned int uqwIndex;
		UniqueQueryWord* uqw = AddWord(start, hash, length, match_type, match_dist, uqwIndex);
		query.words[iq] = uqwIndex;
		query.words_length[iq] = length;
		query.wordIDs[iq] = uqw->wordID;

		if (*cur == ' ') {
			++cur;
		}

		++iq;
	}

	query.num_words = iq;
}

UniqueQueryWord* QueryIndex::AddWord(const char* str, unsigned int hash, unsigned int length, MatchType& match_type, unsigned int& match_dist, unsigned int& uqwIndex) {
	UniqueQueryWord* uqw = NULL;

	bool found = false;
	switch (match_type) {
	case MT_EXACT_MATCH:
		uqw = exactUQWSet.Insert(str, hash, length, found, uqwIndex);
		break;
	case MT_HAMMING_DIST:
		switch(match_dist) {
		case 1:
			uqw = hamming1UQWSet[length].Insert(str, hash, length, found, uqwIndex);
			break;
		case 2:
			uqw = hamming2UQWSet[length].Insert(str, hash, length, found, uqwIndex);
			break;
		case 3:
			uqw = hamming3UQWSet[length].Insert(str, hash, length, found, uqwIndex);
			break;
		}
		break;
	case MT_EDIT_DIST:
		switch(match_dist) {
		case 1:
			uqw = edit1UQWSet[length].Insert(str, hash, length, found, uqwIndex);
			break;
		case 2:
			uqw = edit2UQWSet[length].Insert(str, hash, length, found, uqwIndex);
			break;
		case 3:
			uqw = edit3UQWSet[length].Insert(str, hash, length, found, uqwIndex);
			break;
		}
		break;
	}

	if (!found) {
		int ID = IDCounter++;

#ifdef USE_DEPENDENICES_UPDATE
		activeUniqueQueryWords.push_back(true);
#endif

		Word& word = uqw->word;
		memcpy(word.string, str, length);
		word.string[length] = '\0';
		word.length = length;
		word.hash = hash;

		uqw->wordID = ID;
		uqw->cost = 1;

#ifdef USE_BLOOM_FILTER
		word.bloomFilter = 0ULL;
		for (int i = 0; i < word.length - 1; ++i) {
			unsigned int bIndex = ((word.word[i] - 'a') * 31 + (word.word[i + 1] - 'a')) & 63;
			word.bloomFilter |= (1ULL << bIndex);
		}

		word.bloomFilter = ~word.bloomFilter;
#endif

#ifdef USE_LEVENSHTEIN_MASKS
		if(match_type == MT_EDIT_DIST && match_dist == 3) {

			int length;
			int query_word_length = (int)word.length;
			memset(uqw->masks, 0, sizeof(uqw->masks[0][0]) * 5 * (MAX_WORD_LENGTH + 1));

			length = query_word_length - 2;
			if(length >= MIN_WORD_LENGTH) {
				for(int i = 0; i < length; i++) {
					int l = std::max(i, 0);
					int r = std::min(i + 2, query_word_length- 1);
					for(int j = l; j <= r; j++) {
						uqw->masks[0][i] |= (1 << (word.string[j] - 'a'));
					}
				}
			}

			length = query_word_length - 1;
			if(length >= MIN_WORD_LENGTH) {
				for(int i = 0; i < length; i++) {
					int l = std::max(i - 1, 0);
					int r = std::min(i + 2, query_word_length - 1);
					for(int j = l; j <= r; j++) {
						uqw->masks[1][i] |= (1 << (word.string[j] - 'a'));
					}
				}
			}

			length = word.length;
			for(int i = 0; i < length; i++) {
				int l = std::max(i - 1, 0);
				int r = std::min(i + 1, query_word_length - 1);
				for(int j = l; j <= r; j++) {
					uqw->masks[2][i] |= (1 << (word.string[j] - 'a'));
				}
			}

			length = query_word_length + 1;
			if(length <= MAX_WORD_LENGTH) {
				for(int i = 0; i < length; i++) {
					int l = std::max(i - 2, 0);
					int r = std::min(i + 1, query_word_length - 1);
					for(int j = l; j <= r; j++) {
						uqw->masks[3][i] |= (1 << (word.string[j] - 'a'));
					}
				}
			}

			length = query_word_length + 2;
			if(length <= MAX_WORD_LENGTH) {
				for(int i = 0; i < length; i++) {
					int l = std::max(i - 2, 0);
					int r = std::min(i, query_word_length - 1);
					for(int j = l; j <= r; j++) {
						uqw->masks[4][i] |= (1 << (word.string[j] - 'a'));
					}
				}
			}
		}
#endif
		ThreadPools::queryThreadPool.ProcessNewUniqueQueryWord(str, hash, length, ID, match_type, match_dist);
	} else {
		uqw->cost++;
#ifdef USE_DEPENDENICES_UPDATE
		activeUniqueQueryWords[uqw->wordID] = true;
#endif
		if (uqw->cost == 1) {
			ThreadPools::queryThreadPool.ProcessNewUniqueQueryWord(str, hash, length, uqw->wordID, match_type, match_dist);
		}
	}
	return uqw;
}

void QueryIndex::RemoveQuery(QueryID query_id) {
	unsigned int index = positions[query_id];

	if (index == last_query_index) {
		last_query_index = Queries[index].prev;
	} else {
		unsigned int next = Queries[index].next;
		unsigned int prev = Queries[index].prev;
		Queries[next].prev = prev;
		Queries[prev].next = next;
	}

	emptyIDs.push(index);
	positions.erase(query_id);

	QueriesAmount--;

	Query& query = Queries[index];

	if (query.match_type == MT_EXACT_MATCH) {
		Vector<UniqueQueryWord>& exact_words = exactUQWSet.warehouse;
		for (unsigned int i = 0; i < query.num_words; ++i) {
			exact_words[query.words[i]].cost--;
#ifdef USE_DEPENDENICES_UPDATE
			if(exact_words[query.words[i]].cost == 0) {
				activeUniqueQueryWords[exact_words[query.words[i]].wordID] = false;
			}
#endif
		}
	} else {
		UQWHashset (*unique_query_words_set)[HASHSET_SIZE] = NULL;
		if (query.match_type == MT_HAMMING_DIST) {
			switch(query.match_dist) {
			case 1:
				unique_query_words_set = &hamming1UQWSet;
				break;
			case 2:
				unique_query_words_set = &hamming2UQWSet;
				break;
			case 3:
				unique_query_words_set = &hamming3UQWSet;
				break;
			}
		} else {
			switch(query.match_dist) {
			case 1:
				unique_query_words_set = &edit1UQWSet;
				break;
			case 2:
				unique_query_words_set = &edit2UQWSet;
				break;
			case 3:
				unique_query_words_set = &edit3UQWSet;
				break;
			}
		}
		for (unsigned int i = 0; i < query.num_words; ++i) {
			UniqueQueryWord& uqw = (*unique_query_words_set)[query.words_length[i]].warehouse[query.words[i]];
			uqw.cost--;
#ifdef USE_DEPENDENICES_UPDATE
			if(uqw.cost == 0) {
				activeUniqueQueryWords[uqw.wordID] = false;
			}
#endif
		}
	}
}

END_NAMESPACE
