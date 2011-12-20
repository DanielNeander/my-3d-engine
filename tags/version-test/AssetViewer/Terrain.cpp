#include "stdafx.h"
#include "Terrain.h"
#include "TerrainMesh.h"
#include "HeightQuery.h"

Terrain::Terrain(U2Camera* pSceneCam)
:m_pMesh(0),
m_spEffect(0),
m_spTerrainShader(0),
m_fLOD(50.0f),
m_bNormalInVS(false),
m_iBlockSize(0),
m_iMaxLevels(0),
m_pSceneCam(pSceneCam),
m_spCamNode(U2_NEW U2CameraNode(m_pSceneCam)),
m_pHeightQuery(U2_NEW HeightQuery())
{
	U2ASSERT(m_pSceneCam);
	
	// set up the light direction
	m_vLightDir = D3DXVECTOR4(1.0f,-1.0f,0.5f,0.0f);
	D3DXVec4Normalize(&m_vLightDir, &m_vLightDir);
}

Terrain::~Terrain()
{	
	Terminate();
}

void Terrain::Terminate()
{
	m_spCamNode = 0;
	m_pSceneCam = 0;		 
	m_spTerrainShader = 0;
	m_spEffect = 0;
	U2_DELETE m_pHeightQuery;
	m_pHeightQuery = 0;
	U2_DELETE m_pMesh;
}

bool Terrain::Initialize(int iBlockSize)
{
	
	if (!SetHeightMap(_T("heightfield.dds")))
		return false;

	if (!SetNormalMap(_T("normalmap.dds")))
	{
		return false;
	}

	if (!InitShader())
		return false;

	// 호출순서 주의.

	if (!SetBlockSize(iBlockSize))
		return false;

	//Draw(0.0f, 0.0f, 1.0f, m_iMaxLevels);
	U2ASSERT(m_pMesh);
	

	//D3DXVECTOR3 vScale = m_tmLocal.GetScale();
	//nMeshGroup& meshGroup = m_pMesh->Group(0);
	//meshGroup.SetLocalScale(vScale.x, vScale.y, vScale.z);
	//meshGroup.Update();

	m_pHeightQuery->Initialize();

		
	return true;
}

bool Terrain::SetHeightMap(const TCHAR* heightmap)
{
	U2FilePath fPath;				
	TCHAR fullPath[MAX_PATH];
	fPath.ConvertToAbs(fullPath, MAX_PATH * sizeof(TCHAR), heightmap, TEXTURE_PATH);	
	m_szHeightMap = fullPath;
	return InitHeightField(true, false);
}

bool Terrain::SetNormalMap(const TCHAR* normalMap)
{
	U2FilePath fPath;				
	TCHAR fullPath[MAX_PATH];
	fPath.ConvertToAbs(fullPath, MAX_PATH * sizeof(TCHAR), normalMap, TEXTURE_PATH);	
	m_szNormalMap = fullPath;
	return InitHeightField(false, true);
}


bool Terrain::InitHeightField(bool bInitHeight, bool bInitNormal)
{
	IsRendererOK2;

	// Load HeightField 
	if (bInitHeight)
	{
		m_spVertexTex = U2Dx9Texture::Create(m_szHeightMap.Str(), pRenderer);
		U2ASSERT(m_spVertexTex->GetD3DTex());
		IDirect3DTexture9 *pTex = (IDirect3DTexture9*)m_spVertexTex->GetD3DTex();
		if (FAILED(pTex->GetLevelDesc(0, &m_surfDesc)))
			return false;

		// Setup the camera's world matrix
		float sx = (float)m_surfDesc.Width;
		float sy = (float)MAX_ELEVATION; // the world vertical extent - the elevation span
		float sz = (float)m_surfDesc.Height;
						
		m_tmLocal.SetScale(D3DXVECTOR3(sx, sy, sz));
		
	}

	if (bInitNormal)
	{
		m_spNormalMap = U2Dx9Texture::Create(m_szNormalMap.Str(), pRenderer);
		U2ASSERT(m_spNormalMap->GetD3DTex());		
	}
	// Load Normal Map

	return true;
}

bool Terrain::InitShader()
{
	IsRendererOK2;

	U2FrameShader& FrameShad= U2SceneMgr::Instance()->m_spFrame->GetShader(_T("gputerrain"));

	m_spTerrainShader = FrameShad.GetShader();
	m_spTerrainShader->IncRefCount();

	if (!m_spEffect)
	{
		m_spEffect = U2_NEW U2Dx9FxShaderEffect(m_spTerrainShader, pRenderer);
		m_spEffect->SetFrameShaderName(FrameShad.GetName());
	}

	//m_spEffect->SetFloat(U2FxShaderState::
	m_spEffect->SetVector(U2FxShaderState::LightDir, m_vLightDir);
	m_spEffect->SetTexture(U2FxShaderState::DiffMap0, m_szHeightMap);
	m_spEffect->SetTexture(U2FxShaderState::NormalMap, m_szNormalMap);
	
	if (!m_spTerrainShader)
		return false;

	return true;
}

bool Terrain::SetBlockSize(int iBlockSize)
{
	if (iBlockSize != m_iBlockSize)
	{
		if (!m_pMesh)
			m_pMesh = U2_NEW TerrainMesh(NULL, this);

		AttachChild(m_pMesh);
		Update(0.f);

		// the block size has changed, must recreate the 
		// vertex buffer and index buffer
		m_iBlockSize = iBlockSize;

		if (!m_pMesh->Initialize(m_iBlockSize, m_iBlockSize, true))
			return false;

		m_iMaxLevels = CalculateMaxLevels(m_iBlockSize, m_surfDesc.Width);		
	}	

	return true;
}

//----------------------------------------------------------------------------------

int Terrain::CalculateMaxLevels(int iBlockSize, int iHeightfieldSize)
{
	int iPow2 = (iHeightfieldSize-1) / (iBlockSize-1);
	int iLevels = 0;
	for (int i = 1; i < iPow2; i*=2) {
		iLevels++;
	}
	return iLevels;
}


bool Terrain::Draw(float fBiasU, float fBiasV, float fScale, int iLevel)
{
	IsRendererOK2;

	if (!m_spEffect)
		return false;
	
	float fTexelSizeU, fTexelSizeV;
	fTexelSizeU = (1<<iLevel) / float(m_surfDesc.Width-1);
	fTexelSizeV = (1<<iLevel) / float(m_surfDesc.Height-1);
	
	bool bSuccess = true;

	D3DXVECTOR4 vTexelSize(fTexelSizeU, fTexelSizeV, 1.0f, 0.0f);

	m_spEffect->SetFloat(U2FxShaderState::BiasU, fBiasU);
	m_spEffect->SetFloat(U2FxShaderState::BiasV, fBiasV);
	m_spEffect->SetFloat(U2FxShaderState::Scale, fScale);
	m_spEffect->SetFloat(U2FxShaderState::NormalScale, 2.0f * (1 << iLevel) / MAX_ELEVATION);
	m_spEffect->SetVector(U2FxShaderState::TexelSize, vTexelSize);

	nMeshGroup& meshGroup = m_pMesh->Group(0);

	//m_pMesh->ApplyShader(U2SceneMgr::Instance());

	for(uint32 i=0; i < meshGroup.GetEffectCnt(); ++i)
	{
		U2Dx9FxShaderEffect* pEffect = 
			DynamicCast<U2Dx9FxShaderEffect>(meshGroup.GetEffect(i));
		if(pEffect)
		{			
			pEffect->GetEffectShader()->SetAttribute(pEffect->GetShaderAttb());
			
			pRenderer->SetCurrEffectShader(pEffect->GetEffectShader());			
			pRenderer->SetupMesh(m_pMesh);
			break;
		}
	}						
	

	pRenderer->SetVertexRange(meshGroup.GetFirstVertex(), meshGroup.GetNumVertices());
	pRenderer->SetIndexRange(meshGroup.GetFirstIndex(), meshGroup.GetNumIndices());
	pRenderer->DrawIndexedPrimitive(m_pMesh->GetMeshData()->GetPrimitiveType());

	return bSuccess;
}

void Terrain::Render(float fMinU, float fMinV, float fMaxU, float fMaxV, int iLevel, float fScale)
{

	float fHalfU = (fMinU + fMaxU) * 0.5f;
	float fHalfV = (fMinV + fMaxV) * 0.5f;


	float d = (fMaxU-fMinU) * m_surfDesc.Width / (m_iBlockSize-1.0f);
	float d2 = d * d;

	D3DXVECTOR3 v = D3DXVECTOR3(fHalfU*m_surfDesc.Width, 0, fHalfV*m_surfDesc.Height) - m_pSceneCam->GetEye();
	float l2 = D3DXVec3LengthSq(&v);

	// use distances squared
	float f2 = l2 / d2;

	if (f2 > m_fLOD*m_fLOD || iLevel < 1) {
		Draw(fMinU, fMinV, fScale, iLevel);
	} else {
		fScale = fScale / 2.0f;
		Render(fMinU, fMinV, fHalfU, fHalfV, iLevel-1, fScale);
		Render(fHalfU, fMinV, fMaxU, fHalfV, iLevel-1, fScale);
		Render(fMinU, fHalfV, fHalfU, fMaxV, iLevel-1, fScale);
		Render(fHalfU, fHalfV, fMaxU, fMaxV, iLevel-1, fScale);
	}
}

void Terrain::AvoidCollision()
{
	// eye position in local coordinates
	D3DXVECTOR3 ptEyeLocal = m_pSceneCam->GetEye();

	float height;

	// query the height of the terrain at the eye position
	if (SUCCEEDED(m_pHeightQuery->QueryPoint(ptEyeLocal.x, ptEyeLocal.z, &height))) {
		// if the camera is below the terrain
		if (height > ptEyeLocal.y) {

			// update the camera to set the elevation 2 meters above the ground
			SetCameraElevation(height, 2.0f);
		}
	}
}

void Terrain::SetCameraElevation(float fLocalHeight, float fHeightOverTerrain)
{
	D3DXMATRIX worldTM = m_spCamNode->m_tmWorld.GetRot();

	float height = fLocalHeight * worldTM._22 + fHeightOverTerrain;

	D3DXVECTOR3* pEye = &m_pSceneCam->GetEye();
	pEye->y = height;

	D3DXVECTOR3* pDir = &m_pSceneCam->GetDir();
	pDir->y = height;
	
	m_pSceneCam->SetAxes(*pDir, m_pSceneCam->GetUp(), m_pSceneCam->GetRight());

	m_spCamNode->Update(0.f);
	
}


void Terrain::UpdateWorldData(float fTime)
{
	U2Node::UpdateWorldData(fTime);

	AvoidCollision();

}

