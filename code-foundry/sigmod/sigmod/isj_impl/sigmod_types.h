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

#define LOG 1

typedef struct  {
    std::vector<int> exact;
    std::vector<int>** hamming;
    std::vector<int>** edit;
} Match;
    




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
