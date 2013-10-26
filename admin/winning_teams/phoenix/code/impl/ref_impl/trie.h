
#ifndef __SIGMOD_TRIE_H_
#define __SIGMOD_TRIE_H_

#include "word.h"
#include <pthread.h>
#include <vector>

struct TermData {
	Word*	word;
	uint 	detail;
};

struct TrieTree {
	TrieTree * child[26];
	std::vector<TermData> list;
	int buf;
};

void initTrie();

TrieTree * initTrieNode();

void insertTrie(TrieTree* tree, const char* word_str, Word* word);

void deleteTrie(TrieTree* tree, const char* word_str, Word* word);

#endif // __SIGMOD_TRIE_H_

