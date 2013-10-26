#pragma once

#include "config.h"
#include "structs.h"
#include "document_worker_thread.h"

START_NAMESPACE

class DocumentThreadPool {
	friend class DocumentWorkerThread;

	private:
	DocumentWorkerThread threads[DOCUMENT_POOL_THREAD_COUNT];

	DocumentWorkerThread* stack[DOCUMENT_POOL_THREAD_COUNT];
	volatile int stackSize;

	Mutex mutex;
	Condition ready;

	Condition doneCondition;

	void Ready(DocumentWorkerThread* thread);

	public:
	DocumentThreadPool();
	~DocumentThreadPool();

	void Initialize();

	void ParseDocument(char* documentText, DocID documentID);
	void BuildResult(const unsigned int documentIndex);

	void WaitAll();
	void JoinAll();
};

END_NAMESPACE
