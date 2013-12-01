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



if(strcmp(query_word, doc_word)==0)  matching_word=true;

Your program has successfully passed all tests.
docs: 960 queries: 993 end_queries: 1013
exact_match: 993 hamming_distance: 0 edit_distance: 0
Time=20787[20s:787ms]
Program ended with exit code: 0

build a trie tree

Class Trie Tree {
     
     Trie_Node root;
     
     function insert_word (query_str, query_id, match_type, match_dist) {
     
     
     
     
     }


     


}

CLASS Trie Node
    char letter;
    Tie Node parent;
    alphabet[26] children;   //initialise with 26 elements val oo
      bool terminator
        vector <int> q_exact_match   //query_ids of exact_match queries containting this word
        vector <int> q_hamming_1    //query_ids of hamming_match_1 queries containing this word
        vector <int> q_hamming_2   //query_ids of hamming_match_2 queries containing this word
        vector <int> q_edit_1   //query_ids of edit_distance_1 queries containing this word
        vector <int> q_edit_2   //query_ids of edit_distance_2 queries containing this word
    }
       vector <int> matching_docs  //do we need this?
       
       function check_match_valid (query_id, match_type) {
          look up query_Id in list of eliminated queries
          if it is there, delete it from match_type

       }

       function match_found (doc_id, query_id) {
            decrement the doc_id query_index
       }
       
       function add_char (query_char, query_id, match_type, match_dist) {
       
            if (query_char == \0) {
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
            
            } else {

                if !alphabet[char] new Trie Node
                    [query_char]
                add_char (query_char+1, query_id, match_type, match_dist) 
            }
       }
       function log_node_depths {

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
    




