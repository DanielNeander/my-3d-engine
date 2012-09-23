#ifndef NO_SKELETON_H
#define NO_SKELETON_H

#include <Animation/Rig/noaBone.h>

class noLocalFrame;
class noQTransform;

class noaSkeleton 
{
public:
	inline noaSkeleton();

	noLocalFrame* getLocalFrameForBone( int boneIndex ) const;


public:
	const char* m_name;
	hkInt16* m_parentIndices;
	hkInt32 m_numParentIndices;
	noaBone** m_bones;
	hkInt32 m_numBones;
	noQTransform* m_refPose;
	hkInt32	m_numRefPose;
	char** m_floatSlots;
	hkInt32 m_numFloatSlots;

	struct LocalFrameOnBone 
	{
		noLocalFrame* m_localFrame;
		int m_boneIndex;
	};

	struct LocalFrameOnBone* m_localFrames;
	hkInt32 m_numLocalFrames;

public:

};

__forceinline  noLocalFrame* noaSkeleton::getLocalFrameForBone( int boneIndex ) const
{
	for( int i = 0; i < m_numLocalFrames; i++ )
	{
		if ( m_localFrames[i].m_boneIndex == boneIndex )
		{
			return m_localFrames[i].m_localFrame;
		}
		else if ( m_localFrames[i].m_boneIndex > boneIndex )
		{
			break;
		}
	}

	return NULL;

}


#endif