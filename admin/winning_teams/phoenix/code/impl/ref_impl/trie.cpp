#include "../include/core.h"
#include "trie.h"
#include <cstdio>
#include <cstring>
#include <vector>
#include <deque>
using namespace std;

extern int tableIndex[32][32][32];

long totalTrieNode = 0;

const long MAX_TREE_BUF = 30 * 1024 *  1024 / sizeof(TrieTree) * 1024;

TrieTree *treeBuf;

uint *bufQueue;
uint emptyBuf = 0;

void initTrie()
{
	treeBuf = (TrieTree*)malloc(MAX_TREE_BUF * sizeof(TrieTree));
	bufQueue = (uint*)malloc(MAX_TREE_BUF * sizeof(uint));
}

TrieTree * initTrieNode()
{
	TrieTree * node;
	if (totalTrieNode < MAX_TREE_BUF){
		node = &treeBuf[totalTrieNode];	
		node->buf = totalTrieNode;
	}
	else
	{
		if (emptyBuf > 0){
			int id = bufQueue[emptyBuf - 1];
			--emptyBuf;
			node = &treeBuf[id];
			node->buf = id;
		}
		else
		{
			node = new TrieTree;
			node->buf = -1;
		}
	}
	totalTrieNode++;
	memset(node->child, 0, sizeof(node->child));
	(node->list).clear();
	return node;
}

bool searchInsert(TrieTree* 	tree,
					const char* word_str,
					uint 		now,
					int 		len,
					int*		del,
					Word*		word)
{
	uint dist = ((Query*)word->query)->dist;
	if (*word_str == 0) {
		if (dist < 4 && now != 0 && now != dist)
			return true;
		if (now == 0){
			for (vector<TermData>::iterator iter = (tree->list).begin();
				iter != (tree->list).end();
				++iter)
			{
				if (iter->detail == 0 
					&& ((Query*)(iter->word->query))->dist  == dist)
				{
					word->next = iter->word->next;
					iter->word->next = word;
					return false;
				}
			} 
		}
		TermData data;
		data.word = word;
		data.detail = tableIndex[del[0]][del[1]][del[2]];
		(tree->list).push_back(data);
		return true;
	}
	else
	{
		bool ret = true;
		int id = *word_str - 'a';
		if (tree->child[id] == NULL){
			tree->child[id] = initTrieNode();
		}
		ret = searchInsert(tree->child[id], word_str + 1, now, len + 1,
			del, word);
		
		if (!ret) return false;
	
		if (now < (dist & 3)){
			del[now] = len + 1;
			searchInsert(tree, word_str + 1, now + 1, len + 1, 
				del, word);
			del[now] = 0;
		}
		return true;
	}
}

void insertTrie(TrieTree* tree, const char* word_str, Word* word)
{
	int del[3];
	memset(del, 0, sizeof(del));
	searchInsert(tree, word_str, 0, 0, del, word);
}

bool emptyNode(TrieTree* tree)
{
	if ((tree->list).size() > 0) return false;
	for (int i = 0; i < 26; ++i)
		if (tree->child[i] != NULL) return false;
	return true;
		
}

bool searchDelete(TrieTree* tree, const char* word_str, uint now, Word* word, bool &empty)
{
	uint dist = ((Query*)word->query)->dist;
	if (*word_str == 0) {
		if (dist < 4 && now != 0 && now != dist)
			return true;
		vector<TermData>::iterator iter;
		for (iter = (tree->list).begin(); 
				iter != (tree->list).end();)
		{
			if (iter->word == word){
				iter->word = iter->word->next;
			}
			else if (now == 0 
				&& iter->detail == 0 
				&& ((Query*)(iter->word->query))->dist == dist){
				Word *p, *q;
				p = NULL; q = iter->word;
				while (q!=NULL && q!=word){
					p = q; q = q->next;
				}
				if (q!= NULL){
					p->next = q->next;
				}
				else
					printf("ERROR!\n");
				return false;
			}
			if (iter->word == NULL){
				vector<TermData>::iterator end_iter = (tree->list).end() - 1;
				*iter = *end_iter;
				(tree->list).pop_back();
			}	
			else
				++iter;
		}
		if (emptyNode(tree))
			empty = true;
		else
			empty = false;
		return true;
	}
	else
	{
		bool ret = true;
	
		int id = *word_str - 'a';
		if (tree->child[id] != NULL) {
			ret = searchDelete(tree->child[id], word_str + 1, now, word, empty);
			if (!ret) return false;
			if (empty){
				if (-1 == tree->child[id]->buf)
					delete tree->child[id];
				else{
					bufQueue[emptyBuf++] = tree->child[id]->buf;
				}
				tree->child[id] = NULL;
				if (emptyNode(tree))
					empty = true;
				else
					empty = false;
			}
		}
		if (!empty && now < (dist & 3)) {
			ret = searchDelete(tree, word_str + 1, now + 1, word, empty);
		}
		return true;
	}
}

void deleteTrie(TrieTree* tree, const char* word_str, Word* word)
{
	bool empty = false;
	searchDelete(tree, word_str, 0, word, empty);
}

