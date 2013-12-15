//
//  SearchTree.cpp
//  sigmod
//
//  Created by jonathan on 03/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#include "SearchTree.h"
#include "sigmod_types.h"
#include "DocResults.h"
#include "sigmod_utils.h"

using namespace std;

// Global static pointer used to ensure a single instance of the class.

SearchTree* SearchTree::m_pInstance = NULL;

/**     This function is called to create an instance of the class.
 *	    Calling the constructor publicly is not allowed. The constructor
 *	    is private and is only called by this Instance function.
 */

SearchTree* SearchTree::Instance()
{

    if (!m_pInstance)   // Only allow one instance of class to be generated.
        m_pInstance = new SearchTree;
    return m_pInstance;
}


SearchTree::SearchTree() {
    //constructor
    //create root node

     _root = new SearchNode();

#ifdef USING_LIBCPP
    _query_ids = new unordered_set<QueryID>();
#else
    _query_ids =  new set<QueryID>();
#endif

    _query_ids_map      = new map < QueryID, unsigned  int > ();

    _matched_words_map  = new map < DocID,  set < string > > ();

    _query_ids_map_per_document = new map < DocID, map < QueryID, unsigned int >  > ();

 }




ErrorCode SearchTree::addQuery (  QueryID query_id
                                , const char* query_str
                                , MatchType match_type
                                , unsigned int match_dist
                                , unsigned int query_str_idx
                                , unsigned int query_word_counter
                                ) {
    if (LOGALL) printf("%s\n",__func__);
    _query_ids->insert(query_id);

    //start the word count at 0, we will increment it as we add the query

    _root->addQuery ( query_id
                    , query_str
                    , match_type
                    , match_dist
                    , query_str_idx
                    , query_word_counter
                    );

    return EC_SUCCESS;
}

ErrorCode SearchTree::addDocument  (  DocID doc_id
                                   , const char* doc_str
                                   , int doc_str_idx
                                   ) {
    //initialise _docResultsMap with a new entry for doc_id, set to 0 results.
    if (LOGALL) printf("%s %d %d\n",__func__,doc_id,doc_str_idx);
    if (doc_str_idx ==0) DocResults::Instance()->AddToResult(doc_id, 0);

    map < DocID, map < QueryID, unsigned int >  >::iterator found;
    found = _query_ids_map_per_document->find(doc_id);

    if (found == _query_ids_map_per_document->end()) {
        //set up the document's __query_ids_map_per_document map
        map < QueryID, unsigned int > query_map;
        map < QueryID, unsigned int >::iterator it;
        for ( it = _query_ids_map->begin(); it != _query_ids_map->end(); it++) {
            query_map.insert(std::pair
                             < QueryID, unsigned int >
                             ( it->first, it->second)
                             );
        }
        _query_ids_map_per_document->insert(std::pair
                                            < DocID, map < QueryID, unsigned int > >
                                            (doc_id,query_map)
                                            );
    }
       return EC_SUCCESS;
}




ErrorCode removeNode (SearchNode* node) {
    if (node->hasChildren()) {
        node->removeTerminator();
    } else {
        node->remove();
    }
    return EC_SUCCESS;
}

#pragma mark - printing

void SearchTree::print() {
    printf("\nprinting searchTree...\n");
    _root -> print();
    printf("\n");

}


#pragma mark - query accounting

void SearchTree::addQueryToMap (  QueryID query_id
                    , unsigned int word_count
                    ) {
    if (LOGALL) printf("%s\n",__func__);
    _query_ids_map->insert ( std::pair<QueryID,unsigned int>(query_id,word_count) );
    if (LOG) printMapOfIntInt(_query_ids_map);
}




void SearchTree::decrementQueryInDocumentMap (  DocID doc_id
                                              , QueryID query_id
                                              ) {
    if (LOGALL) printf("%s\n",__func__);
    _query_ids_map_per_document->at(doc_id).at(query_id)--;
    if (_query_ids_map_per_document->at(doc_id).at(query_id) == 0) {
        //all search query words have been found, we have a hit)
        DocResults::Instance()->AddToResult(doc_id, query_id);
    }
}

void SearchTree::addStringToMatchMap ( DocID doc_id
                                    , string word
                                    ) {
    if (LOGALL) printf("%s\n",__func__);
    if (_matched_words_map->find(doc_id) == _matched_words_map->end()) {
        set<string> string_set;
        string_set.insert(word);
        _matched_words_map->insert(std::pair<DocID,set<string> >(doc_id,string_set) );
        
    } else {
        _matched_words_map->at(doc_id).insert(word);
    }

    this->printMatchMap();

}

void SearchTree::removeDocFromMatchedWordsMap  (   DocID doc_id ) {
    _matched_words_map->erase(doc_id);
}


void SearchTree::printMatchMap() {
    if (LOG) printf("%s\n",__func__);
    printMapOfIntSetOfStrings(_matched_words_map);
}

bool SearchTree::stringIsInMatchMap (  DocID doc_id
                         , std::string word
                                     ) {
    if (LOGALL) printf("%s\n",__func__);
    map < DocID, set < std::string > >::iterator found;
    found = _matched_words_map->find(doc_id);

    if (found == _matched_words_map->end()) {
        return false;
    }

    set <std::string> stringset = found->second;
    //std::set<std::string>::iterator it;
    if ( stringset.find(word) == stringset.end() ) {
        return false;
    }
    return true;

}

int  SearchTree::numberOfQueries() {
    if (LOGALL) printf("%s\n",__func__);

    return (int)_query_ids->size();
}

#pragma mark - query validation

void SearchTree::removeQuery   (QueryID query_id) {
    /**
     *  we don't remove any nodes here, just the query index
     *  if a node's query indexes are all not found on the next match
     * , then we remove the node
     *  (so only if we search it again)
     */
    if (LOGALL) printf("%s\n",__func__);
    if (LOG) printf("removing query for query_ids ");
    //printSetOfInts(_query_ids);
    _query_ids->erase(query_id);
    _query_ids_map->erase ( query_id) ;
    if (LOG) printf("query removed: ");
    //printSetOfInts(_query_ids);

}

bool SearchTree::isValidQuery(QueryID query_id) {
    if (LOGALL) printf("%s\n",__func__);

    if ( _query_ids->count(query_id)==1) return true;
    else return false;
}


#pragma mark - unused

ErrorCode SearchTree::matchWord  (  DocID doc_id
                                  , const char* doc_str
                                  , unsigned int word_start_idx
                                  , unsigned int word_length
                                  ) {

    _root -> matchWord (  doc_id
                        , doc_str
                        , word_start_idx
                        , word_length
                        );
    return EC_SUCCESS;

}

ErrorCode SearchTree::startMatching (  DocID doc_id
                                     , const char* doc_start
                                     , int doc_results[]
                                     ) {
    return EC_SUCCESS;
}

ErrorCode SearchTree::tumbleWord  ( WordTumbler* wordTumbler
                                   ) {
    if (LOG) printf("%s\n",__func__);
    return EC_SUCCESS;
}

SearchNode* SearchTree::root (){
    return _root;
}
