
/**************************************************************************************************
module	:	U2AnimUtil
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_ANIMUTIL_H
#define U2_ANIMUTIL_H

#include <U2lib/src/memory/U2MemObj.h>
#include <U2lib/src/U2TString.h>

class U2AnimLoopType : public U2MemObj
{
public:
	// loop types
	enum Type
	{
		LOOP,
		CLAMP,
	};

	// convert to string
	static U2DynString ToString(U2AnimLoopType::Type t);
	// convert from string
	static U2AnimLoopType::Type FromString(const U2DynString& s);
};

#include "U2AnimUtil.inl"

#endif