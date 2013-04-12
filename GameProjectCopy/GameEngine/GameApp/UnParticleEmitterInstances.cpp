#include "stdafx.h"
#include "UnParticleEmitterInstances.h"


noParticleEmitterInstanceType noParticleEmitterInstance::StaticType;

const float noParticleEmitterInstance::PeakActiveParticleUpdateDelta;

noParticleEmitterInstance::noParticleEmitterInstance()
{

}

noParticleEmitterInstance::~noParticleEmitterInstance()
{

}

void noParticleEmitterInstance::InitParameters( noParticleEmitter* InTemplate, BOOL bClearResources /*= TRUE*/ )
{
	SpriteTemplate = static_cast<noParticleSpriteEmitter*>(InTemplate);
	SetupEmitterDuration();
}

void noParticleEmitterInstance::Init()
{

}

BOOL noParticleEmitterInstance::Resize( INT NewMaxActiveParticles, BOOL bSetMaxActiveCount /*= TRUE*/ )
{

}

void noParticleEmitterInstance::Update( FLOAT DeltaTime, BOOL bSuppressSpawning )
{

}

BYTE* noParticleEmitterInstance::GetModuleInstanceData( noParticleEvent* event )
{

}

UINT noParticleEmitterInstance::CalculateParticleStride( UINT ParticleSize )
{

}

void noParticleEmitterInstance::ResetBurstList()
{

}

FLOAT noParticleEmitterInstance::GetCurrentBurstRateOffset( FLOAT& DeltaTime, INT& Burst )
{

}

void noParticleEmitterInstance::ResetParticleParameters( FLOAT DeltaTime, DWORD StatId )
{

}

void noParticleEmitterInstance::CalculateOrbitOffset( FOrbitChainModuleInstancePayload& Payload, FVector& AccumOffset, FVector& AccumRotation, FVector& AccumRotationRate, FLOAT DeltaTime, FVector& Result, FMatrix& RotationMat, TArray<FVector>& Offsets, UBOOL bStoreResult /*= TRUE*/ )
{

}

void noParticleEmitterInstance::UpdateOrbitData( FLOAT DeltaTime )
{

}

void noParticleEmitterInstance::ParticlePrefetch()
{

}

FLOAT noParticleEmitterInstance::Spawn( FLOAT DeltaTime )
{

}

FLOAT noParticleEmitterInstance::Spawn( FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst /*= 0*/, FLOAT BurstTime /*= 0.0f*/ )
{

}
void noParticleEmitterInstance::ForceSpawn( FLOAT DeltaTime, INT InSpawnCount, INT InBurstCount, FVector& InLocation, FVector& InVelocity )
{

}

void noParticleEmitterInstance::PreSpawn( FBaseParticle* Particle )
{

}

BOOL noParticleEmitterInstance::HasCompleted()
{

}

void noParticleEmitterInstance::PostSpawn( noBaseParticle* Particle, FLOAT InterpolationPercentage, FLOAT SpawnTime )
{

}

void noParticleEmitterInstance::KillParticles()
{

}

void noParticleEmitterInstance::KillParticle( INT Index )
{

}

void noParticleEmitterInstance::KillParticlesForced()
{

}

void noParticleEmitterInstance::RemovedFromScene()
{

}

noBaseParticle* noParticleEmitterInstance::GetParticle( INT Index )
{

}

void noParticleEmitterInstance::SetupEmitterDuration()
{
	if (SpriteTemplate == NULL)	
		return;

	int EDCount = EmitterDurations.size();
	if ((EDCount == 0) )//|| (EDCount != SpriteTemplate->))
	{
		EmitterDurations.clear();
		EmitterDurations.resize(1);
		EmitterDurations[0] = 1;
	}


	

}

void noParticleEmitterInstance::ProcessEvents( FLOAT DeltaTime, UBOOL bSuppressSpawning )
{

}

