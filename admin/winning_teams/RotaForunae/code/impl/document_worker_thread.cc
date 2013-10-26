#include "document_worker_thread.h"
#include "thread_pool/document_thread_pool.h"
#include "utils/utils.h"
#include "query_index.h"
#include "exact_word_index.h"
#include "hash_structs/hashset.h"

#ifdef PRINT_STATISTICS
#include "utils/statistics.h"
#endif

#include <cstdio>
#include <stdlib.h>
#include <assert.h>

START_NAMESPACE

DocumentWorkerThread::DocumentWorkerThread() {
	documentReady = false;
}

void DocumentWorkerThread::Initialize(DocumentThreadPool* threadPool, int threadID) {
	this->threadPool = threadPool;
	this->threadID = threadID;
}

void DocumentWorkerThread::Run() {
	while (true) {
		mutex.Lock();
		while (documentReady == false) {
			ready.Wait(mutex);
		}
		mutex.Unlock();

		switch (task) {
		case PARSE_DOCUMENT_TASK:
			if (documentText == NULL) {
				return;
			}
			ParseDocument();
			break;
		case BUILD_RESULT_TASK:
			BuildResult();
			break;
		}

		documentReady = false;

		threadPool->Ready(this);
	}
}

void DocumentWorkerThread::ParseDocument() {
	Document* document = new Document();
	document->documentID = documentID;

	const char* cur = documentText;

	hashes.Init();

	while (cur) {
		const char* start = cur;

		hash_type hash = 0;

		while (*cur != ' ' && *cur != 0) {
			hash = hash * 31 + (*cur - 'a' + 1);
			cur++;
		}

		unsigned int length = cur - start;
		const char* string = start;

		if (hashes.Insert(hash, string, length)) {
			bool found = true;
			int exactWordIndex = ExactWordIndex::words[length].Insert(string, length, hash, found);
			if (!found) {
				ExactWordIndex::documentExactWordStack[length].Insert(exactWordIndex);
			}
			document->words[length].push_back(exactWordIndex);
		}

		if (*cur == ' ') {
			cur++;
		} else {
			cur = NULL;
		}
	}

	free(documentText);

	ExactWordIndex::documentStack.Insert(document);
}

void DocumentWorkerThread::BuildResult() {
	unsigned int n = QueryIndex::QueriesAmount;

	queryIDs.size = 0;

	bool matchingWord[QueryIndex::IDCounter];
	memset(matchingWord, false, QueryIndex::IDCounter * sizeof(bool));

	Document* document = ExactWordIndex::documentStack.warehouse[index];

	for (int length = MIN_WORD_LENGTH; length <= MAX_WORD_LENGTH; ++length) {
		const Vector<unsigned int>& documentWords = document->words[length];
		for (unsigned int i = 0; i != documentWords.size; ++i) {
			int index = documentWords[i];
			const ConcurrentVector<UniqueQueryWordID>* dependencies = ExactWordIndex::words[length].warehouse[index].dependencies;
			for (unsigned int i = 0; i < dependencies->size; ++i) {
				matchingWord[dependencies->warehouse[i]] = true;
			}
		}
	}

	for (unsigned int index = QueryIndex::Queries[0].next, i = 0; i < n; index = QueryIndex::Queries[index].next, i++) {
		bool matching_query = true;
		const Query& query = QueryIndex::Queries[index];

		unsigned int current_query_word_index = 0;
		while (matching_query && current_query_word_index < query.num_words) {
			UniqueQueryWordID wordID = query.wordIDs[current_query_word_index];

			matching_query = matchingWord[wordID];
			++current_query_word_index;
		}

		if (matching_query) {
			queryIDs.push_back(query.query_id);
		}
	}

	document->resultCount = queryIDs.size;
	if (queryIDs.size) {
		document->queryIDs = (unsigned int*)malloc(queryIDs.size * sizeof(unsigned int));
	}

	memcpy(document->queryIDs, queryIDs.warehouse, queryIDs.size * sizeof(unsigned int));
}

void DocumentWorkerThread::AddDocument(char* documentText, DocID documentID) {
	mutex.Lock();
	this->task = PARSE_DOCUMENT_TASK;
	this->documentText = documentText;
	this->documentID = documentID;
	documentReady = true;
	ready.Signal();
	mutex.Unlock();
}

void DocumentWorkerThread::AddIndex(const unsigned int documentIndex) {
	mutex.Lock();
	this->task = BUILD_RESULT_TASK;
	this->index = documentIndex;
	documentReady = true;
	ready.Signal();
	mutex.Unlock();
}

END_NAMESPACE
