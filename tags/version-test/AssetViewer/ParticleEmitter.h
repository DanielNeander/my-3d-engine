#ifndef PARTICLE_EMITTER_H
#define PARTICLE_EMITTER_H

#include "MinMax.h"
#include "ParticleEmitterTokenizer.h"
#include "ParticleEventSequence.h"

class CParticleEmitter : public U2MemObj
{
public:
	std::string m_szLastError;

	CParticleEmitter();
	virtual ~CParticleEmitter();
	virtual void Update(float fElpasedTime, float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT RestoreDeviceObjects();
	virtual void	InvalidateDeviceObjects();

	virtual bool Compile(const char* szScript);
	virtual void Init();

	virtual void Start() {};
	virtual void Pause() {};
	virtual void Stop() {};
	virtual bool IsRunning() const { };

	inline void DeleteAllParticles(void) {

	}

	CMinMax<D3DXVECTOR3>	GetPosRange(void) const { };	
private:
	bool ProcessParticleSystemBlock(
		std::vector<CParticleEmitterToken>::iterator &TokenIter,
		std::vector<CParticleEmitterToken>::iterator &EndIter);

	bool ProcessEventSequenceBlock(CParticleEventSequence &seq,
		std::vector<CParticleEmitterToken>::iterator &TokenIter,
		std::vector<CParticleEmitterToken>::iterator &EndIter);
private:

	CMinMax<D3DXVECTOR3>	m_vPosRange;
	D3DXVECTOR3 m_vPos;

	U2String m_szName;

	U2PrimitiveVec<CParticleEventSequence* > m_vecSeq;

	int m_iMaxParticles;
	bool m_bIsRunning;

	int m_iVBSize;	

};


#endif