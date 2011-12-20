/************************************************************************
module	:	U2TPointerList
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once 
#ifndef	U2_TPOINTERLIST_H
#define	U2_TPOINTERLIST_H

#include "U2TListBase.h"
#include "U2PoolAlloc.h"


template<class T>
class U2TPointerList : public U2TListBase<T, U2PoolAlloc<size_t> >
{
	
};

#endif