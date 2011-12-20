/************************************************************************
module	:	U2SkinModifier
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once 
#ifndef U2_SKINMODIFIER_H
#define U2_SKINMODIFIER_H

#include <U2_3D/Src/Animation/U2BonePalette.h>

class U2Skeleton;
class U2SkinController;

class U2_3D U2SkinModifier : public U2Object
{

public:
	U2SkinModifier();
	virtual ~U2SkinModifier();

	void BeginFragments(int num);
	/// set mesh group index of a skin fragment
	void SetFragGroupIndex(int fragIndex, int meshGroupIndex);
	/// get mesh group index of a skin fragment
	int GetFragGroupIndex(int fragIndex) const;
	/// begin defining the joint palette of a fragment
	void BeginJointPalette(int fragIndex, int numJoints);
	/// add up to 8 joint indices to the fragments joint palette
	void SetJointIndices(int fragIndex, int paletteIndex, 
		int ji0, int ji1, int ji2, int ji3, int ji4, int ji5, int ji6, int ji7);
	/// add a single joint index to the fragments joint palette
	void SetJointIdx(int fragIndex, int paletteIndex, int jointIndex);
	/// finish adding joints to the joint palette
	void EndJointPalette(int fragIndex);
	/// finish defining fragments
	void EndFragments();
	/// get number of fragments
	int GetNumFragments() const;
	/// get number of joints in a fragment's joint palette
	int GetJointPaletteSize(int fragIndex) const;
	/// get a joint index from a fragment's joint palette
	int GetJointIndex(int fragIndex, int paletteIndex) const;


	class Fragment : public U2MemObj
	{
	public:
		Fragment() : m_iMeshGroupIdx(-1) {}

		void SetMeshGroupIdx(int i) { m_iMeshGroupIdx = i; }

		int GetMeshGroupIdx() const { return m_iMeshGroupIdx; }

		U2BonePalette& GetBonePalette() { return m_bonePalette; }
	private:
		int m_iMeshGroupIdx;
		U2BonePalette m_bonePalette;
	};

	Fragment* GetFragment(uint32 idx) const;

	void SetSkeleton(const U2Skeleton* pSkel);
	const U2Skeleton* GetSkeleton() const;

	void SetSkinController(U2SkinController* pSkinCtrl);
	U2SkinController* GetSkinController() const;

	
private:
	

	U2SkinController* m_pSkinCtrl;
	const U2Skeleton* m_pExtSkel;
	U2PrimitiveVec<Fragment*> m_fragments;	
};

typedef U2SmartPtr<U2SkinModifier> U2SkinModifierPtr;

inline void U2SkinModifier::SetSkinController(U2SkinController* pSkinCtrl)
{
	m_pSkinCtrl = pSkinCtrl;
}

inline U2SkinController* U2SkinModifier::GetSkinController() const
{
	return m_pSkinCtrl;
}

inline void U2SkinModifier::SetSkeleton(const U2Skeleton* pSkel)
{
	m_pExtSkel = pSkel;
}

inline const U2Skeleton* U2SkinModifier::GetSkeleton() const
{
	return m_pExtSkel;
}

inline U2SkinModifier::Fragment* U2SkinModifier::GetFragment(uint32 idx) const
{
	return m_fragments[idx];
}


#endif 