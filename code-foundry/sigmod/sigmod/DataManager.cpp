//
//  DataManager.cpp
//  sigmod
//
//  Created by jonathan on 23/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#include "DataManager.h"
#include "DocResults.h"
#include "sigmod_utils.h"

using namespace std;

#pragma mark ----------------------
#pragma mark   singleton initialisation
#pragma mark ----------------------

// Global static pointer used to ensure a single instance of the class.

DataManager* DataManager::m_pInstance = NULL;

/**     This function is called to create an instance of the class.
 *	    Calling the constructor publicly is not allowed. The constructor
 *	    is private and is only called by this Instance function.
 */

DataManager* DataManager::Instance() {
    if (!m_pInstance)   // Only allow one instance of class to be generated.
        m_pInstance = new DataManager;
    return m_pInstance;
}


DataManager::DataManager() {
#ifdef USING_LIBCPP
    _query_ids = new unordered_set<QueryID>();
#else
    _query_ids =  new set<QueryID>();
#endif

    _query_ids_map      = new map < QueryID, unsigned  int > ();
    _query_ids_map_per_document = new map < DocID, map < QueryID, unsigned int >  > ();
    _map_of_sets_of_doc_words = new map < DocID, set < std::string > >();


}


#pragma mark ----------------------
#pragma mark   doc word set management
#pragma mark ----------------------


void DataManager::addWordToDocWordSet ( DocID doc_id
                                      , string word
                                      ) {
    if (LOGALL) printf("%s\n",__func__);
    if (_map_of_sets_of_doc_words->find(doc_id) == _map_of_sets_of_doc_words->end()) {
        set<string> string_set;
        string_set.insert(word);
        _map_of_sets_of_doc_words->insert(std::pair<DocID,set<string> >(doc_id,string_set) );

    } else {
        _map_of_sets_of_doc_words->at(doc_id).insert(word);
    }

}

bool DataManager::wordIsInDocWordSet (  DocID doc_id
                                     , std::string word
                                     ) {
    if (LOGALL) printf("%s\n",__func__);
    map < DocID, set < std::string > >::iterator found;
    found = _map_of_sets_of_doc_words->find(doc_id);

    if (found == _map_of_sets_of_doc_words->end()) {
        //this document is not in _matched_words_map
        return false;
    }

    set <std::string> stringset = found->second;
    //std::set<std::string>::iterator it;
    if ( stringset.find(word) == stringset.end() ) {
        //the document is in _matched_words_map, but the word is not
        return false;
    }
    return true;
    
}

void DataManager::removeDocWordSet  (   DocID doc_id ) {
    _map_of_sets_of_doc_words->erase(doc_id);
}


#pragma mark ----------------------
#pragma mark   query accounting
#pragma mark ----------------------

#pragma mark - when adding query to trie


void DataManager::addQueryToMap (  QueryID query_id
                    , unsigned int word_count
                                 ) {
    if (LOGALL) printf("%s\n",__func__);
    _query_ids->insert(query_id);
    _query_ids_map->insert ( std::pair<QueryID,unsigned int>(query_id,word_count) );
    if (LOG) printMapOfIntInt(_query_ids_map);

}

#pragma mark - when logging a search hit


void DataManager::decrementQueryInDocumentMap (  DocID doc_id
                                  , QueryID query_id
                                               ) {
    printf("%s %d %d\n",__func__,doc_id,query_id);
    _query_ids_map_per_document->at(doc_id).at(query_id)--;
    if (_query_ids_map_per_document->at(doc_id).at(query_id) == 0) {
        //all search query words have been found, we have a hit)
        DocResults::Instance()->AddToResult(doc_id, query_id);
    }


}


#pragma mark - is a query still valid?


bool DataManager::isValidQueryID(QueryID query_id) {
    if (LOGALL) printf("%s\n",__func__);

    if ( _query_ids->count(query_id)==1) return true;
    else return false;
}


void DataManager::removeQueryID   (QueryID query_id) {
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

int  DataManager::numberOfQueries() {
    if (LOGALL) printf("%s\n",__func__);
    return (int)_query_ids->size();
}


