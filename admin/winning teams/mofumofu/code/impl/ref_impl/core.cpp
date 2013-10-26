/*
* core.cpp version 1.0
* Copyright (c) 2013 KAUST - InfoCloud Group (All Rights Reserved)
* Author: Amin Allam
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
#include <cassert>

#include <nmmintrin.h>
#include <pthread.h>

#include <vector>
#include <algorithm>



#include "util.h"
#include "AlignmentAllocator.h"
#include "threadsafe_queue.h"
#include "popcount.h"
#include "word.h"
#include "timsort.hpp"
#include "doc_words_vec.h"



#if 0
#include <map>
#define my_map map
#else
#include "../cpp-btree-1.0.1/btree_map.h"  //from http://code.google.com/p/cpp-btree/
#define my_map btree_map
using namespace btree;
#endif

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////


//queryの文字列情報だけを取り出したもの。
class QueryStrings
{
public:
	Word_32 words[MAX_QUERY_WORDS];
	unsigned int words_len;

	QueryStrings(){}

	QueryStrings(const char* query_str)
	{
		words_len =0;
		const char* __restrict cp = query_str;
		const __m128i ws = _mm_set1_epi8('a');
		const __m128i zero = _mm_setzero_si128();
		//const __m128i one = _mm_set1_epi8(255);
		const __m128i num = _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);

		while (true)
		{
			__m128i c = _mm_loadu_si128((const __m128i* __restrict)cp);
			int mask = _mm_movemask_epi8(_mm_cmpgt_epi8(ws,c));
			int len;
			__m128i co2;
			if(mask){//15文字以下の場合
				len = bsf(mask);
				__m128i sc = _mm_cmpgt_epi8(_mm_shuffle_epi8(_mm_cvtsi32_si128(len),zero),num);
				c= _mm_and_si128(sc,c);
				co2 = zero;
			}else{//16文字以上の場合
				__m128i c2 = _mm_loadu_si128((const __m128i* __restrict)(cp+16));
				int mask2 = _mm_movemask_epi8(_mm_cmpgt_epi8(ws,c2));
				int b = bsf(mask2);
				co2 = _mm_and_si128(_mm_cmpgt_epi8(_mm_shuffle_epi8(_mm_cvtsi32_si128(b),zero),num) ,c2);
				len = 16+ b;
			}
			_mm_store_si128(words[words_len].pack_sse + 0 , c);
			_mm_store_si128(words[words_len].pack_sse + 1 , _mm_insert_epi8(co2,len,15));

			words_len++;
			if(!cp[len])break;
			cp += len+1;
		}
		switch (words_len)
		{
		case 1:
			break;
		case 2:
			if(Word_32_length_comp_R()(words[0],words[1])){
			}else{
				swap(words[0],words[1]);
			}
			break;
		case 3:
			if(Word_32_length_comp_R()(words[0],words[1])){
				if(Word_32_length_comp_R()(words[1],words[2])){
				}else if(Word_32_length_comp_R()(words[0] , words[2])){
					Word_32 t=words[0];
					words[0] = words[2];
					words[2] = words[1];
					words[1] = t;
				}else{
					//					words[0] = ww[0];
					swap(words[1],words[2]);
				}
			}else{
				Word_32 t =words[0];
				if(Word_32_length_comp_R()(words[2] , words[1])){
					words[0] = words[2];
					words[2] =t;
					//words[1] = ww[1];
				}else if(Word_32_length_comp_R()(words[2] , words[0])){
					words[0] = words[1];
					words[1] = words[2];
					words[2] = t;
				}else{
					words[0] = words[1];
					words[1] = t;
					//				words[2] = ww[2];
				}
			}
		default:
			sort(words,words+words_len,Word_32_length_comp_R());
			break;
		}
	}

	//謎ソート(逆順にする)
	MY_INLINE bool operator<(const QueryStrings& right)  const 
	{
		if(words_len< right.words_len)return true;
		else if(words_len > right.words_len)return false;
		for(unsigned int i=0;i<words_len;i++){
			if(words[i] == right.words[i])continue;
			return words[i] < right.words[i];
		}
		return false;
	}

	MY_INLINE bool operator==(const QueryStrings& right) const 
	{
		if(words_len != right.words_len)return false;
		for(unsigned int i=0;i<words_len;i++){
			if(words[i] == right.words[i])continue;
			return false;
		}
		return true;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////
struct CountAlphabetPair{
	unsigned int Count;
	//unsigned int Alphabet;
	CountAlphabetPair(){}
	CountAlphabetPair(unsigned int count):Count(count){}
	//CountAlphabetPair(unsigned int count,unsigned int alphabet):Count(count),Alphabet(alphabet){}
};
//typedef unsigned int CountAlphabetPair;


class WordSet{
public:
	//my_map<Word_32,CountAlphabetPair,Word_32_length_comp,AlignmentAllocator<pair<const Word_32,CountAlphabetPair> > > words;
	my_map<Word_8,CountAlphabetPair,less<Word_8>,AlignmentAllocator<pair<const Word_8,CountAlphabetPair> > > words8;
	my_map<Word_16,CountAlphabetPair,less<Word_16>,AlignmentAllocator<pair<const Word_16,CountAlphabetPair> > > words16;
	my_map<Word_32,CountAlphabetPair,less<Word_32>,AlignmentAllocator<pair<const Word_32,CountAlphabetPair> > > words32;
	vector<Word_8> vec_words8;
	vector<Word_16> vec_words16;
	vector<Word_32> vec_words32;
	bool needUpdate;
	size_t vec_words_offset[3];

	WordSet():words8(),words16(),words32()
		,vec_words8(),vec_words16(),vec_words32(),needUpdate(true)
	{
		vec_words_offset[0] =vec_words_offset[1]=vec_words_offset[2]=0;
	}

	void add(const Word_32 &w){
		if(w.word.len<=8){
			auto ww = Word_8(w);
			auto p = words8.find(ww);
			if(p==words8.end()){
				needUpdate=true;
				words8.insert(make_pair(ww,CountAlphabetPair(1)));
			}
			else{p->second.Count++;}
		}else if(w.word.len<=16){
			auto ww = Word_16(w);
			auto p = words16.find(ww);
			if(p==words16.end()){
				needUpdate=true;
				words16.insert(make_pair(ww,CountAlphabetPair(1)));
			}
			else{p->second.Count++;}
		}else{
			auto p = words32.find(w);
			if(p==words32.end()){
				needUpdate=true;
				words32.insert(make_pair(w,CountAlphabetPair(1)));
			}
			else{p->second.Count++;}
		}
	}

	void remove(const Word_32 &w){
		if(w.word.len<=8){
			auto p =words8.find(Word_8(w));
			if(p->second.Count ==1){
				needUpdate=true;
				words8.erase(p);
			}
			else{p->second.Count--;}
		}else if(w.word.len<=16){
			auto p =words16.find(Word_16(w));
			if(p->second.Count ==1){
				needUpdate=true;
				words16.erase(p);
			}
			else{p->second.Count--;}
		}else{
			auto p =words32.find(w);
			if(p->second.Count ==1){
				needUpdate=true;
				words32.erase(p);
			}
			else{p->second.Count--;}
		}
	}
	void update(){
		if(!needUpdate)return;
		needUpdate=false;
		vec_words8.clear();
		vec_words16.clear();
		vec_words32.clear();

		for(auto itr = words8.begin();itr!= words8.end();itr++){
			const auto &w = itr->first;
			vec_words8.push_back(w);
		}

		for(auto itr = words16.begin();itr!= words16.end();itr++){
			const auto &w = itr->first;
			vec_words16.push_back(w);
		}

		for(auto itr = words32.begin();itr!= words32.end();itr++){
			const auto &w = itr->first;
			vec_words32.push_back(w);
		}
		vec_words_offset[0] = vec_words8.size();
		vec_words_offset[1] = vec_words8.size() + vec_words16.size();
		vec_words_offset[2] = vec_words8.size() + vec_words16.size()+ vec_words32.size();
	}
};
struct QueryIDSet{
	vector<QueryID> QueryIDs;
	unsigned int alphabet[MAX_QUERY_WORDS];
};

struct QueryStringsAndInfo{
	QueryStrings strs;
	vector<QueryID> *QueryIDs;
	//vector<QueryID> QueryIDs;
	unsigned int alphabet[MAX_QUERY_WORDS];
	unsigned int wordIndex[MAX_QUERY_WORDS];
};


class QuerySet{
	my_map<QueryID,QueryStrings,less<QueryID>,AlignmentAllocator<pair<const QueryID,QueryStrings> > > raw_queries;
	my_map<QueryStrings,QueryIDSet,less<QueryStrings>,AlignmentAllocator<pair<const QueryStrings,QueryIDSet> > > Compressed_Querys; //圧縮状態のQuery
public:
	bool isUseWordSet,needUpdate;
	vector<QueryStringsAndInfo,AlignmentAllocator<QueryStringsAndInfo> > QueryVector;


	//QuerySet():isDeleted(false), raw_queries(),addedQueries(),Compressed_Querys(){ }
	QuerySet():raw_queries(),Compressed_Querys() ,isUseWordSet(false),needUpdate(true),QueryVector() { }
private:
	MY_INLINE void innerAdd(const QueryStrings &str,QueryID id,WordSet *wordSet){
		const auto itr = Compressed_Querys.find(str);
		if(itr != Compressed_Querys.end()){
			itr->second.QueryIDs.push_back(id);
		}else{
			QueryIDSet qas;
			for(unsigned int i=0U;i<str.words_len;i++){
				qas.alphabet[i] = GetAlphabetSet(str.words[i]);
			}
			qas.QueryIDs.push_back(id);
			Compressed_Querys.insert(make_pair(str, qas));

			needUpdate=true;
			if(wordSet!=NULL){ //新しいときだけカウント
				for(auto i=0U;i<str.words_len;i++){
					wordSet->add(str.words[i]);
				}
			}
		}
	}

public:

	void addQuery(QueryID query_id, const char* query_str,WordSet *wordSet ){
		auto q =QueryStrings(query_str);
		raw_queries.insert(make_pair(query_id,q ));
		innerAdd(q,query_id,wordSet);
	}

	//処理されたかをチェック
	bool removeQuery(QueryID query_id,WordSet *wordSet){
		if(raw_queries.empty())return false;
		const auto itr =raw_queries.find(query_id);
		if(itr == raw_queries.end())return false;
		const auto d = Compressed_Querys.find(itr->second);
		if(d->second.QueryIDs.size()==1){
			Compressed_Querys.erase(d);
			needUpdate=true;
			if(wordSet!=NULL){ //新しいときだけカウント
				for(auto i=0U;i<itr->second.words_len;i++){
					wordSet->remove(itr->second.words[i]);
				}
			}
		}else{
			auto &arr = d->second.QueryIDs;
			if(arr.size()<=32){
				unsigned int i=0;
				for(;arr[i] != query_id;i++){assert(i<arr.size());}
				arr.erase(arr.begin()+i);
				//arr[i] = arr.back();
				//arr.pop_back();
			}else{
				arr.erase(lower_bound(arr.begin(),arr.end(),query_id));
			}
		}
		//isDeleted=true;

		raw_queries.erase(itr);
		return true;
	}

	MY_INLINE void updateForce(const WordSet &wordSet);

	MY_INLINE void update(const WordSet &wordSet)
	{
		if(!needUpdate)return;
		updateForce(wordSet);
	}

	MY_INLINE void updateForce();

	MY_INLINE void update()
	{
		if(!needUpdate){return;}
		updateForce();
	}

};


MY_INLINE void QuerySet::updateForce()
{
	needUpdate=false;
	QueryVector.clear();
	//QueryVector.reserve(Compressed_Querys.size());
	QueryVector.resize(Compressed_Querys.size());
	int i=0;
	for(auto itr = Compressed_Querys.begin();itr != Compressed_Querys.end();itr++,i++){
		const auto &quer = itr->first;
		QueryStringsAndInfo &info = QueryVector[i];

		info.QueryIDs = &( itr->second.QueryIDs);
		//info.QueryIDs.assign(itr->second.QueryIDs.begin(),itr->second.QueryIDs.end());

		info.strs = quer;
		assert(quer.words_len>0);
		for(unsigned int j=0;j<quer.words_len;j++){
			info.alphabet[j] = itr->second.alphabet[j];
		}
	}
}

MY_INLINE void QuerySet::updateForce(const WordSet &wordSet)
{
	needUpdate=false;
	QueryVector.clear();
	//QueryVector.reserve(Compressed_Querys.size());
	QueryVector.resize(Compressed_Querys.size());
	int i=0;
	for(auto itr = Compressed_Querys.begin();itr != Compressed_Querys.end();itr++,i++){
		const auto &quer = itr->first;
		QueryStringsAndInfo &info = QueryVector[i];

		info.QueryIDs = &( itr->second.QueryIDs);
		//info.QueryIDs.assign(itr->second.QueryIDs.begin(),itr->second.QueryIDs.end());

		info.strs = quer;

		assert(quer.words_len>0);
		for(unsigned int j=0;j<quer.words_len;j++){
			info.alphabet[j] = itr->second.alphabet[j];
			const Word_32 &w = quer.words[j];
			const int len = w.word.len;

			long long index;
			if(len<=8){
				const auto &ww = wordSet.vec_words8;
				index = lower_bound(ww.begin(),ww.end(),Word_8(w)) -ww.begin();
			}else if(len<=16){
				const auto &ww = wordSet.vec_words16;
				index = lower_bound(ww.begin(),ww.end(),Word_16(w)) -ww.begin() 
					+ wordSet.vec_words_offset[0];
			}else{
				const auto &ww = wordSet.vec_words32; 
				index = lower_bound(ww.begin(),ww.end(),w) -ww.begin()
					+ wordSet.vec_words_offset[1];
			}
			assert(0<=index &&  index < (long long)wordSet.vec_words_offset[2]);
			info.wordIndex[j]=(unsigned int) index;
		}
	}
}




class QuerySetSet{
public:
	vector<QuerySet> queries;
	WordSet wordSet;
	bool isUseWordSet;

	QuerySetSet():queries(),wordSet(),isUseWordSet(false){}

	void addQuery(unsigned int dist,QueryID query_id, const char* query_str){
		if(dist!=0)dist--;
		queries[dist].addQuery( query_id,query_str,isUseWordSet?&wordSet:NULL );
	}

	//処理されたかをチェック
	bool removeQuery(QueryID query_id){
		for(auto i=0U;i<queries.size();i++){
			if(queries[i].removeQuery(query_id,isUseWordSet?&wordSet:NULL ))return true;
		}
		return false;
	}

	MY_INLINE void update();
};

MY_INLINE void QuerySetSet::update()
{
	if(!isUseWordSet){
		queries[0].update();
		return;
	}

	if(wordSet.needUpdate){
		wordSet.update();
		for(auto i=0U;i<queries.size();i++)queries[i].updateForce(wordSet);
	}else{
		for(auto i=0U;i<queries.size();i++)queries[i].update(wordSet);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////

// Keeps all query ID results associated with a dcoument
struct Document
{
	DocID doc_id;
	unsigned int num_res;
	QueryID* query_ids;

};


struct DocumentInput
{
	DocID doc_id;
	const char* str;

	DocumentInput(DocID _doc_id,const char * _str):doc_id(_doc_id),str(_str){;}
};

///////////////////////////////////////////////////////////////////////////////////////////////

//static unsigned int MY_INLINE getQueryTypeIndex( MatchType match_type, unsigned int match_dist){
//	return match_type*4+ match_dist;
//}
//static const int activeQueryTypeIndex[]={0,5,6,7,9,10,11};


// Keeps all currently active queries
static const unsigned int MAX_QUERY_SET = 3*4;

typedef vector<Word_32,AlignmentAllocator<Word_32> > doc_words_type;
class calc_class{
	DocumentInput *di;
	const vector<QuerySetSet> *queries;
	vector<QueryID> query_ids;
	doc_words_vec doc_words[MAX_WORD_LENGTH+1];
	vector<signed char> wf;

	static const char ED_YES_MATCH=1;
	static const char ED_NO_MATCH=2;

	MY_INLINE void getDocWordSet();
	MY_INLINE void calcExact();
	MY_INLINE void calcHamming();
	MY_INLINE void calcED();
	MY_INLINE Document returnDocument();

public:
	calc_class():di(NULL),queries(NULL),query_ids(),doc_words(),wf()
	{
		for(int i=0;i<=MAX_WORD_LENGTH;i++){doc_words[i].setLen(i);}
	}

	MY_INLINE Document calc(const vector<QuerySetSet> &queries,DocumentInput & di);
};


MY_INLINE void calc_class::getDocWordSet()
{
	const char* __restrict cp = &di->str[0];
	const __m128i ws = _mm_set1_epi8('a');
	const __m128i zero = _mm_setzero_si128();
	const __m128i num = _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);

	while (true)
	{
		__m128i c = _mm_loadu_si128((const __m128i* __restrict)cp);
		int mask = _mm_movemask_epi8(_mm_cmpgt_epi8(ws,c));
		int len;
		if(mask){//15文字以下の場合
			len = bsf(mask);
			__m128i sc = _mm_cmpgt_epi8(_mm_shuffle_epi8(_mm_cvtsi32_si128(len),zero),num);
			c= _mm_and_si128(sc,c);
			Word_16 w;
			w.pack_sse  =c;
			doc_words[len].addWordCnv8_16(w);
		}else{//16文字以上の場合
			__m128i c2 = _mm_loadu_si128((const __m128i* __restrict)(cp+16));
			int mask2 = _mm_movemask_epi8(_mm_cmpgt_epi8(ws,c2));
			int b = bsf(mask2);
			__m128i co2 = _mm_and_si128(_mm_cmpgt_epi8(_mm_shuffle_epi8(_mm_cvtsi32_si128(b),zero),num) ,c2);
			len = 16+ b;
			Word_32 w;
			w.pack_sse[0]= c;
			w.pack_sse[1]= _mm_insert_epi8(co2,len,15);
			doc_words[len].addWordCnv16_32(w);
		}

		//_mm_store_si128(w.pack_sse + 1 , co2);
		if(!cp[len])break;
		cp += len+1;
	}
	for(int i=MIN_WORD_LENGTH;i<=MAX_WORD_LENGTH;i++){doc_words[i].UniqeAndSetAlphabet();}
}
MY_INLINE void calc_class::calcExact()
{
	auto const & mq = (*queries)[MT_EXACT_MATCH].queries[0];

	for(auto vi =0U;vi<mq.QueryVector.size();vi++){
		const auto &qq =mq.QueryVector[vi];
		const auto &quer = qq.strs;

		assert(quer.words_len>0);
		for(unsigned int j=0;j<quer.words_len;j++){
			const Word_32 &w = quer.words[j];
			if(!doc_words[w.word.len].ExactMatch(w))goto no_match_exact;
		}
		{
			const auto &ids = *qq.QueryIDs;
			query_ids.insert(query_ids.end(),ids.begin(),ids.end());
			//for(unsigned int i =0;i<ids.size();i++){ query_ids.push_back(ids[i]); }
		}
no_match_exact:;
	}
}

MY_INLINE void calc_class::calcHamming()
{
	auto const &qs = (*queries)[MT_HAMMING_DIST]; 

	wf.assign(qs.wordSet.vec_words_offset[2],0);
	for(int dist=3;dist>=1;dist--){
		auto const & mq = qs.queries[dist-1];
		if(mq.QueryVector.empty())continue;
		for(auto vi =0U;vi<mq.QueryVector.size();vi++){
			const auto &qq =mq.QueryVector[vi];
			const auto &quer = qq.strs;
			char state[5]={};

			assert(quer.words_len>0);
			for(unsigned int j=0;j<quer.words_len;j++){
				auto index = qq.wordIndex[j];
				assert(0<=index && index < wf.size());

				auto s = wf[index];
				if(s<0)goto no_match_hamming;
				state[j]=s;
			}
			assert(quer.words_len>0);
			for(unsigned int j=0;j<quer.words_len;j++){
				assert(state[j] >=0);

				if(state[j] == dist)continue;
				const Word_32 &w = quer.words[j];
				const int len = w.word.len;
				const auto & dw = doc_words[len];

				if(dw.size()>=64){ //一致文字列をまず検索
					if(dw.ExactMatch(w))goto match_hamming_pre;
				}

				if(dist ==1){
					if(dw.HammingMatch1(w,qq.alphabet[j]))goto match_hamming_pre;
				}else{
					if(dw.HammingMatch(w,qq.alphabet[j],dist))goto match_hamming_pre;
				}

				wf[qq.wordIndex[j]]=-1;
				goto no_match_hamming;

match_hamming_pre:; //マッチした場合だけ処理
				wf[qq.wordIndex[j]]=dist;
			}
			//マッチした
			{
				const auto &ids = *qq.QueryIDs;
				query_ids.insert(query_ids.end(),ids.begin(),ids.end());
				//for(unsigned int i =0;i<ids.size();i++){ query_ids.push_back(ids[i]); }
			}

no_match_hamming:;
		}
	}
}


MY_INLINE void calc_class::calcED()
{
	auto const &qs = (*queries)[MT_EDIT_DIST]; 
	int max_word_len = MAX_WORD_LENGTH;
	int min_word_len = MIN_WORD_LENGTH;
	for(;doc_words[max_word_len].size()==0 && max_word_len>MIN_WORD_LENGTH;max_word_len--);
	for(;doc_words[min_word_len].size()==0 && min_word_len<MAX_WORD_LENGTH;min_word_len++);
	if(max_word_len < min_word_len)return; //マッチする可能性が無い

	wf.assign(qs.wordSet.vec_words_offset[2],0);
	for(int dist=1;dist<=3;dist++){
		auto const & mq = qs.queries[dist-1];
		if(mq.QueryVector.empty())continue;


		for(auto vi =0U;vi<mq.QueryVector.size();vi++){
			const auto &qq =mq.QueryVector[vi];
			const auto &quer = qq.strs;
			char state[5]={};

			assert(quer.words_len>0);
			for(unsigned int j=0;j<quer.words_len;j++){
				auto index = qq.wordIndex[j];
				assert(0<=index && index < wf.size());

				auto s = wf[index];
				if(s<=-dist){goto no_match_edit;}
				state[j]=s;
			}
			assert(quer.words_len>0);
			for(unsigned int j=0;j<quer.words_len;j++){
				assert(state[j]>-dist);

				if(state[j] > 0){
					//if(state[j]==dist)printf(".");
					//else printf("#");
					continue;
				}
				const Word_32 &w = quer.words[j];

				const int len = w.word.len;
				if( (len + dist >= min_word_len) && 
					(len - dist <= max_word_len))
				{
					const unsigned int a = qq.alphabet[j];

					if(true){ //一致文字列をまず検索
						const auto & dw = doc_words[len];
						if(dw.ExactMatch(w))goto match_edit_pre;
					}

					if(dist == 1){
						if(doc_words[len].HammingMatch1(w,a))goto match_edit_pre;
						if(len - 1 >=min_word_len){
							if( doc_words[len-1].EditDistMatch1_1(w,a))goto match_edit_pre;
						}
						if(len+1<= max_word_len){
							if( doc_words[len+1].EditDistMatch1_2(w,a))goto match_edit_pre;
						}
					}else{
						const int s = max(min_word_len,len -(int) dist);
						const int e = min(max_word_len,len +(int) dist);
						assert(s<=e);
						if(len <=16 && e <= 16){
							const EditDistFast edf(w);
							for(int r=s;r<=e;r++){
								const auto & dw = doc_words[r];
								if(dw.size()==0)continue;
								const int d = dist*2- abs(len-r);
								if(dw.EditDistMatch(edf,a,d,dist,len-r))goto match_edit_pre;
							}					
						}else{
							EditDist ed(w.word.str,len);
							for(int r=s;r<=e;r++){
								const auto & dw = doc_words[r];
								if(dw.size()==0)continue;
								const int d = dist*2- abs(len-r);
								if(len<=16 && r<=16){
									const EditDistFast edf(w);
									if(dw.EditDistMatch(edf,a,d,dist,len-r))goto match_edit_pre;
								}else{
									if(dw.EditDistMatch(ed,a,d,dist))goto match_edit_pre;
								}
							}
						}
					}
				}
				wf[qq.wordIndex[j]]=-dist;
				goto no_match_edit;

match_edit_pre:; //マッチした場合だけ処理
				//				wf[itr->second.wordIndex[j]]= dist;
				wf[qq.wordIndex[j]]= 1;
			}
			//マッチした
			{
				const auto &ids = *qq.QueryIDs;
				query_ids.insert(query_ids.end(),ids.begin(),ids.end());
				//for(unsigned int i =0;i<ids.size();i++){ query_ids.push_back(ids[i]); }
			}

no_match_edit:;
		}
	}
}
MY_INLINE Document calc_class::returnDocument()
{
	Document doc;
	doc.doc_id=di->doc_id;
	doc.num_res = (unsigned int)query_ids.size();
	doc.query_ids=NULL;
	if(doc.num_res){
		//gfx::timsort(query_ids.begin(),query_ids.end());
		//sort(query_ids.begin(),query_ids.end());
		doc.query_ids=(unsigned int*)malloc(doc.num_res*sizeof(unsigned int));
		memcpy(doc.query_ids,&query_ids[0],sizeof(QueryID) * doc.num_res);
		gfx::timsort(doc.query_ids,doc.query_ids+doc.num_res);
		//sort(doc.query_ids,doc.query_ids+doc.num_res);
	}
	return doc;		
}

MY_INLINE Document calc_class::calc(const vector<QuerySetSet> &queries,DocumentInput & di)
{
	this->di=&di;
	this->queries = &queries;
	for(int i=MIN_WORD_LENGTH;i<=MAX_WORD_LENGTH;i++)doc_words[i].clear();
	query_ids.clear();

	getDocWordSet();

	calcExact();
	calcHamming();
	calcED();

	return  returnDocument();
}

///////////////////////////////////////////////////////////////////////////////////////////////

void * work_func( void * arg );

class WorkerThread{
public:
	static const int THREAD_MAX=24; //TODO :スレッド数の適正値を割り出し
	pthread_t thread[THREAD_MAX];
	ts_queue<DocumentInput> job_que;
	ts_queue<Document> end_que;

	void Create()
	{
		job_que = ts_queue<DocumentInput>();
		end_que = ts_queue<Document>();

		for(int i=0;i<THREAD_MAX;i++){
			pthread_create( &thread[i], 0, work_func, this );
		}
	}

	void Destruct()
	{
		for(int i=0;i<THREAD_MAX;i++){job_que.enqueue(DocumentInput(0,NULL));}
		for(int i=0;i<THREAD_MAX;i++){pthread_join(thread[i],NULL);}
		while(end_que.size()!=0){ //後始末
			Document d = end_que.dequeue();
			free(d.query_ids);
		}
	}
};



WorkerThread worker;
vector<QuerySetSet> g_queries;
int activeDocumentCount;

void * work_func( void * arg )
{
	WorkerThread * wt = (WorkerThread *) arg;
	calc_class cc;
	while (true)
	{
		DocumentInput di = wt->job_que.dequeue();
		if(di.str ==NULL) break;//スレッド終了用
		Document d = cc.calc(g_queries,di);
		free((char *)di.str);
		wt->end_que.enqueue(d);
	}
	return 0;
}


ErrorCode InitializeIndex()
{
	g_queries.resize(3);
	g_queries[MT_EXACT_MATCH].queries.resize(1);
	g_queries[MT_HAMMING_DIST].queries.resize(3);
	g_queries[MT_EDIT_DIST].queries.resize(3);
	g_queries[MT_EXACT_MATCH].isUseWordSet=false;
	g_queries[MT_HAMMING_DIST].isUseWordSet=true;
	g_queries[MT_EDIT_DIST].isUseWordSet=true;

	worker.Create();
	activeDocumentCount=0;

	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode DestroyIndex()
{
	//色々開放する
	worker.Destruct();
	g_queries.clear();
	return EC_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist)
{
	if(match_type == MT_EXACT_MATCH){match_dist = 0;}//念のため
	if(match_dist == 0){match_type = MT_EXACT_MATCH;}//念のため
	g_queries[match_type].addQuery(match_dist,query_id,query_str);
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////


ErrorCode EndQuery(QueryID query_id)
{
	for(unsigned int i=0;i<MAX_QUERY_SET;i++){
		if(g_queries[i].removeQuery(query_id)){return EC_SUCCESS;}
	}
	return EC_FAIL;
}

///////////////////////////////////////////////////////////////////////////////////////////////


ErrorCode MatchDocument(DocID doc_id, const char* doc_str)
{
	g_queries[MT_EXACT_MATCH].update();
	g_queries[MT_HAMMING_DIST].update();
	g_queries[MT_EDIT_DIST].update();

	char * buf = (char * )malloc(sizeof(char) * (MAX_DOC_LENGTH+32));

	//アクセス範囲外にならないように多めに確保？

	strcpy(buf,doc_str);
	worker.job_que.enqueue(DocumentInput(doc_id,buf));
	activeDocumentCount++;
	return EC_SUCCESS;
}


ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids)
{
	*p_doc_id=0; *p_num_res=0; *p_query_ids=0;
	if(activeDocumentCount==0)return EC_NO_AVAIL_RES;
	Document d = worker.end_que.dequeue();
	*p_doc_id = d.doc_id;*p_num_res = d.num_res;*p_query_ids = d.query_ids;
	activeDocumentCount--;
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
