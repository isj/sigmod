//
//  SearchNode.cpp
//  sigmod
//
//  Created by jonathan on 03/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#include "SearchNode.h"
#include "SearchTree.h"
using namespace std;


SearchNode::SearchNode() {
    //root node constructor
    _depth = 0;
}

SearchNode::~SearchNode() {
    //destructor
    //destroy class variables
    _depth = 0;
}

SearchNode::SearchNode (       QueryID  query_id
                        ,   const char* query_str
                        ,    MatchType  match_type
                        , unsigned int  match_dist
                        , unsigned int  query_str_idx
                        , SearchNode*   parent_node
                        ) {
    printf("SearchNode::%s\n",__func__);
    _parent_node = parent_node;
    printf("%p parent depth %d\n",this,_parent_node->_depth);
    _depth = (_parent_node->_depth)+1;
    printf("%p my depth %d\n",this,_depth);
    match.hamming = new vector<int>*[3];
    match.hamming[0]= new vector<int>();
    match.hamming[1]= new vector<int>();
    match.hamming[2]= new vector<int>();
    match.edit = new vector<int>*[3];
    match.edit[0]= new vector<int>();
    match.edit[1]= new vector<int>();
    match.edit[2]= new vector<int>();

    this->addQuery(query_id, query_str, match_type, match_dist,query_str_idx);
}


void SearchNode::addQuery(       QueryID  query_id
                          ,   const char* query_str
                          ,    MatchType  match_type
                          , unsigned int  match_dist
                          , unsigned int  query_str_idx
                          ) {

    printf("SearchNode::%s\n",__func__);
    printf("depth::%d\n",_depth);



    if (query_str[_depth+query_str_idx]=='\0' || query_str[_depth+query_str_idx]==' ') {
        //we have reached the last letter - mark this node as a "terminator"
        _terminator = true;

        //now we need to record the search query index number against it's search type.

        switch (match_type) {
            case 0: //exact match
                match.exact.push_back(query_id);
                break;
            case 1:
                match.hamming[match_dist-1]->push_back(query_id);
                break;
            case 2:
                match.edit[match_dist-1]->push_back(query_id);
                break;

            default:
                cout << "invalid match_type error\n";
                break;
        }
        if (query_str[_depth+query_str_idx]==' ') {
            //we have more words to add
            query_str_idx = _depth+query_str_idx+1;
            if (query_str[query_str_idx] == '\0') {
                printf("warning: appear to have a nil search query word\n");
            } else {
                SearchTree* tree = SearchTree::Instance();
                tree->addQuery(query_id, query_str, match_type, match_dist,query_str_idx);
            }

        }
    } else {
        //we have not reached the end of the word, keep building...
        if (_child_letters[query_str[_depth+query_str_idx]]==0) {
            //need to create a new child

            printf("creating search node for next letter:%c\n", query_str[_depth+query_str_idx]);
            SearchNode* next_letter = new SearchNode(  query_id
                                                , query_str
                                                , match_type
                                                , match_dist
                                                , query_str_idx
                                                , this
                                                );
            _child_letters[query_str[_depth+query_str_idx]] = next_letter;
        }   else {
            SearchNode* node =_child_letters[query_str[_depth+query_str_idx]];
            node->addQuery(query_id, query_str, match_type, match_dist,query_str_idx);
        }
        
        
    }
}


