#include "query_worker_thread.h"
#include "query_index.h"
#include "utils/utils.h"
#include "thread_pool/query_thread_pool.h"
#include "query_index.h"
#include "exact_word_index.h"
#include "assert.h"

START_NAMESPACE

QueryWorkerThread::QueryWorkerThread() {
	wordReady = false;
}

void QueryWorkerThread::Initialize(QueryThreadPool* threadPool, int threadID) {
	this->threadPool = threadPool;
	this->threadID = threadID;
}

void QueryWorkerThread::Run() {
	while (true) {
		mutex.Lock();
		while (wordReady == false) {
			ready.Wait(mutex);
		}
		mutex.Unlock();

		switch (task) {
		case NEW_UQW_TASK:
			ProcessNewUniqueQueryWord();
			break;
		case NEW_EXACT_WORD_TASK:
			if (exactWord == NULL) {
				return;
			}
			ProcessNewExactWord();
			break;
		case UPDATE_DEPENDENCIES_TASK:
			UpdateDependencies();
			break;
		}

		wordReady = false;

		threadPool->Ready(this);
	}
}

void QueryWorkerThread::ProcessNewUniqueQueryWord() {
	unsigned int hashsetSize;
	if(matchType == MT_HAMMING_DIST) {
		//HAMMING_DIST

		ExactWordHashset& hashset = ExactWordIndex::words[uniqueQueryWordLength];
		hashsetSize = ExactWordIndex::sizes[uniqueQueryWordLength];
		switch (matchDist) {
		case 1:
			for(unsigned int i = 0u; i < hashsetSize; ++i) {
				ExactWord& word = hashset.warehouse[i];
				if(CheckHammingDistance_1(word.word.string, uniqueQueryWordLength, uniqueQueryWordText)) {
					word.dependencies->Insert(uniqueQueryWordID);
				}
			}
			break;
		case 2:
			for(unsigned int i = 0u; i < hashsetSize; ++i) {
				ExactWord& word = hashset.warehouse[i];
				if(CheckHammingDistance_2(word.word.string, uniqueQueryWordLength, uniqueQueryWordText)) {
					word.dependencies->Insert(uniqueQueryWordID);
				}
			}
			break;
		case 3:
			for(unsigned int i = 0u; i < hashsetSize; ++i) {
				ExactWord& word = hashset.warehouse[i];
				if(CheckHammingDistance_3(word.word.string, uniqueQueryWordLength, uniqueQueryWordText)) {
					word.dependencies->Insert(uniqueQueryWordID);
				}
			}
			break;
		}
	} else {
		if(matchType == MT_EXACT_MATCH) {
			//EXACT_MATCH

			ExactWordHashset& hashset = ExactWordIndex::words[uniqueQueryWordLength];
			int index = hashset.Find(uniqueQueryWordText, uniqueQueryWordLength, uniqueQueryWordHash);
			if(index != -1 && index < (int)ExactWordIndex::sizes[uniqueQueryWordLength]) {
				hashset.warehouse[index].dependencies->Insert(uniqueQueryWordID);
			}

		} else {
			//EDIT_DIST

			unsigned int exactWordLength;

			switch (matchDist) {
			case 1: {
				exactWordLength = uniqueQueryWordLength - 1;
				ExactWordHashset& hashset_0 = ExactWordIndex::words[exactWordLength];
				hashsetSize = ExactWordIndex::sizes[exactWordLength];
				for(unsigned int i = 0u; i < hashsetSize; ++i) {
					ExactWord& word = hashset_0.warehouse[i];
					if(CheckEditDistance_1_1(word.word.string, word.word.length, uniqueQueryWordText)) {
						word.dependencies->Insert(uniqueQueryWordID);
					}
				}
				exactWordLength = uniqueQueryWordLength;
				ExactWordHashset& hashset_1 = ExactWordIndex::words[exactWordLength];
				hashsetSize = ExactWordIndex::sizes[exactWordLength];
				for(unsigned int i = 0u; i < hashsetSize; ++i) {
					ExactWord& word = hashset_1.warehouse[i];
					if(CheckEditDistance_1_0(word.word.string, word.word.length, uniqueQueryWordText)) {
						word.dependencies->Insert(uniqueQueryWordID);
					}
				}
				exactWordLength = uniqueQueryWordLength + 1;
				ExactWordHashset& hashset_2 = ExactWordIndex::words[exactWordLength];
				hashsetSize = ExactWordIndex::sizes[exactWordLength];
				for(unsigned int i = 0u; i < hashsetSize; ++i) {
					ExactWord& word = hashset_2.warehouse[i];
					if(CheckEditDistance_1_1(uniqueQueryWordText, uniqueQueryWordLength, word.word.string)) {
						word.dependencies->Insert(uniqueQueryWordID);
					}
				}
			}
				break;
			case 2: {
				exactWordLength = uniqueQueryWordLength - 2;
				ExactWordHashset& hashset_0 = ExactWordIndex::words[exactWordLength];
				hashsetSize = ExactWordIndex::sizes[exactWordLength];
				for(unsigned int i = 0u; i < hashsetSize; ++i) {
					ExactWord& word = hashset_0.warehouse[i];
					if(CheckEditDistance_2_2(word.word.string, word.word.length, uniqueQueryWordText)) {
						word.dependencies->Insert(uniqueQueryWordID);
					}
				}
				exactWordLength = uniqueQueryWordLength - 1;
				ExactWordHashset& hashset_1 = ExactWordIndex::words[exactWordLength];
				hashsetSize = ExactWordIndex::sizes[exactWordLength];
				for(unsigned int i = 0u; i < hashsetSize; ++i) {
					ExactWord& word = hashset_1.warehouse[i];
					if(CheckEditDistance_2_1(word.word.string, word.word.length, uniqueQueryWordText)) {
						word.dependencies->Insert(uniqueQueryWordID);
					}
				}
				exactWordLength = uniqueQueryWordLength;
				ExactWordHashset& hashset_2 = ExactWordIndex::words[exactWordLength];
				hashsetSize = ExactWordIndex::sizes[exactWordLength];
				for(unsigned int i = 0u; i < hashsetSize; ++i) {
					ExactWord& word = hashset_2.warehouse[i];
					if(CheckEditDistance_2_0(word.word.string, word.word.length, uniqueQueryWordText)) {
						word.dependencies->Insert(uniqueQueryWordID);
					}
				}
				exactWordLength = uniqueQueryWordLength + 1;
				ExactWordHashset& hashset_3 = ExactWordIndex::words[exactWordLength];
				hashsetSize = ExactWordIndex::sizes[exactWordLength];
				for(unsigned int i = 0u; i < hashsetSize; ++i) {
					ExactWord& word = hashset_3.warehouse[i];
					if(CheckEditDistance_2_1(uniqueQueryWordText, uniqueQueryWordLength, word.word.string)) {
						word.dependencies->Insert(uniqueQueryWordID);
					}
				}
				exactWordLength = uniqueQueryWordLength + 2;
				ExactWordHashset& hashset_4 = ExactWordIndex::words[exactWordLength];
				hashsetSize = ExactWordIndex::sizes[exactWordLength];
				for(unsigned int i = 0u; i < hashsetSize; ++i) {
					ExactWord& word = hashset_4.warehouse[i];
					if(CheckEditDistance_2_2(uniqueQueryWordText, uniqueQueryWordLength, word.word.string)) {
						word.dependencies->Insert(uniqueQueryWordID);
					}
				}
			}
				break;
			case 3: {
				exactWordLength = uniqueQueryWordLength - 3;
				ExactWordHashset& hashset_0 = ExactWordIndex::words[exactWordLength];
				hashsetSize = ExactWordIndex::sizes[exactWordLength];
				for(unsigned int i = 0u; i < hashsetSize; ++i) {
					ExactWord& word = hashset_0.warehouse[i];
					if(CheckEditDistance_3_3(word.word.string, word.word.length, uniqueQueryWordText)) {
						word.dependencies->Insert(uniqueQueryWordID);
					}
				}
				exactWordLength = uniqueQueryWordLength - 2;
				ExactWordHashset& hashset_1 = ExactWordIndex::words[exactWordLength];
				hashsetSize = ExactWordIndex::sizes[exactWordLength];
				for(unsigned int i = 0u; i < hashsetSize; ++i) {
					ExactWord& word = hashset_1.warehouse[i];
					if(CheckEditDistance_3_2(word.word.string, word.word.length, uniqueQueryWordText)) {
						word.dependencies->Insert(uniqueQueryWordID);
					}
				}
				exactWordLength = uniqueQueryWordLength - 1;
				ExactWordHashset& hashset_2 = ExactWordIndex::words[exactWordLength];
				hashsetSize = ExactWordIndex::sizes[exactWordLength];
				for(unsigned int i = 0u; i < hashsetSize; ++i) {
					ExactWord& word = hashset_2.warehouse[i];
					if(CheckEditDistance_3_1(word.word.string, word.word.length, uniqueQueryWordText)) {
						word.dependencies->Insert(uniqueQueryWordID);
					}
				}
				exactWordLength = uniqueQueryWordLength;
				ExactWordHashset& hashset_3 = ExactWordIndex::words[exactWordLength];
				hashsetSize = ExactWordIndex::sizes[exactWordLength];
				for(unsigned int i = 0u; i < hashsetSize; ++i) {
					ExactWord& word = hashset_3.warehouse[i];
					if(CheckEditDistance_3_0(word.word.string, word.word.length, uniqueQueryWordText)) {
						word.dependencies->Insert(uniqueQueryWordID);
					}
				}
				exactWordLength = uniqueQueryWordLength + 1;
				ExactWordHashset& hashset_4 = ExactWordIndex::words[exactWordLength];
				hashsetSize = ExactWordIndex::sizes[exactWordLength];
				for(unsigned int i = 0u; i < hashsetSize; ++i) {
					ExactWord& word = hashset_4.warehouse[i];
					if(CheckEditDistance_3_1(uniqueQueryWordText, uniqueQueryWordLength, word.word.string)) {
						word.dependencies->Insert(uniqueQueryWordID);
					}
				}
				exactWordLength = uniqueQueryWordLength + 2;
				ExactWordHashset& hashset_5 = ExactWordIndex::words[exactWordLength];
				hashsetSize = ExactWordIndex::sizes[exactWordLength];
				for(unsigned int i = 0u; i < hashsetSize; ++i) {
					ExactWord& word = hashset_5.warehouse[i];
					if(CheckEditDistance_3_2(uniqueQueryWordText, uniqueQueryWordLength, word.word.string)) {
						word.dependencies->Insert(uniqueQueryWordID);
					}
				}
				exactWordLength = uniqueQueryWordLength + 3;
				ExactWordHashset& hashset_6 = ExactWordIndex::words[exactWordLength];
				hashsetSize = ExactWordIndex::sizes[exactWordLength];
				for(unsigned int i = 0u; i < hashsetSize; ++i) {
					ExactWord& word = hashset_6.warehouse[i];
					if(CheckEditDistance_3_3(uniqueQueryWordText, uniqueQueryWordLength, word.word.string)) {
						word.dependencies->Insert(uniqueQueryWordID);
					}
				}
			}
				break;
			}
		}
	}
}

void QueryWorkerThread::ProcessNewExactWord() {
	ExactWordIndex::ProcessNewExactWord(exactWord);
}

void QueryWorkerThread::UpdateDependencies() {
	for(unsigned int i = updatePackOffset; i < updatePackOffset + updatePackSize; ++i) {
		ConcurrentVector<UniqueQueryWordID>& dependencies = *(ExactWordIndex::words[updatePackWordLength].warehouse[i].dependencies);

		unsigned int k = 0;
		while(k + 1 < dependencies.size) {
			if(!QueryIndex::activeUniqueQueryWords[dependencies.warehouse[k]]) {
				dependencies.warehouse[k] = dependencies.warehouse[dependencies.size - 1];
				--dependencies.size;
			} else {
				++k;
			}
		}
		if(k + 1 == dependencies.size && !QueryIndex::activeUniqueQueryWords[dependencies.warehouse[dependencies.size - 1]]) {
			--dependencies.size;
		}

//			unsigned int k_2 = 0;
//			for(unsigned int k_1 = 0; k_1 < dependencies.size; ++k_1) {
//				if(QueryIndex::activeUniqueQueryWords[dependencies.warehouse[k_1]]) {
//					dependencies.warehouse[k_2] = dependencies.warehouse[k_1];
//					++k_2;
//				}
//			}
//			dependencies.size = k_2;

	}
}

void QueryWorkerThread::AddUniqueQueryWord(const char* uniqueQueryWordText, hash_type uniqueQueryWordHash, int uniqueQueryWordLength, UniqueQueryWordID uniqueQueryWordID, MatchType matchType, unsigned int matchDist) {
	mutex.Lock();
	this->task = NEW_UQW_TASK;
	memcpy(this->uniqueQueryWordText, uniqueQueryWordText, uniqueQueryWordLength);
	this->uniqueQueryWordText[uniqueQueryWordLength] = '\0';
	this->uniqueQueryWordHash   = uniqueQueryWordHash;
	this->uniqueQueryWordLength = uniqueQueryWordLength;
	this->uniqueQueryWordID     = uniqueQueryWordID;
	this->matchType = matchType;
	this->matchDist = matchDist;
	wordReady = true;
	ready.Signal();
	mutex.Unlock();
}

void QueryWorkerThread::AddExactWord(ExactWord* exactWord) {
	mutex.Lock();
	this->task = NEW_EXACT_WORD_TASK;
	this->exactWord = exactWord;
	wordReady = true;
	ready.Signal();
	mutex.Unlock();
}

void QueryWorkerThread::AddUpdatePart(unsigned int wordLength, unsigned int offset, unsigned int size) {
	mutex.Lock();
	this->task = UPDATE_DEPENDENCIES_TASK;
	this->updatePackWordLength = wordLength;
	this->updatePackOffset = offset;
	this->updatePackSize = size;
	wordReady = true;
	ready.Signal();
	mutex.Unlock();
}

END_NAMESPACE
