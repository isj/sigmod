#ifndef LIB_INDEX_TIME_H
#define LIB_INDEX_TIME_H

#include "common.h"

struct Data
{
	int id;
	int time;
	Data()
	{
	}
	Data(int _id, int _time)
	{
		id = _id;
		time = _time;
	}
};

class IndexTime
{
	friend class SearchTime;
public:
	void init(int);
	void insert(int, const char*, int);
	void clear();
	IndexTime();
	~IndexTime();
private:
	void insertInternal(int, const char*, int);
	void insert0Internal(int, const char*, int);

	std::vector<Part> **partIndex;
	int **partLen, **partPos;

	int maxID;
	google::dense_hash_map<unsigned long long, std::vector<Data>> **invList;
	google::dense_hash_map<std::string, Data> index0;
	int D, PN;
};

#endif

