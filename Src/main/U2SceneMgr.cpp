#include <U2_3D/Src/U23DLibPCH.h>
#include "U2SceneMgr.h"
#include <U2_3D/Src/dx9/U2Dx9FramePhase.h>
#include <U2_3D/Src/dx9/U2Dx9FrameShader.h>
#include <U2_3D/Src/Main/U2RenderContext.h>
#include <U2_3D/src/dx9/U2Dx9ShadowMgr.h>
#include <U2_3D/src/dx9/U2ShadowNode.h>

#define  USE_HDR 1
#define USER_SHADER 


// Header에 선언하면 안됨.. 주의.
U2SceneMgr::LightInfo::~LightInfo()
{
	//U2_DELETE m_pLightNode;
	m_pLightNode = 0;
}


U2SceneMgr* U2SceneMgr::ms_pInstance = 0;
D3DXVECTOR3 U2SceneMgr::ms_cameraPos;
int U2SceneMgr::ms_iSortingOrder = U2FramePhase::FRONT_TO_BACK;


U2SceneMgr* U2SceneMgr::Create(U2Dx9Renderer* pRenderer)
{
	U2SceneMgr* pThs = U2_NEW U2SceneMgr(pRenderer);	
	pThs->Initialize();
	return pThs;
}


U2SceneMgr::U2SceneMgr(U2Dx9Renderer* pRenderer)
:m_pRenderer(pRenderer),
m_spRootNode(NULL),
m_bClipPlaneFencing(true),
m_bObjLightLinks(false),
m_bShadowEnabled(true),
m_aMeshBuckts(0, 1024),
m_pShadowMgr(0)
{
	ms_pInstance = this;
	
	
}

//-------------------------------------------------------------------------------------------------
U2SceneMgr::~U2SceneMgr()
{
	Terminate();
}

//-------------------------------------------------------------------------------------------------
U2SceneMgr* U2SceneMgr::Instance()
{
	return ms_pInstance;
}

//-------------------------------------------------------------------------------------------------
bool U2SceneMgr::Initialize()
{

	m_spRootNode = U2_NEW U2Node;

	U2ASSERT(!m_spFrame);
	m_spFrame = U2_NEW U2Frame;

	U2FilePath fPath;				
	TCHAR fullPath[MAX_PATH];
	U2DynString dir(DATA_PATH);
	dir.Concatenate(_T("\\shaders"));

#if USE_HDR
#ifdef  USER_SHADER
	fPath.ConvertToAbs(fullPath, MAX_PATH * sizeof(TCHAR), _T("dx9hdr_shaders_user.xml"), dir);	
#else
	fPath.ConvertToAbs(fullPath, MAX_PATH * sizeof(TCHAR), _T("dx9hdr_shaders_origin.xml"), dir);	
#endif
#else 
	fPath.ConvertToAbs(fullPath, MAX_PATH * sizeof(TCHAR), _T("dx9_shaders.xml"), dir);	
#endif 

	m_spFrame->SetFilename(fullPath);
#if 1
	if(m_spFrame->OpenXml())
	{
		bool bInit = m_spFrame->Initialize();
		U2ASSERT(bInit);

		m_spFrame->CloseXml();
	}

	// Reset 에러 주범..
	//m_pShadowMgr = U2_NEW U2Dx9ShadowMgr(NULL);
	//m_pShadowMgr->Initialize();
#endif


	return true;
}

//-------------------------------------------------------------------------------------------------
void U2SceneMgr::Terminate()
{
	unsigned int i;
	for(i=0; i < m_aLights.FilledSize(); ++i)
	{
		U2_DELETE m_aLights[i];
	}	

	U2_DELETE m_pShadowMgr;
}

//-------------------------------------------------------------------------------------------------
bool U2SceneMgr::BeginScene( const D3DXMATRIX viewMat)
{
	// 필요 작업
	//D3DXMATRIX transPoseView;
	//D3DXMatrixTranspose(&transPoseView, &viewMat);		
	//m_pRenderer->SetTransform(U2Dx9Renderer::TMT_VIEW, viewMat);
	
	if(m_pRenderer->BeginScene())
	{
		//`1UpdatePerFrameSharedShaderParams();

		return true;
	}
	else 
		return false;
}

//-------------------------------------------------------------------------------------------------
bool U2SceneMgr::EndScene()
{
	return m_pRenderer->EndScene();
}


//-------------------------------------------------------------------------------------------------
void U2SceneMgr::SortNodes()
{
	unsigned i;
	unsigned numMesh = m_aMeshBuckts.Size();
	for(i = 0; i < numMesh; ++i)
	{
		int numIndices = m_aMeshBuckts[i].FilledSize();
		if(numIndices > 0)
		{
			uint16* pIndices = (uint16*)m_aMeshBuckts[i].GetBase();
			std::qsort(pIndices, numIndices, sizeof(uint16), (int(__cdecl*)(const void* ,
				const void*))CompareNodes);
		}
	}

}

//-------------------------------------------------------------------------------------------------
int  U2SceneMgr::CompareNodes(const uint16* i1, const uint16 *i2)
{
	const U2SceneMgr::MeshInfo* pInfo1 = ms_pInstance->m_aMeshes[*i1];
	const U2SceneMgr::MeshInfo* pInfo2 = ms_pInstance->m_aMeshes[*i2];
	U2ASSERT(pInfo1 && pInfo2);

	int cmp = pInfo1->m_iPriority - pInfo2->m_iPriority;
	if(cmp != 0)
	{
		return cmp;
	}

	// by indentical scene node 
	cmp = int(pInfo1->m_pMeshGroup) - int(pInfo2->m_pMeshGroup);
	if(cmp == 0)
	{
		return cmp;
	}

	// distance to camera(closet first)
	static D3DXVECTOR3 dist1;
	static D3DXVECTOR3 dist2;

	ms_cameraPos.x = (U2Dx9Renderer::GetRenderer()->GetViewMat())._41; 
	ms_cameraPos.y = (U2Dx9Renderer::GetRenderer()->GetViewMat())._42;
	ms_cameraPos.z = (U2Dx9Renderer::GetRenderer()->GetViewMat())._43;

	dist1 = ms_cameraPos - pInfo1->m_pMeshGroup->GetWorldTranslate();
	dist2 = ms_cameraPos - pInfo2->m_pMeshGroup->GetWorldTranslate();

	float diff = U2Math::Vec3SquareLength(dist1) - U2Math::Vec3SquareLength(dist2);

	switch(ms_iSortingOrder)
	{
	case U2FramePhase::FRONT_TO_BACK:
		{
			if(diff < 0.001f)	return -1;
			if(diff > 0.001f)	return	1;			
		}

	case U2FramePhase::BACK_TO_FRONT:
		{
			if(diff > 0.001f)	return -1;
			if(diff < 0.001f)	return	1;
		}
	}

	// nodes are identical
	return 0;
}

//-------------------------------------------------------------------------------------------------
// Reflection 관련..
void U2SceneMgr::UpdateCameraNodes()
{
	unsigned int i;
	for(i = 0; i < m_aCameras.FilledSize(); ++i)
	{
		CameraInfo* pCameraInfo = m_aCameras[i];

		//unsigned int sectionIdx = const U2StackString& szFrameSection = ;		
	}
}

void U2SceneMgr::DrawReflectOrRefractionScene()
{
	for(unsigned int camIdx = 0; camIdx < m_aCameras.FilledSize(); camIdx++)
	{
		CameraInfo* pCam = this->m_aCameras.GetElem(camIdx);
		U2CameraNode* pCamNode = pCam->m_pCameraNode;

		const U2DynString& szFrameSection = pCam->m_szFrameSection;
		int sectionIdx = m_spFrame->FindFrameSectionIdx(szFrameSection);

		if ( -1 != sectionIdx)
		{
			// Update camera view matrix
			//m_pRenderer->GetTransform(U2Dx9Renderer::TMT_VIEW);
			//m_pRenderer->GetTransform(U2Dx9Renderer::TMT_PROJ);

			m_pRenderer->SetCamera(pCamNode->GetCamera());
			m_pRenderer->SetProjectionTM(pCamNode->GetCamera());

			m_pRenderer->PushTransform(U2Dx9Renderer::TMT_VIEW, m_pRenderer->GetViewMat());
			m_pRenderer->PushTransform(U2Dx9Renderer::TMT_PROJ, m_pRenderer->GetProjMat());

			DrawFrame(*(this->GetFrame()->GetFrameSection(sectionIdx)));

			m_pRenderer->PopTransform(U2Dx9Renderer::TMT_PROJ);
			m_pRenderer->PopTransform(U2Dx9Renderer::TMT_VIEW);	

		}		
		
	}
}


//-------------------------------------------------------------------------------------------------
void U2SceneMgr::DrawScene(U2VisibleSet& visibleSet)
{

	const uint32 uVisibleCnt = visibleSet.GetCount();

	if(uVisibleCnt == 0)
	{
		return;
	}
	U2VisibleObject* pVisible = visibleSet.GetVisible();
	/*for(uint32 i=0; i < uVisibleCnt; ++i)
	{

	}*/
	
	
	// ComputeLightScissorAndClipPlanes();

	m_aShadowLights.RemoveAll();

	SortNodes();

	
	// Reflection, Refraction Flag Check
	//UpdateCameraNodes();
	if (this->GetCameraEnabled())
		

	// reset light passes in Mesh between frame
	for(unsigned int i=0; i < m_aMeshBuckts.Size(); ++i)
	{
		const U2PrimitiveVec<uint16>& m_meshIndices = 
			m_aMeshBuckts[i];
		for(unsigned int j=0; j < m_meshIndices.FilledSize(); ++j)
		{
			m_aMeshes[m_meshIndices[j]]->m_iLightPass = 0;
		}
	}	

	//unsigned int sectionIdx = m_spFrame->FindFrameSectionIdx(_T("default"));
	for (uint32 sectionIdx = 0; sectionIdx < 
		m_spFrame->GetFrameSections().FilledSize(); sectionIdx++)
		DrawFrame(*m_spFrame->GetFrameSection(sectionIdx));
}

//-------------------------------------------------------------------------------------------------
/**************************************************************************************************
Func name	:	DrawFrame
params		:	U2FrameSection&
params Desc :	
Func Desc	:	curPass의 Begin에서 U2D3DXEffectShader::SetShaderAttribute하고,
				curPhase의 각 렌더링 함수 내에서 sequence의 Begin 함수 호출시
				U2D3DXEffectShader::SetShaderAttribute 호출하여 쉐이더를 업데이트한다.				
/*************************************************************************************************/

void U2SceneMgr::DrawFrame(U2FrameSection& frameSection)
{
	//debug_con << "Frame Section Begin";
	//DebugConsole::flush();
	PROFILER_DECLARE(prof);
	PROFILER_START(prof);	
	unsigned int numPasses = frameSection.Begin();
	PROFILER_STOP(prof);
	PROFILER_OUTPUT(prof);

	unsigned int passIdx;
	int numDrawShadow = 0;
	for(passIdx =0; passIdx < numPasses; ++passIdx )
	{
		U2FramePass& curPass = frameSection.GetPass(passIdx);

		if(curPass.GetDrawGui())
		{
			continue;
		}

		if(curPass.GetShadowTechnique() != U2FramePass::NO_SHADOW)
		{
			// Render Shadow
			if (GetShadowEnabled())
			{			
			//	GatherShadowLights();
			//	RenderShadowVolume(curPass);
			}
			//RenderShadowMap(curPass);
			++numDrawShadow;
		}
		else if(curPass.GetOcculsionQuery())
		{
			// perform light source occulsion query,
			// this marks the light sources in the scene as occluded or not
		}
		else 
		{
			// 병목 지점... 
			
			unsigned int numPhases = curPass.Begin(); // BeginRenderTarget						
			unsigned int phaseIdx;
			for(phaseIdx = 0; phaseIdx < numPhases; ++phaseIdx)
			{
				U2FramePhase& curPhase = *curPass.GetFramePhase(phaseIdx);
				switch(curPhase.GetLightMode())
				{
				case U2FramePhase::LM_OFF:
					RenderPhaseOffLight(curPhase);
					break;
				case U2FramePhase::LM_FIXED_FUNC:
					break;
				case U2FramePhase::LM_SHADER:
					RenderPhaseShaderLight(curPhase);
					break;
				}
			}			
			//debug_con << "Curr Pass End: " << passIdx;
			//DebugConsole::flush();
			PROFILER_DECLARE(prof);
			PROFILER_START(prof);			
			curPass.End();	// End RenderTarget
			PROFILER_STOP(prof);
			PROFILER_OUTPUT(prof);		

		}
	}

	//debug_con << "Frame Section Begin";
	//DebugConsole::flush();	
	PROFILER_START(prof);		
	frameSection.End();
	PROFILER_STOP(prof);
	PROFILER_OUTPUT(prof);		
}

void U2SceneMgr::RenderShadowMap(U2FramePass& curPass)
{
	// calculate the distances of split planes	
	U2Dx9ShadowMgr::Instance()->RenderShadowMap();
	
}


//-------------------------------------------------------------------------------------------------
void U2SceneMgr::RenderShadowVolume(U2FramePass& curPass)
{
	U2Dx9ShadowMgr* pShadowMgr = U2Dx9ShadowMgr::Instance();	

	int maxShadowLits = 1;
	if(curPass.GetShadowTechnique() == U2FramePass::MULTI_LIGHT)
	{
		maxShadowLits = 4;
		m_pRenderer->BeginRenderTarget(curPass.GetRenderTarget(), U2Dx9Renderer::CLEAR_NONE);
		m_pRenderer->UpdateScissorRect();
	}
	m_pRenderer->GetLightMgr()->SetLightingType(U2Dx9LightMgr::LT_OFF);
	m_pRenderer->m_bWvpMatOnly = true;
	m_pRenderer->SetHint(U2Dx9Renderer::MODEL_VIEW_PROJ_ONLY, true);

	D3DXMATRIX shadowProj = m_pRenderer->m_aTM[U2Dx9Renderer::TMT_SHADOWPROJ];
	m_pRenderer->PushTransform(U2Dx9Renderer::TMT_PROJ, shadowProj);

	UpdatePerSceneSharedShaderParams();

	if(m_pRenderer->BeginScene())
	{
		if(curPass.GetShadowTechnique() == U2FramePass::MULTI_LIGHT)
		{
			m_pRenderer->SetBackgroundColor(D3DXCOLOR(0.f,0.f, 0.f, 0.f));
			m_pRenderer->SetDepthClear(0.f);
			m_pRenderer->SetStencilClear(0);
			m_pRenderer->ClearBuffers(U2Dx9Renderer::CLEAR_BACKBUFFER);
		}

		uint32 numShadowLights = m_aShadowLights.FilledSize();
		if(numShadowLights > 0 && m_aShadows.FilledSize() > 0)
		{
			// Begin Shadow
			if(pShadowMgr->BeginShadow())
			{
				int shadowLitIdx;

				for(shadowLitIdx = 0; shadowLitIdx < U2Math::Min<int>(maxShadowLits, numShadowLights);
					++shadowLitIdx)
				{
					// only process non-occluded lights
					const LightInfo& litInfo = *m_aShadowLights[shadowLitIdx];
					if(!litInfo.m_pRenderContext->GetFlag(U2RenderContext::OCCLUDED))
					{
						U2LightNode* pLightNode = litInfo.m_pLightNode;
						U2ASSERT(pLightNode->GetCastShadows());

						pLightNode->ApplyLight(litInfo.m_shadowLightMask);
						pLightNode->UpdateLight();
						const U2Light& lit = *pLightNode->GetLight();
						float fShadowIntensity = litInfo.m_pRenderContext->GetShadowIntensity();

						pShadowMgr->BeginLight(lit);
						this->ApplyLightScissors(litInfo);

						 // FIXME: sort shadow nodes by shadow caster geometry
						uint32 numShadows = m_aShadows.FilledSize();
						uint32 shadowIdx;
						for(shadowIdx = 0; shadowIdx < numShadows; ++shadowIdx)
						{
							ShadowInfo& shadow = *m_aShadows[shadowIdx];
							U2ASSERT(shadow.m_pRenderContext->GetFlag(U2RenderContext::SHADOW_VISIBLE));
							if(!shadow.m_pRenderContext->GetFlag(U2RenderContext::OCCLUDED))
							{
								// check if current shadow casting light sees this shape
								if(GetObjectLightLinks())
								{


								}
								else 
								{
									// no obey light links, just render the shadow
									shadow.m_pShadowNode->RenderShadowVolume(this, 
										shadow.m_pRenderContext);									
								}
							}
						}
						pShadowMgr->EndLight();

						 // if multilight shadowing, store the stencil buffer in an accumulation render target
						if(curPass.GetShadowTechnique() == U2FramePass::MULTI_LIGHT)
						{
							this->CopyStencilBuffertoTexture(curPass, litInfo.m_shadowLightMask);
						}
					} // end if

				} // end each for
				pShadowMgr->EndShadow();
			} // end beginShadow
			
		}
		m_pRenderer->EndScene();
	}
	if(curPass.GetShadowTechnique() == U2FramePass::MULTI_LIGHT)
	{
		m_pRenderer->EndRenderTarget();
	}
	m_pRenderer->PopTransform(U2Dx9Renderer::TMT_PROJ);
	m_pRenderer->SetHint(U2Dx9Renderer::MODEL_VIEW_PROJ_ONLY, false);	
}

//-------------------------------------------------------------------------------------------------
void U2SceneMgr::RenderPhaseOffLight(U2FramePhase& curPhase)
{
	unsigned int numSeqs = curPhase.Begin();
	m_pRenderer->GetLightMgr()->SetLightingType(U2Dx9LightMgr::LT_OFF);

	unsigned int seqIdx;
	for(seqIdx = 0; seqIdx < numSeqs; ++seqIdx)
	{
		U2FrameSequence& curSeq = *curPhase.GetFrameSeq(seqIdx);
		bool bShaderUpdateEnabled = curSeq.GetShaderUpdatesEnabled();
		int bucketIdx = curSeq.GetShaderBucketIdx();
		U2ASSERT(bucketIdx >= 0);

		const U2PrimitiveVec<uint16>& meshes = m_aMeshBuckts[bucketIdx];
		unsigned int numMesh = meshes.FilledSize();
		if(numMesh > 0)
		{
			unsigned int numSeq = curSeq.Begin();
			unsigned int seqIdx;
			for(seqIdx = 0; seqIdx < numSeq; ++seqIdx)
			{
				curSeq.BeginPass(seqIdx);

				unsigned int meshIdx;
				for(meshIdx = 0; meshIdx < numMesh; ++meshIdx)
				{
					const MeshInfo& meshInfo = *m_aMeshes[meshes[meshIdx]];
					nMeshGroup& mesh = *meshInfo.m_pMeshGroup;
					// meshvisible
					// occuluded
					if(!meshInfo.m_pRenderContext->GetFlag(U2RenderContext::OCCLUDED))
					{
						
						for(uint32 i=0; i < mesh.GetEffectCnt(); ++i)
						{
							U2Dx9FxShaderEffect* pEffect = 
								DynamicCast<U2Dx9FxShaderEffect>(mesh.GetEffect(i));
							if (pEffect)
							{
								pEffect->GetEffectShader()->SetAttribute(pEffect->GetShaderAttb());

								m_pRenderer->SetCurrEffectShader(pEffect->GetEffectShader());			
								break;
							}
						}	


						D3DXMATRIX hWorldMat;
						mesh.m_tmWorld.GetHomogeneousMat(hWorldMat);
						m_pRenderer->SetTransform(U2Dx9Renderer::TMT_WORLD, hWorldMat);

						if(bShaderUpdateEnabled)
						{
							// Update shader parameter
							for(uint32 i=0; i < mesh.GetEffectCnt(); ++i)
							{
								U2Dx9FxShaderEffect* pEffect = 
									DynamicCast<U2Dx9FxShaderEffect>(mesh.GetEffect(i));
								if(pEffect)
								{
									pEffect->GetEffectShader()->SetAttribute(
										meshInfo.m_pRenderContext->GetShaderOverides());
									pEffect->UpdateShader();									
									break;
								}
							}
							
						}
						
						// Render
						m_pRenderer->SetupMesh(mesh.GetMesh());						
						mesh.Render(this, meshInfo.m_pRenderContext);
					}
				}
				curSeq.EndPass();
			}			
			curSeq.End();
		}
		
	}
	curPhase.End();
}


//-------------------------------------------------------------------------------------------------
void U2SceneMgr::ApplyLightScissors(const LightInfo& lightInfo)
{
	if(m_bClipPlaneFencing)
		m_pRenderer->SetScissorRect(lightInfo.m_scissorRect);
}

//-------------------------------------------------------------------------------------------------
void U2SceneMgr::ApplyLightClipPlanes(const LightInfo& lightInfo)
{
	if(m_bClipPlaneFencing)
		m_pRenderer->SetUserClipPlanes((U2Plane*)&lightInfo.m_aPlane[0]);
}


//-------------------------------------------------------------------------------------------------
void U2SceneMgr::RenderPhaseShaderLight(U2FramePhase& curPhase)
{
	U2Dx9LightMgr *pLitMgr = m_pRenderer->GetLightMgr();
	pLitMgr->SetLightingType(U2Dx9LightMgr::LT_SHADER);
	PROFILER_DECLARE(prof);

	unsigned int numLits = m_aLights.FilledSize();
	unsigned int litIdx;
	for(litIdx = 0; litIdx < numLits; ++litIdx)
	{
		/*debug_con << "LightIndex : " << litIdx;
		DebugConsole::flush();
		debug_con << "DisableLightRenderStates";
		DebugConsole::flush();*/
		
		PROFILER_START(prof);	
		pLitMgr->DisableLightRenderStates();
		PROFILER_STOP(prof);
		PROFILER_OUTPUT(prof);

		const LightInfo& litInfo = *m_aLights[litIdx];
		U2RenderContext* pLitRenderContext = litInfo.m_pRenderContext;

		if(!pLitRenderContext->GetFlag(U2RenderContext::OCCLUDED))
		{
			//dbgout("ApplyLight");			
			PROFILER_START(prof);	
			litInfo.m_pLightNode->ApplyLight(litInfo.m_shadowLightMask);
			PROFILER_STOP(prof);
			PROFILER_OUTPUT(prof);

			//dbgout("Phase Begin");			
			PROFILER_START(prof);	
			uint32 numSeqs = curPhase.Begin();
			PROFILER_STOP(prof);
			PROFILER_OUTPUT(prof);
			
			//dbgout("ApplyLightScissors");			
			PROFILER_START(prof);
			ApplyLightScissors(litInfo);
			PROFILER_STOP(prof);
			PROFILER_OUTPUT(prof);


			//dbgout("ApplyLightClipPlanes");			
			PROFILER_START(prof);
			if(litInfo.m_bClipping)
				ApplyLightClipPlanes(litInfo);
			PROFILER_STOP(prof);
			PROFILER_OUTPUT(prof);

			int seqIdx;
			for(seqIdx = 0; seqIdx < numSeqs; ++seqIdx)
			{
				U2FrameSequence& curSeq = *curPhase.GetFrameSeq(seqIdx);
				bool bShaderUpdateEnabled = curSeq.GetShaderUpdatesEnabled();
				int bucketIdx = curSeq.GetShaderBucketIdx();
				U2ASSERT(bucketIdx >= 0);

				const U2PrimitiveVec<uint16>& meshes = m_aMeshBuckts[bucketIdx];
				unsigned int numMesh = meshes.FilledSize();
				unsigned int meshIdx;

								
				if(numMesh > 0)
				{
					//dbgout("CurSeq Begin");			
					PROFILER_START(prof);
					unsigned int numSeqPasses = curSeq.Begin();
					PROFILER_STOP(prof);
					PROFILER_OUTPUT(prof);

					unsigned int seqPassIdx;
					for(seqPassIdx = 0; seqPassIdx < numSeqPasses; ++seqPassIdx)
					{
						//debug_con << "curSeq.BeginPass : " << seqPassIdx;
						DebugConsole::flush();
						PROFILER_START(prof);
						curSeq.BeginPass(seqPassIdx);						
						PROFILER_STOP(prof);
						PROFILER_OUTPUT(prof);

						for(meshIdx = 0; meshIdx < numMesh; ++meshIdx)
						{
							unsigned int idx = meshes[meshIdx];
							MeshInfo *pMeshInfo = m_aMeshes[idx];					

							nMeshGroup& mesh = *pMeshInfo->m_pMeshGroup;
							if(!m_aMeshes[meshIdx]->m_pRenderContext->GetFlag(U2RenderContext::OCCLUDED))
							{
								bool bShapeInfluncedByLight = true;
								if(m_bObjLightLinks)
								{
									// 
									bShapeInfluncedByLight = false;
								}
								if(bShapeInfluncedByLight)
								{
									//mesh.GetMesh()->ApplyShader(this);	

									//dbgout("SetCurrEffectShader Loop");			
									PROFILER_START(prof);
									for(uint32 i=0; i < mesh.GetEffectCnt(); ++i)
									{
										U2Dx9FxShaderEffect* pEffect = 
											DynamicCast<U2Dx9FxShaderEffect>(mesh.GetEffect(i));
										if(pEffect)
										{
											PROFILER_START(prof);
											pEffect->GetEffectShader()->SetAttribute(pEffect->GetShaderAttb());
											PROFILER_STOP(prof);
											//PROFILER_OUTPUT(prof);
											//dbgout(prof.GetTimeInSecs());
											m_pRenderer->SetCurrEffectShader(pEffect->GetEffectShader());			
											break;
										}
									}							
									PROFILER_STOP(prof);
									PROFILER_OUTPUT(prof);


								}

								D3DXMATRIX hWorldMat;

								//dbgout("SetTransform");			
								PROFILER_START(prof);
								mesh.m_tmWorld.GetHomogeneousMat(hWorldMat);
								m_pRenderer->SetTransform(U2Dx9Renderer::TMT_WORLD, hWorldMat);
								PROFILER_STOP(prof);
								PROFILER_OUTPUT(prof);


								if(bShaderUpdateEnabled)
								{
									//dbgout("bShaderUpdateEnabled Loop");			
									PROFILER_START(prof);

									//mesh.GetMesh()->UpdateEffectShader(this, pMeshInfo->m_pRenderContext);
									for(uint32 i=0; i < mesh.GetEffectCnt(); ++i)
									{
										U2Dx9FxShaderEffect* pEffect = 
											DynamicCast<U2Dx9FxShaderEffect>(mesh.GetEffect(i));
										if(pEffect)
										{
											pEffect->GetEffectShader()->SetAttribute(
												pMeshInfo->m_pRenderContext->GetShaderOverides());
											break;
										}
									}
									PROFILER_STOP(prof);
									PROFILER_OUTPUT(prof);
								}

								//dbgout("UpdateLight");			
								PROFILER_START(prof);
								litInfo.m_pLightNode->UpdateLight();
								PROFILER_STOP(prof);
								PROFILER_OUTPUT(prof);

								// Render
								//dbgout("Mesh Render");			
								PROFILER_START(prof);
								bool bFirstLightAlpha = curSeq.GetFirstLightAlphaEnabled();
								U2D3DXEffectShader* pShader = m_pRenderer->GetCurrEffectShader();
								if(0 == pMeshInfo->m_iLightPass++)
								{
									pShader->SetBool(U2FxShaderState::AlphaBlendEnable, bFirstLightAlpha);
								}
								else 
								{
									pShader->SetBool(U2FxShaderState::AlphaBlendEnable, true);
								}

								// Set D3D VB /IB
								for(uint32 i=0; i < mesh.GetEffectCnt(); ++i)
								{
									U2Dx9FxShaderEffect* pEffect = 
										DynamicCast<U2Dx9FxShaderEffect>(mesh.GetEffect(i));
									if(pEffect)
									{
										pEffect->UpdateShader();
										m_pRenderer->SetupMesh(mesh.GetMesh());
									}
								}

								mesh.Render(this, pMeshInfo->m_pRenderContext);
								PROFILER_STOP(prof);
								PROFILER_OUTPUT(prof);
							}
						}
						//dbgout("curSeq.EndPass");			
						PROFILER_START(prof);
						curSeq.EndPass();			
						PROFILER_STOP(prof);
						PROFILER_OUTPUT(prof);
					}
					
					PROFILER_START(prof);
					curSeq.End();
					PROFILER_STOP(prof);
					PROFILER_OUTPUT(prof);
				}				
			}			
			PROFILER_START(prof);
			curPhase.End();
			PROFILER_STOP(prof);	

		}

	}	
	//dbgout("ResetLightScissorsAndClipPlanes");			
	PROFILER_START(prof);
	ResetLightScissorsAndClipPlanes();
	PROFILER_STOP(prof);
	PROFILER_OUTPUT(prof);
}




//-------------------------------------------------------------------------------------------------
void U2SceneMgr::RenderMeshShaderLight(MeshInfo& meshInfo, const U2FrameSequence& frameSeq)
{
	bool bFirstLightAlpha = frameSeq.GetFirstLightAlphaEnabled();
	if(0 == meshInfo.m_iLightPass++)
	{
		m_pRenderer->GetCurrEffectShader()->SetBool(U2FxShaderState::AlphaBlendEnable, bFirstLightAlpha);
	}
	else 
	{
		m_pRenderer->GetCurrEffectShader()->SetBool(U2FxShaderState::AlphaBlendEnable, true);
	}

	((U2N2Mesh*)meshInfo.m_pMeshGroup)->Render(this, meshInfo.m_pRenderContext);

}

//-------------------------------------------------------------------------------------------------
void U2SceneMgr::ResetLightScissorsAndClipPlanes()
{
	if(m_bClipPlaneFencing)
	{
		static const U2Rect<float> fullScreenRect(0.0f, 0.0f, 1.0f, 1.0f);
		m_pRenderer->SetScissorRect(fullScreenRect);
		m_pRenderer->SetUserClipPlanes(0);
	}
}

//-------------------------------------------------------------------------------------------------
void U2SceneMgr::UpdatePerFrameSharedShaderParams()
{
	if(m_pRenderer->m_spSharedShader)
	{	
		float fTime = GetCurrTimeInSec();

		m_pRenderer->m_spSharedShader->SetFloat(U2FxShaderState::Time, fTime);
	}
}

//-------------------------------------------------------------------------------------------------
void U2SceneMgr::UpdatePerSceneSharedShaderParams()
{
	if(m_pRenderer->m_spSharedShader)
	{		
		float fW = float(m_pRenderer->GetCurrRenderTargets()->GetWidth(0));
		float fH = float(m_pRenderer->GetCurrRenderTargets()->GetHeight(0));
		D3DXVECTOR4 dispRes(fW, fH, 0.0f, 0.0f);
		m_pRenderer->m_spSharedShader->SetVector4(U2FxShaderState::DisplayResolution, dispRes);



		D3DXVECTOR4 halfPixelSize(VECTOR4_ZERO);
		halfPixelSize.x = (1.0f / fW) * 0.5f;
		halfPixelSize.y = (1.0f / fH) * 0.5f;
		m_pRenderer->m_spSharedShader->SetVector4(U2FxShaderState::HalfPixelSize, halfPixelSize);	
	}
}

//-------------------------------------------------------------------------------------------------
void U2SceneMgr::BeginShape()
{
	int numPasses = m_pRenderer->m_spShapeShader->Begin(true);
	U2ASSERT(1 == numPasses);
}


void U2SceneMgr::DrawShape(U2Mesh* pMesh, 
			   const D3DXMATRIX& modelMat, const D3DXVECTOR4& color, bool bSetShaderParam)
{
	if(bSetShaderParam) 
	{	
		// update color in shader
		if(m_pRenderer->m_spShapeShader->IsParamUsed(U2FxShaderState::MatDiffuse))
		{
			m_pRenderer->m_spShapeShader->SetVector4(U2FxShaderState::MatDiffuse,color);
		}
	}

	m_pRenderer->PushTransform(U2Dx9Renderer::TMT_WORLD, modelMat);
	m_pRenderer->m_spShapeShader->BeginPass(0);
	
	// Draw 
	U2MeshData *pModelData = pMesh->GetMeshData();
	m_pRenderer->PackMeshData(pModelData, *pModelData->GetVertexAttbs(),
		*pModelData->GetVertexAttbs());

	

	m_pRenderer->m_spShapeShader->EndPass();
	m_pRenderer->PopTransform(U2Dx9Renderer::TMT_WORLD);
	
}

//-------------------------------------------------------------------------------------------------
void U2SceneMgr::EndShape()
{
	m_pRenderer->m_spSharedShader->End();

	HRESULT hr = m_pRenderer->GetD3DDevice()->SetStreamSource(0, 0, 0, 0);
	U2_DXTrace(hr, _T("SetStreamSource() on Failed"));

	hr = m_pRenderer->GetD3DDevice()->SetIndices(0);
	U2_DXTrace(hr, _T("SetIndices() failed"));	
}



void U2SceneMgr::AttachMeshGroup(U2N2Mesh* pMesh)
{
	unsigned int i;
	for(i=0; i < pMesh->GetNumGroups(); ++i)
	{
		nMeshGroup* group = &pMesh->Group(i);
		m_spRootNode->AttachChild(group);
	}

	m_spRootNode->Update(0.f);
	m_spRootNode->UpdateBound();
}

//------------------------------------------------------------------------------
/**
Copy the current stencil buffer state into a texture color channel.
This accumulates the stencil bits for up to 4 light sources
(one per RGBA channel).
*/
void U2SceneMgr::CopyStencilBuffertoTexture(U2FramePass& pass,const D3DXVECTOR4& shadowLitMask)
{
	U2D3DXEffectShader* pShader = pass.GetEffectShader();
	if(pShader)
	{
		pShader->SetVector4(U2FxShaderState::ShadowIndex, shadowLitMask);		
		pShader->Begin(true);
		pShader->BeginPass(0);
		pass.DrawFullScreenQuad();
		pShader->EndPass();
		pShader->End();
	}
}

struct LightPriority 
{
	int litIdx;
	float priority;
};

int Compare(const void* elem1, const void* elem2)
{
	LightPriority* lit1 = (LightPriority*)elem1;
	LightPriority* lit2 = (LightPriority*)elem2;

	return (lit1->priority - lit2->priority);
}

const D3DXVECTOR4 GetShadowLitColor(int shadowLitIdx, float value)
{
	D3DXVECTOR4 idxCol;
	idxCol.x = (shadowLitIdx == 0) ? value : 0.0f;
	idxCol.y = (shadowLitIdx == 1) ? value : 0.0f;
	idxCol.z = (shadowLitIdx == 2) ? value : 0.0f;
	idxCol.w = (shadowLitIdx == 3) ? value : 0.0f;
	return idxCol;
}

//------------------------------------------------------------------------------
/**
This method goes through all attached light sources and decides which
4 of them should cast shadows. This takes the occlusion status, distance and
range and intensity into account. The method should be called after
occlusion culling. The result is that the shadowLightArray will be filled.
*/
// 오리지널 nebula 코드와 다름.
void U2SceneMgr::GatherShadowLights()
{
	D3DXVECTOR3 viewerPos = 
		*((D3DXVECTOR3*)&U2Dx9Renderer::GetRenderer()->m_aTM[U2Dx9Renderer::TMT_INVVIEW]._31);

	unsigned int numLights = this->m_aLights.FilledSize();

	

	std::vector<LightPriority> m_priorLits;

	int litIdx;
	for (litIdx  = 0; litIdx < numLights; litIdx++)
	{
		LightInfo& litInfo = *(this->m_aLights[litIdx]);

		// only look at shadow casting light sources
		if ( litInfo.m_pRenderContext->GetFlag(U2RenderContext::CAST_SHADOWS))
		{

			if ( (!litInfo.m_pRenderContext->GetFlag(U2RenderContext::OCCLUDED)))
			{
				U2LightNode* litNode = litInfo.m_pLightNode;
				if (litNode->GetCastShadows())
				{
					float shadowIntensity = litInfo.m_pRenderContext->GetShadowIntensity();
					litInfo.m_shadowLightMask = U2Dx9Renderer::GetShadowLightIndexVector(litIdx, shadowIntensity);

					float priority;					
					
					switch (litNode->GetLight()->m_eLitType)
					{
					case U2Light::LT_POINT:
						{					
							D3DXVECTOR3 vDiff = litNode->GetWorldTranslate() - viewerPos;
							float len = D3DXVec3Length(&vDiff);
							priority = -len / litNode->GetLight()->m_fLitRange;
						}
						break;						
					case U2Light::LT_DIRECTIONAL:
						priority = 100000.0f;
						break;

					default:
						priority = 0.0f;
						break;	
					}
					LightPriority data;
					data.litIdx = litIdx;
					data.priority = priority;
					m_priorLits.push_back(data);
				}				
			}
		}
	}

	 // the 4 highest priority light sources are now in the priority array
	std::qsort(&m_priorLits, m_priorLits.size(), sizeof(LightPriority), Compare);

	
	for (size_t litIdx = 0; litIdx < m_priorLits.size(); litIdx++)
	{
		LightInfo& litInfo = *(this->m_aLights[litIdx]);
		float shadowIntensity = litInfo.m_pRenderContext->GetShadowIntensity();
		litInfo.m_shadowLightMask = ::GetShadowLitColor(litIdx, shadowIntensity);
		m_aShadowLights.AddElem(&litInfo);
	}
}
