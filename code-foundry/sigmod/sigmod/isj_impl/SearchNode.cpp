//
//  SearchNode.cpp
//  sigmod
//
//  Created by jonathan on 03/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#include "SearchNode.h"
#include "SearchTree.h"
#include "WordTumbler.h"
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
    if (LOG) printf("SearchNode::%s\n",__func__);
    _parent_node = parent_node;
    if (LOG) printf("%p parent depth %d\n",this,_parent_node->_depth);
    _depth = (_parent_node->_depth)+1;
    if (LOG) printf("%p my depth %d\n",this,_depth);

    // _match stores QueryIDs of search queries

    //QueryIDs of exact match queries
    _match.hamming = new vector<int>*[3];


    //QueryIDs of hamming match queries
    _match.hamming[0]= new vector<int>(); //match_distance 1
    _match.hamming[1]= new vector<int>(); //match_distance 2
    _match.hamming[2]= new vector<int>(); //match_distance 3

    //QueryIDs of edit match queries
    _match.edit = new vector<int>*[3];
    _match.edit[0]= new vector<int>(); //match_distance 1
    _match.edit[1]= new vector<int>(); //match_distance 2
    _match.edit[2]= new vector<int>(); //match_distance 3

    this->_letter = query_str[_depth+query_str_idx-1];
    this->addQuery(query_id, query_str, match_type, match_dist,query_str_idx);
}


void SearchNode::addQuery(       QueryID  query_id
                          ,   const char* query_str
                          ,    MatchType  match_type
                          , unsigned int  match_dist
                          , unsigned int  query_str_idx
                          ) {

    if (LOG) printf("SearchNode::%s\n",__func__);
    if (LOG) printf("depth::%d\n",_depth);



    if (query_str[_depth+query_str_idx]=='\0' || query_str[_depth+query_str_idx]==' ') {
        //we have reached the last letter - mark this node as a "terminator"
        _terminator = true;

        //now we need to record the search query index number against it's search type.

        switch (match_type) {
            case 0: //exact match
                _match.exact.push_back(query_id);
                break;
            case 1:
                _match.hamming[match_dist-1]->push_back(query_id);
                break;
            case 2:
                _match.edit[match_dist-1]->push_back(query_id);
                break;

            default:
                if (LOG) cout << "invalid match_type error\n";
                break;
        }
        if (query_str[_depth+query_str_idx]==' ') {
            //we have more words to add
            query_str_idx = _depth+query_str_idx+1;
            if (query_str[query_str_idx] == '\0') {
                if (LOG) printf("warning: appear to have a nil search query word\n");
            } else {
                SearchTree* tree = SearchTree::Instance();
                tree->addQuery(query_id, query_str, match_type, match_dist,query_str_idx);
            }

        }
    } else {
        //we have not reached the end of the word, keep building...
        if (_child_letters[query_str[_depth+query_str_idx]]==0) {
            //need to create a new child

            if (LOG) printf("creating search node for next letter:%c\n", query_str[_depth+query_str_idx]);
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

void SearchNode::addDocument(        DocID  doc_id
                 ,  const char* doc_str
                 ,         int  doc_str_idx
                 ) {
    if (doc_str[_depth+doc_str_idx]=='\0' || doc_str[_depth+doc_str_idx]==' ') {
        //we have reached the last letter - did we get a match?
    }



}

void SearchNode::matchWord (  DocID doc_id
                , const char* doc_str
                , unsigned int word_start_idx
                , unsigned int word_length
                            ) {

}


void print_match_vector (vector<int> match_vector){
    if (match_vector.size()>0) {
        for (int i =0; i<match_vector.size(); i++)
            cout <<match_vector[i]<<" ";
    } else {
        cout <<"- ";
    }
}

void SearchNode::print_search_queries() {
    cout << "  x ";
    if(_match.exact.size()==0) cout <<"- ";
    for (int i =0; i<_match.exact.size(); i++)
        cout << _match.exact[i]<<" ";

    cout << "h ";
    for (int i=0; i<3;i++) {
        vector<int> ham = *_match.hamming[i];
        print_match_vector(*_match.hamming[i]);
    }

    cout << "e ";
    for (int i=0; i<3;i++) {
        vector<int> ham = *_match.edit[i];
        print_match_vector(*_match.edit[i]);
    }

}



void SearchNode::print() {
    //this print function is only run by the root node, as it has no letters to start with
    int alphabet_length = 128;
    for (int i=0; i< alphabet_length; i++ ) {
        if (_child_letters[i]) {
            _child_letters[i]-> print (nullptr);
        }
    }
}

void SearchNode::print (  char letters[] ) {
    //this print function is called recursively on each found child letter
    char new_letters[_depth+1];  //lenth is length-of-word + 1 (for terminating '\0')
    if (letters!=nullptr) {
        for (int i = 0; i<_depth; i++) {
            new_letters[i]= letters[i];
        }
    }
    new_letters[_depth-1] = _letter;
    new_letters[_depth] = '\0';
    if (_terminator == true ) {
        printf("%s",new_letters);
        print_search_queries();
        cout << "\n";

    }

    for (int i=kFirstASCIIChar; i<=kLastASCIIChar; i++ ) {
        if (_child_letters[i]) {
            _child_letters[i]-> print (new_letters);
        }
    }
    

}




