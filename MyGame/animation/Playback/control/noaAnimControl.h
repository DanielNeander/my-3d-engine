#ifndef NOANIMCONTROL_H
#define NOANIMCONTROL_H

#include <Common/Base/hkBase.h>

class noaAnimBinding;

class noaAnimControl 
{
public:

	noaAnimControl( const noaAnimBinding* binding);
	virtual ~noaAnimControl();

	virtual void Update( hkReal stepDelta ) = 0;

	inline void SetLocalTime( hkReal time ) { m_localTime = time; }
	inline hkReal GetWeight() const { return m_weight; }

protected:
	hkReal m_localTime;
	hkReal m_weight;
	hkArray<hkUint8> m_transformTrackWeights;
	hkArray<hkUint8> m_floatTrackWeights;

	const noaAnimBinding* m_binding;

	hkReal m_motionTrackWeight;
};


#endif