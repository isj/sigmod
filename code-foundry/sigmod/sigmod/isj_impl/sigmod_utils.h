//
//  sigmod_utils.h
//  sigmod
//
//  Created by jonathan on 06/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#ifndef __sigmod__sigmod_utils__
#define __sigmod__sigmod_utils__

#include <iostream>
#include "sigmod_types.h"
#include <vector>

#include "SearchNode.h"
/**
 *  sigmod_utils
 *
 *   utility functions
 */


/**
 *  AddToArray
 *
 *  C dynamic array
 *  http://fydo.net/gamedev/dynamic-arrays
 */



//int AddToArray (DynamicArray array,unsigned int element);

//unsigned int* VectorToArray (SingleDocResultVector vec);

unsigned int* SetToArray (SingleDocResultSet* set);

//unsigned int* MapToArray (SingleDocResultMap);

//void printMap (std::map);

#pragma mark - printing data sets

void printMapOfIntInt          (MapOfIntInt* mapToPrint);
void printMapOfIntMapOfIntInt  (MapOfIntMapOfIntInt* mapToPrint);
void printSetOfInts            (std::set < unsigned int>* setToPrint);

void printSetOfStrings         (std::set < std::string>* setToPrint);
void printMapOfIntSetOfStrings (MapOfIntSetOfStrings* mapToPrint);
void printMapOfIntSetOfInts    (MapofIntSetOfInts* mapToPrint);
void printVectorOfInts         (std::vector<unsigned int>* vectorToPrint);
void printVectorOfStrings         (std::vector<std::string>* vectorToPrint);
void rPrintVectorOfStrings         (std::vector<std::string>& vectorToPrint);

bool LevenshteinDistance(char* s, char* t, int limit);



void rSearch (std::vector<std::string>&,const char* word, int limit);

#endif /* defined(__sigmod__sigmod_utils__) */
