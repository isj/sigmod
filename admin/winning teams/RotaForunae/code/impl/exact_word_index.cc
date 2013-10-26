#include "exact_word_index.h"
#include "query_index.h"
#include "utils/utils.h"

START_NAMESPACE

ExactWordHashset ExactWordIndex::words[HASHSET_SIZE];
unsigned int ExactWordIndex::sizes[HASHSET_SIZE];

ConcurrentVector<Document*> ExactWordIndex::documentStack;
ConcurrentVector<int> ExactWordIndex::documentExactWordStack[MAX_WORD_LENGTH + 1];

void ExactWordIndex::UpdateSizes() {
	for(unsigned int i = MIN_WORD_LENGTH; i <= MAX_WORD_LENGTH; ++i) {
		sizes[i] = words[i].Size();
	}
}

void ExactWordIndex::UpdateDependencies() {
	for(unsigned int length = MIN_WORD_LENGTH; length < MAX_WORD_LENGTH; ++length) {
		for(unsigned int offset = 0; offset < words[length].Size(); offset += UPDATE_PACK_SIZE) {
			ThreadPools::queryThreadPool.UpdateDependencies(length, offset, std::min(UPDATE_PACK_SIZE, words[length].Size() - offset));
		}
	}
}

void ExactWordIndex::Cleanup() {
	for(int i = 0; i < HASHSET_SIZE; ++i) {
		for (unsigned int j = 0; j < words[i].Size(); ++j) {
			delete words[i].warehouse[j].dependencies;
		}
	}
}

void ExactWordIndex::ProcessNewExactWord(ExactWord* exactWord) {
	unsigned int uqwLength;

	const char* exactString = exactWord->word.string;
	unsigned int exactLength = exactWord->word.length;

	ConcurrentVector<UniqueQueryWordID>& dependencies = *(exactWord->dependencies);

	UniqueQueryWord* uqw = QueryIndex::exactUQWSet.Find(exactWord->word);
	if (uqw != NULL) {
		dependencies.Insert(uqw->wordID);
	}

	//Hamming

	const UQWHashset& hamming1UQWSet = QueryIndex::hamming1UQWSet[exactLength];
	for (unsigned int i = 0; i < hamming1UQWSet.Size(); ++i) {
		const UniqueQueryWord& word = hamming1UQWSet.warehouse[i];
		if (word.cost != 0 && CheckHammingDistance_1(word.word.string, exactLength, exactString)) {
			dependencies.Insert(word.wordID);
		}
	}
	const UQWHashset& hamming2UQWSet = QueryIndex::hamming2UQWSet[exactLength];
	for (unsigned int i = 0; i < hamming2UQWSet.Size(); ++i) {
		const UniqueQueryWord& word = hamming2UQWSet.warehouse[i];
		if (word.cost != 0 && CheckHammingDistance_2(word.word.string, exactLength, exactString)) {
			dependencies.Insert(word.wordID);
		}
	}
	const UQWHashset& hamming3UQWSet = QueryIndex::hamming3UQWSet[exactLength];
	for(unsigned int i = 0; i < hamming3UQWSet.Size(); ++i) {
		const UniqueQueryWord& word = hamming3UQWSet.warehouse[i];
		if (word.cost != 0 && CheckHammingDistance_3(word.word.string, exactLength, exactString)) {
			dependencies.Insert(word.wordID);
		}
	}

	//Levenshtein

	//distance: 1
	uqwLength = exactLength - 1;
	const UQWHashset& edit1UQWSet_0 = QueryIndex::edit1UQWSet[uqwLength];
	for(unsigned int i = 0; i < edit1UQWSet_0.Size(); ++i) {
		const UniqueQueryWord& word = edit1UQWSet_0.warehouse[i];
		if (word.cost != 0 && CheckEditDistance_1_1(word.word.string, uqwLength, exactString)) {
			dependencies.Insert(word.wordID);
		}
	}
	uqwLength = exactLength;
	const UQWHashset& edit1UQWSet_1 = QueryIndex::edit1UQWSet[uqwLength];
	for(unsigned int i = 0; i < edit1UQWSet_1.Size(); ++i) {
		const UniqueQueryWord& word = edit1UQWSet_1.warehouse[i];
		if (word.cost != 0 && CheckEditDistance_1_0(word.word.string, exactLength, exactString)) {
			dependencies.Insert(word.wordID);
		}
	}
	uqwLength = exactLength + 1;
	const UQWHashset& edit1UQWSet_2 = QueryIndex::edit1UQWSet[uqwLength];
	for(unsigned int i = 0; i < edit1UQWSet_2.Size(); ++i) {
		const UniqueQueryWord& word = edit1UQWSet_2.warehouse[i];
		if (word.cost != 0 && CheckEditDistance_1_1(exactString, exactLength, word.word.string)) {
			dependencies.Insert(word.wordID);
		}
	}

	//distance: 2
	uqwLength = exactLength - 2;
	const UQWHashset& edit2UQWSet_0 = QueryIndex::edit2UQWSet[uqwLength];
	for(unsigned int i = 0; i < edit2UQWSet_0.Size(); ++i) {
		const UniqueQueryWord& word = edit2UQWSet_0.warehouse[i];
		if (word.cost != 0 && CheckEditDistance_2_2(word.word.string, uqwLength, exactString)) {
			dependencies.Insert(word.wordID);
		}
	}
	uqwLength = exactLength - 1;
	const UQWHashset& edit2UQWSet_1 = QueryIndex::edit2UQWSet[uqwLength];
	for(unsigned int i = 0; i < edit2UQWSet_1.Size(); ++i) {
		const UniqueQueryWord& word = edit2UQWSet_1.warehouse[i];
		if (word.cost != 0 && CheckEditDistance_2_1(word.word.string, uqwLength, exactString)) {
			dependencies.Insert(word.wordID);
		}
	}
	uqwLength = exactLength;
	const UQWHashset& edit2UQWSet_2 = QueryIndex::edit2UQWSet[uqwLength];
	for(unsigned int i = 0; i < edit2UQWSet_2.Size(); ++i) {
		const UniqueQueryWord& word = edit2UQWSet_2.warehouse[i];
		if (word.cost != 0 && CheckEditDistance_2_0(word.word.string, exactLength, exactString)) {
			dependencies.Insert(word.wordID);
		}
	}
	uqwLength = exactLength + 1;
	const UQWHashset& edit2UQWSet_3 = QueryIndex::edit2UQWSet[uqwLength];
	for(unsigned int i = 0; i < edit2UQWSet_3.Size(); ++i) {
		const UniqueQueryWord& word = edit2UQWSet_3.warehouse[i];
		if (word.cost != 0 && CheckEditDistance_2_1(exactString, exactLength, word.word.string)) {
			dependencies.Insert(word.wordID);
		}
	}
	uqwLength = exactLength + 2;
	const UQWHashset& edit2UQWSet_4 = QueryIndex::edit2UQWSet[uqwLength];
	for(unsigned int i = 0; i < edit2UQWSet_4.Size(); ++i) {
		const UniqueQueryWord& word = edit2UQWSet_4.warehouse[i];
		if (word.cost != 0 && CheckEditDistance_2_2(exactString, exactLength, word.word.string)) {
			dependencies.Insert(word.wordID);
		}
	}

	//distance: 3
	uqwLength = exactLength - 3;
	const UQWHashset& edit3UQWSet_0 = QueryIndex::edit3UQWSet[uqwLength];
	for(unsigned int i = 0; i < edit3UQWSet_0.Size(); ++i) {
		const UniqueQueryWord& word = edit3UQWSet_0.warehouse[i];
		if (word.cost != 0 && CheckEditDistance_3_3(word.word.string, uqwLength, exactString)) {
			dependencies.Insert(word.wordID);
		}
	}
	uqwLength = exactLength - 2;
	const UQWHashset& edit3UQWSet_1 = QueryIndex::edit3UQWSet[uqwLength];
	for(unsigned int i = 0; i < edit3UQWSet_1.Size(); ++i) {
		const UniqueQueryWord& word = edit3UQWSet_1.warehouse[i];
		if (word.cost != 0 && CheckEditDistance_3_2(word.word.string, uqwLength, exactString)) {
			dependencies.Insert(word.wordID);
		}
	}
	uqwLength = exactLength - 1;
	const UQWHashset& edit3UQWSet_2 = QueryIndex::edit3UQWSet[uqwLength];
	for(unsigned int i = 0; i < edit3UQWSet_2.Size(); ++i) {
		const UniqueQueryWord& word = edit3UQWSet_2.warehouse[i];
		if (word.cost != 0 && CheckEditDistance_3_1(word.word.string, uqwLength, exactString)) {
			dependencies.Insert(word.wordID);
		}
	}
	uqwLength = exactLength;
	const UQWHashset& edit3UQWSet_3 = QueryIndex::edit3UQWSet[uqwLength];
	for(unsigned int i = 0; i < edit3UQWSet_3.Size(); ++i) {
		const UniqueQueryWord& word = edit3UQWSet_3.warehouse[i];
		if (word.cost != 0 && CheckEditDistance_3_0(exactString, uqwLength, word.word.string)) {
			dependencies.Insert(word.wordID);
		}
	}
	uqwLength = exactLength + 1;
	const UQWHashset& edit3UQWSet_4 = QueryIndex::edit3UQWSet[uqwLength];
	for(unsigned int i = 0; i < edit3UQWSet_4.Size(); ++i) {
		const UniqueQueryWord& word = edit3UQWSet_4.warehouse[i];
		if (word.cost != 0 && CheckEditDistance_3_1(exactString, exactLength, word.word.string)) {
			dependencies.Insert(word.wordID);
		}
	}
	uqwLength = exactLength + 2;
	const UQWHashset& edit3UQWSet_5 = QueryIndex::edit3UQWSet[uqwLength];
	for(unsigned int i = 0; i < edit3UQWSet_5.Size(); ++i) {
		const UniqueQueryWord& word = edit3UQWSet_5.warehouse[i];
		if (word.cost != 0 && CheckEditDistance_3_2(exactString, exactLength, word.word.string)) {
			dependencies.Insert(word.wordID);
		}
	}
	uqwLength = exactLength + 3;
	const UQWHashset& edit3UQWSet_6 = QueryIndex::edit3UQWSet[uqwLength];
	for(unsigned int i = 0; i < edit3UQWSet_6.Size(); ++i) {
		const UniqueQueryWord& word = edit3UQWSet_6.warehouse[i];
		if (word.cost != 0 && CheckEditDistance_3_3(exactString, exactLength, word.word.string)) {
			dependencies.Insert(word.wordID);
		}
	}
}

END_NAMESPACE
