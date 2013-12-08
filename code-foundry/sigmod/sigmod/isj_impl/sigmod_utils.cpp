//
//  sigmod_utils.cpp
//  sigmod
//
//  Created by jonathan on 06/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#include "sigmod_utils.h"
#include <cstdlib>
#include <vector>




/*
 * AddToArray
 * This is a C "dynamic" array
 * (it's a static array that we double in size whenever we fill it)
 * it would be simpler to use C++ vectors but the core.h file
 * is quite explicit: p_query_ids MUST be a C array.

* @param[out] *p_query_ids
*   An array of the IDs of the *p_num_res matching queries,
*   ordered by the ID values. This array must be allocated by this
*   core library using malloc(). This array must not be freed by the
*   core library, since it will be freed by the testing benchmark.
*   If *p_num_res=0, this array must not be allocated, as it will not
*   be freed by the testing benchmark in that case.
*   Allocating this array using "new" is not acceptable.
*   In case of *p_num_res is not zero, The size of this array must be
*   equal to "(*p_num_res)*sizeof(QueryID)" bytes.
*/


/**
 *  this dynamic array breaks this:
 *
 *   In case of *p_num_res is not zero, The size of this array must be
 *   equal to "(*p_num_res)*sizeof(QueryID)" bytes.
 *
 *  but the code still passes the tests
 *  if we need to change it then consider:
 * we only  require this flexiblility in order to process searches PER WORD
 * if we batch process PER DOCUMENT we can accumulate results in a vector and
 * create an array once on exit.
 * or... we can store in a vector and create the array once on GetNextResult request
 */
/**
 *  realloc vs malloc
 *
 *  http://stackoverflow.com/questions/1401234/differences-between-using-realloc-vs-free-malloc-functions#1401247
 * 
 *
 *  realloc uses the same memory block as existing.
 *  it is SLOWER than free() malloc()
 *  but FASTER if you have to copy existing data into the new memory
 *
 */

/*
 int AddToArray (DynamicArray array,unsigned int element)
{
    if(array.num_elements == array.num_allocated) // Are more refs required?
    {
        // Feel free to change the initial number of refs
        // and the rate at which refs are allocated.
        if (array.num_allocated == 0)
            array.num_allocated = 3; // Start off with 3 refs
        else
            array.num_allocated *= 2; // Double the number
        // of refs allocated

        // Make the reallocation transactional
        // by using a temporary variable first
        void *_tmp = realloc(array.elements, (array.num_allocated * sizeof(array.elements)));

        // If the reallocation didn't go so well,
        // inform the user and bail out
        if (!_tmp)
        {
            fprintf(stderr, "ERROR: Couldn't realloc memory!\n");
            return(-1);
        }

        // Things are looking good so far
        array.elements = (unsigned int*)_tmp;
    }

    array.elements[array.num_elements] = element;
    array.num_elements++;

    return array.num_elements;
}
*/

/**
 *  alternatives to AddToArray - using fixed-sized malloc'd array
 *  for vectors, we would need to sort first (sorting is not implemented here)
 *  set and map versions are sorted as we insert new items into the set / map
 */
unsigned int* VectorToArray (SingleDocResultVector vec) {
    //for vectors we would need to sort first (not implemented)
    unsigned int* array = (unsigned int*)malloc(vec.size()*sizeof(unsigned int));
    std::copy(vec.begin(), vec.end(), array);
    return array;
}

unsigned int* SetToArray (SingleDocResultSet* set) {
    unsigned int* array = (unsigned int*)malloc(set->size()*sizeof(unsigned int));
    std::copy(set->begin(), set->end(), array);
    return array;
}

unsigned int* MapToArray (SingleDocResultMap map) {
    unsigned int* array = (unsigned int*)malloc(map.size()*sizeof(unsigned int));
    std::map<unsigned int,unsigned int>::iterator it;
    int i = 0;
    for (it=map.begin(); it!=map.end(); ++it) {
        array[i] = it -> first;
        i++;
    }
    return array;
}

