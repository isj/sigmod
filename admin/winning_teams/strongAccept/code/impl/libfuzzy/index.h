#ifndef LIB_INDEX_H
#define LIB_INDEX_H

#include "common.h"

class Part
{
public:
	int stPos;    // start position of string
	int Lo;       // start position of segment
	int partLen;  // substring/segment length
	int len;      // length of indexed string
	Part(int _s, int _o, int _p, int _l)
		: stPos(_s), Lo(_o), partLen(_p), len(_l) {}
};

class Index
{
	friend class Search;
	friend class IndexTime;
public:
	static void init();

	void init(int);
	void insert(int, const char*);
	void clear();
	Index();
	~Index();
private:
	void insertInternal(int, const char*);
	void insert0Internal(int, const char*);

	static std::vector<Part> **partIndexes[MAXED];
	static int **partLens[MAXED], **partPoses[MAXED];

	std::vector<Part> **partIndex;
	int **partLen, **partPos;

	int maxID;
	google::dense_hash_map<unsigned long long, std::vector<int>> **invList;
	google::dense_hash_map<std::string, int> index0;
	int D, PN;
};

#endif

