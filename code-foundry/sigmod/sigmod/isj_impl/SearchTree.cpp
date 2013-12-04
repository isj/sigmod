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
    printf("%s\n",__func__);

    if (!m_pInstance)   // Only allow one instance of class to be generated.
        m_pInstance = new SearchTree;
    return m_pInstance;
}


SearchTree::SearchTree() {
    //constructor
    //create root node
    printf("%s\n",__func__);
     _root = new SearchNode();
 }


ErrorCode SearchTree::addQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist, int query_str_idx) {
    printf("%s\n",__func__);
    
    _root->addQuery(query_id, query_str, match_type, match_dist, query_str_idx);

    return EC_SUCCESS;
}

ErrorCode matchWord(DocID doc_id, const char* doc_word) {
    printf("%s\n",__func__);

    return EC_SUCCESS;
}

