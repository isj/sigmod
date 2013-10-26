#pragma once

static const int alignment = 16;

#include "util.h"
#include <cstdlib>
#include <cstddef>

template <class T>
class AlignmentAllocator
{
public:
	// 型定義
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;
	
	// アロケータをU型にバインドする
	template <class U>
	struct rebind
	{
		typedef AlignmentAllocator<U> other;
	};
	
	// コンストラクタ
	AlignmentAllocator() throw(){}
	AlignmentAllocator(const AlignmentAllocator&) throw(){}
	template <class U> AlignmentAllocator(const AlignmentAllocator<U>&) throw(){}
	// デストラクタ
	~AlignmentAllocator() throw(){}
	
	// メモリを割り当てる
	pointer allocate(size_type num, const void * hint = 0)
	{
		return (pointer)( _aligned_malloc(num * sizeof(T) ,alignment) );
	}
	// 割当て済みの領域を初期化する
	void construct(pointer p, const T& value)
	{
		new( p ) T(value);
	}
	
	// メモリを解放する
	void deallocate(pointer p, size_type num)
	{
		_aligned_free(p);
	}
	// 初期化済みの領域を削除する
	void destroy(pointer p)
	{
		p->~T();
	}
	
	// アドレスを返す
	pointer address(reference value) const { return &value; }
	const_pointer address(const_reference value) const { &value; }
	
	// 割当てることができる最大の要素数を返す
	size_type max_size() const throw()
	{
		const size_t s = ~((size_t)0);
		return s / sizeof(T);
	}

	
	inline bool operator==(const AlignmentAllocator& right) const 
	{
		return ((void *)this) == &right;
	}
};