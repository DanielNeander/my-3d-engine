#ifndef PARTICLE_EVENTSEQUENCE_H
#define PARTICLE_EVENTSEQUENCE_H
#include "particle.h"
#include "dynamicarraypool.h"
#include "particleevent.h"

class CParticleEventSequence : public U2MemObj
{
public:
	HRESULT RestoreDevieceObjects();
	HRESULT InvalidateDeviceObjects();

	void Reset(void);
	void Update(float fElapsedTime, float fDeltaTime, D3DXVECTOR3 m_vPtPos);
	void Render();

	void CreateNewParticle(D3DXVECTOR3 m_vPtPos);
	void CreateFadeLists();

	void SortEvents(void);
	void NailDownRandomTimes(void);

	CParticleEventSequence();
	virtual ~CParticleEventSequence();


	void RunEvents(CParticle& pt);
	U2PrimitiveVec<CParticleEvent* > m_aEvents;	

private:
	CDynamicArrayPool<CParticle> *m_pParticles;

	CMinMax<float> m_LifeTime;
	CMinMax<float> m_EmitRate;
	int m_Loops;
	CMinMax<D3DXVECTOR3> m_vSpawnDir;
	CMinMax<D3DXVECTOR3> m_vEmitRadius;
	CMinMax<D3DXVECTOR3> m_vGravity;

	std::string m_szTexFilename;
	int m_iNumParticles;
	int m_iDestBlendMode;
	int m_iSrcBlendMode;

	long int m_iTotalParticleLives;

	std::string m_szName;

	float m_fNumNewPtsExces;
};

#endif