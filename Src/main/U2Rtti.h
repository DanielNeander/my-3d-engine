/**************************************************************************************************
module	:	U2Rtti
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once 
#ifndef U2_RTTI_H
#define U2_RTTI_H

#include <U2_3D/Src/U2_3DLibType.h>

class U2_3D	U2Rtti
{
public:
	U2Rtti(const TCHAR* acName, const U2Rtti* pBaseType);
	~U2Rtti();

	const TCHAR* GetName() const;
	int		GetDiskUsed() const;

	bool	IsExactly(const U2Rtti& type) const;
	bool	IsDerived(const U2Rtti& type) const;

private:
	const TCHAR* m_acName;
	const U2Rtti*	m_pBaseType;

};

#include "U2Rtti.inl"

#endif