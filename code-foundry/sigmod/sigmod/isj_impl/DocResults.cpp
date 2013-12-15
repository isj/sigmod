//
//  DocResults.cpp
//  sigmod
//
//  Created by jonathan on 06/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#include "DocResults.h"
#include "sigmod_utils.h"
#include "SearchTree.h"


using namespace std;


#pragma mark - singleton initialisation


// Global static pointer used to ensure a single instance of the class.

DocResults* DocResults::m_pInstance = NULL;

/**     This function is called to create an instance of the class.
 *	    Calling the constructor publicly is not allowed. The constructor
 *	    is private and is only called by this Instance function.
 */

DocResults* DocResults::Instance()
{
    if (LOGALL) printf("%s\n",__func__);
    if (!m_pInstance)   // Only allow one instance of class to be generated.
        m_pInstance = new DocResults;
    return m_pInstance;
}


DocResults::DocResults() {
    //constructor
    //create _results storage

    //initialise _docResultsMap with key "0"

    /* 
     * set version
     * SingleDocResultSet setResult;
     * _docResultsMap[0].insert(setResult);
     *
     */

    SingleDocResultSet* mapResult = new SingleDocResultSet();
    _docResultsMap[0]=mapResult;
    //_docResultsKeys = new vector<unsigned int>();
}


#pragma mark - debug/logging of AllDocsResultsMap

void logSet (SingleDocResultSet* singleDocSet) {
    if (LOGALL) printf("%s\n",__func__);
    for (SingleDocResultSet::iterator it=singleDocSet->begin(); it!=singleDocSet->end(); ++it)
        std::cout << ' ' << *it;
    std::cout << endl;

}
void logMap (AllDocsResultsMap resultsMap) {
    if (LOGALL) printf("%s\n",__func__);
    AllDocsResultsMap::iterator it;
    SingleDocResultSet* result;
    for (AllDocsResultsMap::iterator it=resultsMap.begin(); it!=resultsMap.end(); ++it) {
        std::cout << "docID   " << it->first << '\n' <<  "queryIDs ";
        result = it->second;
        logSet (result);
    }

}

#pragma mark - GetNextAvailRes


ErrorCode DocResults::GetNextAvailRes ( DocID* p_doc_id
                                      , unsigned int* p_num_res
                                      , QueryID** p_query_ids
                                      ) {
    if (LOGALL) printf("%s\n",__func__);
    *p_doc_id=0; *p_num_res=0; *p_query_ids=0;
    if (LOG) printf("_docResultsMap for all results\n");
    if (LOG) logMap(_docResultsMap);
    AllDocsResultsMap::iterator it=_docResultsMap.end();
    it--;
    unsigned int key = it->first;
    if (_docResultsMap.size()==0 || key == 0){
        if (LOG)  printf( " GetNextAvailRes: EC_NO_AVAIL_RES %d %d ", *p_doc_id, *p_num_res );
        return EC_NO_AVAIL_RES;
    } else {

        SingleDocResultSet* result = it->second;

        _docResultsMap.erase(key);
        SearchTree::Instance()->removeDocFromMatchedWordsMap(key);

        *p_doc_id = key;
        //remove the 0 entry
        if (LOG) printf("SingleDocResultSet before 0-removal\n");
        if (LOG) printSetOfInts(result);
        if (LOG) printf("-----\n");

        result->erase(0);
        if (LOG) printf("SingleDocResultSet after 0-removal\n");
        if (LOG) printSetOfInts(result);
        if (LOG) printf("-----\n");
        unsigned int* results_array = SetToArray(result);
        unsigned int size = (unsigned int)result->size();
        *p_num_res = size;
        *p_query_ids = results_array;
        if (LOG)  printf( " GetNextAvailRes: %d, %d ", *p_doc_id, *p_num_res );
        return EC_SUCCESS;
    }
}

#pragma mark - adding new results


ErrorCode DocResults::AddResult (DocID p_doc_id, unsigned int p_num_res, std::vector<unsigned int> p_query_ids) {
    //TODO
	return EC_SUCCESS;
}

ErrorCode DocResults::AddToResult (DocID p_doc_id, unsigned int p_query_id) {
    if (LOGALL) printf("%s %d %d\n",__func__,p_doc_id, p_query_id);
    // retrieve the docResultsSet from our map of doc results
    AllDocsResultsMap::iterator found;
    found = _docResultsMap.find(p_doc_id);

    if (found == _docResultsMap.end())
        // we don't yet have any results for this document
        //so we need to create a new DocResultsSet
    {
        SingleDocResultSet* d_result = new SingleDocResultSet;
        if (p_query_id>0) {
            //if p_query_id = 0, this should initialise a new entry with 0 wordcount.
            d_result->insert(p_query_id);
        }
        _docResultsMap[p_doc_id] = d_result;

    } else {
        SingleDocResultSet* result = _docResultsMap[p_doc_id];
        result->insert(p_query_id);
        printSetOfInts(result);
    }
    return EC_SUCCESS;
}


#pragma mark - old code (used C dynamic array)

/*
ErrorCode DocResults::AddToResult1 (DocID p_doc_id, unsigned int p_query_id) {

    // add items
    map<unsigned int, DocResult>::iterator found = _docResultsMap.find(p_doc_id);

    if (found == _docResultsMap.end()) // this result is empty
    {
        DynamicArray d_array;
        d_array.elements[0] = p_query_id;
        d_array.num_elements = 1;
        d_array.num_allocated = 1;
        DocResult d_result;
        d_result._p_doc_id = p_doc_id;
        d_result._p_num_res = 1;
        d_result._p_query_ids = d_array;
        _docResultsMap[p_doc_id] = d_result;
        //_docResultsKeys->push_back(p_doc_id);
    } else {
        DocResult result = _docResultsMap[p_doc_id];
        result._p_num_res = AddToArray(result._p_query_ids,p_query_id);
    }

    return EC_SUCCESS;
}
*/


