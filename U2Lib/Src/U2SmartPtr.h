/************************************************************************
module	:	U2SmartPtr
Author	:	Yun	sangyong
Desc	:	Modern C++ Design ����.�������� Reference Counted, 
			�Ϲ��� ����ȯ ���. �ݵ�� �ʿ��� ����Լ��� ����
			T�� U2RefObj�� ����� Ŭ�������� �Ѵ�.
************************************************************************/
#pragma once
#ifndef U2_SMARTPTR_H
#define	U2_SMARTPTR_H

#include <U2Lib/Src/Memory/U2MemObj.h>
#include <U2Lib/Src/U2RefObject.h>


template<class T>
class  U2SmartPtr : public U2MemObj
{
public:
	U2SmartPtr();
	U2SmartPtr(T* pObj);
	U2SmartPtr(const U2SmartPtr<T>& rhs);
	~U2SmartPtr();

	operator	T*() const;	
	//operator	void*();	// ��� : if������ ������ ���� �߻�...
	T&			operator*() const;
	//const T&	operator*() const;
	T*			operator->() const;
	//const T*	operator->() const;

	U2SmartPtr&		operator=(const U2SmartPtr<T>& rhs);
	U2SmartPtr&		operator=(T* pObj);
	
	// const�� ���������� ��ȣ�� ����
	bool		operator==(const U2SmartPtr<T>& rhs) const;	
	bool		operator!=(const U2SmartPtr<T>& rhs) const;	
	bool		operator==(T* pObj) const;
	bool		operator!=(T* pObj) const;

	// ��ȣ�� �߱�..
	//template<class U>
	//friend bool operator==(const U2SmartPtr<T>& lhs, U* rhs);
	//
	//template<class U>
	//friend bool operator!=(const U2SmartPtr<T>& lhs, U* rhs);	
	//
	//template<class U>
	//friend bool operator==(U* rhs, const U2SmartPtr<T>& lhs);

	//template<class U>
	//friend bool operator!=(U* rhs, const U2SmartPtr<T>& lhs);

	T*	m_pObj;			// U2RefObj�� ���.

};


#define U2SmartPointer(classname) \
	class classname; \
	typedef U2SmartPtr<classname> classname##Ptr

#define SmartPtrCast(type, smartptr) ((type*) (void*) (smartptr))

#include "U2SmartPtr.inl"


#endif