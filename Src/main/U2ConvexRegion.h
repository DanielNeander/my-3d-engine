/**************************************************************************************************
module	:	U2ConvexRegion
Author	:	Yun sangyong
Desc	:
**************************************************************************************************/
#pragma once
#ifndef	U2_CONVEXREGION_H
#define	U2_CONVEXREGION_H

#include "U2Camera.h"
#include "U2Node.h"

U2SmartPointer(U2Portal);

class U2_3D U2ConvexRegion : public U2Node 
{
	DECLARE_RTTI;
public:
	U2ConvexRegion(uint32 uPortalCnt, U2PrimitiveVec<U2PortalPtr>* aspPortal);
	virtual ~U2ConvexRegion();

	uint32 GetProtalCnt() const;
	U2Portal* GetPortal(int i) const;

	virtual void GetVisibleSet(U2Culler& culler, bool bNoCull);

protected:
	U2ConvexRegion();

	virtual void UpdateWorldData(float fAppTime);

	uint32 m_uPortalCnt;

	U2PrimitiveVec<U2PortalPtr>* m_aspPortal;


	bool m_bVisited;
};

#include "U2ConvexRegion.inl"

#endif




