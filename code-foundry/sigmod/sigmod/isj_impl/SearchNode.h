//
//  SearchNode.h
//  sigmod
//
//  Created by jonathan on 03/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#ifndef __sigmod__SearchNode__
#define __sigmod__SearchNode__

#include <iostream>
#include "core.h"
#include "sigmod_types.h"


class SearchNode {
private:
    char* _word;
    Match _matchType;
    SearchNode* parentNode;
    int* _childNodes;

    
public:
    SearchNode (const char* word);
    ~SearchNode();

    /**
     *  add a query to the correct query array
     *
     *  @param queryID       id number of query
     *  @param matchType     exact, hamming or edit (0,1,2)
     *  @param MatchDistance 0 | 1 | 2
     */
    void addQuery (int queryID, int matchType, int MatchDistance);
    
};

#endif /* defined(__sigmod__SearchNode__) */
