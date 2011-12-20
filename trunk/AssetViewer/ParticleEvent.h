#ifndef PARTICLEEVENT_H
#define PARTICLEEVNET_H

#include "Particle.h"
#include "ParticleEmitterTokenizer.h"
#include "MinMax.h"
#include <algorithm>
#include <functional>
#include <stdio.h>

class CParticleEvent : public U2MemObj
{
public:
	CParticleEvent() { };
	virtual ~CParticleEvent() {}
	
	virtual void RestoreDeviceObjects() {};
	virtual void InvalidateDeviceObjects() {}

	CMinMax<float> GetTimeRange(void) const { return(m_TimeRange); }
	void SetTimeRange(const CMinMax<float> data) { m_TimeRange = data; }

	float GetActualTime(void) const { return(m_ActualTime); }
	void SetActualTime(const float data) { m_ActualTime = data; }

	bool IsFade(void) const { return(m_bFade); }
	void SetFade(const bool data = true) { m_bFade = data; }

	virtual void DoItToIt(CParticle &part) = 0;
	virtual bool FadeAllowed() = 0;

	virtual bool ProcessTokenStream(std::vector<CParticleEmitterToken>::iterator &TokenIter, 
		std::vector<CParticleEmitterToken>::iterator &EndIter) = 0;

	static void ProcessPropEqualsValue(CMinMax<float> &prop,
		std::vector<CParticleEmitterToken>::iterator &TokenIter, 
		std::vector<CParticleEmitterToken>::iterator &EndIter);

	static void ProcessPropEqualsValue(CMinMax<D3DXVECTOR3> &prop,
		std::vector<CParticleEmitterToken>::iterator &TokenIter, 
		std::vector<CParticleEmitterToken>::iterator &EndIter);

	static void ProcessPropEqualsValue(CMinMax<D3DXCOLOR> &prop,
		std::vector<CParticleEmitterToken>::iterator &TokenIter, 
		std::vector<CParticleEmitterToken>::iterator &EndIter);

	CParticleEvent* m_pNextFadeEvent;
protected:
	CMinMax<float> m_TimeRange;
	float m_ActualTime;
	bool m_bFade;
};

	class CParticleEventCompareFunc : 
		public std::binary_function<CParticleEvent*, CParticleEvent *, bool> {
	public:
		bool operator() (const CParticleEvent* lhs, const CParticleEvent* rhs) const {
			return lhs->GetActualTime() < rhs->GetActualTime();
		}

	};

	class CParticleEventSize : public CParticleEvent
	{
	public:
		bool FadeAllowed() { return(true); }
		void DoItToIt(CParticle &pt);
		virtual bool ProcessTokenStream(std::vector<CParticleEmitterToken>::iterator &TokenIter, 
			std::vector<CParticleEmitterToken>::iterator &EndIter);

		CMinMax<float> GetSize(void) const { return(m_Size); }
		void SetSize(const CMinMax<float> data) { m_Size = data; }
		
	protected:
		CMinMax<float> m_Size;
	};

#endif