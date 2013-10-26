#pragma once

#include "structs.h"
#include "config.h"
#include "thread_pool/mutex.h"
#include "thread_pool/condition.h"
#include "thread_pool/thread.h"

START_NAMESPACE

class QueryThreadPool;

enum QueryThreaPoolTask {
	NEW_UQW_TASK,
	NEW_EXACT_WORD_TASK,
	UPDATE_DEPENDENCIES_TASK
};

class QueryWorkerThread : public Thread {
	private:
	volatile bool wordReady;

	volatile QueryThreaPoolTask task;

	MatchType matchType;
	unsigned int matchDist;
	int uniqueQueryWordLength;
	hash_type uniqueQueryWordHash;
	UniqueQueryWordID uniqueQueryWordID;
	char uniqueQueryWordText[MAX_WORD_LENGTH + 1];

	ExactWord* exactWord;

	unsigned int updatePackWordLength;
	unsigned int updatePackOffset;
	unsigned int updatePackSize;

	volatile bool stopped;

	QueryThreadPool* threadPool;

	Mutex mutex;
	Condition ready;

	void ProcessNewUniqueQueryWord();
	void ProcessNewExactWord();
	void UpdateDependencies();

	public:
	QueryWorkerThread();
	void Initialize(QueryThreadPool* threadPool, int threadID);
	void AddUniqueQueryWord(const char*       uniqueQueryWordText,
							hash_type         uniqueQueryWordHash,
							int               uniqueQueryWordLength,
							UniqueQueryWordID uniqueQueryWordID,
							MatchType matchType,
							unsigned int matchDist);
	void AddExactWord(ExactWord* exactWord);
	void AddUpdatePart(unsigned int wordLength, unsigned int offset, unsigned int size);
	void Run();

	int threadID;
};

END_NAMESPACE
