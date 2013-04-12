#include "stdafx.h"
#include "DamageModel.h"
#include "DamageAnimator.h"
#include "DamageZone.h"
#include "GameApp/GameApp.h"



CArBaseModel::CArBaseModel() : m_pAnimator(NULL)
{

}

CArBaseModel::~CArBaseModel()
{
	Destroy();
}

void CArBaseModel::Destroy()
{
	SAFE_DELETE(m_pAnimator);
	/*for ( UINT32 i = 0; i < m_DamageZones.size(); ++i )
	{
		SAFE_DELETE( m_DamageZones[ i ] );
	}*/

	for ( UINT32 i = 0; i < m_Meshes.size(); ++i )
	{
		SAFE_DELETE( m_Meshes[ i ] );
	}

	m_Meshes.clear();
}

void CArBaseModel::Initialize(const SMD5Skeleton* skel) {

	m_pAnimator = new CArAnimator();
	m_pAnimator->Initialize( skel );
	m_pSkel = skel;
}

void CArBaseModel::CreateBuffers()
{


	HRESULT hr;

	//////////////////////////////////////////////////////////////////////////
	// Create the GPU buffers.
	//////////////////////////////////////////////////////////////////////////

	FormatDesc vbFmt[] = {
		0, TYPE_VERTEX, FORMAT_FLOAT,  3,
		0, TYPE_NORMAL, FORMAT_FLOAT,   3,
		0, TYPE_TANGENT,FORMAT_FLOAT, 3,
		0, TYPE_BINORMAL,FORMAT_FLOAT, 3,		
		0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,
		0, TYPE_BONES,	 FORMAT_FLOAT,   4,
		0, TYPE_WEIGHTS, FORMAT_FLOAT,  4,			
	};
	int vertexStride = 0;
	for (int i = 0; i < elementsOf(vbFmt); ++i)
	{
		vertexStride +=vbFmt[i].size * GetRenderer()->getFormatSize(vbFmt[i].format);
	}

	for ( UINT32 i = 0; i < m_Meshes.size(); ++i )
	{
		CArModelMesh *pMesh = m_Meshes[ i ];
		CArGeometry *pGeom = pMesh->m_pGeometry;
				
		pGeom->m_shader = gSkinned;
		pGeom->m_vf = GetRenderer()->addVertexFormat(vbFmt, elementsOf(vbFmt), pGeom->m_shader);
		pGeom->m_vb = GetRenderer()->addVertexBuffer(pGeom->m_uiNumVertices * sizeof( SVertex ), STATIC, pGeom->m_pVertices);
		pGeom->m_ib = GetRenderer()->addIndexBuffer(pGeom->m_uiNumTriangles * 3, sizeof(WORD), STATIC, pGeom->m_pTriangles);
		pGeom->m_samplerState = GetRenderer()->addSamplerState(LINEAR, WRAP, WRAP, WRAP);

	}
}


void CArBaseModel::Render( idRenderModelMD5* pEntity, idAnimator* pAnimator )
{

	m_pAnimator->Update(m_pSkel, pAnimator);

	idJointMat *joint;
	int numJoints;
	pAnimator->GetJoints(&numJoints, &joint);

	noVec3 offset;
	noMat3 axis;	
	std::vector<mat4> skinMats;

	int miliSecPerFrame = 1000/ 60;

	static size_t frame = 0;
	
	//if (frame >= miliSecPerFrame)
	frame += miliSecPerFrame;
	noQuat q;
	if (frame > 1000) {
		skinMats.resize(numJoints);
		for (int i=0; i < numJoints; i++)
		{
			offset = joint[i].ToVec3();
			axis = joint[i].ToMat3();														
			noMat4 m = noMat4(axis, offset);									
			m.TransposeSelf();			
			skinMats[i] = ToMat4(m);			
			//skinMats[i] = identity4();		

			// Concatenate with the inverse bind pose matrix for GPU Skinning.
			// By transforming by the inverse bind pose matrix we can transform the vertices of the mesh from base
			// pose to a local space which is then transformed by the animated joints to world space in the vertex shader.
			noMat4 invPose((float*)m_pSkel->m_pJoints[i].m_InverseBindPoseMatrix);
			skinMats[i] = ToMat4(invPose) * skinMats[i];
		}
		frame = 0;
	}

	Renderer* renderer = GetRenderer();
	renderer->resetToDefaults();

	//for ( UINT32 i = 0; i < m_Meshes.size(); ++i )
	int					i;
	idMD5Mesh			*mesh;
	for( mesh = pEntity->meshes.Ptr(), i = 0; i < pEntity->meshes.Num(); i++, mesh++ )
	{
		const idMaterial *shader = mesh->shader;
		CArModelMesh *pMesh = m_Meshes[ i ];
		CArGeometry *pGeom = pMesh->m_pGeometry;
		renderer->reset();
		renderer->setVertexFormat(pGeom->m_vf);
		renderer->setVertexBuffer(0, pGeom->m_vb);
		renderer->setIndexBuffer(pGeom->m_ib);	
		renderer->setShader(pGeom->m_shader);		
		if (shader->diffuseImg != -1) renderer->setTexture("g_txDiffuse", shader->diffuseImg);	
		renderer->setSamplerState("g_samLinear", pGeom->m_samplerState);
		renderer->setDepthState(GetApp()->depthWrite_);

		renderer->setShaderConstant4x4f("g_mWorld", WorldMat());		
		renderer->setShaderConstant4x4f("g_mView", ViewMat());
		renderer->setShaderConstant4x4f("g_mWorldViewProjection",  WorldMat() * ViewMat() * ProjMat());

		if(!skinMats.empty())
			renderer->setShaderConstantArray4x4f("g_matrices", &skinMats[0], numJoints);		
		renderer->apply();
		renderer->drawElements(PRIM_TRIANGLES, 0, pGeom->m_uiNumTriangles * 3, 0, 0);
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

void GenerateRemappedMesh( CArGeometry *pOldGeom, const std::vector< int > &TriIndices,
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
						//pNewGeom->m_pVertices[ iCurVert ].Color = uiColor;

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

void SeparateInfluencedTriSets( std::vector< int > *pInSet, std::vector< int > *pNotInSet,
	CArGeometry *pGeom, const std::vector< int > &JointList )
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
	std::vector< CArDamageZone * > m_DamageZones;
	int m_iStartVertex;
	int m_iVertexRange;
};

//////////////////////////////////////////////////////////////////////////
// GenerateLimbList
//////////////////////////////////////////////////////////////////////////

void GenerateLimbList( std::vector< CArDamageSurfaceLimb * > &LimbList, std::vector< int > &ChildJointList, const std::vector< CArDamageZone * > &DamageZones, const SMD5Skeleton &Skel )
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
		std::vector< CArDamageZone * >::const_iterator iterDz = DamageZones.begin();
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

inline void CalcQuaternionW( D3DXQUATERNION &Q )
{
	float t = 1.0f - ( Q.x * Q.x ) - ( Q.y * Q.y ) - ( Q.z * Q.z );
	Q.w = t < 0.0f ? 0.0f : -sqrtf( t );

	D3DXQuaternionNormalize(&Q, &Q);
}


void CArBaseModel::GenerateDamageSurfaces( std::vector< CArDamageZone * > &DamageZones )
{
	//////////////////////////////////////////////////////////////////////////
	// NOTE: The decomposition of geometry into sub-geometry can be avoided
	// if an artist supplied a mesh for each damage surface.
	//////////////////////////////////////////////////////////////////////////

	std::vector< int > ChildJointList;
	std::vector< CArDamageSurfaceLimb * > LimbList;

	//GenerateLimbList( LimbList, ChildJointList, DamageZones, m_pModelFile->GetSkeleton() );
}

