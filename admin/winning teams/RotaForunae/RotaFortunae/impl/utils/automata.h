#include "config.h"

#include <cstdio>

START_NAMESPACE

int transitions_3_0[64][8];
int transitions_3_1[256][16];
int transitions_3_2[64][8];

inline int PackState_3_0(int value1, int value2, int value3) {
	int result = 0;

	int badState = (value2 == 4);

	value1 = std::min(value1, 3);
	value2 = std::min(value2, 3);
	value3 = std::min(value3, 3);

	result |= (value1 << 5);
	result |= (value2 << 3);
	result |= (value3 << 1);

	if (badState) {
		result |= badState;
	} else {
		result >>= 1;
	}

	return result;
}

inline int PackState_3_1(int value1, int value2, int value3, int value4) {
	int result = 0;

	int badState = (value3 == 4);

	value1 = std::min(value1, 3);
	value2 = std::min(value2, 3);
	value3 = std::min(value3, 3);
	value4 = std::min(value4, 3);

	result |= (value1 << 6);
	result |= (value2 << 4);
	result |= (value3 << 2);
	result |= (value4 << 0);

	if (badState) {
		result = 0;
	}

	return result;
}

inline int PackState_3_2(int value1, int value2, int value3) {
	int result = 0;

	int badState = (value3 == 4);

	value1 = std::min(value1, 3);
	value2 = std::min(value2, 3);
	value3 = std::min(value3, 3);

	result |= (value1 << 4);
	result |= (value2 << 2);
	result |= (value3 << 0);

	if (badState) {
		result = 0;
	}

	return result;
}

inline void ConstructAutomata_3_0() {
	for (int value1 = 0; value1 <= 3; ++value1) {
		for (int value2 = 0; value2 <= 3; ++value2) {
			for (int value3 = 0; value3 <= 3; ++value3) {
				int currentState = PackState_3_0(value1, value2, value3);

				for (int i = 0; i < 8; ++i) {
					int next1 = value1;
					int next2 = value2;
					int next3 = value3;

					bool b1 = !!(i & 4);
					bool b2 = !!(i & 2);
					bool b3 = !!(i & 1);

					if (b1) {
						next1 = std::min(next1 + 1, next2 + 1);
					}

					if (b2) {
						next2 = std::min(next2 + 1, std::min(next1 + 1, next3 + 1));
					}

					if (b3) {
						next3 = std::min(next2 + 1, next3 + 1);
					}

					int nextState = PackState_3_0(next1, next2, next3);

					transitions_3_0[currentState][i] = nextState;
				}
			}
		}
	}
}

inline void ConstructAutomata_3_1() {
	for (int value1 = 0; value1 <= 3; ++value1) {
		for (int value2 = 0; value2 <= 3; ++value2) {
			for (int value3 = 0; value3 <= 3; ++value3) {
				for (int value4 = 0; value4 <= 3; ++value4) {
					int currentState = PackState_3_1(value1, value2, value3, value4);

					for (int i = 0; i < 16; ++i) {
						int next1 = value1;
						int next2 = value2;
						int next3 = value3;
						int next4 = value4;

						bool b1 = !!(i & 8);
						bool b2 = !!(i & 4);
						bool b3 = !!(i & 2);
						bool b4 = !!(i & 1);

						if (b1) {
							next1 = std::min(next1 + 1, next2 + 1);
						}
						if (b2) {
							next2 = std::min(next2 + 1, std::min(next1 + 1, next3 + 1));
						}
						if (b3) {
							next3 = std::min(next3 + 1, std::min(next2 + 1, next4 + 1));
						}
						if (b4) {
							next4 = std::min(next3 + 1, next4 + 1);
						}

						int nextState = PackState_3_1(next1, next2, next3, next4);

						transitions_3_1[currentState][i] = nextState;
					}
				}
			}
		}
	}
}

inline void ConstructAutomata_3_2() {
	for (int value1 = 0; value1 <= 3; ++value1) {
		for (int value2 = 0; value2 <= 3; ++value2) {
			for (int value3 = 0; value3 <= 3; ++value3) {
				int currentState = PackState_3_2(value1, value2, value3);

				for (int i = 0; i < 8; ++i) {
					int next1 = value1;
					int next2 = value2;
					int next3 = value3;

					bool b1 = !!(i & 4);
					bool b2 = !!(i & 2);
					bool b3 = !!(i & 1);

					if (b1) {
						next1 = std::min(next1 + 1, next2 + 1);
					}

					if (b2) {
						next2 = std::min(next2 + 1, std::min(next1 + 1, next3 + 1));
					}

					if (b3) {
						next3 = std::min(next2 + 1, next3 + 1);
					}

					int nextState = PackState_3_2(next1, next2, next3);

					transitions_3_2[currentState][i] = nextState;
				}
			}
		}
	}
}

inline void PrintTransitions_3_0() {
	printf("{");
	for (int state = 0; state < 64; ++state) {
		//printf("{");
		for (int i = 0; i < 7; ++i) {
			printf("%d, ", transitions_3_0[state][i]);
		}
		printf("%d,\n", transitions_3_0[state][7]);
	}
	printf("}");
}

inline void PrintTransitions_3_1() {
	printf("{");
	for (int state = 0; state < 256; ++state) {
		//printf("{");
		for (int i = 0; i < 15; ++i) {
			printf("%d, ", transitions_3_1[state][i]);
		}
		printf("%d,\n", transitions_3_1[state][15]);
	}
	printf("}");
}

inline void PrintTransitions_3_2() {
	printf("{");
	for (int state = 0; state < 64; ++state) {
		//printf("{");
		for (int i = 0; i < 7; ++i) {
			printf("%d, ", transitions_3_2[state][i]);
		}
		printf("%d,\n", transitions_3_2[state][7]);
	}
	printf("}");
}

END_NAMESPACE
