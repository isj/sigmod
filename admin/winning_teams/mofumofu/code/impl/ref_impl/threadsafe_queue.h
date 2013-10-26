#pragma once 

#include <pthread.h>
#include <queue>

using namespace std;


template<class T> class ts_queue
{
	queue<T> que;
	pthread_mutex_t mutex;
	pthread_cond_t cond_empty;
public:
	ts_queue():que(),mutex(PTHREAD_MUTEX_INITIALIZER),cond_empty(PTHREAD_COND_INITIALIZER){;}

	void enqueue(const T &value)
	{
		pthread_mutex_lock(&mutex);
		que.push(value);
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&cond_empty);
	}

	T dequeue()
	{
		pthread_mutex_lock(&mutex);
		while (que.empty()) pthread_cond_wait(&cond_empty, &mutex);
		T value = que.front();
		que.pop();
		pthread_mutex_unlock(&mutex);
		return value;
	}

	size_t size()
	{
		pthread_mutex_lock(&mutex);
		size_t size = que.size();
		pthread_mutex_unlock(&mutex);
		return size;
	}
};
