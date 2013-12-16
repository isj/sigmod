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
    std::set < std::string>::iterator it;
    for (it = setToPrint->begin(); it != setToPrint->end(); it++) {
        std::cout <<*it << " ";
    }
    std::cout << std::endl;
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

void printVectorOfStrings         (std::vector<std::string>* vectorToPrint) {
    if (LOG) {

        printf("%s\n",__func__);

        for (int i = 0; i < vectorToPrint->size(); i++) {
            std::cout << &vectorToPrint[i] << " ";
        }
        std::cout << std::endl;
    }
}

void rPrintVectorOfStrings         (std::vector<std::string>& vectorToPrint) {
        printf("%s\n",__func__);

        for (int i = 0; i < vectorToPrint.size(); i++) {
            std::cout << vectorToPrint[i] << " ";
        }
        std::cout << std::endl;
}
void printMatchIndex (Match match, std::string word) {

    printf("%s\n",__func__);
    std::cout<<"printing match index for query word: " << word << endl;
    std::cout<<"exact ";
    for (int i = 0; i<match.exact.size(); i++) {
        std::cout << match.exact[i] << " ";
    }
    std::cout<<endl;
    std::cout<<"hamming-1 ";
    for (int i = 0; i<match.hamming[0]->size(); i++) {
        std::cout << match.hamming[0]->at(i) << " ";
    }
    std::cout<<endl;
    std::cout<<"hamming-2 ";
    for (int i = 0; i<match.hamming[1]->size(); i++) {
        std::cout << match.hamming[1]->at(i) << " ";
    }
    std::cout<<endl;
    std::cout<<"hamming-3 ";
    for (int i = 0; i<match.hamming[2]->size(); i++) {
        std::cout << match.hamming[2]->at(i) << " ";
    }
    std::cout<<endl;
    std::cout<<"edit-1 ";
    for (int i = 0; i<match.edit[0]->size(); i++) {
        std::cout << match.edit[0]->at(i) << " ";
    }
    std::cout<<endl;

    std::cout<<"edit-2 ";
    for (int i = 0; i<match.edit[1]->size(); i++) {
        std::cout << match.edit[1]->at(i) << " ";
    }
    std::cout<<endl;

    std::cout<<"edit-3 ";
    for (int i = 0; i<match.edit[2]->size(); i++) {
        std::cout << match.edit[2]->at(i) << " ";
    }
    std::cout<<endl;

    std::cout<<"all ";
    for (int i = 0; i<match.all.size(); i++) {
        std::cout << match.all.at(i) << " ";
    }
    std::cout<<endl;

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





////////////////////////////////////////////////////////////////////////
////////////////////////rSearch2////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


int minCurrentRow(int* row, int length) {
    int result = row[0];
    for (int i=0; i<length;i++) {
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



void r2limitSearch (
                   SearchNode* node
                   , char letter
                   , const char* word
                   , size_t length_word
                   , int* previousRow
                   , int max_cost



                   ) {
    max_cost = 3;
    int columns = (int)max_cost+1;
    int currentRow[columns];
    currentRow[0] = previousRow[0]+1;

}

int maxCost(BranchMatchTypes match_types, int max_cost) {
    //TODO
    return max_cost;
}

bool includesHammingMatch(BranchMatchTypes match_types) {
    return (match_types.hamming1 > 0 || match_types.hamming2 > 0 || match_types.hamming3 > 0);
}

bool includesExactMatch(BranchMatchTypes match_types) {
    return (match_types.exact > 0);
}

bool includesEditMatch(BranchMatchTypes match_types) {
    return (match_types.edit1 > 0 || match_types.edit2 > 0 || match_types.edit3 > 0);
}

void r2RecursiveSearch(
                         DocID doc_id
                       , SearchNode* node
                       , char letter
                       , const char* word
                       , int length_word
                       , int* previousRow
                       , int hamming_cost
                       , int max_cost
                       , bool match_exact
                       , bool match_hamming
                       , bool match_edit
                       ) {
    if (match_exact == true) {
        if (word[node->depth()]!=letter) {
            match_exact = false;
        }
    }
    int columns = (int)length_word+1;
    int currentRow[columns];
    currentRow[0] = previousRow[0]+1;
    int replaceCost = 0;

//edit match algorith
    if (match_edit == true) {
        for (int column=1; column <= columns; column++) {
            int insertCost = currentRow[column-1] + 1;
            int deleteCost = previousRow[column] + 1;
            if (word[column -1] != letter) {
                replaceCost = previousRow[column-1]+1;
            } else {
                replaceCost = previousRow[column-1];
            }
            int costToAdd = min(insertCost,min(deleteCost, replaceCost));
            currentRow[column] = costToAdd;
        }
        if (LOG) {
            printf("current row  %c            ",letter);
            for (int i=0; i<columns; i++) {
                printf("%d",currentRow[i]);
            }
            printf("\n");
        }
    }

    if (match_hamming == true) {
        if (word[node->depth()] != node->letter()) hamming_cost++;
    }
    //if the last entry in the row indicates the optimal cost is less than the max costs,
    //and there is a word in this trie node, then add it
    if (node->isTerminator() == true) {
        int edit_distance = 0;
        if (match_edit == true && currentRow[columns-1]<= max_cost) {
            edit_distance =currentRow[columns-1];
        }
    node->checkEditResult (doc_id, currentRow[columns-1], length_word, hamming_cost,match_exact);
    }


    //if any entries in the row are less than max cost, then recursively search each branch of the trie
    //printf("minCurrentRow: %d",minCurrentRow(currentRow,columns));
    //printf(" maxCost: %d\n",max_cost);

    if (minCurrentRow(currentRow,columns) <= max_cost|| hamming_cost <= max_cost) {
        for (int i=kFirstASCIIChar; i<=kLastASCIIChar; i++ ) {
            if (node->child(i) != nullptr) {
                SearchNode* search_node = node->child(i);
                char letter = search_node->letter();
                BranchMatchTypes match_types = search_node->branch_matches();
                if (MATCH_TYPES_FILTER) {
                    max_cost = maxCost(match_types,max_cost);
                    match_hamming = includesHammingMatch(match_types);
                    match_edit = includesEditMatch(match_types);
                }
                if (match_exact == true || match_hamming == true || match_edit==true) {
                r2RecursiveSearch(
                                  doc_id
                                  , search_node
                                  , letter
                                  , word
                                  , length_word
                                  , currentRow
                                  , hamming_cost
                                  , max_cost
                                  , match_exact
                                  , match_hamming
                                  , match_edit
                                  );
                }
            }
        }

    }else {
        if (LOG) printf("exceeded costs, stop searching this branch\n");
    }

}

void r2Search (DocID doc_id,  char* word,int word_length, int limit) {

    // size_t length_word = sWord.length();

    //build first row
    int row[word_length+1];

    for (int i = 0; i< (word_length+1); i++) {
        row[i] = i;
    }
    if (LOG) {
        printf("doc id: %d matching word: %s\n",doc_id,word);
        printf("first   row               ");
        for (int i=0; i<word_length+1; i++) {
            printf("%d",row[i]);
        }
        printf("\n");
    }
    int hamming_cost = 0;
    //recursive search of each branch of our searchTree for letter in kids
    SearchNode* node =SearchTree::Instance()->root();
    for (int i=kFirstASCIIChar; i<=kLastASCIIChar; i++ ) {
        if (node->child(i) != nullptr) {
            SearchNode* search_node = node->child(i);
            char letter = search_node->letter();
            BranchMatchTypes match_types = search_node->branch_matches();
            if (MATCH_TYPES_FILTER) limit = maxCost(match_types,limit);
            bool match_hamming = true;
            bool match_exact = true;
            bool match_edit = true;
            if (MATCH_TYPES_FILTER) {
                 match_hamming = includesHammingMatch(match_types);
                 match_edit = includesEditMatch(match_types);
            }
            r2RecursiveSearch  (
                                doc_id
                                , node->child(i)
                                , letter
                                , word
                                , word_length
                                , row
                                , hamming_cost
                                , limit
                                , match_exact
                                , match_hamming
                                , match_edit
                                );
        }
    }
    
}


