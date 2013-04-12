// Part of Smoke Framework implementation for procedural Fire system.

// Copyright ?2008-2009 Intel Corporation
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

#ifndef WATERBALL_H
#define WATERBALL_H

#include <list>
#include <vector>
#include "PSystem.h"
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

//
// Procedural fire system includes
//
#include "aabb.h"
#include "HeatParticle.h"

namespace ParticleEmitter
{
    ///////////////////////////////////////////////////////////////////////////////
    /// <summary>
    ///   A class that extends ParticleSystemWithEmitter to simulate cold emission (e.g. water)
    /// </summary>
    ///////////////////////////////////////////////////////////////////////////////
    class ColdParticle : public ParticleSystemWithEmitter
    {
    public:
        V3 mParticleRadius;
        /*V3 mCameraPos;*/
        f32 mInitialSize;
        V3 m_LastPos;
        HeatEmitter * m_pColdParticles;

        ColdParticle( const V3& accelbias, const V3& accelshift, const V3 Radius, const V3 pos,
            const aabb& box, int maxNumParticles, float timePerParticle, float minLifeTime = 1.0f,
            float maxLifeTime = 2.0f, float minSize = 10.0f, float maxSize = 15.0f, float minAmplitude = 1.0f,
            float maxAmplitude = 2.0f
        )
            : ParticleSystemWithEmitter(
                HeatEmitter::Type_Water, accelbias, accelshift, pos, box, 
                maxNumParticles, timePerParticle, minLifeTime, 
                maxLifeTime, minSize, maxSize, minAmplitude, maxAmplitude )
            , mParticleRadius(Radius)
            , m_LastPos(mInitPos)
            , m_pColdParticles(GetEmitter())
        {}

        ~ColdParticle() {}

        void setInitPos(V3 pos) { mInitPos = pos; }
        void setRadius(V3 radius) { mParticleRadius = radius; }
        void initParticle(Particle& out);
        virtual void reinitialize(){};
        virtual void update(float dt);
    };
} //end namespace
#endif // WATERBALL_H