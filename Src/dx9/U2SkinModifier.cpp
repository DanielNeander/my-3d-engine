#include <U2_3D/Src/U23DLibPCH.h>
#include "U2SkinModifier.h"


U2SkinModifier::U2SkinModifier()
:m_pSkinCtrl(0),
m_pExtSkel(0)
{

}

U2SkinModifier::~U2SkinModifier()
{
	
}

//------------------------------------------------------------------------------
/**
Begin defining mesh fragment. A skin mesh may be divided into several
fragments to account for gfx hardware which an only render a skinned
mesh with a limited number of influence objects (joints).
*/
void
U2SkinModifier::BeginFragments(int num)
{
	U2ASSERT(num > 0);
	this->m_fragments.Resize(num);

	for(int i=0; i < num; ++i)
	{
		Fragment *pNewFrag = U2_NEW Fragment;
		m_fragments.AddElem(pNewFrag);
	}
}

//------------------------------------------------------------------------------
/**
Set the mesh group index for a skin fragment.
*/
void
U2SkinModifier::SetFragGroupIndex(int fragIndex, int meshGroupIndex)
{
	this->m_fragments[fragIndex]->SetMeshGroupIdx(meshGroupIndex);
}

//------------------------------------------------------------------------------
/**
Get the mesh group index for a skin fragment.
*/
int
U2SkinModifier::GetFragGroupIndex(int fragIndex) const
{
	return this->m_fragments[fragIndex]->GetMeshGroupIdx();
}

//------------------------------------------------------------------------------
/**
Begin defining the joint palette of a skin fragment.
*/
void
U2SkinModifier::BeginJointPalette(int fragIndex, int numJoints)
{
	this->m_fragments[fragIndex]->GetBonePalette().BeginJoints(numJoints);
}

//------------------------------------------------------------------------------
/**
Add up to 8 joints to a fragments joint palette starting at a given
palette index.
*/
void
U2SkinModifier::SetJointIndices(int fragIndex, int paletteIndex, int ji0, int ji1, int ji2, int ji3, int ji4, int ji5, int ji6, int ji7)
{
	U2BonePalette& pal = this->m_fragments[fragIndex]->GetBonePalette();
	int numJoints = pal.GetNumJoints();
	if (paletteIndex < numJoints) pal.SetJointIdx(paletteIndex++, ji0);
	if (paletteIndex < numJoints) pal.SetJointIdx(paletteIndex++, ji1);
	if (paletteIndex < numJoints) pal.SetJointIdx(paletteIndex++, ji2);
	if (paletteIndex < numJoints) pal.SetJointIdx(paletteIndex++, ji3);
	if (paletteIndex < numJoints) pal.SetJointIdx(paletteIndex++, ji4);
	if (paletteIndex < numJoints) pal.SetJointIdx(paletteIndex++, ji5);
	if (paletteIndex < numJoints) pal.SetJointIdx(paletteIndex++, ji6);
	if (paletteIndex < numJoints) pal.SetJointIdx(paletteIndex++, ji7);
}

//------------------------------------------------------------------------------
/**
*/
void
U2SkinModifier::SetJointIdx(int fragIndex, int paletteIndex, int jointIndex)
{
	U2BonePalette& pal = this->m_fragments[fragIndex]->GetBonePalette();
	pal.SetJointIdx(paletteIndex, jointIndex);
}

//------------------------------------------------------------------------------
/**
Finish defining the joint palette of a skin fragment.
*/
void
U2SkinModifier::EndJointPalette(int fragIndex)
{
	this->m_fragments[fragIndex]->GetBonePalette().EndJoints();
}

//------------------------------------------------------------------------------
/**
Finish defining fragments.
*/
void
U2SkinModifier::EndFragments()
{
	// empty
}

//------------------------------------------------------------------------------
/**
Get number of fragments.
*/
int
U2SkinModifier::GetNumFragments() const
{
	return this->m_fragments.Size();
}

//------------------------------------------------------------------------------
/**
Get joint palette size of a skin fragment.
*/
int
U2SkinModifier::GetJointPaletteSize(int fragIndex) const
{
	return this->m_fragments[fragIndex]->GetBonePalette().GetNumJoints();
}

//------------------------------------------------------------------------------
/**
Get a joint index from a fragment's joint index.
*/
int
U2SkinModifier::GetJointIndex(int fragIndex, int paletteIndex) const
{
	return this->m_fragments[fragIndex]->GetBonePalette().GetJointIdx(paletteIndex);
}