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
#include <cstring>

using namespace std;


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
//unsigned int* VectorToArray (SingleDocResultVector vec) {
//    //for vectors we would need to sort first (not implemented)
//    unsigned int* array = (unsigned int*)malloc(vec.size()*sizeof(unsigned int));
//    std::copy(vec.begin(), vec.end(), array);
//    return array;
//}

unsigned int* SetToArray (SingleDocResultSet* set) {
    unsigned int* array = (unsigned int*)malloc(set->size()*sizeof(unsigned int));
    std::copy(set->begin(), set->end(), array);
    return array;
}

//unsigned int* MapToArray (SingleDocResultMap map) {
//    unsigned int* array = (unsigned int*)malloc(map.size()*sizeof(unsigned int));
//    std::map<unsigned int,unsigned int>::iterator it;
//    int i = 0;
//    for (it=map.begin(); it!=map.end(); ++it) {
//        array[i] = it -> first;
//        i++;
//    }
//    return array;
//}

#pragma mark - utility functions to print map data strucures


//std::map < unsigned int, unsigned int > MapOfIntInt;

void printMapOfIntInt (MapOfIntInt* mapToPrint) {
    if (LOG) printf("%s\n",__func__);

    std::map < unsigned int, unsigned int >::iterator it;
    for (it = mapToPrint->begin();
         it != mapToPrint-> end();
         it++ ){

        std::cout << it->first <<" "<< it->second << std::endl;

    }



}


void printMapOfIntMapOfIntInt (MapOfIntMapOfIntInt* mapToPrint) {
    if (LOG) printf("%s\n",__func__);

    std::map <unsigned int, std::map < unsigned int, unsigned int > >::iterator it;
    for (it = mapToPrint->begin();
         it != mapToPrint-> end();
         it++ ){
        std::cout << it->first <<" "<< std::endl;
        std::cout << "     ";
        printMapOfIntInt(&it->second);
    }
    std::cout << std::endl;

}

//std::map < unsigned int, std::set < std::string > > MapOfIntSetOfStrings;

void printSetOfStrings (std::set < std::string>* setToPrint) {
    if (LOG) printf("%s\n",__func__);

    std::set < std::string>::iterator it;
    for (it = setToPrint->begin(); it != setToPrint->end(); it++) {
        std::cout <<*it << " ";
    }
    std::cout << std::endl;


}



void printMapOfIntSetOfStrings (MapOfIntSetOfStrings* mapToPrint)  {
    if (LOG) printf("%s\n",__func__);
    std::map < unsigned int, std::set < std::string > >::iterator it;
    for (it = mapToPrint->begin(); it != mapToPrint->end(); it++) {
        std::cout << it->first << std::endl;
        std::cout <<"      ";
        printSetOfStrings(&it->second);
        ;
    }
    std::cout << std::endl;

}

void printSetOfInts (std::set < unsigned int>* setToPrint) {
    if (LOG) printf("%s\n",__func__);

    std::set <  unsigned int >::iterator it;
    for (it = setToPrint->begin(); it != setToPrint->end(); it++) {
        std::cout <<*it << " ";
    }
    std::cout << std::endl;
    
    
}

void printMapOfIntSetOfInts    (MapofIntSetOfInts* mapToPrint) {
    if (LOG) printf("%s\n",__func__);
    std::map < unsigned int, std::set < unsigned int > >::iterator it;
    for (it = mapToPrint->begin(); it != mapToPrint->end(); it++) {
        std::cout << it->first << std::endl;
        std::cout <<"      ";
    }

}


void printVectorOfInts (std::vector<unsigned int>* vectorToPrint) {
    if (LOG) printf("%s\n",__func__);

    for (int i = 0; i < vectorToPrint->size(); i++) {
        std::cout << &vectorToPrint[i] << " ";
    }
    std::cout << std::endl;
}


//https://en.wikipedia.org/wiki/Levenshtein_distance#cite_note-8
//http://www.codeproject.com/Articles/13525/Fast-memory-efficient-Levenshtein-algorithm

int LevenshteinDistance(char* s, char* t)
{
    // degenerate cases
    int length_t = (int)strlen(t);
    int length_s = (int)strlen(s);
    if (s == t) return 0;
    if (strlen(s)==0) return length_t;
    if (strlen(t)==0) return length_s;
    int v0[31] = {};
    int v1[31] = {};
   // int* v0 = (int*)malloc((1+length_t)*sizeof(char));
   // int* v1 = (int*)malloc((1+length_t)*sizeof(char));
    if (NULL == v0 || NULL == v1) {
        printf("%s error allocating memory",__func__);
        return 100;
    }

    for (int i = 0; i < 1+length_t; i++)
        v0[i] = i;

    for (int i = 0; i < length_s; i++)
    {
        // calculate v1 (current row distances) from the previous row v0

        // first element of v1 is A[i+1][0]
        //   edit distance is delete (i+1) chars from s to match empty t
        v1[0] = i + 1;

        // use formula to fill in the rest of the row
        for (int j = 0; j < length_t; j++)
        {
            int cost = (s[i] == t[j]) ? 0 : 1;
            v1[j + 1] = min(v1[j] + 1, min(v0[j + 1] + 1,v0[j] + cost));
        }

        // copy v1 (current row) to v0 (previous row) for next iteration
        for (int j = 0; j < 1+length_t; j++)
            v0[j] = v1[j];
    }
    int result = v1[length_t];
   // free(v0);
   // free(v1);
    return result;
}

