#pragma once 


#include "ParticleTrail.h"

/*-----------------------------------------------------------------------------
	Particle Emitter Instance type
-----------------------------------------------------------------------------*/
struct FParticleEmitterInstanceType
{
	const TCHAR* Name;
	FParticleEmitterInstanceType* Super;

	/** Constructor */
	FParticleEmitterInstanceType(const TCHAR* InName, FParticleEmitterInstanceType* InSuper) :
		Name(InName),
		Super(InSuper)
	{
	}

	/** 
	 *	IsA 
	 *	@param	Type			The type to check for
	 *	@return UBOOL	TRUE	if the type is exactly or is derived from the given type.
	 *					FALSE	if not
	 */
	FORCEINLINE UBOOL IsA(FParticleEmitterInstanceType& Type)
	{
		FParticleEmitterInstanceType* CurrentSuper = this;
		while (CurrentSuper)
		{
			if (CurrentSuper == &Type)
			{
				return TRUE;
			}
			CurrentSuper = CurrentSuper->Super;
		}

		return FALSE;
	}
};

#define DECLARE_PARTICLEEMITTERINSTANCE_TYPE(TypeName, SuperType)	\
	typedef SuperType SuperResource;								\
	static FParticleEmitterInstanceType StaticType;					\
	virtual FParticleEmitterInstanceType* Type()					\
	{																\
		return &StaticType;											\
	}

#define IMPLEMENT_PARTICLEEMITTERINSTANCE_TYPE(TypeName)			\
	FParticleEmitterInstanceType TypeName::StaticType(				\
		TEXT(#TypeName), &TypeName::SuperResource::StaticType);

struct FLODBurstFired
{
    TArray<UBOOL> BurstFired;
};

/*-----------------------------------------------------------------------------
	FParticleEmitterInstance
-----------------------------------------------------------------------------*/
// This is the base-class 'IMPLEMENT' line
//FParticleEmitterInstanceType FParticleEmitterInstance::StaticType(TEXT("FParticleEmitterInstance"),NULL);
struct FParticleEmitterInstance
{
public:
	static FParticleEmitterInstanceType	StaticType;

	/** The maximum DeltaTime allowed for updating PeakActiveParticle tracking.
	 *	Any delta time > this value will not impact active particle tracking.
	 */
	static const FLOAT PeakActiveParticleUpdateDelta;

	/** The template this instance is based on.							*/
	UParticleSpriteEmitter* SpriteTemplate;
	/** The component who owns it.										*/
    UParticleSystemComponent* Component;
	/** The index of the currently set LOD level.						*/
    INT CurrentLODLevelIndex;
	/** The currently set LOD level.									*/
    //UParticleLODLevel* CurrentLODLevel;
	/** The offset to the TypeDate payload in the particle data.		*/
    INT TypeDataOffset;
	/** The offset to the SubUV payload in the particle data.			*/
    INT SubUVDataOffset;
	/** The offset to the dynamic parameter payload in the particle data*/
    INT DynamicParameterDataOffset;
	/** The offset to the Orbit module payload in the particle data.	*/
	INT OrbitModuleOffset;
	/** The location of the emitter instance							*/
    FVector Location;
	/** If TRUE, kill this emitter instance when it is deactivated.		*/
    BITFIELD KillOnDeactivate:1;
	/** if TRUE, kill this emitter instance when it has completed.		*/
    BITFIELD bKillOnCompleted:1;
	/** Whether this emitter requires sorting as specified by artist.	*/
	BITFIELD bRequiresSorting:1;
	/** Pointer to the particle data array.								*/
    BYTE* ParticleData;
	/** Pointer to the particle index array.							*/
    WORD* ParticleIndices;
	/** Map module pointers to their offset into the particle data.		*/
    std::map<UParticleModule*,UINT> ModuleOffsetMap;
	/** Pointer to the instance data array.								*/
    BYTE* InstanceData;
	/** The size of the Instance data array.							*/
    INT InstancePayloadSize;
	/** Map module pointers to their offset into the instance data.		*/
	std:map<UParticleModule*,UINT> ModuleInstanceOffsetMap;
	/** The offset to the particle data.								*/
    INT PayloadOffset;
	/** The total size of a particle (in bytes).						*/
    INT ParticleSize;
	/** The stride between particles in the ParticleData array.			*/
    INT ParticleStride;
	/** The number of particles currently active in the emitter.		*/
    INT ActiveParticles;
	/** The maximum number of active particles that can be held in 
	 *	the particle data array.
	 */
    INT MaxActiveParticles;
	/** The fraction of time left over from spawning.					*/
    FLOAT SpawnFraction;
	/** The number of seconds that have passed since the instance was
	 *	created.
	 */
    FLOAT SecondsSinceCreation;
	/** */
    FLOAT EmitterTime;
	/** The previous location of the instance.							*/
    FVector OldLocation;
	/** The bounding box for the particles.								*/
    FBox ParticleBoundingBox;
	/** The BurstFire information.										*/
    TArray<struct FLODBurstFired> BurstFired;
	/** The number of loops completed by the instance.					*/
    INT LoopCount;
	/** Flag indicating if the render data is dirty.					*/
	INT IsRenderDataDirty;
	/** The AxisLock module - to avoid finding each Tick.				*/
    //UParticleModuleOrientationAxisLock* Module_AxisLock;
	/** The current duration fo the emitter instance.					*/
    FLOAT EmitterDuration;
	/** The emitter duration at each LOD level for the instance.		*/
    TArray<FLOAT> EmitterDurations;

	/** The number of triangles to render								*/
	INT	TrianglesToRender;
	INT MaxVertexIndex;

	/** The material to render this instance with.						*/
	//UMaterialInterface* CurrentMaterial;

	UBOOL bUseNxFluid;

#if !FINAL_RELEASE
	/** Number of events this emitter has generated... */
	INT EventCount;
	INT MaxEventCount;
#endif	//#if !FINAL_RELEASE

	/** Constructor	*/
	FParticleEmitterInstance();

	/** Destructor	*/
	virtual ~FParticleEmitterInstance();

	// Type interface
	virtual FParticleEmitterInstanceType* Type()			{	return &StaticType;		}
};


struct FParticleTrail2EmitterInstance : public FParticleEmitterInstance
{
	DECLARE_PARTICLEEMITTERINSTANCE_TYPE(FParticleTrail2EmitterInstance, FParticleEmitterInstance);

	ParticleTypeDataTrail*	TrailTypeData;
	ParticleTrailSource*		TrailModule_Source;
	INT								TrailModule_Source_Offset;
	ParticleTrailSpawn*		TrailModule_Spawn;
	INT								TrailModule_Spawn_Offset;
	ParticleTrailTaper*		TrailModule_Taper;
	INT								TrailModule_Taper_Offset;

	BITFIELD FirstEmission:1;
	INT LastEmittedParticleIndex;
	INT LastSelectedParticleIndex;
	INT TickCount;
	INT ForceSpawnCount;
	INT VertexCount;
	INT TriangleCount;
	INT Tessellation;
	TArray<INT> TextureTiles;
	INT TrailCount;
	INT MaxTrailCount;
	TArray<FLOAT> TrailSpawnTimes;
	TArray<FVector> SourcePosition;
	TArray<FVector> LastSourcePosition;
	TArray<FVector> CurrentSourcePosition;
	TArray<FVector> LastSpawnPosition;
	TArray<FVector> LastSpawnTangent;
	TArray<FLOAT> SourceDistanceTravelled;
	AActor* SourceActor;
	TArray<FVector> SourceOffsets;
	FParticleEmitterInstance* SourceEmitter;
	INT ActuallySpawned;

	/** Constructor	*/
	FParticleTrail2EmitterInstance();

	/** Destructor	*/
	virtual ~FParticleTrail2EmitterInstance();

	virtual void InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, UBOOL bClearResources = TRUE);
	virtual void Init();
	virtual void Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning);

	//virtual void UpdateBoundingBox(FLOAT DeltaTime);
	virtual FLOAT Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst = 0, FLOAT BurstTime = 0.0f);
	virtual void PreSpawn(FBaseParticle* Particle);
	virtual void KillParticles();

	void SetupTrailModules();
	void ResolveSource();
	void UpdateSourceData(FLOAT DeltaTime);
	void DetermineVertexAndTriangleCount();

};