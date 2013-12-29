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

#ifdef USING_LIBCPP
#endif
//#include <unordered_set>  //WARNING - unordered_set is OSX or BOOST

#include "SearchNode.h"
#include "core.h"
#include "sigmod_types.h"


class SearchTree {
private:
    //singleton pattern, we only ever want ONE searchTree.

    /**
     *  useage
     *  SearchNode::Instance()->addQuery(...);
     */
    SearchNode* _root;  //class SearchNode
    SearchTree ();
    SearchTree(SearchTree const&){};             // copy constructor is private
    //SearchTree& operator=(SearchTree const&){};  // assignment operator is private
    static SearchTree* m_pInstance;






public:
    static SearchTree* Instance();


    ErrorCode addQuery(  QueryID query_id
                       , const char* query_str
                       , MatchType match_type
                       , unsigned int match_dist
                       , unsigned int query_str_idx
                       , unsigned int query_word_counter
                       );

    ErrorCode removeNode (SearchNode* node);

    void print ();



#pragma mark -
    int  numberOfQueries();
    void removeQuery   ( QueryID query_id );
    SearchNode* root();
};

#endif /* defined(__sigmod__SearchTree__) */

