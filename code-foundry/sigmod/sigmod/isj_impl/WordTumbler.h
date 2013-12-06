//
//  WordTumbler.h
//  sigmod
//
//  Created by jonathan on 03/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#ifndef __sigmod__WordTumbler__
#define __sigmod__WordTumbler__

#include <iostream>
#include "core.h"

class SearchTree;
/**
 *  we create one of these for each new document word....
 */
class WordTumbler {
private:
    char* _doc_word;
    int _word_length;
    int* _doc_results;
    DocID _doc_id;
    const char* _doc_str;
    int _doc_str_idx;
    void start_tumbling();


public:
    WordTumbler (DocID doc_id, const char* doc_str ,  int doc_str_idx);
    ~WordTumbler();

    ErrorCode tring();


};

#endif /* defined(__sigmod__WordTumbler__) */
