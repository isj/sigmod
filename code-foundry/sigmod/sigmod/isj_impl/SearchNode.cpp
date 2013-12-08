//
//  SearchNode.cpp
//  sigmod
//
//  Created by jonathan on 03/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#include "SearchNode.h"
#include "SearchTree.h"
#include "DocResults.h"
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
                        , unsigned int  query_word_counter
                        , SearchNode*   parent_node
                        ) {
    if (LOG) printf("SearchNode::%s\n",__func__);
    _parent_node = parent_node;
    if (LOG) printf("%p parent depth %d\n",this,_parent_node->_depth);
    _depth = (_parent_node->_depth)+1;
    if (LOG) printf("%p my depth %d\n",this,_depth);

    // _match stores QueryIDs of search queries

    //QueryIDs of exact match queries
    //_match.exact = new vector<int>();

    //QueryIDs of hamming match queries
    _match.hamming = new vector<int>*[3];
    _match.hamming[0]= new vector<int>(); //match_distance 1
    _match.hamming[1]= new vector<int>(); //match_distance 2
    _match.hamming[2]= new vector<int>(); //match_distance 3

    //QueryIDs of edit match queries
    _match.edit = new vector<int>*[3];
    _match.edit[0]= new vector<int>(); //match_distance 1
    _match.edit[1]= new vector<int>(); //match_distance 2
    _match.edit[2]= new vector<int>(); //match_distance 3

    //_match.all = new vector<int>();


    this->_letter = query_str[_depth+query_str_idx-1];
    this->addQuery ( query_id
                   , query_str
                   , match_type
                   , match_dist
                   , query_str_idx
                   , query_word_counter);
}


void SearchNode::addQuery(       QueryID  query_id
                          ,   const char* query_str
                          ,    MatchType  match_type
                          , unsigned int  match_dist
                          , unsigned int  query_str_idx
                          , unsigned int  query_word_counter
                          ) {

  //  if (LOG) printf("SearchNode::%s\n",__func__);
 //   if (LOG) printf("depth::%d\n",_depth);

    if (_depth==0) {
        SearchTree::Instance()->print();
        for (int i=kFirstASCIIChar; i<=kLastASCIIChar; i++ ) {
            //printf("_child_letter %p",_child_letters[i]);

        }


    }

    if (query_str[_depth+query_str_idx]=='\0' || query_str[_depth+query_str_idx]==' ') {
        query_word_counter++;
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
        _match.all.push_back(query_id);

        if (query_str[_depth+query_str_idx]==' ') {
            //we have more words to add
            query_str_idx = _depth+query_str_idx+1;
            if (query_str[query_str_idx] == '\0') {
                if (LOG) printf("warning: appear to have a nil search query word\n");
            } else {
                SearchTree* tree = SearchTree::Instance();
                tree->addQuery(  query_id
                               , query_str
                               , match_type
                               , match_dist
                               , query_str_idx
                               , query_word_counter
                               );
            }

        } else {
            //we have added the last word, need to register the query and it's wordcount
            //with the tree's _query_ids_map
            SearchTree::Instance()->addQueryToMap(query_id, query_word_counter);
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
                                                , query_word_counter
                                                , this
                                                );
            _child_letters[query_str[_depth+query_str_idx]] = next_letter;
        }   else {
            SearchNode* node =_child_letters[query_str[_depth+query_str_idx]];
            node->addQuery ( query_id
                           , query_str
                           , match_type
                           , match_dist
                           , query_str_idx
                           , query_word_counter);
        }
        
        
    }
}

void SearchNode::addDocument(        DocID  doc_id
                             ,  const char* string
                             ,         int  string_idx
                             ) {

    if (string[string_idx+_depth]=='\0' || string[string_idx+_depth]==' ') {
        //we have reached a word ending - are we on a terminator node?
        if (_terminator == true) {
            //we have a match - record it...
//            if (LOG) printf("\nfound match doc %d idx %d ",doc_id, string_idx);
//            for (int i = 1; i<=_depth; i++) {
//               if (LOG)  printf("%c",getLetterFromParentForDepth(i));
//            }
      //      cout << endl;
            this->reportResult(doc_id);

            
            

        }
        if (string[string_idx+_depth]==' ') {
            //we have more words to add
            string_idx = string_idx+_depth+1;
            if (string[string_idx] == '\0') {
                if (LOG) printf("warning: appear to have a nil search query word\n");
            } else {
                SearchTree* tree = SearchTree::Instance();
                tree->addDocument(doc_id, string, string_idx);
            }
        }
    } else {
        //we have not reached the end of the word...
        if (_child_letters[string[string_idx+_depth]]==0) {
            //no more nodes, stop the search
        }   else {
            //found a matching child node, keep searching
            SearchNode* node =_child_letters[string[string_idx+_depth]];
            node->addDocument(doc_id, string, string_idx);
        }
        
        
    }



}

void SearchNode::matchWord (  DocID doc_id
                , const char* doc_str
                , unsigned int word_start_idx
                , unsigned int word_length
                            ) {

    

}

void validate (vector<int> match_vector) {

}

void print_match_vector (vector<int> match_vector){
    if (match_vector.size()>0) {
        for (int i =0; i<match_vector.size(); i++)
            if (SearchTree::Instance()->isValidQuery(match_vector[i])) {
                cout <<match_vector[i]<<" ";
            }
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


void SearchNode::print () {
    //performs a depth-first search through the tree looking for terminator nodes
    if (_terminator == true ) {
        cout << this->string() <<" ";
        this-> print_search_queries();
        cout << endl;
    }
    for (int i=kFirstASCIIChar; i<=kLastASCIIChar; i++ ) {
        if (_child_letters[i]) {
            _child_letters[i]-> print ();
        }
    }
}

char SearchNode::getLetterFromParentForDepth(int depth) {
    //reach upwards from a node to get parent letters
    if (_depth==depth)
        return _letter;
    else
        return ( _parent_node->getLetterFromParentForDepth(depth));
}

string SearchNode::string() {
    std::string string;
    for (int i=1; i<=_depth; i++) {
        const char next_letter = this->getLetterFromParentForDepth(i);
        char next_letters[ ] = " ";
        next_letters[0]=next_letter;
        string.append( next_letters );
    }

    return string;
}

void SearchNode::reportResult (DocID doc_id) {

    /**
     *  - add found word to "document's found set"
     *  - find which query IDs we match against
     *  - decrement our documents' queryIDsIndex for those IDs
     */
    std::string string = this->string();
    if (SearchTree::Instance()->stringIsInMatchMap(doc_id, string)) {
        cout << string << " is already in match map for doc id "<<doc_id<<", returning" << endl;
      return;
    }
    cout << "found match doc "<< doc_id  << " " << string << endl;
    SearchTree::Instance()->addStringToMatchMap(doc_id, this->string());
    
    for (int i=0;i<_match.all.size();i++) {
        QueryID query = _match.all.at(i);
        SearchTree::Instance()->decrementQueryInDocumentMap(doc_id, query);
    }


}











