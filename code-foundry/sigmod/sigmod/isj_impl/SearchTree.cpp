//
//  SearchTree.cpp
//  sigmod
//
//  Created by jonathan on 03/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#include "SearchTree.h"
#include "sigmod_types.h"
#include "DocResults.h"
#include "sigmod_utils.h"
#include "DataManager.h"

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

     _root = new SearchNode();

 }




ErrorCode SearchTree::addQuery (  QueryID query_id
                                , const char* query_str
                                , MatchType match_type
                                , unsigned int match_dist
                                , unsigned int query_str_idx
                                ,         char query_word_counter
                                ) {
    if (LOGALL) printf("%s\n",__func__);

    //start the word count at 0, we will increment it as we add the query

    _root->addQuery ( query_id
                    , query_str
                    , match_type
                    , match_dist
                    , query_str_idx
                    , query_word_counter
                    );

    return EC_SUCCESS;
}



ErrorCode removeNode (SearchNode* node) {
    if (node->hasChildren()) {
        node->removeTerminator();
    } else {
        node->remove();
    }
    return EC_SUCCESS;
}

#pragma mark - printing

void SearchTree::print() {
    printf("\nprinting searchTree...\n");
    _root -> print();
    printf("\n");

}


#pragma mark - unused




SearchNode* SearchTree::root (){
    return _root;
}

