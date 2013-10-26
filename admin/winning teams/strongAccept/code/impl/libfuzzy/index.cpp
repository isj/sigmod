#include "fuzzy.h"

using namespace std;

vector<Part> **Index::partIndexes[MAXED];
int **Index::partLens[MAXED], **Index::partPoses[MAXED];

void Index::init()
{
	for (int D = 0; D < MAXED; D++)
	{
		int PN = D + 1;
		auto &partLen = partLens[D];
		auto &partIndex = partIndexes[D];
		auto &partPos = partPoses[D];

		alloc2d(partLen, PN, SAFELEN);
		alloc2d(partIndex, PN, SAFELEN);
		alloc2d(partPos, PN + 1, SAFELEN);

		// init the information of the first and last segment
		for (int len = 0; len <= MAXLEN; len++)
		{
			partPos[0][len] = 0;
			partLen[0][len] = len / PN;
			partPos[PN][len] = len;
		}

		// fill int the information of middle segments
		for (int pid = 1; pid < PN; pid++)
		{
			for (int len = 0; len <= MAXLEN; len++)
			{
				partPos[pid][len] = partPos[pid - 1][len] + partLen[pid - 1][len];
				if (pid == (PN - len % PN))
					partLen[pid][len] = partLen[pid - 1][len] + 1;
				else
					partLen[pid][len] = partLen[pid - 1][len];
			}
		}

		// generate the set of pairs of segment and substring to be check
		for (int currentLen = 0; currentLen < MAXLEN; currentLen++)
		{
			for (int pid = 0; pid < PN; pid++)
			{
				// enumerate all lengths of string which are potential similar to current string
				for (int len = max(currentLen - D, 0); len <= min(currentLen + D, MAXLEN); len++)
				{
					// enumerate the valid substrings with respect to currentLen, len and pid
					for (int stPos = max3(0, partPos[pid][len] - pid,
								partPos[pid][len] + (currentLen - len) - (D - pid));
							stPos <= min3(currentLen - partLen[pid][len], partPos[pid][len] + pid,
								partPos[pid][len] + (currentLen - len) + (D - pid)); stPos++)
						partIndex[pid][currentLen].emplace_back(
									stPos, partPos[pid][len], partLen[pid][len], len);
				}
				// sort the pairs of segment and substring to be check,
				// for conveniently hash substring and segment
				sort(partIndex[pid][currentLen].begin(), partIndex[pid][currentLen].end(), [](const Part &p1, const Part & p2)
						{
						if (p1.stPos + p1.partLen < p2.stPos + p2.partLen) return true;
						if (p1.stPos + p1.partLen > p2.stPos + p2.partLen) return false;
						if (p1.stPos < p2.stPos) return true;
						if (p1.stPos > p2.stPos) return false;
						return false;
						});
			}
		}

		// if (PN == 4)
		// {
		// 	for (int clen = 4; clen < 8; clen++)
		// 	{
		// 		printf("~~~~~~~~~~~ Current Length : %d ~~~~~~~~~~~\n", clen);
		// 		for (int pid = 0; pid < PN; pid++) {
		// 			printf("    #### Part Id : %d ####\n", pid);
		// 			for (unsigned lp = 0; lp < partIndex[pid][clen].size(); lp++)
		// 				printf("stPos: %d\tendPos: %d\tpartLen: %d\tLo: %d\tlen: %d\n",
		// 						partIndex[pid][clen][lp].stPos,
		// 						partIndex[pid][clen][lp].stPos + partIndex[pid][clen][lp].partLen,
		// 						partIndex[pid][clen][lp].partLen,
		// 						partIndex[pid][clen][lp].Lo,
		// 						partIndex[pid][clen][lp].len);
		// 		}
		// 	}
		// }
	}
}

Index::Index()
{
	invList = NULL;
}

Index::~Index()
{
	if (invList) free2d(invList);
}

void Index::init(int threshold)
{
	maxID = 0;
	D = threshold;
	if (D != 0)
	{
		PN = D + 1;
		partLen = partLens[D];
		partPos = partPoses[D];
		partIndex = partIndexes[D];
		alloc2d(invList, PN, SAFELEN);
		for (int i = 0; i < PN; i++)
			for (int j = 0; j < SAFELEN; j++)
				invList[i][j].set_empty_key(0);
	} else {
		index0.set_empty_key("");
	}
}

void Index::insert(int id, const char *record)
{
	maxID = max(id, maxID);
	if (D == 0)
	{
		insert0Internal(id, record);
	} else {
		insertInternal(id, record);
	}
}

void Index::insert0Internal(int id, const char *record)
{
	index0[record] = id;
}

void Index::insertInternal(int id, const char *record)
{
	int len = strlen(record);
	for (int partId = 0; partId < PN; partId++)
	{
		int pLen = partLen[partId][len];
		int stPos = partPos[partId][len];
		invList[partId][len][Hash::getHash(record, stPos, pLen)].push_back(id);
	}
}

void Index::clear()
{
	index0.clear();
	for (int i = 0; i < PN; i++)
		for (int j = 0; j < SAFELEN; j++)
			invList[i][j].clear();
}

