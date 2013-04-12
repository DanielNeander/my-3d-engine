/*
--------------------------------------------------------------------------------
This source file is part of SkyX.
Visit http://www.paradise-studios.net/products/skyx/

Copyright (C) 2009-2012 Xavier Vergu? Gonz?ez <xavyiy@gmail.com>

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
--------------------------------------------------------------------------------
*/
#include "stdafx.h"
#include "AtmosphereManager.h"
#include "VCloudSkySystem.h"
#include "GameApp/GameApp.h"

namespace VCloud
{
	AtmosphereManager::AtmosphereManager(VCloudSkySystem *s)
		: mSkyX(s)
		, mOptions(Options())
	{
	}

	AtmosphereManager::~AtmosphereManager()
	{
	}

	void AtmosphereManager::_update(const Options& NewOptions, const bool& ForceToUpdateAll)
	{
		
		if (NewOptions.InnerRadius != mOptions.InnerRadius || 
			NewOptions.OuterRadius != mOptions.OuterRadius ||
			ForceToUpdateAll)
		{
			mOptions.InnerRadius = NewOptions.InnerRadius;
			mOptions.OuterRadius = NewOptions.OuterRadius;

			float Scale = 1.0f / (mOptions.OuterRadius - mOptions.InnerRadius),
				  ScaleDepth = (mOptions.OuterRadius - mOptions.InnerRadius) / 2.0f,
				  ScaleOverScaleDepth = Scale / ScaleDepth;

			GetRenderer()->setShader(mSkyX->mSkydomeShader);
			GetRenderer()->setShaderConstant1f("uInnerRadius", mOptions.InnerRadius);
			GetRenderer()->setShaderConstant3f("uCameraPos", vec3(0, mOptions.InnerRadius + (mOptions.OuterRadius-mOptions.InnerRadius)*mOptions.HeightPosition, 0));			
			GetRenderer()->setShaderConstant1f("uScale", Scale);
			GetRenderer()->setShaderConstant1f("uScaleDepth", ScaleDepth);			
			GetRenderer()->setShaderConstant1f("uScaleOverScaleDepth", ScaleOverScaleDepth);					
			
		}

		if (NewOptions.HeightPosition != mOptions.HeightPosition ||
			ForceToUpdateAll)
		{
			mOptions.HeightPosition = NewOptions.HeightPosition;

			GetRenderer()->setShader(mSkyX->mSkydomeShader);
			GetRenderer()->setShaderConstant3f("uCameraPos", vec3(0, mOptions.InnerRadius + (mOptions.OuterRadius-mOptions.InnerRadius)*mOptions.HeightPosition, 0));						
		}

		if (NewOptions.RayleighMultiplier != mOptions.RayleighMultiplier ||
			NewOptions.SunIntensity       != mOptions.SunIntensity       ||
			ForceToUpdateAll)
		{
			mOptions.RayleighMultiplier = NewOptions.RayleighMultiplier;

			float Kr4PI  = mOptions.RayleighMultiplier * 4.0f * noMath::PI,
				  KrESun = mOptions.RayleighMultiplier * mOptions.SunIntensity;

			GetRenderer()->setShader(mSkyX->mSkydomeShader);
			GetRenderer()->setShaderConstant1f("uKr4PI", Kr4PI);
			GetRenderer()->setShaderConstant1f("uKrESun", KrESun);            
		}

		if (NewOptions.MieMultiplier != mOptions.MieMultiplier ||
			NewOptions.SunIntensity  != mOptions.SunIntensity  ||
			ForceToUpdateAll)
		{
			mOptions.MieMultiplier = NewOptions.MieMultiplier;

			float Km4PI  = mOptions.MieMultiplier * 4.0f * noMath::PI,
				  KmESun = mOptions.MieMultiplier * mOptions.SunIntensity;

			GetRenderer()->setShader(mSkyX->mSkydomeShader);
			GetRenderer()->setShaderConstant1f("uKm4PI", Km4PI);
			GetRenderer()->setShaderConstant1f("uKmESun", KmESun);                        
		}

		if (NewOptions.NumberOfSamples != mOptions.NumberOfSamples ||
			ForceToUpdateAll)
		{
			mOptions.NumberOfSamples = NewOptions.NumberOfSamples;

			GetRenderer()->setShader(mSkyX->mSkydomeShader);
			GetRenderer()->setShaderConstant1i("uNumberOfSamples", mOptions.NumberOfSamples);
			GetRenderer()->setShaderConstant1f("uSamples", static_cast<float>(mOptions.NumberOfSamples));
			
		}

		if (NewOptions.WaveLength != mOptions.WaveLength ||
			ForceToUpdateAll)
		{
			mOptions.WaveLength = NewOptions.WaveLength;

			GetRenderer()->setShader(mSkyX->mSkydomeShader);
			GetRenderer()->setShaderConstant3f("uInvWaveLength", 
				vec3(1.0f / noMath::Pow(mOptions.WaveLength.x, 4.0f),
				1.0f / noMath::Pow(mOptions.WaveLength.y, 4.0f),
				1.0f / noMath::Pow(mOptions.WaveLength.z, 4.0f)));			
		}

		if (NewOptions.G != mOptions.G ||
			ForceToUpdateAll)
		{
			mOptions.G = NewOptions.G;

			GetRenderer()->setShader(mSkyX->mSkydomeShader);
			GetRenderer()->setShaderConstant1f("uG", mOptions.G);
			GetRenderer()->setShaderConstant1f("uG2", mOptions.G*mOptions.G);			
		}

		if (NewOptions.Exposure != mOptions.Exposure ||
			ForceToUpdateAll)
		{
			mOptions.Exposure = NewOptions.Exposure;

			GetRenderer()->setShader(mSkyX->mSkydomeShader);
			GetRenderer()->setShaderConstant1f("uExposure", mOptions.Exposure);			
		}

		mSkyX->getCloudsManager()->update();
	}

	const float AtmosphereManager::_scale(const float& cos, const float& uScaleDepth) const
	{
		float x = 1 - cos;
		return uScaleDepth * noMath::Exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
	}

	const noVec3 AtmosphereManager::getColorAt(const noVec3& Direction) const
	{
		/*if (Direction.y < 0)
		{
			return noVec3(0,0,0);
		}*/
		
		// Parameters
		double Scale = 1.0f / (mOptions.OuterRadius - mOptions.InnerRadius),
			   ScaleDepth = (mOptions.OuterRadius - mOptions.InnerRadius) / 2.0f,
		       ScaleOverScaleDepth = Scale / ScaleDepth,
			   Kr4PI  = mOptions.RayleighMultiplier * 4.0f * noMath::PI,
			   KrESun = mOptions.RayleighMultiplier * mOptions.SunIntensity,
			   Km4PI  = mOptions.MieMultiplier * 4.0f * noMath::PI,
			   KmESun = mOptions.MieMultiplier * mOptions.SunIntensity;

		// --- Start vertex program simulation ---
		noVec3
			uLightDir = mSkyX->getController()->getSunDirection(),
			v3Pos = Direction.NormalizeCopy(),
			uCameraPos = noVec3(0, mOptions.InnerRadius + (mOptions.OuterRadius-mOptions.InnerRadius)*mOptions.HeightPosition, 0),
			uInvWaveLength = noVec3(
			                    1.0f / noMath::Pow(mOptions.WaveLength.x, 4.0f),
			                    1.0f / noMath::Pow(mOptions.WaveLength.y, 4.0f),
		   	                    1.0f / noMath::Pow(mOptions.WaveLength.z, 4.0f));

		// Get the ray from the camera to the vertex, and it's Length (far point)
		v3Pos.y += mOptions.InnerRadius;
		noVec3 v3Ray = v3Pos - uCameraPos;
		double fFar = v3Ray.Length();
		v3Ray /= fFar;

		// Calculate the ray's starting position, then calculate its scattering offset
		noVec3 v3Start = uCameraPos;
		double fHeight = uCameraPos.y,
		       fStartAngle = v3Ray * (v3Start) / fHeight,
		       fDepth = noMath::Exp(ScaleOverScaleDepth * (mOptions.InnerRadius - uCameraPos.y)),
		       fStartOffset = fDepth * _scale(fStartAngle, ScaleDepth);

		// Init loop variables
		double fSampleLength = fFar /(double)mOptions.NumberOfSamples,
		       fScaledLength = fSampleLength * Scale,
			   fHeight_, fDepth_, fLightAngle, fCameraAngle, fScatter;
		noVec3 v3SampleRay = v3Ray * fSampleLength,
		              v3SamplePoint = v3Start + v3SampleRay * 0.5f,
					  color = noVec3(0,0,0), v3Attenuate;

        // Loop the ray
		for (int i = 0; i < mOptions.NumberOfSamples; i++)
		{
			fHeight_ = v3SamplePoint.Length();
			fDepth_ = noMath::Exp(ScaleOverScaleDepth * (mOptions.InnerRadius-fHeight_));

			fLightAngle = uLightDir* (v3SamplePoint) / fHeight_;
			fCameraAngle = v3Ray* (v3SamplePoint) / fHeight_;

			fScatter = (fStartOffset + fDepth*(_scale(fLightAngle, ScaleDepth) - _scale(fCameraAngle, ScaleDepth)));

			v3Attenuate = noVec3(
				noMath::Exp(-fScatter * (uInvWaveLength.x * Kr4PI + Km4PI)),
				noMath::Exp(-fScatter * (uInvWaveLength.y * Kr4PI + Km4PI)),
				noMath::Exp(-fScatter * (uInvWaveLength.z * Kr4PI + Km4PI)));

			// Accumulate color
			v3Attenuate *= (fDepth_ * fScaledLength);
			color += v3Attenuate;

			// Next sample point
			v3SamplePoint += v3SampleRay;
		}

		// Outputs
		noVec3 oRayleighColor = color.Multiply(uInvWaveLength * KrESun),
		              oMieColor      = color * KmESun,
		              oDirection     = uCameraPos - v3Pos;

		// --- End vertex program simulation ---
		// --- Start fragment program simulation ---

		double cos = uLightDir* (oDirection) / oDirection.Length(),
		       cos2 = cos*cos,
		       rayleighPhase = 0.75 * (1.0 + 0.5*cos2),
			   g2 = mOptions.G*mOptions.G,
		       miePhase = 1.5f * ((1.0f - g2) / (2.0f + g2)) * 
			            (1.0f + cos2) / noMath::Pow(1.0f + g2 - 2.0f * mOptions.G * cos, 1.5f);

		noVec3 oColor;

		if (mSkyX->getLightingMode() == VCloudSkySystem::LM_LDR)
		{
			oColor = noVec3(
				1 - noMath::Exp(-mOptions.Exposure * (rayleighPhase * oRayleighColor.x + miePhase * oMieColor.x)),
				1 - noMath::Exp(-mOptions.Exposure * (rayleighPhase * oRayleighColor.y + miePhase * oMieColor.y)),
				1 - noMath::Exp(-mOptions.Exposure * (rayleighPhase * oRayleighColor.z + miePhase * oMieColor.z)));
		
			// For night rendering
			float color_ =  (1 - max(oColor.x, max(oColor.y, oColor.z))*10);
			oColor += noMath::ClampFloat(0, 1, color_) 
				* (noVec3(0.05, 0.05, 0.1)
				* (2-0.75f*noMath::ClampFloat(0, 1, -uLightDir.y)) * noMath::Pow(1-Direction.y, 3));
		}
		else
		{
			oColor = mOptions.Exposure * (rayleighPhase * oRayleighColor + miePhase * oMieColor);
		
			float color_= 1 - max(oColor.x, max(oColor.y, oColor.z));
			float nightmult = noMath::ClampFloat(0, 1, (color_*10));

			// For night rendering
			noVec3 nightCol =  noVec3(0.05, 0.05, 0.1) * (2-0.75f*noMath::ClampFloat( 0, 1, -uLightDir.y)) * noMath::Pow(1-Direction.y, 3);
			nightCol.x = noMath::Pow(nightCol.x, 2.2);
			nightCol.y = noMath::Pow(nightCol.y, 2.2);
			nightCol.z = noMath::Pow(nightCol.z, 2.2);

			oColor += nightmult*nightCol;
		}

		// --- End fragment program simulation ---

		// Output color
		return oColor;
	}
}
