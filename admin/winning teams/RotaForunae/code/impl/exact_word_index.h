#pragma once

#include "config.h"
#include "core.h"
#include "hash_structs/exact_word_hashset.h"
#include "thread_pool/document_thread_pool.h"
#include "thread_pool/thread_pools.h"

START_NAMESPACE

class ExactWordIndex {
	public:
	static void UpdateDependencies();

	static void ProcessNewExactWord(ExactWord* exactWord);

	static ExactWordHashset words[HASHSET_SIZE];
	static unsigned int sizes[HASHSET_SIZE];

	static void UpdateSizes();

	static void Cleanup();

	static ConcurrentVector<Document*> documentStack;
	static ConcurrentVector<int> documentExactWordStack[MAX_WORD_LENGTH + 1];
};

END_NAMESPACE
