// Animator.cpp:
// Created by: Aurelio Reis

#include "stdafx.h"

#include "Animator.h"
#include "BoundingVolume.h"
#include "DXUT/Optional/DXUTcamera.h"

#include <string>
#include <iostream>


//////////////////////////////////////////////////////////////////////////
// CArAnimator::CArAnimator
//////////////////////////////////////////////////////////////////////////

CArAnimator::CArAnimator() : 
	m_pAnimFile( NULL ), m_bLoop( false ),
	m_fCurAnimFrame( 0.0f ), m_fAnimScale( 1.0f ),
	m_pDismemberedDz( NULL )
{

}

//////////////////////////////////////////////////////////////////////////
// CArAnimator::~CArAnimator
//////////////////////////////////////////////////////////////////////////

CArAnimator::~CArAnimator()
{

}

//////////////////////////////////////////////////////////////////////////
// CArAnimator::Initialize
//////////////////////////////////////////////////////////////////////////

void CArAnimator::Initialize( const SMD5Skeleton *pSkel )
{
	m_pBlendedJoints = new SMD5Joint[ pSkel->m_iNumJoints ];
	m_pJointTransforms = new D3DXMATRIXA16[ pSkel->m_iNumJoints ];
	m_pJointTransformsSkel = new D3DXMATRIXA16[ pSkel->m_iNumJoints ];
	m_iNumTransforms = pSkel->m_iNumJoints;

	for ( int i = 0; i < pSkel->m_iNumJoints; ++i )
	{
		D3DXMatrixIdentity( &m_pJointTransformsSkel[ i ] );
		D3DXMatrixIdentity( &m_pJointTransforms[ i ] );
	}
}


//////////////////////////////////////////////////////////////////////////
// CArAnimator::Destroy
//////////////////////////////////////////////////////////////////////////

void CArAnimator::Destroy()
{
	SAFE_DELETE_ARRAY( m_pBlendedJoints );
	SAFE_DELETE_ARRAY( m_pJointTransforms );
	SAFE_DELETE_ARRAY( m_pJointTransformsSkel );
}

//////////////////////////////////////////////////////////////////////////
// CArAnimator::Update
//////////////////////////////////////////////////////////////////////////

void CArAnimator::Update( const SMD5Skeleton *pSkel )
{
	if ( !m_pAnimFile /*|| m_fAnimScale < FLT_MIN*/ )
	{
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Update frame timers.
	//////////////////////////////////////////////////////////////////////////

	m_fCurAnimFrame += g_fElapsedTime * (float)m_pAnimFile->m_iFrameRate * g_fGlobalAnimScale * m_fAnimScale;

	int iCurFrame = int( m_fCurAnimFrame ) % m_pAnimFile->m_iNumFrames;
	int iNxtFrame = int( m_fCurAnimFrame + 1 ) % m_pAnimFile->m_iNumFrames;

	// The blend fraction; how in-between frames we are.
	float fBlendFrac = m_fCurAnimFrame - (int)m_fCurAnimFrame;

	//////////////////////////////////////////////////////////////////////////
	// Build the animated skeleton and blend between current and next frame.
	//////////////////////////////////////////////////////////////////////////

	for ( int i = 0; i < pSkel->m_iNumJoints; ++i )
	{
		const SMD5Joint &BaseFrameJoint = m_pAnimFile->m_BaseFrame.m_pJoints[ i ];
		const SMD5HierarchyJoint &HierarchyJoint = m_pAnimFile->m_AnimHierarchy.m_pJoints[ i ];

		const SMD5AnimFrame &CurFrameData = m_pAnimFile->g_pAnimFrames[ iCurFrame ];
		const SMD5AnimFrame &NxtFrameData = m_pAnimFile->g_pAnimFrames[ iNxtFrame ];

		const SMD5Joint *pCurFrameJoint;
		const SMD5Joint *pNxtFrameJoint;

		// Modified for this animation?
		if ( HierarchyJoint.m_iFlags != 0 )
		{
			// Use animated frame data.
			pCurFrameJoint = CurFrameData.m_ppModifiedJoints[ i ];
			pNxtFrameJoint = NxtFrameData.m_ppModifiedJoints[ i ];
		}
		else
		{
			// Use base frame data.
			pCurFrameJoint = &BaseFrameJoint;
			pNxtFrameJoint = &BaseFrameJoint;
		}

		SMD5Joint &BlendJoint = m_pBlendedJoints[ i ];

		// NOTE: Scale may be modified prior to this point so do not overwrite values.

		// Blend the joints.
		BlendJoint.m_Position = pCurFrameJoint->m_Position + fBlendFrac * ( pNxtFrameJoint->m_Position - pCurFrameJoint->m_Position );
		D3DXQuaternionSlerp( &BlendJoint.m_Orientation, &pCurFrameJoint->m_Orientation, &pNxtFrameJoint->m_Orientation, fBlendFrac );

		BlendJoint.m_iParentIndex = HierarchyJoint.m_iParentJoint;
	}

	//////////////////////////////////////////////////////////////////////////
	// Accumulate transforms.
	//////////////////////////////////////////////////////////////////////////

	// Initialize root joint.
	// NOTE: Since joints are ordered sequentially we can reuse the blended joints array.
	//m_pBlendedJoints[ 0 ];

	// TEMP: Disable root motion.
	m_pBlendedJoints[ 0 ].m_Position = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

	// Transform the skeleton.
	for ( int i = 1; i < pSkel->m_iNumJoints; ++i )
	{
		SMD5Joint &OutJoint = m_pBlendedJoints[ i ];
		const SMD5Joint &InJoint = m_pBlendedJoints[ i ];
		const SMD5Joint &ParentJoint = m_pBlendedJoints[ InJoint.m_iParentIndex ];

		// Rotate the animated position and add to parents position.
		D3DXVECTOR3 vRotatedPos;
		D3DXMATRIXA16 mTransform;
		D3DXMatrixRotationQuaternion( &mTransform, &ParentJoint.m_Orientation );
		D3DXVec3TransformCoord( &vRotatedPos, &InJoint.m_Position, &mTransform );
		OutJoint.m_Position = vRotatedPos + ParentJoint.m_Position;

		// Concatenate rotations.
		D3DXQuaternionMultiply( &OutJoint.m_Orientation, &InJoint.m_Orientation, &ParentJoint.m_Orientation );
		D3DXQuaternionNormalize( &OutJoint.m_Orientation, &OutJoint.m_Orientation );

		OutJoint.m_fScale *= ParentJoint.m_fScale;

		// Move the position back up to the parent if scale is zero.
		if ( ParentJoint.m_fScale < FLT_MIN )
		{
			OutJoint.m_Position = ParentJoint.m_Position;
		}

		//////////////////////////////////////////////////////////////////////////
		// Generate the final joint transform matrix from the position/quaternion pair.
		//////////////////////////////////////////////////////////////////////////

		D3DXMATRIXA16 TransformMatrix;
		D3DXMatrixRotationQuaternion( &TransformMatrix, &OutJoint.m_Orientation );

		TransformMatrix._41 = OutJoint.m_Position.x;
		TransformMatrix._42 = OutJoint.m_Position.y;
		TransformMatrix._43 = OutJoint.m_Position.z;

		D3DXMATRIXA16 ScaleMatrix;
		D3DXMatrixScaling( &ScaleMatrix, OutJoint.m_fScale, OutJoint.m_fScale, OutJoint.m_fScale );
		D3DXMatrixMultiply( &TransformMatrix, &ScaleMatrix, &TransformMatrix );

		// Store the bone transform.
		m_pJointTransformsSkel[ i ] = TransformMatrix;

		if ( m_pDismemberedDz != NULL && !g_bSkipOptimizedLimbs )
		{
			continue;
		}

		// Concatenate with the inverse bind pose matrix for GPU Skinning.
		// By transforming by the inverse bind pose matrix we can transform the vertices of the mesh from base
		// pose to a local space which is then transformed by the animated joints to world space in the vertex shader.
		D3DXMatrixMultiply( &m_pJointTransforms[ i ], &pSkel->m_pJoints[ i ].m_InverseBindPoseMatrix, &TransformMatrix );
	}

	//////////////////////////////////////////////////////////////////////////
	// Dismemberment modifications.
	//////////////////////////////////////////////////////////////////////////

	if ( m_pDismemberedDz == NULL )
	{
		return;
	}

	// Don't collapse limb joints, allowing us to see the damage surface geometry.
	if ( g_bSkipOptimizedLimbs )
	{
		CArDamageZone *pLimbHb = (CArDamageZone *)m_pDismemberedDz;

		assert( pLimbHb->m_iJointRange != -1 );

		for ( int i = 0; i < m_pAnimFile->m_AnimHierarchy.m_iNumJoints; ++i )
		{
			D3DXMATRIXA16 TransformMatrix = m_pJointTransformsSkel[ i ];

			// Move limb to origin.
			TransformMatrix._41 -= m_pJointTransformsSkel[ pLimbHb->m_iAttachmentJoint  ]._41;
			TransformMatrix._42 -= m_pJointTransformsSkel[ pLimbHb->m_iAttachmentJoint ]._42;
			TransformMatrix._43 -= m_pJointTransformsSkel[ pLimbHb->m_iAttachmentJoint ]._43;

			D3DXMatrixMultiply( &m_pJointTransforms[ i ], &pSkel->m_pJoints[ i ].m_InverseBindPoseMatrix, &TransformMatrix );
		}

		return;
	}

	// Reverse collapse the skeleton and move the joints to an origin relative to
	// the main damage zone.
	CArDamageZone *pLimbHb = (CArDamageZone *)m_pDismemberedDz;

	assert( pLimbHb->m_iJointRange != -1 );

	for ( int i = 0; i < m_pAnimFile->m_AnimHierarchy.m_iNumJoints; ++i )
	{
		D3DXMATRIXA16 TransformMatrix = m_pJointTransformsSkel[ i ];

		// Not a limb joint?
		if ( i < pLimbHb->m_iAttachmentJoint || i > pLimbHb->m_iJointRange )
		{
			TransformMatrix = m_pJointTransformsSkel[ pLimbHb->m_iAttachmentJoint ];

			TransformMatrix._41 -= m_pJointTransformsSkel[ pLimbHb->m_iAttachmentJoint ]._41;
			TransformMatrix._42 -= m_pJointTransformsSkel[ pLimbHb->m_iAttachmentJoint ]._42;
			TransformMatrix._43 -= m_pJointTransformsSkel[ pLimbHb->m_iAttachmentJoint ]._43;

			D3DXMATRIXA16 ScaleMatrix;
			D3DXMatrixScaling( &ScaleMatrix, 0.0f, 0.0f, 0.0f );
			D3DXMatrixMultiply( &TransformMatrix, &ScaleMatrix, &TransformMatrix );
		}
		else
		{
			// Move limb to origin.
			TransformMatrix._41 -= m_pJointTransformsSkel[ pLimbHb->m_iAttachmentJoint  ]._41;
			TransformMatrix._42 -= m_pJointTransformsSkel[ pLimbHb->m_iAttachmentJoint ]._42;
			TransformMatrix._43 -= m_pJointTransformsSkel[ pLimbHb->m_iAttachmentJoint ]._43;
		}

		//m_pJointTransformsSkel[ i ] = TransformMatrix;

		D3DXMatrixMultiply( &m_pJointTransforms[ i ], &pSkel->m_pJoints[ i ].m_InverseBindPoseMatrix, &TransformMatrix );
	}
}

//////////////////////////////////////////////////////////////////////////
// CArAnimator::Pause
//////////////////////////////////////////////////////////////////////////

void CArAnimator::Pause()
{
	m_fAnimScale = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// CArAnimator::SetFrame
//////////////////////////////////////////////////////////////////////////

void CArAnimator::SetFrame( float fFrame )
{
	m_fCurAnimFrame = fFrame;
}

//////////////////////////////////////////////////////////////////////////
// CArAnimator::GetFrame
//////////////////////////////////////////////////////////////////////////

float CArAnimator::GetFrame()
{
	return m_fCurAnimFrame;
}