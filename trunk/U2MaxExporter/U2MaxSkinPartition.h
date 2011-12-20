/**************************************************************************************************
module	:	U2MaxSkinPartition
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_MAXSKINPARTITION_H
#define U2_MAXSKINPARTITION_H

#include "U2MaxLib.h"

class U2SkinModifier;
class U2MaxMeshExport;
class U2MeshBuilder;

class U2MaxMeshFragement : public U2MemObj
{
public:
	struct Fragement : public U2MemObj
	{
		/// group map index in meshbuilder.
		int groupMapIdx;

		/// joint array of a joint palette.
		U2PrimitiveVec<int> bonePalettes;
	};

	U2SkinModifier* m_pSkinMod;	

	U2PrimitiveVec<Fragement*> m_fragments;
};


class U2MaxSkinPartition 
{
public:
	U2MaxSkinPartition();
	~U2MaxSkinPartition();

	void Execute(U2PrimitiveVec<U2MaxMeshExport*>& meshArray, U2MeshBuilder& meshBuilder,
		U2PrimitiveVec<U2SkinModifier*>& skinMods);

	void BuildMeshFragments(U2PrimitiveVec<U2MaxMeshFragement*>& meshFragments, 
		U2PrimitiveVec<U2SkinModifier*>& skinMods);
};


#endif
