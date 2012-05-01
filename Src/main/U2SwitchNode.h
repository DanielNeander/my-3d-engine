/**************************************************************************************************
module	:	U2SwitchNode
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once 
#ifndef U2_SWITCHNODE_H
#define U2_SWITCHNODE_H

#include "U2Node.h"

U2SmartPointer(U2SwitchNode);

class U2_3D U2SwitchNode : public U2Node 
{
	DECLARE_RTTI;

public:
	U2SwitchNode();
	virtual ~U2SwitchNode();

	enum { SN_INVALID_CHILD = -1 };

	void SetActiveChild(uint32 uActiveChild);
	uint32 GetActiveChild() const;
	void DisableAllChildren();

protected:
	virtual void GetVisibleSet(U2Culler& culler, bool bNoCull);

	uint32 m_uiActiveChild;

};

#endif

