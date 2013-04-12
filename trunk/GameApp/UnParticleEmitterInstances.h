#ifndef PARTICLE_EMITTER_INSTANCES_H
#define PARTICLE_EMITTER_INSTANCES_H

#include <vector>
#include <map>

#include <EngineCore/Math/Vector.h>

class noParticleEmitter;
class noParticleSpriteEmitter;
class noParticleEvent;
class noParticleEvent;

class noParticleEventDataBeam2;
class noParticleEventBeamSource;
class noParticleEventBeamTarget;
class noParticleEventBeamNoise;
class noParticleEventBeamModifier;
class noParticleEventBeamModifier;

struct noParticleEmitterInstanceType {
	const char* Name;
	noParticleEmitterInstanceType* Super;
	noParticleEmitterInstanceType(const char* name, noParticleEmitterInstanceType* super)
		:Name(name), Super(super)
	{

	}

	__forceinline BOOL IsA(noParticleEmitterInstanceType& Type)
	{
		noParticleEmitterInstanceType* CurrentSuper = this;
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
	static noParticleEmitterInstanceType StaticType;					\
	virtual noParticleEmitterInstanceType* Type()					\
{																\
	return &StaticType;											\
}

#define IMPLEMENT_PARTICLEEMITTERINSTANCE_TYPE(TypeName)			\
	noParticleEmitterInstanceType TypeName::StaticType(				\
	TEXT(#TypeName), &TypeName::SuperResource::StaticType);

struct FLODBurstFired
{
	std::vector<UINT32> BurstFired;
};


struct noParticleEmitterInstance {
public:
	static noParticleEmitterInstanceType StaticType;

	static const float PeakActiveParticleUpdateDelta;
	noParticleSpriteEmitter* SpriteTemplate;
	INT CurrentLODLevelIndex;
	INT TypeDataOffset;
	INT SubUVDataOffset;
	INT DynamicParameterDataOffset;
	/** The offset to the Orbit module payload in the particle data.	*/
	INT OrbitModuleOffset;
	/** The location of the emitter instance							*/
	noVec3 Location;
	/** If TRUE, kill this emitter instance when it is deactivated.		*/
	UINT32 KillOnDeactivate:1;
	/** if TRUE, kill this emitter instance when it has completed.		*/
	UINT32 bKillOnCompleted:1;
	/** Whether this emitter requires sorting as specified by artist.	*/
	UINT32 bRequiresSorting:1;
	/** Pointer to the particle data array.								*/
	BYTE* ParticleData;
	/** Pointer to the particle index array.							*/
	WORD* ParticleIndices;
	/** Map module pointers to their offset into the particle data.		*/
	std::map<noParticleEvent*,UINT> ModuleOffsetMap;
	/** Pointer to the instance data array.								*/
	BYTE* InstanceData;
	/** The size of the Instance data array.							*/
	INT InstancePayloadSize;

	/** Map module pointers to their offset into the instance data.		*/
    std::map<noParticleEvent*,UINT> ModuleInstanceOffsetMap;
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
	noVec3 OldLocation;
	/** The bounding box for the particles.								*/
	//FBox ParticleBoundingBox;
	/** The BurstFire information.										*/
	std::vector<struct FLODBurstFired> BurstFired;
	/** The number of loops completed by the instance.					*/
	INT LoopCount;
	/** Flag indicating if the render data is dirty.					*/
	INT IsRenderDataDirty;
	/** The AxisLock module - to avoid finding each Tick.				*/
	//UParticleModuleOrientationAxisLock* Module_AxisLock;
	/** The current duration fo the emitter instance.					*/
	FLOAT EmitterDuration;
	/** The emitter duration at each LOD level for the instance.		*/
	std::vector<FLOAT> EmitterDurations;

	/** The number of triangles to render								*/
	INT	TrianglesToRender;
	INT MaxVertexIndex;

	/** The material to render this instance with.						*/
	//UMaterialInterface* CurrentMaterial;

	noParticleEmitterInstance();
	virtual ~noParticleEmitterInstance();

	virtual noParticleEmitterInstanceType* Type()			{	return &StaticType;		}

	virtual void InitParameters(noParticleEmitter* InTemplate, BOOL bClearResources = TRUE);
	virtual void Init();
	virtual BOOL Resize(INT NewMaxActiveParticles, BOOL bSetMaxActiveCount = TRUE);
	virtual void Update(FLOAT DeltaTime, BOOL bSuppressSpawning);

	virtual BYTE* GetModuleInstanceData(noParticleEvent* event);
	virtual UINT CalculateParticleStride(UINT ParticleSize);
	virtual void ResetBurstList();
	virtual FLOAT GetCurrentBurstRateOffset(FLOAT& DeltaTime, INT& Burst);
	virtual void ResetParticleParameters(FLOAT DeltaTime, DWORD StatId);
	/*void CalculateOrbitOffset(FOrbitChainModuleInstancePayload& Payload, 
		noVec3& AccumOffset, noVec3& AccumRotation, noVec3& AccumRotationRate, 
		FLOAT DeltaTime, noVec3& Result, FMatrix& RotationMat, TArray<noVec3>& Offsets, UBOOL bStoreResult = TRUE);*/
	virtual void UpdateOrbitData(FLOAT DeltaTime);
	virtual void ParticlePrefetch();

	virtual FLOAT Spawn(FLOAT DeltaTime);

	virtual void ForceSpawn(FLOAT DeltaTime, INT InSpawnCount, INT InBurstCount, noVec3& InLocation, noVec3& InVelocity);
	virtual FLOAT Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst = 0, FLOAT BurstTime = 0.0f);
	virtual void PreSpawn(noBaseParticle* Particle);
	virtual BOOL HasCompleted();
	virtual void PostSpawn(noBaseParticle* Particle, FLOAT InterpolationPercentage, FLOAT SpawnTime);
	virtual void KillParticles();

	virtual void KillParticle(INT Index);
	virtual void KillParticlesForced();
	virtual void RemovedFromScene();
	virtual noBaseParticle* GetParticle(INT Index);

	/**
	 *	Calculates the emitter duration for the instance.
	 */
	void SetupEmitterDuration();
	
	/**
	 * Returns whether the system has any active particles.
	 *
	 * @return TRUE if there are active particles, FALSE otherwise.
	 */
	BOOL HasActiveParticles()
	{
		return ActiveParticles > 0;
	}

	/**
	 *	Retrieves the dynamic data for the emitter
	 */
	/*virtual FDynamicEmitterDataBase* GetDynamicData(UBOOL bSelected)
	{
		return NULL;
	}*/
	
	/**
	 *	Retrieves replay data for the emitter
	 *
	 *	@return	The replay data, or NULL on failure
	 */
	/*virtual noDynamicEmitterReplayDataBase* GetReplayData()
	{
		return NULL;
	}*/

	virtual void GetAllocatedSize(INT& InNum, INT& InMax)
	{
		InNum = 0;
		InMax = 0;
	}

	virtual void ProcessEvents(FLOAT DeltaTime, BOOL bSuppressSpawning);

protected:

};

template<class T> T* CastEmitterInstance(noParticleEmitterInstance* Src)
{
	return Src && Src->Type()->IsA(T::StaticType) ? (T*)Src : NULL;
}

template<class T, class U> T* CastEmitterInstanceChecked(U* Src)
{
	if (!Src || !Src->Type()->IsA(T::StaticType))
	{
		//appErrorf(TEXT("Cast of %s to %s failed"), Src ? Src->Type()->Name : TEXT("NULL"), T::StaticType.Name);
		ASSERT(false);
	}
	return (T*)Src;
}

struct noParticleSpriteEmitterInstance : public noParticleEmitterInstance {

	noParticleSpriteEmitterInstance();

	virtual ~noParticleSpriteEmitterInstance();

protected:


};

struct noParticleBeamEmitterInstance : public noParticleEmitterInstance {

	noParticleEventDataBeam2*		m_beamData;
	noParticleEventBeamSource*		m_beamEventSrc;
	noParticleEventBeamTarget*		m_beamEventTgt;
	noParticleEventBeamNoise*		m_beamEventNoise;
	noParticleEventBeamModifier*	m_beamEventSrcModifier;
	int								m_beamEventSrcModifierOffset;						
	noParticleEventBeamModifier*	m_beamEventTgtModifier;
	int								m_beamEventTgtModifierOffset;

	BOOL							m_firstEmission;
	int								m_lastEmittedParticleIndex;
	int								m_tickCnt;
	int								m_forceSpawnCnt;
	int								m_beamMethod;
	std::vector<int>				m_textureTiles;
	int								m_beamCnt;
	noParticleEmitterInstance*		m_srcEmitter;
	std::vector<noVec3>				m_userSetSrcArray;
	std::vector<noVec3>				m_userSetSrcTangentArray;
	std::vector<noVec3>				m_userSetSrcStrenthArray;
	std::vector<float>				m_distArray;
	std::vector<float>				m_targetPointArray;
	std::vector<std::string>		m_targetPointSrcNames;
	std::vector<noVec3>				m_targetTangentArray;
	std::vector<float>				m_userSetTargetStrengthArray;

	noParticleBeamEmitterInstance*	m_targetEmitter;
	std::vector<noVec3>				m_userSetTargetArray;
	std::vector<noVec3>				m_userSetTargetTangentArray;
	std::vector<noVec3>				m_userSetTargetStrenthArray;

	int								m_vertexCnt;
	int								m_triangleCnt;
	std::vector<int>				m_beamTrianglesPerSheet;

	noParticleBeamEmitterInstance();
	virtual ~noParticleBeamEmitterInstance();

	virtual void SetBeamType(INT NewMethod);
	virtual void SetTessellationFactor(FLOAT NewFactor);
	virtual void SetEndPoint(noVec3 NewEndPoint);
	virtual void SetDistance(FLOAT Distance);
	virtual void SetSourcePoint(noVec3 NewSourcePoint,INT SourceIndex);
	virtual void SetSourceTangent(noVec3 NewTangentPoint,INT SourceIndex);
	virtual void SetSourceStrength(FLOAT NewSourceStrength,INT SourceIndex);
	virtual void SetTargetPoint(noVec3 NewTargetPoint,INT TargetIndex);
	virtual void SetTargetTangent(noVec3 NewTangentPoint,INT TargetIndex);
	virtual void SetTargetStrength(FLOAT NewTargetStrength,INT TargetIndex);

	virtual void InitParameters(noParticleEmitter* InTemplate, BOOL bClearResources = TRUE);
	virtual void Init();
	virtual void Update(FLOAT DeltaTime, BOOL bSuppressSpawning);
	//virtual void UpdateBoundingBox(FLOAT DeltaTime);
	//virtual UINT RequiredBytes();
	virtual FLOAT Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst = 0, FLOAT BurstTime = 0.0f);
	virtual void PreSpawn(noBaseParticle* Particle);

	void	SetupBeamEvents();
	void	SetupBeamModifierEvents();

protected:

};

#endif