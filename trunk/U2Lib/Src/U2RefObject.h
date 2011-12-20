/************************************************************************
module : U2RefObject
Author : yun sangyong 
Desc   :
************************************************************************/

#ifndef	U2_REFOBJECT_H
#define	U2_REFOBJECT_H

#include "U2DataType.h"
#include "U2InterlockedOp.h"
#include <U2Lib/src/Memory/U2MemObj.h>


class U2LIB U2RefObject : public U2MemObj
{
public:
	U2RefObject();
	virtual ~U2RefObject();

	inline void	IncRefCount();
	inline void	DecRefCount();
	inline unsigned int GetRefCount() const;
	static unsigned int GetTotalObjCount();

	// 연산자 정의 안함..

protected:
	virtual void Delete();

private:
	uint32 m_uRefCnt;
	static uint32 ms_uObjs;	

};


inline 
U2RefObject::U2RefObject()
{
	m_uRefCnt = 0;
	U2AtomicInc(ms_uObjs);
}

inline
U2RefObject::~U2RefObject()
{
	U2AtomicDec(ms_uObjs);
}


inline 
void U2RefObject::IncRefCount()
{
	U2AtomicInc(m_uRefCnt);
}


inline 
void U2RefObject::DecRefCount()
{
	if(U2AtomicDec(m_uRefCnt) == 0)
		Delete();
}


inline
unsigned int U2RefObject::GetRefCount() const 
{
	return m_uRefCnt;
}

inline
unsigned int U2RefObject::GetTotalObjCount()
{
	return ms_uObjs;
}

#endif










