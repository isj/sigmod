#ifndef LIB_SEARCH_TIME_H
#define LIB_SEARCH_TIME_H

#include "common.h"

struct SharedInfoTime
{
	IndexTime edIndexes[MAXED];
	IndexTime hmIndexes[MAXED];

	void init();
	void clear();
	void insertED(int ed, int id, const char *record, int time);
	void insertHM(int hm, int id, const char *record, int time);
};

class SearchTime
{
public:
	void init(SharedInfoTime *sharedInfo);
	void searchHM(int time, const std::vector<std::string> &dict, int hm, const char *query, std::vector<std::pair<int, int>> &res);
	void searchED(int time, const std::vector<std::string> &dict, int ed, const char *query, std::vector<std::pair<int, int>> &res);
private:
	void getED(__m128i *a, const char *b, int *alen, int blen, int *eds, int count, int D);
	void getHM(__m128i *a, const char *b, int len, int *hms, int count, int D);
	void checkQuickRefID(IndexTime*);

	int D;
	IndexTime *index;
	SharedInfoTime *sharedInfo;
	std::vector<int> testIDs;
	int quickRefSize;
	bool *quickRef;

	__m128i matrix[SAFELEN][SAFELEN] __attribute__((aligned (16)));
	char buffer[SAFELEN][16] __attribute__((aligned (16)));
	Hash hash;
};

#endif

