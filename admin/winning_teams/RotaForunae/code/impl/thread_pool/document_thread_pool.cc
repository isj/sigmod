#include "document_thread_pool.h"

START_NAMESPACE

DocumentThreadPool::DocumentThreadPool() {}

void DocumentThreadPool::Initialize() {
	stackSize = 0;

	for (int i = 0; i < DOCUMENT_POOL_THREAD_COUNT; ++i) {
		threads[i].Initialize(this, i);
		threads[i].Start();

		Ready(threads + i);
	}
}

DocumentThreadPool::~DocumentThreadPool() {}

void DocumentThreadPool::ParseDocument(char* documentText, DocID documentID) {
	mutex.Lock();
	while (!stackSize) {
		ready.Wait(mutex);
	}

	DocumentWorkerThread* thread = stack[--stackSize];

	mutex.Unlock();

	thread->AddDocument(documentText, documentID);
}

void DocumentThreadPool::BuildResult(const unsigned int documentIndex) {
	mutex.Lock();
	while (!stackSize) {
		ready.Wait(mutex);
	}

	DocumentWorkerThread* thread = stack[--stackSize];

	mutex.Unlock();

	thread->AddIndex(documentIndex);
}

void DocumentThreadPool::Ready(DocumentWorkerThread* thread) {
	mutex.Lock();

	int index = stackSize++;
	stack[index] = thread;

	if (!index) {
		ready.Signal();
	}
	mutex.Unlock();

	if (index + 1 == DOCUMENT_POOL_THREAD_COUNT) {
		mutex.Lock();
		doneCondition.Signal();
		mutex.Unlock();
	}
}

void DocumentThreadPool::WaitAll() {
	mutex.Lock();
	while (stackSize != DOCUMENT_POOL_THREAD_COUNT) {
		doneCondition.Wait(mutex);
	}
	mutex.Unlock();
}

void DocumentThreadPool::JoinAll() {
	for (int i = 0; i < DOCUMENT_POOL_THREAD_COUNT; ++i) {
		threads[i].AddDocument(NULL, -1);
		threads[i].Join();
	}
}

END_NAMESPACE
