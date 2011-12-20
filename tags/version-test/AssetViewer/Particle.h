#pragma once 
#ifndef PARTICLE_H
#define PARTILCE_H

#include <U2Lib/Src/Memory/U2MemObj.h>
#include <vector>

struct VERTEX_PARTICLE
{
	D3DXVECTOR3 position;
	float		pointsize;
	D3DCOLOR	color;
} ;

class CParticleEvent;



class CParticle : public U2MemObj
{
public:
	CParticle() {

	}

	virtual ~CParticle() {}


	CParticleEvent*		m_pCurEvent;
	float m_fWeight;
	float m_fSize;
	float m_fSizeStep;

	float m_fLifeTime;
	float m_fAge;

	float m_fEventTimer;	// different from age

	D3DXCOLOR m_Color;
	D3DXCOLOR m_ColorStep;

	D3DXVECTOR3 m_vPos;

	D3DXVECTOR3 m_vDir;
	D3DXVECTOR3 m_vDirStep;

	// this function is inline	not because it's small, but because it's only called
	// in one place (CParticleEmitter.Update()).  This way we can also dodge the
	// performance hit associated with calling a function many times (remember, we're
	// in a for loop in CParticleEmitter.Update()).
	inline bool Update(float fDeltaTime)
	{
		// age the particle
		m_fAge += fDeltaTime;
		m_fEventTimer += fDeltaTime;


	}
};


#endif