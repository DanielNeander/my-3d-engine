#pragma once 
#ifndef U2_MAXLIGHTEPXORT_H
#define U2_MAXLIGHTEPXORT_H


#include "U2MaxLib.h"

class U2MaxLightExport : public U2MemObj
{
public:
	U2MaxLightExport(TimeValue animStart, TimeValue animEnd);

	void ExportLight(Interface* pIf, INode* pMaxNode, U2Node* pCurNode,  Object* pObj);	
	bool BuildLight(Interface* pIf, Object *obj, U2LightNode* pLightNode);

	TimeValue m_animStart, m_animEnd;


};

#endif 