#include "thread.h"
#include <errno.h>
#include <stdexcept>
#include <stdio.h>

#include <assert.h>

START_NAMESPACE

// A helper function used to run the thread's main functions
static void *_thread_func(void *arg) {
	Thread* thread = static_cast<Thread*>(arg);
	thread->Run();

	// Exit the thread
	pthread_exit(NULL);

	return NULL;
}

// Constructs a new thread
Thread::Thread() {
	thread_ = NULL;
}

// Starts the thread
void Thread::Start() {
	if (thread_ != NULL) {
		return;
	}

	thread_ = new pthread_t;

	int result = pthread_create(thread_, NULL, *_thread_func, this);
	if (result != 0) {
		assert(false);
	}
}

// Blocks the calling thread until the thread terminates
void Thread::Join() {
	if (thread_) {
		pthread_join(*thread_, NULL);

		delete thread_;
		thread_ = NULL;
	}
}

END_NAMESPACE
