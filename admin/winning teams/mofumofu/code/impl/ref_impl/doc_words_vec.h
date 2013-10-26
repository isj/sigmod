#pragma once 
#include <vector>
#include <cstdio>

#include "AlignmentAllocator.h"
#include "word.h"
#include "edit_distance.h"

class doc_words_vec{
	unsigned int len;
	size_t sz;

	vector<Word_8 ,AlignmentAllocator<Word_8 > > w8;
	vector<Word_16,AlignmentAllocator<Word_16> > w16;
	vector<Word_32,AlignmentAllocator<Word_32> > w32;
	vector<unsigned int,AlignmentAllocator<unsigned int> > alp;

	void addWord(const Word_8  &w){w8.push_back(w);}
	void addWord(const Word_16 &w){w16.push_back(w);}
	void addWord(const Word_32 &w){w32.push_back(w);}
public:
	void setLen(int l){assert(l<=31);len=l;sz=0;}
	size_t size()const {return sz;}

	MY_INLINE void addWordCnv(const Word_32 &w){
		//assert(w.word.len == len);
		if(len<=8){	addWord(Word_8(w));}
		else if(len<=16) { addWord(Word_16(w));}
		else { addWord(w);}
	}

	MY_INLINE void addWordCnv8_16(const Word_16 &w){
		//assert(w.word.len == len);
		if(len<=8){addWord(Word_8(w));}
		else{addWord(w);}
	}

	MY_INLINE void addWordCnv16_32(const Word_32 &w){
		//assert(w.word.len == len);
		if(len<=16){addWord(Word_16(w));}
		else{addWord(w);}
	}
	MY_INLINE void clear(){
		w8.clear();w16.clear();w32.clear();
	}

	MY_INLINE void UniqeAndSetAlphabet(){
		//const int buf_size=8;
		const int buf_size=0;

		if(len<=8){
			auto&dw  =w8;
			if(dw.size()>1){
				sort(dw.begin(),dw.end());
				dw.erase( unique(dw.begin(),dw.end()), dw.end());
			}
			sz = dw.size();
			alp.resize(sz+buf_size);
			for(unsigned int j=0 ; j<sz;j++){alp[j] = GetAlphabetSet(dw[j].word.str,len);}
			//alp[sz]=alp[sz+1]=alp[sz+2]=alp[sz+3]=~(unsigned int)0;
		}else if(len<=16){
			auto&dw  =w16;
			if(dw.size()>1){
				sort(dw.begin(),dw.end());
				dw.erase( unique(dw.begin(),dw.end()), dw.end());
			}
			sz = dw.size();
			alp.resize(sz+buf_size);
			for(unsigned int j=0 ; j<sz;j++){alp[j] = GetAlphabetSet(dw[j].word.str,len);}
			//alp[sz]=alp[sz+1]=alp[sz+2]=alp[sz+3]=~(unsigned int)0;
		}else{
			auto&dw  =w32;
			if(dw.size()>1){
				sort(dw.begin(),dw.end());
				dw.erase( unique(dw.begin(),dw.end()), dw.end());
			}
			sz = dw.size();
			alp.resize(sz+buf_size);
			for(unsigned int j=0 ; j<sz;j++){alp[j] = GetAlphabetSet(dw[j].word.str,len);}
		}
	}

	MY_INLINE bool ExactMatch(const Word_32 &q)const;
	MY_INLINE bool HammingMatch1(const Word_32 &q,const unsigned int a)const;
	MY_INLINE bool HammingMatch(const Word_32 &q,const unsigned int a,const int dist)const;
	MY_INLINE bool EditDistMatch(const EditDist &ed, const unsigned int a, const int d,const int dist)const;
	MY_INLINE bool EditDistMatch(const EditDistFast &edf, const unsigned int a, const int d,const int dist,const int diff)const;


	MY_INLINE bool EditDistMatch1_1( const Word_32 &w,const unsigned int a)const;
	MY_INLINE bool EditDistMatch1_2( const Word_32 &w,const unsigned int a)const;
};

MY_INLINE bool doc_words_vec::ExactMatch(const Word_32 &q)const
{
	assert(q.word.len == len);
	if(sz==0)return false;
	if(len <=8){
		Word_8 w(q);
		const auto & dw = w8;
		if(sz>=16){ return binary_search(dw.begin(),dw.end(),w);}
		else{ for(unsigned int k=0;k<sz;k++){ if(w == dw[k])return true;}}

	}else if(len <=16){
		Word_16 w(q);
		const auto & dw = w16;
		if(sz>=16){ return binary_search(dw.begin(),dw.end(),w);}
		else{ for(unsigned int k=0;k<sz;k++){ if(w == dw[k])return true;} }

	}else{
		const Word_32 &w = q;
		const auto & dw = w32;
		if (sz>=16){return binary_search(dw.begin(),dw.end(),w);}
		else { for(unsigned int k=0;k<sz;k++){ if(w == dw[k])return true; } }
	}
	return false;
}

MY_INLINE bool doc_words_vec::HammingMatch1(const Word_32 &q,const unsigned int a)const
{
	assert(q.word.len == len);
	const int dist =1;
	if(len <=8){
		Word_8 w(q);
		const auto & dw = w8;
		for(int k=(int)sz;k-->0;){
			auto x=w.HammingMatchResult(dw[k]);
			auto ans = (unsigned short)(x&(x-1)) ;
			if(ans==0 ){return true;}
		}
	}else if(len <=16){
		Word_16 w(q);
		const auto & dw = w16;
		for(int k=(int)sz;k-->0;){
			//if(MY_POPCOUNT(a ^ alp[k]) > dist*2)continue;
			auto x=w.HammingMatchResult(dw[k]);
			auto ans = (unsigned short)(x&(x-1)) ;
			if(ans==0 ){return true;}
		}
	}else{
		const Word_32 & w = q;
		const auto & dw = w32;
		for(int k=(int)sz;k-->0;){
			if(MY_POPCOUNT(a ^ alp[k]) > dist*2)continue;
			auto num_mismatches=w.HammingDistance(dw[k]);
			if(num_mismatches<=(unsigned int)dist)return true;
		}
	}
	return false;
}

MY_INLINE bool doc_words_vec::HammingMatch(const Word_32 &q,const unsigned int a,const int dist)const
{
	assert(q.word.len == len);
	assert(1<=dist && dist<=3);
	if(len <=8){
		Word_8 w(q);
		const auto & dw = w8;
		for(int k=(int)sz;k-->0;){
			auto num_mismatches=w.HammingDistance(dw[k]);
			if(unlikely( num_mismatches<=(unsigned int)dist))return true;
		}
	}else if(len <=16){
		Word_16 w(q);
		const auto & dw = w16;
		for(int k=(int)sz;k-->0;){
			if(MY_POPCOUNT(a ^ alp[k]) > dist*2)continue;
			auto num_mismatches=w.HammingDistance(dw[k]);
			if(num_mismatches<=(unsigned int)dist)return true;
		}
	}else{
		const Word_32 & w = q;
		const auto & dw = w32;
		for(int k=(int)sz;k-->0;){
			if(MY_POPCOUNT(a ^ alp[k]) > dist*2)continue;
			auto num_mismatches=w.HammingDistance(dw[k]);
			if(num_mismatches<=(unsigned int)dist)return true;
		}
	}
	return false;
}

MY_INLINE bool doc_words_vec::EditDistMatch(const EditDistFast &edf, const unsigned int a, const int d,const int dist,const int diff)const
{
	assert(len<=16);
	assert(2<=dist && dist<=3);
	if(diff>=0){
		if(len<=8){
			//for(unsigned int k=0;k<sz;k++){
			//	if(likely(MY_POPCOUNT(alp[k]^a) >d)){ continue;}
			//	if(edf.getEditState(diff, w8[k].ToWord_16(),dist))return true;
			//}

			const unsigned int * pp = &(alp[0]);
			for(unsigned int k=0;k<sz;k++,pp++){
				if(likely(MY_POPCOUNT(pp[0]^a) >d)){ continue;}
				if(edf.getEditState(diff, w8[k].ToWord_16(),dist))return true;
			}
		}else{
			for(unsigned int k=0;k<sz;k++){
				if(likely(MY_POPCOUNT(alp[k]^a) >d)){ continue;}
				if(edf.getEditState(diff, w16[k],dist))return true;
			}
		}
	}else{
		if(len<=8){
			const unsigned int * pp = &(alp[0]);
			for(unsigned int k=0;k<sz;k++,pp++){
				if(likely(MY_POPCOUNT(pp[0]^a) >d)){ continue;}
				if(edf.getEditStateR(diff, w8[k].ToWord_16(),dist))return true;
			}
		}else{
			for(unsigned int k=0;k<sz;k++){
				if(likely(MY_POPCOUNT(alp[k]^a) >d)){ continue;}
				if(edf.getEditStateR(diff, w16[k],dist))return true;
			}
		}
	}
	return false;
}

MY_INLINE bool doc_words_vec::EditDistMatch(const EditDist &ed, const unsigned int a, const int d,const int dist)const
{
	assert(len>8);
	if(len<=16){
		for(unsigned int k=0;k<sz;k++){
			if(likely(MY_POPCOUNT(alp[k]^a) >d)){ continue;}
			const char * str = w16[k].word.str;
			int edit_dist = ed.getDist(str, len,dist);
			if(edit_dist<=dist) return true;
		}
	}else{
		for(unsigned int k=0;k<sz;k++){
			if(likely(MY_POPCOUNT(alp[k]^a) >d)){ continue;}
			const char * str = w32[k].word.str;
			int edit_dist = ed.getDist(str, len,dist);
			if(edit_dist<=dist) return true;
		}
	}
	return false;
}


MY_INLINE bool doc_words_vec::EditDistMatch1_1( const Word_32 &w,const unsigned int a)const{
	assert(len < w.word.len);
	if(w.word.len<=16){
		Word_16 ww(w);
		for(unsigned int k=0;k<sz;k++){
			auto x = alp[k] ^ a;
			if((x&(x-1)) !=0) continue;
			Word_16 wt;
			if(len<=8)       wt =w8[k].ToWord_16();
			else wt =w16[k];
			if(EditDistance1(ww,wt))return true;
		}
	}else if(len<=16){
		for(unsigned int k=0;k<sz;k++){
			auto x = alp[k] ^ a;
			if((x&(x-1)) !=0) continue;
			if(EditDistance1(w.word.str, w.word.len, w16[k].word.str, len)<=1)return true;
		}
	}else{
		for(unsigned int k=0;k<sz;k++){
			auto x = alp[k] ^ a;
			if((x&(x-1)) !=0) continue;
			if(EditDistance1(w.word.str, w.word.len, w32[k].word.str, len)<=1)return true;
		}
	}
	return false;
}

MY_INLINE bool doc_words_vec::EditDistMatch1_2( const Word_32 &w,const unsigned int a)const{
	assert(len > w.word.len);
	if(len<=16){
		Word_16 ww(w);
		for(unsigned int k=0;k<sz;k++){
			auto x = alp[k] ^ a;
			if((x&(x-1)) !=0) continue;
			Word_16 wt;
			if(len<=8) wt =w8[k].ToWord_16();
			else wt =w16[k];

			if(EditDistance1(wt,ww))return true;
		}
	}else{
		for(unsigned int k=0;k<sz;k++){
			auto x = alp[k] ^ a;
			if((x&(x-1)) !=0) continue;
			if(EditDistance1(w32[k].word.str, len ,w.word.str, w.word.len)<=1)return true;
		}
	}

	return false;
}