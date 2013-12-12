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
#include "SearchTree.h"
//#include "SearchNode.h"

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
    if (LOG) {
        printf("%s\n",__func__);

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

}

//std::map < unsigned int, std::set < std::string > > MapOfIntSetOfStrings;

void printSetOfStrings (std::set < std::string>* setToPrint) {
    if (LOGALL) printf("%s\n",__func__);
    if (LOG) {
    std::set < std::string>::iterator it;
    for (it = setToPrint->begin(); it != setToPrint->end(); it++) {
        std::cout <<*it << " ";
    }
    std::cout << std::endl;
    }

}



void printMapOfIntSetOfStrings (MapOfIntSetOfStrings* mapToPrint)  {
    if (LOGALL) printf("%s\n",__func__);
    if (LOG ) {
        std::map < unsigned int, std::set < std::string > >::iterator it;
        for (it = mapToPrint->begin(); it != mapToPrint->end(); it++) {
            std::cout << it->first;
            std::cout <<"      ";
            printSetOfStrings(&it->second);
            ;
        }
        std::cout << std::endl;
    }
}

void printSetOfInts (std::set < unsigned int>* setToPrint) {
    if (LOG) {

        printf("%s\n",__func__);

        std::set <  unsigned int >::iterator it;
        for (it = setToPrint->begin(); it != setToPrint->end(); it++) {
            std::cout <<*it << " ";
        }
        std::cout << std::endl;
        
    }
}

void printMapOfIntSetOfInts    (MapofIntSetOfInts* mapToPrint) {
    if (LOG) {
        printf("%s\n",__func__);
        std::map < unsigned int, std::set < unsigned int > >::iterator it;
        for (it = mapToPrint->begin(); it != mapToPrint->end(); it++) {
            std::cout << it->first << std::endl;
            std::cout <<"      ";
        }
    }
}


void printVectorOfInts (std::vector<unsigned int>* vectorToPrint) {
    if (LOG) {

        printf("%s\n",__func__);

        for (int i = 0; i < vectorToPrint->size(); i++) {
            std::cout << &vectorToPrint[i] << " ";
        }
        std::cout << std::endl;
    }
}


//https://en.wikipedia.org/wiki/Levenshtein_distance#cite_note-8
//http://www.codeproject.com/Articles/13525/Fast-memory-efficient-Levenshtein-algorithm

bool LevenshteinDistance(char* s, char* t, int limit)
{
    if (LOG)printf("%s %s %s %d",__func__, s, t, limit);
    //filter for s=t before passing in here
    // degenerate cases
    int length_t = (int)strlen(t);
    int length_s = (int)strlen(s);
    //if (s == t) return 0;
    if (length_s ==0) return length_t;
    if (length_t ==0) return length_s;
    int v0[31] = {};
    int v1[31] = {};


    for (int i = 0; i < length_t+1; i++)
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
            if (v1[j+1] > limit) {
                if (LOG)printf("cost: %d \n",v1[j + 1]);
                if (LOG)printf("no match\n");
                return false;
            }
            if (LOG)printf("cost: %d \n",v1[j + 1]);
        }

        // copy v1 (current row) to v0 (previous row) for next iteration
        for (int j = 0; j < 1+length_t; j++)
            v0[j] = v1[j];
    }
    int result = v1[length_t];
    printf("result: %d \n",result);
    printf("match\n");
    return true;
   // free(v0);
   // free(v1);
}

bool uiLevenshteinDistance(char* s, char* t, int limit)
{
    if (LOG)printf("%s %s %s %d",__func__, s, t, limit);
    string s1 = string (s);
    string s2 = string (t);
    const size_t m(s1.size());
    const size_t n(s2.size());

    if( m==0 ) return n;
    if( n==0 ) return m;

    size_t *costs = new size_t[n + 1];

    for( size_t k=0; k<=n; k++ ) costs[k] = k;

    size_t i = 0;
    for ( auto it1 = s1.begin(); it1 != s1.end(); ++it1, ++i )
    {
        costs[0] = i+1;
        size_t corner = i;

        size_t j = 0;
        for ( auto it2 = s2.begin(); it2 != s2.end(); ++it2, ++j )
        {
            size_t upper = costs[j+1];
            if( *it1 == *it2 )
            {
                costs[j+1] = corner;
            }
            else
            {
                size_t t(upper<corner?upper:corner);
                costs[j+1] = (costs[j]<t?costs[j]:t)+1;
                if (costs[j+1]> limit) {
                    if (LOG)printf("cost: %zu \n",costs[j+1]);
                    if (LOG)printf("no match\n");
                    return false;
                }
                if (LOG)printf("cost: %zu \n",costs[j+1]);

            }
            
            corner = upper;
        }
    }
    
    size_t result = costs[n];
    delete [] costs;
    printf("result: %zu \n",result);
    printf("match\n");
    return true;
}

int minCurrentRow(int* row) {
    int result = row[0];
    for (int i=0; i<sizeof(row)/sizeof(int);i++) {
        if (row[i] < result) {
            result = row[i];
        }
    }
    return result;

}



/**
 * recursiveSearch
 * search (char* word, int limit)
 *  http://stevehanov.ca/blog/index.php?id=114
 *   
 * returns a _list of words_ from the searchtree 
 * that are less than the given max distance from the target word
 * we can extend this by checking the node indexes for queries
 */

//

vector<string>  recursiveSearch(
                                SearchNode* node
                                , char letter
                                , char* word
                                , int* previousRow
                                ,  int max_cost) {

    //int columns[31]; //should be length-of-word+1
    int currentRow[31];//should be ??? previousRow[0]+1??
    int currentRow_index = 0;
    int replaceCost = 0;
    vector<string> results;

    currentRow[currentRow_index]= previousRow[currentRow_index] + 1;
    for (int column=0; column < 31; column++) {
        int insertCost = currentRow[column] + 1;
        int deleteCost = previousRow[column] + 1;
        if (word[column -1] != letter) {
             replaceCost = previousRow[column-1]+1;
        } else {
            replaceCost = previousRow[column-1];
        }

        int costToAdd = min(insertCost,min(deleteCost, replaceCost));
        currentRow[column] = costToAdd;
    }

    //if the last entry in the row indicates the optimal cost is less than the max costs,
    //and there is a word in this trie node, then add it

    if ( currentRow[30]<= max_cost && node->isTerminator() == true ) {
        cout << "found word " << node->string();
        results.push_back( node->string());
    }

//if any entries in the row are less than max cost, then recursively search each branch of the trie

    if (minCurrentRow(currentRow) <= max_cost) {
        for (int i=kFirstASCIIChar; i<=kLastASCIIChar; i++ ) {
            if (node->child(i) != nullptr) {
                results = recursiveSearch(node->child(i)
                                          ,letter
                                          ,word
                                          ,currentRow
                                          , max_cost);
            }
        }

    }

    return results;
}

std::vector<std::string> search (char* word, int limit) {
    string sWord = string (word);

    size_t length_word = sWord.length();
    //build first row
    int row[length_word+1];

    vector<string> results;

    //recursive search of each branch of our searchTree for letter in kids

    results = recursiveSearch (SearchTree::Instance()->root()
                               ,0,word,row,0 );

    return results;

}


