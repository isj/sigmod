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

static void start_tumbling();


WordTumbler::WordTumbler (DocID doc_id, const char* doc_str ,  int doc_str_idx) {
    _doc_id = doc_id;
    _doc_str = doc_str;
    _doc_str_idx = 0;

    start_tumbling();
}

WordTumbler::~WordTumbler(){
   //nil any variables;
}

void WordTumbler::start_tumbling() {
    //iterate through doc_str, sending each word to the search tree
    int start_of_word_idx = 0;
    int end_of_word_idx = 0;
    int doc_str_idx = 0;
    int word_length = 0;
    while (_doc_str[doc_str_idx] != '\0') {
        if (_doc_str[doc_str_idx] == ' ') {//new word
            word_length = doc_str_idx-start_of_word_idx;
            

        } else {
            doc_str_idx++;
        }



    }

}

