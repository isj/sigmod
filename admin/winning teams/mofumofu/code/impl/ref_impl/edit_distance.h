#pragma once

#include <algorithm>
#include <nmmintrin.h>

#include "util.h"
#include "word.h"

using namespace std;

static ALWAYS_INLINE bool isAllOne(const unsigned short a){return a == (unsigned short)~0U;}
static ALWAYS_INLINE unsigned short getModifiedOne(const unsigned short a){return a|(a+1);}
static ALWAYS_INLINE unsigned short getDeleteOne(const unsigned short m1,const unsigned short m2){return (m1&~(m1+1))|m2;}
static ALWAYS_INLINE unsigned short getInsertOne(const unsigned short m1,const unsigned short m2){return (m1^(m1+1))|m2;}

ALWAYS_INLINE bool EditDistance1(const Word_16 a,const Word_16 b)
{
	//a.len > b.len
	const __m128i am = a.pack_sse;
	const __m128i bm = b.pack_sse;

	unsigned short m1 = _mm_movemask_epi8( _mm_cmpeq_epi8(am,bm));
	unsigned short m2 = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_srli_si128(am,1),bm));
	bool ret = isAllOne(getDeleteOne(m1,m2));
	return ret;
}

class EditDistFast{
public:
	Word_16 w;
	EditDistFast(const Word_32 & _w):w(_w){assert(_w.word.len<=16);}

	static MY_INLINE bool getEditState(const int d,const Word_16 &a, const Word_16 &b, unsigned int limitDist);
	MY_INLINE bool getEditState(const int diff,const Word_16 &b, unsigned int limitDist)const{
		return getEditState(diff,w,b,limitDist);
	}

	MY_INLINE bool getEditStateR(const int diff,const Word_16 &b, unsigned int limitDist)const{
		return getEditState(-diff,b,w,limitDist);
	}
};

MY_INLINE bool EditDistFast::getEditState(const int d,const Word_16 &a, const Word_16 &b, unsigned int limitDist)
{
	assert(d>=0);
	__m128i am = a.pack_sse;
	__m128i bm = b.pack_sse;
	bool ret;
	const unsigned short m1 = _mm_movemask_epi8( _mm_cmpeq_epi8(am,bm));
	const unsigned short m2r = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_srli_si128(am,1),bm));
	if(limitDist==2)
	{
		if(d==0) //len(am)=len(bm)
		{
			const unsigned short m2l = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_slli_si128(am,1),bm));
			
			//m2 (modify 2)
			ret =  isAllOne(getModifiedOne(getModifiedOne(m1)));

			//+1,-1  (insert 1, delete 1)
			ret |= isAllOne(getDeleteOne(getInsertOne(m1,m2l),m1));
			//-1,+1
			ret |= isAllOne(getInsertOne(getDeleteOne(m1,m2r),m1)); 
		}
		else if(d==1) //len(am) = len(bm)+1
		{
			//-1,m1
			ret = isAllOne(getModifiedOne(getDeleteOne(m1,m2r)));

			//m1,-1
			ret |= isAllOne(getDeleteOne(getModifiedOne(m1),m2r));
		}
		else //len(am) = len(bm)+2
		{
			assert(d==2);
			unsigned short m3 = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_srli_si128(am,2),bm));
			ret = isAllOne(getDeleteOne(getDeleteOne(m1,m2r),m3));
		}
	}else{
		assert(limitDist ==3);
		if(d==0) //len(am) = len(bm)
		{
			const unsigned short m2l = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_slli_si128(am,1),bm));
			
			// m3
			ret =  isAllOne(getModifiedOne(getModifiedOne(getModifiedOne(m1))));

			// m1,+1,-1
			ret |= isAllOne(getDeleteOne(getInsertOne(getModifiedOne(m1),m2l),m1));

			// m1,-1,+1
			ret |= isAllOne(getInsertOne(getDeleteOne(getModifiedOne(m1),m2r),m1));

			
			// -1,+1,m1
			ret |= isAllOne(getModifiedOne(getInsertOne(getDeleteOne(m1,m2r),m1)));

			// -1,m1,+1
			ret |= isAllOne(getInsertOne(getModifiedOne(getDeleteOne(m1,m2r)),m1));

			// +1,-1,m1
			ret |= isAllOne(getModifiedOne(getDeleteOne(getInsertOne(m1,m2l),m1)));

			//+1,m1,-1
			ret |= isAllOne(getDeleteOne(getModifiedOne(getInsertOne(m1,m2l)),m1));
		}
		else if(d==1)//len(am) = len(bm)+1
		{
			const unsigned short m2l = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_slli_si128(am,1),bm));
			const unsigned short m3 = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_srli_si128(am,2),bm));

			//m1,m1,-1
			ret =  isAllOne(getDeleteOne(getModifiedOne(getModifiedOne(m1)),m2r));
			//m1,-1,m1
			ret |= isAllOne(getModifiedOne(getDeleteOne(getModifiedOne(m1),m2r)));

			//-1,m1,m1
			ret |= isAllOne(getModifiedOne(getModifiedOne(getDeleteOne(m1,m2r))));

			//-1,-1,+1
			ret |= isAllOne(getInsertOne(getDeleteOne(getDeleteOne( m1,m2r),m3),m2r));

			//-1,+1,-1
			ret |= isAllOne(getDeleteOne(getInsertOne(getDeleteOne( m1,m2r),m1),m2r));

			//+1,-1,-1
			ret |= isAllOne(getDeleteOne(getDeleteOne( getInsertOne(m1,m2l),m1),m2r));

		}
		else if(d==2) //len(am) = len(bm)+2
		{
			const unsigned short m3 = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_srli_si128(am,2),bm));

			ret = isAllOne(getModifiedOne( getDeleteOne   (getDeleteOne  (m1,m2r) ,m3)));
			ret|= isAllOne(getDeleteOne  ( getModifiedOne (getDeleteOne  (m1,m2r)),m3));
			ret|= isAllOne(getDeleteOne  ( getDeleteOne   (getModifiedOne(m1),m2r),m3));

		}
		else //len(am) = len(bm)+3
		{
			assert(d==3);
			const unsigned short m3 = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_srli_si128(am,2),bm));
			const unsigned short m4 = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_srli_si128(am,3),bm));
			ret = isAllOne(getDeleteOne(getDeleteOne(getDeleteOne(m1,m2r),m3),m4));
		}
	}
	return ret;
}


///////////////////////////////////////////////////////////////////////////////////////////////
// H. Hyyro. A Bit-Vector Algorithm for Computing Levenshtein and Damerau Edit Distances.
// Nordic Journal of Computing, 10:1-11, 2003.
///////////////////////////////////////////////////////////////////////////////////////////////
class EditDist{
public:
	static const int SIZE=26;
	int m;
	const char* d_str;
	unsigned int PMs[SIZE];

	EditDist(const char * str,unsigned int len) {
		assert(len<=31);
		m=len;
		d_str=str;
		memset(PMs,0,sizeof(PMs));
		for (unsigned int i = 0; i < len; i++)PMs[str[i]-'a' ] |= 1L << i;
	}

	MY_INLINE int getDist(const char *str,unsigned int len,unsigned int limitDist)const;
};

MY_INLINE int EditDist::getDist(const char *str,unsigned int len,unsigned int limitDist)const {
	assert(len<=31);
	int D = m;
	unsigned int PM = PMs[str[0]-'a'];
	unsigned int top = 1L << (m - 1);
	unsigned int VP = ~0;
	unsigned int VN = 0;
	const int oo= 64;

	for (unsigned int i=0;i<len-1;i++){
		unsigned int D0 = (((PM & VP) + VP) ^ VP) | (PM | VN);
		PM = PMs[str[i+1]-'a'];
		unsigned int HP = VN | ~(D0 | VP);
		unsigned int HN = D0 & VP;
		VP = (HN << 1) | ~(D0 | ((HP << 1) | 1));
		VN = D0 & ((HP << 1) | 1);
		if ((HP & top) != 0) {++D;}
		else if ((HN & top) != 0) {--D;continue;}
		//D += ((HP & top) != 0); //ê≥ìñê´ïsñæ
		//D -= ((HN & top) != 0);
		if(D+i >= limitDist +len)return oo;
	}
	{
		unsigned int D0 = (((PM & VP) + VP) ^ VP) | (PM | VN);
		unsigned int HP = VN | ~(D0 | VP);
		unsigned int HN = D0 & VP;
		
		if ((HP & top) != 0) {++D;}
		else if ((HN & top) != 0) {--D;}
		//D += ((HP & top) != 0); //ê≥ìñê´ïsñæ
		//D -= ((HN & top) != 0);
	}
	return D;
}


////////////////////////////////////////////////////////////////////////////////////////////////

MY_INLINE int EditDistance1(const char* __restrict a, int na,const char* __restrict b, int nb)
{
	//if(na<nb){swap(na,nb);swap(a,b);}
	assert(na - nb == 1);
	const int r=1;
	const int oo= 64;
	int T[2];
	{
		T[0] =1;
		char c= a[0];
		int  d3=(c!=b[0]);
		T[1]=d3;
	}
	for(int ia=2;ia<=na;ia++)
	{
		const int s = ia - 1;

		char c= a[ia-1];
		int prev2 = T[0];
		int minret;
		{
			int  d3=prev2 + (c!=b[s-1]);
			prev2 = T[1];
			int ret = prev2+1;
			if(d3<ret)ret=d3;

			T[0]=ret;
			minret=ret;
		}
		{
			int ret=prev2 + (c!=b[s]);
			T[1]=ret;
			if(ret<minret)minret=ret;
		}
		//é}ä†ÇË
		if(minret >r) return oo;
	}
	return T[0];
}
