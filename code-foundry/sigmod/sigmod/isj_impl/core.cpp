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

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <limits.h>

#define LOG 1

#include <iostream>

size_t uiLevenshteinDistance(const string &s1, const string &s2)
{
    const size_t m(s1.size());
    const size_t n(s2.size());

    if( m==0 ) return n;
    if( n==0 ) return m;

    size_t *costs = new size_t[n + 1];

    for( size_t k=0; k<=n; k++ ) costs[k] = k;

    size_t i = 0;
    for ( auto it1 = s1.begin(); it1 != s1.end(); ++it1, ++i )
    {
        costs[0] = i+1;
        size_t corner = i;

        size_t j = 0;
        for ( auto it2 = s2.begin(); it2 != s2.end(); ++it2, ++j )
        {
            size_t upper = costs[j+1];
            if( *it1 == *it2 )
            {
                costs[j+1] = corner;
            }
            else
            {
                size_t t(upper<corner?upper:corner);
                costs[j+1] = (costs[j]<t?costs[j]:t)+1;
            }
            
            corner = upper;
        }
    }
    
    size_t result = costs[n];
    delete [] costs;
    
    return result;
}


bool exact_match(const char* query_word,int query_word_length,const char* doc_word,int doc_word_length) {
    if (query_word_length != doc_word_length) return false;
   // if (query_word != doc_word) return false;
    if ( strcmp(query_word, doc_word) !=0 ) return false;
    return true;
}


/*
 as gnu-strcmp with 2 modifications
 (1) return value is a bool - true == NO_MATCH false == MATCH
 (2) filters to reject strings of unequal length
 */
bool jm_strcmp (const char* query_word,int query_word_length,const char* doc_word,int doc_word_length)
{
    //this is gnu code for strcmp, adapted
    if (query_word_length != doc_word_length) return true;


    register const unsigned char *s1 = (const unsigned char *) query_word;
    register const unsigned char *s2 = (const unsigned char *) doc_word;
    unsigned char c1, c2;

    do
    {
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;
        if (c1 == '\0')
            return c1 - c2;
    }
    while (c1 == c2);
    return c1 - c2;
}

int bsd_strcmp(const char * s1, const char * s2) {
    //freeBSD strcmp
       //  register const char *s1, *s2;
 {
             while (*s1 == *s2++)
                         if (*s1++ == 0)
                                     return (0);
             return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));

 }
}


//GNU-C strcmp
/* Compare S1 and S2, returning less than, equal to or
 greater than zero if S1 is lexicographically less than,
 equal to or greater than S2.  */
int gnu_strcmp (const char *p1, const char *p2)
{
    register const unsigned char *s1 = (const unsigned char *) p1;
    register const unsigned char *s2 = (const unsigned char *) p2;
    //unsigned reg_char c1, c2;
    unsigned char c1, c2;
    do
    {
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;
        if (c1 == '\0')
            return c1 - c2;
    }
    while (c1 == c2);
    return c1 - c2;
}


const char *bitap_bitwise_search2(char* string_a, int length_a, char* string_b, int length_b)
{
   // size_t length_a = strlen(string_a);
   // size_t length_b = strlen(string_b);

    /*
     this version recurses long strings down to machine-word-length strings
     (should be faster for bit operations)
     didn't see much difference in practice
     
     */

    unsigned long R;
    unsigned long pattern_mask[CHAR_MAX+1];
    int i;

    if (string_b[0] == '\0') return string_a;
    if (length_b > 31) return "The pattern is too long!";
    if (length_b != length_a) return NULL;
    int bit_length = 1;
    if (length_a > bit_length) {
        for (int idx = 0; idx<=(length_a/bit_length); idx++ ) {
       //     int check = length_a/bit_length;

            char* substring_a= &string_a[idx*bit_length];
            char* substring_b= &string_b[idx*bit_length];
            int length = (idx <length_a/bit_length)?bit_length:length_a % bit_length;
            if (idx  == length_a/bit_length)
            if (bitap_bitwise_search2(substring_a,length, substring_b, length) == 0) {
              //  printf(" no match ");
                return NULL;
            }

        }
    }


    /* Initialize the bit array R */
    R = ~1;

    /* Initialize the pattern bitmasks */
    for (i=0; i <= CHAR_MAX; ++i)
        pattern_mask[i] = ~0;
    for (i=0; i < length_b; ++i)
        pattern_mask[string_b[i]] &= ~(1UL << i);

    for (i=0; i<length_a; ++i) {
        /* Update the bit array */
        R |= pattern_mask[string_a[i]];
        R <<= 1;

        if (0 == (R & (1UL << length_b)))
            return (string_a + i - length_b) + 1;
    }

    return NULL;
}
/*

void WM(char *string_a, char *string_b, int length_a, int length_b, int k) {
    //http://www.cs.ucf.edu/courses/cda6211/patmatch.pdf
    unsigned int j, last1, last2, lim, mask, S[ASIZE], R[KSIZE];
    int i;
    // Preprocessing
    for (i=0; i < ASIZE; i++)
        S[i]= ̃0;
    lim=0;
    for (i=0, j=1; i < m; i++, j<<=1) {
        S[x[i]]&= ̃j;
        lim|=j; }
    lim= ̃(lim>>1);
    R[0]= ̃0;
    for (j=1; j <= k; j++) R[j]=R[j-1]>>1;
    // Search
    for (i=0; i < n; i++) {
        last1=R[0];
        mask=S[y[i]];
        R[0]=(R[0]<<1)|mask;
        for (j=1; j <= k; j++) {
            last2=R[j];
            R[j]=((R[j]<<1)|mask)&((last1&R[j-1])<<1)&last1;
            last1=last2;
        }
        if (R[k] < lim) OUTPUT(i-m+1);
    }
}
*/

const char *bitap_bitwise_search(char* string_a, int length_a, char* string_b, int length_b)
{
    unsigned long R;
    unsigned long pattern_mask[CHAR_MAX+1];
    int i;

    if (string_b[0] == '\0') return string_a;
    if (length_b > 31) return "The pattern is too long!";

    /* Initialize the bit array R */
    R = ~1;

    /* Initialize the pattern bitmasks */
    for (i=0; i <= CHAR_MAX; ++i)
        pattern_mask[i] = ~0;
    for (i=0; i < length_b; ++i)
        pattern_mask[string_b[i]] &= ~(1UL << i);

    for (i=0; string_a[i] != '\0'; ++i) {
        /* Update the bit array */
        R |= pattern_mask[string_a[i]];
        R <<= 1;

        if (0 == (R & (1UL << length_b)))
            return (string_a + i - length_b) + 1;
    }
    
    return NULL;
}

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
   if (LOG) printf( " StartQuery: id %d ", query_id );
	Query query;
	query.query_id=query_id;
	strcpy(query.str, query_str);
	query.match_type=match_type;
	query.match_dist=match_dist;
	// Add this query to the active query set
	queries.push_back(query);
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode EndQuery(QueryID query_id)
{
    if (LOG)printf( " EndQuery: %d ", query_id );

	// Remove this query from the active query set
	unsigned int i, n=queries.size();
	for(i=0;i<n;i++)
	{
		if(queries[i].query_id==query_id)
		{
			queries.erase(queries.begin()+i);
			break;
		}
	}
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode MatchDocument(DocID doc_id, const char* doc_str)
{
    for (int mi=0; mi<1; mi++) {

   if (LOG) printf( " MatchDocument: %d ", doc_id );

	char current_doc_string[MAX_DOC_LENGTH];
	strcpy(current_doc_string, doc_str);

	unsigned int i, n=queries.size();
	vector<unsigned int> query_ids;

	// Iterate on all active queries to compare them with this new document
	for(i=0;i<n;i++)
	{
		bool matching_query=true;
		Query* query =&queries[i];

		int iq=0;
		while(query->str[iq] && matching_query)
		{
			while(query->str[iq]==' ') iq++;
			if(!query->str[iq]) break;
			char* query_word=&query->str[iq];

			int query_word_length =iq;
			while(query->str[iq] && query->str[iq]!=' ') {
             // if (query->str[iq]>120)  printf("%d ",query->str[iq]);
                iq++;
            }
			char qt=query->str[iq];
			query->str[iq]=0;
			query_word_length=iq-query_word_length;

			bool matching_word=false;

			int doc_idx=0;
			while(current_doc_string[doc_idx] && !matching_word)
			{
				while(current_doc_string[doc_idx]==' ') doc_idx++;
				if(!current_doc_string[doc_idx]) break;
				char* doc_word=&current_doc_string[doc_idx];

				int doc_word_start=doc_idx;
				while(current_doc_string[doc_idx] && current_doc_string[doc_idx]!=' ') doc_idx++;
				char dt=current_doc_string[doc_idx];
				current_doc_string[doc_idx]=0;

				int doc_word_length=doc_idx-doc_word_start;
               // for (int i=0; i<1000; i++) {
                 //   printf("%d ",i);
                    if(query->match_type==MT_EXACT_MATCH)
                    {
                     if (exact_match(query_word,query_word_length,doc_word,doc_word_length)) matching_word=true;
               //  if(jm_strcmp(query_word,query_word_length,doc_word,doc_word_length)==false)  matching_word=true;
                     //   if (doc_word_length != query_word_length)
                     //       matching_word=false;
                     //   else
                        //   if(strcmp(query_word, doc_word)==0)  matching_word=true;

               //       if (bitap_bitwise_search(query_word,query_word_length, doc_word,doc_word_length) !=0) matching_word=true;

                     // if (bitap_bitwise_search2(query_word,query_word_length, doc_word,doc_word_length) !=0) matching_word=true;


                    }
                    else if(query->match_type==MT_HAMMING_DIST)
                    {
                        
                        unsigned int num_mismatches=HammingDistance(query_word, query_word_length, doc_word, doc_word_length);
                        if(num_mismatches<=query->match_dist) matching_word=true;
                    }
                    else if(query->match_type==MT_EDIT_DIST)
                    {
                        //size_t edit_dist = uiLevenshteinDistance(query_word,doc_word);
                        unsigned int edit_dist=EditDistance(query_word, query_word_length, doc_word, doc_word_length);
                        if(edit_dist<=query->match_dist) matching_word=true;
                    }
              //  }

				current_doc_string[doc_idx]=dt;
			}

			query->str[iq]=qt;

			if(!matching_word)
			{
				// This query has a word that does not match any word in the document
				matching_query=false;
			}
		}

		if(matching_query)
		{
			// This query matches the document
			query_ids.push_back(query->query_id);
		}
	}

	Document doc;
	doc.doc_id=doc_id;
	doc.num_res=query_ids.size();
	doc.query_ids=0;
	if(doc.num_res) doc.query_ids=(unsigned int*)malloc(doc.num_res*sizeof(unsigned int));
	for(i=0;i<doc.num_res;i++) doc.query_ids[i]=query_ids[i];
	// Add this result to the set of undelivered results
	docs.push_back(doc);
    }
    //printf("success");
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids)
{
	// Get the first undeliverd resuilt from "docs" and return it
	*p_doc_id=0; *p_num_res=0; *p_query_ids=0;
	if(docs.size()==0) {
      if (LOG)  printf( " GetNextAvailRes: EC_NO_AVAIL_RES ", *p_doc_id, *p_num_res );
        return EC_NO_AVAIL_RES;
    }
	*p_doc_id=docs[0].doc_id; *p_num_res=docs[0].num_res; *p_query_ids=docs[0].query_ids;
	docs.erase(docs.begin());
   if (LOG)  printf( " GetNextAvailRes: %d, %d ", *p_doc_id, *p_num_res );
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
