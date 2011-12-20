/**************************************************************************************************
module	:	U2MaxUtil
Author	:	Yun sangyong
Desc	:	
*************************************************************************************************/
#pragma once
#ifndef U2_MAXUtil_H
#define U2_MAXUtil_H

#include "U2MaxLib.h"

#define ND_CLEAR 0
#define ND_SET 1

class U2MaxUtil 
{
public:
	static Object* GetBaseObject(INode* pINode, TimeValue time);
	
	static Modifier* FindMorpherModifier(Object* pObj);
	static Modifier* FindModifier(Object* pObj, Class_ID modId);

	static SClass_ID GetSuperClassID(Object* pObj);
	static bool IsMorphObject(INode* pINode);
	static bool IsGeomObject(INode *node);
	static bool IsBone(INode* pMaxNode);
	static bool IsDummy(INode* pMaxNode);
	static bool IsFootStep(INode* inode);


	//static bool IsParticle(INode* pINode);
	
	static Modifier* FindPhysique(INode* inode);
	static Modifier* FindSkin(INode* pINode);
	static int GetMaterialCnt(INode* pINode);

	static void SetFlags(ReferenceMaker* pRM, int iStat);

};

#endif