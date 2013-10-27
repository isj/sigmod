#pragma once

#include "config.h"
#include "mutex.h"
#include <pthread.h>

START_NAMESPACE

class Condition {
	public:
	Condition() {
		pthread_cond_init(&cond, NULL);
	}

	~Condition() {
	}

	void Signal() {
		pthread_cond_signal(&cond);
	}

	void Broadcast() {
		pthread_cond_broadcast(&cond);
	}

	void Wait(Mutex& mutex) {
		pthread_cond_wait(&cond, &mutex.mutex_);
	}

	private:
	pthread_cond_t cond;
};

END_NAMESPACE
