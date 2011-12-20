/************************************************************************
module	:	U2TVec
Author	:	Yun sangyong
Desc	:	index 체크시에는 FilledSize 사용 
			NonzeroElemSize는 0을 제외한 배열 요소의 수이다.
************************************************************************/
#pragma once
#ifndef U2_TVEC_H
#define U2_TVEC_H

#include <U2Lib/src/U2ContainerAlloc.h>
#include <U2Lib/src/Memory/U2MemObj.h>

template<class T, class TAlloc>
class  U2TVec : public U2MemObj
{

public:
	U2TVec(unsigned int maxSize = 0, unsigned int incrSize = 1);
	virtual ~U2TVec();

	unsigned int Size() const;	
	unsigned int FilledSize() const;
	unsigned int NonzeroElemSize() const;



	void	Resize(unsigned int reSize);
	void	SetIncreSize(unsigned int increSize);

	T*		GetBase();
	const	T&	GetElem(unsigned int index) const;
	T&	GetElem(unsigned int index);
	void	SetElem(unsigned int idx, const T& elem);
	unsigned int	SetSafeElem(unsigned int idx, const T& elem);
	
	unsigned int	AddElem(const T& elem);		
	unsigned int	AddEmptySlot(const T& elem);
	T		Remove(unsigned int index);
	T		RemoveAndFill(unsigned int uiIdx);
	T		RemoveEnd();
	void	RemoveAll();

	// 배열을 연속적인 공간에 놓도록 정렬..
	void	Refresh();

	
	unsigned int	Remove(const T& elem);
	unsigned int	Find(const T& elem, unsigned int startIdx = 0);

	// 삭제한 후에 배열의 시작부터 처음 비워있는 슬롯 업데이트
	void	UpdateFirstEmpty();

	const	T&	operator[](unsigned int index) const;
	T&		operator[](unsigned int idx);

protected:
	
	T*	m_pBase;				// 배열의 시작 포인터
	unsigned int m_uiSize;			// 배열의 최대 크기(number)
	unsigned int m_uiFirstAvailEmpty; 
	unsigned int m_uiFilledSize;		// 배열을 채운 데이터의 수
	unsigned int m_uiIncreSize;			// 배열이 다 찼을 때 배열의 크기를 증가할 크기.

private:
	U2TVec(const U2TVec&);
	U2TVec& operator=(const U2TVec&);

};

template<class T>
class U2ObjVec : public U2TVec<T, U2NewAlloc<T> >
{
public:
	U2ObjVec(unsigned int maxSize = 0, unsigned int increSize = 1);	// 기본 생성자를 호출하는 것을 방지.
};


template<class T> 
class U2PrimitiveVec : public U2TVec<T, U2MallocAlloc<T> >
{
public:
	U2PrimitiveVec(unsigned int maxSize = 0, unsigned int increSize = 1);
};

#include "U2TVec.inl"

#endif