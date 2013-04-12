#ifndef __PARTICLE_TRAIL_H__
#define __PARTICLE_TRAIL_H__

#include "EngineCore/Unreal3/UnName.h"
#include "Distributions.h"


class UParticleSystemComponent : public GameObjectComponentBase
{
public:
	BITFIELD bJustAttached:1;
};

class UParticleEmitter //: public UObject
{
public:
	//## BEGIN PROPS ParticleEmitter
	FName EmitterName;
	BITFIELD UseLocalSpace:1;
	BITFIELD KillOnDeactivate:1;
	BITFIELD bKillOnCompleted:1;
	BITFIELD ScaleUV:1;
	BITFIELD DirectUV:1;
	BITFIELD bEnabled:1;
	BITFIELD ConvertedModules:1;
	struct FRawDistributionFloat SpawnRate;
	FLOAT EmitterDuration;
	INT EmitterLoops;
	BYTE ParticleBurstMethod;
	BYTE InterpolationMethod;
	BYTE EmitterRenderMode;
	TArrayNoInit<struct FParticleBurst> BurstList;
	INT SubImages_Horizontal;
	INT SubImages_Vertical;
	FLOAT RandomImageTime;
	INT RandomImageChanges;
	INT SubUVDataOffset;
	FColor EmitterEditorColor;
	TArrayNoInit<class UParticleLODLevel*> LODLevels;
	TArrayNoInit<class UParticleModule*> Modules;
	class UParticleModule* TypeDataModule;
	TArrayNoInit<class UParticleModule*> SpawnModules;
	TArrayNoInit<class UParticleModule*> UpdateModules;
	INT PeakActiveParticles;
	INT InitialAllocationCount;
};

class UParticleSpriteEmitter : public UParticleEmitter
{
public:

};

class UDistributionFloatParticleParameter : public UDistributionFloatParameterBase
{
public:
	//## BEGIN PROPS DistributionFloatParticleParameter
	//## END PROPS DistributionFloatParticleParameter

	//DECLARE_CLASS(UDistributionFloatParticleParameter,UDistributionFloatParameterBase,0,Engine)
	virtual UBOOL GetParamValue(UObject* Data, FName ParamName, FLOAT& OutFloat);
};



class UParticleModule
{
public:
};

class ParticleTrailBase : public UParticleModule
{

public:
	//virtual EModuleType	GetModuleType() const	{	return EPMT_Trail;	}

};

class ParticleTrailSource : public ParticleTrailBase
{
public:
	BYTE SourceMethod;
	BYTE SelectionMethod;
	FName SourceName;
	struct FRawDistributionFloat SourceStrength;
	BITFIELD bLockSourceStength:1;
	BITFIELD bInheritRotation:1;
	INT SourceOffsetCount;
	TArrayNoInit<FVector> SourceOffsetDefaults;
};

class ParticleTrailSpawn : public ParticleTrailBase
{
public:
	UDistributionFloatParticleParameter* SpawnDistanceMap;
	FLOAT MinSpawnVelocity;
};

class ParticleTrailTaper : public ParticleTrailBase
{
public:
	BYTE TaperMethod;
	noRawDistributionFloat TaperFactor;
};

class ParticleTypeDataBase 
{
public:

};

class ParticleTypeDataTrail : public ParticleTypeDataBase
{
public:
	INT TessellationFactor;
	FLOAT TessellationFactorDistance;
	FLOAT TessellationStrength;
	INT TextureTile;
	INT Sheets;
	INT MaxTrailCount;
	INT MaxParticleInTrailCount;
	BITFIELD RenderGeometry:1;
	BITFIELD RenderDirectLine:1;
	BITFIELD RenderLines:1;
	BITFIELD RenderTessellation:1;
};

#endif