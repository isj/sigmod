//
//  sigmod_types.h
//  sigmod
//
//  Created by jonathan on 03/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#ifndef sigmod_sigmod_types_h
#define sigmod_sigmod_types_h
#include <vector>
#include <map>
#include <set>
#include <iostream>
#define LOG 1


static const  int kFirstASCIIChar = 48; //0
static const  int kLastASCIIChar = 122; //z

typedef struct  {
    std::vector<int> exact;
    std::vector<int>** hamming;
    std::vector<int>** edit;
} Match;


typedef struct {
    unsigned int* elements;
    unsigned int num_elements = 0; // Keeps track of the number of elements used
    unsigned int num_allocated = 0; // This is essentially how large the array is
} DynamicArray;


//typedef struct {
//    unsigned int _p_doc_id = 0;
//    unsigned int _p_num_res = 0;
//    DynamicArray _p_query_ids;
//} DocResult1;




/**
 *  DocResult
 *  Data strucuture to store a single document's results
 *  it holds the three values we need to return in GetNextAvailRes
 *  (we need to convert the set into an array before return it, see sigmod_utils)
 *
 *  ltstr
 *  this is the sort function for _p_query_ids set
 * http://stackoverflow.com/questions/3255971/method-declared-in-struct-in-c-stl
 * http://stackoverflow.com/questions/16966558/unordered-set-with-custom-predicate-make-linker-error-duplicate-symbol
 * problems with OSX http://stackoverflow.com/questions/19746903/cant-push-a-map-inside-a-vector
 * libc++ (default) has this problem
 * libstdc++ does not understand unordered_set, need to use BOOST instead (or just not use)
 */


struct ltstr
{
    bool operator()(const char* s1, const char* s2) const
    {
        return strcmp(s1, s2) < 0;
    }
};

/**
 *  SingleDocResults - Vector / Set / Map
 *  Data structure to store a SINGLE document result collection of QueryIDs
 *
 *
 *  on GetNextAvailRes we return one    and erase it from the map.
 *  keyed by Document ID.
 */


typedef std::set<unsigned int> DocResultSet;
//this is the ideal container
//associative - ordered - set - unique keys - allocator-aware
//



typedef std::vector < unsigned int > SingleDocResultVector;
//sequence - dynamic array - allocator-aware



typedef std::map < unsigned int, unsigned int > SingleDocResultMap;
////associative - ordered - map - unique keys - allocator-aware

//to treat it like a set we can set each key's value to 0 (or to the key)


/**
 *  DocResultsMap
 *  Data structure to store all document results
 *  on GetNextAvailRes we return one    and erase it from the map.
 *  keyed by Document ID.
 */
typedef  std::map <unsigned int, SingleDocResultMap> AllDocsResultsMap;

/*
 
 Doc18 -> QiD 2, Qid 6, Qid9...  (sorted)
 (unsorted)
 
 
 so either we insert QiDs in a sorted manner using MAP or SET
 
 OR we sort QiDs in VectorToArray...
 
 
 
 
 
 */





/**
 *  match type
 
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







#endif
