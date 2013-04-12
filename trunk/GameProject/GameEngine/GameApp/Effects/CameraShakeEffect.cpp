#include "stdafx.h"
#include "GameApp/GameCamera.h"
#include "CameraShakeEffect.h"


void ScreenShake::UpdateScreenShake(FLOAT DeltaTime, FScreenShakeStruct& Shake, FTPOV& OutPOV)
{
	Shake.TimeToGo -= DeltaTime;

	// Do not update screen shake if not needed
	if( Shake.TimeToGo <= 0.f )
	{
		return;
	}

	// Smooth fade out
	FLOAT ShakePct = Clamp<FLOAT>(Shake.TimeToGo / Shake.TimeDuration, 0.f, 1.f);
	ShakePct = ShakePct*ShakePct*(3.f - 2.f*ShakePct);

	// do not update if percentage is null
	if( ShakePct <= 0.f )
	{
		return;
	}

	// View Offset, Compute sin wave value for each component
	if( Shake.LocAmplitude != vec3_zero )
	{
		FVector	LocOffset = FVector(0, 0, 0);
		if( Shake.LocAmplitude.x != 0.0 ) 
		{
			Shake.LocSinOffset.x += DeltaTime * Shake.LocFrequency.x * ShakePct;
			LocOffset.x = Shake.LocAmplitude.x * appSin(Shake.LocSinOffset.x) * ShakePct;
		}
		if( Shake.LocAmplitude.y != 0.0 ) 
		{
			Shake.LocSinOffset.y += DeltaTime * Shake.LocFrequency.y * ShakePct;
			LocOffset.y = Shake.LocAmplitude.y * appSin(Shake.LocSinOffset.y) * ShakePct;
		}
		if( Shake.LocAmplitude.z != 0.0 ) 
		{
			Shake.LocSinOffset.z += DeltaTime * Shake.LocFrequency.z * ShakePct;
			LocOffset.z = Shake.LocAmplitude.z * appSin(Shake.LocSinOffset.z) * ShakePct;
		}

		// Offset is relative to camera orientation
		//FRotationMatrix CamRotMatrix(OutPOV.Rotation);
		//OutPOV.Location += LocOffset.x * CamRotMatrix.GetAxis(0) + LocOffset.Y * CamRotMatrix.GetAxis(1) + LocOffset.Z * CamRotMatrix.GetAxis(2);
	}

	// View Rotation, compute sin wave value for each component
	if( Shake.RotAmplitude != vec3_zero )
	{
		FVector	RotOffset = FVector(0, 0, 0);
		if( Shake.RotAmplitude.x != 0.0 ) 
		{
			Shake.RotSinOffset.x += DeltaTime * Shake.RotFrequency.x * ShakePct;
			RotOffset.x = Shake.RotAmplitude.x * appSin(Shake.RotSinOffset.x);
		}
		if( Shake.RotAmplitude.y != 0.0 ) 
		{
			Shake.RotSinOffset.y += DeltaTime * Shake.RotFrequency.y * ShakePct;
			RotOffset.y = Shake.RotAmplitude.y * appSin(Shake.RotSinOffset.y);
		}
		if( Shake.RotAmplitude.z != 0.0 ) 
		{
			Shake.RotSinOffset.z += DeltaTime * Shake.RotFrequency.z * ShakePct;
			RotOffset.z = Shake.RotAmplitude.z * appSin(Shake.RotSinOffset.z);
		}
		RotOffset				*= ShakePct;
		OutPOV.Rotation.pitch	+= appTrunc(RotOffset.x);
		OutPOV.Rotation.yaw		+= appTrunc(RotOffset.y);
		OutPOV.Rotation.roll	+= appTrunc(RotOffset.z);
	}

	// Compute FOV change
	if( Shake.FOVAmplitude != 0.0 ) 
	{
		Shake.FOVSinOffset	+= DeltaTime * Shake.FOVFrequency * ShakePct;
		OutPOV.FOV			+= ShakePct * Shake.FOVAmplitude * appSin(Shake.FOVSinOffset);
	}
}
