#ifndef NO_INTERLEAVED_ANIMATION_H
#define NO_INTERLEAVED_ANIMATION_H

#include <Animation/noaAnimation.h>

class noaInterleavedAnim : public noaAnimation
{
public:

	void SampleTracks(hkReal time, noQTransform* transformTrackOut, hkReal* floatTracks);


};

#endif