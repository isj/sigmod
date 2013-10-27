
#include "word.h"
#include <cstring>
#include <cstdio>
#include <vector>
#include <unordered_map>
#include <pthread.h>
#include <algorithm>

using namespace std;

const int MAX_INDEX = 6000;

uint tableIndex[32][32][32];

int matchTable[MAX_INDEX][MAX_INDEX];

unordered_map<QueryID, Query*> query_hash;

const int MAX_QUERY_BUF = 1000010;
Query queryBuf[MAX_QUERY_BUF];
int bufQueue_q[MAX_QUERY_BUF];
int emptyBuf_q = 0;
 
int optMatch(int la[4], int lb[4])
{
	int l1, l2;
	l1 = la[0];
	l2 = lb[0];
	if (l1 == l2){
		bool same = true;
		for (int i = 1; i <= l1; ++i)
			if (la[i] != lb[i]){
				same = false;
				break;
			}
		if (same)
			return l1;
	}
		
	if (l1 == 0)
		return 4 + l2;
	if (l2 == 0)
		return 4 + l1;
	int f[4][4];
	memset(f, 0, sizeof(f));
	for (int j = 0; j <= l2; ++j)
		f[0][j] = j;
	for (int i = 1; i <= l1; ++i){
		f[i][0] = i;
		for (int j = 1; j <= l2; ++j){
			f[i][j] = f[i-1][j] + 1;
			if (f[i][j] > f[i][j-1] + 1) f[i][j] = f[i][j-1] + 1;
			if (la[i] - i == lb[j] - j && f[i][j] > f[i-1][j-1] + 1) f[i][j] = f[i-1][j-1] + 1; 
			if (f[i][j] > 3) return -1;
		}
	}
	if (f[l1][l2] > 3) return -1; else return 4 + f[l1][l2];
}

inline void AddTable(int i, int j, int k, int&tot, int invert[MAX_INDEX][4])
{
	if (j==0) {
		invert[tot][0] = (i!=0);
		invert[tot][1] = i;
		tableIndex[i][0][0] = tot++;
	}
	else if (k==0){
		invert[tot][0] = 2;
		invert[tot][1] = j; 
		invert[tot][2] = i;
		tableIndex[j][i][0] = tot++;
	}
	else
	{
		invert[tot][0] = 3;
		invert[tot][1] = k;
		invert[tot][2] = j;
		invert[tot][3] = i;
		tableIndex[k][j][i] = tot++;
	}
	
}

void PrepareMatchTable()
{
	int tot = 0;
	int invert[MAX_INDEX][4];
	memset(invert, 0, sizeof(invert));
	memset(tableIndex, 0xFF, sizeof(tableIndex));
	for (int i = 0; i <= MAX_WORD_LENGTH; ++i)
		AddTable(i, 0, 0, tot, invert);
	for (int i = 1; i <= MAX_WORD_LENGTH; ++i)
		for (int j = 1; j < i; ++j)
			AddTable(i, j, 0, tot, invert);
	for (int i = 1; i <= MAX_WORD_LENGTH; ++i)
		for (int j = 1; j < i; ++j)
			for (int k = 1; k < j; ++k)
				AddTable(i, j, k, tot, invert);
	for (int i = 0; i < tot; ++i)
		for (int j = i; j < tot; ++j) {	
			matchTable[i][j] = matchTable[j][i] = optMatch(invert[i], invert[j]);
		}
}

void initQueryHash()
{
	query_hash.clear();
	for (int i = 0; i < MAX_QUERY_BUF; ++i)
		bufQueue_q[emptyBuf_q++] = i;
}

Query* initQuery(QueryID query_id, MatchType match_type, unsigned int match_dist)
{
	Query* q;
	if (emptyBuf_q == 0){
		q = new Query;
		q->buf_id = -1;
	}
	else {
		q = &queryBuf[bufQueue_q[emptyBuf_q - 1]];
		q->buf_id = bufQueue_q[emptyBuf_q - 1];
		emptyBuf_q--;
	}
	q->query_id = query_id;
	q->dist = match_dist;
	memset(q->mark, 0, sizeof(q->mark));
	if (match_type == MT_EDIT_DIST)
		q->dist |= 4;
	pair<QueryID, Query*> item(query_id, q);
	query_hash.insert(item);
	
	return q;
}

Query* findQuery(QueryID query_id) 
{
	unordered_map<QueryID, Query*>::iterator iter = query_hash.find(query_id);
	if (iter!=query_hash.end())
		return iter->second;
	else
		return NULL;
}

void deleteQuery(QueryID query_id)
{
	unordered_map<QueryID, Query*>::iterator iter = query_hash.find(query_id);
	if (iter!=query_hash.end()) {
		Query* query = iter->second;
		if (query->buf_id == -1)
			delete query;	
		else {
			bufQueue_q[emptyBuf_q++] = query->buf_id;
		}
		query_hash.erase(iter);
	}
}

