//
//  DataManager.h
//  sigmod
//
//  Created by jonathan on 23/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#ifndef __sigmod__DataManager__
#define __sigmod__DataManager__

#include <iostream>
#include "core.h"
#include "sigmod_types.h"

#endif /* defined(__sigmod__DataManager__) */


class DataManager {
    /**
     *  DataManager singleton class
     *  this is where we keep references to our global data
     *  and provide methods for adding, removing, querying the data
     */
private:
    //singleton pattern, we only ever want ONE dataManager.

    /**
     *  useage
     *  DataManager::Instance()->addQuery(...);
     */
    DataManager ();
    DataManager(DataManager const&){};             // copy constructor is private
    //SearchTree& operator=(SearchTree const&){};  // assignment operator is private
    static DataManager* m_pInstance;
    



    /**
     *  unordered_set of query_ids... (this is a hash table)
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


    /**
     *  _query_ids
     *  we use _query_ids to track LIVE querys. end_query removes the id from this list.
     *  we could/should also remove the query from every node where it is indexed.
     *  to do that we would have to keep a cross-reference from query ids to nodes.
     */
#ifdef USING_LIBCPP
    //OSX-C++ std libary feature only. For Linux we would need to use BOOST
    std::unordered_set < QueryID > * _query_ids;   //OSX-only
#else
    //for Linux let's use an ordered set instead
    std::set < QueryID > * _query_ids;
#endif

    //_query_ids_map
    //we need to store a list of query ids with their associated word count
    //so that we can decrement for any particular document.

    //this reflects the current state of the app
    //we make a COPY of this for each new document, to reflect the state of the app
    //when the document interrogation starts
    //we add that copy to our _query_ids_map_per_document, from which we will harvest results

    std::map < QueryID, unsigned int > * _query_ids_map;

    //_query_ids_map_per_document
    //document query count map
    //list of query IDs with word counts
    //we initialise one for each new document search
    //on each found word we decrement the relevant query counts
    //successful query matches are those that decrement to zero.

    std::map <DocID, std::map < QueryID, unsigned int >  > * _query_ids_map_per_document;

    //_doc_words_set
    // we keep a set of document words so that we do not process duplicates
    std::map < DocID, std::set < std::string > > * _map_of_sets_of_doc_words;


    /**
     *  _docResultsMap
     *  Data structure to store all document results
     *  on GetNextAvailRes we return one    and erase it from the map.
     *  keyed by Document ID.
     */
    std::map <DocID, std::set<QueryID>*> _docResultsMap;
    //does this need to be a map of pointers to sets? If we always use a pointer to this map, we are already on the heap....



public:

#pragma mark ----------------------
#pragma mark   singleton initialisation
#pragma mark ----------------------

    static DataManager* Instance();


#pragma mark ----------------------
#pragma mark   doc word set management
#pragma mark ----------------------

    void addWordToDocWordSet ( DocID doc_id
                              , std::string word
                              );
    bool wordIsInDocWordSet  ( DocID doc_id
                              , std::string word
                              );
    void removeDocWordSet  (   DocID doc_id );


#pragma mark ----------------------
#pragma mark   query accounting
#pragma mark ----------------------


    //from searchTree

    void addQueryToMap (  QueryID query_id
                        , unsigned int word_count
                        );



    void decrementQueryInDocumentMap (  DocID doc_id
                                      , QueryID query_id
                                      ) ;



    bool isValidQueryID  ( QueryID query_id );
    void removeQueryID   ( QueryID query_id );
    int  numberOfQueries();


#pragma mark ----------------------

    //from searchNode

 //   void reportResult (DocID doc_id);

 //   void checkEditResult (DocID doc_id, int edit_distance, int word_length, int hamming_cost,bool exact_match);

};