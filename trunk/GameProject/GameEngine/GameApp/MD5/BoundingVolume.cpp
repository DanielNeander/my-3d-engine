// BoundingVolume.cpp:
// Created by: Aurelio Reis

#include "stdafx.h"

#include "BoundingVolume.h"
#include "MD5Model.h"


//////////////////////////////////////////////////////////////////////////
// DrawAabbWireframe
//////////////////////////////////////////////////////////////////////////

void DrawAabbWireframe( const CArAabb &Box, const D3DXMATRIXA16 *pTransform, UINT Color )
{
	const D3DXVECTOR3 &vMin = Box.m_vMin;
	const D3DXVECTOR3 &vMax = Box.m_vMax;

	//////////////////////////////////////////////////////////////////////////
	// Generate corners for the box (8 total)
	//////////////////////////////////////////////////////////////////////////

	enum EAabbCorner { LBF, RBF, LBB, RBB, LTF, RTF, LTB, RTB };
	D3DXVECTOR3 vCorners[ 8 ] =
	{
		D3DXVECTOR3( vMin.x, vMin.y, vMin.z ),	// left bottom front
		D3DXVECTOR3( vMax.x, vMin.y, vMin.z ),	// right bottom front

		D3DXVECTOR3( vMin.x, vMin.y, vMax.z ),	// left bottom back
		D3DXVECTOR3( vMax.x, vMin.y, vMax.z ),	// right bottom back

		D3DXVECTOR3( vMin.x, vMax.y, vMin.z ),	// left top front
		D3DXVECTOR3( vMax.x, vMax.y, vMin.z ),	// right top front

		D3DXVECTOR3( vMin.x, vMax.y, vMax.z ),	// left top back
		D3DXVECTOR3( vMax.x, vMax.y, vMax.z ),	// right top back
	};

	//////////////////////////////////////////////////////////////////////////
	// Apply transforms (if any)
	//////////////////////////////////////////////////////////////////////////

	if ( pTransform )
	{
		for ( int i = 0; i < 8; ++i )
		{
			D3DXVec3TransformCoord( &vCorners[ i ], &vCorners[ i ], pTransform );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Create edge lines
	//////////////////////////////////////////////////////////////////////////

	SVertex EdgeLines[ 12 * 2 ];		// 12 edges
	int iLine = 0;

	//////////////////////////////////////////////////////////////////////////
	// Top face
	//////////////////////////////////////////////////////////////////////////

	// left
	EdgeLines[ iLine++ ].Pos = vCorners[ LTF ];
	EdgeLines[ iLine++ ].Pos = vCorners[ LTB ];

	// right
	EdgeLines[ iLine++ ].Pos = vCorners[ RTF ];
	EdgeLines[ iLine++ ].Pos = vCorners[ RTB ];

	// back
	EdgeLines[ iLine++ ].Pos = vCorners[ LTB ];
	EdgeLines[ iLine++ ].Pos = vCorners[ RTB ];

	// front
	EdgeLines[ iLine++ ].Pos = vCorners[ LTF ];
	EdgeLines[ iLine++ ].Pos = vCorners[ RTF ];

	//////////////////////////////////////////////////////////////////////////
	// Bottom face
	//////////////////////////////////////////////////////////////////////////

	// left
	EdgeLines[ iLine++ ].Pos = vCorners[ LBF ];
	EdgeLines[ iLine++ ].Pos = vCorners[ LBB ];

	// right
	EdgeLines[ iLine++ ].Pos = vCorners[ RBF ];
	EdgeLines[ iLine++ ].Pos = vCorners[ RBB ];

	// back
	EdgeLines[ iLine++ ].Pos = vCorners[ LBB ];
	EdgeLines[ iLine++ ].Pos = vCorners[ RBB ];

	// front
	EdgeLines[ iLine++ ].Pos = vCorners[ LBF ];
	EdgeLines[ iLine++ ].Pos = vCorners[ RBF ];

	//////////////////////////////////////////////////////////////////////////
	// Side faces
	//////////////////////////////////////////////////////////////////////////

	// back left
	EdgeLines[ iLine++ ].Pos = vCorners[ LTB ];
	EdgeLines[ iLine++ ].Pos = vCorners[ LBB ];

	// back right
	EdgeLines[ iLine++ ].Pos = vCorners[ RTB ];
	EdgeLines[ iLine++ ].Pos = vCorners[ RBB ];

	// front left
	EdgeLines[ iLine++ ].Pos = vCorners[ LTF ];
	EdgeLines[ iLine++ ].Pos = vCorners[ LBF ];

	// front right
	EdgeLines[ iLine++ ].Pos = vCorners[ RTF ];
	EdgeLines[ iLine++ ].Pos = vCorners[ RBF ];

	for ( int i = 0; i < iLine; ++i )
	{
		EdgeLines[ i ].Color = Color;
	}

	//////////////////////////////////////////////////////////////////////////
	// Finally draw...
	//////////////////////////////////////////////////////////////////////////

	HRESULT hr;
	V( g_pEffect->SetTechnique( "RenderSimpleAlways" ) );

	UINT NumPasses;
	g_pEffect->Begin( &NumPasses, 0 );
	g_pEffect->BeginPass( 0 );
	g_pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, iLine / 2, EdgeLines, sizeof( SVertex ) );
	g_pEffect->EndPass();
	g_pEffect->End();
}


//////////////////////////////////////////////////////////////////////////
// DrawAabb
//////////////////////////////////////////////////////////////////////////

void DrawAabb( const CArAabb &Box, const D3DXMATRIXA16 *pTransform, UINT Color )
{
	const D3DXVECTOR3 &vMin = Box.m_vMin;
	const D3DXVECTOR3 &vMax = Box.m_vMax;

	//////////////////////////////////////////////////////////////////////////
	// Generate triangles (12 total)
	//////////////////////////////////////////////////////////////////////////

	D3DXVECTOR3 Tris[ 12 * 3 ] =
	{
		// Bottom face
		D3DXVECTOR3( vMin.x, vMin.y, vMax.z ),	// left bottom back
		D3DXVECTOR3( vMax.x, vMin.y, vMin.z ),	// right bottom front
		D3DXVECTOR3( vMin.x, vMin.y, vMin.z ),	// left bottom front

		D3DXVECTOR3( vMin.x, vMin.y, vMax.z ),	// left bottom back
		D3DXVECTOR3( vMax.x, vMin.y, vMax.z ),	// right bottom back
		D3DXVECTOR3( vMax.x, vMin.y, vMin.z ),	// right bottom front

		// Top face
		D3DXVECTOR3( vMin.x, vMax.y, vMax.z ),	// left top back
		D3DXVECTOR3( vMax.x, vMax.y, vMin.z ),	// right top front
		D3DXVECTOR3( vMin.x, vMax.y, vMin.z ),	// left top front

		D3DXVECTOR3( vMin.x, vMax.y, vMax.z ),	// left top back
		D3DXVECTOR3( vMax.x, vMax.y, vMax.z ),	// right top back
		D3DXVECTOR3( vMax.x, vMax.y, vMin.z ),	// right top front

		// Front face
		D3DXVECTOR3( vMin.x, vMax.y, vMin.z ),	// left top front
		D3DXVECTOR3( vMax.x, vMin.y, vMin.z ),	// right bottom front
		D3DXVECTOR3( vMin.x, vMin.y, vMin.z ),	// left bottom front

		D3DXVECTOR3( vMin.x, vMax.y, vMin.z ),	// left top front
		D3DXVECTOR3( vMax.x, vMax.y, vMin.z ),	// right top front
		D3DXVECTOR3( vMax.x, vMin.y, vMin.z ),	// right bottom front

		// Back face
		D3DXVECTOR3( vMin.x, vMax.y, vMax.z ),	// left top front
		D3DXVECTOR3( vMax.x, vMin.y, vMax.z ),	// right bottom front
		D3DXVECTOR3( vMin.x, vMin.y, vMax.z ),	// left bottom front

		D3DXVECTOR3( vMin.x, vMax.y, vMax.z ),	// left top front
		D3DXVECTOR3( vMax.x, vMax.y, vMax.z ),	// right top front
		D3DXVECTOR3( vMax.x, vMin.y, vMax.z ),	// right bottom front

		// Left face
		D3DXVECTOR3( vMin.x, vMax.y, vMax.z ),	// left top back
		D3DXVECTOR3( vMin.x, vMin.y, vMin.z ),	// left bottom front
		D3DXVECTOR3( vMin.x, vMin.y, vMax.z ),	// left bottom back

		D3DXVECTOR3( vMin.x, vMax.y, vMax.z ),	// left top back
		D3DXVECTOR3( vMin.x, vMax.y, vMin.z ),	// left top front
		D3DXVECTOR3( vMin.x, vMin.y, vMin.z ),	// left bottom front

		// Right face
		D3DXVECTOR3( vMax.x, vMax.y, vMax.z ),	// left top back
		D3DXVECTOR3( vMax.x, vMin.y, vMin.z ),	// left bottom front
		D3DXVECTOR3( vMax.x, vMin.y, vMax.z ),	// left bottom back

		D3DXVECTOR3( vMax.x, vMax.y, vMax.z ),	// right top back
		D3DXVECTOR3( vMax.x, vMax.y, vMin.z ),	// right top front
		D3DXVECTOR3( vMax.x, vMin.y, vMin.z ),	// right bottom front
	};

	//////////////////////////////////////////////////////////////////////////
	// Apply transforms (if any)
	//////////////////////////////////////////////////////////////////////////

	if ( pTransform )
	{
		for ( int i = 0; i < ( 12 * 3 ); ++i )
		{
			D3DXVec3TransformCoord( &Tris[ i ], &Tris[ i ], pTransform );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Finally draw...
	//////////////////////////////////////////////////////////////////////////

	SVertex Verts[ 12 * 3 ];
	for ( int i = 0; i < ( 12 * 3 ); ++i )
	{
		Verts[ i ].Pos = Tris[ i ];
		Verts[ i ].Color = Color;
	}

	HRESULT hr;
	V( g_pEffect->SetTechnique( "RenderSimpleAlways" ) );

	UINT NumPasses;
	g_pEffect->Begin( &NumPasses, 0 );
	g_pEffect->BeginPass( 0 );
	g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 12, Verts, sizeof( SVertex ) );
	g_pEffect->EndPass();
	g_pEffect->End();
}


//////////////////////////////////////////////////////////////////////////
// CArDamageZone::GatherChildJoints
//////////////////////////////////////////////////////////////////////////

void CArDamageZone::GatherChildJoints( const SMD5Skeleton &Skel )
{
	// This probably shouldn't happen (a hitbox on the root bone)?
	if ( m_iAttachmentJoint == -1 )
	{
		return;
	}

	ChildJointList.push_back( m_iAttachmentJoint );
	AddUniqueJoint( ChildJointList, m_iAttachmentJoint );

	// Get all the child joints.
	for ( int i = m_iAttachmentJoint + 1; i < Skel.m_iNumJoints; ++i )
	{
		// If the joints parent is greater than the hitbox joint, it's a child.
		if ( Skel.m_pJoints[ i ].m_iParentIndex >= m_iAttachmentJoint )
		{
			ChildJointList.push_back( i );
			AddUniqueJoint( ChildJointList, i );
		}
		else
		{
			break;
		}
	}

	m_iJointRange = ChildJointList[ ChildJointList.size() - 1 ];
}

//////////////////////////////////////////////////////////////////////////
// AddUniqueJoint
//////////////////////////////////////////////////////////////////////////

void AddUniqueJoint( vector< int > &JointList, int iJoint )
{
	for ( UINT32 i = 0; i < JointList.size(); ++i )
	{
		if ( iJoint == JointList[ i ] )
		{
			return;
		}
	}

	JointList.push_back( iJoint );
}
