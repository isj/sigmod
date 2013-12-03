//
//  SearchTree.h
//  sigmod
//
//  Created by jonathan on 03/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#ifndef __sigmod__SearchTree__
#define __sigmod__SearchTree__

#include <iostream>
#include "SearchNode.h"
#include "core.h"



class SearchTree {
private:
    //singleton pattern, we only ever want ONE searchTree.

    /**
     *  useage
     *  SearchNode::Instance()->addQuery(...);
     */

    SearchNode* _root;  //class SearchNode

    SearchTree (){};
    SearchTree(SearchTree const&){};             // copy constructor is private
    //SearchTree& operator=(SearchTree const&){};  // assignment operator is private
    static SearchTree* m_pInstance;
public:
    static SearchTree* Instance();


    /**
     *  addQuery
     *  add a new query search term to the tree
     *  this will add a new node if the search term does not already exist
     *  otherwise it will add the queryID, matchType and matchDistance to an existing node
     *
     *  rearrange tree if necessary
     *
     */
    ErrorCode addQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist);

    /**
     *  matchWord
     *  should this be inside the class
     *  or external? 
     */
    ErrorCode matchWord(DocID doc_id, const char* doc_word);




    
    
};

#endif /* defined(__sigmod__SearchTree__) */

