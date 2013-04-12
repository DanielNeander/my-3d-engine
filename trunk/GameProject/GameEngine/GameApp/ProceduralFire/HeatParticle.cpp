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
#include "HeatParticle.h"

namespace ParticleEmitter{

void HeatEmitter::initParticle(ParticleEmitter::HeatParticle& out)
{
	// Time particle is created relative to the global running
	// time of the particle system.
	out.initialTime = mTime;

	// Flare lives for 1-2 seconds.
	out.lifeTime   = GetRandomFloat(mMinLifeTime,mMaxLifeTime );
 
	//  velocity to Heat direction
	GetRandomVec(out.initialVelocity);
    out.initialVelocity.x *= mAccelImpulse.x;
    out.initialVelocity.y *= mAccelImpulse.y;
    out.initialVelocity.z *= mAccelImpulse.z;
    out.initialVelocity.x += mAccelShift.x;
    out.initialVelocity.y += mAccelShift.y;
    out.initialVelocity.z += mAccelShift.z;
	
	out.mass = GetRandomFloat(mMinAmplitude, mMaxAmplitude);

	out.initialPos = mInitPos;
    out.currentPos = mInitPos;
    out.prevPos = mInitPos;
}

void HeatEmitter::Activate( 
	const V3 pos,
	const V3 impulse,
	const V3 shift,
	int maxNumParticles,
	float timePerParticle,
    float minLifeTime ,
    float maxLifeTime ,
    float minSize     ,
    float maxSize     ,
    float minAmplitude,
    float maxAmplitude)
{
	mAccelImpulse=impulse;
	mAccelShift=shift;
	mInitPos=pos;
    mPrevTime =0;
    mTime = 0;
    mMaxNumParticles = maxNumParticles;
    mTimePerParticle = timePerParticle;
    mMinLifeTime = (minLifeTime);
    mMaxLifeTime = (maxLifeTime);
    mMinSize = (minSize);
    mMaxSize = (maxSize);
    mMinAmplitude = (minAmplitude);
    mMaxAmplitude = (maxAmplitude);
    
    init();
    mIsActive = true;

}

void HeatEmitter::update(float dt)
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
		// Is the particle dead?
  		if( (mTime - mParticles[i].initialTime) > mParticles[i].lifeTime)
        {
            mParticles[i].currentPos = mParticles[i].prevPos;
			mDeadParticles.push_back(&mParticles[i]);
		}
		else
		{

            mParticles[i].update(dt);
            mAliveParticles.push_back(&mParticles[i]);
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

void HeatEmitter::addParticle()
{
	if( mDeadParticles.size() > 0)
	{
		// Reinitialize a particle.
		HeatParticle* p = mDeadParticles.back();
		initParticle(*p);

		// No longer dead.
		mDeadParticles.pop_back();
		mAliveParticles.push_back(p);
	}
}

Bool HeatEmitter::getPositions( std::vector<V3>  &curPos, 
                                std::vector<V3>  &prevPos) const 
{
	Bool bOutput = False;

    if(mIsActive ){
        {
            std::vector<ParticleEmitter::HeatParticle *>::const_iterator i,iend;
            i = mAliveParticles.begin();
            iend = mAliveParticles.end();
            for (i;i!=iend;i++)
            {
                prevPos.push_back((*i)->prevPos);
                curPos.push_back((*i)->currentPos);
            }
            bOutput = True;
        }
    }

	return bOutput;
}


}