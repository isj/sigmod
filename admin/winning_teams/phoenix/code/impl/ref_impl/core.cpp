#include "../include/core.h"
#include "trie.h"
#include "word.h"
#include "threadspool.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <deque>
#include <algorithm>
#include <string>
#include <ctime>
#include <pthread.h>
#include <semaphore.h>
#include <set>
using namespace std;

extern uint tableIndex[32][32][32];
extern int matchTable[6000][6000];
extern int totalTrieNode;

int onesCount[1<<6];

TrieTree * trie;

struct Document
{
	DocID doc_id;
	unsigned int num_res;
	QueryID* query_ids;
};

vector<Document> docs;

pthread_mutex_t mut_document;

bool operator < (const MatchResult& a, const MatchResult& b)
{
	long pa = (long)a.node;
	long pb = (long)b.node;
	if (pa != pb)
		return pa < pb;
	else
		return a.match_index < b.match_index;
}

struct MatchArgument
{
	DocID 		doc_id;
	char 		doc_str[MAX_DOC_LENGTH+1];
} argpool[48];

int pool_count = 0;

/******************************* Word-Matching ***************************************/

void Search(TrieTree* tree, const char* word_str, int now, int len, int* del, ThreadPackage*package)
{
	if (word_str[len] == 0){
		if ((tree->list).size() == 0) return;	
		package->matches[package->tot].node = tree;
		package->matches[package->tot].match_index = tableIndex[del[0]][del[1]][del[2]];
		++(package->tot);
	}
	else
	{
		if (now < 3) {
			del[now] = len + 1;
			Search(tree, word_str, now + 1, len + 1, del, package);
			del[now] = 0;
		}
		int id = word_str[len] - 'a';
		if (tree->child[id] != NULL)
			Search(tree->child[id], word_str, now, len + 1, del, package);
		
	}
}

inline void MatchWord(const char* word_str, TrieTree* tree, ThreadPackage* package)
{
	int del[3];
	memset(del, 0, sizeof(del));
	Search(trie, word_str, 0, 0, del, package);
}

void DFS(int node, int lev, char* word, ThreadPackage* package)
{
	if (package->isWord[node]) {
		word[lev] = 0;
		MatchWord(word, trie, package);
	}
	for (int i = 0; i < package->tree_t[node]; ++i) {
		word[lev] = (char)((package->tree[node][i] & 31) + 'a');
		DFS((package->tree[node][i] >> 5), lev+1, word, package);
	}
}

void BuildTree(MatchArgument* arg, ThreadPackage* package)
{
	int tot = 0;
	package->isWord[tot] = false;
	package->tree_t[tot] = 0;
	const char *p = arg->doc_str;
	while ((*p)!=0) {
		while ((*p)!=0 && (*p)==' ') ++p;
		if ((*p)==0) break;
		int node = 0;
		while (*p!=0 && *p!=' '){
			int id = *p - 'a';
			int exist = package->tree_t[node];
			int next = -1;
			for (int i = 0; i < exist; ++i)
				if (id == (package->tree[node][i] & 31)){
					next = (package->tree[node][i] >> 5);
					break;
				}
			if (next == -1){
				++tot;
				next = tot;
				package->isWord[tot] = false;
				package->tree_t[tot] = 0;
				package->tree_t[node] = exist + 1;
				package->tree[node][exist] = (tot << 5) + id;
			}
			node = next; 
			++p;
		}		
		package->isWord[node] = true;
	}
}

inline void SearchTree(ThreadPackage* package)
{
	package->tot = 0;
	char word[MAX_WORD_LENGTH+1];
	DFS(0, 0, word, package);
}

void *MatchDocumentThread(void* _arg, void* _package)
{
	MatchArgument* arg = (MatchArgument*)_arg;
	ThreadPackage* package = (ThreadPackage*)_package;
	
	BuildTree(arg, package);

	SearchTree(package);

	vector<uint> query_ids;

	uint detail;
	sort(package->matches, package->matches + package->tot);

	Query* querys[1000000];
	int t_querys = 0;

	uint index_list[10000];
	int index_count;

	bool mark[6000];
	memset(mark, 0, sizeof(mark));
	for (int i = 0; i < package->tot;)
	{
		int j = i;
		index_count = 0;
		for (j = i; j < package->tot && (package->matches[i]).node == (package->matches[j]).node; ++j)
		{
			if (index_count == 0 || index_list[index_count - 1] != (package->matches[j]).match_index){
				index_list[index_count++] = (package->matches[j]).match_index;
				mark[index_list[index_count - 1]] = true;
			}
		}
		vector<TermData>::iterator end_iter = ((package->matches[i]).node->list).end();
		for (vector<TermData>::iterator iter_data = ((package->matches[i]).node->list).begin();
				iter_data != end_iter;
				++iter_data)
		{
			Word* word = iter_data->word;
			Query* query = (Query*)word->query;
			detail = iter_data->detail;
		
			if (0 != (query->mark[package->id] & (1 << word->id))) 
				continue;
	
			bool flag = false;
			if (query->dist < 4){
				flag = mark[detail];
			}
			else
				for (int i = 0; i < index_count; ++i)
					if (matchTable[index_list[i]][detail] != -1
					&& (matchTable[index_list[i]][detail] & 3) <= (query->dist & 3))
				{
					flag = true;
					break;
				}
			if (flag)
			{
				Word* p = word;
				while (p!=NULL){
					if (((Query*)p->query)->mark[package->id] == 0)
						querys[t_querys++] = (Query*)p->query;
					((Query*)(p->query))->mark[package->id] |= (1 << p->id);
					p = p->next;
				}
			
			}
		}
		for (int k = 0; k < index_count; ++k) mark[index_list[k]] = false;
		i = j;
	}
	for (int i = 0; i < t_querys; ++i)
	{
		if (onesCount[(int)querys[i]->mark[package->id]] == querys[i]->t_word)
			query_ids.push_back(querys[i]->query_id);
		querys[i]->mark[package->id] = 0;
	}
	sort(query_ids.begin(), query_ids.end());
 	
	Document doc;
	doc.doc_id = arg->doc_id;
	doc.num_res = query_ids.size();
	doc.query_ids = 0;
	if (doc.num_res != 0)
		doc.query_ids = (unsigned int*)malloc(doc.num_res*sizeof(unsigned int));
	for (unsigned int i = 0; i < doc.num_res; ++i)
		doc.query_ids[i] = query_ids[i];
	
	pthread_mutex_lock(&mut_document);
	docs.push_back(doc);
	pthread_mutex_unlock(&mut_document);

	return NULL;
}

/****************************** The End of Word-Matching ****************************/

/****************************** Main API Implementation *****************************/

ErrorCode InitializeIndex()
{
	for (int i = 0; i < (1<<6); ++i){
		int k = i;
		onesCount[i] = 0;
		while (k!=0){
			onesCount[i] += (k&1);	
			k>>=1;
		}
	}
	pthread_mutex_init(&mut_document, NULL);
	InitThreadspool();	
	initTrie();

	PrepareMatchTable();
	initQueryHash();
	trie = initTrieNode();
	return EC_SUCCESS;
}

ErrorCode DestroyIndex()
{
	DestroyThreadspool();
	return EC_SUCCESS;
}

ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist)
{
	if (pool_count != 0){
		AllJobDone();
		pool_count = 0;
	}

	Query* query = initQuery(query_id, match_type, match_dist);
	
	set<string> words;
	int word_id = 0;
	const char* p = query_str;
	while ((*p)!=0) {
		while ((*p)!=0 && (*p)==' ') ++p;
		if ((*p)==0) break;
		const char* q = p;
		while ((*q)!=0 && (*q)!=' ') ++q;
	
		Word* word = &(query->word_list[word_id]);
		memcpy(word->word_str, p, q - p);
		word->word_str[q-p] = 0;
		word->id = word_id;
		word->query = (void*)query;
		word->next = NULL;
		insertTrie(trie, word->word_str, word);
		++word_id;

		p = q;
	}
	query->t_word = word_id;
	return EC_SUCCESS;
}

ErrorCode EndQuery(QueryID query_id)
{
	if (pool_count != 0){
		AllJobDone();
		pool_count = 0;
	}
	Query* query = findQuery(query_id);
	if (query != NULL){
		for (int i = 0; i < query->t_word; ++i)
		{
			deleteTrie(trie, (query->word_list[i]).word_str, &(query->word_list[i]));
		}
		deleteQuery(query_id);
	}
	return EC_SUCCESS;
}


ErrorCode MatchDocument(DocID doc_id, const char* doc_str)
{
	if (pool_count == 48){
		AllJobDone();
		pool_count = 0;
	}
	MatchArgument* marg = &argpool[pool_count++];

	marg->doc_id = doc_id;
	memcpy(marg->doc_str, doc_str, strlen(doc_str) + 1);
	
	Job* job = new Job;
	job->f = MatchDocumentThread;
	job->arg = (void*)marg;

	AddJob(job);
	
	return EC_SUCCESS;
}

ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids)
{
	if (pool_count != 0){
		AllJobDone();
		pool_count = 0;
	}

	*p_doc_id=0; 
	*p_num_res=0; 
	*p_query_ids=0;
	if(docs.size()==0) 
		return EC_NO_AVAIL_RES;
	
	pthread_mutex_lock(&mut_document);
	*p_doc_id=docs[0].doc_id; 
	*p_num_res=docs[0].num_res; 
	*p_query_ids=docs[0].query_ids;
	docs.erase(docs.begin());
	pthread_mutex_unlock(&mut_document);
	
	return EC_SUCCESS;
}

/********************************************************************************************/
