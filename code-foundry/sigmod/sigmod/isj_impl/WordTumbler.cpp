//
//  WordTumbler.cpp
//  sigmod
//
//  Created by jonathan on 03/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#include "WordTumbler.h"
#include "SearchTree.h"
using namespace  std;

static void start_tumbling(char* doc_word,  int* doc_results);


WordTumbler::WordTumbler (char* doc_word,  int* doc_results) {

    start_tumbling(doc_word,doc_results);
}

WordTumbler::~WordTumbler(){
   //nil any variables;
}

 void start_tumbling(char* doc_word,  int* doc_results) {
    /*
     search through the tree for matches to _word
     maybe we have to do three searches, one for each matchType?

     when we get a match we call decrementQueryID on docResult for each queryID that contains the match word/matchType.
     */
    
}