//
//  DocResults.cpp
//  sigmod
//
//  Created by jonathan on 06/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#include "DocResults.h"
#include "sigmod_utils.h"

using namespace std;


// Global static pointer used to ensure a single instance of the class.

DocResults* DocResults::m_pInstance = NULL;

/**     This function is called to create an instance of the class.
 *	    Calling the constructor publicly is not allowed. The constructor
 *	    is private and is only called by this Instance function.
 */

DocResults* DocResults::Instance()
{
    if (LOG) printf("%s\n",__func__);

    if (!m_pInstance)   // Only allow one instance of class to be generated.
        m_pInstance = new DocResults;
    return m_pInstance;
}


DocResults::DocResults() {
    //constructor
    //create _results storage
    if (LOG) printf("%s\n",__func__);

    //initialise _docResultsMap with key "0"
    DocResult result;
    result._p_doc_id= 0;
    _docResultsMap[0]=result;
    //_docResultsKeys = new vector<unsigned int>();
}

ErrorCode DocResults::GetNextAvailRes ( DocID* p_doc_id
                                      , unsigned int* p_num_res
                                      , QueryID** p_query_ids
                                      ) {
    DocResultsMap::iterator it=_docResultsMap.end();
    unsigned int key = it->first;
    DocResult result = it->second;

    if (_docResultsMap.size()==0 || key == 0){
            if (LOG)  printf( " GetNextAvailRes: EC_NO_AVAIL_RES %d %d ", *p_doc_id, *p_num_res );
            return EC_NO_AVAIL_RES;
    }
    _docResultsMap.erase(key);
    p_doc_id = &result._p_doc_id;
    p_num_res = &result._p_num_res;
    p_query_ids = &result._p_query_ids.elements;
    if (LOG)  printf( " GetNextAvailRes: %d, %d ", *p_doc_id, *p_num_res );
	return EC_SUCCESS;
}



ErrorCode DocResults::AddResult (DocID p_doc_id, unsigned int p_num_res, std::vector<unsigned int> p_query_ids) {
    //TODO
	return EC_SUCCESS;
}

ErrorCode DocResults::AddToResult (DocID p_doc_id, unsigned int p_query_id) {

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


