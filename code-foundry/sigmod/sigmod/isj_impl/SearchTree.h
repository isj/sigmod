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
#include <unordered_set>  //WARNING - unordered_set MAY BE PLATFORM-SPECIFIC
#include "SearchNode.h"
#include "core.h"
#include "sigmod_types.h"

class WordTumbler;

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


    /**
     *  set of query_ids... (this is a hash table)
     *
     * Internally, the elements in the unordered_set are not 
     * sorted in any particular order, but organized into buckets depending 
     * on their hash values to allow for fast access to individual elements 
     * directly by their values (with a constant average time complexity on average).

     * unordered_set containers are faster than set containers to access individual 
     * elements by their key, although they are generally less efficient for range 
     * iteration through a subset of their elements.
     * http://www.cplusplus.com/reference/unordered_set/unordered_set/
     */

    std::unordered_set <QueryID>* _query_ids;

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



    ErrorCode addQuery(  QueryID query_id
                       , const char* query_str
                       , MatchType match_type
                       , unsigned int match_dist
                       , int query_str_idx
                       );


    ErrorCode addDocument(  DocID doc_id
                          , const char* doc_str
                          , int doc_str_idx
                          );

    ErrorCode matchWord  (  DocID doc_id
                          , const char* doc_str
                          , unsigned int word_start_idx
                          , unsigned int word_length
                          );





    ErrorCode tumbleWord(WordTumbler* wordTumbler);

    ErrorCode startMatching (  DocID doc_id
                             , const char* doc_start
                             , int doc_results[]
                             );
    
    void print ();

    /**
     *  Query accounting
     *
     */

    int  numberOfQueries();
    bool isValidQuery  (QueryID query_id);
    void removeQuery   (QueryID query_id);
};

#endif /* defined(__sigmod__SearchTree__) */

