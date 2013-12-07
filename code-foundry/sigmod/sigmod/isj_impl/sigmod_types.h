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


typedef struct {
    unsigned int _p_doc_id = 0;
    unsigned int _p_num_res = 0;
    DynamicArray _p_query_ids;
} DocResult;


typedef  std::map <unsigned int, DocResult> DocResultsMap;





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
