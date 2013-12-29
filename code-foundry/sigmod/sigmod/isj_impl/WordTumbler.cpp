//
//  Word_cpp
//  sigmod
//
//  Created by jonathan on 22/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#include "WordTumbler.h"
#include "DataManager.h"

using namespace std;

#pragma mark ----------------------
#pragma mark - initialise
#pragma mark ----------------------

 WordTumbler::WordTumbler (DocID doc_id,  char* word,int word_length, int limit) {
    _node =SearchTree::Instance()->root();
    _doc_id = doc_id;
    _doc_word = word;
    _doc_word_length = word_length;
    _exact.test = true;
    _exact.cost = 1;
    _hamming.max_cost = limit;
    _edit.max_cost = limit;
    _hamming.current_cost = 0;
    _edit.previous_row = nullptr;

    //build first results row for edit match
    int row[word_length+1];
    for (int i = 0; i< (word_length+1); i++) {
        row[i] = i;
    }
    _edit.previous_row = row;
}

#pragma mark ----------------------
#pragma mark - tumbling
#pragma mark ----------------------

void WordTumbler::tumble() {
    bool continue_searching = true;
    testExact();
    testHamming();
    testEdit();
    testResults();
    if (continue_searching) {
        searchChildNodes();
    }
}

void WordTumbler::searchChildNodes() {
    std::map<char, SearchNode*> map = mapOfChildNodesToFollow();
    std::map<char,SearchNode*>::iterator it;
    for (it = map.begin(); it != map.end();it++) {
        SearchNode* child_node = it->second;
        printf("inserting letter %c\n",child_node->letter());
        _node = child_node;
        if (_exact.test) _exact.test = child_node->branchHasExactMatches();
        _exact.cost = (_exact.test) ? 1 : 0;
        _hamming.max_cost = child_node->branchHasHammingMatches();
        _edit.max_cost = child_node->branchHasEditMatches();
        if (_exact.cost + _hamming.max_cost + _edit.max_cost > 0) {
            tumble();
        }
    }
}

std::map<char, SearchNode*> WordTumbler::mapOfChildNodesToFollow () {
    //TODO - implement this as data structure in node
    std::map<char, SearchNode*> map;
    std::map<char,SearchNode*>::iterator it = map.begin();  //using iterator for position hints, more efficient insert
    for (int i=kFirstASCIIChar; i<=kLastASCIIChar; i++ ) {
        if (_node->child(i) != nullptr) {
            printf("inserting letter %c\n",_node->child(i)->letter());
            map.insert ( it,std::pair<char,SearchNode*>(i,_node->child(i)) );
        }
    }
    return map;
}

#pragma mark ----------------------
#pragma mark - match tests
#pragma mark ----------------------

bool WordTumbler::lettersMatch() {
    return (_doc_word[_node->depth()-1] == _node->letter());
}

void WordTumbler::testExact(){
    if (_exact.test) {
        cout << " exact test " << _doc_id << " word "<<_doc_word << " node_string "<<_node->string()<<endl;
        if (!lettersMatch()) {
            _exact.test = false;
        }
    }
}

void WordTumbler::testHamming(){
    if (_hamming.max_cost > 0) {
        printf("hamming test\n");
        if (_hamming.current_cost <= _hamming.max_cost)
            if (!lettersMatch()) {
                _hamming.current_cost ++;
            }
    }
}

/**
 * void WordTumbler::testEdit(){
 *
 *  http://stevehanov.ca/blog/index.php?id=114
 *
 * returns a _list of words_ from the searchtree
 * that are less than the given max distance from the target word
 * we can extend this by checking the node indexes for queries
 */

void WordTumbler::testEdit(){
    if (_edit.max_cost > 0) {
        printf("edit test\n");

        int columns = (int)_doc_word_length+1;
        int currentRow[columns];
        currentRow[0] = _edit.previous_row[0]+1;
        int replaceCost = 0;

        //edit match algorith
        // if (match_edit == true) {
        for (int column=1; column <= columns; column++) {
            int insertCost = currentRow[column-1] + 1;
            int deleteCost = _edit.previous_row[column] + 1;
            if (_doc_word[column -1] != _node->letter()) {
                replaceCost = _edit.previous_row[column-1]+1;
            } else {
                replaceCost = _edit.previous_row[column-1];
            }
            int costToAdd = min(insertCost,min(deleteCost, replaceCost));
            currentRow[column] = costToAdd;
        }
        if (LOG) {
            printf("current row  %c            ",_node->letter());
            for (int i=0; i<columns; i++) {
                printf("%d",currentRow[i]);
            }
            printf("\n");
        }
        //}
    }

}


#pragma mark ----------------------
#pragma mark -  results test
#pragma mark ----------------------


void WordTumbler::testResults(){
    if (_node->isTerminator()) {
        printf("test results\n");
        int edit_distance = 0;


        //_node->checkEditResult (_doc_id,  edit_distance, _doc_word_length,  _hamming.current_cost,  _exact.test);

    }
}





void addToResults (std::vector<int>& resultsRef, std::vector<int> additions) {
    for (int i = 0; i < additions.size();i++) {
        resultsRef.push_back(additions[i]);
    }

}



void WordTumbler::logResult () {

    /**
     *  we got a result
     *
     *  we need to know whether the match-type is in our matches_array
     * then check off any associated query_ids
     */
    std::string string = _node->string();




    vector<int> results;
    vector<int>& resultsRef = results;

    //an exact match also satisfies all edit and hamming matches
    //an edit distance of E also satisfies edit distance E+1
    //a hamming distance of H also satisfies hamming distance H+1
    //an edit distance of E also satisfies a hamming distance of H if doc_word and query_word are same length (length_match==true)

    cout <<"checking edit distance... " <<endl;

    if (_edit.current_cost >= 1) {
        addToResults(resultsRef, *_node->match().edit[0]);
        if (_edit.current_cost >= 2) {
            addToResults(resultsRef, *_node->match().edit[1]);
            if (_edit.current_cost >= 3) {
                addToResults(resultsRef, *_node->match().edit[2]);
            }
        }
    }
    cout <<"checking hamming distance... " <<endl;
        if (_hamming.current_cost >= 1) {
            addToResults(resultsRef, *_node->match().hamming[0]);
            if (_hamming.current_cost >= 2) {
                addToResults(resultsRef, *_node->match().hamming[1]);
                if (_hamming.current_cost  >= 3) {
                    addToResults(resultsRef, *_node->match().hamming[2]);
                }
            }
        }
    if (_exact.test) addToResults(resultsRef, _node->match().exact);


    if (results.size()>0) {
        bool hit = false;
        for (int i=0;i<results.size();i++) {
            QueryID query_id = results.at(i);
            if (DataManager::Instance()->isValidQueryID(query_id)) {
                hit = true;
                DataManager::Instance()->decrementQueryInDocumentMap(_doc_id, query_id);
            }
        }
        if (hit==false){
            //TODO - fix
            //all our queries are invalid, self-destruct
            //this is not currently correct
            //we should only delete a node if all of it's _match arrays are empty
            //not just if THIS query gets us no hits
            //if (DELETE_NODES) _node->remove();
        } else {
            //just for breakpoints
            int x = 1;
            x++;
        }
    }
    
}



