#include "core.h"

#include "utils/utils.h"
#include "exact_word_index.h"
#include "query_index.h"
#include "structs.h"
#include "thread_pool/document_thread_pool.h"
#include "thread_pool/thread_pools.h"

#include <cstdlib>
#include <cstdio>

#ifdef PRINT_STATISTICS
#include "utils/statistics.h"
#endif

#include "utils/automata.h"

using namespace NAMESPACE;

QueryThreadPool ThreadPools::queryThreadPool;
DocumentThreadPool ThreadPools::documentThreadPool;

enum Stage {
	QUERIES_STAGE,
	DOCUMENTS_STAGE,
	RESULTS_STAGE,
	UNDEFINED
};

Stage currentStage;

unsigned int batchID = 0;
unsigned int availibleResultCount = 0;

void StartQueriesStage() {
	ThreadPools::queryThreadPool.WaitAll();
	ExactWordIndex::UpdateSizes();
}

void FinishQueriesStage() {
	++batchID;
#ifdef USE_DEPENDENICES_UPDATE
	if(batchID % UPDATE_PERIOD == 0) {
//		ExactWordIndex::UpdateDependencies();
//		QueryIndex::queryThreadPool.WaitAll();
	}
#endif
}

void StartDocumentsStage() {
}

void FinishDocumentsStage() {
	ThreadPools::documentThreadPool.WaitAll();
	for (int length = MIN_WORD_LENGTH; length < MAX_WORD_LENGTH + 1; ++length) {
		ConcurrentVector<int>& stack = ExactWordIndex::documentExactWordStack[length];
		for(unsigned int i = 0; i < stack.size; ++i) {
			ThreadPools::queryThreadPool.ProcessNewExactWord(&(ExactWordIndex::words[length].warehouse[stack.warehouse[i]]));
		}
		stack.size = 0;
	}
	ThreadPools::queryThreadPool.WaitAll();

	unsigned int size = ExactWordIndex::documentStack.size;
	for(unsigned int i = availibleResultCount; i < size; ++i) {
		ThreadPools::documentThreadPool.BuildResult(i);
	}
	ThreadPools::documentThreadPool.WaitAll();
	availibleResultCount = size;
}

void StartResultsStage() {
}

void FinishResultsStage() {
}

void UpdateStage(Stage stage) {
	if (currentStage != stage) {
		switch (currentStage) {
		case QUERIES_STAGE:
			FinishQueriesStage();
			break;
		case DOCUMENTS_STAGE:
			FinishDocumentsStage();
			break;
		case RESULTS_STAGE:
			FinishResultsStage();
			break;
		case UNDEFINED:
			break;
		}

		currentStage = stage;

		switch (currentStage) {
		case QUERIES_STAGE:
			StartQueriesStage();
			break;
		case DOCUMENTS_STAGE:
			StartDocumentsStage();
			break;
		case RESULTS_STAGE:
			StartResultsStage();
			break;
		case UNDEFINED:
			break;
		}

	}
}

ErrorCode InitializeIndex() {
	ThreadPools::queryThreadPool.Initialize();

	//TODO: Free
	QueryIndex::Queries = (Query*)malloc(MAX_ACTIVE_QUERY_AMOUNT * sizeof(Query));

	//TODO: Free
	ThreadPools::documentThreadPool.Initialize();

	QueryIndex::exactUQWSet.Init();

	for (int i = MIN_WORD_LENGTH; i <= MAX_WORD_LENGTH; ++i) {
		ExactWordIndex::words[i].Init();

		QueryIndex::hamming1UQWSet[i].Init();
		QueryIndex::hamming2UQWSet[i].Init();
		QueryIndex::hamming3UQWSet[i].Init();

		QueryIndex::edit1UQWSet[i].Init();
		QueryIndex::edit2UQWSet[i].Init();
		QueryIndex::edit3UQWSet[i].Init();
	}

	currentStage = UNDEFINED;

	return EC_SUCCESS;
}

ErrorCode DestroyIndex() {
	UpdateStage(UNDEFINED);

	ThreadPools::documentThreadPool.JoinAll();
	ThreadPools::queryThreadPool.JoinAll();

#ifdef PRINT_DEPENDENCIES_STATE
	for(unsigned int i = MIN_WORD_LENGTH; i <= MAX_WORD_LENGTH; ++i) {
		unsigned int a = 0;
		unsigned int b = 0;
		unsigned int c = 0;
		for(unsigned int j = 0; j < ExactWordIndex::words[i].Size(); ++j) {
			ConcurrentVector<UniqueQueryWordID>* dependencies = ExactWordIndex::words[i].warehouse[j].dependencies;
			a += dependencies->size;
			if(dependencies->size == 0) {
				++c;
			}
			++b;
		}
		printf("%u: %f [%u,%u]\n", i,  ((float)a) / ((float)b), c, b);
	}
	printf("\n");
#endif

#ifdef PRINT_STATISTICS
	Statistics::Print();
#endif

#ifdef PRINT_EXACT_WORD_HASHSET_STATE
	printf("FINISH!\n");
	for (int len = MIN_WORD_LENGTH; len <= MAX_WORD_LENGTH; ++len) {
		Hashset check;
		check.Init();

		printf("LEN = %d\n", len);
		printf("SIZE = %d\n", ExactWordIndex::words[len].Size());

		for (unsigned int i = 0; i < ExactWordIndex::words[len].Size(); ++i) {
			Word& word = ExactWordIndex::words[len].warehouse[i].word;

			check.Insert(word.hash);
		}
		printf("ACTUAL SIZE = %d\n\n", check.Size());
	}

#endif
	return EC_SUCCESS;
}

ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist) {
	UpdateStage(QUERIES_STAGE);

	// Add this query to the active query set
	QueryIndex::AddQuery(query_str, query_id, match_type, match_dist);

	return EC_SUCCESS;
}

ErrorCode EndQuery(QueryID query_id) {
	UpdateStage(QUERIES_STAGE);

	// Remove this query from the active query set
	QueryIndex::RemoveQuery(query_id);

	return EC_SUCCESS;
}

ErrorCode MatchDocument(DocID doc_id, const char* doc_str) {
	UpdateStage(DOCUMENTS_STAGE);

	char* documentText = (char*)malloc((strlen(doc_str) + 1) * sizeof(char));
	strcpy(documentText, doc_str);

	ThreadPools::documentThreadPool.ParseDocument(documentText, doc_id);

	return EC_SUCCESS;
}

ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids) {
	UpdateStage(RESULTS_STAGE);

	if(availibleResultCount == 0) {
		return EC_NO_AVAIL_RES;
	}

	ConcurrentVector<Document*>& stack = ExactWordIndex::documentStack;
	Document* document = stack.warehouse[availibleResultCount - 1];
	if(availibleResultCount != stack.size) {
		stack.warehouse[availibleResultCount - 1] = stack.warehouse[stack.size - 1];
	}
	--stack.size;
	--availibleResultCount;

	*p_doc_id = document->documentID;
	*p_num_res = document->resultCount;
	*p_query_ids = document->queryIDs;

	delete document;

	return EC_SUCCESS;
}
