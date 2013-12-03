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

/**
 *  we create one of these for each new document word....
 */
class WordTumbler {
private:

public:
    WordTumbler (char* doc_word,  int* doc_results);
    ~WordTumbler();
};

#endif /* defined(__sigmod__WordTumbler__) */
