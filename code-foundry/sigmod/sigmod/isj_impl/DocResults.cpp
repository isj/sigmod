//
//  DocResults.cpp
//  sigmod
//
//  Created by jonathan on 06/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#include "DocResults.h"
#include "sigmod_types.h"

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
    _results = *new vector<vector<unsigned int>*>();
}

ErrorCode DocResults::GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids) {
    vector<unsigned int> last_result = *_results.back();
    p_doc_id = &last_result.back(); //(?, last element of last _results vector)
    last_result.pop_back();
    p_num_res = &last_result.back();
    last_result.pop_back();

    //&p_query_ids = last_result;

	return EC_SUCCESS;

}



