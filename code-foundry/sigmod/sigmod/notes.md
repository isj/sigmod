__notes__

Exact match

looping MatchDocument x 1000 on an exact-match-only set of queries (4 x queries, 60 x documents)



Your program has successfully passed all tests.
docs: 960 queries: 993 end_queries: 1013
exact_match: 993 hamming_distance: 0 edit_distance: 0
Time=21111[21s:111ms]
Program ended with exit code: 0

Your program has successfully passed all tests.
docs: 960 queries: 406 end_queries: 1013
exact_match: 0 hamming_distance: 406 edit_distance: 0
Time=8367[8s:367ms]
Program ended with exit code: 0

Your program has successfully passed all tests.
docs: 960 queries: 521 end_queries: 1013
exact_match: 0 hamming_distance: 0 edit_distance: 521
Time=212353[3m:32s:353ms]
Program ended with exit code: 0

selvans code
exact_match: 993 hamming_distance: 406 edit_distance: 521
Time=880973[14m:40s:973ms]

default code
exact_match: 993 hamming_distance: 406 edit_distance: 521
Time=326319[5m:26s:319ms]
Program ended with exit code: 0



if(strcmp(query_word, doc_word)==0)  matching_word=true;

Your program has successfully passed all tests.
docs: 960 queries: 993 end_queries: 1013
exact_match: 993 hamming_distance: 0 edit_distance: 0
Time=20787[20s:787ms]
Program ended with exit code: 0

build a trie tree

Class Trie Tree {
     
     Trie_Node root;  //make sure to initalise ROOT with word_length 0.
     
     function insert_word (query_str, query_id, match_type, match_dist) {
     
     
     
     
     }


     


}

CLASS Trie Node
    char letter;
    Tie Node parent;
    int depth;
     char* [26] child_alphabet;   //initialise with 26 elements val oo
     vetor <int>word_lengths;
      bool terminator
        vector <int> q_exact_match   //query_ids of exact_match queries containting this word
        vector <int> q_hamming_1    //query_ids of hamming_match_1 queries containing this word
        vector <int> q_hamming_2   //query_ids of hamming_match_2 queries containing this word
        vector <int> q_edit_1   //query_ids of edit_distance_1 queries containing this word
        vector <int> q_edit_2   //query_ids of edit_distance_2 queries containing this word
    }
       vector <int> matching_docs  //do we need this?
       
       - void check_match_valid (query_id, match_type) {
          look up query_Id in list of eliminated queries
          if it is there, delete it from match_type
          
         // this way we don't need to delete eliminated entries.
         // OPTIMISATION=>may need to reconsider for deleting eliminated entries.

       }
       


       -void match_found (doc_id, query_id) {
            decrement the doc_id query_index
       }
       
       -void add_char (query_char, query_id, match_type, match_dist) {
       //adding the next character from a new query word;
       
            if (query_char == \0) {  //we've reached the end for this query word
                //reached word termination
                switch (match_type){
                   case exact_match:
                    push_back(q_exact_match, query_id);
                   break;
                   case hamming_match:
                     if (match_dist == 1) push_back (q_hamming_1, query_id)
                     else push_back (q_hammning_2, query_id)
                   break;
                   case edit_distance:
                     if (match_dist == 1) push_back (q_edit_1, query_id)
                     else push_back (q_edit_2, query_id)    
                   break;
                }
            
            } else {  // not yet terminating

                if (child_alphabet[char]==oo) {
                    new Trie Node [query_char]
                }
                TrieNode* node = child_alphabet[char];

                node::add_char (query_char+1, query_id, match_type, match_dist)
            }
       }
       


       - void log_node_depths {
       //when we have reached the final insert of a new query_word, only then do we know our word length.
       //Npw climb back up the tree to register word_length with every parent node.
       //we use this info at every node to work out whether to explore the tree further
       //(eg if doc-word length is 8 but following this node only yields query words max 6 length, don't bother)
           parent add_word_length[self.depth]

       }

       - int  depth {
       if (!_depth) {
          _depth = parent.depth+1;
        }
        return _depth;
       }
}



ErrorCode StartQuery(QueryID        query_id,   
                     const char*    query_str,
                     MatchType      match_type,
                     unsigned int   match_dist) {

            TrieTree::insert_word (query_str, query_id, match_type, match_dist);

                     
]


matching

class word {
    public:word
    
    functions:
    match_word (=matchQuery)
    
    
    
    
    
}
    




