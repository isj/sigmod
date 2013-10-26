#include "fuzzy.h"

using namespace std;

IndexTime::IndexTime()
{
	invList = NULL;
}

IndexTime::~IndexTime()
{
	if (invList) free2d(invList);
}

void IndexTime::init(int threshold)
{
	maxID = 0;
	D = threshold;
	if (D != 0)
	{
		PN = D + 1;
		partLen = Index::partLens[D];
		partPos = Index::partPoses[D];
		partIndex = Index::partIndexes[D];
		alloc2d(invList, PN, SAFELEN);
		for (int i = 0; i < PN; i++)
			for (int j = 0; j < SAFELEN; j++)
				invList[i][j].set_empty_key(0);
	} else {
		index0.set_empty_key("");
	}
}

void IndexTime::insert(int id, const char *record, int time)
{
	maxID = max(id, maxID);
	if (D == 0)
	{
		insert0Internal(id, record, time);
	} else {
		insertInternal(id, record, time);
	}
}

void IndexTime::insert0Internal(int id, const char *record, int time)
{
	index0[record] = Data(id, time);
}

void IndexTime::insertInternal(int id, const char *record, int time)
{
	int len = strlen(record);
	for (int partId = 0; partId < PN; partId++)
	{
		int pLen = partLen[partId][len];
		int stPos = partPos[partId][len];
		invList[partId][len][Hash::getHash(record, stPos, pLen)].emplace_back(id, time);
	}
}

void IndexTime::clear()
{
	index0.clear();
	for (int i = 0; i < PN; i++)
		for (int j = 0; j < SAFELEN; j++)
			invList[i][j].clear();
}

