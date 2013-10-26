
#ifndef __SIGMOD_THREADSPOOL_H
#define __SIGMOD_THREADSPOOL_H

#include "../include/core.h"
#include "trie.h"
#include <pthread.h>
#include <semaphore.h>
#include <deque>
#include <vector>

#define MAX_THREADS 20  

struct MatchResult
{
	TrieTree* node;
	uint	match_index;
};

struct ThreadPackage
{
	int id;

	MatchResult matches[1000000];
	int tot;
	int tree[MAX_DOC_LENGTH>>2][26];
	bool isWord[MAX_DOC_LENGTH>>2];
	int tree_t[MAX_DOC_LENGTH>>2];
};

struct Job
{
	void*	(*f)(void* arg, void* package);
	void*	arg;
};

struct Threadspool
{
	pthread_t 		threads[MAX_THREADS];
	std::deque<Job*>	jobs;
	int 			tot_threads;
	sem_t*			sem;
	bool			kill;
};

void* FunctionThreads(void* message);

void InitThreadspool();

void DestroyThreadspool();

void AddJob(Job* job);

Job* GetJob(Threadspool* thpool);

void AllJobDone();

#endif // __SIGMOD_THREADSPOOL_H
