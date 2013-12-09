//
//  SearchNode.h
//  sigmod
//
//  Created by jonathan on 03/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#ifndef __sigmod__SearchNode__
#define __sigmod__SearchNode__

#include <iostream>
#include "core.h"
#include "sigmod_types.h"
#include <vector>

class SearchTree;
class WordTumbler;

class SearchNode {
private:
    unsigned int _depth;
    SearchNode* _parent_node;
    char _letter;

    /**
     *  bool _terminator
     *  Marker if this node terminates a word
     */
    bool _terminator = false;

    /**
     *  SearchNode* _child_letters
     *  array to hold child node pointers. One array position per alphabet letter.
     *  we declare and initialise an array large enough for any ASCII7 character.
     *  is this excessive (memory use)? should be use a hash table? something else?
     */
    SearchNode* _child_letters[128]={};


    /**
     *      Match match;
     3 - element array of pointers (exact, hamming, edit)

     exact -> vector of numbers which are search query indexes
     hamming -> 3-element array of pointers
     ->0 -> vector of numbers which are search query indexes for edit distance 1
     ->1 -> vector of numbers which are search query indexes for edit distance 2
     ->2 -> vector of numbers which are search query indexes for edit distance 3
     edit -> 3-element array of pointers
     ->0 -> vector of numbers which are search query indexes for edit distance 1
     ->1 -> vector of numbers which are search query indexes for edit distance 2
     ->2 -> vector of numbers which are search query indexes for edit distance 3
     */

    Match _match;




    
public:
    SearchNode();  //constructor for root node

    SearchNode ( //constructor for non-root node
                        QueryID  query_id
                ,   const char* query_str
                ,    MatchType  match_type
                , unsigned int  match_dist
                , unsigned int  query_str_idx
                , unsigned int  query_word_counter
                , SearchNode*   parent_node
                );

    ~SearchNode();  //TODO: destructor 

    void addQuery(       QueryID  query_id
                  ,   const char* query_str
                  ,    MatchType  match_type
                  , unsigned int  match_dist
                  , unsigned int  query_str_idx
                  , unsigned int  query_word_counter
                 );
    
    void addDocument(        DocID  doc_id
                     ,  const char* doc_str
                     ,         int  doc_str_idx
                    );
    void addDocumentL(        DocID  doc_id
                     ,  const char* doc_str
                     , unsigned int doc_str_idx
                     );

    void matchWord (  DocID doc_id
                    , const char* doc_str
                    , unsigned int word_start_idx
                    , unsigned int word_length
                    );

    void print (  char* letters );
    void print (  );
    void print_search_queries();

    std::string string();

    std::vector < unsigned int > queryIDs();

    char getLetterFromParentForDepth(int depth);

    void reportResult (DocID doc_id);



};




#endif /* defined(__sigmod__SearchNode__) */
