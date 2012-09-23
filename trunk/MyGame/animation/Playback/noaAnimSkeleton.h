#ifndef NO_PLAYBACK_ANIMSKELETON_H
#define NO_PLAYBACK_ANIMSKELETON_H

#include <Animation/noaAnimation.h>

class noaSkeleton;
class noaChunkCache;
class noaAnimControl;
class noaPose;

class NOAnimatedSkeleton 
{
public:
	NOAnimatedSkeleton( const noaSkeleton* skeleton );
	~NOAnimatedSkeleton();

	virtual void StepDeltaTime( hkReal time );

	void AddAnimControls( noaAnimControl* anim );
	void RemoveAnimControl( noaAnimControl* anim );
	inline int getNumAnimControls() const { return m_animControls.getSize(); }
	inline noaAnimControl* GetAnimControl( int i ) const 
	{
		return m_animControls[i];
	}

protected:
	hkArray<noaAnimControl*>	m_animControls;
	const noaSkeleton* m_skeleton;
	hkReal m_refPoseWeightThreshold;
};

#endif