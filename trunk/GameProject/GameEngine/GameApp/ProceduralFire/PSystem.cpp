//=============================================================================
// PSystem.cpp by Frank Luna (C) 2004 All Rights Reserved.
//=============================================================================
// This file was originally inspired by Frank Luna from his book 
// Introduction to Game Programming with DX9 a shader approach.  
// It is likely to be unrecognizable from Franks original work but the
// intent is to give credit for the basic inspiration for the particle system structure 
// that I developed.  I would encourage you to get a copy of his book as it might very
// well inspire you as it did me.


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
#include "PSystem.h"

namespace ParticleEmitter{

void ParticleSystem::reinitialize()
{
    for(int i = 0; i < mMaxNumParticles; ++i)
	{
// 		mParticles[i].age = 0.0f;
// 		mParticles[i].lifeTime = 0.0f;
        mParticles[i].initialTime = 0.0f;
//	    mParticles[i].initialPos = V3(0.0f,0.0f,0.0f);
//	    mParticles[i].initialVelocity = V3(0.0f,0.0f,0.0f);
//	    mParticles[i].initialSize = 0.0f; // In pixels.
	}
}

void ParticleSystem::addParticle()
{
	if( mDeadParticles.size() > 0)
	{
		// Reinitialize a particle.
		Particle* p = mDeadParticles.back();
		initParticle(*p);

		// No longer dead.
		mDeadParticles.pop_back();
		mAliveParticles.push_back(p);
	}
}

void ParticleSystem::update(float dt)
{
    mPrevTime = mTime;
	mTime += dt;

	// Rebuild the dead and alive list.  Note that resize(0) does
	// not deallocate memory (i.e., the capacity of the vector does
	// not change).
	mDeadParticles.resize(0);
    mAliveParticles.resize(0);

	// For each particle.
    if(mIsActive)
    {
	    for(int i = 0; i < mMaxNumParticles; ++i)
	    {
            mParticles[i].age = mPrevTime;
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
    }
    else
    {
	    for(int i = 0; i < mMaxNumParticles; ++i)
	    {
            mParticles[i].age = mPrevTime;
            mParticles[i].lifeTime = 0;
            mDeadParticles.push_back(&mParticles[i]);
        }
    }


	// A negative or zero mTimePerParticle value denotes
	// not to emit any particles.
	if( mTimePerParticle > 0.0f )
	{
		// Emit particles.
		static float timeAccum = 0.0f;
		timeAccum += dt;
		while( timeAccum >= mTimePerParticle )
		{
			addParticle();
			timeAccum -= mTimePerParticle;
		}
	}
}

}