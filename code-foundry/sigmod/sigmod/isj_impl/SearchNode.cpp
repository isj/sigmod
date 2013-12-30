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
#include "DataManager.h"
#import "sigmod_utils.h"
using namespace std;



SearchNode::SearchNode() {
    //root node constructor
    _depth = 0;
    _child_count = 0;
}



SearchNode::SearchNode (       QueryID  query_id
                        ,   const char* query_str
                        ,    MatchType  match_type
                        , unsigned int  match_dist
                        , unsigned int  query_str_idx
                        ,         char  query_word_counter
                        , SearchNode*   parent_node
                        ) {
    _parent_node = parent_node;
    _depth = (_parent_node->_depth)+1;

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

    _branch_matches.exact = false;
    _branch_matches.hamming1 = 0;
    _branch_matches.hamming2 = 0;
    _branch_matches.hamming3 = 0;
    _branch_matches.edit1 = 0;
    _branch_matches.edit2 = 0;
    _branch_matches.edit3 = 0;


}

SearchNode::~SearchNode() {
    delete _match.hamming[0];
    delete _match.hamming[1];
    delete _match.hamming[2];
    delete _match.hamming;
    delete _match.edit[0];
    delete _match.edit[1];
    delete _match.edit[2];
    delete _match.edit;
}


void SearchNode::addQuery(       QueryID  query_id
                          ,   const char* query_str
                          ,    MatchType  match_type
                          , unsigned int  match_dist
                          , unsigned int  query_str_idx
                          ,         char  query_word_counter
                          ) {

  //  if (LOG) printf("SearchNode::%s\n",__func__);
 //   if (LOG) printf("depth::%d\n",_depth);


    if (query_str[_depth+query_str_idx]=='\0' || query_str[_depth+query_str_idx]==' ') {
        //we have reached the last letter - mark this node as a "terminator"
        _terminator = true;

        query_word_counter++;

        //make a query_ref from the query_id and the word_counter
        //this allows us to retain both the queryID and the word number from the query in a single int.
        unsigned int query_ref = makeQueryRef(query_id, query_word_counter);


        //now we need to record the search query index number against it's search type.

        switch (match_type) {
            case 0: //exact match
                _match.exact.push_back(query_ref);
                break;
            case 1:
                _match.hamming[match_dist-1]->push_back(query_ref);
                break;
            case 2:
                _match.edit[match_dist-1]->push_back(query_ref);
                break;

            default:
                if (LOG) cout << "invalid match_type error\n";
                break;
        }
        //_match.all.push_back(query_ref);
        incrementBranchMatches(match_type, match_dist,kIncrementTypeAdd);


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

            //ADDBACK AFTER WE FIX RECURSION BUG
            DataManager::Instance()->addQueryToMap(query_id, query_word_counter);

        }
    } else {
        //we have not reached the end of the word, keep building...
        if (_child_letters[query_str[_depth+query_str_idx]]==0) {
            //need to create a new child
            SearchNode* next_letter = new SearchNode(  query_id
                                                 , query_str
                                                , match_type
                                                , match_dist
                                                , query_str_idx
                                                , query_word_counter
                                                , this
                                                );
            _child_letters[query_str[_depth+query_str_idx]] = next_letter;
            _child_count++;
        }

        SearchNode* node =_child_letters[query_str[_depth+query_str_idx]];
        node->addQuery ( query_id
                        , query_str
                        , match_type
                        , match_dist
                        , query_str_idx
                        , query_word_counter);

    }
}

void SearchNode::incrementBranchMatches (MatchType match_type, unsigned int match_distance, IncrementType increment) {

    switch (match_type) {
        case MT_EXACT_MATCH:
            _branch_matches.exact += increment;
            break;
        case MT_HAMMING_DIST:
            switch (match_distance) {
                case 1:
                    _branch_matches.hamming1 += increment;
                    break;
                case 2:
                    _branch_matches.hamming2 += increment;
                    break;
                case 3:
                    _branch_matches.hamming3 += increment;
                    break;

            }
        case MT_EDIT_DIST:
            switch (match_distance) {
                case 1:
                    _branch_matches.edit1 += increment;
                    break;
                case 2:
                    _branch_matches.edit2 += increment;
                    break;
                case 3:
                    _branch_matches.edit3 += increment;
                    break;
            }
            break;
    }

    if (_depth != 0) {
        _parent_node->incrementBranchMatches(match_type, match_distance, increment);
    }
}


void validate (vector<int> match_vector) {

}

void print_match_vector (vector<int> match_vector){
    if (LOG) {

    if (match_vector.size()>0) {
        for (int i =0; i<match_vector.size(); i++)
            if (DataManager::Instance()->isValidQueryID(match_vector[i])) {
              cout <<match_vector[i]<<" ";
            }
    } else {
        cout <<"- ";
    }
    }
}

void SearchNode::print_search_queries() {
    if (LOG) {
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

Match SearchNode::match () {
    return _match;
}

/*
void SearchNode::reportResult (DocID doc_id) {


    std::string string = this->string();
    if (MATCHED_MAP && SearchTree::Instance()->stringIsInMatchMap(doc_id, string)) {
      if (LOG)  cout << string << " is already in matched map for doc id "<<doc_id<<", returning" << endl;
      return;
    }
   if (LOG)  cout << endl <<  "found match doc "<< doc_id  << " " << string << endl;
    if (MATCHED_MAP) SearchTree::Instance()->addStringToMatchMap(doc_id, this->string());
    bool hit = false;
    for (int i=0;i<_match.all.size();i++) {
        QueryID query = _match.all.at(i);
        if (SearchTree::Instance()->isValidQuery(query)) {
            hit = true;
            SearchTree::Instance()->decrementQueryInDocumentMap(doc_id, query);
        } else {

            //we could clean up here...
            //if relevant queries have been removed we should destroy this node?

        }
    }
    if (hit==false){
        //all our queries are invalid, self-destruct
       if (DELETE_NODES) this->remove();
    } else {
        int x = 1;
        x++;
    }


}
 */





bool SearchNode::hasChildren() {
    bool result = false;
    if (_child_count>0) {
        result = true;
    }
    return result;
}

bool SearchNode::isTerminator() {
    return _terminator;
}

void SearchNode::removeTerminator() {
    _terminator = false;
}

void SearchNode::remove() {
    _parent_node->removeChild(this);
}

void SearchNode::removeChild(SearchNode* child) {
    if (LOG) printf("removing letter %c\n", child->_letter);
    _child_letters[child->_letter] = nullptr;
    _child_count--;
    child->~SearchNode();
    child = nullptr;
    if ((_child_count ==0) && !_terminator) {
        this->remove();
    }
}

SearchNode** SearchNode::getChildren() {
    return _child_letters;
}

SearchNode* SearchNode::child(unsigned int child_index) {
    SearchNode* node =  _child_letters[child_index];
    return node;
}


char SearchNode::letter() {
    return _letter;
}


unsigned int SearchNode::depth() {
    return _depth;
}

BranchMatchTypes  SearchNode::branch_matches() {
    return _branch_matches;
}

unsigned int SearchNode::branchHasEditMatches() {
    unsigned int result =
        (_branch_matches.edit3 > 0)? 3:
        (_branch_matches.edit2 > 0)? 2:
        (_branch_matches.edit1 > 0)? 1:0;
    return result;
}

unsigned int SearchNode::branchHasHammingMatches() {
    unsigned int result =
        (_branch_matches.hamming3 > 0)? 3:
        (_branch_matches.hamming2 > 0)? 2:
        (_branch_matches.hamming1 > 0)? 1:0;
    return result;
}

bool SearchNode::branchHasExactMatches() {
    return _branch_matches.exact;

}









