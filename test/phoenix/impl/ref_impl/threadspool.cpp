
#include "threadspool.h"
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <deque>

using namespace std;

Threadspool thpool;

pthread_mutex_t mut_jobqueue;
pthread_mutex_t mut_count;
pthread_mutex_t mut_active;

sem_t * sem_finish;

int active;

void* FunctionThreads(void* message)
{
	Threadspool* thpool = (Threadspool*)message;
	ThreadPackage* package = new ThreadPackage;

	pthread_mutex_lock(&mut_count);
	package->id = thpool->tot_threads;
	++(thpool->tot_threads);
	pthread_mutex_unlock(&mut_count);

	while (!thpool->kill) {
		if (sem_wait(thpool->sem)) {
			printf("ERROR %d\n", package->id);
			 return NULL;
		}
		
		if (!thpool->kill){
			pthread_mutex_lock(&mut_active);
			++active;
			pthread_mutex_unlock(&mut_active);

			Job* job = GetJob(thpool);
			if (job == NULL){
				printf("Job Queue Error!\n");
				return NULL;
			}
			job->f(job->arg, (void*)package);
			delete job;

			pthread_mutex_lock(&mut_active);
			--active;
			pthread_mutex_unlock(&mut_active);
		}		

	}
	return NULL;
}

void InitThreadspool()
{
	sem_finish = new sem_t;

	thpool.jobs.clear();
	thpool.kill = false;
	thpool.tot_threads = 0;

	pthread_mutex_init(&mut_count, NULL);
	pthread_mutex_init(&mut_active, NULL);
	active = 0;

	thpool.sem = new sem_t;
	sem_init(thpool.sem, 0, 0);
	for (int i = 0; i < MAX_THREADS; ++i)
		pthread_create(&thpool.threads[i], NULL, FunctionThreads, &thpool); 

	pthread_mutex_init(&mut_jobqueue, NULL);	
}

void DestroyThreadspool()
{
	thpool.kill = true;
	for (int i = 0; i < MAX_THREADS; ++i)
		sem_post(thpool.sem);
	for (int i = 0; i < MAX_THREADS; ++i)
		pthread_join(thpool.threads[i], NULL);
}

void AddJob(Job* job)
{
	pthread_mutex_lock(&mut_jobqueue);
	thpool.jobs.push_back(job);
	pthread_mutex_unlock(&mut_jobqueue);

	sem_post(thpool.sem);
}
 
Job* GetJob(Threadspool* thpool)
{
	Job* job;
	pthread_mutex_lock(&mut_jobqueue);
	if ((thpool->jobs).size() == 0) 
		job = NULL;
	else
	{
		job = (thpool->jobs)[0];
		(thpool->jobs).pop_front();	
	}
	pthread_mutex_unlock(&mut_jobqueue);
	
	return job;
}

void* JobFinish(void* arg, void* package)
{
	int local_active = -1;
	while (local_active != 1) {
		pthread_mutex_lock(&mut_active);
		local_active = active;
		pthread_mutex_unlock(&mut_active);
	}
	sem_post(sem_finish);
	return NULL;
}

void AllJobDone()
{
	Job* job = new Job;
	job->f = JobFinish;
	job->arg = NULL;

	AddJob(job);

	if (sem_wait(sem_finish)) 
		printf("ERROR\n");
}

