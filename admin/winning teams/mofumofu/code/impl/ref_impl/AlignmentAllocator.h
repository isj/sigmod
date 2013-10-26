#pragma once

static const int alignment = 16;

#include "util.h"
#include <cstdlib>
#include <cstddef>

template <class T>
class AlignmentAllocator
{
public:
	// �^��`
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;
	
	// �A���P�[�^��U�^�Ƀo�C���h����
	template <class U>
	struct rebind
	{
		typedef AlignmentAllocator<U> other;
	};
	
	// �R���X�g���N�^
	AlignmentAllocator() throw(){}
	AlignmentAllocator(const AlignmentAllocator&) throw(){}
	template <class U> AlignmentAllocator(const AlignmentAllocator<U>&) throw(){}
	// �f�X�g���N�^
	~AlignmentAllocator() throw(){}
	
	// �����������蓖�Ă�
	pointer allocate(size_type num, const void * hint = 0)
	{
		return (pointer)( _aligned_malloc(num * sizeof(T) ,alignment) );
	}
	// �����čς݂̗̈������������
	void construct(pointer p, const T& value)
	{
		new( p ) T(value);
	}
	
	// ���������������
	void deallocate(pointer p, size_type num)
	{
		_aligned_free(p);
	}
	// �������ς݂̗̈���폜����
	void destroy(pointer p)
	{
		p->~T();
	}
	
	// �A�h���X��Ԃ�
	pointer address(reference value) const { return &value; }
	const_pointer address(const_reference value) const { &value; }
	
	// �����Ă邱�Ƃ��ł���ő�̗v�f����Ԃ�
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