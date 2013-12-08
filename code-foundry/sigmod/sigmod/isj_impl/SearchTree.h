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
    #include <unordered_set>  //WARNING - unordered_set is OSX or BOOST
#endif
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

#ifdef USING_LIBCPP
    //OSX-C++ std libary feature only. For Linux we would need to use BOOST
    std::unordered_set < QueryID > * _query_ids;   //OSX-only
#else
    //for Linux let's use an ordered set instead
    std::set < QueryID > * _query_ids;
#endif

    //query ids map
    //we need to store a list of query ids with their associated word count
    //so that we can decrement for any particular document.
    std::map < QueryID, unsigned int > * _query_ids_map;


    //matched words map
    // we keep a list of match_words per document so that we do not match twice
    std::map < DocID, std::set < std::string > > * _matched_words_map;

    //document query count map
    //list of query IDs with word counts
    //we initialise one for each new document search
    //on each found word we decrement the relevant query counts
    //successful query matches are those that decrement to zero.

    std::map <DocID, std::map < QueryID, unsigned int >  > * _query_ids_map_per_document;






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
                       , unsigned int query_str_idx
                       , unsigned int query_word_counter
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


    void addQueryToMap (  QueryID query_id
                        , unsigned int word_count
                        );

    void removeQueryFromMap  (  QueryID query_id );

    void decrementQueryInDocumentMap (  DocID doc_id
                                      , QueryID query_id
                                      ) ;


    void addStringToMatchMap ( DocID doc_id
                             , std::string word
                             );
    bool stringIsInMatchMap  ( DocID doc_id
                             , std::string word
                             );

    void printMatchMap();


    int  numberOfQueries();
    bool isValidQuery  ( QueryID query_id );
    void removeQuery   ( QueryID query_id );
};

#endif /* defined(__sigmod__SearchTree__) */

