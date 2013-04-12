//Writen by Hugh Smith summer and fall 2007
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

#include "stdafx.h"

#include <list>
#include <vector>
#include "PSystem.h"
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"
#include "Fire.h"

namespace ParticleEmitter{
void Fire::reinitialize()
{
	for(int i = 0; i < mMaxNumParticles; ++i)
	{
 		mParticles[i].age = 0.0f;
 		mParticles[i].lifeTime = 0.0f;
        mParticles[i].initialTime = 0.0f;
	    mParticles[i].initialPos = V3(0.0f,0.0f,0.0f);
	    mParticles[i].initialVelocity = V3(0.0f,0.0f,0.0f);
	    mParticles[i].initialSize = 0.0f; // In pixels.
	}

}
void Fire::initParticle(ParticleEmitter::Particle& out)
{

	// Time particle is created relative to the global running
	// time of the particle system.
	out.initialTime = mTime;
    out.age = mTime;

	// Flare lives for 2-4 seconds.
	//known good
	//out.lifeTime   = GetRandomFloat(2.0f, 8.0f);
	out.lifeTime   = GetRandomFloat(mMinLifeTime, mMaxLifeTime);

	// Initial size i1.0n pixels.
	out.initialSize  = GetRandomFloat(mMinSize, mMaxSize );

	// Give a very small initial velocity to give the flares
	// some randomness.
	GetRandomVec(out.initialVelocity);
    out.initialVelocity.x *= mAccelImpulse.x;
    out.initialVelocity.y *= mAccelImpulse.y;
    out.initialVelocity.z *= mAccelImpulse.z;
    out.initialVelocity.x += mAccelShift.x;
    out.initialVelocity.y += mAccelShift.y;
    out.initialVelocity.z += mAccelShift.z;

	// Scalar value used in vertex shader as an amplitude factor.
	out.mass = GetRandomFloat(mMinAmplitude, mMaxAmplitude);

	// Start color at 50-100% intensity when born for variation.
	out.initialColor = static_cast<DWORD>(GetRandomFloat(0.5f, 1.0f) *    1.0f);//white;

	V3 temp = mParticleLine - mInitPos;
	float scale = GetRandomFloat(0.0f, 1.0f);
	out.initialPos = mInitPos + (temp * scale);
}

}