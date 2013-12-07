/*
 * core.cpp version 1.0
 * Copyright (c) 2013 KAUST - InfoCloud Group (All Rights Reserved)
 * Author: Amin Allam
 *
 * HACKED VERSION BY isj
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "../include/core.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>

#include "SearchTree.h"
#include "WordTumbler.h"
#include "sigmod_types.h"
#include "DocResults.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <limits.h>


#include <iostream>

// Computes edit distance between a null-terminated string "a" with length "na"
//  and a null-terminated string "b" with length "nb" 
int EditDistance(char* string_a, int length_a, char* string_b, int length_b)
{
    //string_a = "ANIMAL";
    //string_b = "BDIMAL";
    //length_a = 6;
    //length_b = 6;

	int oo=0x7FFFFFFF;

	static int Table[2][MAX_WORD_LENGTH+1];  // 2-row table to hold the two matchwords

	int ia, ib;

	int current_row=0;
	ia=0;

	for(ib=0;ib<=length_b;ib++)
        //add word b to the table (1st row)
		Table[current_row][ib]=ib;                                          // Ob

	current_row=1-current_row;

	for(ia=1;ia<=length_a;ia++)
        //iterate through word a letters
	{
		for(ib=0;ib<=length_b;ib++)
            //iterate through word b letters
			Table[current_row][ib]=oo;                                      //0a x 0b
        //initialise word b positions in row a to oo


		int ib_start=0;                                            //01
		int ib_end=length_b;                                       //01

		if(ib_start==0)
		{
			ib=0;
			Table[current_row][ib]=ia;
			ib_start++;
		}

		for(ib=ib_start;ib<=ib_end;ib++)                //0(b)
		{
			int ret=oo;

			int d1=Table[1-current_row][ib]+1;          //0(1)
			int d2=Table[current_row][ib-1]+1;          //0(1)
			int d3=Table[1-current_row][ib-1];          //0(1)
            if(string_a[ia-1]!=string_b[ib-1])
                d3++;                                    //0(1)

			if(d1<ret) ret=d1;                           //0(1)
			if(d2<ret) ret=d2;                           //0(1)
			if(d3<ret) ret=d3;                           //0(1)

			Table[current_row][ib]=ret;                 //0(1)
		}

		current_row=1-current_row;                 //0(1)
	}

	int ret=Table[1-current_row][length_b];

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Computes Hamming distance between a null-terminated string "a" with length "na"
//  and a null-terminated string "b" with length "nb" 
unsigned int HammingDistance(char* a, int na, char* b, int nb)
{
	int j, oo=0x7FFFFFFF;
	if(na!=nb) return oo;
	
	unsigned int num_mismatches=0;
	for(j=0;j<na;j++) if(a[j]!=b[j]) num_mismatches++;
	
	return num_mismatches;
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Keeps all information related to an active query
struct Query
{
	QueryID query_id;
	char str[MAX_QUERY_LENGTH];
	MatchType match_type;
	unsigned int match_dist;
};

///////////////////////////////////////////////////////////////////////////////////////////////

// Keeps all query ID results associated with a dcoument
struct Document
{
	DocID doc_id;
	unsigned int num_res;
	QueryID* query_ids;
};

///////////////////////////////////////////////////////////////////////////////////////////////

// Keeps all currently active queries
vector<Query> queries;


// Keeps all currently available results that has not been returned yet
vector<Document> docs;

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode InitializeIndex(){return EC_SUCCESS;}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode DestroyIndex(){return EC_SUCCESS;}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist)
{
    int query_str_idx = 0;
    //we use query_str_idx to track word breaks as we consume the query_str
    SearchTree::Instance()->addQuery(query_id, query_str, match_type, match_dist,query_str_idx);
    if (LOG) printf( " StartQuery: id %d \n", query_id );

	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode EndQuery(QueryID query_id)
{


    printf("\nending query....\n");
    SearchTree::Instance()->print();
    printf("removeQuery ....%d\n",query_id);

    SearchTree::Instance()->removeQuery(query_id);
    printf("after removing query....\n");
    SearchTree::Instance()->print();




	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode MatchDocument(DocID doc_id, const char* doc_str)
{

    unsigned int word_start_idx = 0;
    unsigned int word_length = 0;


    while (doc_str[word_start_idx+word_length] != '\0') {
        if (doc_str[word_start_idx+word_length]  == ' ') {
            //SearchTree::Instance()->matchWord(doc_id, doc_str, word_start_idx, word_length);
            word_start_idx+=word_length+1;
            word_length = 0;
        } else {
            word_length++;

        }
    }
	return EC_SUCCESS;
}
/*
 
 
 
 
 
 */

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids)
{

    ErrorCode result = DocResults::Instance()->GetNextAvailRes(p_doc_id, p_num_res, p_query_ids);

	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////
