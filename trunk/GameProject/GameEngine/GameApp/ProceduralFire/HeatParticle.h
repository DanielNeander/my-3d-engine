//=============================================================================
// PSystem.h by Frank Luna (C) 2004 All Rights Reserved.
//=============================================================================

// This file was originally inspired by Frank Luna from his book 
// Introduction to Game Programming with DX9 a shader approach.  
// It is likely to be unrecognizable from Franks original work but the
// intent is to give credit for the basic inspiration for the particle system structure 
// that I developed.  I would encourage you to get a copy of his book as it might very
// well inspire you as it did me.


//Writen by Hugh Smith summer and fall 2007
// Part of Smoke Framework implementation for procedural Fire system.

// Copyright © 2008-2009 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

#ifndef HEAT_H
#define HEAT_H
#include <list>
#include <vector>
#include "PSystem.h"
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"
#include "aabb.h"


namespace ParticleEmitter{

struct HeatParticle : public ParticleBase
{
	V3 currentPos;
    V3 prevPos;

    void update ( float dt )
    {
        prevPos = currentPos;
        currentPos = currentPos + initialVelocity * dt;
    }
}; // struct HeatParticle

///////////////////////////////////////////////////////////////////////////////
/// <summary>
///   A class that extends ParticleSystemBase to represent simulate heat emission
/// </summary>
///////////////////////////////////////////////////////////////////////////////
class HeatEmitter : public ParticleSystemBase<HeatParticle>
{
public:
    enum Emitter_Type
    {
        Type_Fire,
        Type_Water
    };

    Emitter_Type m_EmitterType;
	V3 mInitPos;

    HeatEmitter( Emitter_Type eType )
        : m_EmitterType( eType )
    {
        setInactive();
    };
	
    void Activate( 
		const V3 pos,
		const V3 impulse,
        const V3 shift,
		int maxNumParticles,
		float timePerParticle,
        float minLifeTime = 1.0f,
        float maxLifeTime = 50.0f,
        float minSize = 1.0f,
        float maxSize = 1.0f,
        float minAmplitude = 1.0f,
        float maxAmplitude = 2.0f);

    void setPos(const V3& pos)
    {
        mInitPos = pos;
    }
    Bool getPositions( std::vector<V3>  &curPos, 
                       std::vector<V3>  &prevPos) const;

	virtual void update(float dt);
    virtual void initParticle(HeatParticle& out);

	void addParticle();

    // Returns an unnormalized random Vector.
    void GetRandomVec(V3& out)
    {
	    out.x = GetRandomFloat(-1.0f, 1.0f);
	    out.y = GetRandomFloat(-1.0f, 1.0f);
	    out.z = GetRandomFloat(-1.0f, 1.0f);
    }
};


///////////////////////////////////////////////////////////////////////////////
/// <summary>
///   A class that extends ParticleSystem to emitter "smart" particles
/// </summary>
///////////////////////////////////////////////////////////////////////////////
class ParticleSystemWithEmitter : public ParticleSystem
{
public:
	V3 mInitPos;
    HeatEmitter *m_pHeatParticles;

	ParticleSystemWithEmitter(
        HeatEmitter::Emitter_Type type,
		const V3& accelbias, 
		const V3& accelshift, 
		const V3 pos,
        const aabb& box,
		int maxNumParticles,
		float timePerParticle,
        float minLifeTime = 1.0f,
        float maxLifeTime = 2.0f,
        float minSize = 10.0f,
        float maxSize = 15.0f,
        float minAmplitude = 1.0f,
        float maxAmplitude = 2.0f
        )
		: ParticleSystem(accelbias, accelshift, box, 
			maxNumParticles, timePerParticle,
            minLifeTime, maxLifeTime, 
            minSize, maxSize,
            minAmplitude, maxAmplitude)
		, mInitPos(pos)
        , m_pHeatParticles(NULL)
	{
        m_pHeatParticles = new HeatEmitter(type);
    }

    ~ParticleSystemWithEmitter ()
    {
        delete m_pHeatParticles;
    }

    HeatEmitter* GetEmitter()
    {
        return m_pHeatParticles;
    };

    const std::vector<HeatParticle*>& getAliveParticles()
    {
        return m_pHeatParticles->getAliveParticles();
    }
    u32 getAliveParticleCount() const
    {
        return m_pHeatParticles->getAliveParticleCount();
    }
};
} //end namespace
#endif // HEAT
