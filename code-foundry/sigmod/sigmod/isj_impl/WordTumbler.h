//
//  WordTumbler.h
//  sigmod
//
//  Created by jonathan on 22/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#ifndef __sigmod__WordTumbler__
#define __sigmod__WordTumbler__

#include <iostream>
#include "sigmod_types.h"
#include "SearchTree.h"


#endif /* defined(__sigmod__WordTumbler__) */


class WordTumbler {





private:

    typedef struct {
        bool test;
        int cost;
    } ExactMatch;

    typedef struct {
        int current_cost;
        int max_cost;
    } HammingDistance;

    typedef struct {
        int* previous_row;
        int current_cost;
        int max_cost;
    } EditDistance;

    DocID _doc_id;
    SearchNode* _node;
    const char* _doc_word;
    int _doc_word_length;
    ExactMatch _exact;
    HammingDistance _hamming;
    EditDistance _edit;

    void searchChildNodes();
    void testExact();
    void testHamming();
    void testEdit();
    void testResults();
    std::map<char, SearchNode*> mapOfChildNodesToFollow ();
    bool lettersMatch();

    void logResult ();


public:
    WordTumbler(DocID doc_id,  char* word,int word_length, int limit);

    void tumble();







};