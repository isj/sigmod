#pragma once

#include "structs.h"
#include "config.h"
#include "thread_pool/mutex.h"
#include "thread_pool/condition.h"
#include "thread_pool/thread.h"
#include "hash_structs/hashset.h"

START_NAMESPACE

class DocumentThreadPool;

enum DocumentThreaPoolTask {
	PARSE_DOCUMENT_TASK,
	BUILD_RESULT_TASK
};

class DocumentWorkerThread : public Thread {
	private:
	volatile bool documentReady;
	DocumentThreadPool* threadPool;

	int threadID;

	Mutex mutex;
	Condition ready;

	DocumentThreaPoolTask task;

	char* documentText;
	DocID documentID;

	unsigned int index;

	Hashset hashes;
	Vector<unsigned int> queryIDs;

	void ParseDocument();
	void BuildResult();

	public:
	DocumentWorkerThread();
	void Initialize(DocumentThreadPool* threadPool, int threadID);
	void AddDocument(char* documentText, DocID documentID);
	void AddIndex(const unsigned int documentIndex);

	void Run();
};

END_NAMESPACE
