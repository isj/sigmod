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

class DocResults {
private:
    //singleton pattern, we only ever want ONE searchTree.

    /**
     *  useage
     *  SearchNode::Instance()->addQuery(...);
     */

    std::vector<std::vector<int>*> _results;
    DocResults ();
    DocResults(DocResults const&){};             // copy constructor is private
    //SearchTree& operator=(SearchTree const&){};  // assignment operator is private
    static DocResults* m_pInstance;
public:
    static DocResults* Instance();

    

};

    

#endif /* defined(__sigmod__DocResults__) */
