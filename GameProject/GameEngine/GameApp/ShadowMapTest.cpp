#include "stdafx.h"
#include "GameApp.h"
#include <GameApp/Util/EffectUtil.h>
#include "DemoSetup.h"
#include "PSSMShadowMapDx11.h"
#include "Mesh_D3D11.h"

ID3DX11Effect *g_pEffect = NULL;
ID3DX11EffectTechnique *g_pTechniqueShadowMap_GSC = NULL;
ID3DX11EffectTechnique *g_pTechniqueShadowMap_Inst = NULL;
ID3DX11EffectTechnique *g_pTechniqueShadows = NULL;
ID3DX11EffectMatrixVariable *g_pWorldVariable = NULL;
ID3DX11EffectMatrixVariable *g_pViewProjVariable = NULL;
ID3DX11EffectVectorVariable *g_pLightDirVariable = NULL;
ID3DX11EffectVectorVariable *g_pLightAmbientVariable = NULL;
ID3DX11EffectVectorVariable *g_pLightColorVariable = NULL;
ID3DX11EffectMatrixVariable *g_pViewVariable = NULL;
ID3DX11EffectMatrixVariable *g_pCropMatrixVariable = NULL;
ID3DX11EffectShaderResourceVariable *g_pShadowMapTextureArrayVariable = NULL;
ID3DX11EffectMatrixVariable *g_pTextureMatrixVariable = NULL;
ID3DX11EffectScalarVariable *g_pSplitPlaneVariable = NULL;
ID3DX11EffectScalarVariable *g_pFirstSplitVariable = NULL;
ID3DX11EffectScalarVariable *g_pLastSplitVariable = NULL;
ID3DX11EffectTechnique *g_pTechniqueShadowMap_Standard = NULL;
ID3DX11EffectTechnique *g_pTechniqueShadows_MP = NULL;
ID3DX11EffectTechnique *g_pTechniqueShadows_DX9 = NULL;
ID3DX11EffectShaderResourceVariable *g_pShadowMapTextureVariable = NULL;






bool App::FrustumTestLoad()
{
	CreateMeshes();

	CreateScene();

	DestroyShadowMaps();

	PSSMShadowMapDx11 *pShadowMap = new PSSMShadowMapDx11();
	g_ShadowMaps.push_back(pShadowMap);
	//if(!pShadowMap->CreateAsTextureArray(g_iShadowMapSize, NUM_SPLITS_IN_SHADER)) return false;
	if(!pShadowMap->CreateAsTextureCube(g_iShadowMapSize)) return false;

	return true;
}

bool CreateShaders(void)
{
	ID3DBlob *pErrors=NULL;

	HRESULT hr = LoadEffectFromFile(GetApp()->GetDevice(), TEXT("Shaders\\D3D10.fx"), &g_pEffect);

	// Pre-April 2007 SDK version
	//HRESULT hr = D3DX10CreateEffectFromFile( TEXT("Shaders\\D3D10.fx"), NULL, NULL, D3D10_SHADER_ENABLE_STRICTNESS, 0, 
	//                                         GetApp()->GetDevice(), NULL, NULL, &g_pEffect, &pErrors );

	if(FAILED(hr))
	{
		MessageBox(NULL, TEXT("Loading effect failed!"), TEXT("Error!"), MB_OK);
		if(pErrors!=NULL)
		{
			MessageBoxA(NULL,(const char *)pErrors->GetBufferPointer(),"Compilation errors",MB_OK);
		}
		return false;
	}
	if(pErrors!=NULL) pErrors->Release();
	g_pTechniqueShadowMap_GSC = g_pEffect->GetTechniqueByName("RenderShadowMap_GSC");
	g_pTechniqueShadowMap_Inst = g_pEffect->GetTechniqueByName("RenderShadowMap_Inst");
	g_pTechniqueShadows = g_pEffect->GetTechniqueByName("RenderShadows");
	g_pWorldVariable = g_pEffect->GetVariableByName("g_mWorld")->AsMatrix();
	g_pViewProjVariable = g_pEffect->GetVariableByName("g_mViewProj")->AsMatrix();
	g_pViewVariable = g_pEffect->GetVariableByName("g_mView")->AsMatrix();
	g_pLightDirVariable = g_pEffect->GetVariableByName("g_vLightDir")->AsVector();
	g_pLightAmbientVariable = g_pEffect->GetVariableByName("g_vAmbient")->AsVector();
	g_pLightColorVariable = g_pEffect->GetVariableByName("g_vLightColor")->AsVector();
	g_pShadowMapTextureArrayVariable = g_pEffect->GetVariableByName("g_txShadowMapArray")->AsShaderResource();
	g_pCropMatrixVariable = g_pEffect->GetVariableByName("g_mCropMatrix")->AsMatrix();
	g_pTextureMatrixVariable = g_pEffect->GetVariableByName("g_mTextureMatrix")->AsMatrix();
	g_pSplitPlaneVariable = g_pEffect->GetVariableByName("g_fSplitPlane")->AsScalar();
	g_pFirstSplitVariable = g_pEffect->GetVariableByName("g_iFirstSplit")->AsScalar();
	g_pLastSplitVariable = g_pEffect->GetVariableByName("g_iLastSplit")->AsScalar();

	g_pTechniqueShadowMap_Standard = g_pEffect->GetTechniqueByName("RenderShadowMap_Standard");
	g_pTechniqueShadows_DX9 = g_pEffect->GetTechniqueByName("RenderShadows_DX9");
	g_pTechniqueShadows_MP = g_pEffect->GetTechniqueByName("RenderShadows_MP");
	g_pShadowMapTextureVariable = g_pEffect->GetVariableByName("g_txShadowMap")->AsShaderResource();

	// Load font
	//

	return true;
}


static void RenderCasters(std::set<SceneObject *> &Objects, const Matrix &mViewProj, Matrix *mCropMatrix)
{
	// set constants
	g_pViewProjVariable->SetMatrix((float*)&mViewProj);
	g_pCropMatrixVariable->SetMatrixArray((float*)mCropMatrix, 0, g_iNumSplits);
	delete[] mCropMatrix;

	// for each pass in technique
	D3DX11_TECHNIQUE_DESC  DescTech;
	//if(g_iRenderingMethod == METHOD_DX10_INST)
		g_pTechniqueShadowMap_Inst->GetDesc( &DescTech );
	//else if(g_iRenderingMethod == METHOD_DX10_GSC)
	//	g_pTechniqueShadowMap_GSC->GetDesc( &DescTech );

	for(UINT p = 0; p < DescTech.Passes; ++p)
	{
		// for each object
		std::set<SceneObject *>::iterator it;
		for(it = Objects.begin(); it != Objects.end(); it++)
		{
			SceneObject *pObject = (*it);

			// set world matrix
			g_pWorldVariable->SetMatrix((float*)&pObject->m_mWorld);

			// set split range
			g_pFirstSplitVariable->SetInt(pObject->m_iFirstSplit);
			g_pLastSplitVariable->SetInt(pObject->m_iLastSplit);

			// keep triangle count accurate
			g_iTrisPerFrame += pObject->m_pMesh->m_iNumTris * (pObject->m_iLastSplit - pObject->m_iFirstSplit);

			//if(g_iRenderingMethod == METHOD_DX10_INST)
			{
				// activate pass
				g_pTechniqueShadowMap_Inst->GetPassByIndex(p)->Apply(0, GetApp()->GetContext());
				// draw instanced
				int iNumInstances = pObject->m_iLastSplit - pObject->m_iFirstSplit + 1;
				((Mesh_D3D11 *)pObject->m_pMesh)->DrawInstanced(iNumInstances);
			}
			//else
			//{
			//	// activate pass
			//	g_pTechniqueShadowMap_GSC->GetPassByIndex(p)->Apply(0);
			//	// draw
			//	pObject->m_pMesh->Draw();
			//}

			// reset variables
			pObject->m_iFirstSplit = INT_MAX;
			pObject->m_iLastSplit = INT_MIN;
		}
	}
}

static void RenderReceivers(const std::vector<SceneObject *> &Objects, const Matrix &mViewProj)
{
	// set constants
	g_pViewProjVariable->SetMatrix((float*)&mViewProj);
	g_pViewVariable->SetMatrix((float*)&g_ShadowCamera.m_mView);
	g_pLightDirVariable->SetFloatVector((float*)&g_Light.GetDir());
	g_pLightColorVariable->SetFloatVector((float*)&g_Light.m_vLightDiffuse);
	g_pLightAmbientVariable->SetFloatVector((float*)&g_Light.m_vLightAmbient);

	// for each pass in technique
	D3DX11_TECHNIQUE_DESC DescTech;
	g_pTechniqueShadows->GetDesc( &DescTech );
	for(UINT p = 0; p < DescTech.Passes; ++p)
	{
		// for each object
		for(unsigned int j = 0; j < Objects.size(); j++)
		{
			SceneObject *pObject = Objects[j];

			// set world matrix
			g_pWorldVariable->SetMatrix((float*)&pObject->m_mWorld);

			// activate pass
			g_pTechniqueShadows->GetPassByIndex(p)->Apply(0, GetApp()->GetContext());
			// draw
			pObject->m_pMesh->Draw();
		}
	}
}


// Set rendering range for given casters
//
//
static void UpdateSplitRange(const std::vector<SceneObject*> &casters, int iSplit)
{
	for(unsigned int i=0; i < casters.size(); i++)
	{
		SceneObject *pCaster = casters[i];
		if(iSplit < pCaster->m_iFirstSplit) pCaster->m_iFirstSplit = iSplit;
		if(iSplit > pCaster->m_iLastSplit) pCaster->m_iLastSplit = iSplit;
	}
}

// Starts rendering to shadow maps
//
//
static void ActivateShadowMaps(void)
{
	// unbind shadow map
	g_pShadowMapTextureArrayVariable->SetResource(NULL);

	// Enable rendering to shadow map
	GetShadowMap<PSSMShadowMapDx11>()->EnableRendering();

	// Clear texture
	float ClearSM[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	GetApp()->GetContext()->ClearDepthStencilView(GetShadowMap<PSSMShadowMapDx11>()->m_pDSV, D3D10_CLEAR_DEPTH, 1.0f, 0);
}


// Stops rendering to shadow maps
//
//
static void DeactivateShadowMaps(void)
{
	// Disable rendering to shadow map
	GetShadowMap<PSSMShadowMapDx11>()->DisableRendering();
}


// Sets shader parameters for hardware shadow rendering
//
//
static void SetShaderParams(Matrix *mTextureMatrix)
{
	g_pTextureMatrixVariable->SetMatrixArray((float*)mTextureMatrix, 0, g_iNumSplits);
	delete[] mTextureMatrix;

	// store split end positions in shader
	g_pSplitPlaneVariable->SetFloatArray(&g_fSplitPos[1], 0, g_iNumSplits);

	// bind shadow map texture array
	g_pShadowMapTextureArrayVariable->SetResource(GetShadowMap<PSSMShadowMapDx11>()->m_pSRV);
}

static Matrix GetTexScaleBiasMatrix(void)
{
	// Calculate a matrix to transform points to shadow map texture coordinates
	// (this should be exactly like in your standard shadow map implementation)
	//
	float fTexOffset = 0.5f + (0.5f / (float)g_ShadowMaps[0]->GetSize());

	return Matrix(       0.5f,        0.0f,   0.0f,  0.0f,
		0.0f,       -0.5f,   0.0f,  0.0f,
		0.0f,        0.0f,   1.0f,  0.0f,
		fTexOffset,  fTexOffset,   0.0f,  1.0f);
}


void Render_DX11(void)
{
	// find receivers
	std::vector<SceneObject *> receivers;
	std::set<SceneObject *> casters;
	receivers = g_ShadowCamera.FindReceivers();

	// adjust camera planes to contain scene tightly
	g_ShadowCamera.AdjustPlanes(receivers);

	// calculate the distances of split planes
	g_ShadowCamera.CalculateSplitPositions(g_fSplitPos);

	// array of texture matrices
	Matrix *mTextureMatrix = new Matrix[g_iNumSplits];
	// array of crop matrices
	Matrix *mCropMatrix = new Matrix[g_iNumSplits];

	// for each split
	for(int i = 0; i < g_iNumSplits; i++)
	{
		// calculate frustum
		Frustum splitFrustum;
		splitFrustum = g_ShadowCamera.CalculateFrustum(g_fSplitPos[i], g_fSplitPos[i+1]);
		// find casters
		std::vector<SceneObject *> castersInSplit;
		castersInSplit = g_Light.FindCasters(splitFrustum);
		UpdateSplitRange(castersInSplit, i);
		casters.insert(castersInSplit.begin(), castersInSplit.end());

		// calculate crop matrix
		mCropMatrix[i] = g_Light.CalculateCropMatrix(castersInSplit, receivers, splitFrustum);
		// calculate texture matrix
		mTextureMatrix[i] = g_Light.m_mView * g_Light.m_mProj * mCropMatrix[i] * GetTexScaleBiasMatrix();
	}

	// render shadow map
	ActivateShadowMaps();
	RenderCasters(casters, g_Light.m_mView * g_Light.m_mProj, mCropMatrix);
	DeactivateShadowMaps();

	// render scene
	SetShaderParams(mTextureMatrix);
	RenderReceivers(receivers, g_ShadowCamera.m_mView * g_ShadowCamera.m_mProj);
}



bool g_bDontRender = false;
double g_fTimePerFrame = 0;
void Render(Renderer* pRenderer)
{
	// Make the app more reference rasterizer friendly
	// 
	/*if(GetApp()->GetParams().bReferenceRasterizer)
	{
		if(g_bDontRender)
		{
			char pText[1024];
			pText[0]=0;
			_snprintf(pText, 1024, "Frame rendered in %g seconds. Press SPACE to continue.", g_fTimePerFrame);
			SetWindowTextA( GetApp()->GetHWND(), pText);
			if(GetKeyDown(VK_SPACE)) g_bDontRender = false;
			Sleep(10);
			return;
		}
		SetWindowText( GetApp()->GetHWND(), TEXT("Rendering using reference rasterizer...") );
		g_fTimePerFrame = GetAccurateTime();
		g_bDontRender = true;
	}*/

	// move camera
	//if(GetApp()->GetParams().bReferenceRasterizer) g_Camera.CalculateMatrices();
	//else
		g_ShadowCamera.DoControls();

	// move light
	g_Light.DoControls();

	DoControls();

	// reset triangle counter
	g_iTrisPerFrame = 0;

	g_iTrisPerFrame = 0;


	// Clear
	//float ClearBG[4] = { 0.25f, 0.25f, 0.25f, 1.0f };
	//GetApp()->GetDevice()->ClearRenderTargetView(GetApp()->GetRTV(), ClearBG);
	//GetApp()->GetDevice()->ClearDepthStencilView(GetApp()->GetDSV(), D3D10_CLEAR_DEPTH, 1.0f, 0);
	g_iTrisPerFrame = 0;
	
	Render_DX11();

	
	// present
	//GetApp()->GetSwapChain()->Present(0, 0);

	/*if(GetApp()->GetParams().bReferenceRasterizer)
	{
		g_fTimePerFrame = GetAccurateTime() - g_fTimePerFrame;
	}*/
}


void App::drawShadowedModels()
{
	Render(this->renderer);
}