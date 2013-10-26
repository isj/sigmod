#include "core.h"

using namespace std;

int queryRounds;
int lastTempRebuildRounds;
int lastRebuildRounds;
int rebuildRounds;

int realQuery;
int indexQuery;
QueryID maxQueryID;
int docSize;

vector<Query*> queries;
vector<Document*> documents;
vector<Result> results;

Search permSearcher[THREADS];
SearchTime tempSearcher[THREADS];
SharedInfo *permSharedInfo;
SharedInfoTime *tempSharedInfo;

vector<string> qwordVector;
vector<KeyIndex*> keyIndexes;
google::dense_hash_map<string, int> qwordIDMap;
google::dense_hash_map<int, int> qwordFreqMap;
typedef google::dense_hash_map<std::string, ResultKeyDist*> Dword2resultKeyDist;
vector<Dword2resultKeyDist::iterator> distVectorPart[26];
vector<Dword2resultKeyDist::iterator> distVector;
Dword2resultKeyDist dwordCache[26];

int insertTaskID;
int searchTaskID;
int calculateTaskID;
int combineTaskID;
int splitTaskID;

void clearAll()
{
	permSharedInfo->clear();
	for (KeyIndex *keyIndex : keyIndexes)
		if (keyIndex != NULL) keyIndex->clear();
}

void doInsert(int id)
{
	for (KeyIndex *keyIndex : keyIndexes)
	{
		if (keyIndex == NULL) continue;
		if (id < 4)
		{
			if (id == keyIndex->maxED)
				permSharedInfo->insertED(id, keyIndex->keyIndexID, qwordVector[keyIndex->keyIndexID].c_str());
		} else {
			if (id - 4 == keyIndex->maxHM)
				permSharedInfo->insertHM(id - 4, keyIndex->keyIndexID, qwordVector[keyIndex->keyIndexID].c_str());
		}
	}
}

void insertQueries(int startID, bool isRebuild)
{
	vector<Query*> sortQueries(queries.begin() + startID, queries.end());
	auto middle = partition(sortQueries.begin(), sortQueries.end(), [](const Query *query) {
		return query->matchDist > 1;
	});
	partition(sortQueries.begin(), middle, [](const Query *query) {
		return query->matchDist == 3;
	});
	partition(middle, sortQueries.end(), [](const Query *query) {
		return query->matchDist == 1;
	});

	for (Query *query : sortQueries)
	{
		if (query->isDeleted) continue;
		if (query->intWords.empty())
		{
			for (const string &word : query->strWords)
			{
				int id = 0;
				auto iter = qwordIDMap.find(word);
				if (iter == qwordIDMap.end())
				{
					id = keyIndexes.size();
					KeyIndex *keyIndex = new KeyIndex(keyIndexes.size());
					qwordIDMap.insert(make_pair(word, keyIndexes.size()));
					keyIndexes.push_back(keyIndex);
					qwordVector.push_back(word);
				} else {
					id = iter->second;
				}
				query->intWords.push_back(id);
				qwordFreqMap[id]++;
			}
			sort(query->intWords.begin(), query->intWords.end(), [](int id1, int id2) {
				return qwordFreqMap[id1] < qwordFreqMap[id2];
			});
		}

		bool isFirst = true;
		for (int word : query->intWords)
		{
			KeyIndex *keyIndex = keyIndexes[word];
			const char *str = qwordVector[word].c_str();
			bool changed = keyIndex->insert(*query, isFirst);
			if (changed)
			{
				if (isEDQuery(query))
				{
					if (!isRebuild)
						permSharedInfo->insertED(query->matchDist, keyIndex->keyIndexID, str);
					if (!query->isInserted)
						tempSharedInfo->insertED(query->matchDist, keyIndex->keyIndexID, str, queryRounds);
				} else {
					if (!isRebuild)
						permSharedInfo->insertHM(query->matchDist, keyIndex->keyIndexID, str);
					if (!query->isInserted)
						tempSharedInfo->insertHM(query->matchDist, keyIndex->keyIndexID, str, queryRounds);
				}
			}
			isFirst = false;
		}

		query->isInserted = true;
	}

	if (isRebuild)
		doTask(insertTaskID, 8);
}

void calculateDocument(Document *doc, Result *res)
{
	vector<int> notZeroIDs;
	google::dense_hash_map<int, int> filterMapED;
	google::dense_hash_map<int, int> filterMapHM;
	filterMapED.set_empty_key(0);
	filterMapHM.set_empty_key(0);

	for (const string &word : doc->words)
	{
		auto &resultKeyDist = dwordCache[word[0] - 'a'][word];
		for (int k = 0; k < (int)resultKeyDist->edRes.size(); k++)
		{
			int id = resultKeyDist->edRes[k].first;
			auto pair = filterMapED.insert(make_pair(id, resultKeyDist->edRes[k].second));
			if (!pair.second) pair.first->second = min(pair.first->second, resultKeyDist->edRes[k].second);
		}

		for (int k = 0; k < (int)resultKeyDist->hmRes.size(); k++)
		{
			int id = resultKeyDist->hmRes[k].first;
			auto pair = filterMapHM.insert(make_pair(id, resultKeyDist->hmRes[k].second));
			if (!pair.second) pair.first->second = min(pair.first->second, resultKeyDist->hmRes[k].second);
		}
    }

	vector<QueryID> matchQueryIDs;
	for (auto iter = filterMapED.begin(); iter != filterMapED.end(); iter++)
	{
		for (int ed = iter->second; ed < MAX_DISTANCE; ed++)
		{
			for (int queryID : keyIndexes[iter->first]->edIDs[ed])
			{
				if (queries[queryID]->isDeleted) continue;
				bool isFirst = true;
				bool isResult = true;
				for (int word : queries[queryID]->intWords)
				{
					if (isFirst)
					{
						isFirst = false;
					} else {
						auto iter = filterMapED.find(word);
						if (iter == filterMapED.end()
								|| iter->second > queries[queryID]->matchDist)
						{
							isResult = false;
							break;
						}
					}
				}
				if (isResult) matchQueryIDs.push_back(queryID);
			}
		}
	}

	for (auto iter = filterMapHM.begin(); iter != filterMapHM.end(); iter++)
	{
		for (int hm = iter->second; hm < MAX_DISTANCE; hm++)
		{
			for (int queryID : keyIndexes[iter->first]->hmIDs[hm])
			{
				if (queries[queryID]->isDeleted) continue;
				bool isFirst = true;
				bool isResult = true;
				for (int word : queries[queryID]->intWords)
				{
					if (isFirst)
					{
						isFirst = false;
					} else {
						auto iter = filterMapHM.find(word);
						if (iter == filterMapHM.end()
								|| iter->second > queries[queryID]->matchDist)
						{
							isResult = false;
							break;
						}
					}
				}
				if (isResult) matchQueryIDs.push_back(queryID);
			}
		}
	}

	sort(matchQueryIDs.begin(), matchQueryIDs.end());

	res->docID = doc->docID;
	res->numRes = (unsigned int)matchQueryIDs.size();
	res->matchQueryIDs = 0;
	if (res->numRes)
		res->matchQueryIDs = (QueryID*)malloc(sizeof(QueryID) * res->numRes);
	for (int k = 0; k < (int)res->numRes; k++)
        res->matchQueryIDs[k] = matchQueryIDs[k];
}

void doSplit(int id)
{
	documents[id]->split();
}

void doSearch(int id)
{
	auto &pair = distVector[id];
	const char *query = pair->first.c_str();
	ResultKeyDist *result = pair->second;

	int caseNumber;
	if (result->timeStamp == 0 ||
			result->timeStamp < lastTempRebuildRounds ||
			(result->lastGlobal < rebuildRounds &&
			 	((result->timeStamp < queryRounds - 20) ||
			 result->useTempScore >= (rebuildRounds - lastRebuildRounds) * 1.6)))
	{
		caseNumber = 1;
		result->clear();
		result->lastGlobal = queryRounds;
	} else {
		result->useTempScore += queryRounds - result->timeStamp;
		caseNumber = 2;
	}

	if (caseNumber == 1)
	{
		for (int dist = 0; dist < MAX_DISTANCE; dist++)
		{
			permSearcher[workerID].searchED(qwordVector, dist, query, result->edRes);
			permSearcher[workerID].searchHM(qwordVector, dist, query, result->hmRes);
		}
	} else {
		for (int dist = 0; dist < MAX_DISTANCE; dist++)
		{
			tempSearcher[workerID].searchED(result->timeStamp + 1, qwordVector, dist, query, result->edRes);
			tempSearcher[workerID].searchHM(result->timeStamp + 1, qwordVector, dist, query, result->hmRes);
		}
	}

	result->timeStamp = queryRounds;
}

void doCombine(int k)
{
	google::dense_hash_set<string> newWordSet;
	newWordSet.set_empty_key("");

	for (int id = 0; id < docSize; id++)
	{
		for (const string &word : documents[id]->words)
			if (word[0] - 'a' == k && newWordSet.find(word) == newWordSet.end())
			{
				newWordSet.insert(word);
				auto pair = dwordCache[k].insert(make_pair(word, new ResultKeyDist()));
				if (pair.second) pair.first->second->timeStamp = 0;
			}
	}

	for (const string &word : newWordSet)
		distVectorPart[k].push_back(dwordCache[k].find(word));
}

void doCalculate(int id)
{
	calculateDocument(documents[id], &results[id]);
}

ErrorCode InitializeIndex()
{
	Hash::init();
	Index::init();
	queries.push_back(NULL);
	keyIndexes.push_back(NULL);
	qwordIDMap.set_empty_key("");
	qwordFreqMap.set_empty_key(0);
	qwordVector.push_back("");

	for (int k = 0; k < 26; k++) dwordCache[k].set_empty_key("");

	permSharedInfo = new SharedInfo();
	tempSharedInfo = new SharedInfoTime();
	permSharedInfo->init();
	tempSharedInfo->init();
	registerStarter([]() {
		permSearcher[workerID].init(permSharedInfo);
		tempSearcher[workerID].init(tempSharedInfo);
	});

	insertTaskID = registerTask(doInsert);
	searchTaskID = registerTask(doSearch);
	calculateTaskID = registerTask(doCalculate);
	combineTaskID = registerTask(doCombine);
	splitTaskID = registerTask(doSplit);

	startThreads();

	return EC_SUCCESS;
}

ErrorCode DestroyIndex()
{
	stopThreads();
	Hash::destroy();
	return EC_SUCCESS;
}

ErrorCode StartQuery(QueryID queryID, const char *queryStr, MatchType matchType, unsigned int matchDist)
{
	realQuery++;
	indexQuery++;
	maxQueryID = max(queryID, maxQueryID);
	queries.push_back(new Query(queryID, queryStr, matchType, matchDist));
	return EC_SUCCESS;
}

ErrorCode EndQuery(QueryID queryID)
{
	realQuery--;
	Query *query = queries[queryID];
	if (!query->isInserted)
	{
		indexQuery--;
	}
	query->isDeleted = true;
	return EC_SUCCESS;
}

ErrorCode MatchDocument(DocID docID, const char *docStr)
{
	static int startID = 1;
	static int rebuildCounter = 0;
	if (!queries.back()->isInserted)
	{
		queryRounds++;
		bool isRebuild = false;
		if (realQuery / (double)indexQuery < 0.8)
		{
			startID = 1;
			isRebuild = true;
			rebuildCounter++;
			lastRebuildRounds = rebuildRounds;
			rebuildRounds = queryRounds;
			indexQuery = realQuery;
			clearAll();

			if (rebuildCounter % 20 == 0)
			{
				lastTempRebuildRounds = queryRounds;
				tempSharedInfo->clear();
			}
		}

		insertQueries(startID, isRebuild);
		startID = queries.size();
	}

	if (docSize == (int)documents.size())
	{
		documents.push_back(new Document());
	}

	documents[docSize++]->init(docID, docStr);
	return EC_SUCCESS;
}

ErrorCode GetNextAvailRes(DocID *docID, unsigned int *numRes, QueryID **queryIDs)
{
	if (docSize != 0)
	{
		// split document
		doTask(splitTaskID, docSize);
		// sort documents
		sort(documents.begin(), documents.begin() + docSize, [](const Document *d1, const Document *d2) {
			return d1->words.size() > d2->words.size();
		});
		// combine document
		distVector.clear();
		for (int k = 0; k < 26; k++)
			distVectorPart[k].clear();
		doTask(combineTaskID, 26);
		for (int k = 0; k < 26; k++)
			distVector.insert(distVector.end(), distVectorPart[k].begin(), distVectorPart[k].end());
		// search document
		doTask(searchTaskID, distVector.size());
		// final calculate
		results.resize(docSize);
		doTask(calculateTaskID, docSize);
		docSize = 0;
	}

	*docID = 0;
	*numRes = 0;
	*queryIDs = 0;
	if (results.empty())
		return EC_NO_AVAIL_RES;
	*docID = results.back().docID;
	*numRes = results.back().numRes;
	*queryIDs = results.back().matchQueryIDs;
	results.erase(results.end() - 1);
	return EC_SUCCESS;
}

