//
//  DocResults.h
//  sigmod
//
//  Created by jonathan on 06/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#ifndef __sigmod__DocResults__
#define __sigmod__DocResults__

#include <iostream>
#include <vector>
#include "core.h"


class DocResults {
private:
    //singleton pattern, we only ever want ONE DocResults class.

    /**
     *  useage
     *  DocResults::Instance()->getResult(...);
     */

    std::vector<std::vector<unsigned int>*> _results;
    DocResults ();
    DocResults(DocResults const&){};             // copy constructor is private
    //SearchTree& operator=(SearchTree const&){};  // assignment operator is private
    static DocResults* m_pInstance;
public:
    static DocResults* Instance();

    ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids);
    
};
#endif /* defined(__sigmod__DocResults__) */
