#ifndef EFFECT_PARTICLESYSTEM_H
#define EFFECT_PARTICLESYSTEM_H

//#include <Math/Vector.h>
//#include <Math/Quaternion.h>
#include <vector>
#include <string>
#include <Framework/noObject.h>
#include <Math/Distribution.h>



class noViewport3D;

class noParticleEmitterInstance;
class noBaseParticle;

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

class noParticleSystem 
{
	std::vector<class noParticleEmitter*> m_emitters;

};

class noParticleLODLevel : public noObject {
public:
	int m_level;
	int m_levelSetting;
	UINT32 m_enabled:1;
	UINT32 m_convertedEvents:1;
	std::vector<class noParticleEvent*> m_events;
	class noParticleEvent* m_dataEvent;
	class noParticleEventSpawn* m_spawnEvent;
	class noParticleEventGenerator* m_eventGenerator;
	std::vector<class noParticleEventSpawnBase*> m_spawningEvents;
	std::vector<class noParticleEvent*> m_spawnEvents;
	std::vector<class noParticleEvent*> m_updateEvents;
	INT m_peakActiveParticles;

	virtual void	UpdateEventLists();

	virtual BOOL	GenerateFromLODLevel(noParticleLODLevel* SourceLODLevel, FLOAT Percentage, BOOL bGenerateModuleData = TRUE);

	virtual INT	CalculateMaxActiveParticleCount();

	virtual void	SetLevelIndex(INT InLevelIndex);

};

class noParticleEmitter {

public:
	std::string m_name;
	struct noRawDistributionFloat m_spawnRate;
	float m_emitterDuration;
	int	m_emitterLoops;
	BYTE m_particleBurstMethod;
	BYTE m_interpolationMethod;
	BYTE m_EmitterRenderMode;
	std::vector<struct noParticleBurst> m_burstList;
	int m_subImages_Horizontal;
	int m_subImages_Vertical;
	float m_randomImageTime;
	int m_randomImageChanges;
	int m_subUVdataOffset;
	std::vector<class noParticleLODLevel*> m_lodLevels;	
	std::vector<class noParticleEvent*> m_events;
	std::vector<class noParticleEvent*> m_spawnEvents;
	std::vector<class noParticleEvent*> m_updateEvents;	
	int m_peakActiveParticles;
	int m_initialAllocationCount;


	noParticleLODLevel*	GetLODLevel(INT LODLevel);



};


class noParticleSpriteEmitter : public noParticleEmitter {

};

struct noParticleCurvePair {
	std::string curveName;
};

class noParticleEvent {
public:
	uint32 m_isSpawn:1;
	uint32 m_isUpdate:1;
	uint32 m_curveAsColor:1;
	uint32 m_3dDrawMode:1;
	uint32 m_enabled:1;
	uint32 m_editable:1;
	uint32 m_lodDuplicate:1;
	//uint8	m_lodValidty; 

	virtual void	Spawn(noParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	/**
	 *	Called on a particle that is being updated by its emitter.
	 *	
	 *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
	 *	@param	Offset		The modules offset into the data payload of the particle.
	 *	@param	DeltaTime	The time since the last update.
	 */
	virtual void	Update(noParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);


	virtual void Render3DPreview(noParticleEmitterInstance* Owner, const noViewport3D* ,FPrimitiveDrawInterface* PDI)	{};

};

class noParticleEventBeamBase : public noParticleEvent {

public:
	virtual EModuleType GetModuleType() const { return EPMT_Beam; }
};

struct noBeamModifierOptions {
	UINT32 bModify:1;
	UINT32 bScale:1;
	UINT32 bLock:1;

	noBeamModifierOptions() {}
};

class noParticleEventBeamModifier : public noParticleEventBeamBase {
public:
	BYTE m_modifierType;
	struct noBeamModifierOptions m_positionOptions;
	struct noDistributionVec m_position;
	struct noBeamModifierOptions m_tangentOptions;
	struct noDistributionVec m_tangent;
	UINT32	m_absoluteTangent:1;
	struct noBeamModifierOptions m_strengthOptions;
	struct noRawDistributionFloat m_strength;

	virtual void	Spawn(noParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(noParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);	
};

class noParticleEventBeamNoise : public noParticleEventBeamBase {
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

	virtual void	Spawn(noParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(noParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);	

};

class noParticleEventBeamSource : public noParticleEventBeamBase {

public:
	BYTE m_srcMethod;
	BYTE m_srcTangentMethod;
	std::string m_srcName;
	UINT32 m_srcAbsolute:1;
	UINT32 m_lockSrc:1;
	UINT32 m_lockSrcTangent:1;
	UINT32 m_lockSrcStrength:1;
	struct noRawDistributionVec m_src;
	struct noRawDistributionVec m_srcTangent;
	struct noRawDistributionFloat m_srcStrength;

	virtual void	Spawn(noParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(noParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);	



};

class noParticleEventBeamTarget : public noParticleEventBeamBase {
public:
	BYTE m_tgtMethod;
	BYTE m_tgtTangentMethod;
	std::string m_tgtName;
	UINT32 m_tgtAbsolute:1;
	UINT32 m_lockTgt:1;
	UINT32 m_lockTgtTangent:1;
	UINT32 m_lockTgtStrength:1;
	struct noRawDistributionVec m_tgt;
	struct noRawDistributionVec m_tgtTangent;
	struct noRawDistributionFloat m_tgtStrength;

	virtual void	Spawn(noParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(noParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);	

};

class noParticleEventDataBase : public noParticleEvent {
public:
	virtual noParticleSpriteEmitterInstance* CreateInstance(noParticleEmitter* In);

	virtual void	PreSpawn(noParticleEmitterInstance* Owner, noBaseParticle* Particle)			{};
	virtual void	PreUpdate(noParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime)		{};
	virtual void	PostUpdate(noParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime)	{};

	virtual EModuleType	GetModuleType() const							{	return EPMT_TypeData;	}
	virtual BOOL		SupportsSpecificScreenAlignmentFlags() const	{	return FALSE;			}
};

class noParticleEventDataBeam : public noParticleEventDataBase {
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
	
	virtual void	Spawn(noParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	PreUpdate(noParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime)		{};
	
	noVec3			DetermineEndPointPosition(noParticleEmitterInstance* Owner, FLOAT DeltaTime);
	noVec3			DetermineParticlePosition(noParticleEmitterInstance* Owner, noBaseParticle* pkParticle, FLOAT DeltaTime);
	
	virtual noParticleEmitterInstance* CreateInstance(noParticleEmitter* InEmitterParent);

};

class noParticleEventDataBeam2 : public noParticleEventDataBase {
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
	std::string m_branchParentName;		
	struct noRawDistributionFloat m_dist;
	struct noRawDistributionFloat m_taperFactor;
	struct noRawDistributionFloat m_taperScale;

	virtual void	Spawn(noParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(noParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);	
	virtual void	PreUpdate(noParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime)		{};

	noVec3			DetermineEndPointPosition(noParticleEmitterInstance* Owner, FLOAT DeltaTime);
	noVec3			DetermineParticlePosition(noParticleEmitterInstance* Owner, noBaseParticle* pkParticle, FLOAT DeltaTime);

	virtual noParticleEmitterInstance* CreateInstance(noParticleEmitter* InEmitterParent);
};

class noParticleEventSpawnBase : public noParticleEvent {
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
	virtual BOOL GetSpawnAmount(noParticleEmitterInstance* Owner, INT Offset, FLOAT OldLeftover, 
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
	virtual BOOL GetBurstCount(noParticleEmitterInstance* Owner, INT Offset, FLOAT OldLeftover, 
		FLOAT DeltaTime, INT& Number)
	{
		Number = 0;
		return m_processBurstList;
	}

	
	virtual EModuleType	GetModuleType() const	{	return EPMT_Spawn;	}
};

class noParticleEventSpawn : public noParticleEventSpawnBase {
public:
	struct noRawDistributionFloat m_rate;
	struct noRawDistributionFloat m_rateScale;
	uint8	m_particleBurstMethod;
	std::vector<struct noParticleBurst> m_burstList;

	
	virtual BOOL GetSpawnAmount(noParticleEmitterInstance* Owner, INT Offset, FLOAT OldLeftover, 
		FLOAT DeltaTime, INT& Number, FLOAT& Rate);

	virtual BOOL	GenerateLODModuleValues(noParticleEvent* SourceEvent, FLOAT Percentage, noParticleLODLevel* LODLevel);
	
};

class noParticleEventSpawnPerUnit : public noParticleEventSpawnBase {
public:
	FLOAT m_unitScalar;
	struct noRawDistributionFloat m_spawnPerUnit;
	UINT32 m_ignoreSpawnRateWhenMoving:1;
	FLOAT m_movementTolerance;

	virtual void	Spawn(noParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual BOOL GetSpawnAmount(noParticleEmitterInstance* Owner, INT Offset, FLOAT OldLeftover, 
		FLOAT DeltaTime, INT& Number, FLOAT& Rate);
};

#endif 