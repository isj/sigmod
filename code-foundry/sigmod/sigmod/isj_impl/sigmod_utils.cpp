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




////////////////////////////////////////////////////////////////////////
////////////////////////rSearch3////////////////////////////////////////
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

//
//typedef struct {
//    bool test;
//    int cost;
//} ExactMatch;
//
//typedef struct {
//    int current_cost;
//    int max_cost;
//} HammingDistance;
//
//typedef struct {
//    int* previous_row;
//    int max_cost;
//} EditDistance;
//
//typedef struct {
//    DocID doc_id
//    ; SearchNode* node
//    ; const char* doc_word
//    ; int doc_word_length
//    ; ExactMatch exact;
//    ; HammingDistance hamming;
//    ; EditDistance edit;
//} WordTumbler;
//
//
//std::map<char, SearchNode*> mapOfChildNodesToFollow (SearchNode* node) {
//    std::map<char, SearchNode*> map;
//    std::map<char,SearchNode*>::iterator it = map.begin();  //using iterator for position hints, more efficient insert
//    for (int i=kFirstASCIIChar; i<=kLastASCIIChar; i++ ) {
//        if (node->child(i) != nullptr) {
//            printf("inserting letter %c\n",node->child(i)->letter());
//              map.insert ( it,std::pair<char,SearchNode*>(i,node->child(i)) );
//        }
//    }
//    return map;
//}
//
//void r3RecursiveSearch(WordTumbler& tumbler);
//
//static void searchChildNodes(WordTumbler& tumbler) {
//    std::map<char, SearchNode*> map = mapOfChildNodesToFollow(tumbler.node);
//    std::map<char,SearchNode*>::iterator it;
//    for (it = map.begin(); it != map.end();it++) {
//        SearchNode* child_node = it->second;
//        printf("inserting letter %c\n",child_node->letter());
//        tumbler.node = child_node;
//        if (tumbler.exact.test) tumbler.exact.test = child_node->branchHasExactMatches();
//        tumbler.exact.cost = (tumbler.exact.test) ? 1 : 0;
//        tumbler.hamming.max_cost = child_node->branchHasHammingMatches();
//        tumbler.edit.max_cost = child_node->branchHasEditMatches();
//        if (tumbler.exact.cost + tumbler.hamming.max_cost + tumbler.edit.max_cost > 0) {
//            r3RecursiveSearch(tumbler);
//        }
//    }
//
//}
//
//bool lettersMatch(WordTumbler& tumbler) {
//    return (tumbler.doc_word[tumbler.node->depth()-1] == tumbler.node->letter());
//}
//
//static void testExact(WordTumbler& tumbler) {
//    if (tumbler.exact.test) {
//        cout << " exact test " << tumbler.doc_id << " word "<<tumbler.doc_word << " node_string "<<tumbler.node->string()<<endl;
//        if (!lettersMatch(tumbler)) {
//            tumbler.exact.test = false;
//        }
//    }
//}
//
//static void testHamming (WordTumbler& tumbler) {
//    if (tumbler.hamming.max_cost > 0) {
//        printf("hamming test\n");
//        if (tumbler.hamming.current_cost <= tumbler.hamming.max_cost)
//            if (!lettersMatch(tumbler)) {
//                tumbler.hamming.current_cost ++;
//            }
//    }
//}
//
//static void testEdit (WordTumbler& tumbler) {
//    if (tumbler.edit.max_cost > 0) {
//        printf("edit test\n");
//
//        int columns = (int)tumbler.doc_word_length+1;
//        int currentRow[columns];
//        currentRow[0] = tumbler.edit.previous_row[0]+1;
//        int replaceCost = 0;
//
//        //edit match algorith
//        // if (match_edit == true) {
//        for (int column=1; column <= columns; column++) {
//            int insertCost = currentRow[column-1] + 1;
//            int deleteCost = tumbler.edit.previous_row[column] + 1;
//            if (tumbler.doc_word[column -1] != tumbler.node->letter()) {
//                replaceCost = tumbler.edit.previous_row[column-1]+1;
//            } else {
//                replaceCost = tumbler.edit.previous_row[column-1];
//            }
//            int costToAdd = min(insertCost,min(deleteCost, replaceCost));
//            currentRow[column] = costToAdd;
//        }
//        if (LOG) {
//            printf("current row  %c            ",tumbler.node->letter());
//            for (int i=0; i<columns; i++) {
//                printf("%d",currentRow[i]);
//            }
//            printf("\n");
//        }
//        //}
//    }
//}
//
//
//static void testResults(WordTumbler& tumbler) {
//    if (tumbler.node->isTerminator()) {
//        printf("test results\n");
//    }
//
//}
//
//void r3RecursiveSearch(WordTumbler& tumbler) {
//    bool continue_searching = true;
//    testExact(tumbler);
//    //testHamming(tumbler);
//    //testEdit(tumbler);
//    testResults(tumbler);
//    if (continue_searching) {
//        searchChildNodes(tumbler);
//    }
//}
//
//WordTumbler newTumbler(DocID doc_id,  char* word,int word_length, int limit) {
//    WordTumbler tumbler;
//    SearchNode* node =SearchTree::Instance()->root();
//    tumbler.node = node;
//    tumbler.doc_id = doc_id;
//    tumbler.doc_word = word;
//    tumbler.doc_word_length = word_length;
//    tumbler.exact.test = true;
//    tumbler.exact.cost = 1;
//    tumbler.hamming.max_cost = limit;
//    tumbler.edit.max_cost = limit;
//    tumbler.hamming.current_cost = 0;
//    tumbler.edit.previous_row = nullptr;
//
//    //build first results row for edit match
//    int row[word_length+1];
//    for (int i = 0; i< (word_length+1); i++) {
//        row[i] = i;
//    }
//    tumbler.edit.previous_row = row;
//    return tumbler;
//}
//
//void r2Search (DocID doc_id
//               , char* word
//               , int word_length
//               , int limit) {
//
//    WordTumbler tumbler = newTumbler(doc_id
//                                     ,word
//                                     ,word_length
//                                     ,limit);
//    r3RecursiveSearch  (tumbler);
//}




