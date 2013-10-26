#pragma once

#include "config.h"
#include "structs.h"
#include "query_worker_thread.h"

START_NAMESPACE

class QueryThreadPool {
	friend class QueryWorkerThread;

private:
	QueryWorkerThread threads[QUERY_POOL_THREAD_COUNT];

	QueryWorkerThread* stack[QUERY_POOL_THREAD_COUNT];
	volatile int stackSize;

	Mutex mutex;
	Condition ready;

	Condition doneCondition;

	void Ready(QueryWorkerThread* thread);

public:
	QueryThreadPool();
	~QueryThreadPool();

	void Initialize();

	void ProcessNewUniqueQueryWord(const char*       uniqueQueryWordText,
								   hash_type         uniqueQueryWordHash,
								   int               uniqueQueryWordLength,
								   UniqueQueryWordID uniqueQueryWordID, 
								   MatchType matchType,
								   unsigned int matchDist);
	void ProcessNewExactWord(ExactWord* exactWord);
	void UpdateDependencies(unsigned int wordLength, unsigned int offset, unsigned int size);
	void WaitAll();
	void JoinAll();
};

END_NAMESPACE
