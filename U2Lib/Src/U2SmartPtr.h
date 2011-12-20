/************************************************************************
module	:	U2SmartPtr
Author	:	Yun	sangyong
Desc	:	Modern C++ Design 참조.소유궈은 Reference Counted, 
			암묵적 형변환 사용. 반드시 필요한 멤버함수만 정의
			T는 U2RefObj를 상속한 클래스여야 한다.
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
	//operator	void*();	// 경고 : if문에서 컨벤션 문제 발생...
	T&			operator*() const;
	//const T&	operator*() const;
	T*			operator->() const;
	//const T*	operator->() const;

	U2SmartPtr&		operator=(const U2SmartPtr<T>& rhs);
	U2SmartPtr&		operator=(T* pObj);
	
	// const가 붙지않으면 모호성 에러
	bool		operator==(const U2SmartPtr<T>& rhs) const;	
	bool		operator!=(const U2SmartPtr<T>& rhs) const;	
	bool		operator==(T* pObj) const;
	bool		operator!=(T* pObj) const;

	// 모호성 야기..
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

	T*	m_pObj;			// U2RefObj를 상속.

};


#define U2SmartPointer(classname) \
	class classname; \
	typedef U2SmartPtr<classname> classname##Ptr

#define SmartPtrCast(type, smartptr) ((type*) (void*) (smartptr))

#include "U2SmartPtr.inl"


#endif