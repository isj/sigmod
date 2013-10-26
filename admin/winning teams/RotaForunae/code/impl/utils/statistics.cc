#include "statistics.h"

#include <cstdio>

START_NAMESPACE

long long Statistics::checkExactMatchCalls = 0;
long long Statistics::checkHammingDistance1Calls = 0;
long long Statistics::checkHammingDistance2Calls = 0;
long long Statistics::checkHammingDistance3Calls = 0;
long long Statistics::checkEditDistance1Calls = 0;
long long Statistics::checkEditDistance2Calls = 0;
long long Statistics::checkEditDistance3Calls = 0;

Statistics::Statistics() {

}

void Statistics::Print() {
	printf("\n");

	printf("CEM  calls: %lld\n", Statistics::checkExactMatchCalls);
	printf("CHD1 calls: %lld\n", Statistics::checkHammingDistance1Calls);
	printf("CHD2 calls: %lld\n", Statistics::checkHammingDistance2Calls);
	printf("CHD3 calls: %lld\n", Statistics::checkHammingDistance3Calls);
	printf("CED1 calls: %lld\n", Statistics::checkEditDistance1Calls);
	printf("CED2 calls: %lld\n", Statistics::checkEditDistance2Calls);
	printf("CED3 calls: %lld\n", Statistics::checkEditDistance3Calls);

	printf("\n");
}

END_NAMESPACE
