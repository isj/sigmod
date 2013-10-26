#include "query_thread_pool.h"

#include <cstdio>

START_NAMESPACE

QueryThreadPool::QueryThreadPool() {}

void QueryThreadPool::Initialize() {
	stackSize = 0;
	for (int i = 0; i < QUERY_POOL_THREAD_COUNT; ++i) {
		threads[i].Initialize(this, i);
		threads[i].Start();

		Ready(threads + i);
	}
}

QueryThreadPool::~QueryThreadPool() {}

void QueryThreadPool::ProcessNewUniqueQueryWord(const char* uniqueQueryWordText, hash_type uniqueQueryWordHash, int uniqueQueryWordLength, UniqueQueryWordID uniqueQueryWordID, MatchType matchType, unsigned int matchDist) {
	mutex.Lock();
	while (!stackSize) {
		ready.Wait(mutex);
	}

	QueryWorkerThread* thread = stack[--stackSize];

	mutex.Unlock();

	thread->AddUniqueQueryWord(uniqueQueryWordText, uniqueQueryWordHash, uniqueQueryWordLength, uniqueQueryWordID, matchType, matchDist);
}

void QueryThreadPool::ProcessNewExactWord(ExactWord* exactWord) {
	mutex.Lock();
	while (!stackSize) {
		ready.Wait(mutex);
	}

	QueryWorkerThread* thread = stack[--stackSize];

	mutex.Unlock();

	thread->AddExactWord(exactWord);
}

void QueryThreadPool::UpdateDependencies(unsigned int wordLength, unsigned int offset, unsigned int size) {
	mutex.Lock();
	while (!stackSize) {
		ready.Wait(mutex);
	}

	QueryWorkerThread* thread = stack[--stackSize];

	mutex.Unlock();

	thread->AddUpdatePart(wordLength, offset, size);
}

void QueryThreadPool::Ready(QueryWorkerThread* thread) {
	mutex.Lock();

	int index = stackSize++;
	stack[index] = thread;

	if (!index) {
		ready.Signal();
	}
	mutex.Unlock();

	if (index + 1 == QUERY_POOL_THREAD_COUNT) {
		mutex.Lock();
		doneCondition.Signal();
		mutex.Unlock();
	}
}

void QueryThreadPool::WaitAll() {
	mutex.Lock();
	while (stackSize != QUERY_POOL_THREAD_COUNT) {
		doneCondition.Wait(mutex);
	}
	mutex.Unlock();
}

void QueryThreadPool::JoinAll() {
	for (int i = 0; i < QUERY_POOL_THREAD_COUNT; ++i) {
		threads[i].AddExactWord(NULL);
		threads[i].Join();
	}
}

END_NAMESPACE
