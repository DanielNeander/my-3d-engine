/************************************************************************
module	:	U2TVec
Author	:	Yun sangyong
Desc	:	index üũ�ÿ��� FilledSize ��� 
			NonzeroElemSize�� 0�� ������ �迭 ����� ���̴�.
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

	// �迭�� �������� ������ ������ ����..
	void	Refresh();

	
	unsigned int	Remove(const T& elem);
	unsigned int	Find(const T& elem, unsigned int startIdx = 0);

	// ������ �Ŀ� �迭�� ���ۺ��� ó�� ����ִ� ���� ������Ʈ
	void	UpdateFirstEmpty();

	const	T&	operator[](unsigned int index) const;
	T&		operator[](unsigned int idx);

protected:
	
	T*	m_pBase;				// �迭�� ���� ������
	unsigned int m_uiSize;			// �迭�� �ִ� ũ��(number)
	unsigned int m_uiFirstAvailEmpty; 
	unsigned int m_uiFilledSize;		// �迭�� ä�� �������� ��
	unsigned int m_uiIncreSize;			// �迭�� �� á�� �� �迭�� ũ�⸦ ������ ũ��.

private:
	U2TVec(const U2TVec&);
	U2TVec& operator=(const U2TVec&);

};

template<class T>
class U2ObjVec : public U2TVec<T, U2NewAlloc<T> >
{
public:
	U2ObjVec(unsigned int maxSize = 0, unsigned int increSize = 1);	// �⺻ �����ڸ� ȣ���ϴ� ���� ����.
};


template<class T> 
class U2PrimitiveVec : public U2TVec<T, U2MallocAlloc<T> >
{
public:
	U2PrimitiveVec(unsigned int maxSize = 0, unsigned int increSize = 1);
};

#include "U2TVec.inl"

#endif