#pragma once

#include "config.h"

#include <cassert>
#include <cstdlib>

START_NAMESPACE

template <class T>
class Vector {
	public:
	Vector() {
		size = 0;
		maxSize = INITIAL_SIZE;
		warehouse = (T*)malloc(INITIAL_SIZE * sizeof(T));
	}

	~Vector() {
		free(warehouse);
	}

	void push_back(const T& newElement) {
		if (size == maxSize) {
			Resize();
		}
		warehouse[size++] = newElement;
	}

	T& push_back() {
		if (size == maxSize) {
			Resize();
		}
		return warehouse[size++];
	}
	
	unsigned int push_back_return_index() {
		if (size == maxSize) {
			Resize();
		}
		return size++;
	}

	const T& operator[] (unsigned int index) const {
		//assert(index < size);
		return warehouse[index];
	}

	T& operator[] (unsigned int index) {
		//assert(index < size);
		return warehouse[index];
	}

	void reserve(int newSize) {
		free(warehouse);
		maxSize = newSize;
		warehouse = (T*)malloc(maxSize * sizeof(T));
	}

	void Resize(int new_size) {
		maxSize = new_size;
		T* newWarehouse = (T*)malloc(maxSize * sizeof(T));
		memcpy(newWarehouse, warehouse, size * sizeof(T));
		free(warehouse);
		warehouse = newWarehouse;
	}

	T* warehouse;
	unsigned int size;
	
	private:
	void Resize() {
		T* newWarehouse = (T*)malloc(maxSize * 2 * sizeof(T));
		memcpy(newWarehouse, warehouse, maxSize * sizeof(T));
		maxSize *= 2;
		T* oldWarehouse = warehouse;
		warehouse = newWarehouse;
		free(oldWarehouse);
	}

	static const int INITIAL_SIZE = 16;
	unsigned int maxSize;
};

END_NAMESPACE
