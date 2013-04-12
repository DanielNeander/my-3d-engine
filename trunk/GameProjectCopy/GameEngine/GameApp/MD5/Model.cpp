// Model.cpp:
// Created by: Aurelio Reis

#include "stdafx.h"

#include "Model.h"
#include "MD5Anim.h"
#include "Animator.h"
#include "BoundingVolume.h"
#include "TextureManager.h"
#include "boost/tokenizer.hpp"

#include <string>
#include <iostream>


//////////////////////////////////////////////////////////////////////////
// CArBaseModel::CArBaseModel
//////////////////////////////////////////////////////////////////////////

CArBaseModel::CArBaseModel() {}

//////////////////////////////////////////////////////////////////////////
// CArBaseModel::~CArBaseModel
//////////////////////////////////////////////////////////////////////////

CArBaseModel::~CArBaseModel()
{
	Destroy();
}

//////////////////////////////////////////////////////////////////////////
// CArBaseModel::Destroy
//////////////////////////////////////////////////////////////////////////

void CArBaseModel::Destroy()
{
	for ( UINT32 i = 0; i < m_DamageZones.size(); ++i )
	{
		SAFE_DELETE( m_DamageZones[ i ] );
	}

	for ( UINT32 i = 0; i < m_Meshes.size(); ++i )
	{
		SAFE_DELETE( m_Meshes[ i ] );
	}

	m_Meshes.clear();
}

//////////////////////////////////////////////////////////////////////////
// CArBaseModel::CreateBuffers
//////////////////////////////////////////////////////////////////////////

void CArBaseModel::CreateBuffers()
{
	HRESULT hr;

	//////////////////////////////////////////////////////////////////////////
	// Create the GPU buffers.
	//////////////////////////////////////////////////////////////////////////

	for ( UINT32 i = 0; i < m_Meshes.size(); ++i )
	{
		CArModelMesh *pMesh = m_Meshes[ i ];
		CArGeometry *pGeom = pMesh->m_pGeometry;

		//////////////////////////////////////////////////////////////////////////
		// Create vertex buffer.
		//////////////////////////////////////////////////////////////////////////

		V( g_pd3dDevice->CreateVertexBuffer( pGeom->m_uiNumVertices * sizeof( SVertex ), 0, 
											  0, D3DPOOL_MANAGED, &pGeom->m_pVB, NULL ) );

		SVertex *pVtx = NULL;
		V( pGeom->m_pVB->Lock( 0, 0, (void**)&pVtx, 0 ) );
		memcpy( pVtx, pGeom->m_pVertices, sizeof( SVertex ) * pGeom->m_uiNumVertices );
		pGeom->m_pVB->Unlock();

		//////////////////////////////////////////////////////////////////////////
		// Create index buffer.
		//////////////////////////////////////////////////////////////////////////

		int iIbSize = pGeom->m_uiNumTriangles * 3;
		V( g_pd3dDevice->CreateIndexBuffer( iIbSize * sizeof( WORD ), 0,
											 D3DFMT_INDEX16, D3DPOOL_MANAGED, &pGeom->m_pIB, NULL ) );

		WORD *pIdx = NULL;
		V( pGeom->m_pIB->Lock( 0, 0, (void**)&pIdx, 0 ) );
		memcpy( pIdx, pGeom->m_pTriangles, sizeof( STriangle ) * pGeom->m_uiNumTriangles );
		pGeom->m_pIB->Unlock();
	}
}

//////////////////////////////////////////////////////////////////////////
// IsVertexInfluencedByJoint
//////////////////////////////////////////////////////////////////////////

bool IsVertexInfluencedByJoint( const SVertex &v, UINT32 uiJoint )
{
	for ( int k = 0; k < 4; ++k )
	{
		if ( v.BlendIndices[ k ] == uiJoint )
		{
			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// GenerateRemappedMesh
//////////////////////////////////////////////////////////////////////////

void GenerateRemappedMesh( CArGeometry *pOldGeom, const vector< int > &TriIndices,
						   CArGeometry *pNewGeom, UINT32 uiColor )
{
	pNewGeom->m_uiNumVertices = 0;

	bool *pUsedVertices = new bool[ pOldGeom->m_uiNumVertices ];
	memset( pUsedVertices, 0, sizeof( bool ) * pOldGeom->m_uiNumVertices );

	pNewGeom->m_uiNumTriangles = (UINT32)TriIndices.size();
	pNewGeom->m_pTriangles = new STriangle[ pNewGeom->m_uiNumTriangles ];

	//////////////////////////////////////////////////////////////////////////
	// Find out how many vertices are referenced.
	//////////////////////////////////////////////////////////////////////////

	for ( UINT32 i = 0; i < TriIndices.size(); ++i )
	{
		const STriangle &Triangle = pOldGeom->m_pTriangles[ TriIndices[ i ] ];

		for ( UINT32 j = 0; j < 3; ++j )
		{
			int idx = Triangle.m_Indices[ j ];

			if ( !pUsedVertices[ idx ] )
			{
				pUsedVertices[ idx ] = true;
				pNewGeom->m_uiNumVertices++;
			}
		}
	}

	pNewGeom->m_pVertices = new SVertex[ pNewGeom->m_uiNumVertices ];

	UINT32 iCurVert = 0;

	//////////////////////////////////////////////////////////////////////////
	// Create a new sequential order vertex list and at the same time create a new
	// triangle index list that references the new vertex index.
	//////////////////////////////////////////////////////////////////////////

	// For every referenced vertex...
	for ( UINT32 i = 0; i < pOldGeom->m_uiNumVertices; ++i )
	{
		// Vertex wasn't in the referenced set so skip.
		if ( !pUsedVertices[ i ] )
		{
			continue;
		}

		SVertex &v = pOldGeom->m_pVertices[ i ];

		bool bFound = false;

		// For every triangle...
		for ( UINT32 j = 0; j < TriIndices.size(); ++j )
		{
			const STriangle &Triangle = pOldGeom->m_pTriangles[ TriIndices[ j ] ];

			// For every tri index...
			for ( UINT32 k = 0; k < 3; ++k )
			{
				// If this triangle contains this vertex...
				if ( Triangle.m_Indices[ k ] == i )
				{
					if ( !bFound )
					{
						// Add to the list.
						pNewGeom->m_pVertices[ iCurVert ] = v;
						pNewGeom->m_pVertices[ iCurVert ].Color = uiColor;

						iCurVert++;
						bFound = true;
					}

					// Store the new vertex index.
					pNewGeom->m_pTriangles[ j ].m_Indices[ k ] = WORD( iCurVert - 1 );
				}
			}
		}
	}

	assert( iCurVert == pNewGeom->m_uiNumVertices );

	SAFE_DELETE_ARRAY( pUsedVertices );
}

//////////////////////////////////////////////////////////////////////////
// SeparateInfluencedTriSets
//////////////////////////////////////////////////////////////////////////

void SeparateInfluencedTriSets( vector< int > *pInSet, vector< int > *pNotInSet,
								CArGeometry *pGeom, const vector< int > &JointList )
{
	// For every triangle...
	for ( UINT32 i = 0; i < pGeom->m_uiNumTriangles; ++i )
	{
		const STriangle &Triangle = pGeom->m_pTriangles[ i ];

		// For every triangle vertex...
		UINT32 j = 0;
		for ( ; j < 3; ++j )
		{
			const SVertex &Vertex = pGeom->m_pVertices[ Triangle.m_Indices[ j ] ];

			// For every child joint...
			UINT32 l = 0;
			for ( ; l < JointList.size(); ++l )
			{
				if ( IsVertexInfluencedByJoint( Vertex, JointList[ l ] ) )
				{
					break;
				}
			}

			// Found it already.
			if ( l != JointList.size() )
			{
				break;
			}
		}

		// Made it to the end of the list?
		if ( j == 3 )
		{
			if ( pNotInSet )
			{
				pNotInSet->push_back( i );
			}
		}
		else
		{
			if ( pInSet )
			{
				pInSet->push_back( i );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// CArDamageSurfaceLimb
//////////////////////////////////////////////////////////////////////////

class CArDamageSurfaceLimb
{
public:
	vector< CArDamageZone * > m_DamageZones;
	int m_iStartVertex;
	int m_iVertexRange;
};

//////////////////////////////////////////////////////////////////////////
// GenerateLimbList
//////////////////////////////////////////////////////////////////////////

void GenerateLimbList( vector< CArDamageSurfaceLimb * > &LimbList, vector< int > &ChildJointList, const vector< CArDamageZone * > &DamageZones, const SMD5Skeleton &Skel )
{
	//////////////////////////////////////////////////////////////////////////
	// Go through every joint and when a damage zone is encountered, start a limb
	// made up of every damage zone below that joint.
	//////////////////////////////////////////////////////////////////////////

	CArDamageSurfaceLimb *pCurLimb = NULL;
	int iCurParentJoint = Skel.m_pJoints[ 0 ].m_iParentIndex;

	for ( int i = 0; i < Skel.m_iNumJoints; ++i )
	{
		CArDamageZone *pDzone = NULL;

		// Get the joint damage zone.
		// FIXME: Inefficient!
		vector< CArDamageZone * >::const_iterator iterDz = DamageZones.begin();
		for ( ; iterDz != DamageZones.end(); ++iterDz )
		{
			if ( (*iterDz)->m_iAttachmentJoint == i )
			{
				pDzone = *iterDz;
			}
		}

		if ( pDzone )
		{
			if ( !pCurLimb )
			{
				pCurLimb = new CArDamageSurfaceLimb();
				LimbList.push_back( pCurLimb );
			}

			pCurLimb->m_DamageZones.push_back( pDzone );
			pDzone->GatherChildJoints( Skel );

			for ( UINT32 j = 0; j < pDzone->ChildJointList.size(); ++j )
			{
				AddUniqueJoint( ChildJointList, pDzone->ChildJointList[ j ] );
			}
		}

		if ( Skel.m_pJoints[ i ].m_iParentIndex < iCurParentJoint )
		{
			pCurLimb = NULL;
		}

		iCurParentJoint = Skel.m_pJoints[ i ].m_iParentIndex;
	}
}

//////////////////////////////////////////////////////////////////////////
// CArBaseModel::GenerateDamageSurfaces
//////////////////////////////////////////////////////////////////////////

void CArBaseModel::GenerateDamageSurfaces( vector< CArDamageZone * > &DamageZones )
{
	//////////////////////////////////////////////////////////////////////////
	// NOTE: The decomposition of geometry into sub-geometry can be avoided
	// if an artist supplied a mesh for each damage surface.
	//////////////////////////////////////////////////////////////////////////

	vector< int > ChildJointList;
	vector< CArDamageSurfaceLimb * > LimbList;

	GenerateLimbList( LimbList, ChildJointList, DamageZones, m_pModelFile->GetSkeleton() );

	//////////////////////////////////////////////////////////////////////////
	// Separate the base non-damage zone influenced polys from the damage zone polys.
	// This removes stray invalid polys between valid polys, generates sequenced vertex
	// and index groups and gives us a smaller working set.
	//////////////////////////////////////////////////////////////////////////

	CArGeometry *pOldGeom = m_Meshes[ 0 ]->m_pGeometry;

	vector< int > InSetTriIndices;
	vector< int > OutSetTriIndices;

	typedef vector< int > TriIndexList;
	vector< TriIndexList > DamageSurfTriIndices;
	vector< CArGeometry * > DamageSurfaceGeoms;

	CArGeometry *pOutSetGeom = NULL;
	CArGeometry *pInSetGeom = NULL;

	UINT32 iDz = 0;
	UINT32 iCurVert = 0;
	UINT32 iCurTri = 0;

	vector< CArDamageSurfaceLimb * >::iterator iterLimb = LimbList.begin();
	for ( int iCurLimb = 0; iterLimb != LimbList.end(); ++iterLimb, ++iCurLimb )
	{
		CArDamageSurfaceLimb *pLimb = *iterLimb;

		assert( pLimb->m_DamageZones.size() != 0 );

		//////////////////////////////////////////////////////////////////////////
		// Start separating damage surfaces.
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// The way it works is like so:
		//	*	i = in-set, o = out-set
		//	*	For each damage zone, isolate the geometry by the triangles contained by
		//		the joints of that dz and those that are not.
		//	*	To begin, we trash o and feed i to the next iteration.
		//	*	The subsequent iterations store o and feed i.
		//	*	On the last iteration, we store both o and i.
		//////////////////////////////////////////////////////////////////////////

		CArDamageZone *pCurDz = pLimb->m_DamageZones[ 0 ];
		SeparateInfluencedTriSets( &InSetTriIndices, NULL, pOldGeom, pCurDz->ChildJointList );

		CArGeometry *pCurGeom;
		CArGeometry *pOriginalGeom;

		pOriginalGeom = pInSetGeom = pCurGeom = new CArGeometry();
		GenerateRemappedMesh( pOldGeom, InSetTriIndices, pCurGeom, g_Colors[ iCurLimb % MAX_COLORS ] );

		InSetTriIndices.clear();

		CArDamageZone *pPrevDz = pCurDz;

		UINT32 iLocalVertCount = pCurGeom->m_uiNumVertices;
		UINT32 iLocalTriCount = pCurGeom->m_uiNumTriangles;

		for ( iDz = 1; iDz < pLimb->m_DamageZones.size(); ++iDz )
		{
			pCurDz = pLimb->m_DamageZones[ iDz ];

			// Separate a list of tris into sets that are (and aren't) influenced by this damage zone's contained joints.
			SeparateInfluencedTriSets( &InSetTriIndices, &OutSetTriIndices, pCurGeom, pCurDz->ChildJointList );

			pOutSetGeom = new CArGeometry();
			GenerateRemappedMesh( pCurGeom, OutSetTriIndices, pOutSetGeom, g_Colors[ ( iCurLimb + iDz ) % MAX_COLORS ] );

			pInSetGeom = new CArGeometry();
			GenerateRemappedMesh( pCurGeom, InSetTriIndices, pInSetGeom, g_Colors[ ( iCurLimb + iDz + 1 ) % MAX_COLORS ] );

			// Use the in-set as the input (reduced) geometry for the next separation.
			pCurGeom = pInSetGeom;

			// Store the out-set surface data.
			pPrevDz->m_Surface.m_uiNumVerts = iLocalVertCount;
			pPrevDz->m_Surface.m_uiNumTris = iLocalTriCount;
			pPrevDz->m_Surface.m_uiStartVert = iCurVert;
			pPrevDz->m_Surface.m_uiStartIndex = iCurTri * 3;

			iCurVert += pOutSetGeom->m_uiNumVertices;
			iCurTri += pOutSetGeom->m_uiNumTriangles;

			// Each damage zone (but the last) should contain primitives from each damage zone below it.
			iLocalVertCount -= pOutSetGeom->m_uiNumVertices;
			iLocalTriCount -= pOutSetGeom->m_uiNumTriangles;

			DamageSurfaceGeoms.push_back( pOutSetGeom );

			InSetTriIndices.clear();
			OutSetTriIndices.clear();

			// Keep track of previous.
			pPrevDz = pCurDz;
		}

		// Delete the starting geometry if it's not the _only_ geometry.
		if ( pInSetGeom != pOriginalGeom )
		{
			SAFE_DELETE( pOriginalGeom );
		}

		// On the last iteration store the in-set surface data.
		pPrevDz->m_Surface.m_uiNumVerts = pInSetGeom->m_uiNumVertices;
		pPrevDz->m_Surface.m_uiNumTris = pInSetGeom->m_uiNumTriangles;
		pPrevDz->m_Surface.m_uiStartVert = iCurVert;
		pPrevDz->m_Surface.m_uiStartIndex = iCurTri * 3;
		iCurVert += pInSetGeom->m_uiNumVertices;
		iCurTri += pInSetGeom->m_uiNumTriangles;

		DamageSurfaceGeoms.push_back( pInSetGeom );

		// Done with this limb.
		SAFE_DELETE( pLimb );
	}

	//////////////////////////////////////////////////////////////////////////
	// Create the non-influenced geometry.
	//////////////////////////////////////////////////////////////////////////

	OutSetTriIndices.clear();

	SeparateInfluencedTriSets( NULL, &OutSetTriIndices, pOldGeom, ChildJointList );

	CArGeometry *pNewGeom = new CArGeometry();
	GenerateRemappedMesh( pOldGeom, OutSetTriIndices, pNewGeom, g_Colors[ COLOR_GREEN ] );

	DamageSurfaceGeoms.push_back( pNewGeom );

	//////////////////////////////////////////////////////////////////////////
	// Stitch the geometry back together.
	//////////////////////////////////////////////////////////////////////////

	SVertex		*pCombinedVertices;
	STriangle	*pCombinedTris;
	int			iNumCombinedVerts = 0;
	int			iNumCombinedTris = 0;

	for ( UINT32 i = 0; i < DamageSurfaceGeoms.size(); ++i )
	{
		iNumCombinedVerts += DamageSurfaceGeoms[ i ]->m_uiNumVertices;
		iNumCombinedTris += DamageSurfaceGeoms[ i ]->m_uiNumTriangles;
	}

	pCombinedVertices = new SVertex[ iNumCombinedVerts ];
	pCombinedTris = new STriangle[ iNumCombinedTris ];

	for ( UINT32 i = 0, iVertCount = 0, iTriCount = 0; i < DamageSurfaceGeoms.size(); ++i )
	{
		const CArGeometry *pGeom = DamageSurfaceGeoms[ i ];

		memcpy( pCombinedVertices + iVertCount, pGeom->m_pVertices, sizeof( SVertex ) * pGeom->m_uiNumVertices );
		memcpy( pCombinedTris + iTriCount, pGeom->m_pTriangles, sizeof( STriangle ) * pGeom->m_uiNumTriangles );

		// Offset each geom's indices by the current vertex count.
		for ( UINT32 j = 0; j < pGeom->m_uiNumTriangles; ++j )
		{
			for ( UINT32 k = 0; k < 3; ++k )
			{
				STriangle *pTri = (pCombinedTris + iTriCount + j);
				pTri->m_Indices[ k ] += (WORD)iVertCount;
			}
		}

		iVertCount += pGeom->m_uiNumVertices;
		iTriCount += pGeom->m_uiNumTriangles;
	}

	//////////////////////////////////////////////////////////////////////////
	// Destroy temp geometry.
	//////////////////////////////////////////////////////////////////////////

	for ( UINT32 i = 0; i < DamageSurfaceGeoms.size(); ++i )
	{
		SAFE_DELETE( DamageSurfaceGeoms[ i ] );
	}

	//////////////////////////////////////////////////////////////////////////
	// Transfer the new geometry over to the old.
	//////////////////////////////////////////////////////////////////////////

	SAFE_DELETE( pOldGeom->m_pVertices );
	pOldGeom->m_pVertices = pCombinedVertices;

	SAFE_DELETE( pOldGeom->m_pTriangles );
	pOldGeom->m_pTriangles = pCombinedTris;

	pOldGeom->m_uiNumVertices = iNumCombinedVerts;
	pOldGeom->m_uiNumTriangles = iNumCombinedTris;
}

static D3DXMATRIXA16 WorldMatrix;
static D3DXMATRIXA16 ViewMatrix;
static D3DXMATRIXA16 ProjMatrix;
static D3DXMATRIXA16 WvpMatrix;

//////////////////////////////////////////////////////////////////////////
// CArBaseModel::Render
//////////////////////////////////////////////////////////////////////////

void CArBaseModel::Render( D3DXMATRIX WorldMatrix, CArAnimator &Animator )
{
	//////////////////////////////////////////////////////////////////////////
	// Build the current frame skeleton.
	//////////////////////////////////////////////////////////////////////////
	
	Animator.Update( &m_pModelFile->GetSkeleton() );
	g_pEffect->SetMatrixArray( "g_JointTransforms", Animator.m_pJointTransforms, Animator.m_iNumTransforms );

	HRESULT hr;

	//////////////////////////////////////////////////////////////////////////
	// Update the geometry.
	//////////////////////////////////////////////////////////////////////////

	// FIXME:
	CArModelMesh *pMesh = m_Meshes[ 0 ];

#if 0
	SVertex *pVtx = NULL;
	V( pMesh->m_pVB->Lock( 0, 0, (void**)&pVtx, 0 ) );
	
	for ( int i = 0; i < Md5Mesh.m_iNumVertices; ++i )
	{
#if 0
		SMD5Vertex &Vert = Md5Mesh.m_pVertices[ i ];

		D3DXVECTOR3 vPos( 0.0f, 0.0f, 0.0f );

		for ( int j = 0; j < Vert.m_iWeightCount; ++j )
		{
			const SMD5Weight &Weight = Md5Mesh.m_pWeights[ Vert.m_iStartWeight + j ];
			const SMD5Joint &Joint = g_Anim.pAnimatedJoints[ Weight.m_iJointId ];

			D3DXMATRIXA16 mTransform;
			D3DXMatrixRotationQuaternion( &mTransform, &Joint.m_Orientation );

			// Calculate the transformed position based on the joint orientation and weight position.
			D3DXVECTOR3 vRotatedPt;
			D3DXVec3TransformCoord( &vRotatedPt, &Weight.m_Position, &mTransform );
			vPos += ( Joint.m_Position + vRotatedPt ) * Weight.m_fBias;
		}

		pVtx[ i ].Pos = vPos;
		pVtx[ i ].Texcoord = Vert.m_ST;
		pVtx[ i ].Color = 0xFFFFFFFF;
#else
		const SVertex &OrigVertex = g_pVertBuffer[ 0 ][ i ];

		D3DXVECTOR3 vTempPos = OrigVertex.Pos;
		D3DXVECTOR3 vFinal( 0.0f, 0.0f, 0.0f );

		// Apply influences.
		for ( int i = 0; i < 4; ++i )
		{
			const int iJointIdx = OrigVertex.BlendIndices[ i ];
			const float fWeight = OrigVertex.BlendWeights[ i ];

			D3DXVec3TransformCoord( &vTempPos, &OrigVertex.Pos, &m_pJointTransforms[ iJointIdx ] );

			// Accumulate the weighted positions.
			vFinal += vTempPos * fWeight;
		}

		pVtx[ i ] = g_pVertBuffer[ 0 ][ i ];
		pVtx[ i ].Pos = vFinal;
#endif
	}

	pMesh->m_pVB->Unlock();
#endif

	//////////////////////////////////////////////////////////////////////////
	// Render the model.
	//////////////////////////////////////////////////////////////////////////

	if ( g_bTintSurfaces )
	{
		V( g_pEffect->SetTechnique( "RenderModelTinted" ) );
	}
	else
	{
		V( g_pEffect->SetTechnique( "RenderModel" ) );
	}

	V( g_pEffect->SetTexture( "g_DiffuseTexture", g_pTexture->GetD3dTexture() ) );

	D3DXMatrixMultiply( &WorldMatrix, &WorldMatrix, g_Camera.GetWorldMatrix() );
	ViewMatrix = *g_Camera.GetViewMatrix();
	ProjMatrix = *g_Camera.GetProjMatrix();
	WvpMatrix = WorldMatrix * ViewMatrix * ProjMatrix;
	V( g_pEffect->SetMatrix( "g_WvpMatrix", &WvpMatrix ) );

	g_pd3dDevice->SetVertexDeclaration( g_pDecl );
	g_pd3dDevice->SetStreamSource( 0, pMesh->m_pGeometry->m_pVB, 0, sizeof( SVertex ) );
	g_pd3dDevice->SetIndices( pMesh->m_pGeometry->m_pIB );

	UINT NumPasses;
	g_pEffect->Begin( &NumPasses, 0 );
	for( UINT iPass=0; iPass < NumPasses; iPass++ )
	{
		g_pEffect->BeginPass( iPass );

		if ( Animator.m_pDismemberedDz )
		{
			CArDamageZone *pLimbHb = (CArDamageZone *)Animator.m_pDismemberedDz;
			g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, pLimbHb->m_Surface.m_uiStartVert, pMesh->m_pGeometry->m_uiNumVertices/*pLimbHb->m_Surface.m_uiNumVerts*/, pLimbHb->m_Surface.m_uiStartIndex, pLimbHb->m_Surface.m_uiNumTris );
		}
		else
		{
			g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, pMesh->m_pGeometry->m_uiNumVertices, 0, pMesh->m_pGeometry->m_uiNumTriangles );
		}

		g_pEffect->EndPass();
	}
	g_pEffect->End();

	//////////////////////////////////////////////////////////////////////////
	// Draw overlaid wireframe.
	//////////////////////////////////////////////////////////////////////////

	if ( g_bDrawWireframe )
	{	
		V( g_pEffect->SetTechnique( "RenderModelWireframe" ) );

		g_pEffect->Begin( &NumPasses, 0 );
		for( UINT iPass=0; iPass < NumPasses; iPass++ )
		{
			g_pEffect->BeginPass( iPass );
			g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, pMesh->m_pGeometry->m_uiNumVertices, 0, pMesh->m_pGeometry->m_uiNumTriangles );
			g_pEffect->EndPass();
		}
		g_pEffect->End();
	}

	//////////////////////////////////////////////////////////////////////////
	// Draw the skeleton.
	//////////////////////////////////////////////////////////////////////////

	if ( Animator.m_pDismemberedDz == NULL && g_bDrawSkeleton )
	{	
		DrawSkeleton( Animator );
	}
}

//////////////////////////////////////////////////////////////////////////
// CArBaseModel::DrawSkeleton
//////////////////////////////////////////////////////////////////////////

void CArBaseModel::DrawSkeleton( const CArAnimator &Animator )
{
	HRESULT hr;
	UINT NumPasses;

	SVertex SkeletonLines[ 80 * 2 ];
	int iLinePt = 0;

	for ( int i = 0; i < Animator.m_iNumTransforms; ++i )
	{
		// Draw dots for the joints.
		CArAabb Box;
		float fSize = 0.3f;
		Box.m_vMin = D3DXVECTOR3( -fSize, -fSize, -fSize );
		Box.m_vMax = D3DXVECTOR3( fSize, fSize, fSize );
		DrawAabb( Box, &Animator.m_pJointTransformsSkel[ i ], D3DCOLOR_COLORVALUE( 1.0f, 0.0f, 0.0f, 1.0f ) );

		// Also draw axis.
		DrawAxis( 2.0f, Animator.m_pJointTransformsSkel[ i ], true );

		const SMD5HierarchyJoint &HierarchyJoint = Animator.m_pAnimFile->m_AnimHierarchy.m_pJoints[ i ];

		if ( HierarchyJoint.m_iParentJoint == -1 )
		{
			continue;
		}
		
		D3DXVECTOR3 vCurPos = D3DXVECTOR3(	Animator.m_pJointTransformsSkel[ i ]._41,
											Animator.m_pJointTransformsSkel[ i ]._42,
											Animator.m_pJointTransformsSkel[ i ]._43 );
		D3DXVECTOR3 vParentPos = D3DXVECTOR3(	Animator.m_pJointTransformsSkel[ HierarchyJoint.m_iParentJoint ]._41,
												Animator.m_pJointTransformsSkel[ HierarchyJoint.m_iParentJoint ]._42,
												Animator.m_pJointTransformsSkel[ HierarchyJoint.m_iParentJoint ]._43 );

		SkeletonLines[ iLinePt ].Pos = vParentPos;
		SkeletonLines[ iLinePt++ ].Color = 0xFFFFFFFF;
		SkeletonLines[ iLinePt ].Pos = vCurPos;
		SkeletonLines[ iLinePt++ ].Color = 0xFFFFFFFF;
	}

	V( g_pEffect->SetTechnique( "RenderSimpleAlways" ) );

	g_pEffect->Begin( &NumPasses, 0 );
	g_pEffect->BeginPass( 0 );
	g_pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, iLinePt / 2, SkeletonLines, sizeof( SVertex ) );
	g_pEffect->EndPass();
	g_pEffect->End();
}

//////////////////////////////////////////////////////////////////////////
// CArBaseModel::FindJointIndex
//////////////////////////////////////////////////////////////////////////

int CArBaseModel::FindJointIndex( const char *strJointName )
{
	return m_pModelFile->FindJointIndex( strJointName );
}

//////////////////////////////////////////////////////////////////////////
// CArModelInstance::InitializeFromModelFile
//////////////////////////////////////////////////////////////////////////

void CArModelInstance::InitializeFromModelFile( IArModelFile *pModelFile )
{
	assert( pModelFile );

	m_pModelFile = pModelFile;

	for ( UINT32 i = 0; i < pModelFile->GetBaseModel()->m_Meshes.size(); ++i )
	{
		CArModelMesh *pOtherMesh = pModelFile->GetBaseModel()->m_Meshes[ i ];
		CArModelMesh *pNewMesh = new CArModelMesh();

		m_Meshes.push_back( pNewMesh );

		*pNewMesh = *pOtherMesh;
		pNewMesh->m_bIsGeometryReferenced = true;
	}
}
