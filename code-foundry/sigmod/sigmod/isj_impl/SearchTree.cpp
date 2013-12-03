//
//  SearchTree.cpp
//  sigmod
//
//  Created by jonathan on 03/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#include "SearchTree.h"
using namespace std;

// Global static pointer used to ensure a single instance of the class.

SearchTree* SearchTree::m_pInstance = NULL;

/**     This function is called to create an instance of the class.
 *	    Calling the constructor publicly is not allowed. The constructor
 *	    is private and is only called by this Instance function.
 */

SearchTree* SearchTree::Instance()
{
    if (!m_pInstance)   // Only allow one instance of class to be generated.
        m_pInstance = new SearchTree;
    return m_pInstance;
}


SearchTree::SearchTree() {
    //constructor
    //create root node
     _root = new SearchNode("");
 }


ErrorCode SearchTree::addQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist) {
    return EC_SUCCESS;
}

ErrorCode matchWord(DocID doc_id, const char* doc_word) {
    return EC_SUCCESS;
}

