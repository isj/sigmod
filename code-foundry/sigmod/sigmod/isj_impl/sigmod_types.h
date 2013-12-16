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
#include <string>
#include <iostream>
#define LOG 1  //turn this off if measuring running time!
#define LOGALL 0  //turn this off if measuring running time!
#define EDIT_DISTANCE 0
#define EDIT_DISTANCE_TEST 0
#define MATCHED_MAP 1 //do we cache a list of matched words so we don't have to match them again?
#define DELETE_NODES 0 // don't use, has errors
#define DUPLICATE_WORD_FILTER 0
#define MATCH_TYPES_FILTER 0

//#define USING_LIBCPP  //libc++ is OSX C++ library. Linux uses libstc++. It provides us with <unordered_set>.


static const  int kFirstASCIIChar = 48; //0
static const  int kLastASCIIChar = 122; //z

typedef struct  {
    std::vector<int> exact;
    std::vector<int>** hamming;
    std::vector<int>** edit;
    std::vector<int> all;
} Match;


//typedef struct {
//    unsigned int* elements;
//    unsigned int num_elements = 0; // Keeps track of the number of elements used
//    unsigned int num_allocated = 0; // This is essentially how large the array is
//} DynamicArray;

typedef  struct {
    int exact;
    int hamming1;
    int hamming2;
    int hamming3;
    int edit1;
    int edit2;
    int edit3;
} BranchMatchTypes;

typedef enum {
    kIncrementTypeSubtract = -1,
    kIncrementTypeAdd = 1

}IncrementType;

//typedef struct {
//    unsigned int _p_doc_id = 0;
//    unsigned int _p_num_res = 0;
//    DynamicArray _p_query_ids;
//} DocResult1;




//_query_ids_map
typedef     std::map < unsigned int, unsigned int > MapOfIntInt;

//_matched_words_map
typedef     std::map <unsigned int, MapOfIntInt  > MapOfIntMapOfIntInt;

//_query_ids_map_per_document
typedef     std::map < unsigned int, std::set < std::string > > MapOfIntSetOfStrings;

// allDocumentResultsMap
typedef     std::map < unsigned int, std::set <unsigned int >  > MapofIntSetOfInts;



/**
 *  DocResult
 *  Data strucuture to store a single document's results
 *  it holds the three values we need to return in GetNextAvailRes
 *  (we need to convert the set into an array before return it, see sigmod_utils)
 *
  */


/**
 *  SingleDocResults - Vector / Set / Map
 *  Data structure to store a SINGLE document result collection of QueryIDs
 *
 *
 *  on GetNextAvailRes we return one    and erase it from the map.
 *  keyed by Document ID.
 */

typedef std::set<unsigned int> SingleDocResultSet;
//this is the ideal container
//associative - ordered - set - unique keys - allocator-aware
//we need to use default ordering.
//XCode has problems with comparison function

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
typedef  std::map <unsigned int, SingleDocResultSet*> AllDocsResultsMap;







#endif
