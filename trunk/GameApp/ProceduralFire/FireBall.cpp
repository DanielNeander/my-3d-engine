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
#include "FireBall.h"



namespace ParticleEmitter{


void FireBall::initParticle(ParticleEmitter::Particle& out)
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
	out.initialSize  = GetRandomFloat(mMinSize, mMaxSize);

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

	V3 temp = mParticleRadius - mInitPos;
    float length = temp.Length();
    V3 tempNormal = temp.Normalize();
	
	//D3DXMATRIX outmtx;
	noMat3 outmtx;
	//D3DXMatrixRotationYawPitchRoll(&outmtx,GetRandomFloat(0.0f,2.0f)*noMath::PI,GetRandomFloat(0.0f,2.0f)*noMath::PI,0.0f);
	noAngles angle(RAD2DEG(GetRandomFloat(0.0f,2.0f)*noMath::PI), RAD2DEG(GetRandomFloat(0.0f,2.0f)*noMath::PI), 0.0f);
	outmtx = angle.ToMat3();
	
	//D3DXVec3TransformCoord(&temp,&temp,&outmtx);	
	temp = outmtx * temp;	
	out.initialPos = mInitPos + (temp * length);
}

}