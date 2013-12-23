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
private:


    //from searchTree

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

    //from searchTree

    void addQueryToMap (  QueryID query_id
                        , unsigned int word_count
                        );


    void removeDocFromMatchedWordsMap  (   DocID doc_id );


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

    //from searchNode

    void reportResult (DocID doc_id);

    void checkEditResult (DocID doc_id, int edit_distance, int word_length, int hamming_cost,bool exact_match);

};