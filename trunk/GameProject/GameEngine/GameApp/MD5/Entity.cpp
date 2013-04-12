// Entity.cpp:
// Created by: Aurelio Reis

#include "stdafx.h"

#include "Entity.h"
#include "DXUT/Optional/DXUTcamera.h"

#include "Tracer.h"
#include "BoundingVolume.h"
#include "MD5Model.h"
#include "MD5Anim.h"


CArEntity *CArEntity::s_pHead = NULL;
int CArEntity::s_iNumEntities = 0;


//////////////////////////////////////////////////////////////////////////
// CArEntity::CArEntity()
//////////////////////////////////////////////////////////////////////////

CArEntity::CArEntity() :
	m_pNext( NULL ),
	m_vOrigin( 0.0f, 0.0f, 0.0f ), m_vVelocity( 0.0f, 0.0f, 0.0f ),
	m_pModel( NULL ), m_pAnimator( NULL ),
	m_bDismembered( false )
{
	m_Angles.Zero();
}

//////////////////////////////////////////////////////////////////////////
// CArEntity::~CArEntity()
//////////////////////////////////////////////////////////////////////////

CArEntity::~CArEntity()
{
	SAFE_DELETE( m_pModel );
	SAFE_DELETE( m_pAnimator );
}

//////////////////////////////////////////////////////////////////////////
// CArEntity::Initialize
//////////////////////////////////////////////////////////////////////////

void CArEntity::Initialize( IArModelFile *pModelFile )
{
	m_pModel = new CArModelInstance();
	m_pModel->InitializeFromModelFile( pModelFile );

	m_pAnimator = new CArAnimator();
	m_pAnimator->Initialize( &m_pModel->m_pModelFile->GetSkeleton() );
}

//////////////////////////////////////////////////////////////////////////
// CArEntity::Update
//////////////////////////////////////////////////////////////////////////

void CArEntity::Update()
{
	//////////////////////////////////////////////////////////////////////////
	// Crude physics simulation.
	//////////////////////////////////////////////////////////////////////////

#if 1
	if ( m_vOrigin.y > 0.0f && m_pAnimator->m_pDismemberedDz )
	{
		D3DXVECTOR3 vAccel = D3DXVECTOR3( 0.0f, -185.0f, 0.0f );
		
		m_vVelocity += vAccel * g_fElapsedTime;
		m_vOrigin += m_vVelocity * g_fElapsedTime;

		if ( m_vOrigin.y < 5.0f )
		{
			m_vOrigin.y = 0.0f;
		}
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// Test collision of the tracers against the model hitboxes.
	//////////////////////////////////////////////////////////////////////////

	if ( m_bDismembered )
	{
		return;
	}

	vector< CArDamageZone * >::iterator iterHbox;
	iterHbox = m_pModel->m_pModelFile->GetBaseModel()->m_DamageZones.begin();
	for ( ; iterHbox != m_pModel->m_pModelFile->GetBaseModel()->m_DamageZones.end(); ++iterHbox )
	{
		const CArDamageZone &Dz = **iterHbox;

		if ( Dz.m_iAttachmentJoint == -1 )
		{
			continue;
		}

		D3DXMATRIXA16 &Joint = m_pAnimator->m_pJointTransformsSkel[ Dz.m_iAttachmentJoint ];
		D3DXVECTOR3 c( Joint._41, Joint._42, Joint._43 );
		float r = Dz.m_Box.CalculateRadius();

		CArSphere Sphere( c, r );

		CArTracer *pCurTracer = CArTracer::s_pHead;
		for ( ; pCurTracer != NULL; pCurTracer = pCurTracer->m_pNext )
		{
			// Only do a hit with the initial trace.
			if ( pCurTracer->m_uiBirthFrame != g_uiFrameNum )
			{
				continue;
			}

			CArRay Ray;
			Ray.m_vOrigin = pCurTracer->m_vStart;
			Ray.m_vDirection = pCurTracer->m_vEnd - pCurTracer->m_vStart;
			D3DXVec3Normalize( &Ray.m_vDirection, &Ray.m_vDirection );

			if ( Sphere.IntersectRay( Ray ) )
			{
				// NOTE: To allow multiple dismemberment we would have to keep track of all transforms for each
				// child damage zone for a given limb so that if a damage zone was dismembered and then it's parent
				// was as well, the parent dismembered part would have the proper joints flattened.
				m_bDismembered = true;

				SMD5Joint &NullJoint = m_pAnimator->m_pBlendedJoints[ Dz.m_iAttachmentJoint ];
				NullJoint.m_fScale = 0.0f;

				CreateDismemberedLimbEntity( Dz, Ray );

				return;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// CArEntity::Draw
//////////////////////////////////////////////////////////////////////////

void CArEntity::Draw()
{
	//////////////////////////////////////////////////////////////////////////
	// Setup the entity transform (euler to matrix).
	//////////////////////////////////////////////////////////////////////////

	D3DXMATRIX TransformMatrix;
	D3DXMatrixRotationYawPitchRoll( &TransformMatrix, m_Angles.m_fYaw, m_Angles.m_fPitch, m_Angles.m_fRoll );

	TransformMatrix._41 = m_vOrigin.x;
	TransformMatrix._42 = m_vOrigin.y;
	TransformMatrix._43 = m_vOrigin.z;

	//////////////////////////////////////////////////////////////////////////
	// Render the model.
	//////////////////////////////////////////////////////////////////////////

	m_pModel->Render( TransformMatrix, *m_pAnimator );

	//////////////////////////////////////////////////////////////////////////
	// Show damage zones.
	//////////////////////////////////////////////////////////////////////////

	if ( g_bDrawDamageZones )
	{
		DrawDamageZones( m_pModel->m_pModelFile->GetBaseModel()->m_DamageZones, *m_pAnimator );
	}
}

//////////////////////////////////////////////////////////////////////////
// CArEntity::CreateDismemberedLimbEntity
//////////////////////////////////////////////////////////////////////////

void CArEntity::CreateDismemberedLimbEntity( const CArDamageZone &Dz, const CArRay &Ray )
{
	const D3DXMATRIXA16 &JointMatrix = m_pAnimator->m_pJointTransformsSkel[ Dz.m_iAttachmentJoint ];

	// Create the dismembered object.
	CArEntity *pObj = new CArEntity();
	CArEntity::AddEntity( pObj );

	//CArEntity::SAngles Angles;
	//Angles.MatrixToAngles( JointMatrix );

	pObj->Initialize( m_pModel->m_pModelFile );
	pObj->SetOrigin( D3DXVECTOR3( JointMatrix._41, JointMatrix._42, JointMatrix._43 ) );
	//pObj->SetAngles( Angles );

	// Bias it up a bit.
	D3DXVECTOR3 vDir = Ray.m_vDirection * 100.0f;
	vDir.y += 50.0f;

	// Apply an impulse in the hit direction.
	pObj->m_vVelocity += vDir;

	pObj->m_pAnimator->m_pAnimFile = m_pAnimator->m_pAnimFile;
	pObj->m_pAnimator->SetFrame( m_pAnimator->GetFrame() );

	pObj->m_bDismembered = true;
	pObj->m_pAnimator->m_pDismemberedDz = &Dz;

	pObj->m_pAnimator->Pause();
	pObj->m_pAnimator->Update( &m_pModel->m_pModelFile->GetSkeleton() );

	pObj->Update();
	pObj->Draw();
}

//////////////////////////////////////////////////////////////////////////
// CArEntity::AddEntity
//////////////////////////////////////////////////////////////////////////

void CArEntity::AddEntity( CArEntity *pEntity )
{
	pEntity->m_pNext = CArEntity::s_pHead;
	CArEntity::s_pHead = pEntity;
}

//////////////////////////////////////////////////////////////////////////
// CArEntity::PresentEntities
//////////////////////////////////////////////////////////////////////////

void CArEntity::PresentEntities()
{
	CArEntity *pEnt = CArEntity::s_pHead;
	for ( ; pEnt != NULL; pEnt = pEnt->m_pNext )
	{
		pEnt->Update();
		pEnt->Draw();
	}
}

//////////////////////////////////////////////////////////////////////////
// CArEntity::DeleteEntities
//////////////////////////////////////////////////////////////////////////

void CArEntity::DeleteEntities()
{
	CArEntity *pEnt = CArEntity::s_pHead;
	CArEntity *pNextEnt = NULL;

	for ( ; pEnt != NULL; pEnt = pNextEnt )
	{
		pNextEnt = pEnt->m_pNext;

		SAFE_DELETE( pEnt );
	}

	CArEntity::s_pHead = NULL;
}

//////////////////////////////////////////////////////////////////////////
// CArEntity::PlayAnimation
//////////////////////////////////////////////////////////////////////////

void CArEntity::PlayAnimation( CArMD5Anim *pAnim, bool bLoop )
{
	m_pAnimator->m_pAnimFile = pAnim;
	m_pAnimator->m_bLoop = bLoop;
}

//////////////////////////////////////////////////////////////////////////
// CArEntity::DrawDamageZones
//////////////////////////////////////////////////////////////////////////

void CArEntity::DrawDamageZones( vector< CArDamageZone * > &DamageZones, const CArAnimator &Animator )
{
	// Don't bother with debug drawing if this is a dismembered limb.
	if ( Animator.m_pDismemberedDz )
	{
		return;
	}

	vector< CArDamageZone * >::iterator iterDz;
	iterDz = DamageZones.begin();
	for ( ; iterDz != DamageZones.end(); ++iterDz )
	{
		const CArDamageZone &Dz = **iterDz;

		if ( Dz.m_iAttachmentJoint == -1 )
		{
			continue;
		}

		D3DXMATRIXA16 &JointMat = Animator.m_pJointTransformsSkel[ Dz.m_iAttachmentJoint ];
		DrawAabbWireframe( Dz.m_Box, &JointMat, g_Colors[ Dz.m_iAttachmentJoint % MAX_COLORS ] );

		DrawAxis( 5.0f, JointMat, true );
	}
}

//////////////////////////////////////////////////////////////////////////
// CArEntity::SAngles::MatrixToAngles
//////////////////////////////////////////////////////////////////////////

void CArEntity::SAngles::MatrixToAngles( const D3DXMATRIXA16 &JointMatrix )
{
	m_fPitch = asinf( -JointMatrix._32 ); 

	if ( cosf( m_fPitch ) > FLT_EPSILON )
	{ 
		m_fRoll = atan2f( JointMatrix._12, JointMatrix._22 );
		m_fYaw = atan2f( JointMatrix._31, JointMatrix._33 ); 
	}
	else
	{ 
		m_fRoll = atan2f( -JointMatrix._21, JointMatrix._11 ); 
		m_fYaw = 0.0; 
	}
}