#pragma once

#include "config.h"

#include <pthread.h>

START_NAMESPACE

// A simple object-oriented pthreads wrapper
class Thread {
	public:
	// Constructs a new thread
	Thread();

	// Starts the thread
	void Start();

	// Blocks the calling thread until the thread terminates
	void Join();

	// The thread's main function
	// This is the only function that needs to be implemented by child classes
	virtual void Run() = 0;

	private:
	// The POSIX thread itself
	pthread_t* thread_;
};

END_NAMESPACE
