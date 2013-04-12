#include "stdafx.h"
#include "CascadedShadowsManager.h"
#include "ShadowCamera.h"
#include "GameApp.h"
#include "GameObjectUtil.h"
#include  "Mesh.h"
#include "M2Mesh.h"
#include "Util/EffectUtil.h"
#include "SDKMeshLoader.h"


static const noVec4 g_vFLTMAX (FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX );
static const noVec4 g_vFLTMIN ( -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX );
static const noVec4 g_vHalfVector ( 0.5f, 0.5f, 0.5f, 0.5f );
static const noVec4 g_vMultiplySetzwToZero ( 1.0f, 1.0f, 0.0f, 0.0f );
static const noVec4 g_vZero ( 0.0f, 0.0f, 0.0f, 0.0f );

CascadedShadowsManager::CascadedShadowsManager( CascadeConfig* cascadeConfig )
	:m_pCascadeConfig(cascadeConfig),
	/*m_CopyOfCascadeConfig(*cascadeConfig),*/
	m_RenderOneTileVP( m_RenderVP[0] ),
	m_iBlurBetweenCascades( 0 ),
	m_iDerivativeBasedOffset( 0 ),
	m_fBlurBetweenCascadesAmount( 0.005f ),
	m_iPCFBlurSize( 3 ),
	m_fPCFOffset( 0.002f ),
	m_pcbGlobalConstantBuffer( NULL ),
	m_prsShadow( NULL ),
	m_prsShadowPancake( NULL ),
	m_prsScene( NULL ),
	m_pVertexLayoutMesh( NULL ),
	m_pSamLinear( NULL ),
	m_pSamShadowPCF( NULL ),  
	m_pSamShadowPoint( NULL ),
	m_pCascadedShadowMapTexture( NULL ),
	m_pCascadedShadowMapDSV( NULL ),
	m_pCascadedShadowMapSRV( NULL ),
	m_pvsRenderOrthoShadowBlob( NULL )

{
	memset(m_fCascadePartitionsFrustum, 0, sizeof(m_fCascadePartitionsFrustum));
	for( INT index=0; index < MAX_CASCADES; ++index ) 
	{
		m_RenderVP[index].Height = (FLOAT)m_CopyOfCascadeConfig.m_iBufferSize;
		m_RenderVP[index].Width = (FLOAT)m_CopyOfCascadeConfig.m_iBufferSize;
		m_RenderVP[index].MaxDepth = 1.0f;
		m_RenderVP[index].MinDepth = 0.0f;
		m_RenderVP[index].TopLeftX = 0;
		m_RenderVP[index].TopLeftY = 0;		
		for( int x1 = 0; x1 < 2; ++x1 ) 
		{
			for( int x2 = 0; x2 < 2; ++x2 ) 
			{
				for( int x3 = 0; x3 < 2; ++x3 ) 
				{
					m_aaSceneAllShaders[index][x1][x2][x3] = -1;
				}
			}
		}

	}
}

CascadedShadowsManager::~CascadedShadowsManager()
{
	DestroyAndDeallocateShadowResources();

}

void CascadedShadowsManager::DestroyAndDeallocateShadowResources()
{
	SAFE_RELEASE( m_pVertexLayoutMesh );
	SAFE_RELEASE( m_pSamLinear );
	SAFE_RELEASE( m_pSamShadowPoint );
	SAFE_RELEASE( m_pSamShadowPCF );

	SAFE_RELEASE( m_pCascadedShadowMapTexture );
	SAFE_RELEASE( m_pCascadedShadowMapDSV );
	SAFE_RELEASE( m_pCascadedShadowMapSRV );

	SAFE_RELEASE( m_pcbGlobalConstantBuffer );

	SAFE_RELEASE( m_prsShadow );
	SAFE_RELEASE( m_prsShadowPancake );
	SAFE_RELEASE( m_prsScene );

	SAFE_RELEASE( m_pvsRenderOrthoShadow );

	for( INT iCascadeIndex=0; iCascadeIndex < MAX_CASCADES; ++iCascadeIndex ) 
	{ 
		SAFE_RELEASE( m_pvsRenderScene[iCascadeIndex] );
		for( INT iDerivativeIndex=0; iDerivativeIndex < 2; ++iDerivativeIndex ) 
		{
			for( INT iBlendIndex=0; iBlendIndex < 2; ++iBlendIndex ) 
			{
				for( INT iIntervalIndex=0; iIntervalIndex < 2; ++iIntervalIndex ) 
				{
					SAFE_RELEASE( m_ppsRenderSceneAllShaders[iCascadeIndex][iDerivativeIndex][iBlendIndex][iIntervalIndex] );
				}
			}
		}
	}
}


//--------------------------------------------------------------------------------------
// Used to compute an intersection of the orthographic projection and the Scene AABB
//--------------------------------------------------------------------------------------
struct Triangle 
{
	noVec4 pt[3];
	BOOL culled;
};

//--------------------------------------------------------------------------------------
// Computing an accurate near and flar plane will decrease surface acne and Peter-panning.
// Surface acne is the term for erroneous self shadowing.  Peter-panning is the effect where
// shadows disappear near the base of an object.
// As offsets are generally used with PCF filtering due self shadowing issues, computing the
// correct near and far planes becomes even more important.
// This concept is not complicated, but the intersection code is.
//--------------------------------------------------------------------------------------
void CascadedShadowsManager::ComputeNearAndFar( FLOAT& fNearPlane, FLOAT& fFarPlane, const noVec4& vLightCameraOrthographicMin, const noVec4& vLightCameraOrthographicMax, noVec4* pvPointsInCameraView )
{
	// Initialize the near and far planes
	fNearPlane = FLT_MAX;
	fFarPlane = -FLT_MAX;

	Triangle triangleList[16];
	INT iTriangleCnt = 1;

	triangleList[0].pt[0] = pvPointsInCameraView[0];
	triangleList[0].pt[1] = pvPointsInCameraView[1];
	triangleList[0].pt[2] = pvPointsInCameraView[2];
	triangleList[0].culled = false;

	// These are the indices used to tesselate an AABB into a list of triangles.
	static const INT iAABBTriIndexes[] = 
	{
		0,1,2,  1,2,3,
		4,5,6,  5,6,7,
		0,2,4,  2,4,6,
		1,3,5,  3,5,7,
		0,1,4,  1,4,5,
		2,3,6,  3,6,7 
	};

	INT iPointPassesCollision[3];

	// At a high level: 
	// 1. Iterate over all 12 triangles of the AABB.  
	// 2. Clip the triangles against each plane. Create new triangles as needed.
	// 3. Find the min and max z values as the near and far plane.

	//This is easier because the triangles are in camera spacing making the collisions tests simple comparisions.

	float fLightCameraOrthographicMinX = vLightCameraOrthographicMin.x ;
	float fLightCameraOrthographicMaxX = vLightCameraOrthographicMax.x; 
	float fLightCameraOrthographicMinY = vLightCameraOrthographicMin.y;
	float fLightCameraOrthographicMaxY = vLightCameraOrthographicMax.y;

	for( INT AABBTriIter = 0; AABBTriIter < 12; ++AABBTriIter ) 
	{

		triangleList[0].pt[0] = pvPointsInCameraView[ iAABBTriIndexes[ AABBTriIter*3 + 0 ] ];
		triangleList[0].pt[1] = pvPointsInCameraView[ iAABBTriIndexes[ AABBTriIter*3 + 1 ] ];
		triangleList[0].pt[2] = pvPointsInCameraView[ iAABBTriIndexes[ AABBTriIter*3 + 2 ] ];
		iTriangleCnt = 1;
		triangleList[0].culled = FALSE;

		// Clip each invidual triangle against the 4 frustums.  When ever a triangle is clipped into new triangles, 
		//add them to the list.
		for( INT frustumPlaneIter = 0; frustumPlaneIter < 4; ++frustumPlaneIter ) 
		{

			FLOAT fEdge;
			INT iComponent;

			if( frustumPlaneIter == 0 ) 
			{
				fEdge = fLightCameraOrthographicMinX; // todo make float temp
				iComponent = 0;
			} 
			else if( frustumPlaneIter == 1 ) 
			{
				fEdge = fLightCameraOrthographicMaxX;
				iComponent = 0;
			} 
			else if( frustumPlaneIter == 2 ) 
			{
				fEdge = fLightCameraOrthographicMinY;
				iComponent = 1;
			} 
			else 
			{
				fEdge = fLightCameraOrthographicMaxY;
				iComponent = 1;
			}

			for( INT triIter=0; triIter < iTriangleCnt; ++triIter ) 
			{
				// We don't delete triangles, so we skip those that have been culled.
				if( !triangleList[triIter].culled ) 
				{
					INT iInsideVertCount = 0;
					noVec4 tempOrder;
					// Test against the correct frustum plane.
					// This could be written more compactly, but it would be harder to understand.

					if( frustumPlaneIter == 0 ) 
					{
						for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
						{
							if( ( triangleList[triIter].pt[triPtIter] ).x >
								( vLightCameraOrthographicMin ).x ) 
							{ 
								iPointPassesCollision[triPtIter] = 1;
							}
							else 
							{
								iPointPassesCollision[triPtIter] = 0;
							}
							iInsideVertCount += iPointPassesCollision[triPtIter];
						}
					}
					else if( frustumPlaneIter == 1 ) 
					{
						for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
						{
							if( ( triangleList[triIter].pt[triPtIter] ).x < 
								( vLightCameraOrthographicMax ).x )
							{
								iPointPassesCollision[triPtIter] = 1;
							}
							else
							{ 
								iPointPassesCollision[triPtIter] = 0;
							}
							iInsideVertCount += iPointPassesCollision[triPtIter];
						}
					}
					else if( frustumPlaneIter == 2 ) 
					{
						for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
						{
							if( ( triangleList[triIter].pt[triPtIter] ).y > 
								( vLightCameraOrthographicMin ).y ) 
							{
								iPointPassesCollision[triPtIter] = 1;
							}
							else 
							{
								iPointPassesCollision[triPtIter] = 0;
							}
							iInsideVertCount += iPointPassesCollision[triPtIter];
						}
					}
					else 
					{
						for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
						{
							if( ( triangleList[triIter].pt[triPtIter] ).y < 
								( vLightCameraOrthographicMax ).y ) 
							{
								iPointPassesCollision[triPtIter] = 1;
							}
							else 
							{
								iPointPassesCollision[triPtIter] = 0;
							}
							iInsideVertCount += iPointPassesCollision[triPtIter];
						}
					}

					// Move the points that pass the frustum test to the begining of the array.
					if( iPointPassesCollision[1] && !iPointPassesCollision[0] ) 
					{
						tempOrder =  triangleList[triIter].pt[0];   
						triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
						triangleList[triIter].pt[1] = tempOrder;
						iPointPassesCollision[0] = TRUE;            
						iPointPassesCollision[1] = FALSE;            
					}
					if( iPointPassesCollision[2] && !iPointPassesCollision[1] ) 
					{
						tempOrder =  triangleList[triIter].pt[1];   
						triangleList[triIter].pt[1] = triangleList[triIter].pt[2];
						triangleList[triIter].pt[2] = tempOrder;
						iPointPassesCollision[1] = TRUE;            
						iPointPassesCollision[2] = FALSE;                        
					}
					if( iPointPassesCollision[1] && !iPointPassesCollision[0] ) 
					{
						tempOrder =  triangleList[triIter].pt[0];   
						triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
						triangleList[triIter].pt[1] = tempOrder;
						iPointPassesCollision[0] = TRUE;            
						iPointPassesCollision[1] = FALSE;            
					}

					if( iInsideVertCount == 0 ) 
					{ // All points failed. We're done,  
						triangleList[triIter].culled = true;
					}
					else if( iInsideVertCount == 1 ) 
					{// One point passed. Clip the triangle against the Frustum plane
						triangleList[triIter].culled = false;

						// 
						noVec4 vVert0ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[0];
						noVec4 vVert0ToVert2 = triangleList[triIter].pt[2] - triangleList[triIter].pt[0];

						// Find the collision ratio.
						FLOAT fHitPointTimeRatio = fEdge - triangleList[triIter].pt[0][iComponent] ;
						// Calculate the distance along the vector as ratio of the hit ratio to the component.
						FLOAT fDistanceAlongVector01 = fHitPointTimeRatio /  vVert0ToVert1[iComponent];
						FLOAT fDistanceAlongVector02 = fHitPointTimeRatio /  vVert0ToVert2[iComponent];
						// Add the point plus a percentage of the vector.
						vVert0ToVert1 *= fDistanceAlongVector01;
						vVert0ToVert1 += triangleList[triIter].pt[0];
						vVert0ToVert2 *= fDistanceAlongVector02;
						vVert0ToVert2 += triangleList[triIter].pt[0];

						triangleList[triIter].pt[1] = vVert0ToVert2;
						triangleList[triIter].pt[2] = vVert0ToVert1;

					}
					else if( iInsideVertCount == 2 ) 
					{ // 2 in  // tesselate into 2 triangles


						// Copy the triangle\(if it exists) after the current triangle out of
						// the way so we can override it with the new triangle we're inserting.
						triangleList[iTriangleCnt] = triangleList[triIter+1];

						triangleList[triIter].culled = false;
						triangleList[triIter+1].culled = false;

						// Get the vector from the outside point into the 2 inside points.
						noVec4 vVert2ToVert0 = triangleList[triIter].pt[0] - triangleList[triIter].pt[2];
						noVec4 vVert2ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[2];

						// Get the hit point ratio.
						FLOAT fHitPointTime_2_0 =  fEdge - triangleList[triIter].pt[2][iComponent] ;
						FLOAT fDistanceAlongVector_2_0 = fHitPointTime_2_0 / vVert2ToVert0[iComponent];
						// Calcaulte the new vert by adding the percentage of the vector plus point 2.
						vVert2ToVert0 *= fDistanceAlongVector_2_0;
						vVert2ToVert0 += triangleList[triIter].pt[2];

						// Add a new triangle.
						triangleList[triIter+1].pt[0] = triangleList[triIter].pt[0];
						triangleList[triIter+1].pt[1] = triangleList[triIter].pt[1];
						triangleList[triIter+1].pt[2] = vVert2ToVert0;

						//Get the hit point ratio.
						FLOAT fHitPointTime_2_1 =  fEdge - triangleList[triIter].pt[2][iComponent] ;
						FLOAT fDistanceAlongVector_2_1 = fHitPointTime_2_1 / vVert2ToVert1[iComponent];
						vVert2ToVert1 *= fDistanceAlongVector_2_1;
						vVert2ToVert1 += triangleList[triIter].pt[2];
						triangleList[triIter].pt[0] = triangleList[triIter+1].pt[1];
						triangleList[triIter].pt[1] = triangleList[triIter+1].pt[2];
						triangleList[triIter].pt[2] = vVert2ToVert1;
						// Cncrement triangle count and skip the triangle we just inserted.
						++iTriangleCnt;
						++triIter;


					}
					else 
					{ // all in
						triangleList[triIter].culled = false;

					}
				}// end if !culled loop            
			}
		}
		for( INT index=0; index < iTriangleCnt; ++index ) 
		{
			if( !triangleList[index].culled ) 
			{
				// Set the near and far plan and the min and max z values respectivly.
				for( int vertind = 0; vertind < 3; ++ vertind ) 
				{
					float fTriangleCoordZ = ( triangleList[index].pt[vertind] ).z;
					if( fNearPlane > fTriangleCoordZ ) 
					{
						fNearPlane = fTriangleCoordZ;
					}
					if( fFarPlane  <fTriangleCoordZ ) 
					{
						fFarPlane = fTriangleCoordZ;
					}
				}
			}
		}
	}    
}

//--------------------------------------------------------------------------------------
// This function takes the camera's projection matrix and returns the 8
// points that make up a view frustum.
// The frustum is scaled to fit within the Begin and End interval paramaters.
//--------------------------------------------------------------------------------------
void CascadedShadowsManager::CreateFrustumPointsFromCascadeInterval( FLOAT fCascadeIntervalBegin, FLOAT fCascadeIntervalEnd, noMat4& vProjection, noVec4* pvCornerPointsWorld )
{
	Frustum vViewFrust;
	MathHelpers::ComputeFrustumFromProjection( &vViewFrust, vProjection.ToFloatPtr() );
	vViewFrust.Near = fCascadeIntervalBegin;
	vViewFrust.Far = fCascadeIntervalEnd;

	//static const noVec4 vGrabY = {0x00000000,0xFFFFFFFF,0x00000000,0x00000000};
	//static const noVec4 vGrabX = {0xFFFFFFFF,0x00000000,0x00000000,0x00000000};

	noVec4 vRightTop(vViewFrust.RightSlope,vViewFrust.TopSlope,1.0f,1.0f);
	noVec4 vLeftBottom(vViewFrust.LeftSlope,vViewFrust.BottomSlope,1.0f,1.0f);
	noVec4 vNear(vViewFrust.Near,vViewFrust.Near,vViewFrust.Near,1.0f);
	noVec4 vFar(vViewFrust.Far,vViewFrust.Far,vViewFrust.Far,1.0f);
	noVec4 vRightTopNear = vRightTop.Multiply( vNear );
	noVec4 vRightTopFar =  vRightTop.Multiply( vFar );
	noVec4 vLeftBottomNear = vLeftBottom.Multiply( vNear );
	noVec4 vLeftBottomFar = vLeftBottom.Multiply( vFar );

	pvCornerPointsWorld[0] = vRightTopNear;

	noVec4 cpt1 = vRightTopNear;
	float xOffset = cpt1.x - vLeftBottomNear.x;
	cpt1.x -= xOffset;
	pvCornerPointsWorld[1] =  cpt1;
	pvCornerPointsWorld[2] = vLeftBottomNear;
	noVec4 cpt3 = vRightTopNear;
	float yOffset = cpt3.y - vRightTopNear.y;
	cpt3.y -= yOffset;
	pvCornerPointsWorld[3] = cpt3;

	pvCornerPointsWorld[4] = vRightTopFar;
	noVec4 cpt5 = vRightTopFar;
	 xOffset = cpt5.x - vLeftBottomFar.x;
	cpt5.x -= xOffset;
	pvCornerPointsWorld[5] =  cpt5;
	pvCornerPointsWorld[6] = vLeftBottomFar;
	noVec4 cpt7 = vRightTopFar;
	yOffset = cpt7.y - vLeftBottomFar.y;
	cpt7.y -= yOffset;
	pvCornerPointsWorld[7] = cpt7;
}


void CascadedShadowsManager::ReleaseAndAllocateNewShadowResources( ID3D11Device* pd3dDevice )
{
	HRESULT hr = S_OK;
	// If any of these 3 paramaters was changed, we must reallocate the D3D resources.
	if( m_CopyOfCascadeConfig.m_nCascadeLevels != m_pCascadeConfig->m_nCascadeLevels 
		|| m_CopyOfCascadeConfig.m_ShadowBufferFormat != m_pCascadeConfig->m_ShadowBufferFormat 
		|| m_CopyOfCascadeConfig.m_iBufferSize != m_pCascadeConfig->m_iBufferSize )
	{

		m_CopyOfCascadeConfig = *m_pCascadeConfig;        

		SAFE_RELEASE( m_pSamLinear );
		SAFE_RELEASE( m_pSamShadowPCF );
		SAFE_RELEASE( m_pSamShadowPoint );

		D3D11_SAMPLER_DESC SamDesc;
		SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		SamDesc.MipLODBias = 0.0f;
		SamDesc.MaxAnisotropy = 1;
		SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 0;
		SamDesc.MinLOD = 0;
		SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
		pd3dDevice->CreateSamplerState( &SamDesc, &m_pSamLinear );		

		D3D11_SAMPLER_DESC SamDescShad = 
		{
			D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,// D3D11_FILTER Filter;
			D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressU;
			D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressV;
			D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressW;
			0,//FLOAT MipLODBias;
			0,//UINT MaxAnisotropy;
			D3D11_COMPARISON_LESS , //D3D11_COMPARISON_FUNC ComparisonFunc;
			0.0,0.0,0.0,0.0,//FLOAT BorderColor[ 4 ];
			0,//FLOAT MinLOD;
			0//FLOAT MaxLOD;   
		};

		pd3dDevice->CreateSamplerState( &SamDescShad, &m_pSamShadowPCF );		

		SamDescShad.MaxAnisotropy = 15;
		SamDescShad.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		SamDescShad.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		SamDescShad.Filter = D3D11_FILTER_ANISOTROPIC;
		SamDescShad.ComparisonFunc = D3D11_COMPARISON_NEVER;
		pd3dDevice->CreateSamplerState( &SamDescShad, &m_pSamShadowPoint );		

		for (INT index=0; index < m_CopyOfCascadeConfig.m_nCascadeLevels; ++index ) 
		{ 
			m_RenderVP[index].Height = (FLOAT)m_CopyOfCascadeConfig.m_iBufferSize;
			m_RenderVP[index].Width = (FLOAT)m_CopyOfCascadeConfig.m_iBufferSize;
			m_RenderVP[index].MaxDepth = 1.0f;
			m_RenderVP[index].MinDepth = 0.0f;
			m_RenderVP[index].TopLeftX = (FLOAT)( m_CopyOfCascadeConfig.m_iBufferSize * index );
			m_RenderVP[index].TopLeftY = 0;
		}

		m_RenderOneTileVP.Height = (FLOAT)m_CopyOfCascadeConfig.m_iBufferSize;
		m_RenderOneTileVP.Width = (FLOAT)m_CopyOfCascadeConfig.m_iBufferSize;
		m_RenderOneTileVP.MaxDepth = 1.0f;
		m_RenderOneTileVP.MinDepth = 0.0f;
		m_RenderOneTileVP.TopLeftX = 0.0f;
		m_RenderOneTileVP.TopLeftY = 0.0f;

		SAFE_RELEASE( m_pCascadedShadowMapSRV );
		SAFE_RELEASE( m_pCascadedShadowMapTexture );
		SAFE_RELEASE( m_pCascadedShadowMapDSV );

		DXGI_FORMAT texturefmt = DXGI_FORMAT_R32_TYPELESS;
		DXGI_FORMAT SRVfmt = DXGI_FORMAT_R32_FLOAT;
		DXGI_FORMAT DSVfmt = DXGI_FORMAT_D32_FLOAT;

		switch ( m_CopyOfCascadeConfig.m_ShadowBufferFormat ) 
		{
		case CASCADE_DXGI_FORMAT_R32_TYPELESS : 
			texturefmt = DXGI_FORMAT_R32_TYPELESS;
			SRVfmt = DXGI_FORMAT_R32_FLOAT;
			DSVfmt = DXGI_FORMAT_D32_FLOAT;
			break;
		case CASCADE_DXGI_FORMAT_R24G8_TYPELESS : 
			texturefmt = DXGI_FORMAT_R24G8_TYPELESS ;
			SRVfmt = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			DSVfmt = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		case CASCADE_DXGI_FORMAT_R16_TYPELESS : 
			texturefmt = DXGI_FORMAT_R16_TYPELESS;
			SRVfmt = DXGI_FORMAT_R16_UNORM;
			DSVfmt = DXGI_FORMAT_D16_UNORM;
			break;
		case CASCADE_DXGI_FORMAT_R8_TYPELESS : 
			texturefmt = DXGI_FORMAT_R8_TYPELESS;
			SRVfmt = DXGI_FORMAT_R8_UNORM;
			DSVfmt = DXGI_FORMAT_R8_UNORM;
			break;
		}

		D3D11_TEXTURE2D_DESC dtd = 
		{
			m_CopyOfCascadeConfig.m_iBufferSize * m_CopyOfCascadeConfig.m_nCascadeLevels,//UINT Width;
			m_CopyOfCascadeConfig.m_iBufferSize,//UINT Height;
			1,//UINT MipLevels;
			1,//UINT ArraySize;
			texturefmt,//DXGI_FORMAT Format;
			1,//DXGI_SAMPLE_DESC SampleDesc;
			0,
			D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
			D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
			0,//UINT CPUAccessFlags;
			0//UINT MiscFlags;    
		};

		pd3dDevice->CreateTexture2D( &dtd, NULL, &m_pCascadedShadowMapTexture  );
		

		D3D11_DEPTH_STENCIL_VIEW_DESC  dsvd = 
		{
			DSVfmt,
			D3D11_DSV_DIMENSION_TEXTURE2D,
			0
		};
		pd3dDevice->CreateDepthStencilView( m_pCascadedShadowMapTexture, &dsvd, &m_pCascadedShadowMapDSV ); 
	

		D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd = 
		{
			SRVfmt,
			D3D11_SRV_DIMENSION_TEXTURE2D,
			0,
			0
		};
		dsrvd.Texture2D.MipLevels = 1;

		pd3dDevice->CreateShaderResourceView( m_pCascadedShadowMapTexture, &dsrvd, &m_pCascadedShadowMapSRV );
		
	}
}

void CascadedShadowsManager::Init( const BoundingBox& aabb, ShadowMapCamera* pkViewCamera, ShadowMapCamera* pkLightCamera )
{
	Renderer* renderer = GetApp()->getRenderer();
	ID3D11Device* pd3dDevice = D3D11Dev();

	
	m_vSceneAABBMin = noVec4(aabb.m_vMin, 1); 
	m_vSceneAABBMax = noVec4(aabb.m_vMax, 1);
	// Calcaulte the AABB for the scene by iterating through all the meshes in the SDKMesh file.
	//calculatebb;
	
	m_pViewCamera = pkViewCamera;          
	m_pLightCamera = pkLightCamera;         

	sprintf_s( m_cvsModel, "vs_5_0");
	sprintf_s( m_cpsModel, "ps_5_0");
	sprintf_s( m_cgsModel, "gs_5_0");

	//if ( m_vsorthoShadowShader == -1 ) 
	{
		CompileShaderFromFile( 
			"DATA/Shaders/RenderCascadeShadow.hlsl", NULL, "VSMain", m_cvsModel, &m_pvsRenderOrthoShadowBlob );
		//m_vsorthoShadowShader = renderer->addShader("Data/Shaders/CascadeShadowVS.hlsl");
	}

	pd3dDevice->CreateVertexShader( 
		m_pvsRenderOrthoShadowBlob->GetBufferPointer(), m_pvsRenderOrthoShadowBlob->GetBufferSize(), 
		NULL, &m_pvsRenderOrthoShadow );
	
	// In order to compile optimal versions of each shaders,compile out 64 versions of the same file.  
	// The if statments are dependent upon these macros.  This enables the compiler to optimize out code that can never be reached.
	// D3D11 Dynamic shader linkage would have this same effect without the need to compile 64 versions of the shader.
	D3D_SHADER_MACRO defines[] = 
	{
		"CASCADE_COUNT_FLAG", "1",
		"USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG", "0",
		"BLEND_BETWEEN_CASCADE_LAYERS_FLAG", "0",
		"SELECT_CASCADE_BY_INTERVAL_FLAG", "0",
		NULL, NULL
	};

	char cCascadeDefinition[32];
	char cDerivativeDefinition[32];
	char cBlendDefinition[32];
	char cIntervalDefinition[32];

	for( INT iCascadeIndex=0; iCascadeIndex < MAX_CASCADES; ++iCascadeIndex ) 
	{
		// There is just one vertex shader for the scene.                
		sprintf_s( cCascadeDefinition, "%d", iCascadeIndex + 1 );
		defines[0].Definition = cCascadeDefinition;
		defines[1].Definition = "0";
		defines[2].Definition = "0";
		defines[3].Definition = "0";
		// We don't want to release the last pVertexShaderBuffer until we create the input layout. 

		//if (m_vsSceneShaderiCascadeIndex == -1)
		//if ( m_pvsRenderSceneBlob[iCascadeIndex] == NULL ) 
		{
			//renderer->addShader("Data/Shaders/CascadeShadowSceneVS.hlsl");
			CompileShaderFromFile( "DATA/Shaders/RenderCascadeScene.hlsl", defines, "VSMain", 
				m_cvsModel, &m_pvsRenderSceneBlob[iCascadeIndex] );

			pd3dDevice->CreateVertexShader( m_pvsRenderSceneBlob[iCascadeIndex]->GetBufferPointer(), 
				m_pvsRenderSceneBlob[iCascadeIndex]->GetBufferSize(), NULL, &m_pvsRenderScene[iCascadeIndex]);
		}
		
		for( INT iDerivativeIndex=0; iDerivativeIndex < 2; ++iDerivativeIndex ) 
		{
			for( INT iBlendIndex=0; iBlendIndex < 2; ++iBlendIndex ) 
			{
				for( INT iIntervalIndex=0; iIntervalIndex < 2; ++iIntervalIndex ) 
				{
					sprintf_s( cCascadeDefinition, "%d", iCascadeIndex + 1 );
					sprintf_s( cDerivativeDefinition, "%d", iDerivativeIndex );
					sprintf_s( cBlendDefinition, "%d", iBlendIndex );
					sprintf_s( cIntervalDefinition, "%d", iIntervalIndex );

					defines[0].Definition = cCascadeDefinition;
					defines[1].Definition = cDerivativeDefinition;
					defines[2].Definition = cBlendDefinition;
					defines[3].Definition = cIntervalDefinition;


					//if ( m_ppsSceneAllShaders[iCascadeIndex][iDerivativeIndex][iBlendIndex][iIntervalIndex] == -1 ) 
					{
						CompileShaderFromFile( "DATA/Shaders/RenderCascadeScene.hlsl", defines, "PSMain", 
							m_cpsModel, 
							&m_ppsRenderSceneAllShadersBlob[iCascadeIndex][iDerivativeIndex][iBlendIndex][iIntervalIndex] );
						//m_aaSceneAllShaders[iCascadeIndex][iDerivativeIndex][iBlendIndex][iIntervalIndex] =
						//	renderer->addShader("Data/CascadeShadowScenePS.hlsl");

						pd3dDevice->CreatePixelShader( 
							m_ppsRenderSceneAllShadersBlob[iCascadeIndex][iDerivativeIndex][iBlendIndex][iIntervalIndex]->GetBufferPointer(),
							m_ppsRenderSceneAllShadersBlob[iCascadeIndex][iDerivativeIndex][iBlendIndex][iIntervalIndex]->GetBufferSize(), 
							NULL, 
							&m_ppsRenderSceneAllShaders[iCascadeIndex][iDerivativeIndex][iBlendIndex][iIntervalIndex] );
					}					
				}
			}
		}
	}

	//FormatDesc vbFmt[] = {
	//	0, TYPE_VERTEX, FORMAT_FLOAT,  3,
	//	0, TYPE_NORMAL, FORMAT_FLOAT,   3,
	//	0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,					
	//	//0, TYPE_COLOR, FORMAT_FLOAT, 4,				
	//};
	//m_vb = gRenderer->addVertexFormat(vbFmt, elementsOf(vbFmt), m_vsSceneShader[0]);
	const D3D11_INPUT_ELEMENT_DESC layout_mesh[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	pd3dDevice->CreateInputLayout( 
		layout_mesh, ARRAYSIZE( layout_mesh ), 
		m_pvsRenderSceneBlob[0]->GetBufferPointer(),
		m_pvsRenderSceneBlob[0]->GetBufferSize(), 
		&m_pVertexLayoutMesh );

	D3D11_RASTERIZER_DESC drd = 
	{
		D3D11_FILL_SOLID,//D3D11_FILL_MODE FillMode;
		D3D11_CULL_NONE,//D3D11_CULL_MODE CullMode;
		FALSE,//BOOL FrontCounterClockwise;
		0,//INT DepthBias;
		0.0,//FLOAT DepthBiasClamp;
		0.0,//FLOAT SlopeScaledDepthBias;
		TRUE,//BOOL DepthClipEnable;
		FALSE,//BOOL ScissorEnable;
		TRUE,//BOOL MultisampleEnable;
		FALSE//BOOL AntialiasedLineEnable;   
	};
		
	//rd.cull = gRenderer->addRasterizerState(CULL_BACK);
	pd3dDevice->CreateRasterizerState( &drd, &m_prsScene );
	// Setting the slope scale depth biase greatly decreases surface acne and incorrect self shadowing.
	drd.SlopeScaledDepthBias = 1.0;
	pd3dDevice->CreateRasterizerState( &drd, &m_prsShadow );	
	drd.DepthClipEnable = false;
	pd3dDevice->CreateRasterizerState( &drd, &m_prsShadowPancake );
	
	D3D11_BUFFER_DESC Desc;
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.MiscFlags = 0;

	Desc.ByteWidth = sizeof( CB_ALL_SHADOW_DATA );
	pd3dDevice->CreateBuffer( &Desc, NULL, &m_pcbGlobalConstantBuffer );
}


// This function converts the "center, extents" version of an AABB into 8 points.
//--------------------------------------------------------------------------------------
void CascadedShadowsManager::CreateAABBPoints( noVec4* vAABBPoints, const noVec4& vCenter, const noVec4& vExtents )
{
	//This map enables us to use a for loop and do vector math.
	static const noVec4 vExtentsMap[] = 
	{ 
		noVec4(1.0f, 1.0f, -1.0f, 1.0f), 
		noVec4(-1.0f, 1.0f, -1.0f, 1.0f), 
		noVec4(1.0f, -1.0f, -1.0f, 1.0f), 
		noVec4(-1.0f, -1.0f, -1.0f, 1.0f), 
		noVec4(1.0f, 1.0f, 1.0f, 1.0f), 
		noVec4(-1.0f, 1.0f, 1.0f, 1.0f), 
		noVec4(1.0f, -1.0f, 1.0f, 1.0f), 
		noVec4(-1.0f, -1.0f, 1.0f, 1.0f) 
	};

	for( INT index = 0; index < 8; ++index ) 
	{
		vAABBPoints[index] = noVec4(vExtentsMap[index][0]  *  vExtents[0] + vCenter[0],  
			vExtentsMap[index][1]  *  vExtents[1]+ vCenter[1] ,
		vExtentsMap[index][2]  *  vExtents[2] + vCenter[2],
		vExtentsMap[index][3]  *  vExtents[3] + vCenter[3]); 
	}

}
//--------------------------------------------------------------------------------------
// This function is where the real work is done. We determin the matricies and constants used in 
// shadow generation and scene generation.
//--------------------------------------------------------------------------------------
void CascadedShadowsManager::InitFrame ( ID3D11Device* pd3dDevice)
{
	ReleaseAndAllocateNewShadowResources(pd3dDevice);

	noMat4 view(m_pViewCamera->getViewMatrix());
	noMat4 proj(m_pViewCamera->getProjectionMatrix());
	noMat4 viewproj = view * proj;
	viewproj.TransposeSelf();
	proj.TransposeSelf();
		
	noMat4 lightView(m_pLightCamera->getViewMatrix());
	lightView.TransposeSelf();
	
	float invV[16];
	m_pViewCamera->computeInverseViewMatrix(invV);
	noMat4 invView(invV);
	invView.TransposeSelf();
	
	
	// Convert from min max representation to center extents represnetation.
	// This will make it easier to pull the points out of the transformation.
	noVec4 vSceneCenter = m_vSceneAABBMin + m_vSceneAABBMax;
	vSceneCenter = vSceneCenter.Multiply(g_vHalfVector);
	noVec4 vSceneExtents = m_vSceneAABBMax - m_vSceneAABBMin;
	vSceneExtents =vSceneExtents.Multiply(g_vHalfVector);    

	noVec4 vSceneAABBPointsLightSpace[8];
	// This function simply converts the center and extents of an AABB into 8 points
	CreateAABBPoints( vSceneAABBPointsLightSpace, vSceneCenter, vSceneExtents );
	// Transform the scene AABB to Light space.
	for( int index =0; index < 8; ++index ) 
	{
		vSceneAABBPointsLightSpace[index] =  lightView  * vSceneAABBPointsLightSpace[index]; 
	}

	FLOAT fFrustumIntervalBegin, fFrustumIntervalEnd;
	noVec4 vLightCameraOrthographicMin;  // light space frustrum aabb 
	noVec4 vLightCameraOrthographicMax;
	FLOAT fCameraNearFarRange = m_pViewCamera->GetFar() - m_pViewCamera->GetNear();

	noVec4 vWorldUnitsPerTexel = g_vZero; 

	// We loop over the cascades to calculate the orthographic projection for each cascade.
	for( INT iCascadeIndex=0; iCascadeIndex < m_CopyOfCascadeConfig.m_nCascadeLevels; ++iCascadeIndex ) 
	{
		// Calculate the interval of the View Frustum that this cascade covers. We measure the interval 
		// the cascade covers as a Min and Max distance along the Z Axis.
		if( m_eSelectedCascadesFit == FIT_TO_CASCADES ) 
		{
			// Because we want to fit the orthogrpahic projection tightly around the Cascade, we set the Mimiumum cascade 
			// value to the previous Frustum end Interval
			if( iCascadeIndex==0 ) fFrustumIntervalBegin = 0.0f;
			else fFrustumIntervalBegin = (FLOAT)m_iCascadePartitionsZeroToOne[ iCascadeIndex - 1 ];
		} 
		else 
		{
			// In the FIT_TO_SCENE technique the Cascades overlap eachother.  In other words, interval 1 is coverd by
			// cascades 1 to 8, interval 2 is covered by cascades 2 to 8 and so forth.
			fFrustumIntervalBegin = 0.0f;
		}

		// Scale the intervals between 0 and 1. They are now percentages that we can scale with.
		fFrustumIntervalEnd = (FLOAT)m_iCascadePartitionsZeroToOne[ iCascadeIndex ];        
		fFrustumIntervalBegin/= (FLOAT)m_iCascadePartitionsMax;
		fFrustumIntervalEnd/= (FLOAT)m_iCascadePartitionsMax;
		fFrustumIntervalBegin = fFrustumIntervalBegin * fCameraNearFarRange;
		fFrustumIntervalEnd = fFrustumIntervalEnd * fCameraNearFarRange;
		noVec4 vFrustumPoints[8];

		// This function takes the began and end intervals along with the projection matrix and returns the 8
		// points that repreresent the cascade Interval
		CreateFrustumPointsFromCascadeInterval( fFrustumIntervalBegin, fFrustumIntervalEnd, 
			proj, vFrustumPoints );

		vLightCameraOrthographicMin = g_vFLTMAX;
		vLightCameraOrthographicMax = g_vFLTMIN;

		noVec4 vTempTranslatedCornerPoint;
		// This next section of code calculates the min and max values for the orthographic projection.
		for( int icpIndex=0; icpIndex < 8; ++icpIndex ) 
		{
			// Transform the frustum from camera view space to world space.
			vFrustumPoints[icpIndex] =  invView * vFrustumPoints[icpIndex];
			// Transform the point from world space to Light Camera Space.
			 vTempTranslatedCornerPoint= lightView  *  vFrustumPoints[icpIndex];
			// Find the closest point.
			noVec4 temp = vTempTranslatedCornerPoint;
			temp.Min( vLightCameraOrthographicMin );
			vLightCameraOrthographicMin = temp;
			temp = vTempTranslatedCornerPoint;
			temp.Max( vLightCameraOrthographicMax );
			vLightCameraOrthographicMax = temp;
		}

		// This code removes the shimmering effect along the edges of shadows due to
		// the light changing to fit the camera.
		if( m_eSelectedCascadesFit == FIT_TO_SCENE ) 
		{
			// Fit the ortho projection to the cascades far plane and a near plane of zero. 
			// Pad the projection to be the size of the diagonal of the Frustum partition. 
			// 
			// To do this, we pad the ortho transform so that it is always big enough to cover 
			// the entire camera view frustum.
			noVec4 vDiagonal = vFrustumPoints[0] - vFrustumPoints[6];
			float len = ( vDiagonal ).Length();
			vDiagonal.SetAll(len);

			// The bound is the length of the diagonal of the frustum interval.
			FLOAT fCascadeBound = ( vDiagonal ).x;

			// The offset calculated will pad the ortho projection so that it is always the same size 
			// and big enough to cover the entire cascade interval.
			noVec4 vBoarderOffset = ( vDiagonal - 
				( vLightCameraOrthographicMax - vLightCameraOrthographicMin ) ) 
				.Multiply( g_vHalfVector );
			// Set the Z and W components to zero.
			vBoarderOffset = vBoarderOffset.Multiply(g_vMultiplySetzwToZero);

			// Add the offsets to the projection.
			vLightCameraOrthographicMax += vBoarderOffset;
			vLightCameraOrthographicMin -= vBoarderOffset;

			// The world units per texel are used to snap the shadow the orthographic projection
			// to texel sized increments.  This keeps the edges of the shadows from shimmering.
			FLOAT fWorldUnitsPerTexel = fCascadeBound / (float)m_CopyOfCascadeConfig.m_iBufferSize;
			vWorldUnitsPerTexel.Set( fWorldUnitsPerTexel, fWorldUnitsPerTexel, 0.0f, 0.0f ); 


		} 
		else if( m_eSelectedCascadesFit == FIT_TO_CASCADES ) 
		{

			// We calculate a looser bound based on the size of the PCF blur.  This ensures us that we're 
			// sampling within the correct map.
			float fScaleDuetoBlureAMT = ( (float)( m_iPCFBlurSize * 2 + 1 ) 
				/(float)m_CopyOfCascadeConfig.m_iBufferSize );
			noVec4 vScaleDuetoBlureAMT(fScaleDuetoBlureAMT, fScaleDuetoBlureAMT, 0.0f, 0.0f );


			float fNormalizeByBufferSize = ( 1.0f / (float)m_CopyOfCascadeConfig.m_iBufferSize );
			noVec4 vNormalizeByBufferSize( fNormalizeByBufferSize, fNormalizeByBufferSize, 0.0f, 0.0f );

			// We calculate the offsets as a percentage of the bound.
			noVec4 vBoarderOffset = vLightCameraOrthographicMax - vLightCameraOrthographicMin;
			vBoarderOffset =vBoarderOffset.Multiply(g_vHalfVector);
			vBoarderOffset = vBoarderOffset .Multiply( vScaleDuetoBlureAMT);
			vLightCameraOrthographicMax += vBoarderOffset;
			vLightCameraOrthographicMin -= vBoarderOffset;

			// The world units per texel are used to snap  the orthographic projection
			// to texel sized increments.  
			// Because we're fitting tighly to the cascades, the shimmering shadow edges will still be present when the 
			// camera rotates.  However, when zooming in or strafing the shadow edge will not shimmer.
			vWorldUnitsPerTexel = vLightCameraOrthographicMax - vLightCameraOrthographicMin;
			vWorldUnitsPerTexel = vWorldUnitsPerTexel.Multiply( vNormalizeByBufferSize );

		}
		float fLightCameraOrthographicMinZ =  vLightCameraOrthographicMin.z;


		if( m_bMoveLightTexelSize ) 
		{

			// We snape the camera to 1 pixel increments so that moving the camera does not cause the shadows to jitter.
			// This is a matter of integer dividing by the world space size of a texel
			vLightCameraOrthographicMin /= vWorldUnitsPerTexel;
			vLightCameraOrthographicMin = noVec4(floorf(vLightCameraOrthographicMin[0]), 
				floorf(vLightCameraOrthographicMin[1]), floorf(vLightCameraOrthographicMin[2]), floorf(vLightCameraOrthographicMin[3]));
			vLightCameraOrthographicMin =vLightCameraOrthographicMin.Multiply(vWorldUnitsPerTexel);
			for (size_t i = 0; i < 4; ++i)
				if (vWorldUnitsPerTexel[i] == 0.0f)
					vLightCameraOrthographicMin[i] = 0.0f;

			vLightCameraOrthographicMax /= vWorldUnitsPerTexel;
			vLightCameraOrthographicMax = noVec4(floorf(vLightCameraOrthographicMax[0]),
				floorf(vLightCameraOrthographicMax[1]), floorf(vLightCameraOrthographicMax[2]),
				floorf(vLightCameraOrthographicMax[3]));
			vLightCameraOrthographicMax = vLightCameraOrthographicMax.Multiply(vWorldUnitsPerTexel);
			for (size_t i = 0; i < 4; ++i)
				if (vWorldUnitsPerTexel[i] == 0.0f)
					vLightCameraOrthographicMax[i] = 0.0f;
		}

		//These are the unconfigured near and far plane values.  They are purposly awful to show 
		// how important calculating accurate near and far planes is.
		FLOAT fNearPlane = 0.0f;
		FLOAT fFarPlane = 10000.0f;

		if( m_eSelectedNearFarFit == FIT_NEARFAR_AABB ) 
		{

			noVec4 vLightSpaceSceneAABBminValue = g_vFLTMAX;  // world space scene aabb 
			noVec4 vLightSpaceSceneAABBmaxValue = g_vFLTMIN;       
			// We calculate the min and max vectors of the scene in light space. The min and max "Z" values of the  
			// light space AABB can be used for the near and far plane. This is easier than intersecting the scene with the AABB
			// and in some cases provides similar results.
			for(int index=0; index< 8; ++index) 
			{
				noVec4 temp = vSceneAABBPointsLightSpace[index];
				temp.Min( vLightSpaceSceneAABBminValue );
				vLightSpaceSceneAABBminValue = temp;
				temp = vSceneAABBPointsLightSpace[index];
				temp.Max( vLightSpaceSceneAABBmaxValue );
				vLightSpaceSceneAABBmaxValue = temp;
			}

			// The min and max z values are the near and far planes.
			fNearPlane = ( vLightSpaceSceneAABBminValue ).z;
			fFarPlane = ( vLightSpaceSceneAABBmaxValue ).z;
		} 
		else if( m_eSelectedNearFarFit == FIT_NEARFAR_SCENE_AABB 
			|| m_eSelectedNearFarFit == FIT_NEARFAR_PANCAKING ) 
		{
			// By intersecting the light frustum with the scene AABB we can get a tighter bound on the near and far plane.
			ComputeNearAndFar( fNearPlane, fFarPlane, vLightCameraOrthographicMin, 
				vLightCameraOrthographicMax, vSceneAABBPointsLightSpace );
			if (m_eSelectedNearFarFit == FIT_NEARFAR_PANCAKING )
			{
				if ( fLightCameraOrthographicMinZ > fNearPlane )
				{
					fNearPlane = fLightCameraOrthographicMinZ;
				}
			}
		} 
		else 
		{

		}
		// Craete the orthographic projection for this cascade.
		/*D3DXMatrixOrthoOffCenterLH( &m_matShadowProj[ iCascadeIndex ], 
			noVec3GetX( vLightCameraOrthographicMin ), 
			noVec3GetX( vLightCameraOrthographicMax ), 
			noVec3GetY( vLightCameraOrthographicMin ), 
			noVec3GetY( vLightCameraOrthographicMax ), 
			fNearPlane, fFarPlane );*/
		m_matShadowProj[ iCascadeIndex ] = MatrixOrthoOffCenterLH<mat4>(
			vLightCameraOrthographicMin.x, 
			vLightCameraOrthographicMax.x,
			vLightCameraOrthographicMin.y, 
			vLightCameraOrthographicMax.y, 
			fNearPlane, fFarPlane);

		m_fCascadePartitionsFrustum[ iCascadeIndex ] = fFrustumIntervalEnd;
	}
	noMat4 temp(m_pLightCamera->getViewMatrix());	
	m_matShadowView = ToMat4(temp);	
}

void CascadedShadowsManager::EnableRendering( void )
{
	ID3D11DeviceContext* pd3dDeviceContext = GetApp()->GetContext();
	GetApp()->GetContext()->ClearDepthStencilView( m_pCascadedShadowMapDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
	ID3D11RenderTargetView* pnullView = NULL; 
	// Set a null render target so as not to render color.
	GetApp()->GetContext()->OMSetRenderTargets( 1, &pnullView , m_pCascadedShadowMapDSV );
	
	
}

void CascadedShadowsManager::RenderShadowsForAllCascades()
{
	
	for (size_t i = 0; i < GetApp()->GetActorRoot()->childNodes_.size(); ++i)
	{
		Mesh* pMesh = (Mesh*)GetApp()->GetActorRoot()->childNodes_[i];
		if (pMesh->m_bOnlyReceiveShadows) continue;
		RenderShadow(pMesh);
	}	
}

void CascadedShadowsManager::RenderShadow2(CDXUTSDKMesh* pMesh)
{
	
	ID3D11DeviceContext* pd3dDeviceContext = GetApp()->GetContext();

	if ( m_eSelectedNearFarFit == FIT_NEARFAR_PANCAKING )
	{
		pd3dDeviceContext->RSSetState( m_prsShadowPancake );
	}
	else 
	{
		pd3dDeviceContext->RSSetState( m_prsShadow );
	}

	mat4 dxmatWorldViewProjection;

	for( INT currentCascade=0; currentCascade < m_CopyOfCascadeConfig.m_nCascadeLevels; ++currentCascade ) 
	{
		// Setup view port
		GetApp()->GetContext()->RSSetViewports(1, &m_RenderVP[currentCascade]);

		dxmatWorldViewProjection =  m_matShadowView * m_matShadowProj[currentCascade];

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		pd3dDeviceContext->Map( m_pcbGlobalConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
		CB_ALL_SHADOW_DATA* pcbAllShadowConstants = ( CB_ALL_SHADOW_DATA* )MappedResource.pData;
		mat4 matIdentity;		
		matIdentity = identity4();
		//dxmatWorldViewProjection = transpose(dxmatWorldViewProjection);
		pcbAllShadowConstants->m_WorldViewProj = transpose(matIdentity * dxmatWorldViewProjection);

		// The model was exported in world space, so we can pass the identity up as the world transform.
		pcbAllShadowConstants->m_World = transpose( matIdentity );		
		pd3dDeviceContext->Unmap( m_pcbGlobalConstantBuffer, 0 );
		pd3dDeviceContext->IASetInputLayout( m_pVertexLayoutMesh );
		// No pixel shader is bound as we're only writing out depth.
		pd3dDeviceContext->VSSetShader( m_pvsRenderOrthoShadow, NULL, 0 );
		pd3dDeviceContext->PSSetShader( NULL, NULL, 0 );
		pd3dDeviceContext->GSSetShader( NULL, NULL, 0 );
		pd3dDeviceContext->VSSetConstantBuffers( 0, 1, &m_pcbGlobalConstantBuffer );
		
		 pMesh->Render( pd3dDeviceContext, 0, 1 );
	}

	pd3dDeviceContext->RSSetState( NULL );
	ID3D11RenderTargetView* pnullView = NULL; 
	pd3dDeviceContext->OMSetRenderTargets( 1, &pnullView, NULL );
}


void CascadedShadowsManager::RenderShadow(Mesh* pMesh)
{
	ID3D11DeviceContext* pd3dDeviceContext = GetApp()->GetContext();

	if ( m_eSelectedNearFarFit == FIT_NEARFAR_PANCAKING )
	{
		pd3dDeviceContext->RSSetState( m_prsShadowPancake );
	}
	else 
	{
		pd3dDeviceContext->RSSetState( m_prsShadow );
	}

	mat4 dxmatWorldViewProjection;

	for( INT currentCascade=0; currentCascade < m_CopyOfCascadeConfig.m_nCascadeLevels; ++currentCascade ) 
	{		
		// Setup view port
		GetApp()->GetContext()->RSSetViewports(1, &m_RenderVP[currentCascade]);

		dxmatWorldViewProjection =  m_matShadowView * m_matShadowProj[currentCascade];

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		pd3dDeviceContext->Map( m_pcbGlobalConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
		CB_ALL_SHADOW_DATA* pcbAllShadowConstants = ( CB_ALL_SHADOW_DATA* )MappedResource.pData;
		mat4 matIdentity;
		noMat4 tm(pMesh->worldTM_.rotate_ * pMesh->worldTM_.scale_, pMesh->worldTM_.trans_);				
		matIdentity = ToMat4(tm);
		
		pcbAllShadowConstants->m_WorldViewProj = transpose(matIdentity * dxmatWorldViewProjection);
				

		// The model was exported in world space, so we can pass the identity up as the world transform.
		pcbAllShadowConstants->m_World = transpose( matIdentity );
		pd3dDeviceContext->Unmap( m_pcbGlobalConstantBuffer, 0 );
		pd3dDeviceContext->IASetInputLayout( m_pVertexLayoutMesh );
		// No pixel shader is bound as we're only writing out depth.
		pd3dDeviceContext->VSSetShader( m_pvsRenderOrthoShadow, NULL, 0 );
		pd3dDeviceContext->PSSetShader( NULL, NULL, 0 );
		pd3dDeviceContext->GSSetShader( NULL, NULL, 0 );
		pd3dDeviceContext->VSSetConstantBuffers( 0, 1, &m_pcbGlobalConstantBuffer );

	
		pMesh->DrawCascadedShadow();
	}
}

void CascadedShadowsManager::DisableRendering( void )
{		
	GetApp()->SetDefaultRenderTarget();
}

void CascadedShadowsManager::RenderScene2 ( ShadowMapCamera* pActiveCamera, CDXUTSDKMesh* pMesh)
{
	ID3D11DeviceContext* pd3dDeviceContext = D3D11Context();
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dDeviceContext->RSSetState( m_prsScene );
		
	Renderer* renderer = GetApp()->getRenderer();	
	pd3dDeviceContext->IASetInputLayout( m_pVertexLayoutMesh );


	mat4 matCameraProj(ToMat4(noMat4(pActiveCamera->getProjectionMatrix())));
	mat4 matCameraView(ToMat4(noMat4(pActiveCamera->getViewMatrix())));

	mat4 matWorldViewProjection =  matCameraView * matCameraProj;

	pd3dDeviceContext->Map( m_pcbGlobalConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	CB_ALL_SHADOW_DATA* pcbAllShadowConstants = ( CB_ALL_SHADOW_DATA* )MappedResource.pData;
	pcbAllShadowConstants->m_WorldViewProj = transpose( matWorldViewProjection ) ;
	pcbAllShadowConstants->m_WorldView =  transpose(matCameraView) ; 
	// These are the for loop begin end values. 
	pcbAllShadowConstants->m_iPCFBlurForLoopEnd = m_iPCFBlurSize / 2 +1;
	pcbAllShadowConstants->m_iPCFBlurForLoopStart = m_iPCFBlurSize / -2;
	// This is a floating point number that is used as the percentage to blur between maps.    
	pcbAllShadowConstants->m_fCascadeBlendArea = m_fBlurBetweenCascadesAmount;
	pcbAllShadowConstants->m_fTexelSize = 1.0f / (float)m_CopyOfCascadeConfig.m_iBufferSize; 
	pcbAllShadowConstants->m_fNativeTexelSizeInX = pcbAllShadowConstants->m_fTexelSize / m_CopyOfCascadeConfig.m_nCascadeLevels;
	mat4 matIdentity = identity4();;	
	pcbAllShadowConstants->m_World = matIdentity; 
	/*mat4 matTextureScale;
	matTextureScale = scale(0.5f, -0.5f, 1.0f);

	mat4 matTextureTranslation;
	matTextureTranslation = translate( .5,.5,0 );
	mat4 scaleToTile;
	scaleToTile =scale(1.0f / (float)m_pCascadeConfig->m_nCascadeLevels, 1.0, 1.0 );*/

	D3DXMATRIX matTextureScale;
	D3DXMatrixScaling( &matTextureScale,  
		0.5f, 
		-0.5f,
		1.0f );
	D3DXMATRIX matTextureTranslation;
	D3DXMatrixTranslation( &matTextureTranslation, .5,.5,0 );
	D3DXMATRIX scaleToTile;
	D3DXMatrixScaling( &scaleToTile, 1.0f / (float)m_pCascadeConfig->m_nCascadeLevels, 1.0, 1.0 );

	pcbAllShadowConstants->m_fShadowBiasFromGUI = m_fPCFOffset;
	pcbAllShadowConstants->m_fShadowPartitionSize = 1.0f / (float) m_CopyOfCascadeConfig.m_nCascadeLevels;

	pcbAllShadowConstants->m_Shadow =  transpose(m_matShadowView) ;
	for(int index=0; index < m_CopyOfCascadeConfig.m_nCascadeLevels; ++index ) 
	{
		
		D3DXMATRIX mShadowTexture = D3DXMATRIX((float*)&m_matShadowProj[index]) * matTextureScale * matTextureTranslation;
		pcbAllShadowConstants->m_vCascadeScale[index].x = mShadowTexture._11;
		pcbAllShadowConstants->m_vCascadeScale[index].y = mShadowTexture._22;
		pcbAllShadowConstants->m_vCascadeScale[index].z = mShadowTexture._33;
		pcbAllShadowConstants->m_vCascadeScale[index].w = 1;

		pcbAllShadowConstants->m_vCascadeOffset[index].x = mShadowTexture._41;
		pcbAllShadowConstants->m_vCascadeOffset[index].y = mShadowTexture._42;
		pcbAllShadowConstants->m_vCascadeOffset[index].z = mShadowTexture._43;
		pcbAllShadowConstants->m_vCascadeOffset[index].w = 0;
	}


	// Copy intervals for the depth interval selection method.
	memcpy( pcbAllShadowConstants->m_fCascadeFrustumsEyeSpaceDepths, 
		m_fCascadePartitionsFrustum, MAX_CASCADES*4 );
	for( int index=0; index < MAX_CASCADES; ++index ) 
	{
		pcbAllShadowConstants->m_fCascadeFrustumsEyeSpaceDepthsFloat4[index].x = m_fCascadePartitionsFrustum[index];
	}

	// The border padding values keep the pixel shader from reading the borders during PCF filtering.
	pcbAllShadowConstants->m_fMaxBorderPadding = (float)( m_pCascadeConfig->m_iBufferSize  - 1.0f ) / 
		(float)m_pCascadeConfig->m_iBufferSize;
	pcbAllShadowConstants->m_fMinBorderPadding = (float)( 1.0f ) / 
		(float)m_pCascadeConfig->m_iBufferSize;

	noVec3 ep = m_pLightCamera->GetFrom();
	noVec3 lp = m_pLightCamera->GetTo();
	ep -= lp;
	ep.Normalize();

	pcbAllShadowConstants->m_vLightDir = noVec4( ep.x, ep.y, ep.z, 1.0f );
	pcbAllShadowConstants->m_nCascadeLevels = m_CopyOfCascadeConfig.m_nCascadeLevels;
	pcbAllShadowConstants->m_iVisualizeCascades = 0;

	pd3dDeviceContext->Unmap( m_pcbGlobalConstantBuffer, 0 );

	pd3dDeviceContext->PSSetSamplers( 0, 1, &m_pSamLinear );
	pd3dDeviceContext->PSSetSamplers( 1, 1, &m_pSamLinear );

	pd3dDeviceContext->PSSetSamplers( 5, 1, &m_pSamShadowPCF );
	pd3dDeviceContext->GSSetShader( NULL, NULL, 0 );

	pd3dDeviceContext->VSSetShader( m_pvsRenderScene[m_CopyOfCascadeConfig.m_nCascadeLevels-1], NULL, 0 );

	// There are up to 8 cascades, possible derivative based offsets, blur between cascades, 
	// and two cascade selection maps.  This is a total of 64 permutations of the shader.

	pd3dDeviceContext->PSSetShader(
		m_ppsRenderSceneAllShaders[m_CopyOfCascadeConfig.m_nCascadeLevels-1]
	[m_iDerivativeBasedOffset]
	[m_iBlurBetweenCascades]
	[m_eSelectedCascadeSelection],
		NULL, 0 );


	pd3dDeviceContext->PSSetShaderResources( 5,1, &m_pCascadedShadowMapSRV );

	pd3dDeviceContext->VSSetConstantBuffers( 0, 1, &m_pcbGlobalConstantBuffer);
	pd3dDeviceContext->PSSetConstantBuffers( 0, 1, &m_pcbGlobalConstantBuffer);

	// Render
	pMesh->Render( pd3dDeviceContext, 0, 1 );	

	ID3D11ShaderResourceView* nv[] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	pd3dDeviceContext->PSSetShaderResources( 5,8, nv ); 
}

void CascadedShadowsManager::RenderScene( ShadowMapCamera* pActiveCamera, Mesh* pMesh )
{
	ID3D11DeviceContext* pd3dDeviceContext = D3D11Context();
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dDeviceContext->RSSetState( m_prsScene );
		
	ID3D11RenderTargetView* pBackBufferRTV; 
	ID3D11DepthStencilView* pBackBufferDSV;
	Renderer* renderer = GetApp()->getRenderer();	
	pd3dDeviceContext->IASetInputLayout( m_pVertexLayoutMesh );


	mat4 matIdentity;
	noMat4 tm(pMesh->worldTM_.rotate_ * pMesh->worldTM_.scale_, pMesh->worldTM_.trans_);		
	tm.TransposeSelf();	
	mat4 matWorld = ToMat4(tm);;
	mat4 matCameraProj(ToMat4(noMat4(pActiveCamera->getProjectionMatrix())));
	mat4 matCameraView(ToMat4(noMat4(pActiveCamera->getViewMatrix())));

	mat4 matWorldViewProjection =  matWorld * matCameraView * matCameraProj;

	pd3dDeviceContext->Map( m_pcbGlobalConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	CB_ALL_SHADOW_DATA* pcbAllShadowConstants = ( CB_ALL_SHADOW_DATA* )MappedResource.pData;
		
	pcbAllShadowConstants->m_World = transpose(matWorld);
	pcbAllShadowConstants->m_WorldViewProj = transpose( matWorldViewProjection ) ;
	pcbAllShadowConstants->m_WorldView =  transpose(matCameraView ); 
	// These are the for loop begin end values. 
	pcbAllShadowConstants->m_iPCFBlurForLoopEnd = m_iPCFBlurSize / 2 +1;
	pcbAllShadowConstants->m_iPCFBlurForLoopStart = m_iPCFBlurSize / -2;
	// This is a floating point number that is used as the percentage to blur between maps.    
	pcbAllShadowConstants->m_fCascadeBlendArea = m_fBlurBetweenCascadesAmount;
	pcbAllShadowConstants->m_fTexelSize = 1.0f / (float)m_CopyOfCascadeConfig.m_iBufferSize; 
	pcbAllShadowConstants->m_fNativeTexelSizeInX = pcbAllShadowConstants->m_fTexelSize / m_CopyOfCascadeConfig.m_nCascadeLevels;
	
	/*mat4 matTextureScale;
	matTextureScale = scale(0.5f, -0.5f, 1.0f);
		
	mat4 matTextureTranslation;
	matTextureTranslation = translate( .5,.5,0 );
	mat4 scaleToTile;
	scaleToTile =scale(1.0f / (float)m_pCascadeConfig->m_nCascadeLevels, 1.0, 1.0 ); */

	D3DXMATRIX matTextureScale;
	D3DXMatrixScaling( &matTextureScale,  
		0.5f, 
		-0.5f,
		1.0f );
	D3DXMATRIX matTextureTranslation;
	D3DXMatrixTranslation( &matTextureTranslation, .5,.5,0 );
	D3DXMATRIX scaleToTile;
	D3DXMatrixScaling( &scaleToTile, 1.0f / (float)m_pCascadeConfig->m_nCascadeLevels, 1.0, 1.0 );


	pcbAllShadowConstants->m_fShadowBiasFromGUI = m_fPCFOffset;
	pcbAllShadowConstants->m_fShadowPartitionSize = 1.0f / (float) m_CopyOfCascadeConfig.m_nCascadeLevels;

	pcbAllShadowConstants->m_Shadow =  transpose(m_matShadowView) ;
	for(int index=0; index < m_CopyOfCascadeConfig.m_nCascadeLevels; ++index ) 
	{
		D3DXMATRIX mShadowTexture =  D3DXMATRIX((float*)&m_matShadowProj[index])  * matTextureScale * matTextureTranslation;
		pcbAllShadowConstants->m_vCascadeScale[index].x = mShadowTexture._11;
		pcbAllShadowConstants->m_vCascadeScale[index].y = mShadowTexture._22;
		pcbAllShadowConstants->m_vCascadeScale[index].z = mShadowTexture._33;
		pcbAllShadowConstants->m_vCascadeScale[index].w = 1;

		pcbAllShadowConstants->m_vCascadeOffset[index].x = mShadowTexture._41;
		pcbAllShadowConstants->m_vCascadeOffset[index].y = mShadowTexture._42;
		pcbAllShadowConstants->m_vCascadeOffset[index].z = mShadowTexture._43;
		pcbAllShadowConstants->m_vCascadeOffset[index].w = 0;
	}


	// Copy intervals for the depth interval selection method.
	memcpy( pcbAllShadowConstants->m_fCascadeFrustumsEyeSpaceDepths, 
		m_fCascadePartitionsFrustum, MAX_CASCADES*4 );
	for( int index=0; index < MAX_CASCADES; ++index ) 
	{
		pcbAllShadowConstants->m_fCascadeFrustumsEyeSpaceDepthsFloat4[index].x = m_fCascadePartitionsFrustum[index];
	}

	// The border padding values keep the pixel shader from reading the borders during PCF filtering.
	pcbAllShadowConstants->m_fMaxBorderPadding = (float)( m_pCascadeConfig->m_iBufferSize  - 1.0f ) / 
		(float)m_pCascadeConfig->m_iBufferSize;
	pcbAllShadowConstants->m_fMinBorderPadding = (float)( 1.0f ) / 
		(float)m_pCascadeConfig->m_iBufferSize;

	noVec3 ep = m_pLightCamera->GetFrom();
	noVec3 lp = m_pLightCamera->GetTo();
	ep -= lp;
	ep.Normalize();

	pcbAllShadowConstants->m_vLightDir = noVec4( ep.x, ep.y, ep.z, 1.0f );
	pcbAllShadowConstants->m_nCascadeLevels = m_CopyOfCascadeConfig.m_nCascadeLevels;
	pcbAllShadowConstants->m_iVisualizeCascades = 0;
		
	pd3dDeviceContext->Unmap( m_pcbGlobalConstantBuffer, 0 );

	pd3dDeviceContext->PSSetSamplers( 0, 1, &m_pSamLinear );
	pd3dDeviceContext->PSSetSamplers( 1, 1, &m_pSamLinear );

	pd3dDeviceContext->PSSetSamplers( 5, 1, &m_pSamShadowPCF );
	pd3dDeviceContext->GSSetShader( NULL, NULL, 0 );

	pd3dDeviceContext->VSSetShader( m_pvsRenderScene[m_CopyOfCascadeConfig.m_nCascadeLevels-1], NULL, 0 );

	// There are up to 8 cascades, possible derivative based offsets, blur between cascades, 
	// and two cascade selection maps.  This is a total of 64 permutations of the shader.

	pd3dDeviceContext->PSSetShader(
		m_ppsRenderSceneAllShaders[m_CopyOfCascadeConfig.m_nCascadeLevels-1]
	[m_iDerivativeBasedOffset]
	[m_iBlurBetweenCascades]
	[m_eSelectedCascadeSelection],
		NULL, 0 );

	
	pd3dDeviceContext->PSSetShaderResources( 5,1, &m_pCascadedShadowMapSRV );

	pd3dDeviceContext->VSSetConstantBuffers( 0, 1, &m_pcbGlobalConstantBuffer);
	pd3dDeviceContext->PSSetConstantBuffers( 0, 1, &m_pcbGlobalConstantBuffer);

	// Render
	pMesh->DrawCascaded();

	ID3D11ShaderResourceView* nv[] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	pd3dDeviceContext->PSSetShaderResources( 5,8, nv ); 
}

