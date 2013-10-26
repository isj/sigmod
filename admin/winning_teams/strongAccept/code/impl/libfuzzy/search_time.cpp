#include "fuzzy.h"

extern int queryRounds;
using namespace std;

void SharedInfoTime::init()
{
	for (int dist = 0; dist < MAXED; dist++)
	{
		edIndexes[dist].init(dist);
		hmIndexes[dist].init(dist);
	}
}

void SharedInfoTime::clear()
{
	for (int dist = 0; dist < MAXED; dist++)
	{
		edIndexes[dist].clear();
		hmIndexes[dist].clear();
	}
}

void SharedInfoTime::insertED(int ed, int id, const char *record, int time)
{
	edIndexes[ed].insert(id, record, time);
}

void SharedInfoTime::insertHM(int hm, int id, const char *record, int time)
{
	hmIndexes[hm].insert(id, record, time);
}

void SearchTime::init(SharedInfoTime *sharedInfo)
{
	quickRefSize = 1e7;
	quickRef = new bool[quickRefSize];
	memset(quickRef, 0, quickRefSize);
	this->sharedInfo = sharedInfo;
	for (int k = 0; k < SAFELEN; k++)
	{
		matrix[k][0] = _mm_set1_epi8(k);
		matrix[0][k] = _mm_set1_epi8(k);
	}
}

void SearchTime::searchHM(int time, const vector<string> &dict, int hm, const char *query, vector<pair<int, int>> &res)
{
	D = hm;
	index = &(sharedInfo->hmIndexes[D]);

	if (D == 0)
	{
		auto iter = index->index0.find(query);
		if (iter != index->index0.end() && iter->second.time >= time)
		{
			res.emplace_back(iter->second.id, 0);
		}
		return;
	}

	checkQuickRefID(index);

	int id[16];
	int result[16];
	int count = 0;

	int qlen = strlen(query);
	hash.initQuery(query, qlen);
	for (int partId = 0; partId < index->PN; partId++)
	{
		int len = index->partLen[partId][qlen];
		int pos = index->partPos[partId][qlen];
		auto value = hash.getQueryValue(pos, pos + len);
		auto inv_it = index->invList[partId][qlen].find(value);
		if (inv_it == index->invList[partId][qlen].end()) continue;

		// enumerate all element in inverted list
		for (auto vit = inv_it->second.rbegin(); vit != inv_it->second.rend(); vit++)
		{
			if (vit->time < time) break;
			const char *candidate = dict[vit->id].c_str();
			for (int k = 0; k < qlen; k++)
				buffer[k][count] = candidate[k];
			id[count] = vit->id;

			if (++count == 16)
			{
				getHM((__m128i*)buffer, query, qlen, result, count, D);
				for (int k = 0; k < 16; k++)
				{
					if (result[k] <= D)
					{
						res.emplace_back(id[k], result[k]);
					}
				}
				count = 0;
			}
		}
	}

	if (count != 0)
	{
		getHM((__m128i*)buffer, query, qlen, result, count, D);
		for (int k = 0; k < count; k++)
		{
			if (result[k] <= D)
			{
				res.emplace_back(id[k], result[k]);
			}
		}
	}
}

void SearchTime::searchED(int time, const vector<string> &dict, int ed, const char *query, vector<pair<int, int>> &res)
{
	D = ed;
	index = &(sharedInfo->edIndexes[D]);

	if (D == 0)
	{
		auto iter = index->index0.find(query);
		if (iter != index->index0.end() && iter->second.time >= time)
		{
			res.emplace_back(iter->second.id, 0);
		}
		return;
	}

	checkQuickRefID(index);

	int id[16];
	int result[16];
	int length[16];

	int qlen = strlen(query);
	hash.initQuery(query, qlen);
	auto &partIndex = index->partIndex;

	int count = 0;
	for (int partId = 0; partId < index->PN; partId++)
	{
		for (unsigned lp = 0; lp < partIndex[partId][qlen].size(); lp++)
		{
			int stPos = partIndex[partId][qlen][lp].stPos;
			int plen = partIndex[partId][qlen][lp].partLen;
			int clen = partIndex[partId][qlen][lp].len;

			auto value = hash.getQueryValue(stPos, stPos + plen);
			auto inv_it = index->invList[partId][clen].find(value);
			if (inv_it == index->invList[partId][clen].end()) continue;

			// enumerate all element in inverted list
			for (auto vit = inv_it->second.rbegin(); vit != inv_it->second.rend(); vit++)
			{
				if (vit->time < time) break;
				if (!quickRef[vit->id])
				{
					quickRef[vit->id] = true;
					testIDs.push_back(vit->id);
				} else {
					continue;
				}

				const char *candidate = dict[vit->id].c_str();
				for (int k = 0; k < clen; k++)
					buffer[k][count] = candidate[k];
				length[count] = clen;
				id[count] = vit->id;

				if (++count == 16)
				{
					getED((__m128i*)buffer, query, length, qlen, result, count, D);
					for (int k = 0; k < 16; k++)
					{
						if (result[k] <= D)
						{
							res.emplace_back(id[k], result[k]);
						}
					}
					count = 0;
				}
			}
		}
	}

	if (count != 0)
	{
		getED((__m128i*)buffer, query, length, qlen, result, count, D);
		for (int k = 0; k < count; k++)
		{
			if (result[k] <= D)
			{
				res.emplace_back(id[k], result[k]);
			}
		}
	}

	for (int id : testIDs)
		quickRef[id] = false;
	testIDs.clear();
}

void SearchTime::getED(__m128i *a, const char *b, int *alen, int blen, int *eds, int count, int D)
{
	__m128i one = _mm_set1_epi8(1);
	for (int i = 1; i <= blen; i++)
	{
		__m128i bcurrent = _mm_set1_epi8(b[i - 1]);
		for(int j = 1; j <= blen + D; j++)
		{
			matrix[i][j] = _mm_add_epi8(matrix[i - 1][j - 1], _mm_cmpeq_epi8(bcurrent, a[j - 1]));
			matrix[i][j] = _mm_min_epi8(matrix[i][j], _mm_min_epi8(matrix[i][j - 1], matrix[i - 1][j]));
			matrix[i][j] = _mm_add_epi8(matrix[i][j], one);
			// matrix[i][j] = _mm_add_epi8(matrix[i - 1][j - 1], _mm_cmpeq_epi8(bcurrent, a[j - 1]));
			// matrix[i][j] = _mm_add_epi8(matrix[i][j], one);
			// matrix[i][j] = _mm_min_epu8(matrix[i][j],
			// 		_mm_add_epi8(_mm_min_epu8(matrix[i][j - 1], matrix[i - 1][j]), one));
		}
	}

	for (int k = 0; k < count; k++)
	{
		eds[k] = ((char*)&(matrix[blen][alen[k]]))[k];
	}
}

void SearchTime::getHM(__m128i *a, const char *b, int len, int *hms, int count, int D)
{
	__m128i result = _mm_set1_epi8(len);
	for (int k = 1; k <= len; k++)
	{
		__m128i bcurrent = _mm_set1_epi8(b[k - 1]);
		result = _mm_add_epi8(result, _mm_cmpeq_epi8(bcurrent, a[k - 1]));
	}

	char *c = (char*)&result;
	for (int k = 0; k < count; k++)
	{
		hms[k] = c[k];
	}
}

void SearchTime::checkQuickRefID(IndexTime *index)
{
	if (index->maxID >= quickRefSize)
	{
		while (index->maxID >= quickRefSize)
			quickRefSize *= 2;
		delete[] quickRef;
		quickRef = new bool[quickRefSize];
		memset(quickRef, 0, quickRefSize);
	}
}

