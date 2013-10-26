#pragma once

#include "config.h"
#include "thread_pool/mutex.h"

#include <cassert>
#include <cstdlib>
#include <cstring>

START_NAMESPACE

template <class T>
class ConcurrentVector {
  public:
	ConcurrentVector() {
		Init();
	}

	void Init() {
		size = 0;
		maxSize = 0;
		warehouse = NULL;
		pthread_mutex_init(&resizeGuard, NULL);
	}

	void Destroy() {
		if (warehouse != NULL)
			free(warehouse);
	}

	~ConcurrentVector() {
		Destroy();
	}

	void Insert(const T& newElement) {
		pthread_mutex_lock(&resizeGuard);
		if (size == maxSize) {
			Resize();
		}
		warehouse[size++] = newElement;
		pthread_mutex_unlock(&resizeGuard);
	}

	unsigned int size;
	T* warehouse;

  private:
	static const int INITIAL_SIZE = 1;

	void Resize() {
		if (maxSize > 0) {
			T* newWarehouse = (T*)malloc(maxSize * 2 * sizeof(T));
			memcpy(newWarehouse, warehouse, maxSize * sizeof(T));
			maxSize *= 2;
			T* oldWarehouse = warehouse;
			warehouse = newWarehouse;
			free(oldWarehouse);
		} else {
			maxSize = INITIAL_SIZE;
			warehouse = (T*)malloc(INITIAL_SIZE * sizeof(T));
		}
	}

	unsigned int maxSize;
	pthread_mutex_t resizeGuard;
};

END_NAMESPACE
