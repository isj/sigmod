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



int AddToArray (DynamicArray array,unsigned int element);

unsigned int* VectorToArray (std::vector<unsigned int> vec);

unsigned int* SetToArray (DocResultSet);

unsigned int* MapToArray (SingleDocResultMap);


#endif /* defined(__sigmod__sigmod_utils__) */
