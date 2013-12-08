//
//  SearchTree.cpp
//  sigmod
//
//  Created by jonathan on 03/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#include "SearchTree.h"
#include "sigmod_types.h"

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
    if (LOG) printf("%s\n",__func__);
     _root = new SearchNode();  //heap
#ifdef UNORDERED_SET
    _query_ids = new unordered_set<QueryID>();
#else
    _query_ids =  new set<QueryID>();
    //* SingleDocResultSet setResult;
    //* _docResultsMap[0].insert(setResult);
#endif

    _query_ids_map = new map<QueryID, unsigned  int>();
    // SingleDocResultSet setResult;
    // _query_ids_map[0].insert(0);
 }


ErrorCode SearchTree::tumbleWord  ( WordTumbler* wordTumbler
                       ) {
    if (LOG) printf("%s\n",__func__);
    return EC_SUCCESS;
}

ErrorCode SearchTree::addQuery (  QueryID query_id
                                , const char* query_str
                                , MatchType match_type
                                , unsigned int match_dist
                                , unsigned int query_str_idx
                                , unsigned int query_word_counter
                                ) {
    if (LOG) printf("%s\n",__func__);
    _query_ids->insert(query_id);

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

ErrorCode SearchTree::addDocument  (  DocID doc_id
                                   , const char* doc_str
                                   , int doc_str_idx
                                   ) {


    _root->addDocument(  doc_id
                       , doc_str
                       , doc_str_idx
                       );
    return EC_SUCCESS;
}

ErrorCode SearchTree::matchWord  (  DocID doc_id
                                  , const char* doc_str
                                  , unsigned int word_start_idx
                                  , unsigned int word_length
                                  ) {
    //if (LOG) printf("%s\n",__func__);
    if (LOG) printf("\"");
    //for (int idx=0; idx<word_length; idx++) {
        //if (LOG)  printf("%c",doc_str[word_start_idx+idx]);
    //}
    if (LOG) printf("\" ");
    _root -> matchWord (  doc_id
                        , doc_str
                        , word_start_idx
                        , word_length
                        );
    return EC_SUCCESS;

}

ErrorCode SearchTree::startMatching (  DocID doc_id
                        , const char* doc_start
                        , int doc_results[]
                        ) {
    return EC_SUCCESS;
}


#pragma mark - printing

void SearchTree::print() {
    printf("\nprinting searchTree...\n");
    _root -> print();
    printf("\n");

}

#pragma mark - query accounting

void SearchTree::addQueryToMap (  QueryID query_id
                    , unsigned int word_count
                    ) {
    _query_ids_map->insert ( std::pair<QueryID,unsigned int>(query_id,word_count) );
}

int  SearchTree::numberOfQueries() {
    return (int)_query_ids->size();
}

#pragma mark - query validation

void SearchTree::removeQuery   (QueryID query_id) {
    _query_ids->erase(query_id);
}

bool SearchTree::isValidQuery(QueryID query_id) {
    if ( _query_ids->count(query_id)==1) return true;
    else return false;
}


