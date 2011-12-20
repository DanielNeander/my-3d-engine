#include "stdafx.h"
#include "ParticleEventSequence.h"


void CParticleEventSequence::RunEvents(CParticle& pt)
{
	CParticleEvent* pCurEvent = pt.m_pCurEvent;
	for (UINT32 ui = 0; ui < m_aEvents.FilledSize() && pCurEvent->GetActualTime() <= pt.m_fEventTimer
		; ui++)
	{
		float fOldEventTimer = pt.m_fEventTimer;
		pCurEvent->DoItToIt(pt);
		if (pt.m_fEventTimer != fOldEventTimer) {
			// event timer has changed, we need to recalc m_CurEvent.
			CParticleEvent* pRecalcPEvent = m_aEvents.GetElem(0);
			for (UINT32 recalcIndex = 0; 
				recalcIndex < m_aEvents.FilledSize() && pRecalcPEvent->GetActualTime() < pt.m_fEventTimer; recalcIndex++)			
				// set our main iterator to the recalculated iterator
				// the -1 just compensates for the i++ in the main for loop
			pCurEvent = m_aEvents.GetElem(--recalcIndex);		
		}		
	}
	pt.m_pCurEvent = pCurEvent;
}

void CParticleEventSequence::Update(float fElapsedTime, float fDeltaTime, D3DXVECTOR3 m_vPtPos)
{
	if (!m_pParticles) return;

	// update existing particles
	{
		for (int q=0; q < m_pParticles->GetTotalElements(); q++) {
			if (m_pParticles->IsAlive(q)) {
				CParticle &pt = m_pParticles->GetAt(q);

				if (!pt.Update(fDeltaTime)) {
					m_pParticles->Delete(&pt);
				}
				else {
					pt.m_vDir += fDeltaTime * m_vGravity.GetRandomNumInRange();
					// run all the particle's events
					RunEvents(pt);
				}
			} // is alive
		} // next particle
	}

	float fEmitRateThisFrame = m_EmitRate.GetRandomNumInRange();
	int iNumNewPts = fEmitRateThisFrame * fDeltaTime;
	m_fNumNewPtsExces += (float)(fEmitRateThisFrame * fDeltaTime) - iNumNewPts;

	if (m_fNumNewPtsExces > 1.0f) {
		iNumNewPts += (int)m_fNumNewPtsExces;
		m_fNumNewPtsExces -= (int)m_fNumNewPtsExces;
	}

	if (m_Loops > 0 && m_iTotalParticleLives * iNumNewPts > m_Loops * m_iNumParticles) {
		iNumNewPts = (m_Loops * m_iNumParticles) - m_iTotalParticleLives;
		if (iNumNewPts <= 0) {
			iNumNewPts = 0;
		}
	}

	for (int q=0; q < iNumNewPts && m_pParticles->GetNumFreeElements(); q++) {
		try {
			CreateNewParticle(m_vPtPos);
		} catch(...) { q = iNumNewPts; } 
	}
}
