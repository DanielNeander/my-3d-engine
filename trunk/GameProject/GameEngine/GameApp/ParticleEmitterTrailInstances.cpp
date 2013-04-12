#include "stdafx.h"
#include "ParticleEmitterInstances.h"

IMPLEMENT_PARTICLEEMITTERINSTANCE_TYPE(FParticleTrail2EmitterInstance);


FParticleTrail2EmitterInstance::FParticleTrail2EmitterInstance() :
FParticleEmitterInstance()
	, TrailTypeData(NULL)
	, TrailModule_Source(NULL)
	, TrailModule_Source_Offset(0)
	, TrailModule_Spawn(NULL)
	, TrailModule_Spawn_Offset(0)
	, TrailModule_Taper(NULL)
	, TrailModule_Taper_Offset(0)
	, FirstEmission(0)
	, LastEmittedParticleIndex(-1)
	, LastSelectedParticleIndex(-1)
	, TickCount(0)
	, ForceSpawnCount(0)
	, VertexCount(0)
	, TriangleCount(0)
	, Tessellation(0)
	, TrailCount(0)
	, MaxTrailCount(0)
	, SourceActor(NULL)
	, SourceEmitter(NULL)
	, ActuallySpawned(0)
{
	TextureTiles.Empty();
	TrailSpawnTimes.Empty();
	SourcePosition.Empty();
	LastSourcePosition.Empty();
	CurrentSourcePosition.Empty();
	LastSpawnPosition.Empty();
	LastSpawnTangent.Empty();
	SourceDistanceTravelled.Empty();
	SourceOffsets.Empty();
}

/** Destructor	*/
FParticleTrail2EmitterInstance::~FParticleTrail2EmitterInstance()
{
	TextureTiles.Empty();
	TrailSpawnTimes.Empty();
	SourcePosition.Empty();
	LastSourcePosition.Empty();
	CurrentSourcePosition.Empty();
	LastSpawnPosition.Empty();
	LastSpawnTangent.Empty();
	SourceDistanceTravelled.Empty();
	SourceOffsets.Empty();
}

/**
 *	Initialize the parameters for the structure
 *
 *	@param	InTemplate		The ParticleEmitter to base the instance on
 *	@param	InComponent		The owning ParticleComponent
 *	@param	bClearResources	If TRUE, clear all resource data
 */
void FParticleTrail2EmitterInstance::InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, UBOOL bClearResources)
{
	FParticleEmitterInstance::InitParameters(InTemplate, InComponent, bClearResources);

	// We don't support LOD on trails
	UParticleLODLevel* LODLevel	= InTemplate->GetLODLevel(0);
	check(LODLevel);
	TrailTypeData	= CastChecked<UParticleModuleTypeDataTrail2>(LODLevel->TypeDataModule);
	check(TrailTypeData);

	TrailModule_Source			= NULL;
	TrailModule_Source_Offset	= 0;
	TrailModule_Spawn			= NULL;
	TrailModule_Spawn_Offset	= 0;
	TrailModule_Taper			= NULL;
	TrailModule_Taper_Offset	= 0;

	// Always have at least one trail
	if (TrailTypeData->MaxTrailCount == 0)
	{
		TrailTypeData->MaxTrailCount	= 1;
	}

	//@todo. Remove this statement once multiple trails per emitter is implemented. 
	TrailTypeData->MaxTrailCount	= 1;

	// Always have at least one particle per trail
	if (TrailTypeData->MaxParticleInTrailCount == 0)
	{
		// Doesn't make sense to have 0 for this...
		//warnf(TEXT("TrailEmitter %s --> MaxParticleInTrailCount == 0!"), *(InTemplate->GetPathName()));		
		TrailTypeData->MaxParticleInTrailCount	= 1;
	}

	MaxTrailCount				= TrailTypeData->MaxTrailCount;
	TrailSpawnTimes.AddZeroed(MaxTrailCount);
	SourceDistanceTravelled.AddZeroed(MaxTrailCount);
	SourcePosition.AddZeroed(MaxTrailCount);
	LastSourcePosition.AddZeroed(MaxTrailCount);
	CurrentSourcePosition.AddZeroed(MaxTrailCount);
	LastSpawnPosition.AddZeroed(MaxTrailCount);
	LastSpawnTangent.AddZeroed(MaxTrailCount);
	SourceDistanceTravelled.AddZeroed(MaxTrailCount);
	FirstEmission				= TRUE;
	LastEmittedParticleIndex	= -1;
	LastSelectedParticleIndex	= -1;
	TickCount					= 0;
	ForceSpawnCount				= 0;

	VertexCount					= 0;
	TriangleCount				= 0;

	TextureTiles.Empty();
	TextureTiles.AddItem(TrailTypeData->TextureTile);

	// Resolve any actors...
	ResolveSource();
}

/**
 *	Initialize the instance
 */
void FParticleTrail2EmitterInstance::Init()
{
	FParticleEmitterInstance::Init();
	// Setup the modules prior to initializing...
	SetupTrailModules();
}

/**
 *	Tick the instance.
 *
 *	@param	DeltaTime			The time slice to use
 *	@param	bSuppressSpawning	If TRUE, do not spawn during Tick
 */
void FParticleTrail2EmitterInstance::Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning)
{
	// Only support the high LOD
	UParticleLODLevel* LODLevel	= SpriteTemplate->GetLODLevel(0);
	check(LODLevel);

	// Make sure we don't try and do any interpolation on the first frame we are attached (OldLocation is not valid in this circumstance)
	if(Component->bJustAttached)
	{
		Location	= Component->LocalToWorld.GetOrigin();
		OldLocation	= Location;
	}
	else
	{
		// Keep track of location for world- space interpolation and other effects.
		OldLocation	= Location;
		Location	= Component->LocalToWorld.GetOrigin();
	}

	SecondsSinceCreation += DeltaTime;

	// Update time within emitter loop.
	EmitterTime = SecondsSinceCreation;

	UBOOL bValidDuration = FALSE;
	if (EmitterDuration > KINDA_SMALL_NUMBER)
	{
		bValidDuration = TRUE;
		EmitterTime = appFmod(SecondsSinceCreation, EmitterDuration);
	}

	// Take delay into account
	FLOAT EmitterDelay = LODLevel->RequiredModule->EmitterDelay;

	// If looping, handle it
	if (bValidDuration && ((SecondsSinceCreation - (EmitterDuration * LoopCount)) >= EmitterDuration))
	{
		LoopCount++;
		ResetBurstList();

		if ((LoopCount == 1) && (LODLevel->RequiredModule->bDelayFirstLoopOnly == TRUE) && 
			((LODLevel->RequiredModule->EmitterLoops == 0) || (LODLevel->RequiredModule->EmitterLoops > 1)))
		{
			// Need to correct the emitter durations...
			for (INT LODIndex = 0; LODIndex < SpriteTemplate->LODLevels.Num(); LODIndex++)
			{
				UParticleLODLevel* TempLOD = SpriteTemplate->LODLevels(LODIndex);
				EmitterDurations(TempLOD->Level) -= TempLOD->RequiredModule->EmitterDelay;
			}
			EmitterDuration		= EmitterDurations(CurrentLODLevelIndex);
		}
	}

	// Don't delay unless required
	if ((LODLevel->RequiredModule->bDelayFirstLoopOnly == TRUE) && (LoopCount > 0))
	{
		EmitterDelay = 0;
	}

	// 'Reset' the emitter time so that the modules function correctly
	EmitterTime -= EmitterDelay;

	// Update the source data (position, etc.)
	UpdateSourceData(DeltaTime);

	// Kill before the spawn... Otherwise, we can get 'flashing'
	KillParticles();
	
	// We need to update the source travelled distance
	for (INT i=0; i<ActiveParticles; i++)
	{
		DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);

		INT						CurrentOffset		= TypeDataOffset;
		FTrail2TypeDataPayload*	TrailData			= NULL;
		FLOAT*					TaperValues			= NULL;


		TrailTypeData->GetDataPointers(this, (const BYTE*)&Particle, CurrentOffset, TrailData, TaperValues);

		if (TRAIL_EMITTER_IS_START(TrailData->Flags))
		{
			UBOOL	bGotSource	= FALSE;

			FVector LastPosition = SourcePosition(TrailData->TrailIndex);
			FVector Position;

			if (TrailModule_Source)
			{
				Position = CurrentSourcePosition(TrailData->TrailIndex);
				bGotSource	= TRUE;
			}

			if (!bGotSource)
			{
				// Assume it should be taken from the emitter...
				Position	= Component->LocalToWorld.GetOrigin();
			}

			FVector Travelled	= Position - LastPosition;
			FLOAT	Distance	= Travelled.Size();

			SourceDistanceTravelled(TrailData->TrailIndex) += Distance;
			if (Distance > KINDA_SMALL_NUMBER)
			{
				SourcePosition(TrailData->TrailIndex) = Position;
			}
		}
		else
		{
			// Nothing...
		}
	}

	// If not suppressing spawning...
	if (!bSuppressSpawning)
	{
		if ((LODLevel->RequiredModule->EmitterLoops == 0) || 
			(LoopCount < LODLevel->RequiredModule->EmitterLoops) ||
			(SecondsSinceCreation < (EmitterDuration * LODLevel->RequiredModule->EmitterLoops)))
		{
			// For Trails, we probably want to ignore the SpawnRate distribution,
			// and focus strictly on the BurstList...
			FLOAT SpawnRate = 0.0f;
			// Figure out spawn rate for this tick.
			SpawnRate = LODLevel->SpawnModule->Rate.GetValue(EmitterTime, Component);

			// Take Bursts into account as well...
			INT		Burst		= 0;
			FLOAT	BurstTime	= GetCurrentBurstRateOffset(DeltaTime, Burst);
			SpawnRate += BurstTime;

			// Spawn new particles...

			//@todo. Fix the issue of 'blanking' Trails when the count drops...
			// This is a temporary hack!
			if ((ActiveParticles < MaxTrailCount) && (SpawnRate <= KINDA_SMALL_NUMBER))
			{
				// Force the spawn of a single Trail...
				SpawnRate = 1.0f / DeltaTime;
			}

			if (TrailModule_Spawn && TrailModule_Spawn->bEnabled)
			{
				INT	SpawnModCount = TrailModule_Spawn->GetSpawnCount(this, DeltaTime);
				INT	MaxParticlesAllowed	= MaxTrailCount * TrailTypeData->MaxParticleInTrailCount;
				if ((SpawnModCount + ActiveParticles) > MaxParticlesAllowed)
				{
					SpawnModCount	= MaxParticlesAllowed - ActiveParticles - 1;
					if (SpawnModCount < 0)
					{
						SpawnModCount = 0;
					}
				}

				if (ActiveParticles >= (TrailTypeData->MaxParticleInTrailCount * MaxTrailCount))
				{
					SpawnModCount = 0;
				}

				if (SpawnModCount)
				{
					//debugf(TEXT("SpawnModCount = %d"), SpawnModCount);
					// Set the burst for this, if there are any...
					SpawnFraction	= 0.0f;
					Burst			= SpawnModCount;
					SpawnRate		= Burst / DeltaTime;
				}
			}
			else
			{
				if ((ActiveParticles > 0) && (SourceDistanceTravelled(0) == 0.0f))
				{
					SpawnRate = 0.0f;
					//debugf(TEXT("Killing SpawnRate (no distance travelled)"));
				}
			}

			if (SpawnRate > 0.f)
			{
				SpawnFraction = Spawn(SpawnFraction, SpawnRate, DeltaTime, Burst, BurstTime);
			}
		}
	}

	// Reset velocity and size.
	ResetParticleParameters(DeltaTime, STAT_TrailParticlesUpdated);

	UParticleModuleTypeDataBase* pkBase = 0;
	if (LODLevel->TypeDataModule)
	{
		pkBase = Cast<UParticleModuleTypeDataBase>(LODLevel->TypeDataModule);
		//@todo. Need to track TypeData offset into payload!
		pkBase->PreUpdate(this, TypeDataOffset, DeltaTime);
	}


	// Update existing particles (might respawn dying ones).
	for (INT i=0; i<LODLevel->UpdateModules.Num(); i++)
	{
		UParticleModule* ParticleModule	= LODLevel->UpdateModules(i);
		if (!ParticleModule || !ParticleModule->bEnabled)
		{
			continue;
		}

		UINT* Offset = ModuleOffsetMap.Find(ParticleModule);
		ParticleModule->Update(this, Offset ? *Offset : 0, DeltaTime);
	}


	//@todo. This should ALWAYS be true for Trails...
	if (pkBase)
	{
		// The order of the update here is VERY important
		if (TrailModule_Source && TrailModule_Source->bEnabled)
		{
			TrailModule_Source->Update(this, TrailModule_Source_Offset, DeltaTime);
		}
		if (TrailModule_Spawn && TrailModule_Spawn->bEnabled)
		{
			TrailModule_Spawn->Update(this, TrailModule_Spawn_Offset, DeltaTime);
		}
		if (TrailModule_Taper && TrailModule_Taper->bEnabled)
		{
			TrailModule_Taper->Update(this, TrailModule_Taper_Offset, DeltaTime);
		}

		//@todo. Need to track TypeData offset into payload!
		pkBase->Update(this, TypeDataOffset, DeltaTime);
		pkBase->PostUpdate(this, TypeDataOffset, DeltaTime);
	}

	// Calculate bounding box and simulate velocity.
	UpdateBoundingBox(DeltaTime);

	//DetermineVertexAndTriangleCount();

	if (!bSuppressSpawning)
	{
		// Ensure that we flip the 'FirstEmission' flag
		FirstEmission = false;
	}

	// Invalidate the contents of the vertex/index buffer.
	IsRenderDataDirty = 1;

	// Bump the tick count
	TickCount++;

	// 'Reset' the emitter time so that the delay functions correctly
	EmitterTime += EmitterDelay;
	}
}