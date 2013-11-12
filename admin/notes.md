[notes 12/11/13]

The text stream that we are reading consists of four types of data:

s - search query.   
Consists of 1 or more words, a search type, search parameters  
m - document.   
Conssists of 1 or more words for the search queries to operate on  
e - end search.   
Indicates removing a search query from our task list  
r - resultsRequest.   
When in receipt of a results request, we need to report our results. This takes the form:
documentID, searchQueryID... [list of searchQueries satisfied by document)


__Outline of a design__  
The key intuition is that we are reading from a text stream, so we have to arrange our data and queries to operate as soon as we are able to understand the nature of the request/data in the stream. In particular, it would be helpful to be able to search a document before it is fully received.

A second intuition is that we only need to report our results in terms of IDs. IN other words, we do NOT need to store our results in terms of word lists. So we can dispose of unnecessary data as soon as we are done with the search/document.

Conventional search would take a document and place it into a temporary or persistent store. We would then operate on that document with the palette of our search queries, one by one or in parallel, accumlulating a list of satisfied search queries. When all search queries have been run against a document, we can report our result.

For example, a typical regular expression search on a document might look like this:

	`$_ = $document_to_search;  
	@result =~ m/search|terms|to|find/;
where `@result` will contain indexes of successful hits.

Our proposal is to invert the process. Rather than search across the document with our seach queries, we propose to search the queries with our document. This has the major benefit that we can initiate the process as soon as we _start_ receiving our document. As we feed our search queries, word by word, with the contents of the document, we might hope that our seach query task on that docuemt is completed when our initial document read is complete. We might complete our docu search in O(n) which - given that our docu consumption is of necessity O(n) - would mean that our search comes for free.

The style of this process might be characterized as _Declarative_ as against _Imperitive_. To a great extent, we will be building data structures (the search query trees), to which we subject the individual words of our documents. At certain prescribed moments we will harvest results from the trees.

A realworld analogy of our proposal might be a game of Pachinko, in which the input balls represents individual words in the document currently being searched, and the arrangement of pins to collect these balls are the search queries. As we drop each ball into the game, it tumbles through the tree, either being trapped as it satisfies by one of the search terms, or otherwise  falling through to the floor, when we can lose it from memory (we don't care for the precise list of successes and failures, just that there _were_ successes and failures). Another example could be a coin-op vending machine which takes any coin denomination from a legal set, and ejects the rest. We just throw in our coins - in any order as they become available - and the sorter either registers them as received of a certain value, or rejects them to the out tray, accumulating a hit result as we continue to feed it with coins.

That is probably a little optimistic. But if we can construct binary search tree (or some other kind of tree) our of our search terms, then throwing our docu words in their direction we might hope to achieve an O( n log n) result (n for the document consumption, log n for the search per word). 

Other optimisations might bring us good improvements. For example, there are many duplicate words in search terms, we can save energy by not searching the same term twice. On a hardware level, we have 12 cores available to us, so certainly any method that can enable us to split out the task across cores will be hugely beneficial. 

Within this sketch there are many details to be fleshed out.

__Search query tree(s)__  
We will want to construct searchable trees from the search queries. What kind of tree?  
storing text content in nodes (alphabetical search)  
: radix tree  
: trie tree  
: other?  
- storing numerical content in nodes (numeric search)  

binary search tree  
: AVL tree  
: Red/black tree  
: other?  

Some of these tree structures are more efficient for inserts and deletes, others for searching. Perhaps we can construct two or more trees and compare their efficeincies against real input. Perhaps there is another way entirely. Inserts and deletes sound expensive - for the deletes, it might be sufficient to mark entries as active or not. This would be particularly beneficial if there are many repeat search terms in different search queries. But if we end up with a tree with many inactive nodes, our tree will lose optimal efficiency (it will have greater depth than necessary).


__Search type__   
We have three search types to consider - exact match, hamming distance and deletion...   
: can we use the same overall structure for all three?   
: What _are_ good algorithms for the non-exact-match type of queries?  


__Data type__  
The search queries and document strings are _words_ with _word boundaries_ (spaces). We can elect to search on this basis, or we can transform our data into _numbers_, perhaps _unique hashes_, which will transform our problem from text search/sorting to numerical search/sorting. Which is better?

__Data storage__
With 26Gb of input data, we will have to consider the most efficient forms of data storage, as we will be using all of the resources avaiable to the machine. The sooner we can disposse of data, the better.  

__Multithreading/multiprocessing.__   
We want to take advantage of our 12 cores. How to do this? How to avoid wait states (no activity on a core as it is waiting for a docu read for example). How to split tasks across a single document? Can we use multiple copies of the same tree (one per core?)

__Reading results.__   
The efficient steps seem to be the passing of the docu words into the search trees, and the search itself. But what of results reporting? We will need an efficient way to harverst results from our tree nodes.

__Other useful algorithms__  
What other algorithms and data tools should we be considering?   
: Skip lists?  
: Hashes? Should we be planning to convert each word into a unique number?  
Can we search (match) faster when our data fits within a certain maximum (computer)word length? Eg if we break our data into max 32-bit or 64-bit (4- or 8-letter) pieces can we use more efficient match tests?

__Match types__  
The task includes exact match, hamming match and deletion match types. Do we need to take completely different approaches for hamming and deletion matches? This would be undersirable, although we could construct 3 tree types to suit the 3 match types (i think one query only supports one match type)


----

[email 5/11/13] 


Begin forwarded message:

From: "jonathan" 
Subject: small idea
Date: 5 November 2013 13:50:23 GMT
To: Selvan , Ioannis

Here is the kernel of an algorithm idea

instead of searching the documents with the search terms, let's search the search terms with the documents


we get a bunch of queries first

we have to read them in anyway, so at that time we can sort them using some kind of tree, indexing by query number

eg s3 = {tree try trust}
s4 = {big try small}

index as big (s3, s4) try (s3) trust (s3) small (s4)
(in a tree for logN search)

then when we are given a document, we have to consume it anyway, so we search the query tree with each word as we consume it. Then our search is complete as soon as we have consumed the whole document. We should try to avoid searching for duplicate term in the doc as we consume them.

once the document is consumed, we will have to check the state of the tree to work out which queries are completely satisfied. Also when we get new search queries we will have to rearrange our tree.





[notes 3/11/13]

__string search__  
[wikipedia String_search](https://en.wikipedia.org/wiki/String_search)


__naive methods__

for each document  
make a (indexed?) sorted word set (removing duplicates)

	for each document d
		for each searchword s
			for each letter l
			find the index of l in d
			if (doesnt exist) find the index of (next lower letter)...
			  insert word at index+1
			  update index
			if (exists) 
			  find index of it's second letter
			  if (doesnt exist) find the index of (next lower letter)...
			    insert word at index+1
			    update index
			    if (exitsts)
			    
	


for each search term

	
	
	[doc {a=1 b=3 d=7 g=8}]
	
	[a {l=1 t=2 s=3}]
	aleph
	atemp
	asterisk
	
	[b {e=1}]
	[be {t=1 s=3 z=4}]
	[bet {a=1 s=2}]
	beta
	bets
	[bes {a=1}]
	besa
	[bez {a=1}]
	beza
	delta
	gamma
	


to search for besas

b=3; e=1; s=3; a=