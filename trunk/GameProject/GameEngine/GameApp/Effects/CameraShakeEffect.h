#pragma once 

struct FShakeParams
{
	BYTE X;
	BYTE Y;
	BYTE Z;
	BYTE Padding;

	/** Constructors */
	FShakeParams() {}
	FShakeParams(EEventParm)
	{
		appMemzero(this, sizeof(FShakeParams));
	}
};

struct FScreenShakeStruct
{
	FLOAT TimeToGo;
	FLOAT TimeDuration;
	FVector RotAmplitude;
	FVector RotFrequency;
	FVector RotSinOffset;
	struct FShakeParams RotParam;
	FVector LocAmplitude;
	FVector LocFrequency;
	FVector LocSinOffset;
	struct FShakeParams LocParam;
	FLOAT FOVAmplitude;
	FLOAT FOVFrequency;
	FLOAT FOVSinOffset;
	BYTE FOVParam;

	/** Constructors */
	FScreenShakeStruct() {}
	FScreenShakeStruct(EEventParm)
	{
		appMemzero(this, sizeof(FScreenShakeStruct));
	}
};

class ScreenShake 
{
public:
	TArrayNoInit<struct FScreenShakeStruct> Shakes;
	struct FScreenShakeStruct TestShake;

	virtual void UpdateScreenShake(FLOAT DeltaTime,struct FScreenShakeStruct& Shake, struct FTPOV& OutPOV);

};

