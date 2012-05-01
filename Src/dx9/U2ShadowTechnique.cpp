#include <U2_3D/src/U23DLibPCH.h>
#include "U2ShadowTechnique.h"
#include "U2Dx9ShadowMgr.h"
#include "U2_3D/src/collision/U2Cone.h"


U2ShadowTechnique::U2ShadowTechnique()
:m_iDepthBias(4),
m_fBiasSlope(1.0f)
{

}

U2ShadowTechnique::~U2ShadowTechnique()
{

}

bool U2ShadowTechnique::CreateShadowMap()
{
	IsRendererOK2;

	D3DFORMAT zFormat = D3DFMT_D24S8;

	D3DFORMAT colorFormat = D3DFMT_A8R8G8B8;

	unsigned int shadowTexWidth = (m_bSupportsPixelShaders20) ? TEXDEPTH_WIDTH_20 : TEXDEPTH_SIZE_11;
	unsigned int shadowTexHeight = (m_bSupportsPixelShaders20) ? TEXDEPTH_HEIGHT_20 : TEXDEPTH_SIZE_11;
	

	m_spPSMColorTex = U2Dx9Texture::CreateRendered(shadowTexWidth, shadowTexHeight, pRenderer, colorFormat);

	m_spShadowMapRT = U2RenderTargets::Create(1, pRenderer);
	U2ASSERT(m_spShadowMapRT);

	U2Dx9FrameBuffer* pDSBuffer = U2Dx9FrameBuffer::CreateAddedDepthStencilBuffer
		(shadowTexWidth, shadowTexHeight, pRenderer->GetD3DDevice(), zFormat);


	m_spShadowMapRT->AttachBackBuffer(m_spPSMColorTex->m_sp2DBuffer, 0);
	m_spShadowMapRT->AttachDSBuffer(pDSBuffer);

	return true;
}

bool U2ShadowTechnique::Initialize()
{
	IsRendererOK2;

	IDirect3DDevice9* pD3DDev = pRenderer->GetD3DDevice();

	D3DCAPS9 deviceCaps;
	pD3DDev->GetDeviceCaps( &deviceCaps );

	m_bSupportsPixelShaders20 = (deviceCaps.PixelShaderVersion >= D3DPS_VERSION(2,0));

	m_spShadowCam = U2_NEW U2Camera();
	if (!m_spShadowCam)
	{
		return false;
	}
	
	m_spShadowCam->SetViewport(0.0f, 1.0f, 1.0f, 0.0f);
	

	const U2FrameShader& frameShader = U2SceneMgr::Instance()->m_spFrame->GetShader(_T("psm"));
	m_spShadowMapShader = frameShader.GetShader();
	if (!m_spShadowMapShader)
	{

		return false;
	}

	CreateShadowMap();
	CreateQuad();

	 

	
	return true;

}

void U2ShadowTechnique::Terminate()
{

}


bool U2ShadowTechnique::CreateQuad()
{
	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();

	// Create ScreenQuad		
	U2TriStripData* pData = U2_NEW U2TriStripData();


	m_spSMQuad = U2_NEW U2N2Mesh(pData);
	m_spSMQuad->SetUsages(U2N2Mesh::WriteOnly);
	m_spSMQuad->SetVertexComponents(U2N2Mesh::Coord | U2N2Mesh::Normal);
	pData->SetVertexCount(4, 4);
	// Triangle List
	
	uint16* pusArrayLengths = U2_ALLOC(uint16, 1);
	pusArrayLengths[0] = 4;

	int ibSize = sizeof(uint16) * 4;
	uint16 *pIdxArray = (uint16*)U2_MALLOC(ibSize);
	pIdxArray[0] = 0; pIdxArray[1] = 1;	pIdxArray[2] = 2; pIdxArray[3] = 3;

	pData->SetIndices(2, 2, pIdxArray, pusArrayLengths, 1);

	m_spSMQuad->CreateEmptyBuffers();

	float *pV = m_spSMQuad->LockVertices();
	U2ASSERT(pV);

	float value = SMQUAD_SIZE;
	*pV++ = -value; *pV++ = -10.0f;	*pV++ = value;	*pV++ = 0.0f; *pV++ = 1.0f; *pV++ = 0.0f;
	*pV++ = value; *pV++ = -10.0f;	*pV++ = value;	*pV++ = 0.0f; *pV++ = 1.0f; *pV++ = 0.0f;
	*pV++ = -value; *pV++ = -10.0f;	*pV++ = -value;	*pV++ = 0.0f; *pV++ = 1.0f; *pV++ = 0.0f;
	*pV++ = value; *pV++ = -10.0f;	*pV++ = -value;	*pV++ = 0.0f; *pV++ = 1.0f; *pV++ = 0.0f;

	m_spSMQuad->UnlockVertices();


	uint16* pI = m_spSMQuad->LockIndices();
	U2ASSERT(pI);

	*pI++ = 0;	*pI++ = 1;	*pI++ = 2;	*pI++ = 3;
	
	m_spSMQuad->UnlockIndices();	

	return true;
}

void U2ShadowTechnique::DrawQuad()
{
	IsRendererOK;

	m_spShadowMapShader->SetTechnique(_T("UseR32FShadowMap"));

	m_spShadowMapShader->SetTexture(U2FxShaderState::ShadowColor, m_spPSMColorTex);
	m_spShadowMapShader->SetTexture(U2FxShaderState::ShadowModMap, m_spPSMColorTex);

	//set special texture matrix for shadow mapping
	float fOffsetX = 0.5f + (0.5f / (float)((m_bSupportsPixelShaders20)?TEXDEPTH_WIDTH_20:TEXDEPTH_SIZE_11));
	float fOffsetY = 0.5f + (0.5f / (float)((m_bSupportsPixelShaders20)?TEXDEPTH_HEIGHT_20:TEXDEPTH_SIZE_11));
	unsigned int range = 1;
	float fBias    = 0.0f;
	D3DXMATRIX texScaleBiasMat( 0.5f,     0.0f,     0.0f,         0.0f,
		0.0f,    -0.5f,     0.0f,         0.0f,
		0.0f,     0.0f,     (float)range, 0.0f,
		fOffsetX, fOffsetY, 0.0f,         1.0f );

	D3DXMATRIX textureMatrix;
	D3DXMatrixMultiply(&textureMatrix, &m_LightViewProj, &texScaleBiasMat);

	U2Camera* pActiveCam = U2Dx9ShadowMgr::Instance()->GetActiveCam();

	D3DXMATRIX IdentityTM;
	D3DXMatrixIdentity(&IdentityTM);	
	pRenderer->SetTransform(U2Dx9Renderer::TMT_WORLD, IdentityTM);	
	pRenderer->SetCamera(pActiveCam);
	pRenderer->SetTransform(U2Dx9Renderer::TMT_VIEW, pRenderer->GetViewMat());
	pRenderer->SetProjectionTM(pActiveCam);
	pRenderer->SetTransform(U2Dx9Renderer::TMT_PROJ, pRenderer->GetProjMat());

	pRenderer->PushTransform(U2Dx9Renderer::TMT_TEXTURE0, textureMatrix);

	pRenderer->SetupMesh(m_spSMQuad);
	pRenderer->SetVertexRange(0, 4);
	pRenderer->SetIndexRange(0, 4);
	pRenderer->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP);
	pRenderer->SetupMesh(0);
	pRenderer->PopTransform(U2Dx9Renderer::TMT_TEXTURE0);	
}


void U2ShadowTechnique::UpdateShadowParams()
{
	this->BuildPSMProjectionMatrix();

	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();

	m_spShadowMapShader->SetTechnique(_T("GenHardwareShadowMap20"));

	float depthBias = float(m_iDepthBias) / 16777215.f;

	if (m_bSupportsHWShadowMaps)
	{
		pRenderer->GetD3DDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, (m_bDisplayShadowMap)?0xf : 0);
		pRenderer->GetD3DDevice()->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&depthBias);
		pRenderer->GetD3DDevice()->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&m_fBiasSlope);
	}

}


void U2ShadowTechnique::BeginPass(const nMeshGroup& curMesh)
{
	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();
	pRenderer->SetCurrEffectShader(m_spShadowMapShader);

	D3DXMATRIX hWorldMat;
	curMesh.m_tmWorld.GetHomogeneousMat(hWorldMat);
	pRenderer->PushTransform(U2Dx9Renderer::TMT_WORLD, hWorldMat);
	pRenderer->PushTransform(U2Dx9Renderer::TMT_TEXTURE0, hWorldMat);
	pRenderer->PushTransform(U2Dx9Renderer::TMT_PROJ, m_LightViewProj);
	
	m_uiNumShaderPasses = m_spShadowMapShader->Begin(true);
	if(1 == m_uiNumShaderPasses)
	{
		m_spShadowMapShader->BeginPass(0);
	}
}

void U2ShadowTechnique::EndPass()
{
	if(m_uiNumShaderPasses == 1)
	{
		m_spShadowMapShader->EndPass();
	}

	m_spShadowMapShader->End();

	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();
	pRenderer->PopTransform(U2Dx9Renderer::TMT_PROJ);
	pRenderer->PopTransform(U2Dx9Renderer::TMT_TEXTURE0);
	pRenderer->PopTransform(U2Dx9Renderer::TMT_WORLD);
}

//-----------------------------------------------------------------------------
// Name: BuildPSMProjectionMatrix
// Desc: Builds a perpsective shadow map transformation matrix
//-----------------------------------------------------------------------------
void U2ShadowTechnique::BuildPSMProjectionMatrix()
{
//	 D3DXMATRIX lightView, lightProj, virtualCameraViewProj, virtualCameraView, virtualCameraProj;
//
//	const D3DXVECTOR3 yAxis  ( 0.f, 1.f, 0.f);
//	const D3DXVECTOR3 zAxis  ( 0.f, 0.f, 1.f);
//
//	U2Dx9ShadowMgr* pShadowMgr = U2Dx9ShadowMgr::Instance();
//
//	//  update the virutal scene camera's bounding parameters
////	pShadowMgr->ComputeVirtualCameraParameters();
//
//	//  compute 
//	//  compute a slideback, that force some distance between the infinity plane and the view-box
//	const float Z_EPSILON=0.0001f;
//	float infinity = m_zFar/(m_zFar-m_zNear);
//	float fInfinityZ = m_fMinInfinityZ;
//	if ( (infinity<=fInfinityZ) && m_bSlideBack)
//	{
//		float additionalSlide = fInfinityZ*(m_zFar-m_zNear) - m_zFar + Z_EPSILON;
//		m_fSlideBack = additionalSlide;
//		m_zFar += additionalSlide;
//		m_zNear += additionalSlide;
//	}
//
//	if (m_bSlideBack)
//	{
//		//  clamp the view-cube to the slid back objects...
//		const D3DXVECTOR3 eyePt(0.f, 0.f, 0.f);
//		const D3DXVECTOR3 eyeDir(0.f, 0.f, 1.f);
//		D3DXMatrixTranslation(&virtualCameraView, 0.f, 0.f, m_fSlideBack);
//
//		if ( m_bUnitCubeClip )
//		{
//			U2Cone bc( &pShadowMgr->m_ShadowReceiverPoints, &virtualCameraView, &eyePt, &eyeDir);
//			// Compute Projection TM
//		}
//	}
//
//	D3DXMatrixMultiply(&virtualCameraViewProj, &pShadowMgr->m_View, &virtualCameraView);
//	D3DXMatrixMultiply(&virtualCameraViewProj, &virtualCameraViewProj, &virtualCameraProj);
//
//	D3DXMATRIX eyeToPostProjVirtualCam;
//	D3DXMatrixMultiply(&eyeToPostProjVirtualCam, &virtualCameraView, &virtualCameraProj);
//
//	D3DXVECTOR3 eyeLightDir;	
//	D3DXVec3TransformNormal(&eyeLightDir, &pShadowMgr->m_lightDir, &pShadowMgr->m_View);
//
//	//  directional light becomes a point on infinity plane in post-projective space
//	D3DXVECTOR4 lightDirW (eyeLightDir.x, eyeLightDir.y, eyeLightDir.z, 0.f);   
//	D3DXVECTOR4   ppLight;
//
//	D3DXVec4Transform(&ppLight, &lightDirW, &virtualCameraProj);
//
//	m_bShadowTestInverted = (ppLight.w < 0.f); // the light is coming from behind the eye
//
//
//	//  compute the projection matrix...
//	//  if the light is >= 1000 units away from the unit box, use an ortho matrix (standard shadow mapping)
//	if ( (fabsf(ppLight.w) <= W_EPSILON) )  // orthographic matrix; uniform shadow mapping
//	{
//		D3DXVECTOR3 ppLightDirection(ppLight.x, ppLight.y, ppLight.z);
//		D3DXVec3Normalize(&ppLightDirection, &ppLightDirection);
//
//
//		U2Aabb ppUnitBox; ppUnitBox.m_vMax = D3DXVECTOR3(1.f, 1.f, 1.f); ppUnitBox.m_vMin = D3DXVECTOR3(-1.0f, -1.0f, 0.f);
//		D3DXVECTOR3 cubeCenter; 
//		cubeCenter = ppUnitBox.Center();
//		float t;
//
//		ppUnitBox.Intersect(&t, &cubeCenter, &ppLightDirection);
//		D3DXVECTOR3 lightPos = cubeCenter + 2.f*t*ppLightDirection;
//		D3DXVECTOR3 axis = yAxis;
//
//		//  if the yAxis and the view direction are aligned, choose a different up vector, to avoid singularity
//		//  artifacts
//		if ( fabsf(D3DXVec3Dot(&ppLightDirection, &yAxis))>0.99f )
//			axis = zAxis;
//
//		m_ppNear = ppUnitBox.m_vMin.z;
//		m_ppFar = ppUnitBox.m_vMax.z;
//		m_fSlideBack = 0.f;
//	}
//	else  // otherwise, use perspective shadow mapping
//	{
//		D3DXVECTOR3 ppLightPos;
//		float wRecip = 1.0f / ppLight.w;
//		ppLight.x = ppLight.x * wRecip;
//		ppLight.y = ppLight.y * wRecip;
//		ppLight.z = ppLight.z * wRecip;
//
//		D3DXMATRIX eyeToPostProjectiveLightView;
//
//
//		const float ppCubeRadius = 1.5f;  // the post-projective view box is [-1,-1,0]..[1,1,1] in DirectX, so its radius is 1.5
//		const D3DXVECTOR3 ppCubeCenter(0.f, 0.f, 0.5f);
//
//		if (m_bShadowTestInverted)  // use the inverse projection matrix
//		{
//			U2Cone viewCone;
//
//
//			if (!m_bUnitCubeClip)
//			{
//				//  project the entire unit cube into the shadow map  
//				std::vector<U2Aabb> justOneBox;
//				U2Aabb unitCube;
//				unitCube.m_vMin = D3DXVECTOR3(-1.f, -1.f, 0.f);
//				unitCube.m_vMax = D3DXVECTOR3( 1.f, 1.f, 1.f );
//				justOneBox.push_back(unitCube);
//				D3DXMATRIX tmpIdentity;
//				D3DXMatrixIdentity(&tmpIdentity);
//				viewCone = U2Cone(&justOneBox, &tmpIdentity, &ppLightPos);               
//			}
//			else
//			{
//				//  clip the shadow map to just the used portions of the unit box.
//				viewCone = U2Cone(&pShadowMgr->m_ShadowReceiverPoints, &eyeToPostProjVirtualCam, &ppLightPos);
//			}
//
//
//			//  construct the inverse projection matrix -- clamp the fNear value for sanity (clamping at too low
//			//  a value causes significant underflow in a 24-bit depth buffer)
//			//  the multiplication is necessary since I'm not checking shadow casters
//			viewCone.fNear = max(0.001f, viewCone.fNear*0.3f);
//			m_ppNear = -viewCone.fNear;
//			m_ppFar  = viewCone.fNear;
//			lightView = viewCone.m_LookAt;
//
//			// PerspectiveProj
//		}
//		else {
//
//			float fFovy, fAspect, fFar, fNear;
//			if (!m_bUnitCubeClip)
//			{
//
//
//
//
//			}
//			else 
//			{
//				//  unit cube clipping
//				//  fit a cone to the bounding geometries of all shadow receivers (incl. terrain) in the scene
//				U2Cone bc(&pShadowMgr->m_ShadowReceiverPoints, &eyeToPostProjVirtualCam, &ppLightPos);
//				lightView = bc.m_LookAt;
//				D3DXMatrixMultiply(&eyeToPostProjectiveLightView, &eyeToPostProjVirtualCam, &lightView);
//				float fDistance = D3DXVec3Length(&(ppLightPos-ppCubeCenter));
//				fFovy = 2.f * bc.fovy;
//				fAspect = bc.fovx / bc.fovy;
//				fFar = bc.fFar;
//				//  hack alert!  adjust the near-plane value a little bit, to avoid clamping problems
//				fNear = bc.fNear * 0.6f;
//			}
//		}
//
//		//  build the composite matrix that transforms from world space into post-projective light space
//		D3DXMatrixMultiply(&m_LightViewProj, &lightView, &lightProj);
//		D3DXMatrixMultiply(&m_LightViewProj, &virtualCameraViewProj, &m_LightViewProj);
//	}

}

void U2ShadowTechnique::RenderShadowMap(nMeshGroup* curMesh)
{
	// Set RenderStates
	
	
	// Set Shadow Technique	
	IsRendererOK;


	pRenderer->SetVertexRange(curMesh->GetFirstVertex(),curMesh-> GetNumVertices());
	pRenderer->SetIndexRange(curMesh->GetFirstIndex(), curMesh->GetNumIndices());

	
	IDirect3DVertexDeclaration9* pOrigDecl =  curMesh->GetMeshData()->GetVertexDeclaration();

	 curMesh->GetMeshData()->SetVertexDeclaration(m_pShadowVertexDecl);

	if(1 == m_uiNumShaderPasses)
	{
		pRenderer->SetupMesh(curMesh->GetMesh());
		pRenderer->DrawIndexedPrimitive(curMesh->GetMeshData()->GetPrimitiveType());		
	}
	else 
	{
		uint32 pass;
		for(pass = 0; pass < m_uiNumShaderPasses; ++pass)
		{
			m_spShadowMapShader->BeginPass(pass);
			pRenderer->SetupMesh(curMesh->GetMesh());
			pRenderer->DrawIndexedPrimitive(curMesh->GetMeshData()->GetPrimitiveType());			
			m_spShadowMapShader->EndPass();
		}
	}

	curMesh->GetMeshData()->SetVertexDeclaration(pOrigDecl);

}
