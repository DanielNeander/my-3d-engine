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
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

//
// Procedural fire system includes
//
#include "PSystem.h"

#include "ColdParticle.h"


namespace ParticleEmitter
{
    void ColdParticle::initParticle(ParticleEmitter::Particle& out)
    {
        // Time particle is created relative to the global running
        // time of the particle system.
        out.initialTime = mTime;
        out.age=mTime;

        // Flare lives for 2-4 seconds.
        // original values //out.lifeTime   = GetRandomFloat(2.0f, 8.0f);
        out.lifeTime   = GetRandomFloat(mMinLifeTime, mMaxLifeTime);

        // Initial size in pixels.
        // original values //out.initialSize  = GetRandomFloat(10.0f, 15.0f);
        out.initialSize = mInitialSize; //GetRandomFloat(mMinSize, mMaxSize);

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
        out.mass = GetRandomFloat(1.0f, 2.0f);

        // Start color at 50-100% intensity when born for variation.
        out.initialColor = static_cast<DWORD>(GetRandomFloat(0.5f, 1.0f) *    1.0f);//white;
   
        out.initialPos = mInitPos;
    }


    void ColdParticle::update(float dt)
    {
        
        mPrevTime = mTime;
	    mTime += dt;

	    // Rebuild the dead and alive list.  Note that resize(0) does
	    // not deallocate memory (i.e., the capacity of the vector does
	    // not change).
	    mDeadParticles.resize(0);
        mAliveParticles.resize(0);

	    // For each particle.
	    for(int i = 0; i < mMaxNumParticles; ++i)
	    {
            mParticles[i].age = mPrevTime;
            if(!mIsActive)
            {
                mParticles[i].lifeTime = 0;
            }
		    // Is the particle dead?
  		    if( (mTime - mParticles[i].initialTime) > mParticles[i].lifeTime)
		    {
			    mDeadParticles.push_back(&mParticles[i]);
		    }
		    else
		    {
			    mAliveParticles.push_back(&mParticles[i]);
		    }
	    }

        // A negative or zero mTimePerParticle value denotes
	    // not to emit any particles.
	    if( mTimePerParticle > 0.0f )
        {
            // Emit particles.
            addParticle();
        }
    }
}

