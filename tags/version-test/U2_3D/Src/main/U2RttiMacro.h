/************************************************************************
module	:	U2U2RttiMacro
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_U2RttiMACRO_H
#define	U2_U2RttiMACRO_H

//----------------------------------------------------------------------------
#define	DECLARE_RTTI \
public: \
	static const U2Rtti ms_rtti; \
	virtual const U2Rtti& GetType () const { return ms_rtti; }
//----------------------------------------------------------------------------
#define	DECLARE_TEMPLATE_RTTI \
public: \
	U2_3D static const U2Rtti ms_rtti; \
	virtual const U2Rtti& GetType () const { return ms_rtti; }
//----------------------------------------------------------------------------
#define	IMPLEMENT_RTTI(classname,baseclassname) \
	const U2Rtti classname::ms_rtti(_T(#classname),&baseclassname::ms_rtti)
//----------------------------------------------------------------------------
#define IMPLEMENT_TEMPLATE_RTTI(classname,baseclassname) \
	template <> \
	const U2Rtti classname::ms_rtti(_T(#classname), &baseclassname::ms_rtti)
//----------------------------------------------------------------------------



#endif 