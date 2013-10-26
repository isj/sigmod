
#ifndef __SIGMOD_WORD_H_
#define __SIGMOD_WORD_H_

#include <vector>
#include <cstring>
#include <string>
#include "../include/core.h"

typedef unsigned int uint;

struct Word {
	char word_str[MAX_WORD_LENGTH + 1];
	char id;
	void* query;

	Word* next;
};

struct Query {
	int buf_id;
	Word word_list[5];
	int t_word;
	QueryID query_id;
	uint	dist;
	char 	mark[24];
};

void PrepareMatchTable();

void initQueryHash(); 

Query* initQuery(QueryID query_id, MatchType match_type, unsigned int match_dist);

Query* findQuery(QueryID query_id);

void deleteQuery(QueryID query_id);

#endif // __SIGMOD_WORD_H_

