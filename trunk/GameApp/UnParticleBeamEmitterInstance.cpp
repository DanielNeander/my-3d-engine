#include "stdafx.h"
#include "UnParticleUtil.h"
#include "UnParticleSystem.h"
#include "UnParticleEmitterInstances.h"



noParticleBeamEmitterInstance::noParticleBeamEmitterInstance()
{

}

noParticleBeamEmitterInstance::~noParticleBeamEmitterInstance()
{

}

void noParticleBeamEmitterInstance::SetBeamType( INT NewMethod )
{

}

void noParticleBeamEmitterInstance::SetTessellationFactor( FLOAT NewFactor )
{

}

void noParticleBeamEmitterInstance::SetEndPoint( noVec3 NewEndPoint )
{
	if (m_userSetTargetArray.size() < 1)
		m_userSetTargetArray.push_back(NewEndPoint);
	else 
		m_userSetTargetArray[0] = NewEndPoint;
}

void noParticleBeamEmitterInstance::SetDistance( FLOAT Distance )
{

}

void noParticleBeamEmitterInstance::SetSourcePoint( noVec3 NewSourcePoint,INT SourceIndex )
{
	if (SourceIndex < 0) 
		return;
	if (m_userSetSrcArray.size() < (SourceIndex + 1)) 
	{		
		m_userSetSrcArray.resize(SourceIndex + 1);
	}
	m_userSetSrcArray[SourceIndex] = NewSourcePoint;
}

void noParticleBeamEmitterInstance::SetSourceTangent( noVec3 NewTangentPoint,INT SourceIndex )
{
	if (SourceIndex < 0) 
		return;
	if (m_userSetSrcTangentArray.size() < (SourceIndex + 1)) 
	{		
		m_userSetSrcTangentArray.resize(SourceIndex + 1);
	}
	m_userSetSrcTangentArray[SourceIndex] = NewTangentPoint;
}

void noParticleBeamEmitterInstance::SetSourceStrength( FLOAT NewSourceStrength,INT SourceIndex )
{
	if (SourceIndex < 0) 
		return;
	if (m_userSetSrcStrenthArray.size() < (SourceIndex + 1)) 
	{		
		m_userSetSrcStrenthArray.resize(SourceIndex + 1);
	}
	m_userSetSrcStrenthArray[SourceIndex] = NewSourceStrength;
}

void noParticleBeamEmitterInstance::SetTargetPoint( noVec3 NewTargetPoint,INT TargetIndex )
{
	if (TargetIndex < 0) 
		return;

	if (m_userSetTargetArray.size() < (TargetIndex + 1))
	{
		m_userSetTargetArray.resize(TargetIndex + 1);
	}
	m_userSetTargetArray[TargetIndex] = NewTargetPoint;
}

void noParticleBeamEmitterInstance::SetTargetTangent( noVec3 NewTangentPoint,INT TargetIndex )
{
	if (TargetIndex < 0) 
		return;
	if (m_userSetTargetTangentArray.size() < (TargetIndex + 1)) 
	{		
		m_userSetTargetTangentArray.resize(TargetIndex + 1);
	}
	m_userSetTargetTangentArray[TargetIndex] = NewTangentPoint;
}

void noParticleBeamEmitterInstance::SetTargetStrength( FLOAT NewTargetStrength,INT TargetIndex )
{
	if (TargetIndex < 0) 
		return;
	if (m_userSetTargetStrenthArray.size() < (TargetIndex + 1)) 
	{		
		m_userSetTargetStrenthArray.resize(TargetIndex + 1);
	}
	m_userSetTargetStrenthArray[TargetIndex] = NewTargetStrength;

}

void noParticleBeamEmitterInstance::InitParameters( noParticleEmitter* InTemplate, BOOL bClearResources /*= TRUE*/ )
{
	noParticleEmitterInstance::InitParameters(InTemplate, bClearResources);

	noParticleLODLevel* lodLevel = InTemplate->GetLODLevel(0);
	ASSERT(lodLevel);
	m_beamData = static_cast<noParticleEventDataBeam2*>(lodLevel->m_dataEvent);

	m_beamEventSrc = NULL;
	m_beamEventTgt = NULL;
	m_beamEventNoise = NULL;
	m_beamEventSrcModifier = NULL;
	m_beamEventTgtModifier = NULL;

	if (m_beamData->m_maxBeamCnt == 0)
		m_beamData->m_maxBeamCnt = 1;

	m_beamCnt						=	 m_beamData->m_maxBeamCnt;
	m_firstEmission					= TRUE;
	m_lastEmittedParticleIndex		= -1;
	m_tickCnt						= 0;
	
	m_beamMethod					= m_beamData->m_beamMethod;

	m_textureTiles.clear();
	m_textureTiles.push_back(m_beamData->m_txtureTile);

	m_userSetSrcStrenthArray.clear();
	m_userSetSrcTangentArray.clear();
	m_userSetSrcStrenthArray.clear();
	m_distArray.clear();
	m_targetPointArray.clear();
	m_targetPointSrcNames.clear();
	m_userSetTargetArray.clear();
	m_userSetTargetTangentArray.clear();
	m_userSetTargetStrenthArray.clear();



}

void noParticleBeamEmitterInstance::Init()
{
	SetupBeamEvents();
	noParticleEmitterInstance::Init();
	SetupBeamModifierEvents();
}

void noParticleBeamEmitterInstance::Update( FLOAT DeltaTime, BOOL bSuppressSpawning )
{

}

FLOAT noParticleBeamEmitterInstance::Spawn( FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst /*= 0*/, FLOAT BurstTime /*= 0.0f*/ )
{

}

void noParticleBeamEmitterInstance::PreSpawn( noBaseParticle* Particle )
{

}

void noParticleBeamEmitterInstance::SetupBeamEvents()
{

}

void noParticleBeamEmitterInstance::SetupBeamModifierEvents()
{

}