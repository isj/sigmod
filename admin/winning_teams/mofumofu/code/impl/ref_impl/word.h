#pragma once 

#include <nmmintrin.h>

using namespace std;


//アライメントを守ること。stl注意。カスタムアロケーターが必須
union Word_32
{
	struct stWord_32{
		char str[MAX_WORD_LENGTH];
		unsigned char len;
	}word;
	long long int pack[4];
	__m128i pack_sse[2];

	//Word_32(){;} //何もしない

	//Word_32(const Word_32& obj)
	//{
	//	pack_sse[0] = obj.pack_sse[0];
	//	pack_sse[1] = obj.pack_sse[1];
	//}

	//謎ソート
	MY_INLINE bool operator<(const Word_32& right)  const 
	{
		//for(int i=0;i<4;i++){
		//	long long int sa= pack[i]-right.pack[i];
		//	if(sa)return sa<0;
		//}
		//return false;
		long long int sa0= pack[0]-right.pack[0];
		if(sa0)return sa0<0;

		long long int sa3= pack[3]-right.pack[3];
		long long int sa2= pack[2]-right.pack[2];
		long long int sa1= pack[1]-right.pack[1];
		long long int ret = sa3;
		if(sa2!=0)ret = sa2;
		if(sa1!=0)ret = sa1;
		return ret <0;
	}

	MY_INLINE bool operator==(const Word_32& right) const 
	{
		//assert(word.len <=MAX_WORD_LENGTH);
		//assert(right.word.len<=MAX_WORD_LENGTH);
		const long long int *aa =pack;
		const long long int *bb =right.pack;

		return (aa[0]==bb[0])&(aa[1]==bb[1])&(aa[2]==bb[2])&(aa[3]==bb[3]);
	}


	// Computes Hamming distance between a null-terminated string "a" with length "na"
	//  and a null-terminated string "b" with length "nb" 
	MY_INLINE unsigned int HammingDistance(const Word_32 &b) const 
	{
		int aa = _mm_movemask_epi8(_mm_cmpeq_epi8(pack_sse[0],b.pack_sse[0]));
		int bb = _mm_movemask_epi8(_mm_cmpeq_epi8(pack_sse[1],b.pack_sse[1]));
#ifdef SSE4_2
		return 32 - (unsigned int)_mm_popcnt_u32(aa) - (unsigned int)_mm_popcnt_u32(bb);
#else
		return 32 - (unsigned int)BitCount16_2(aa,bb);
#endif
	}
};
struct Word_32_length_comp : public binary_function<Word_32, Word_32, bool>{
public:
    bool operator()(const Word_32& left, const Word_32& right) const 
	{
		if(left.word.len != right.word.len)return left.word.len < right.word.len;
		for(int i=0;i<4;i++){
			long long int sa= left.pack[i]-right.pack[i];
			if(sa)return sa<0;
		}
		return false;
	}
};
struct Word_32_length_comp_R : public binary_function<Word_32, Word_32, bool>{
public:
    bool operator()(const Word_32& left, const Word_32& right) const 
	{
		if(left.word.len != right.word.len)return left.word.len > right.word.len;
		for(int i=0;i<4;i++){
			long long int sa= left.pack[i]-right.pack[i];
			if(sa)return sa<0;
		}
		return false;
	}
};
union Word_16
{
	struct stWord_16{
		char str[16];
	}word;
	long long int pack[2];
	__m128i pack_sse;
	
	Word_16(){}
	explicit Word_16(const __m128i &w){pack_sse = w;}
	explicit Word_16(const Word_32 &w){pack_sse = w.pack_sse[0];}
	//Word_32(){;} //何もしない

	//Word_32(const Word_32& obj)
	//{
	//	pack_sse[0] = obj.pack_sse[0];
	//	pack_sse[1] = obj.pack_sse[1];
	//}

	//謎ソート
	MY_INLINE bool operator<(const Word_16& right)  const 
	{
		//for(int i=0;i<2;i++){
		//	long long int sa= pack[i]-right.pack[i];
		//	if(sa)return sa<0;
		//}
		//return false;
		long long int sa1= pack[1]-right.pack[1];
		long long int sa0= pack[0]-right.pack[0];
		long long int ret = sa1;
		if(sa0!=0)ret = sa0;
		return ret <0;
	}

	MY_INLINE bool operator==(const Word_16& right) const 
	{
		//assert(word.len <=MAX_WORD_LENGTH);
		//assert(right.word.len<=MAX_WORD_LENGTH);
		const long long int *aa =pack;
		const long long int *bb =right.pack;

		return (aa[0]==bb[0])&(aa[1]==bb[1]);
	}

	MY_INLINE unsigned short HammingMatchResult(const Word_16 &b) const 
	{
		return ~(unsigned short) _mm_movemask_epi8(_mm_cmpeq_epi8(pack_sse,b.pack_sse));
	}

	// Computes Hamming distance between a null-terminated string "a" with length "na"
	//  and a null-terminated string "b" with length "nb" 
	MY_INLINE unsigned int HammingDistance(const Word_16 &b) const 
	{
		int aa = _mm_movemask_epi8(_mm_cmpeq_epi8(pack_sse,b.pack_sse));
#ifdef SSE4_2
		return 16 - (unsigned int)_mm_popcnt_u32(aa) ;
#else
		return 16 - (unsigned int)BitCount32(aa);
#endif
	}
};


union Word_8
{
	struct stWord_8{
		char str[8];
	}word;
	long long int pack;

	Word_8(){}
	explicit Word_8(const Word_32 &w){pack = w.pack[0];}
	explicit Word_8(const Word_16 &w){pack = w.pack[0];}

	Word_16 ToWord_16()const{return Word_16(_mm_cvtsi64_si128(pack));}
	//Word_32(const Word_32& obj)
	//{
	//	pack_sse[0] = obj.pack_sse[0];
	//	pack_sse[1] = obj.pack_sse[1];
	//}

	//謎ソート
	MY_INLINE bool operator<(const Word_8& right)  const 
	{
		long long int sa= pack-right.pack;
		return sa<0;
	}

	MY_INLINE bool operator==(const Word_8& right) const 
	{
		return (pack == right.pack);
	}

	MY_INLINE unsigned short HammingMatchResult(const Word_8 &b) const 
	{
		return ~(unsigned short)  (unsigned int)_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_cvtsi64_si128(pack),_mm_cvtsi64_si128(b.pack)));
	}

	MY_INLINE unsigned int HammingDistance(const Word_8 &b) const 
	{
		int aa = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_cvtsi64_si128(pack),_mm_cvtsi64_si128(b.pack)));
#ifdef SSE4_2
		return 16 - (unsigned int)_mm_popcnt_u32(aa) ;
#else
		return 16 - (unsigned int)BitCount32(aa);
#endif
	}
};


MY_INLINE unsigned int GetAlphabetSet(const Word_32& w){
	assert(w.word.len<=31);
	unsigned int a = 0U;
	for(int i=0;i<w.word.len;i++){ a |= 1U<<(w.word.str[i]-'a'); }
	return a;
}
MY_INLINE unsigned int GetAlphabetSet(const char * str,unsigned int len){
	assert(len<=31);
	unsigned int a = 0U;
	for(unsigned int i=0;i<len;i++){ a |= 1U<<(str[i]-'a'); }
	return a;
}


//struct GetLetter { inline unsigned long long int operator()(const Word_32 &key, size_t depth)const {return key.pack[depth];}};
