#ifndef CORE_H
#define CORE_H

#include "../libfuzzy/fuzzy.h"
#include "control.h"
#include "common.h"

#include <sys/time.h>
#include <sparsehash/dense_hash_map>
#include <sparsehash/dense_hash_set>
#include <algorithm>
#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <cassert>
#include <cstring>
#include <cstdio>

const int MIN_WORD_LENGTH = 4;
const int MAX_WORD_LENGTH = 31;
const int MAX_QUERY_WORDS = 5;
const int MAX_DOC_LENGTH = 1 << 22;
const int MAX_QUERY_LENGTH = (MAX_WORD_LENGTH + 1) * MAX_QUERY_WORDS;
const int MAX_DISTANCE = 4;

#ifdef LOCAL
const int THREADS = 8;
#else
const int THREADS = 13;
#endif

extern __thread int workerID;

typedef unsigned int QueryID;
typedef unsigned int DocID;

enum MatchType
{
	MT_EXACT_MATCH,
	MT_HAMMING_DIST,
	MT_EDIT_DIST
};

enum ErrorCode
{
	EC_SUCCESS,
	EC_NO_AVAIL_RES,
	EC_FAIL
};

struct Query
{
	bool isDeleted;
	bool isInserted;
	QueryID queryID;
	std::vector<std::string> strWords;
	std::vector<int> intWords;
	MatchType matchType;
	int matchDist;

	Query(QueryID id, const char *str, MatchType type, unsigned int dist)
		: isDeleted(false), isInserted(false), queryID(id), matchType(type), matchDist(dist)
	{
		splitStringVec(str, ' ', strWords);
		sort(strWords.begin(), strWords.end());
		strWords.erase(unique(strWords.begin(), strWords.end()), strWords.end());
	}
};

inline bool isEDQuery(const Query &query)
{
	return query.matchType == MT_EDIT_DIST
		|| query.matchType == MT_EXACT_MATCH;
}

inline bool isEDQuery(const Query *query)
{
	return isEDQuery(*query);
}

struct Document
{
	DocID docID;
	char str[MAX_DOC_LENGTH + 1];
	google::dense_hash_set<std::string> words;
	bool initGoogleHash = false;

	void init(DocID id, const char *str)
	{
		docID = id;
		strcpy(this->str, str);
		if (!initGoogleHash)
		{
			initGoogleHash = true;
			words.set_empty_key("");
		}
	}

	void split()
	{
		splitStringSet(str, ' ', words);
	}
};

struct Result
{
	DocID docID;
	unsigned int numRes;
	QueryID *matchQueryIDs;
};

struct KeyIndex
{
#ifdef STAT
	int edTotal = 0, hmTotal = 0;
#endif
	int keyIndexID, maxED = -1, maxHM = -1;
	std::vector<QueryID> edIDs[MAX_DISTANCE];
	std::vector<QueryID> hmIDs[MAX_DISTANCE];

	KeyIndex(int id)
	{
		keyIndexID = id;
	}

	void clear()
	{
#ifdef STAT
		edTotal = 0;
		hmTotal = 0;
#endif
		maxED = maxHM = -1;
		for (auto &list : edIDs) list.clear();
		for (auto &list : hmIDs) list.clear();
	}

    bool insert(const Query &query, bool isFirst)
	{
		if (isEDQuery(query))
		{
#ifdef STAT
			edTotal++;
#endif
			return insertInternal(query.queryID, query.matchDist, maxED, edIDs, isFirst);
		} else {
#ifdef STAT
			hmTotal++;
#endif
			return insertInternal(query.queryID, query.matchDist, maxHM, hmIDs, isFirst);
		}
	}
private:
	bool insertInternal(int id, int dist, int &maxDist, std::vector<QueryID> *ids, bool isFirst)
	{
		bool changed = false;
		if (dist > maxDist)
		{
			maxDist = dist;
			changed = true;
		}
		if (isFirst)
			ids[dist].push_back(id);
		return changed;
	}
};

struct ResultKeyDist
{
	int timeStamp;
	int useTempScore = 0;
	int lastGlobal = 0;
	std::vector<std::pair<int, int>> edRes;
	std::vector<std::pair<int, int>> hmRes;

	void clear()
	{
		timeStamp = 0;
		useTempScore = 0;
		lastGlobal = 0;
		edRes.clear();
		hmRes.clear();
	}
};

extern "C"
{
	ErrorCode InitializeIndex();
	ErrorCode DestroyIndex();
	ErrorCode StartQuery(QueryID, const char*, MatchType, unsigned int);
	ErrorCode EndQuery(QueryID);
	ErrorCode MatchDocument(DocID, const char*);
	ErrorCode GetNextAvailRes(DocID*, unsigned int*, QueryID**);
}

#endif

