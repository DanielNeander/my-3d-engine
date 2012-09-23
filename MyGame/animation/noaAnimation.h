#ifndef NO_ANIMATION_H
#define NO_ANIMATION_H

#include <Math/Math.h>
#include <Common/Base/hkBase.h>
#include <Math/Vector.h>
#include <Math/QTransform.h>
#include <Math/Quaternion.h>



struct FRawAnimSequenceTrack
{
	hkArray<noVec3> PosKeys;
	hkArray<noQuat> RotKeys;
	hkArray<FLOAT> KeyTimes;

	/** Constructors */
	FRawAnimSequenceTrack() {}	
};

struct FTranslationTrack
{
	hkArray<noVec3> PosKeys;
	hkArray<FLOAT> Times;

	/** Constructors */
	FTranslationTrack() {}	
};

struct FRotationTrack
{
	hkArray<noQuat> RotKeys;
	hkArray<FLOAT> Times;

	/** Constructors */
	FRotationTrack() {}	
};



class noaAnimation 
{
	virtual void SampleTracks(hkReal time, noQTransform* transformTrackOut, hkReal* floatTracks) = 0;
	

public:

	hkReal m_duration;
	int m_numFrames;
	int m_numOfTransformTracks;
	int m_numOfFloatTracks;

	hkString m_seqName;

	hkArray<struct FRawAnimSequenceTrack> RawAnimData;
	hkArray<struct FTranslationTrack> TranslationData;
	hkArray<struct FRotationTrack> RotationData;
};


#endif