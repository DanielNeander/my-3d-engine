/**************************************************************************************************
module	:	U2MaxNoteTrack
Author	:	Yun sangyong
Desc	:	GetAnimState() ¹Ì±¸Çö..
*************************************************************************************************/
#pragma once 
#ifndef U2_MAXNOTETRACK_H
#define U2_MAXNOTETRACK_H

#include "U2MaxLib.h"
//#include <U2Lib/Src/U2Tvec.h>
#include <U2_3D/src/Tools/U2AnimBuilder.h>

//---------------------------------------------------------------------------
/**
@class U2MaxAnimState 
@ingroup NebulaMaxExport2Contrib

@brief A class which represents animation states.

*/
class U2MaxAnimState : public U2MemObj
{
public:
	U2MaxAnimState();
	~U2MaxAnimState();


	void AddClip(const TCHAR* szWeightChannl);

	const U2DynString& GetClip(int idx) const;

	int m_iFirstFrame;

	int m_iDuration;

	float m_fFadeInTime;

	int m_iLoopType;

	U2DynString m_szName;

	U2PrimitiveVec<U2DynString*> m_clips;
};


//---------------------------------------------------------------------------
/**
@class U2MaxNoteTrack
@ingroup NebulaMaxExport2Contrib

@brief Collect animation states and clips in note track of a max node.
The collected animation states and clips are used when we build
skin animator.

*/
class U2MaxNoteTrack : public U2MemObj
{
public:

	U2MaxNoteTrack(int numFrame);
	virtual ~U2MaxNoteTrack();

	void GetAnimState(INode* inode);

	/// Get the number of animation states which collected.
	int GetNumStates() const;

	U2MaxAnimState& AddAnimState(int firstframe, int duration, 
		float fadeintime, bool loop = true);

	/// Get animation state from the given index.
	const U2MaxAnimState& GetState(int index);

protected:
	///
	int GetNextFrame(int index, DefNoteTrack* n);

protected:
	/// array for animation states which collected from note track.
	U2PrimitiveVec<U2MaxAnimState*> m_states;

	int m_iNumFrames;
};

//-------------------------------------------------------------------------------------------------
inline 
U2MaxAnimState::U2MaxAnimState()
:m_iFirstFrame(0),
m_iDuration(0),
m_fFadeInTime(0.0f),
m_iLoopType(U2AnimBuilder::Group::REPEAT)
{
	AddClip(TSTR("one"));
}

//-------------------------------------------------------------------------------------------------
inline 
void U2MaxAnimState::AddClip(const TCHAR* szWeightChannl)
{
	m_clips.AddElem(U2_NEW U2DynString(szWeightChannl));
}

//-------------------------------------------------------------------------------------------------
inline 
const U2DynString& U2MaxAnimState::GetClip(int idx) const
{
	return *m_clips[idx];
}


//-------------------------------------------------------------------------------------------------
inline U2MaxAnimState::~U2MaxAnimState()
{

}


//---------------------------------------------------------------------------
inline
int U2MaxNoteTrack::GetNumStates() const
{
	return this->m_states.Size();
}
//---------------------------------------------------------------------------
inline
int U2MaxNoteTrack::GetNextFrame(int index, DefNoteTrack* n)
{
	return (index < n->keys.Count() - 1 ? 
		n->keys[index+1]->time/GetTicksPerFrame() : m_iNumFrames);
	
}
//---------------------------------------------------------------------------
inline
const U2MaxAnimState& 
U2MaxNoteTrack::GetState(int index)
{
	U2ASSERT(index >= 0);
	return *m_states[index];
}
//---------------------------------------------------------------------------
inline
U2MaxAnimState&
U2MaxNoteTrack::AddAnimState(int firstframe, int duration, float fadeintime, bool loop)
{
	U2MaxAnimState* animState = U2_NEW U2MaxAnimState;
	animState->m_iFirstFrame	= firstframe;
	animState->m_iDuration		= duration;
	animState->m_fFadeInTime	= fadeintime;

	m_states.AddElem(animState);
	return *animState;
}

#endif 
