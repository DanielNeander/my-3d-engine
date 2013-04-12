#ifndef EFFECT_PARTICLESYSTEM_H
#define EFFECT_PARTICLESYSTEM_H

//#include <Math/Vector.h>
//#include <Math/Quaternion.h>
#include "Distribution.h"



class noViewport3D;

class UParticleEmitterInstance;
class noBaseParticle;
enum ParticleSystemLODMethod
{
	PARTICLESYSTEMLODMETHOD_Automatic=0,
	PARTICLESYSTEMLODMETHOD_DirectSet=1,
	PARTICLESYSTEMLODMETHOD_MAX=2,
};
enum EParticleSystemUpdateMode
{
	EPSUM_RealTime          =0,
	EPSUM_FixedTime         =1,
	EPSUM_MAX               =2,
};
enum EParticleEventType
{
	EPET_Any                =0,
	EPET_Spawn              =1,
	EPET_Death              =2,
	EPET_Collision          =3,
	EPET_Kismet             =4,
	EPET_MAX                =5,
};
enum ParticleReplayState
{
	PRS_Disabled            =0,
	PRS_Capturing           =1,
	PRS_Replaying           =2,
	PRS_MAX                 =3,
};
enum EParticleSysParamType
{
	PSPT_None               =0,
	PSPT_Scalar             =1,
	PSPT_Vector             =2,
	PSPT_Color              =3,
	PSPT_Actor              =4,
	PSPT_Material           =5,
	PSPT_MAX                =6,
};
enum EEmitterRenderMode
{
	ERM_Normal              =0,
	ERM_Point               =1,
	ERM_Cross               =2,
	ERM_None                =3,
	ERM_MAX                 =4,
};
enum EParticleSubUVInterpMethod
{
	PSUVIM_None             =0,
	PSUVIM_Linear           =1,
	PSUVIM_Linear_Blend     =2,
	PSUVIM_Random           =3,
	PSUVIM_Random_Blend     =4,
	PSUVIM_MAX              =5,
};
enum EParticleBurstMethod
{
	EPBM_Instant            =0,
	EPBM_Interpolated       =1,
	EPBM_MAX                =2,
};
enum EParticleScreenAlignment
{
	PSA_Square              =0,
	PSA_Rectangle           =1,
	PSA_Velocity            =2,
	PSA_TypeSpecific        =3,
	PSA_MAX                 =4,
};
enum EParticleSourceSelectionMethod
{
	EPSSM_Random            =0,
	EPSSM_Sequential        =1,
	EPSSM_MAX               =2,
};
enum EModuleType
{
	EPMT_General            =0,
	EPMT_TypeData           =1,
	EPMT_Beam               =2,
	EPMT_Trail              =3,
	EPMT_Spawn              =4,
	EPMT_Required           =5,
	EPMT_Event              =6,
	EPMT_MAX                =7,
};
enum EAttractorParticleSelectionMethod
{
	EAPSM_Random            =0,
	EAPSM_Sequential        =1,
	EAPSM_MAX               =2,
};
enum Beam2SourceTargetTangentMethod
{
	PEB2STTM_Direct         =0,
	PEB2STTM_UserSet        =1,
	PEB2STTM_Distribution   =2,
	PEB2STTM_Emitter        =3,
	PEB2STTM_MAX            =4,
};
enum Beam2SourceTargetMethod
{
	PEB2STM_Default         =0,
	PEB2STM_UserSet         =1,
	PEB2STM_Emitter         =2,
	PEB2STM_Particle        =3,
	PEB2STM_Actor           =4,
	PEB2STM_MAX             =5,
};
enum BeamModifierType
{
	PEB2MT_Source           =0,
	PEB2MT_Target           =1,
	PEB2MT_MAX              =2,
};
enum EParticleCollisionComplete
{
	EPCC_Kill               =0,
	EPCC_Freeze             =1,
	EPCC_HaltCollisions     =2,
	EPCC_FreezeTranslation  =3,
	EPCC_FreezeRotation     =4,
	EPCC_FreezeMovement     =5,
	EPCC_MAX                =6,
};
enum ELocationEmitterSelectionMethod
{
	ELESM_Random            =0,
	ELESM_Sequential        =1,
	ELESM_MAX               =2,
};
enum CylinderHeightAxis
{
	PMLPC_HEIGHTAXIS_X      =0,
	PMLPC_HEIGHTAXIS_Y      =1,
	PMLPC_HEIGHTAXIS_Z      =2,
	PMLPC_HEIGHTAXIS_MAX    =3,
};
enum EOrbitChainMode
{
	EOChainMode_Add         =0,
	EOChainMode_Scale       =1,
	EOChainMode_Link        =2,
	EOChainMode_MAX         =3,
};
enum EParticleAxisLock
{
	EPAL_NONE               =0,
	EPAL_X                  =1,
	EPAL_Y                  =2,
	EPAL_Z                  =3,
	EPAL_NEGATIVE_X         =4,
	EPAL_NEGATIVE_Y         =5,
	EPAL_NEGATIVE_Z         =6,
	EPAL_ROTATE_X           =7,
	EPAL_ROTATE_Y           =8,
	EPAL_ROTATE_Z           =9,
	EPAL_MAX                =10,
};
enum ETrail2SourceMethod
{
	PET2SRCM_Default        =0,
	PET2SRCM_Particle       =1,
	PET2SRCM_Actor          =2,
	PET2SRCM_MAX            =3,
};
enum ETrail2SpawnMethod
{
	PET2SM_Emitter          =0,
	PET2SM_Velocity         =1,
	PET2SM_Distance         =2,
	PET2SM_MAX              =3,
};
enum ETrailTaperMethod
{
	PETTM_None              =0,
	PETTM_Full              =1,
	PETTM_Partial           =2,
	PETTM_MAX               =3,
};
enum EBeamEndPointMethod
{
	PEBEPM_Calculated       =0,
	PEBEPM_Distribution     =1,
	PEBEPM_Distribution_Constant=2,
	PEBEPM_MAX              =3,
};
enum EBeamMethod
{
	PEBM_Distance           =0,
	PEBM_EndPoints          =1,
	PEBM_EndPoints_Interpolated=2,
	PEBM_UserSet_EndPoints  =3,
	PEBM_UserSet_EndPoints_Interpolated=4,
	PEBM_MAX                =5,
};
enum EBeamTaperMethod
{
	PEBTM_None              =0,
	PEBTM_Full              =1,
	PEBTM_Partial           =2,
	PEBTM_MAX               =3,
};
enum EBeam2Method
{
	PEB2M_Distance          =0,
	PEB2M_Target            =1,
	PEB2M_Branch            =2,
	PEB2M_MAX               =3,
};
enum EMeshScreenAlignment
{
	PSMA_MeshFaceCameraWithRoll=0,
	PSMA_MeshFaceCameraWithSpin=1,
	PSMA_MeshFaceCameraWithLockedAxis=2,
	PSMA_MAX                =3,
};
enum EPhysXMeshRotationMethod
{
	PMRM_Disabled           =0,
	PMRM_Spherical          =1,
	PMRM_Box                =2,
	PMRM_LongBox            =3,
	PMRM_FlatBox            =4,
	PMRM_MAX                =5,
};
enum EPacketSizeMultiplier
{
	EPSM_4                  =0,
	EPSM_8                  =1,
	EPSM_16                 =2,
	EPSM_32                 =3,
	EPSM_64                 =4,
	EPSM_128                =5,
	EPSM_MAX                =6,
};
enum ESimulationMethod
{
	ESM_SPH                 =0,
	ESM_NO_PARTICLE_INTERACTION=1,
	ESM_MIXED_MODE          =2,
	ESM_MAX                 =3,
};

class UParticleSystem 
{
	//## BEGIN PROPS ParticleSystem
	BYTE SystemUpdateMode;
	BYTE LODMethod;
	FLOAT UpdateTime_FPS;
	FLOAT UpdateTime_Delta;
	FLOAT WarmupTime;
	TArrayNoInit<class UParticleEmitter*> Emitters;
	//class UParticleSystemComponent* PreviewComponent;
	FRotator ThumbnailAngle;
	FLOAT ThumbnailDistance;
	FLOAT ThumbnailWarmup;
	BITFIELD bLit:1;
	BITFIELD bRegenerateLODDuplicate:1;
	BITFIELD bUseFixedRelativeBoundingBox:1;
	BITFIELD bShouldResetPeakCounts:1;
	BITFIELD bHasPhysics:1;
	BITFIELD bUseRealtimeThumbnail:1;
	BITFIELD ThumbnailImageOutOfDate:1;
	class UInterpCurveEdSetup* CurveEdSetup;
	FLOAT LODDistanceCheckTime;
	TArrayNoInit<FLOAT> LODDistances;
	INT EditorLODSetting;
	FBox FixedRelativeBoundingBox;
	FLOAT SecondsBeforeInactive;
	FStringNoInit FloorMesh;
	FVector FloorPosition;
	FRotator FloorRotation;
	FLOAT FloorScale;
	FVector FloorScale3D;
	//class UTexture2D* ThumbnailImage;
	TArrayNoInit<FName> ContentTags;

	virtual BYTE GetCurrentLODMethod();
	virtual INT GetLODLevelCount();
	virtual FLOAT GetLODDistance(INT LODLevelIndex);
	virtual void SetCurrentLODMethod(BYTE InMethod);
	virtual UBOOL SetLODDistance(INT LODLevelIndex,FLOAT InDistance);

	void UpdateColorModuleClampAlpha(class UParticleModuleColorBase* ColorModule);

	/**
	 *	CalculateMaxActiveParticleCounts
	 *	Determine the maximum active particles that could occur with each emitter.
	 *	This is to avoid reallocation during the life of the emitter.
	 *
	 *	@return	TRUE	if the numbers were determined for each emitter
	 *			FALSE	if not be determined
	 */
	virtual UBOOL		CalculateMaxActiveParticleCounts();
	
	/**
	 *	Retrieve the parameters associated with this particle system.
	 *
	 *	@param	ParticleSysParamList	The list of FParticleSysParams used in the system
	 *	@param	ParticleParameterList	The list of ParticleParameter distributions used in the system
	 */
	void GetParametersUtilized(TArray<TArray<FString> >& ParticleSysParamList,
							   TArray<TArray<FString> >& ParticleParameterList);
};

struct FParticleSysParam
{
	FName Name;
	BYTE ParamType;
	FLOAT Scalar;
	FVector Vector;
	FColor Color;
	//class AActor* Actor;
	//class UMaterialInterface* Material;

	/** Constructors */
	FParticleSysParam() {}
	FParticleSysParam(EEventParm)
	{
		appMemzero(this, sizeof(FParticleSysParam));
	}
};

struct FParticleEventData
{
	INT Type;
	FName EventName;
	FLOAT EmitterTime;
	FVector Location;
	FVector Direction;
	FVector Velocity;

	/** Constructors */
	FParticleEventData() {}
	FParticleEventData(EEventParm)
	{
		appMemzero(this, sizeof(FParticleEventData));
	}
};

struct FParticleEventSpawnData : public FParticleEventData
{

	/** Constructors */
	FParticleEventSpawnData() {}
	FParticleEventSpawnData(EEventParm)
	{
		appMemzero(this, sizeof(FParticleEventSpawnData));
	}
};

struct FParticleEventDeathData : public FParticleEventData
{
	FLOAT ParticleTime;

	/** Constructors */
	FParticleEventDeathData() {}
	FParticleEventDeathData(EEventParm)
	{
		appMemzero(this, sizeof(FParticleEventDeathData));
	}
};

struct FParticleEventCollideData : public FParticleEventData
{
	FLOAT ParticleTime;
	FVector Normal;
	FLOAT Time;
	INT Item;
	FName BoneName;

	/** Constructors */
	FParticleEventCollideData() {}
	FParticleEventCollideData(EEventParm)
	{
		appMemzero(this, sizeof(FParticleEventCollideData));
	}
};

struct FParticleEventKismetData : public FParticleEventData
{
	BITFIELD UsePSysCompLocation:1;
	FVector Normal;

	/** Constructors */
	FParticleEventKismetData() {}
	FParticleEventKismetData(EEventParm)
	{
		appMemzero(this, sizeof(FParticleEventKismetData));
	}
};

class UParticleSystemComponent //: public UPrimitiveComponent
{
public:
	//## BEGIN PROPS ParticleSystemComponent
	class UParticleSystem* Template;
	TArrayNoInit<struct FParticleEmitterInstance*> EmitterInstances;
	//TArrayNoInit<class UStaticMeshComponent*> SMComponents;
	//TArrayNoInit<class UMaterialInterface*> SMMaterialInterfaces;
	BITFIELD bAutoActivate:1;
	BITFIELD bWasCompleted:1;
	BITFIELD bSuppressSpawning:1;
	BITFIELD bWasDeactivated:1;
	BITFIELD bResetOnDetach:1;
	BITFIELD bUpdateOnDedicatedServer:1;
	BITFIELD bJustAttached:1;
	BITFIELD bIsActive:1;
	BITFIELD bWarmingUp:1;
	BITFIELD bIsCachedInPool:1;
	BITFIELD bOverrideLODMethod:1;
	BITFIELD bSkipUpdateDynamicDataDuringTick:1;
	BITFIELD bUpdateComponentInTick:1;
	BITFIELD bDeferredBeamUpdate:1;
	BITFIELD bForcedInActive:1;
	BITFIELD bIsWarmingUp:1;
	BITFIELD bIsViewRelevanceDirty:1;
	BITFIELD bRecacheViewRelevance:1;
	TArrayNoInit<struct FParticleSysParam> InstanceParameters;
	FVector OldPosition;
	FVector PartSysVelocity;
	FLOAT WarmupTime;
	INT LODLevel;
	FLOAT SecondsBeforeInactive;
	INT EditorLODLevel;
	FLOAT AccumTickTime;
	BYTE LODMethod;
	BYTE ReplayState;
	//TArrayNoInit<FMaterialViewRelevance> CachedViewRelevanceFlags;
	TArrayNoInit<class UParticleSystemReplay*> ReplayClips;
	INT ReplayClipIDNumber;
	INT ReplayFrameIndex;
	TArrayNoInit<struct FParticleEventSpawnData> SpawnEvents;
	TArrayNoInit<struct FParticleEventDeathData> DeathEvents;
	TArrayNoInit<struct FParticleEventCollideData> CollisionEvents;
	TArrayNoInit<struct FParticleEventKismetData> KismetEvents;
	
	//## END PROPS ParticleSystemComponent

	void SetTemplate(class UParticleSystem* NewTemplate);
	void ActivateSystem(UBOOL bFlagAsJustAttached=FALSE);
	void DeactivateSystem();
	void KillParticlesForced();
	void SetSkipUpdateDynamicDataDuringTick(UBOOL bInSkipUpdateDynamicDataDuringTick);
	UBOOL GetSkipUpdateDynamicDataDuringTick();
	virtual void SetKillOnDeactivate(INT EmitterIndex,UBOOL bKill);
	virtual void SetKillOnCompleted(INT EmitterIndex,UBOOL bKill);
	virtual void RewindEmitterInstance(INT EmitterIndex);
	virtual void RewindEmitterInstances();
	virtual void SetBeamType(INT EmitterIndex,INT NewMethod);
	virtual void SetBeamTessellationFactor(INT EmitterIndex,FLOAT NewFactor);
	virtual void SetBeamEndPoint(INT EmitterIndex,FVector NewEndPoint);
	virtual void SetBeamDistance(INT EmitterIndex,FLOAT Distance);
	virtual void SetBeamSourcePoint(INT EmitterIndex,FVector NewSourcePoint,INT SourceIndex);
	virtual void SetBeamSourceTangent(INT EmitterIndex,FVector NewTangentPoint,INT SourceIndex);
	virtual void SetBeamSourceStrength(INT EmitterIndex,FLOAT NewSourceStrength,INT SourceIndex);
	virtual void SetBeamTargetPoint(INT EmitterIndex,FVector NewTargetPoint,INT TargetIndex);
	virtual void SetBeamTargetTangent(INT EmitterIndex,FVector NewTangentPoint,INT TargetIndex);
	virtual void SetBeamTargetStrength(INT EmitterIndex,FLOAT NewTargetStrength,INT TargetIndex);
	void SetLODLevel(INT InLODLevel);
	void SetEditorLODLevel(INT InLODLevel);
	INT GetLODLevel();
	INT GetEditorLODLevel();
	void SetFloatParameter(FName ParameterName,FLOAT Param);
	void SetVectorParameter(FName ParameterName,FVector Param);
	void SetColorParameter(FName ParameterName,FColor Param);
	void SetActorParameter(FName ParameterName,class AActor* Param);
	void SetMaterialParameter(FName ParameterName,class UMaterialInterface* Param);
	virtual UBOOL GetFloatParameter(const FName InName,FLOAT& OutFloat);
	virtual UBOOL GetVectorParameter(const FName InName,FVector& OutVector);
	virtual UBOOL GetColorParameter(const FName InName,FColor& OutColor);
	virtual UBOOL GetActorParameter(const FName InName,class AActor*& OutActor);
	virtual UBOOL GetMaterialParameter(const FName InName,class UMaterialInterface*& OutMaterial);
	void ClearParameter(FName ParameterName,BYTE ParameterType=0);
	void SetActive(UBOOL bNowActive);
	void ResetToDefaults();

protected:
	// UActorComponent interface.
	virtual void Attach();
	virtual void UpdateTransform();
	virtual void Detach( UBOOL bWillReattach = FALSE );
	virtual void UpdateLODInformation();

	/**
	 * Static: Supplied with a chunk of replay data, this method will create dynamic emitter data that can
	 * be used to render the particle system
	 *
	 * @param	EmitterInstance		Emitter instance this replay is playing on
	 * @param	EmitterReplayData	Incoming replay data of any time, cannot be NULL
	 * @param	bSelected			TRUE if the particle system is currently selected
	 *
	 * @return	The newly created dynamic data, or NULL on failure
	 */
	static FDynamicEmitterDataBase* CreateDynamicDataFromReplay( FParticleEmitterInstance* EmitterInstance, const FDynamicEmitterReplayDataBase* EmitterReplayData, UBOOL bSelected );

	/**
	 * Creates dynamic particle data for rendering the particle system this frame.  This function
	 * handle creation of dynamic data for regularly simulated particles, but also handles capture
	 * and playback of particle replay data.
	 *
	 * @return	Returns the dynamic data to render this frame
	 */
	FParticleDynamicData* CreateDynamicData();

public:
	virtual void UpdateDynamicData();
	virtual void UpdateDynamicData(FParticleSystemSceneProxy* Proxy);
	virtual void UpdateViewRelevance(FParticleSystemSceneProxy* Proxy);

	// UPrimitiveComponent interface
	virtual void UpdateBounds();
	virtual void Tick(FLOAT DeltaTime);

	virtual void InitParticles();
	void ResetParticles(UBOOL bEmptyInstances = FALSE);
	void ResetBurstLists();
	void UpdateInstances();
	UBOOL HasCompleted();

	void InitializeSystem();

		/**
	 *	Cache the view-relevance for each emitter at each LOD level.
	 *
	 *	@param	NewTemplate		The UParticleSystem* to use as the template.
	 *							If NULL, use the currently set template.
	 */
	void CacheViewRelevanceFlags(class UParticleSystem* NewTemplate = NULL);

	/**
	*	DetermineLODLevel - determines the appropriate LOD level to utilize.
	*/
	INT DetermineLODLevel(const FSceneView* View);

};

class UDistributionFloatParticleParameter : public UDistributionFloatParameterBase
{
public:
	//## BEGIN PROPS DistributionFloatParticleParameter
	//## END PROPS DistributionFloatParticleParameter

	//DECLARE_CLASS(UDistributionFloatParticleParameter,UDistributionFloatParameterBase,0,Engine)
	virtual UBOOL GetParamValue(UObject* Data, FName ParamName, FLOAT& OutFloat);
};

class UDistributionVectorParticleParameter : public UDistributionVectorParameterBase
{
public:
	//## BEGIN PROPS DistributionVectorParticleParameter
	//## END PROPS DistributionVectorParticleParameter

	//DECLARE_CLASS(UDistributionVectorParticleParameter,UDistributionVectorParameterBase,0,Engine)
	virtual UBOOL GetParamValue(UObject* Data, FName ParamName, FVector& OutVector);
};




class UParticleLODLevel {
public:
	INT Level;
	INT LevelSetting;
	BITFIELD bEnabled:1;
	BITFIELD ConvertedModules:1;
	class UParticleModuleRequired* RequiredModule;
	TArrayNoInit<class UParticleModule*> Modules;
	class UParticleModule* TypeDataModule;
	class UParticleModuleSpawn* SpawnModule;
	class UParticleModuleEventGenerator* EventGenerator;
	TArrayNoInit<class UParticleModuleSpawnBase*> SpawningModules;
	TArrayNoInit<class UParticleModule*> SpawnModules;
	TArrayNoInit<class UParticleModule*> UpdateModules;
	TArrayNoInit<class UParticleModuleOrbit*> OrbitModules;
	TArrayNoInit<class UParticleModuleEventReceiverBase*> EventReceiverModules;

	virtual void	UpdateEventLists();

	virtual UBOOL	GenerateFromLODLevel(UParticleLODLevel* SourceLODLevel, FLOAT Percentage, UBOOL bGenerateModuleData = TRUE);

	/**
	 *	CalculateMaxActiveParticleCount
	 *	Determine the maximum active particles that could occur with this emitter.
	 *	This is to avoid reallocation during the life of the emitter.
	 *
	 *	@return		The maximum active particle count for the LOD level.
	 */
	virtual INT	CalculateMaxActiveParticleCount();

	/**
	 *	Update to the new SpawnModule method
	 */
	void	ConvertToSpawnModule();
		
	/**
	 *	Return the index of the given module if it is contained in the LOD level
	 */
	INT		GetModuleIndex(UParticleModule* InModule);

	/**
	 *	Return the module at the given index if it is contained in the LOD level
	 */
	UParticleModule* GetModuleAtIndex(INT InIndex);

	/**
	 *	Sets the LOD 'Level' to the given value, properly updating the modules LOD validity settings.
	 */
	virtual void	SetLevelIndex(INT InLevelIndex);


	 /*	
	 *	@param	InModule	The module of interest.
	 *	@return	TRUE		If it is editable for this LOD level.
	 *			FALSE		If it is not.
	 */
	UBOOL	IsModuleEditable(UParticleModule* InModule);
};

struct FParticleBurst
{
	INT Count;
	INT CountLow;
	FLOAT Time;

	/** Constructors */
	FParticleBurst() {}
	FParticleBurst(EEventParm)
	{
		appMemzero(this, sizeof(FParticleBurst));
	}
};



class UParticleEmitter {

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
	struct noRawDistributionFloat SpawnRate;
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

	virtual FParticleEmitterInstance* CreateInstance(UParticleSystemComponent* InComponent);

	virtual void SetToSensibleDefaults() {}

	virtual void PostLoad();
	virtual void UpdateModuleLists();

	void SetEmitterName(FName Name);
	FName& GetEmitterName();
	virtual	void						SetLODCount(INT LODCount);

	// LOD
	INT					CreateLODLevel(INT LODLevel, UBOOL bGenerateModuleData = TRUE);
	UBOOL				IsLODLevelValid(INT LODLevel);

	/** GetCurrentLODLevel
	*	Returns the currently set LODLevel. Intended for game-time usage.
	*	Assumes that the given LODLevel will be in the [0..# LOD levels] range.
	*	
	*	@return NULL if the requested LODLevel is not valid.
	*			The pointer to the requested UParticleLODLevel if valid.
	*/
	inline UParticleLODLevel* GetCurrentLODLevel(FParticleEmitterInstance* Instance)
	{
		// for the game (where we care about perf) we don't branch
		if (GIsGame == TRUE)
		{
			return Instance->CurrentLODLevel;
		}
		else
		{
			//EditorUpdateCurrentLOD( Instance );
			//return Instance->CurrentLODLevel;
			return NULL;
		}

	}

	UParticleLODLevel*	GetLODLevel(INT LODLevel);

	virtual UBOOL		AutogenerateLowestLODLevel(UBOOL bDuplicateHighest = FALSE);

		/**
	 *	CalculateMaxActiveParticleCount
	 *	Determine the maximum active particles that could occur with this emitter.
	 *	This is to avoid reallocation during the life of the emitter.
	 *
	 *	@return	TRUE	if the number was determined
	 *			FALSE	if the number could not be determined
	 */
	virtual UBOOL		CalculateMaxActiveParticleCount();

	/**
	 *	Retrieve the parameters associated with this particle system.
	 *
	 *	@param	ParticleSysParamList	The list of FParticleSysParams used in the system
	 *	@param	ParticleParameterList	The list of ParticleParameter distributions used in the system
	 */
	void GetParametersUtilized(TArray<FString>& ParticleSysParamList,
							   TArray<FString>& ParticleParameterList);
};



class UParticleSpriteEmitter : public UParticleEmitter {

public:
	//## BEGIN PROPS ParticleSpriteEmitter
	//## END PROPS ParticleSpriteEmitter

	//DECLARE_CLASS(UParticleSpriteEmitter,UParticleEmitter,0,Engine)
	virtual void PostLoad();
	//virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual FParticleEmitterInstance* CreateInstance(UParticleSystemComponent* InComponent);
	virtual void SetToSensibleDefaults();
};

struct FParticleCurvePair {
	FString CurveName;
	class UObject* CurveObject;

	/** Constructors */
	FParticleCurvePair()
		: CurveObject(NULL)
	{}
	FParticleCurvePair(EEventParm)
	{
		appMemzero(this, sizeof(FParticleCurvePair));
	}
};

class UParticleModule : public UObject
{
public:
    //## BEGIN PROPS ParticleModule
    BITFIELD bSpawnModule:1;
    BITFIELD bUpdateModule:1;
    BITFIELD bCurvesAsColor:1;
    BITFIELD b3DDrawMode:1;
    BITFIELD bSupported3DDrawMode:1;
    BITFIELD bEnabled:1;
    BITFIELD bEditable:1;
    BITFIELD LODDuplicate:1;
    FColor ModuleEditorColor;
    BYTE LODValidity;
    TArrayNoInit<FName> IdenticalIgnoreProperties;
    //## END PROPS ParticleModule

    //DECLARE_ABSTRACT_CLASS(UParticleModule,UObject,0,Engine)
	/**
	 *	Called on a particle that is freshly spawned by the emitter.
	 *	
	 *	@param	Owner		The FParticleEmitterInstance that spawned the particle.
	 *	@param	Offset		The modules offset into the data payload of the particle.
	 *	@param	SpawnTime	The time of the spawn.
	 */
	virtual void	Spawn(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	/**
	 *	Called on a particle that is being updated by its emitter.
	 *	
	 *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
	 *	@param	Offset		The modules offset into the data payload of the particle.
	 *	@param	DeltaTime	The time since the last update.
	 */
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);

	/**
	 *	Returns the number of bytes that the module requires in the particle payload block.
	 *
	 *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
	 *
	 *	@return	UINT		The number of bytes the module needs per particle.
	 */
	virtual UINT	RequiredBytes(FParticleEmitterInstance* Owner = NULL);
	/**
	 *	Returns the number of bytes the module requires in the emitters 'per-instance' data block.
	 *	
	 *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
	 *
	 *	@return UINT		The number of bytes the module needs per emitter instance.
	 */
	virtual UINT	RequiredBytesPerInstance(FParticleEmitterInstance* Owner = NULL);
	/**
	 *	Allows the module to prep its 'per-instance' data block.
	 *	
	 *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
	 *	@param	InstData	Pointer to the data block for this module.
	 */
	virtual UINT	PrepPerInstanceBlock(FParticleEmitterInstance* Owner, void* InstData);

	// For Cascade
	/**
	 *	Called when the module is created, this function allows for setting values that make
	 *	sense for the type of emitter they are being used in.
	 *
	 *	@param	Owner			The UParticleEmitter that the module is being added to.
	 */
	virtual void SetToSensibleDefaults(UParticleEmitter* Owner);
	
	/** 
	 *	Fill an array with each Object property that fulfills the FCurveEdInterface interface.
	 *
	 *	@param	OutCurve	The array that should be filled in.
	 */
	virtual void	GetCurveObjects(TArray<FParticleCurvePair>& OutCurves);
	/** 
	 *	Add all curve-editable Objects within this module to the curve editor.
	 *
	 *	@param	EdSetup		The CurveEd setup to use for adding curved.
	 */
	//virtual	void	AddModuleCurvesToEditor(UInterpCurveEdSetup* EdSetup);
	/** 
	 *	Remove all curve-editable Objects within this module from the curve editor.
	 *
	 *	@param	EdSetup		The CurveEd setup to remove the curve from.
	 */
	//void	RemoveModuleCurvesFromEditor(UInterpCurveEdSetup* EdSetup);
	/** 
	 *	Does the module contain curves?
	 *
	 *	@return	UBOOL		TRUE if it does, FALSE if not.
	 */
	UBOOL	ModuleHasCurves();
	/** 
	 *	Are the modules curves displayed in the curve editor?
	 *
	 *	@param	EdSetup		The CurveEd setup to check.
	 *
	 *	@return	UBOOL		TRUE if they are, FALSE if not.
	 */
	UBOOL	IsDisplayedInCurveEd(UInterpCurveEdSetup* EdSetup);
	/** 
	 *	Helper function for updating the curve editor when the module editor color changes.
	 *
	 *	@param	Color		The new color the module is using.
	 *	@param	EdSetup		The CurveEd setup for the module.
	 */
	void	ChangeEditorColor(FColor& Color, UInterpCurveEdSetup* EdSetup);

	/** 
	 *	Render the modules 3D visualization helper primitive.
	 *
	 *	@param	Owner		The FParticleEmitterInstance that 'owns' the module.
	 *	@param	View		The scene view that is being rendered.
	 *	@param	PDI			The FPrimitiveDrawInterface to use for rendering.
	 */
	//virtual void Render3DPreview(FParticleEmitterInstance* Owner, const FSceneView* View,FPrimitiveDrawInterface* PDI)	{};

	/**
	 *	Retrieve the ModuleType of this module.
	 *
	 *	@return	EModuleType		The type of module this is.
	 */
	virtual EModuleType	GetModuleType() const	{	return EPMT_General;	}

	/**
	 *	Helper function used by the editor to auto-populate a placed AEmitter with any
	 *	instance parameters that are utilized.
	 *
	 *	@param	PSysComp		The particle system component to be populated.
	 */
	virtual void	AutoPopulateInstanceProperties(UParticleSystemComponent* PSysComp);
	
	/**
	 *	Helper function used by the editor to auto-generate LOD values from a source module
	 *	and a percentage value used to scale its values.
	 *
	 *	@param	SourceModule	The ParticleModule to utilize as the template.
	 *	@param	Percentage		The value to use when scaling the source values.
	 */
	virtual UBOOL	GenerateLODModuleValues(UParticleModule* SourceModule, FLOAT Percentage, UParticleLODLevel* LODLevel);

	/**
	 *	Conversion functions for distributions.
	 *	Used to setup new distributions to a percentage value of the source.
	 */
	/**
	 *	Store the given percentage of the SourceFloat distribution in the FloatDist
	 *
	 *	@param	FloatDist			The distribution to put the result into.
	 *	@param	SourceFloatDist		The distribution of use as the source.
	 *	@param	Percentage			The percentage of the source value to use [0..100]
	 *
	 *	@return	UBOOL				TRUE if successful, FALSE if not.
	 */
	virtual UBOOL	ConvertFloatDistribution(noDistributionFloat* FloatDist, noDistributionFloat* SourceFloatDist, FLOAT Percentage);
	/**
	 *	Store the given percentage of the SourceVector distribution in the VectorDist
	 *
	 *	@param	VectorDist			The distribution to put the result into.
	 *	@param	SourceVectorDist	The distribution of use as the source.
	 *	@param	Percentage			The percentage of the source value to use [0..100]
	 *
	 *	@return	UBOOL				TRUE if successful, FALSE if not.
	 */
	virtual UBOOL	ConvertVectorDistribution(noDistributionVec* VectorDist, noDistributionVec* SourceVectorDist, FLOAT Percentage);
	/**
	 *	Returns whether the module is SizeMultipleLife or not.
	 *
	 *	@return	UBOOL	TRUE if the module is a UParticleModuleSizeMultipleLife
	 *					FALSE if not
	 */
	virtual UBOOL   IsSizeMultiplyLife() { return FALSE; };

	/**
	 *	Comparison routine...
	 *	Intended for editor-use only, this function will return TRUE if the given
	 *	particle module settings are identical to the one the function is called on.
	 *
	 *	@param	InModule	The module to compare against.
	 *
	 *	@return	TRUE		If the modules have all the relevant settings the same.
	 *			FALSE		If they don't.
	 */
	virtual UBOOL	IsIdentical(const UParticleModule* InModule) const;

	/**
	 *	Used by the comparison routine to check for properties that are irrelevant.
	 *
	 *	@param	InPropName	The name of the property being checked.
	 *
	 *	@return	TRUE		If the property is relevant.
	 *			FALSE		If it isn't.
	 */
	virtual UBOOL	PropertyIsRelevantForIsIdentical(const FName& InPropName) const;

	/**
	 *	Generates a new module for LOD levels, setting the values appropriately.
	 *	Note that the module returned could simply be the module it was called on.
	 *
	 *	@param	SourceLODLevel		The source LODLevel
	 *	@param	DestLODLevel		The destination LODLevel
	 *	@param	Percentage			The percentage value that should be used when setting values
	 *
	 *	@return	UParticleModule*	The generated module, or this if percentage == 100.
	 */
	virtual UParticleModule* GenerateLODModule(UParticleLODLevel* SourceLODLevel, UParticleLODLevel* DestLODLevel, FLOAT Percentage, 
		UBOOL bGenerateModuleData, UBOOL bForceModuleConstruction = FALSE);

	/**
	 *	Returns TRUE if the results of LOD generation for the given percentage will result in a 
	 *	duplicate of the module.
	 *
	 *	@param	SourceLODLevel		The source LODLevel
	 *	@param	DestLODLevel		The destination LODLevel
	 *	@param	Percentage			The percentage value that should be used when setting values
	 *
	 *	@return	UBOOL				TRUE if the generated module will be a duplicate.
	 *								FALSE if not.
	 */
	virtual UBOOL WillGeneratedModuleBeIdentical(UParticleLODLevel* SourceLODLevel, UParticleLODLevel* DestLODLevel, FLOAT Percentage)
	{
		// The assumption is that at 100%, ANY module will be identical...
		// (Although this is virtual to allow over-riding that assumption on a case-by-case basis!)

		if (Percentage != 100.0f)
		{
			return LODDuplicate;
		}

		return TRUE;
	}

	/**
	 *	Returns TRUE if the module validiy flags indicate this module is used in the given LOD level.
	 *
	 *	@param	SourceLODIndex		The index of the source LODLevel
	 *
	 *	@return	UBOOL				TRUE if the generated module is used, FALSE if not.
	 */
	virtual UBOOL IsUsedInLODLevel(INT SourceLODIndex) const;

	/**
	 *	Retrieve the ParticleSysParams associated with this module.
	 *
	 *	@param	ParticleSysParamList	The list of FParticleSysParams to add to
	 */
	virtual void GetParticleSysParamsUtilized(TArray<FString>& ParticleSysParamList);

	/**
	 *	Retrieve the distributions that use ParticleParameters in this module.
	 *
	 *	@param	ParticleParameterList	The list of ParticleParameter distributions to add to
	 */
	virtual void GetParticleParametersUtilized(TArray<FString>& ParticleParameterList);
	
	/**
	 *	Refresh the module...
	 */
	virtual void RefreshModule(UInterpCurveEdSetup* EdSetup, UParticleEmitter* InEmitter, INT InLODLevel) {}
};

// Define private default constructor.
#define NO_DEFAULT_CONSTRUCTOR(cls) \
	protected: cls() {} public:

class UParticleModuleAccelerationBase : public UParticleModule
{
public:
	NO_DEFAULT_CONSTRUCTOR(UParticleModuleAccelerationBase)
};

class UParticleModuleAcceleration : public UParticleModuleAccelerationBase
{
public:
	//## BEGIN PROPS ParticleModuleAcceleration
	struct noRawDistributionVec Acceleration;
	BITFIELD bApplyOwnerScale:1;
	//## END PROPS ParticleModuleAcceleration

	//DECLARE_CLASS(UParticleModuleAcceleration,UParticleModuleAccelerationBase,0,Engine)
	virtual void	Spawn(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	virtual UINT	RequiredBytes(FParticleEmitterInstance* Owner = NULL);
};

class UParticleModuleAccelerationOverLifetime : public UParticleModuleAccelerationBase
{
public:
	//## BEGIN PROPS ParticleModuleAccelerationOverLifetime
	struct noRawDistributionVec AccelOverLife;
	//## END PROPS ParticleModuleAccelerationOverLifetime

	//DECLARE_CLASS(UParticleModuleAccelerationOverLifetime,UParticleModuleAccelerationBase,0,Engine)
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
};

class UParticleModuleAttractorBase : public UParticleModule
{
public:
	//## BEGIN PROPS ParticleModuleAttractorBase
	//## END PROPS ParticleModuleAttractorBase

	//DECLARE_ABSTRACT_CLASS(UParticleModuleAttractorBase,UParticleModule,0,Engine)
	NO_DEFAULT_CONSTRUCTOR(UParticleModuleAttractorBase)
};

class UParticleModuleAttractorLine : public UParticleModuleAttractorBase
{
public:
	//## BEGIN PROPS ParticleModuleAttractorLine
	FVector EndPoint0;
	FVector EndPoint1;
	struct noRawDistributionFloat Range;
	struct noRawDistributionFloat Strength;
	//## END PROPS ParticleModuleAttractorLine

	//DECLARE_CLASS(UParticleModuleAttractorLine,UParticleModuleAttractorBase,0,Engine)
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	//virtual void	Render3DPreview(FParticleEmitterInstance* Owner, const FSceneView* View,FPrimitiveDrawInterface* PDI);
};

class UParticleModuleAttractorParticle : public UParticleModuleAttractorBase
{
public:
	//## BEGIN PROPS ParticleModuleAttractorParticle
	FName EmitterName;
	struct FRawDistributionFloat Range;
	BITFIELD bStrengthByDistance:1;
	BITFIELD bAffectBaseVelocity:1;
	BITFIELD bRenewSource:1;
	BITFIELD bInheritSourceVel:1;
	struct FRawDistributionFloat Strength;
	BYTE SelectionMethod;
	INT LastSelIndex;
	//## END PROPS ParticleModuleAttractorParticle

	//DECLARE_CLASS(UParticleModuleAttractorParticle,UParticleModuleAttractorBase,0,Engine)
	virtual void	Spawn(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	virtual UINT	RequiredBytes(FParticleEmitterInstance* Owner = NULL);
};



class UParticleEventBeamBase : public UParticleEvent {

public:
	virtual EModuleType GetModuleType() const { return EPMT_Beam; }
};



struct noBeamModifierOptions {
	UINT32 bModify:1;
	UINT32 bScale:1;
	UINT32 bLock:1;

	noBeamModifierOptions() {}
};

class UParticleEventBeamModifier : public UParticleEventBeamBase {
public:
	BYTE m_modifierType;
	struct noBeamModifierOptions m_positionOptions;
	struct noDistributionVec m_position;
	struct noBeamModifierOptions m_tangentOptions;
	struct noDistributionVec m_tangent;
	UINT32	m_absoluteTangent:1;
	struct noBeamModifierOptions m_strengthOptions;
	struct noRawDistributionFloat m_strength;

	virtual void	Spawn(UParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(UParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);	
};

class UParticleEventBeamNoise : public UParticleEventBeamBase {
public:
	UINT32 m_lowFreqEanbled:1;
	UINT32 m_nrScaledEmitterTime:1;
	UINT32 m_smooth:1;
	UINT32 m_noiseLock:1;
	UINT32 m_oscillate:1;
	UINT32 m_useNoiseTangents:1;
	UINT32 m_tangentNoise:1;
	UINT32 m_applyNoiseScale:1;
	int m_frequency;
	int m_frequency_lowRange;
	struct noRawDistributionVec m_noiseRange;
	struct noRawDistributionFloat m_noiseRangeScale;
	struct noRawDistributionVec m_noiseSpeed;
	FLOAT m_noiseLockRadius;
	FLOAT m_noiseLockTime;
	FLOAT m_noiseTension;
	struct noRawDistributionFloat m_noiseTangentStrength;
	INT m_noiseTessellation;
	FLOAT m_frequencyDistance;
	struct noRawDistributionFloat m_noiseScale;

	virtual void	Spawn(UParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(UParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);	

};

class UParticleEventBeamSource : public UParticleEventBeamBase {

public:
	BYTE m_srcMethod;
	BYTE m_srcTangentMethod;
	FString m_srcName;
	UINT32 m_srcAbsolute:1;
	UINT32 m_lockSrc:1;
	UINT32 m_lockSrcTangent:1;
	UINT32 m_lockSrcStrength:1;
	struct noRawDistributionVec m_src;
	struct noRawDistributionVec m_srcTangent;
	struct noRawDistributionFloat m_srcStrength;

	virtual void	Spawn(UParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(UParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);	



};

class UParticleEventBeamTarget : public UParticleEventBeamBase {
public:
	BYTE m_tgtMethod;
	BYTE m_tgtTangentMethod;
	FString m_tgtName;
	UINT32 m_tgtAbsolute:1;
	UINT32 m_lockTgt:1;
	UINT32 m_lockTgtTangent:1;
	UINT32 m_lockTgtStrength:1;
	struct noRawDistributionVec m_tgt;
	struct noRawDistributionVec m_tgtTangent;
	struct noRawDistributionFloat m_tgtStrength;

	virtual void	Spawn(UParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(UParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);	

};

class UParticleEventDataBase : public UParticleEvent {
public:
	virtual UParticleSpriteEmitterInstance* CreateInstance(UParticleEmitter* In);

	virtual void	PreSpawn(UParticleEmitterInstance* Owner, noBaseParticle* Particle)			{};
	virtual void	PreUpdate(UParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime)		{};
	virtual void	PostUpdate(UParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime)	{};

	virtual EModuleType	GetModuleType() const							{	return EPMT_TypeData;	}
	virtual BOOL		SupportsSpecificScreenAlignmentFlags() const	{	return FALSE;			}
};

class UParticleEventDataBeam : public UParticleEventDataBase {
public:
	BYTE m_beamMethod;
	BYTE m_endPointMethod;
	struct noRawDistributionFloat m_dist;
	struct noRawDistributionVec m_endPoint;
	INT m_tessellationFactor;
	struct noRawDistributionFloat m_emitterStrength;
	struct noRawDistributionFloat m_tgtStrength;
	struct noRawDistributionVec m_endPointDir;
	INT m_txtureTile;
	
	virtual void	Spawn(UParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	PreUpdate(UParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime)		{};
	
	noVec3			DetermineEndPointPosition(UParticleEmitterInstance* Owner, FLOAT DeltaTime);
	noVec3			DetermineParticlePosition(UParticleEmitterInstance* Owner, noBaseParticle* pkParticle, FLOAT DeltaTime);
	
	virtual UParticleEmitterInstance* CreateInstance(UParticleEmitter* InEmitterParent);

};

class UParticleEventDataBeam2 : public UParticleEventDataBase {
public:
	BYTE m_beamMethod;
	BYTE m_taperMethod;
	INT m_txtureTile;
	FLOAT m_textureTileDist;
	INT m_sheets;
	INT m_maxBeamCnt;
	FLOAT m_speed;
	INT m_interpPoints;
	INT m_upVecStepSize;
	FString m_branchParentName;		
	struct noRawDistributionFloat m_dist;
	struct noRawDistributionFloat m_taperFactor;
	struct noRawDistributionFloat m_taperScale;

	virtual void	Spawn(UParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(UParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);	
	virtual void	PreUpdate(UParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime)		{};

	noVec3			DetermineEndPointPosition(UParticleEmitterInstance* Owner, FLOAT DeltaTime);
	noVec3			DetermineParticlePosition(UParticleEmitterInstance* Owner, noBaseParticle* pkParticle, FLOAT DeltaTime);

	virtual UParticleEmitterInstance* CreateInstance(UParticleEmitter* InEmitterParent);
};

class UParticleEventSpawnBase : public UParticleEvent {
public:
	UINT32 m_processSpawnRate:1;
	UINT32 m_processBurstList:1;

		/**
	 *	Retrieve the spawn amount this module is contributing.
	 *	Note that if multiple Spawn-specific modules are present, if any one
	 *	of them ignores the SpawnRate processing it will be ignored.
	 *
	 *	@param	Owner		The particle emitter instance that is spawning.
	 *	@param	Offset		The offset into the particle payload for the module.
	 *	@param	OldLeftover	The bit of timeslice left over from the previous frame.
	 *	@param	DeltaTime	The time that has expired since the last frame.
	 *	@param	Number		The number of particles to spawn. (OUTPUT)
	 *	@param	Rate		The spawn rate of the module. (OUTPUT)
	 *
	 *	@return	UBOOL		FALSE if the SpawnRate should be ignored.
	 *						TRUE if the SpawnRate should still be processed.
	 */
	virtual BOOL GetSpawnAmount(UParticleEmitterInstance* Owner, INT Offset, FLOAT OldLeftover, 
		FLOAT DeltaTime, INT& Number, FLOAT& Rate)
	{
		return m_processSpawnRate;
	}
	
	/**
	 *	Retrieve the burst count this module is contributing.
	 *	Note that if multiple Spawn-specific modules are present, if any one
	 *	of them ignores the default BurstList, it will be ignored.
	 *
	 *	@param	Owner		The particle emitter instance that is spawning.
	 *	@param	Offset		The offset into the particle payload for the module.
	 *	@param	OldLeftover	The bit of timeslice left over from the previous frame.
	 *	@param	DeltaTime	The time that has expired since the last frame.
	 *	@param	Number		The number of particles to burst. (OUTPUT)
	 *
	 *	@return	UBOOL		FALSE if the default BurstList should be ignored.
	 *						TRUE if the default BurstList should still be processed.
	 */
	virtual BOOL GetBurstCount(UParticleEmitterInstance* Owner, INT Offset, FLOAT OldLeftover, 
		FLOAT DeltaTime, INT& Number)
	{
		Number = 0;
		return m_processBurstList;
	}

	
	virtual EModuleType	GetModuleType() const	{	return EPMT_Spawn;	}
};

class UParticleEventSpawn : public UParticleEventSpawnBase {
public:
	struct noRawDistributionFloat m_rate;
	struct noRawDistributionFloat m_rateScale;
	uint8	m_particleBurstMethod;
	TArrayNoInit<struct UParticleBurst> m_burstList;

	
	virtual BOOL GetSpawnAmount(UParticleEmitterInstance* Owner, INT Offset, FLOAT OldLeftover, 
		FLOAT DeltaTime, INT& Number, FLOAT& Rate);

	virtual BOOL	GenerateLODModuleValues(UParticleEvent* SourceEvent, FLOAT Percentage, UParticleLODLevel* LODLevel);
	
};

class UParticleEventSpawnPerUnit : public UParticleEventSpawnBase {
public:
	FLOAT m_unitScalar;
	struct noRawDistributionFloat m_spawnPerUnit;
	UINT32 m_ignoreSpawnRateWhenMoving:1;
	FLOAT m_movementTolerance;

	virtual void	Spawn(UParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual BOOL GetSpawnAmount(UParticleEmitterInstance* Owner, INT Offset, FLOAT OldLeftover, 
		FLOAT DeltaTime, INT& Number, FLOAT& Rate);
};

#endif 