#ifndef NOANIMATED_REFERENCE_FRAME_H
#define NOANIMATED_REFERENCE_FRAME_H

#include <Common/Base/hkBase.h>
//#include <Animation/Motion/NOAnimatedReferenceFrame.h>

class noaAnimatedRefFrame : 
{
public:
	virtual void GetRefFrame(hkReal time, hkQsTransform& motionOut) const = 0;

	virtual void getDeltaRefFrame( hkReal time, hkReal nextTime, int loops, hkQsTransform& deltaMotionOut) const = 0;

	virtual hkReal GetDuration() const = 0;

public:
	HK_FORCE_INLINE noaAnimatedRefFrame( ) {}

};

#endif