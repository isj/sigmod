#pragma once

#include "config.h"

#include <pthread.h>

START_NAMESPACE

class Mutex {
	friend class Condition;

	public:
	Mutex() {
		pthread_mutex_init(&mutex_, 0);
	}

	~Mutex() {
		pthread_mutex_destroy(&mutex_);
	}

	void Lock() {
		pthread_mutex_lock(&mutex_);
	}

	void Unlock() {
		pthread_mutex_unlock(&mutex_);
	}

	private:
	pthread_mutex_t mutex_;
};

END_NAMESPACE
