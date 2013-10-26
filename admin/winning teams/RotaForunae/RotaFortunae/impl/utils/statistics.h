#pragma once

#include "config.h"

START_NAMESPACE

class Statistics {
  public:
	Statistics();

	static long long checkExactMatchCalls;
	static long long checkHammingDistance1Calls;
	static long long checkHammingDistance2Calls;
	static long long checkHammingDistance3Calls;
	static long long checkEditDistance1Calls;
	static long long checkEditDistance2Calls;
	static long long checkEditDistance3Calls;

	static void Print();
};

END_NAMESPACE
