#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9Renderer.h"

#include "U2RenderTarget.h"
#include "U2Dx9LightMgr.h"
#include "U2Dx9RenderStateMgr.h"
#include "U2Dx9TextureMgr.h"
#include "U2Dx9RenderStateMgr.h"
#include "U2Dx9LightMgr.h"
#include "U2Dx9VertexBufferMgr.h"
#include "U2Dx9IndexBufferMgr.h"
#include <U2_3D/src/Shader/U2VertexAttributes.h>

LPDIRECT3D9	U2Dx9Renderer::ms_pD3D9 = NULL;
unsigned int U2Dx9Renderer::ms_uiMaxTextureBlendStages;
unsigned int U2Dx9Renderer::ms_uiMaxSimultaneousTextures;
unsigned int U2Dx9Renderer::ms_uiMaxSamplers;
unsigned int U2Dx9Renderer::ms_uProjectedTextureFlags;
unsigned int U2Dx9Renderer::ms_uMaxActiveLights;

U2AllocDeclareStatics(U2Shader, sizeof(U2Shader) * 256);
U2AllocDeclareStatics(U2Dx9FrameBuffer, sizeof(U2Dx9FrameBuffer) * 8);



DWORD U2Dx9Renderer::ms_adwTexMinFilter[U2Dx9BaseTexture::MAX_FILTER_TYPES] =
{
	D3DTEXF_POINT,  // U2Dx9BaseTexture::MM_NEAREST
	D3DTEXF_LINEAR, // U2Dx9BaseTexture::MM_LINEAR
	D3DTEXF_POINT,  // U2Dx9BaseTexture::MM_NEAREST_NEAREST
	D3DTEXF_POINT,  // U2Dx9BaseTexture::MM_NEAREST_LINEAR
	D3DTEXF_LINEAR, // U2Dx9BaseTexture::MM_LINEAR_NEAREST
	D3DTEXF_LINEAR, // U2Dx9BaseTexture::MM_LINEAR_LINEAR
};

DWORD U2Dx9Renderer::ms_adwTexMipFilter[U2Dx9BaseTexture::MAX_FILTER_TYPES] =
{
	D3DTEXF_NONE,   // U2Dx9BaseTexture::MM_NEAREST
	D3DTEXF_NONE,   // U2Dx9BaseTexture::MM_LINEAR
	D3DTEXF_POINT,  // U2Dx9BaseTexture::MM_NEAREST_NEAREST
	D3DTEXF_LINEAR, // U2Dx9BaseTexture::MM_NEAREST_LINEAR
	D3DTEXF_POINT,  // U2Dx9BaseTexture::MM_LINEAR_NEAREST
	D3DTEXF_LINEAR, // U2Dx9BaseTexture::MM_LINEAR_LINEAR
};

DWORD U2Dx9Renderer::ms_adwTexWrapMode[U2Dx9BaseTexture::MAX_WRAP_TYPES] =
{
	D3DTADDRESS_CLAMP,      // U2Dx9BaseTexture::CLAMP
	D3DTADDRESS_WRAP,       // U2Dx9BaseTexture::REPEAT
	D3DTADDRESS_MIRROR,     // U2Dx9BaseTexture::MIRRORED_REPEAT
	D3DTADDRESS_BORDER,     // U2Dx9BaseTexture::CLAMP_BORDER
	D3DTADDRESS_CLAMP,      // U2Dx9BaseTexture::CLAMP_EDGE
};



//#define USE_RENDERSTATE 



U2Dx9Renderer* U2Dx9Renderer::ms_pRenderer;

#define D3D_SUCCESS		0
#define D3D_FAIL		-1

U2Dx9Renderer*	U2Dx9Renderer::GetRenderer()
{
	return ms_pRenderer;
}


void U2Dx9Renderer::InitDefault()
{
	m_pD3DDevice9 = 0;
	ZeroMemory(&m_d3dCaps9, sizeof(m_d3dCaps9));
	m_hDevice = 0;
	m_hFocus = 0;
	m_uAdapter = D3DADAPTER_DEFAULT;	// 장치 어뎁터 아이디
	m_eD3DDevType = D3DDEVTYPE_HAL;		// D3D 장치 타입
	m_uBehaviorFlags = 0;
	m_eD3DAdapterFormat = D3DFMT_UNKNOWN;

	m_uHWBones = 0;				// 하드웨어 스키닝시 정점에 영향을 주는 뼈대의 최대수
	m_uMaxStreams = 0;			// 쉐이터 스트림 수
	m_uMaxPixelShaderVer = 0;	//	그래픽 카드가 지원하는 최대 쉐이더 버전
	m_uMaxVertexShaderVer = 0;
	m_bMipmapCubeMaps = false;
	m_bMRTPostPixelShaderBlending = false;
	
	// 장치 리셋, 로스트 관련 
	m_uiResetCount = 0;
	m_bDeviceLost = false;

	m_uBgColor = D3DCOLOR_XRGB(128, 128, 128);	// 화면 배경 색
	m_fZClear = 1.0f;
	m_uStencilClear = 0;
	m_uRenderFlags = 0;
	
	m_fNearDepth = 0.1f;			// near 
	m_fDepthRange = 10000.f;		// far 

	// ViewPort 초기화
	ZeroMemory(&m_d3dPort, sizeof(D3DVIEWPORT9));
	m_d3dPort.X = 0;
	m_d3dPort.Y = 0;
	m_d3dPort.Width = 1;
	m_d3dPort.Height = 1;
	m_d3dPort.MinZ = 0.0f;
	m_d3dPort.MaxZ = 1.f;

	m_bWvpMatOnly = false;
	unsigned int i;
	for(i= 0; i < MAX_TRANSFORM_TYPES; ++i)
	{
		D3DXMatrixIdentity(&m_aTM[i]);
	}

	memset(m_aiTMTopOfStack, 0, sizeof(m_aiTMTopOfStack));

	m_pCurrRenderTarget = NULL;

	m_ppActiveSamplers = NULL;
	m_uNumActiveSamplers = 0;
	
	m_aspLights = 0;

	m_pVBMgr = NULL;
	m_pIBMgr = NULL;

	m_pTexMgr = 0;				// 텍스쳐 관리자
	m_pRenderState = 0;			// 렌더 스테이트 
	m_pLightMgr = 0;			// 라이트 관리자


	m_uDeviceThreadId = 0;
	
	m_pCurrRenderTarget = NULL;
	m_spDefaultRenderTarget= NULL;

	m_pCurrRSC = NULL;
	m_pCurrRSC = NULL;

	m_eFrameState = FS_OUTSIDE_FRAME;	

	m_pD3DEffectPool = NULL;	

	m_iHints = COUNT_STATS;
	
}


int U2Dx9Renderer::CreateDirect3D9()
{
	int ret = D3D_SUCCESS;

	if(ms_pD3D9 == 0)
	{
		ms_pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
		
		if(ms_pD3D9 == 0)
		{		
			// _T를 안해도 내부적으로 유니코드 문자로 변환해서 출력.
			// 이후로 언급안함.				
			FMsg("Create D3D9 Failed...\n");
			ret = D3D_FAIL;
		}
		else 
		{
			FMsg("Create D3D Success...\n");
			ret = D3D_SUCCESS;			
		}
	}



	return ret;
}


void U2Dx9Renderer:: Terminate()
{	
	// 원인 찾아 해결..
	// 원인 : D3DDevice 가 해제되지 않음..
	int refc = (int)ms_pD3D9->Release();	
	//refc = ms_pD3D9->Release();
	//U2ASSERT(refc == 0);	
}



U2Dx9Renderer::U2Dx9Renderer()
{

	ms_pRenderer = this;
	InitDefault();
}


U2Dx9Renderer::~U2Dx9Renderer()
{
	if(m_pD3DDevice9)
	{
		uint32 u;
		if(m_pRenderState)
		{
			for(u = 0; u < ms_uiMaxTextureBlendStages; ++u)
				m_pRenderState->SetTexture(u, 0);

			for(u=0; u < m_uMaxStreams; ++u)
				m_pD3DDevice9->SetStreamSource(u, 0, 0, 0);
			m_pD3DDevice9->SetIndices(0);
		}
	}

	//U2_DELETE [] m_aspLights;

	//HWND hWnd;
	//U2RenderTarget* pRTItr = m_renderTargets.FirstVal(hWnd);
	//HWND hPrevWnd;
	//U2RenderTarget* pPrevRT;
	//while(pRTItr)
	//{
	//	hPrevWnd = hWnd;
	//	pPrevRT = pRTItr;
	//	pRTItr = m_renderTargets.NextVal(hWnd);
	//	m_renderTargets.Remove(hPrevWnd);
	//	
	//}	

	//m_spShapeShader = 0;
	//m_spSharedShader = 0;
		
	
		
	U2D3DXEffectShader* pShader = U2D3DXEffectShader::GetHead();
	U2D3DXEffectShader* pDelElem;
	while(pShader)
	{
		pDelElem = pShader;		
		pShader = pShader->GetNext();	
		U2_DELETE pDelElem;				
	}		

//	ULONG rf = m_pD3DEffectPool->Release();
	//SAFE_RELEASE(m_pD3DEffectPool);
	

	U2PoolShutdown(U2Shader);
		
	m_spDefaultRenderTarget = 0;

	if(m_pRenderTargets)
	{	
		m_pRenderTargets->RemoveAll();
		U2_DELETE m_pRenderTargets;
		m_pRenderTargets = NULL;
	}

	U2PoolShutdown(U2Dx9FrameBuffer);

	if(m_pVBMgr)
	{	
		U2_DELETE m_pVBMgr;
		m_pVBMgr = NULL;
	}

	if(m_pIBMgr)
	{
		U2_DELETE m_pIBMgr;
		m_pIBMgr = NULL;
	}

	U2_DELETE m_pRenderState;
	m_pRenderState = NULL;

	U2_DELETE m_pTexMgr;
	m_pTexMgr = NULL;

	U2_DELETE m_pLightMgr;
	m_pLightMgr = NULL;

	U2_FREE(m_ppActiveSamplers);
	m_ppActiveSamplers = NULL;	

	for(unsigned int i = 0; i < m_fontArray.Size(); ++i)
	{
		SAFE_RELEASE(m_fontArray[i]);
	}	


	int refc = (int)(m_pD3DDevice9->Release());								

	//SAFE_RELEASE(m_pD3DDevice9);
	
	

	
	
}

U2Dx9Renderer* U2Dx9Renderer::Create(unsigned int uWidth, unsigned int uHeight, 
									 unsigned int uUseFlags, HWND hDevice, HWND hFocus, 
									 bool useFixedFuncRS, 
									 unsigned int uAdapter /* = D3DADAPTER_DEFAULT */, 
									 DeviceDesc eDesc /* = DEVDESC_PURE */, 
									 FrameBufferFormat eFBFormat /* = FBFMT_UNKNOWN */,
									 DepthStencilFormat eDSFormat /* = DSFMT_UNKNOWN */,
									 PresentationInterval ePresentationInterval /* = PRESENT_INTERVAL_ONE */, 
									 SwapEffect eSwapEffect /* = SWAPEFFECT_DEFAULT */, 
									 unsigned int uFBMode /* = FBMODE_DEFAULT */, 
									 unsigned int uBackBufferCount /* = 1 */, 
									 unsigned int uRefreshRate /* = REFRESHRATE_DEFAULT */)
{
	if(CreateDirect3D9() == D3D_FAIL)
	{
		FDebug("U2Dx9Renderer::Create Failed");
		return NULL;
	}

	U2Dx9Renderer* pRenderer = U2_NEW U2Dx9Renderer();
	U2ASSERT(pRenderer);

	bool bSuccess = pRenderer->Initialize(uWidth, uHeight, uUseFlags, hDevice, hFocus, useFixedFuncRS,
		uAdapter, eDesc, eFBFormat, eDSFormat, ePresentationInterval, eSwapEffect, uFBMode, 
		uBackBufferCount, uRefreshRate);

	if( bSuccess == false)
	{
		U2_DELETE pRenderer;
		return NULL;
	}

	return pRenderer;
}




bool U2Dx9Renderer::Initialize(unsigned int uWidth, unsigned int uHeight, 
									 unsigned int uUseFlags, HWND hDevice, HWND hFocus, 
									 bool useFixedFuncRS, 
									 unsigned int uAdapter /* = D3DADAPTER_DEFAULT */, 
									 DeviceDesc eDesc /* = DEVDESC_PURE */, 
									 FrameBufferFormat eFBFormat /* = FBFMT_UNKNOWN */,
									 DepthStencilFormat eDSFormat /* = DSFMT_UNKNOWN */,
									 PresentationInterval ePresentationInterval /* = PRESENT_INTERVAL_ONE */, 
									 SwapEffect eSwapEffect /* = SWAPEFFECT_DEFAULT */, 
									 unsigned int uFBMode /* = FBMODE_DEFAULT */, 
									 unsigned int uBackBufferCount /* = 1 */, 
									 unsigned int uRefreshRate /* = REFRESHRATE_DEFAULT */)
{
	// 초기값 저장
	m_uInitWidth = uWidth;
	m_uInitHeight = uHeight;
	m_uInitUseFlags = uUseFlags;
	m_hInitDevice = hDevice;
	m_hInitFocus = hFocus;
	m_uInitAdapter = uAdapter;
	m_eInitDesc = eDesc;
	m_eInitFBFormat = eFBFormat;
	m_eInitDSFormat = eDSFormat;
	m_eInitPresentationInterval = ePresentationInterval;
	m_uInitFBMode = uFBMode;
	m_uInitBackBufferCnt = uBackBufferCount;
	m_eInitSwapEffect = eSwapEffect;
	m_uInitFBMode = uFBMode;
	m_uInitBackBufferCnt = uBackBufferCount;
	m_uInitRefreshRate = uRefreshRate;

	
	m_uAdapter = uAdapter;

	m_hDevice = hDevice;
	m_hFocus = hFocus;	

	
#ifdef DEBUG_SHADER 
	eDesc = DEVDESC_REF;
#endif 

	if(!SelectD3DDevicetype(eDesc, m_eD3DDevType, m_uBehaviorFlags))
	{
		FDebug("U2Dx9Renderer::Initialize..");
		return false;
	}

	

	// D3D 부동 소숫점 계산을 위한 정확성 설정
	if(uUseFlags & USE_FPU_PRESERVE)
		m_uBehaviorFlags |= D3DCREATE_FPU_PRESERVE;

	// D3D 프로그램이 멀티 쓰레드에 안전하도록 장치에 요청함.
	if(uUseFlags & USE_MULTITHREADED)
		m_uBehaviorFlags |= D3DCREATE_MULTITHREADED;

//#ifdef DEBUG_VS
//	if(m_uBehaviorFlags != D3DDEVTYPE_REF)
//	{
//		m_uBehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
//		m_uBehaviorFlags &= ~D3DCREATE_PUREDEVICE;
//		m_uBehaviorFlags &= ~D3DCREATE_SOFTWARE_VERTEXPROCESSING;
//		
//	}
//#endif 

	



	// 장치 만들기 

	HRESULT hRet;

	D3DPRESENT_PARAMETERS d3dPP;
	ZeroMemory(&d3dPP, sizeof(d3dPP));

	if(InitializePresentParams(hDevice, uWidth, uHeight, uUseFlags, uFBMode, 
		eFBFormat,eDSFormat, uBackBufferCount,
		eSwapEffect, uRefreshRate, ePresentationInterval, d3dPP))
	{
		uBackBufferCount = d3dPP.BackBufferCount;

#ifdef _DEBUG

		for(uint32 adapter=0; adapter < ms_pD3D9->GetAdapterCount(); adapter++)
		{
			D3DADAPTER_IDENTIFIER9 id;

			ms_pD3D9->GetAdapterIdentifier(adapter, 0, &id);
			if(_tcsstr(id.Description, _T("PerfHUD")) != 0)
			{
				m_uAdapter = adapter;
				m_eD3DDevType = D3DDEVTYPE_REF;
				break;
			}
		}
#endif

		hRet = ms_pD3D9->CreateDevice(m_uAdapter, m_eD3DDevType, m_hFocus, 
			m_uBehaviorFlags, &d3dPP, &m_pD3DDevice9);

		m_uDeviceThreadId = (uint32)GetCurrentThreadId();

		if(SUCCEEDED(hRet))
		{
			FMsg("Create D3D Device Success...");
		}

		m_bDeviceLost = false;			
	}

	if(!m_pD3DDevice9) 
	{
		FDebug("U2Dx9Renderer::Initialize Failed...");
		return false;
	}

	
	if(!InitDeviceCaps(d3dPP))
	{
		return false;
	}

	m_bSWVertexCapable = 
		  ((m_uBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) == 0);
	m_bSWVertexSwitchable = 
		((m_uBehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) != 0);


	m_pRenderTargets = U2_NEW U2THashTable<HWND, U2RenderTargetPtr>(4);

	// 현재 백버퍼와 깊이버퍼 정보를 저장한다.
	U2Dx9FrameBuffer* pBackBuffer = U2Dx9FrameBuffer::
		CreateBackBuffer(m_pD3DDevice9, d3dPP);

	U2Dx9FrameBuffer* pDSBuffer = U2Dx9FrameBuffer::
		CreateDepthStencilBuffer(m_pD3DDevice9);
	m_spDefaultRenderTarget = U2RenderTargets::Create(1, this);
	m_spDefaultRenderTarget->AttachBackBuffer(pBackBuffer, 0);
	m_spDefaultRenderTarget->AttachDSBuffer(pDSBuffer);

	m_pRenderTargets->Insert(hDevice, m_spDefaultRenderTarget);

	m_eD3DAdapterFormat = m_spDefaultRenderTarget->GetBuffer(0)->GetSurfaceFormat();

	D3DDISPLAYMODE dispMode;
	hRet = m_pD3DDevice9->GetDisplayMode(0, &dispMode);

	m_eD3DAdapterFormat = dispMode.Format;


	// 렌더 스테이트 매니저 초기화 
	if ( useFixedFuncRS )
	{	
		if(m_pRenderState)
		{
			m_pRenderState->Reset(m_d3dCaps9, true);
		}
		else 
		{
			// commented date : 2010-03-08
			// Nebula Shader System과 충돌.. 
			// Camera의 변환에 영향을 주므로 주의..
			m_pRenderState = U2Dx9RenderStateMgr::Create(this, true);

			// Shader 초기화
		}
	}

	//// Set RenderTarget..

	// 매니저 생성 
#if 1
	m_pTexMgr = U2_NEW U2Dx9TextureMgr(this);
	
	m_pVBMgr = U2_NEW U2Dx9VertexBufferMgr(m_pD3DDevice9);
	U2ASSERT(m_pVBMgr);

	m_pIBMgr = U2_NEW U2Dx9IndexBufferMgr(m_pD3DDevice9);
	U2ASSERT(m_pIBMgr);

	m_pTexMgr = U2_NEW U2Dx9TextureMgr(this);
	m_pLightMgr = U2_NEW U2Dx9LightMgr(m_pRenderState, m_pD3DDevice9);
#endif

	if(d3dPP.MultiSampleType != D3DMULTISAMPLE_NONE)
	{
		if(m_pRenderState)
			m_pRenderState->SetAntiAlias(true);
	}	

	//// Create the default font.
	m_iFontID = LoadFont(_T("Arial"),12,false,false);
	U2ASSERT(m_iFontID == 0);

	// Create Light Array (사용중)
	m_aspLights = U2_NEW  U2LightPtr[ms_uiMaxSimultaneousTextures];

	// Create D3DXEffectPool
	HRESULT hr = D3DXCreateEffectPool(&m_pD3DEffectPool);
	if (FAILED(hr))
	{
		FDebug("U2Dx9Renderer: Could not create effect pool!\nDirectX Error is: %s\n", DXGetErrorString(hr));
		return false;
	}

	// 현재는 xml에서 로드한 결과로만 로드하지만..
	// 결과에서 찾지 못하면 새로 생성해주는 처리 필요.
	// Shape shader 
	// 	
#if 1
	m_spShapeShader = U2_NEW U2D3DXEffectShader(this);

	m_spShapeShader->SetFilename(_T("shape.fx"));

	if(!m_spShapeShader->LoadResource())
	{
		U2ASSERT(m_spShapeShader);		
		return false;
	}
	
	m_shaders.InsertToTail(m_spShapeShader);


	// Shared Shader
	//
	//m_spSharedShader = U2FrameShader::FindEffectShader(_T("shared.fx"));
	m_spSharedShader = U2_NEW U2D3DXEffectShader(this);

	m_spSharedShader->SetFilename(_T("shared.fx"));

	if(!m_spSharedShader->LoadResource())
	{
		U2ASSERT(m_spSharedShader);
		return false;
	}

	m_shaders.InsertToTail(m_spSharedShader);
		

#endif

	
	// 카메라 설정
	m_pCurrRenderTarget = m_spDefaultRenderTarget;		

	

	return true;
}


bool U2Dx9Renderer::Recreate()
{
	// 라이트 리셋 
	

	//m_pLightMgr->ReleaseLights();
	
	m_pVBMgr->DestoryBuffers();
	m_pIBMgr->DestoryBuffers();

	U2Dx9FrameBuffer::ClearRenderTarget(m_pD3DDevice9, 1);
	U2Dx9FrameBuffer::ClearRenderTarget(m_pD3DDevice9, 2);
	U2Dx9FrameBuffer::ClearRenderTarget(m_pD3DDevice9, 3);

	HWND hWnd;
	U2RenderTargets* pRTItr = m_pRenderTargets->FirstVal(&hWnd);
	
	U2Dx9FrameBuffer* pBackBuffer;
	U2Dx9FrameBuffer* pDSBuffer;	

	while(pRTItr)
	{

		pBackBuffer = pRTItr->GetBuffer(0);
		U2ASSERT(pBackBuffer);
		
		pBackBuffer->Shutdown();
		
		pDSBuffer = pRTItr->GetDepthStencilBuffer();
		if(pDSBuffer)
		{
			pDSBuffer->Shutdown();
		}

		pRTItr = m_pRenderTargets->NextVal(&hWnd);		
	}

	

	// RenderTexture Reset
	U2DynString strKey;
	U2Dx9Texture** pRenderTexItr =  m_renderedTextures.FirstVal(&strKey);
	while(pRenderTexItr)
	{
		(*pRenderTexItr)->Release();
		pRenderTexItr = m_renderedTextures.NextVal(&strKey);		
	}

	U2DynString strKey2;
	U2Dx9Texture** pCubeTexItr = m_renderedCubeTextures.FirstVal(&strKey2);
	while(pCubeTexItr)
	{
		(*pCubeTexItr)->Release();
		pCubeTexItr = m_renderedCubeTextures.NextVal(&strKey2);
	}

	U2DynString strKey3;
	U2Dx9Texture** pDynamicTexItr = m_dynamicTextures.FirstVal(&strKey3);
	while(pDynamicTexItr)
	{
		(*pDynamicTexItr)->Release();
		pDynamicTexItr = m_dynamicTextures.NextVal(&strKey3);
	}

	U2Dx9FrameBuffer::ShutdownAddedDepthStencilAll();

	U2Dx9BaseTexture* pTex = U2Dx9BaseTexture::GetHead();

	
	while(pTex)
	{
		pTex->OnLostDevice();
		pTex = pTex->GetNext();			
	}	

	U2ListNode<U2Shader*>* pItr = m_shaders.GetHeadNode();
	while(pItr)
	{
		U2Shader* pShader = m_shaders.GetNextElem(pItr);
		U2ASSERT(pShader);
		pShader->OnLostDevice();
	}

	//SAFE_RELEASE(m_pD3DEffectPool);
	
	// 응용프로그램 구체적인 Reset Funcition

	
	pBackBuffer = m_spDefaultRenderTarget->GetBuffer(0);
	

	HRESULT hr = m_pD3DDevice9->Reset((D3DPRESENT_PARAMETERS*)&
		pBackBuffer->GetPresentParams());	


	DXTRACE_ERR("Reset Failed", hr);

	if(FAILED(hr))
		return false;

	m_bDeviceLost = false;

	
	pTex = U2Dx9BaseTexture::GetHead();

	while(pTex)
	{
		pTex->OnResetDevice();
		pTex = pTex->GetNext();			
	}	

	//hr = D3DXCreateEffectPool(&m_pD3DEffectPool);


	pItr = m_shaders.GetHeadNode();
	while(pItr)
	{
		U2Shader* pShader = m_shaders.GetNextElem(pItr);
		U2ASSERT(pShader);
		pShader->OnResetDevice();
	}

	pRTItr = m_pRenderTargets->FirstVal(&hWnd);
	while(pRTItr)
	{
		pBackBuffer = pRTItr->GetBuffer(0);
		U2ASSERT(pBackBuffer);

		pBackBuffer->RecreateBackBuffer(m_pD3DDevice9);

		pDSBuffer = pRTItr->GetDepthStencilBuffer();
		if(pDSBuffer)
		{
			pDSBuffer->RecreateDepthStencilBuffer(m_pD3DDevice9);
		}

		pRTItr = m_pRenderTargets->NextVal(&hWnd);		
	}
	
	U2Dx9FrameBuffer::RecreateAddedDepthStencilAll(m_pD3DDevice9);

	// Recreate  RenderTexture Data

	if(m_pRenderState)
		m_pRenderState->InitAllStates();
	if (m_pLightMgr)
		m_pLightMgr->InitLightRenderStates();
	
	//응용프로그램 Reset 호출 
	if( U2SceneMgr::Instance() )
		U2SceneMgr::Instance()->GetFrame()->RecreateScreenQuads();

	return true;
}


U2Dx9Renderer::RecreateStatus U2Dx9Renderer::Recreate(unsigned int uiWidth, unsigned int uiHeight,
										 unsigned int uiUseFlags, HWND kWndDevice, 
										 FrameBufferFormat eFBFormat, 
										 DepthStencilFormat eDSFormat, 
										 PresentationInterval ePresentationInterval,
										 SwapEffect eSwapEffect,
										 unsigned int uiFBMode,
										 unsigned int uiBackBufferCount, 
										 unsigned int uiRefreshRate)
{
	U2Dx9FrameBuffer* pBackBuffer = m_spDefaultRenderTarget->GetBuffer(0);
	
	D3DPRESENT_PARAMETERS& primaryPP = pBackBuffer->GetPresentParams();
	D3DPRESENT_PARAMETERS tempPP;
	memcpy_s(&tempPP, sizeof(primaryPP), &primaryPP, sizeof(primaryPP));

	if(!InitializePresentParams(kWndDevice, uiWidth, uiHeight, uiUseFlags, 
		uiFBMode, eFBFormat, eDSFormat, uiBackBufferCount, eSwapEffect, 
		uiRefreshRate, ePresentationInterval, primaryPP))
	{
		memcpy_s(&primaryPP, sizeof(primaryPP), &tempPP, sizeof(tempPP));
		return RECREATESTATUS_FAILED;
	}

	if(kWndDevice)
	{
		primaryPP.hDeviceWindow = kWndDevice;
	}

	if(!Recreate())
	{
		memcpy_s(&primaryPP, sizeof(primaryPP), &tempPP, sizeof(tempPP));
		if(!Recreate())
		{
			FDebug("2Dx9Renderer::Recreate Failed to Reset Device - "
				"try to reset original present parameters");
			return RECREATESTATUS_FAILED;
		}
		else 
		{
			FDebug("2Dx9Renderer::Recreate - "
				"could not  reset originla present parameters");
			return RECREATESTATUS_RESTORED;
		}

	}

	return RECREATESTATUS_OK;
}

U2Dx9Renderer::RecreateStatus U2Dx9Renderer::Recreate(unsigned int uiWidth, unsigned int uiHeight)
{
	U2Dx9FrameBuffer* pBackBuffer = m_spDefaultRenderTarget->GetBuffer(0);

	D3DPRESENT_PARAMETERS& primaryPP = pBackBuffer->GetPresentParams();
	D3DPRESENT_PARAMETERS tempPP;
	memcpy_s(&tempPP, sizeof(primaryPP), &primaryPP, sizeof(primaryPP));


	primaryPP.BackBufferWidth = uiWidth;
	primaryPP.BackBufferHeight = uiHeight;


	if(!Recreate())
	{
		memcpy_s(&primaryPP, sizeof(primaryPP), &tempPP, sizeof(tempPP));
		if(!Recreate())
		{

			FDebug("2Dx9Renderer::Recreate Failed to Reset Device - "
				"try to reset original present parameters");			
			return RECREATESTATUS_FAILED;
		}
		else 
		{
			FDebug("2Dx9Renderer::Recreate - "
				"could not  reset originla present parameters");
			return RECREATESTATUS_RESTORED;
		}

	}

	return RECREATESTATUS_OK;

}


bool U2Dx9Renderer::IsTextureFormatOk( D3DFORMAT pixelFormat, DWORD dwUsage, D3DRESOURCETYPE eResType ) 
{

	HRESULT hr;
	D3DDISPLAYMODE dispMode;
	hr = m_pD3DDevice9->GetDisplayMode(0, &dispMode);
	U2_DXTrace(hr, _T("GetDisplayMode() failed"));	

	hr = ms_pD3D9->CheckDeviceFormat( m_uAdapter,
		m_eD3DDevType,
		dispMode.Format,
		dwUsage,
		eResType,
		pixelFormat);

	U2_DXTrace(hr, _T("Failed to U2Dx9Renderer::IsTextureFormatOk"));

	return SUCCEEDED( hr );	
}



bool U2Dx9Renderer::InitDeviceCaps(
	const D3DPRESENT_PARAMETERS& d3dPP)
{
	HRESULT hr = m_pD3DDevice9->GetDeviceCaps(&m_d3dCaps9);
	if(FAILED(hr))
	{
		FDebug("U2Dx9Renderer::InitDeviceCaps Failed");
		return false;
	}

	m_uRenderFlags = 0;

	m_usHwMaxAnisotropy = (unsigned short)m_d3dCaps9.MaxAnisotropy;
	
	// 다중매트릭스 정점 블렌딩할 때 장치가 적용할 수 있는 최대 매트릭스의 수
	m_uHWBones = m_d3dCaps9.MaxVertexBlendMatrices;

	//if(m_uHWBones >= HW_BONE_LIMIT)
	//	m_uRenderFlags |= CAPS_HARDWARESKINNING;

	m_uMaxStreams = m_d3dCaps9.MaxStreams;
	m_uMaxPixelShaderVer = m_d3dCaps9.PixelShaderVersion;
	m_uMaxVertexShaderVer = m_d3dCaps9.VertexShaderVersion;		
	m_bMipmapCubeMaps = 
		(m_d3dCaps9.TextureCaps & D3DPTEXTURECAPS_MIPCUBEMAP) != 0;
	m_bDynamicTexturesCapable =
		(m_d3dCaps9.Caps2 & D3DCAPS2_DYNAMICTEXTURES) != 0;
	m_uMaxNumRenderTargets = m_d3dCaps9.NumSimultaneousRTs;

	// Device supports different bit depths for multiple render targets.
	m_bIndependentBitDepths = (m_d3dCaps9.PrimitiveMiscCaps & 
		D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS) != 0;
	// Device supports post-pixel shader operations for multiple render targets.
	m_bMRTPostPixelShaderBlending = (m_d3dCaps9.PrimitiveMiscCaps & 
		D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING) != 0;

	ms_uMaxActiveLights = m_d3dCaps9.MaxActiveLights;
	m_uiMaxClipPlanes = m_d3dCaps9.MaxUserClipPlanes;


	ms_uiMaxTextureBlendStages = m_d3dCaps9.MaxTextureBlendStages;
	ms_uiMaxSimultaneousTextures = m_d3dCaps9.MaxSimultaneousTextures;

	if(ms_uiMaxSimultaneousTextures > 0)
	{
		m_ppActiveSamplers = U2_ALLOC(U2ShaderSamplerInfo* , 
			ms_uiMaxSimultaneousTextures);

		ZeroMemory(m_ppActiveSamplers, ms_uiMaxSimultaneousTextures * 
			sizeof(U2ShaderSamplerInfo*));
	}

	if(m_uMaxPixelShaderVer >= D3DPS_VERSION(2, 0))
		ms_uiMaxSamplers = 16;
	else 
		ms_uiMaxSamplers = m_d3dCaps9.MaxTextureBlendStages;

	// Projected
	if (m_d3dCaps9.TextureCaps & D3DPTEXTURECAPS_PROJECTED)
	{
		ms_uProjectedTextureFlags = D3DTTFF_COUNT3 | D3DTTFF_PROJECTED;
	}
	else
	{
		ms_uProjectedTextureFlags = D3DTTFF_COUNT2;
	}


	return true;
}

bool U2Dx9Renderer::SelectD3DDevicetype(DeviceDesc eDesc, D3DDEVTYPE& eDevType, uint32& uBehaviorFlags)
{
	uBehaviorFlags &= ~(D3DCREATE_HARDWARE_VERTEXPROCESSING | 
		D3DCREATE_MIXED_VERTEXPROCESSING |
		D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE);

	switch (eDesc)
	{
	case DEVDESC_PURE:
		eDevType = D3DDEVTYPE_HAL;
		uBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING |
			D3DCREATE_PUREDEVICE;
		break;
	case DEVDESC_HAL_HWVERTEX:
		eDevType = D3DDEVTYPE_HAL;
		uBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
		break;
	case DEVDESC_HAL_MIXEDVERTEX:
		eDevType = D3DDEVTYPE_HAL;
		uBehaviorFlags |= D3DCREATE_MIXED_VERTEXPROCESSING;
		break;
	case DEVDESC_HAL_SWVERTEX:
		eDevType = D3DDEVTYPE_HAL;
		uBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		break;
	case DEVDESC_REF_HWVERTEX:
		eDevType = D3DDEVTYPE_REF;
		uBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
		break;
	case DEVDESC_REF_MIXEDVERTEX:
		eDevType = D3DDEVTYPE_REF;
		uBehaviorFlags |= D3DCREATE_MIXED_VERTEXPROCESSING;
		break;
	case DEVDESC_REF:
		eDevType = D3DDEVTYPE_REF;
		uBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		break;
	default:
		return false;
	};

	return true;

}


LPDIRECT3D9 U2Dx9Renderer::GetDirect3D() 
{
	return ms_pD3D9;
}


// 'Check' functions
bool U2Dx9Renderer::InitializePresentParams(HWND hDevice, unsigned int uWidth,
											unsigned int uHeight, unsigned int uUseFlags, 
											unsigned int uFBMode, 
											FrameBufferFormat eFBFormat, DepthStencilFormat eDSFormat,
											unsigned int uBackBufferCount, SwapEffect eSwapEffect, 
											unsigned int uRefreshRate, 
											PresentationInterval ePresentationInterval,
											D3DPRESENT_PARAMETERS& d3dPresentParams) const
{
	bool bWindowed= (uUseFlags & USE_FULLSCREEN) ? false : true;

	
	D3DCAPS9 d3dCaps9;
	ms_pD3D9->GetDeviceCaps(m_uAdapter, m_eD3DDevType, 
		&d3dCaps9);

	// 해상도 설정
	UINT uBitDepth = 32;
	if(uUseFlags & USE_16BITBUFFERS)
		uBitDepth = 16;
	if(eFBFormat == FBFMT_UNKNOWN)
	{
		eFBFormat = FBFMT_A8R8G8B8;
	}

	D3DFORMAT eD3DFBFmt = GetD3DFormat(eFBFormat);
	D3DFORMAT eAdapterFmt = GetD3DAdapterFormat(eFBFormat);

	if(eDSFormat == DSFMT_UNKNOWN)
	{
		eDSFormat = DSFMT_D24S8;
	}

	ZeroMemory(&d3dPresentParams, sizeof(d3dPresentParams));

	d3dPresentParams.BackBufferWidth = uWidth;
	d3dPresentParams.BackBufferHeight = uHeight;
	d3dPresentParams.BackBufferFormat = eD3DFBFmt;
	d3dPresentParams.BackBufferCount = uBackBufferCount;	
	d3dPresentParams.MultiSampleType = GetMultiSampleType(uFBMode) ;
	d3dPresentParams.MultiSampleQuality = GetMultiSampleQuality(uFBMode);
	d3dPresentParams.AutoDepthStencilFormat = GetD3DFormat(eDSFormat);
	d3dPresentParams.EnableAutoDepthStencil = 
		(uUseFlags & USE_MANUALDEPTHSTENCIL) ? false : true;
	d3dPresentParams.Windowed = bWindowed;
	d3dPresentParams.SwapEffect = GetSwapEffect(eSwapEffect, uFBMode, uUseFlags);
	
	// Back Buffer Lock 가능
	d3dPresentParams.Flags = (uFBMode == FBMODE_LOCKABLE) ? D3DPRESENTFLAG_LOCKABLE_BACKBUFFER : 0;

	d3dPresentParams.hDeviceWindow = hDevice;
	d3dPresentParams.FullScreen_RefreshRateInHz = GetRefreshRate(uRefreshRate);
	d3dPresentParams.PresentationInterval = GetPresentInterval(ePresentationInterval);


	DWORD uiQualityLevels;
	HRESULT hRet = ms_pD3D9->CheckDeviceMultiSampleType(m_uAdapter,
		m_eD3DDevType, d3dPresentParams.BackBufferFormat,
		d3dPresentParams.Windowed, d3dPresentParams.MultiSampleType, 
		&uiQualityLevels);

	if(FAILED(hRet))
	{
		FDebug("U2Dx9Renderer::Initialize - Failed to set"
			"antialiasing mode - Restore\n");
		d3dPresentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
	}
	else if(uiQualityLevels <= d3dPresentParams.MultiSampleQuality)	// 제한.
		d3dPresentParams.MultiSampleQuality = uiQualityLevels - 1;

	if(d3dPresentParams.BackBufferCount == 0)
		d3dPresentParams.BackBufferCount = 1;
	else if(d3dPresentParams.BackBufferCount > 3)
		d3dPresentParams.BackBufferCount = 3;

	if (d3dPresentParams.Windowed)
	{
		d3dPresentParams.FullScreen_RefreshRateInHz = 0;
	}
	
	if(d3dPresentParams.Windowed)
	{
		if (d3dPresentParams.PresentationInterval != 
			D3DPRESENT_INTERVAL_DEFAULT &&
			d3dPresentParams.PresentationInterval != 
			D3DPRESENT_INTERVAL_IMMEDIATE &&
			d3dPresentParams.PresentationInterval != 
			D3DPRESENT_INTERVAL_ONE)
		{
			d3dPresentParams.PresentationInterval = 
				D3DPRESENT_INTERVAL_IMMEDIATE;
		}
	}
	else if (!(d3dPresentParams.PresentationInterval &
		d3dCaps9.PresentationIntervals))
	{
		d3dPresentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	}
	

	return true;
}


unsigned int U2Dx9Renderer::GetRefreshRate(
	unsigned int uiRefreshRateHz)
{
	switch (uiRefreshRateHz)
	{
	case REFRESHRATE_DEFAULT:
		return D3DPRESENT_RATE_DEFAULT;
	}

	return uiRefreshRateHz;
}


unsigned int U2Dx9Renderer::GetPresentInterval(
	U2Dx9Renderer::PresentationInterval ePresentationInterval)
{
	switch (ePresentationInterval)
	{
	case PRESENT_INTERVAL_IMMEDIATE:
		return D3DPRESENT_INTERVAL_IMMEDIATE;
	case PRESENT_INTERVAL_ONE:
		return D3DPRESENT_INTERVAL_ONE;
	case PRESENT_INTERVAL_TWO:
		return D3DPRESENT_INTERVAL_TWO;
	case PRESENT_INTERVAL_THREE:
		return D3DPRESENT_INTERVAL_THREE;
	case PRESENT_INTERVAL_FOUR:
		return D3DPRESENT_INTERVAL_FOUR;
	}

	return D3DPRESENT_INTERVAL_DEFAULT;
}


D3DSWAPEFFECT U2Dx9Renderer::GetSwapEffect(
	SwapEffect eMode, unsigned int uiFBMode, unsigned int)
{
	// Must use discard with a multisampled framebuffer
	if ((uiFBMode != FBMODE_DEFAULT) && (uiFBMode != FBMODE_LOCKABLE))
		return D3DSWAPEFFECT_DISCARD;

	switch (eMode)
	{
	case SWAPEFFECT_DISCARD:
		return D3DSWAPEFFECT_DISCARD;
	case SWAPEFFECT_FLIP:
		return D3DSWAPEFFECT_FLIP;
	case SWAPEFFECT_COPY:
		return D3DSWAPEFFECT_COPY;
	default:
		return D3DSWAPEFFECT_DISCARD;
	}
}


D3DFORMAT U2Dx9Renderer::GetD3DAdapterFormat(
	U2Dx9Renderer::FrameBufferFormat eFBFormat)
{
	switch (eFBFormat)
	{
	case FBFMT_UNKNOWN:
	case FBFMT_A8:
	case FBFMT_A8R3G3B2:
		return D3DFMT_UNKNOWN;
	case FBFMT_R8G8B8:
		return D3DFMT_R8G8B8;
	case FBFMT_A8R8G8B8:
	case FBFMT_X8R8G8B8:
		return D3DFMT_X8R8G8B8;
	case FBFMT_R5G6B5:
		return D3DFMT_R5G6B5;
	case FBFMT_X1R5G5B5:
	case FBFMT_A1R5G5B5:
		return D3DFMT_X1R5G5B5;
	case FBFMT_X4R4G4B4:
	case FBFMT_A4R4G4B4:
		return D3DFMT_X4R4G4B4;
	case FBFMT_R3G3B2:
		return D3DFMT_R3G3B2;
	}

	return D3DFMT_UNKNOWN;
}


D3DFORMAT U2Dx9Renderer::GetD3DFormat(
	 U2Dx9Renderer::FrameBufferFormat eFBFormat)
{
	switch (eFBFormat)
	{
	case FBFMT_UNKNOWN:
		return D3DFMT_UNKNOWN;
	case FBFMT_R8G8B8:
		return D3DFMT_R8G8B8;
	case FBFMT_A8R8G8B8:
		return D3DFMT_A8R8G8B8;
	case FBFMT_X8R8G8B8:
		return D3DFMT_X8R8G8B8;
	case FBFMT_R5G6B5:
		return D3DFMT_R5G6B5;
	case FBFMT_X1R5G5B5:
		return D3DFMT_X1R5G5B5;
	case FBFMT_A1R5G5B5:
		return D3DFMT_A1R5G5B5;
	case FBFMT_A4R4G4B4:
		return D3DFMT_A4R4G4B4;
	case FBFMT_R3G3B2:
		return D3DFMT_R3G3B2;
	case FBFMT_A8:
		return D3DFMT_A8;
	case FBFMT_A8R3G3B2:
		return D3DFMT_A8R3G3B2;
	case FBFMT_X4R4G4B4:
		return D3DFMT_X4R4G4B4;
	case FBFMT_R16F:
		return D3DFMT_R16F;
	case FBFMT_G16R16F:
		return D3DFMT_G16R16F;
	case FBFMT_A16B16G16R16F:
		return D3DFMT_A16B16G16R16F;
	case FBFMT_R32F:
		return D3DFMT_R32F;
	case FBFMT_G32R32F:
		return D3DFMT_G32R32F;
	case FBFMT_A32B32G32R32F:
		return D3DFMT_A32B32G32R32F;
	}

	return D3DFMT_UNKNOWN;
}



D3DMULTISAMPLE_TYPE U2Dx9Renderer::GetMultiSampleType(
	unsigned int uiMode)
{
	if (uiMode == FBMODE_DEFAULT || uiMode == FBMODE_LOCKABLE)
		return D3DMULTISAMPLE_NONE;

	uiMode &= ~FBMODE_QUALITY_MASK;

	switch (uiMode)
	{
	case FBMODE_MULTISAMPLES_2:
		return D3DMULTISAMPLE_2_SAMPLES;
	case FBMODE_MULTISAMPLES_3:
		return D3DMULTISAMPLE_3_SAMPLES;
	case FBMODE_MULTISAMPLES_4:
		return D3DMULTISAMPLE_4_SAMPLES;
	case FBMODE_MULTISAMPLES_5:
		return D3DMULTISAMPLE_5_SAMPLES;
	case FBMODE_MULTISAMPLES_6:
		return D3DMULTISAMPLE_6_SAMPLES;
	case FBMODE_MULTISAMPLES_7:
		return D3DMULTISAMPLE_7_SAMPLES;
	case FBMODE_MULTISAMPLES_8:
		return D3DMULTISAMPLE_8_SAMPLES;
	case FBMODE_MULTISAMPLES_9:
		return D3DMULTISAMPLE_9_SAMPLES;
	case FBMODE_MULTISAMPLES_10:
		return D3DMULTISAMPLE_10_SAMPLES;
	case FBMODE_MULTISAMPLES_11:
		return D3DMULTISAMPLE_11_SAMPLES;
	case FBMODE_MULTISAMPLES_12:
		return D3DMULTISAMPLE_12_SAMPLES;
	case FBMODE_MULTISAMPLES_13:
		return D3DMULTISAMPLE_13_SAMPLES;
	case FBMODE_MULTISAMPLES_14:
		return D3DMULTISAMPLE_14_SAMPLES;
	case FBMODE_MULTISAMPLES_15:
		return D3DMULTISAMPLE_15_SAMPLES;
	case FBMODE_MULTISAMPLES_16:
		return D3DMULTISAMPLE_16_SAMPLES;
	case FBMODE_MULTISAMPLES_NONMASKABLE:
		return D3DMULTISAMPLE_NONMASKABLE;
	default:
		U2ASSERT(false);
	}

	return D3DMULTISAMPLE_NONE;
}


unsigned int U2Dx9Renderer::GetMultiSampleQuality(unsigned int uiMode)
{
	if (uiMode == FBMODE_DEFAULT || uiMode == FBMODE_LOCKABLE)
		return 0;

	return (uiMode & FBMODE_QUALITY_MASK);
}


unsigned int U2Dx9Renderer::GetMaxRenderTargets() const
{
	return m_uMaxNumRenderTargets;
}


bool U2Dx9Renderer::GetIndependentBufferBitDepths() const
{
	return m_bIndependentBitDepths;
}


bool U2Dx9Renderer::RestoreLostDevice()
{
	HRESULT hr = m_pD3DDevice9->TestCooperativeLevel();

	switch(hr)
	{
	case D3DERR_DEVICELOST:
		if(m_bDeviceLost == false)
		{
			m_bDeviceLost = true;
		}
		return false;
	case D3DERR_DEVICENOTRESET:
		bool bSuccess = Recreate();
		return bSuccess;
	}
	
	U2ASSERT(SUCCEEDED(hr));
	return true;			
}


bool U2Dx9Renderer::IsDSBufferCompatible(U2Dx9FrameBuffer* pBackBuffer,
										 U2Dx9FrameBuffer* pDSBuffer)
{	
	if(!pBackBuffer || !pDSBuffer)
		return false;

	// Render target must be smaller than Depth Stencil
	if(pBackBuffer->GetWidth() > pDSBuffer->GetWidth() ||
		pBackBuffer->GetHeight() > pDSBuffer->GetHeight())
		return false;

	D3DSURFACE_DESC bufferDesc;
	D3DSURFACE_DESC dsDesc;
	HRESULT hr;	

	hr = pBackBuffer->GetSurface()->GetDesc(&bufferDesc);
	U2ASSERT(SUCCEEDED(hr));
	hr = pDSBuffer->GetSurface()->GetDesc(&dsDesc);
	U2ASSERT(SUCCEEDED(hr));

	if((bufferDesc.MultiSampleQuality != dsDesc.MultiSampleQuality ) ||
		(bufferDesc.MultiSampleType != dsDesc.MultiSampleType))
		return false;

	hr = ms_pD3D9->CheckDepthStencilMatch(GetAdapter(), GetDevType(), 
		m_eD3DAdapterFormat, bufferDesc.Format, dsDesc.Format);
	if(FAILED(hr))
		return false;

	return true;
}


bool U2Dx9Renderer::BeginScene()
{
	//if(!CheckFrameState("BeginScene", FS_OUTSIDE_FRAME))
	//	return false;

	if(RestoreLostDevice())
	{
		// Reset 		

		if(FAILED(m_pD3DDevice9->BeginScene()))
		{
			// Error			
			return false;
		}		

		// Reset 
		while(m_frameBuffersToPresent.Size())
			m_frameBuffersToPresent.RemoveHead();

		m_eFrameState = FS_INSIDE_FRAME;
	}
	
	return true;
}


bool U2Dx9Renderer::EndScene()
{
	//if(!CheckFrameState("EndScene", FS_INSIDE_FRAME))
	//	return false;
	
	if(!m_bDeviceLost)
	{		
		if(FAILED(m_pD3DDevice9->EndScene()))
		{
			// Error
			m_eFrameState = FS_OUTSIDE_FRAME;	
#ifdef _DEBUG
			DPrintf("NiRenderer::EndFrame> Failed - " 
				"Returning to frame to 'not set' state.\n");
#endif
			return false;
		}	

		m_eFrameState = FS_WAITING_FOR_DISPLAY;
	}
	return true;
}


void U2Dx9Renderer::ClearBuffers(unsigned int uClearMode)
{
	if(uClearMode == CLEAR_NONE)
		return;

	if(!m_bDeviceLost)
	{
		D3DRECT clearRect;
		DWORD dwFlags = 0;

		unsigned short bufferWidth = m_pCurrRenderTarget->GetWidth(0);
		unsigned short bufferHeight = m_pCurrRenderTarget->GetHeight(0);

		clearRect.x1 = 0;
		clearRect.y1 = 0;
		clearRect.x2 = (long)bufferWidth;
		clearRect.y2 = (long)bufferHeight;
	
		dwFlags |= ((uClearMode & CLEAR_BACKBUFFER) ? D3DCLEAR_TARGET : 0);

		U2Dx9FrameBuffer *pDSBuffer = m_pCurrRenderTarget->GetDepthStencilBuffer();

		if(pDSBuffer)
		{
			dwFlags |= (((uClearMode && CLEAR_ZBUFFER) && 
				(pDSBuffer->GetDepthType() == U2Dx9FrameBuffer::DT_DEPTH)) ? 
				D3DCLEAR_ZBUFFER : 0);
			dwFlags |= (((uClearMode && CLEAR_STENCIL) && 
				(pDSBuffer->GetStencilType() == U2Dx9FrameBuffer::ST_STENCIL))?
				D3DCLEAR_STENCIL : 0);
		}

		m_d3dPort.Width = m_pCurrRenderTarget->GetWidth(0);
		m_d3dPort.Height = m_pCurrRenderTarget->GetHeight(0);
		m_pD3DDevice9->SetViewport(&m_d3dPort);

		HRESULT hr = m_pD3DDevice9->Clear(1, &clearRect, dwFlags, 
			m_uBgColor, m_fZClear, m_uStencilClear);
		U2ASSERT(!FAILED(hr));
	}
}


bool U2Dx9Renderer::DisplayBackBuffer()
{
	if(!CheckFrameState("DisplayBackBuffer", FS_WAITING_FOR_DISPLAY))
		return false;

	if(!m_bDeviceLost)
	{
		while(m_frameBuffersToPresent.Size())
		{
			U2Dx9FrameBuffer* pBuffer = m_frameBuffersToPresent.RemoveHead();
			
			
			HRESULT hr = m_pD3DDevice9->Present(NULL, NULL, NULL, NULL);
			if((hr))
			{
				m_eFrameState = FS_OUTSIDE_FRAME;	
				return true;
			}
		}
	}	
	
	m_eFrameState = FS_OUTSIDE_FRAME;
	return false;
}


uint32  U2Dx9Renderer::GetMaxTextureBlendStages()
{
	return ms_uiMaxTextureBlendStages;
}


uint32  U2Dx9Renderer::GetMaxSimultaneousTextures()
{	
	return ms_uiMaxSimultaneousTextures;
}


uint32 U2Dx9Renderer::GetMaxSamplers()
{
	return ms_uiMaxSamplers;
}

const TCHAR* U2Dx9Renderer::GetDriverInfo() const
{
	D3DADAPTER_IDENTIFIER9 kD3DAdapterIdentifier9;

	m_acDriverDesc[0] = _T('\0');
	memset((void*)&kD3DAdapterIdentifier9, 0, sizeof(kD3DAdapterIdentifier9));

	if (SUCCEEDED(ms_pD3D9->GetAdapterIdentifier(m_uAdapter,
		0, &kD3DAdapterIdentifier9)))
	{
		_stprintf_s(m_acDriverDesc, 512, _T("%s (%s-%s)"), 
			kD3DAdapterIdentifier9.Description, GetDevTypeString(), 
			GetBehaviorString());
	}

	return m_acDriverDesc;
}


bool U2Dx9Renderer::BeginDefaultRenderTarget(unsigned int uClearMode )
{
	return BeginRenderTarget(m_spDefaultRenderTarget, uClearMode);
}



bool U2Dx9Renderer::BeginRenderTarget(U2RenderTargets* pTarget, unsigned int uClearMode )
{
	if(!m_bDeviceLost)
	{
		U2Dx9FrameBuffer::ClearRenderTarget(m_pD3DDevice9, 1);
		U2Dx9FrameBuffer::ClearRenderTarget(m_pD3DDevice9, 2);
		U2Dx9FrameBuffer::ClearRenderTarget(m_pD3DDevice9, 3);

		U2Dx9FrameBuffer *pBackBuffer = NULL;
		for(uint32 i = 0; i < pTarget->GetBackBufferCnt(); ++i)
		{
			pBackBuffer = pTarget->GetBuffer(i);

			if(pBackBuffer)
			{
				bool bSet = pBackBuffer->SetRenderTarget(m_pD3DDevice9, i);
				if(!bSet)
				{
					bool bReset = 
						BeginRenderTarget(m_spDefaultRenderTarget, uClearMode);

					FDebug("U2Dx9Renderer::BeginRenderTarget Failed - "
						"Reset to Default Render Target");
					return false;
				}
			}			
		}

		U2Dx9FrameBuffer* pDSBuffer = pTarget->GetDepthStencilBuffer();
		if(pDSBuffer)
		{
			bool bSet = pDSBuffer->SetDepthStencilTarget(m_pD3DDevice9);
			if(!bSet)
			{
				bool bReset = 
					BeginRenderTarget(m_spDefaultRenderTarget, uClearMode);

				FDebug("U2Dx9Renderer::BeginRenderTarget Failed - "
					"Reset to Default Render Target");
				return false;
			}
		}

		m_pCurrRenderTarget = pTarget;

		ClearBuffers(uClearMode);
	}

	return true;

}


bool U2Dx9Renderer::EndRenderTarget()
{
	if(!m_bDeviceLost)
	{
		U2ASSERT(m_pCurrRenderTarget);

		U2Dx9FrameBuffer *pBackBuffer = NULL;
		for(uint32 i=0; i < m_pCurrRenderTarget->GetBackBufferCnt(); ++i)
		{
			pBackBuffer = m_pCurrRenderTarget->GetBuffer(i);

			if(pBackBuffer->CanDisplayFrame())
			{
				if(!m_frameBuffersToPresent.Find(pBackBuffer))
				{
					m_frameBuffersToPresent.InsertToTail(pBackBuffer);
				}
			}
		}
		m_pCurrRenderTarget = NULL;				

	}
	return true;
}





void U2Dx9Renderer::SetTransform(TransformType eType, const D3DXMATRIX& mat)
{
	GenerateTansform(eType, mat);
	
	U2D3DXEffectShader *pShd = SmartPtrCast(U2D3DXEffectShader, m_spSharedShader);
	U2ASSERT(pShd);

	bool bSetWVP = false;
	bool bSetCamDir = false;
	bool bSetCamPos = false;
	bool bSetWorldCamPos = false;
	
	m_bWvpMatOnly = GetHint(MODEL_VIEW_PROJ_ONLY);

	switch(eType)
	{
		
	case TMT_WORLD:

		if(!m_bWvpMatOnly)
		{
			pShd->SetMatrix(U2FxShaderState::Model, m_aTM[TMT_WORLD]);
			pShd->SetMatrix(U2FxShaderState::InvModel, m_aTM[TMT_WORLD]);
			pShd->SetMatrix(U2FxShaderState::ModelView, m_aTM[TMT_WORLDVIEW]);
			pShd->SetMatrix(U2FxShaderState::InvModelView, m_aTM[TMT_INVWORLDVIEW]);
		}
		bSetWorldCamPos = true;
		bSetWVP = true;
		break;	

	case TMT_VIEW:
		{
			if(!m_bWvpMatOnly)
			{
				pShd->SetMatrix(U2FxShaderState::View, m_aTM[TMT_VIEW]);
				pShd->SetMatrix(U2FxShaderState::InvView, m_aTM[TMT_INVVIEW]);
				pShd->SetMatrix(U2FxShaderState::ModelView, m_aTM[TMT_WORLDVIEW]);
				pShd->SetMatrix(U2FxShaderState::InvModelView, m_aTM[TMT_INVWORLDVIEW]);
				bSetCamPos = true;

			}
			bSetWorldCamPos = true;
			bSetCamDir = true;
			bSetWVP = true;
			break;
		}
	case TMT_PROJ:
		if(!m_bWvpMatOnly)
		{
			pShd->SetMatrix(U2FxShaderState::Projection, m_aTM[TMT_PROJ]);
		}
		bSetWVP = true;
		break;
	case TMT_TEXTURE0:
		if(!m_bWvpMatOnly)
		{
			pShd->SetMatrix(U2FxShaderState::TextureTransform0, m_aTM[TMT_TEXTURE0]);
		}
		break;
	case TMT_TEXTURE1:
		if(!m_bWvpMatOnly)
		{
			pShd->SetMatrix(U2FxShaderState::TextureTransform1, m_aTM[TMT_TEXTURE1]);
		}
		break;
	case TMT_TEXTURE2:
		if(!m_bWvpMatOnly)
		{
			pShd->SetMatrix(U2FxShaderState::TextureTransform2, m_aTM[TMT_TEXTURE2]);
		}
		break;
	case TMT_TEXTURE3:
		if(!m_bWvpMatOnly)
		{
			pShd->SetMatrix(U2FxShaderState::TextureTransform3, m_aTM[TMT_TEXTURE3]);
		}
		break;
	case TMT_LIGHT:
		break;
	}

	if(bSetWVP)
		pShd->SetMatrix(U2FxShaderState::ModelViewProjection, m_aTM[TMT_WORLDVIEWPROJ]);
	if(!m_bWvpMatOnly && bSetCamPos)
	{
		pShd->SetVector3(U2FxShaderState::EyePos, 
			D3DXVECTOR3(m_aTM[TMT_INVVIEW]._41, m_aTM[TMT_INVVIEW]._42, m_aTM[TMT_INVVIEW]._43));
	}

	// world eye pos always needed in lighting formula
	if(bSetWorldCamPos)
	{
		pShd->SetVector3(U2FxShaderState::ModelEyePos, 
			D3DXVECTOR3(m_aTM[TMT_INVWORLDVIEW]._41,
			m_aTM[TMT_INVWORLDVIEW]._42, m_aTM[TMT_INVWORLDVIEW]._43));
	}

	if(bSetCamDir)
	{
		pShd->SetVector3(U2FxShaderState::EyeDir, 
			-D3DXVECTOR3(m_aTM[TMT_VIEW]._31, m_aTM[TMT_VIEW]._32, m_aTM[TMT_VIEW]._33));
	}


}


void U2Dx9Renderer::GenerateTansform(TransformType eType, const D3DXMATRIX& mat)
{
	bool bUpdateWorldView = false;
	bool bUpdateWorldLight = false;
	bool bUpdateViewProj = false;
	switch(eType)
	{
	case TMT_WORLD:
		m_aTM[TMT_WORLD] = mat;		
		D3DXMatrixInverse(&m_aTM[TMT_INVWORLD], NULL, &mat);
		bUpdateWorldView = true;
		bUpdateWorldLight = true;
		break;
	case TMT_VIEW:
		m_aTM[TMT_VIEW] = mat;
		D3DXMatrixInverse(&m_aTM[TMT_INVVIEW], NULL, &mat);
		bUpdateWorldView = true;

		bUpdateViewProj = true;
		break;
	case TMT_PROJ:
		m_aTM[TMT_PROJ] = mat;
		bUpdateViewProj = true;
		break;

	case TMT_SHADOWPROJ:
		m_aTM[TMT_SHADOWPROJ] = mat;		
		break;

	case TMT_TEXTURE0:
	case TMT_TEXTURE1:
	case TMT_TEXTURE2:
	case TMT_TEXTURE3:
		m_aTM[eType] = mat;
		break;

	case TMT_LIGHT:
		m_aTM[eType] = mat;
		bUpdateWorldLight = true;
		break;

	default:
		U2ASSERT(_T("U2D3D9FxEffect::GenerateTransform() Trying to set read-only transform type!"));
		break;
	}

	if(bUpdateWorldView)
	{
		m_aTM[TMT_WORLDVIEW] = m_aTM[TMT_WORLD] * m_aTM[TMT_VIEW];		
		m_aTM[TMT_INVWORLDVIEW] = m_aTM[TMT_INVVIEW] * m_aTM[TMT_INVWORLD];
	}


	if(bUpdateWorldLight)
	{
		m_aTM[TMT_WORLDLight] = m_aTM[TMT_WORLD] * m_aTM[TMT_LIGHT];		
		m_aTM[TMT_INVWORLDLight] = m_aTM[TMT_LIGHT] * m_aTM[TMT_INVWORLD];
		
	}


	if(bUpdateViewProj)
	{
		m_aTM[TMT_VIEWPROJ] = m_aTM[TMT_VIEW] * m_aTM[TMT_PROJ];
		
	}

	if(bUpdateWorldView || bUpdateViewProj)
	{
		m_aTM[TMT_WORLDVIEWPROJ] = m_aTM[TMT_WORLDVIEW] * m_aTM[TMT_PROJ];		
	}
}


//------------------------------------------------------------------------------
/**
Push current transformation on stack and set new matrix.

@param  type        transform type
@param  matrix      the 4x4 matrix
*/
void U2Dx9Renderer::PushTransform(TransformType eType, const D3DXMATRIX& mat)
{
	U2ASSERT(eType < MAX_TRANSFORM_TYPES);
	U2ASSERT(m_aiTMTopOfStack[eType] < MAX_TRANSFORM_STACK_DEPTH);
	m_aaTMStack[eType][m_aiTMTopOfStack[eType]++] = m_aTM[eType];
	SetTransform(eType, mat);
}

//------------------------------------------------------------------------------
/**
Pop transformation from stack and make it the current transform.
*/
const D3DXMATRIX& U2Dx9Renderer::PopTransform(TransformType eType)
{
	U2ASSERT(eType < MAX_TRANSFORM_TYPES);
	SetTransform(eType, m_aaTMStack[eType][--m_aiTMTopOfStack[eType]]);
	return m_aTM[eType];
}



/**************************************************************************************************
Func name	:	RenderTriStrip
params		:	
params Desc :
Func Desc	:	 
/*************************************************************************************************/
void U2Dx9Renderer::RenderTriStrip(U2TriStrip* pStrip)
{
	//U2TriStripData* pData = (U2TriStripData*)pStrip->GetMeshData();

	//m_pCurrMesh = pStrip;
	//m_pCurrRSC = pStrip->m_spRSC;

	//U2ASSERT(pData);

	//uint32 uActiveVertCnt = pData->GetActiveVertexCount();
	//if(m_bDeviceLost || uActiveVertCnt == 0)
	//	return;

	//U2VBGroup* pVBGroup = m_pDefaultGroup;
	//if(pData->GetMeshState() == U2MeshData::DYNAMIC_MESH)
	//	pVBGroup = m_pDynamicGroup;

	//m_pVBGroupMgr->AddMeshToGroup(pVBGroup, pData, false);

	//// Lighting is applied first. Effects are applied second
	//bool bPrimaryEffect = true;

	//const uint32 uStart = pStrip->GetStartEffect();
	//const uint32 uEnd = pStrip->GetEffectCnt();

	//for(uint32 i = uStart; i < uEnd; ++i)
	//{
	//	U2ShaderEffect* pEffect = 
	//		DynamicCast<U2ShaderEffect>(pStrip->GetEffect(i));
	//	if(pEffect)
	//	{
	//		DrawPrimitves(pStrip, pData, pEffect, bPrimaryEffect);
	//	}
	//}

	//m_pCurrMesh = NULL;	
}



/**************************************************************************************************
Func name	:	RenderTriList
params		:	
params Desc :
Func Desc	:	 
/*************************************************************************************************/
void U2Dx9Renderer::RenderTriList(U2TriList* pTriListMesh)
{
	//U2TriListData* pData = (U2TriListData*)pTriListMesh->GetMeshData();

	//m_pCurrMesh = pTriListMesh;
	//m_pCurrRSC = pTriListMesh->m_spRSC;

	//U2ASSERT(pData);

	//uint32 uActiveVertCnt = pData->GetActiveVertexCount();
	//if(m_bDeviceLost || uActiveVertCnt == 0)
	//	return;


	//U2VBGroup* pVBGroup = m_pDefaultGroup;
	//if(pData->GetMeshState() == U2MeshData::DYNAMIC_MESH)
	//	pVBGroup = m_pDynamicGroup;

	//m_pVBGroupMgr->AddMeshToGroup(pVBGroup, pData, false);

	//// Lighting is applied first. Effects are applied second
	//bool bPrimaryEffect = true;

 //	const uint32 uStart = pTriListMesh->GetStartEffect();
	//const uint32 uEnd = pTriListMesh->GetEffectCnt();

	//for(uint32 i = uStart; i < uEnd; ++i)
	//{
	//	U2ShaderEffect* pEffect = 
	//		DynamicCast<U2ShaderEffect>(pTriListMesh->GetEffect(i));
	//	if(pEffect)
	//	{
	//		DrawPrimitves(pTriListMesh, pData, pEffect, bPrimaryEffect);
	//	}
	//}
	//
	//m_pCurrMesh = NULL;	
}


void U2Dx9Renderer::RenderLines(U2PolyLine* pLines)
{
	if(m_bDeviceLost)
		return;

	m_pCurrMesh = pLines;
	m_pCurrRSC = pLines->m_spRSC;


	m_worldMat = pLines->m_matHWorld;	


	// Lighting is applied first. Effects are applied second
	bool bPrimaryEffect = true;

	const uint32 uStart = pLines->GetStartEffect();
	const uint32 uEnd = pLines->GetEffectCnt();

	for(uint32 i = uStart; i < uEnd; ++i)
	{
		U2ShaderEffect* pEffect = 
			DynamicCast<U2ShaderEffect>(pLines->GetEffect(i));
		if(pEffect)
		{
			DrawLines(pLines, pEffect, bPrimaryEffect);
		}
	}

	m_pCurrMesh = NULL;	

}


/**************************************************************************************************
Func name	:	DrawLines
params		:	
params Desc :
Func Desc	:	 
/*************************************************************************************************/
void U2Dx9Renderer::DrawLines(U2PolyLine* pLine, U2ShaderEffect* pEffect, bool& bPrimaryEffect)
{
	using namespace U2Math;

	U2PolyLineData* pData = (U2PolyLineData*)pLine->GetMeshData();
	U2ASSERT(pData);

	unsigned short usPoints = pData->GetVertexCount();
	if(usPoints == 0)
		return;

//	m_pVBGroupMgr->AddMeshToGroup(m_pDynamicGroup, pData, false);

	unsigned char* pConnFlags = pData->GetConnFlags();
	D3DXVECTOR3* pPoint = pData->GetVertices();
	D3DXVECTOR2* pTCoord = pData->GetTexCoords();
	D3DXCOLOR* pColor = pData->GetColors();

	unsigned int uConn = 0;


	LPDIRECT3DINDEXBUFFER9 pSrcIB =
		m_pIBMgr->ResizeTempLineConnect(usPoints, pConnFlags, uConn);
	if(pSrcIB == 0 || uConn == 0)
		return;

	unsigned int uiFVF = D3DFVF_XYZ;
	unsigned int uiVertexStride = sizeof(float) * 3;

	if(pTCoord)
	{
		uiFVF |= D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEX1;
		uiVertexStride += sizeof(float) * 2;
	}
	if(pColor)
	{
		uiFVF |= D3DFVF_DIFFUSE;
		uiVertexStride += sizeof(unsigned int);
	}

	unsigned int uiSize = usPoints * uiVertexStride;

	pData->SetStreamCount(1);
	pData->SetVertexCount(usPoints, usPoints);
	pData->SetFVF(uiFVF);
	pData->SetVertexStride(0, uiVertexStride);
	//m_pVBMgr->AllocateBuffer(pData, 0);
	//U2VBBlock* pBlock = pData->GetVBBlock(0);

	//if(!pBlock->GetVB())
	//{
	//	return;
	//}

	//if(pTCoord && pColor)
	//{
	//	BaseVertex* pVerts = (BaseVertex*)m_pVBMgr->LockVB(pBlock->GetVB(), pBlock->GetCurrOffset(), 
	//		pBlock->GetSize(), pBlock->GetFlags());

	//	if(pVerts == NULL)
	//		return;

	//	for(uint32 i=0; i < usPoints; ++i)
	//	{
	//		pVerts->x = pPoint->x;
	//		pVerts->y = pPoint->y;
	//		pVerts->z = pPoint->z;
	//		pVerts->color = 
	//			(FastFloatToInt(pColor->a * 255.0f) << 24) |
	//			(FastFloatToInt(pColor->r * 255.0f) << 16) |
	//			(FastFloatToInt(pColor->g * 255.0f) <<  8) |
	//			FastFloatToInt(pColor->b * 255.0f);
	//		pVerts->tu = pTCoord->x;
	//		pVerts->tv = pTCoord->y;

	//		pVerts++;
	//		pPoint++;
	//		pColor++;
	//		pTCoord++;
	//	}
	//}
	//else if(pTCoord)
	//{
	//	TexturedLineVertex* pVert = (TexturedLineVertex*)m_pVBMgr->LockVB(pBlock->GetVB(), 
	//		pBlock->GetCurrOffset(), pBlock->GetSize(), pBlock->GetFlags());

	//	if(pVert == NULL)
	//		return;

	//	for(uint32 i=0; i < usPoints; ++i)
	//	{
	//		pVert->x = pPoint->x;
	//		pVert->y = pPoint->y;
	//		pVert->z = pPoint->z;
	//		pVert->tu = pTCoord->x;
	//		pVert->tv = pTCoord->y;

	//		pVert++;
	//		pPoint++;
	//		pTCoord++;
	//	}
	//}
	//else if(pColor)
	//{
	//	ColoredLineVertex* pVerts = (ColoredLineVertex*)m_pVBMgr->LockVB(pBlock->GetVB(), 
	//		pBlock->GetCurrOffset(), pBlock->GetSize(), pBlock->GetFlags());


	//	if(pVerts == NULL)
	//		return;

	//	for(uint32 i=0; i < usPoints; ++i)
	//	{
	//		pVerts->x = pPoint->x;
	//		pVerts->y = pPoint->y;
	//		pVerts->z = pPoint->z;
	//		pVerts->color = 
	//			(FastFloatToInt(pColor->a * 255.0f) << 24) |
	//			(FastFloatToInt(pColor->r * 255.0f) << 16) |
	//			(FastFloatToInt(pColor->g * 255.0f) <<  8) |
	//			FastFloatToInt(pColor->b * 255.0f);			

	//		pVerts++;
	//		pPoint++;
	//		pColor++;			
	//	}

	//}
	//else 
	//{

	//	LineVertex* pVerts = (LineVertex*)m_pVBMgr->LockVB(pBlock->GetVB(), 
	//		pBlock->GetCurrOffset(), pBlock->GetSize(), pBlock->GetFlags());


	//	if(pVerts == NULL)
	//		return;

	//	for(uint32 i=0; i < usPoints; ++i)
	//	{
	//		pVerts->x = pPoint->x;
	//		pVerts->y = pPoint->y;
	//		pVerts->z = pPoint->z;			

	//		pVerts++;
	//		pPoint++;			
	//	}

	//}

	//m_pVBMgr->UnlockVB(pBlock->GetVB());	

	//

	//if (m_pCurrRSC)
	//{
	//	U2ASSERT(m_pCurrRSC);
	//	m_pRenderState->UpdateRenderState(m_pCurrRSC);

	//	const uint32 uPassCnt = pEffect->GetPassCnt();
	////	HRESULT hr;
	//	for(uint32 pass = 0; pass < uPassCnt; ++pass)
	//	{

	//		pEffect->SetRenderState(pass, m_pRenderState, bPrimaryEffect);

	//		m_uNumActiveSamplers = 0;

	//		pEffect->ConnVSConstants(pass);
	//		pEffect->ConnPSConstants(pass);

	//		U2VertexProgram* pVProgram = pEffect->GetVProgram(pass);
	//		U2PixelProgram* pPProgram = pEffect->GetPProgram(pass);	

	//		const U2VertexAttributes& inputAttr = pVProgram->GetInputVertAtts();
	//		const U2VertexAttributes& outputAttr = pVProgram->GetOutputVertAtts();


	//		// Asm Shader Load
	//		// EnableVetexProgram, EnablePixelProgram을 먼저 로드해야 
	//		// EnableTexture에 해당 데이터를 얻을 수 있으므로 
	//		// 순서 주의

	//		if(pass == 0)
	//		{	

	//			EnableVertexProgram(pVProgram);
	//			EnablePixelProgram(pPProgram);				

	//			// 순서 주의 .. 
	//			// PackMeshData함수 안에서 VertexDeclation이 생성됨.
	//			LPDIRECT3DVERTEXDECLARATION9 pVertDecl =
	//				m_pVBMgr->GenerateShaderVertexParams(inputAttr, uiVertexStride);
	//			
	//			pData->SetVertexDeclaration(pVertDecl);		
	//		}		

	//		// Keep track of the current sampler to be used in enabling the
	//		// textures.
	//		m_uCurrSampler = 0;
	//		const uint32 uVTextureCnt = pEffect->GetVTextureQuantity(pass);
	//		uint32 uTex;
	//		bool bChanged = false, bMipmap = false, bNonPow2 = false;
	//		U2Dx9BaseTexture* pTex;
	//		for(uTex=0; uTex < uVTextureCnt; ++uTex)
	//		{
	//			pTex = 
	//				SmartPtrCast(U2Dx9BaseTexture, pEffect->GetVTexture(pass, uTex));
	//			m_pTexMgr->GetTexture(pTex, bChanged, bMipmap, bNonPow2);
	//			EnableTexture(pTex);
	//			m_uCurrSampler++;
	//		}

	//		const uint32 uPTextureCnt = pEffect->GetPTextureQuantity(pass);		
	//		for(uTex=0; uTex < uPTextureCnt; ++uTex)
	//		{
	//			pTex = 
	//				SmartPtrCast(U2Dx9BaseTexture, pEffect->GetPTexture(pass, uTex));
	//			m_pTexMgr->GetTexture(pTex, bChanged, bMipmap, bNonPow2);
	//			EnableTexture(pTex);
	//			m_uCurrSampler++;
	//		}

	//		
	//		m_pD3DDevice9->SetVertexShader(pVProgram->m_pD3DVS);		
	//		m_pD3DDevice9->SetPixelShader(pPProgram->m_pD3DPS);
	//		m_pD3DDevice9->SetVertexDeclaration(pData->GetVertexDeclaration());


	//		//U2ASSERT(pMeshData->GetVertexDeclaration() != NULL);

	//		HRESULT hr;

	//		hr = m_pD3DDevice9->SetStreamSource(0, pBlock->GetVB(), 0,
	//				 pData->GetVertexStride(0));			
	//		U2ASSERT(SUCCEEDED(hr));
	//		
	//		hr = m_pD3DDevice9->SetIndices(pSrcIB);		
	//		U2ASSERT(SUCCEEDED(hr));


	//		m_pRenderState->CommitShaderConsts();


	//		hr = m_pD3DDevice9->DrawIndexedPrimitive(D3DPT_LINELIST, 
	//						pData->GetBaseVertexIndex(), 0, 
	//				pData->GetVertexCount(), 0, uConn / 2);
	//				U2ASSERT(SUCCEEDED(hr));

	//		m_uCurrSampler = 0;


	//		//Disable Texture;
	//		for(uTex=0; uTex < uVTextureCnt; ++uTex)
	//		{
	//			DisableTexture();
	//		}

	//		for(uTex=0; uTex < uPTextureCnt; ++uTex)
	//		{
	//			DisableTexture();
	//		}


	//		/*hr = m_pD3DDevice9->SetVertexShader(0);
	//		U2ASSERT(SUCCEEDED(hr));	

	//		hr = m_pD3DDevice9->SetPixelShader(0);
	//		U2ASSERT(SUCCEEDED(hr));*/

	//		pEffect->RestoreRenderState(pass, m_pRenderState, bPrimaryEffect);
	//	}

	//	bPrimaryEffect = false;
	//}
	//else 
	//{
	//	

	//}

	////	m_pRenderState->RestoreRenderState();
	//

}



/**************************************************************************************************
Func name	:	DrawPritives
params		:	U2Mesh*, U2MeshData*, U2ShaderEffect*, bool
params Desc :
Func Desc	:	 
/*************************************************************************************************/
//void U2Dx9Renderer::DrawPrimitves(U2Mesh* pMesh, U2MeshData* pMeshData, 
//								  U2ShaderEffect* pEffect,bool& bPrimaryEffect)
//{
//
//	U2ASSERT(m_pCurrRSC);
//	m_pRenderState->UpdateRenderState(m_pCurrRSC);
//	
//	// Update Light State	
//	//m_pkD3DRenderer->GetLightManager()->SetState(pkEffects, 
//	//	pkState->GetTexturing(), pkState->GetVertexColor());
//
//
//	//m_uiOldLightingValue = 
//	//	m_pkD3DRenderState->GetRenderState(D3DRS_LIGHTING);
//	
//	
//	// EnableVertexProgram();
//
//	// EnablePixelProgram();
//
//
//	// EnableTexture();
//
//
//	// PAckBuffer
//
//
//	// Commiit ShaderConstants
//
//	//iif (IB)
//	//  DrawIndexedPrimitive
//	//else 
//	// DrawPrimitive
//
//	// m_pkD3DRenderState->SetRenderState(D3DRS_LIGHTING, m_uiOldLightingValue);
//
//	// Restore  Global RenderState 	
//
//	m_worldMat = pMesh->m_matHWorld;	
//
//
//	const uint32 uPassCnt = pEffect->GetPassCnt();
//	HRESULT hr;
//	for(uint32 pass = 0; pass < uPassCnt; ++pass)
//	{
//
//		pEffect->SetRenderState(pass, m_pRenderState, bPrimaryEffect);
//
//		m_uNumActiveSamplers = 0;
//
//		// Set user shader constant
//		pEffect->ConnVSConstants(pass);
//		pEffect->ConnPSConstants(pass);
//
//		U2VertexProgram* pVProgram = pEffect->GetVProgram(pass);
//		U2PixelProgram* pPProgram = pEffect->GetPProgram(pass);	
//
//		const U2VertexAttributes& inputAttr = pVProgram->GetInputVertAtts();
//		const U2VertexAttributes& outputAttr = pVProgram->GetOutputVertAtts();
//
//
//		// Asm Shader Load
//		// EnableVetexProgram, EnablePixelProgram을 먼저 로드해야 
//		// EnableTexture에 해당 데이터를 얻을 수 있으므로 
//		// 순서 주의
//
//		if(pass == 0)
//		{	
//
//			EnableVertexProgram(pVProgram);
//			EnablePixelProgram(pPProgram);				
//
//			// 순서 주의 .. 
//			// PackMeshData함수 안에서 VertexDeclation이 생성됨.
//			PackMeshData(pMeshData, inputAttr, outputAttr);					
//
//		}		
//
//		// Keep track of the current sampler to be used in enabling the
//		// textures.
//		m_uCurrSampler = 0;
//		const uint32 uVTextureCnt = pEffect->GetVTextureQuantity(pass);
//		uint32 uTex;
//		bool bChanged = false, bMipmap = false, bNonPow2 = false;
//		U2Dx9BaseTexture* pTex;
//		for(uTex=0; uTex < uVTextureCnt; ++uTex)
//		{
//			pTex = 
//				SmartPtrCast(U2Dx9BaseTexture, pEffect->GetVTexture(pass, uTex));
//			m_pTexMgr->GetTexture(pTex, bChanged, bMipmap, bNonPow2);
//			EnableTexture(pTex);
//			m_uCurrSampler++;
//		}
//
//		const uint32 uPTextureCnt = pEffect->GetPTextureQuantity(pass);		
//		for(uTex=0; uTex < uPTextureCnt; ++uTex)
//		{
//			pTex = 
//				SmartPtrCast(U2Dx9BaseTexture, pEffect->GetPTexture(pass, uTex));
//			m_pTexMgr->GetTexture(pTex, bChanged, bMipmap, bNonPow2);
//			EnableTexture(pTex);
//			m_uCurrSampler++;
//		}
//
//
//
//		if(pass == 0 // || DYNAMIC_MEsh && skinning 
//			)
//		{
//
//		}
//
//		/*m_pD3DDevice9->SetVertexShader(pVProgram->m_pD3DVS);		
//		m_pD3DDevice9->SetPixelShader(pPProgram->m_pD3DPS);
//		m_pD3DDevice9->SetVertexDeclaration(pMeshData->GetVertexDeclaration());*/
//		m_pRenderState->SetVertexShader(pVProgram->m_pD3DVS);
//		m_pRenderState->SetPixelShader(pPProgram->m_pD3DPS);
//		m_pRenderState->SetVertexDecl(pMeshData->GetVertexDeclaration());
//		
//
//		//U2ASSERT(pMeshData->GetVertexDeclaration() != NULL);
//
//		HRESULT hr;
//
//		for(uint32 i=0;i < pMeshData->GetStreamCount(); ++i)
//		{
//				hr = m_pD3DDevice9->SetStreamSource(0, pMeshData->GetVBBlock(i)
//					->GetVB(), 0, pMeshData->GetVertexStride(i));			
//				U2ASSERT(SUCCEEDED(hr));
//		}
//
//		hr = m_pD3DDevice9->SetIndices(pMeshData->GetIB());		
//		U2ASSERT(SUCCEEDED(hr));
//		
//
//		m_pRenderState->CommitShaderConsts();
//
//
//
//		if(pMeshData->GetIB())
//		{
//			uint32 uStartIdx = 0;
//
//			for(uint32 i=0; i < pMeshData->GetNumArrays(); ++i)
//			{
//				uint32 uiPrimitiveCnt;
//				const uint16* pusArrayLength =
//					pMeshData->GetArrayLengths();
//					if(pusArrayLength)
//					{
//						// TriStrip 
//						uiPrimitiveCnt = pusArrayLength[i] -2;
//					}
//					else 
//						U2ASSERT(pMeshData->GetNumArrays() == 1)
//						uiPrimitiveCnt = pMeshData->GetTriCount();
//
//					
//					hr = m_pD3DDevice9->DrawIndexedPrimitive(pMeshData->GetPrimitiveType(), 
//						pMeshData->GetBaseVertexIndex(), 0, 
//						pMeshData->GetVertexCount(), uStartIdx, uiPrimitiveCnt);
//					U2ASSERT(SUCCEEDED(hr));
//
//					uStartIdx += uiPrimitiveCnt + 2;
//			}
//		}
//		else 
//		{
//			m_pD3DDevice9->DrawPrimitive(pMeshData->GetPrimitiveType(), 
//				pMeshData->GetBaseVertexIndex(), pMeshData->GetTriCount());
//		}
//	
//
//		m_uCurrSampler = 0;
//
//		// Draw();
//
//		// Disable Texture;
//		//for(uTex=0; uTex < uVTextureCnt; ++uTex)
//		//{
//		//	DisableTexture();
//		//}
//
//		//for(uTex=0; uTex < uPTextureCnt; ++uTex)
//		//{
//		//	DisableTexture();
//		//}
//
//		
//		/*hr = m_pD3DDevice9->SetVertexShader(0);
//		U2ASSERT(SUCCEEDED(hr));	
//						
//		hr = m_pD3DDevice9->SetPixelShader(0);
//		U2ASSERT(SUCCEEDED(hr));*/
//
//		pEffect->RestoreRenderState(pass, m_pRenderState, bPrimaryEffect);
//	}
//
//	m_pRenderState->RestoreRenderState();
//	
//
//	bPrimaryEffect = false;
//}

void U2Dx9Renderer::DrawPrimitves(U2Mesh* pMesh, U2MeshData* pMeshData, 
								  U2ShaderEffect* pEffect,bool& bPrimaryEffect)
{

	U2ASSERT(m_pCurrRSC);
	m_pRenderState->UpdateRenderState(m_pCurrRSC);

	// Update Light State	
	//m_pkD3DRenderer->GetLightManager()->SetState(pkEffects, 
	//	pkState->GetTexturing(), pkState->GetVertexColor());


	//m_uiOldLightingValue = 
	//	m_pkD3DRenderState->GetRenderState(D3DRS_LIGHTING);


	// EnableVertexProgram();

	// EnablePixelProgram();


	// EnableTexture();


	// PAckBuffer


	// Commiit ShaderConstants

	//iif (IB)
	//  DrawIndexedPrimitive
	//else 
	// DrawPrimitive

	// m_pkD3DRenderState->SetRenderState(D3DRS_LIGHTING, m_uiOldLightingValue);

	// Restore  Global RenderState 	

	m_worldMat = pMesh->m_matHWorld;	


	const uint32 uPassCnt = pEffect->Begin();
	for(uint32 pass = 0; pass < uPassCnt; ++pass)
	{
		
		pEffect->BeginEffect(pass, this, bPrimaryEffect);
		
		// Setup texture stage 
		pEffect->SetupStage(pass, this);

		pEffect->Commit(pass, this);
		
		//  Move to Commit 
		//HRESULT hr;

		//for(uint32 i=0;i < pMeshData->GetStreamCount(); ++i)
		//{
		//	hr = m_pD3DDevice9->SetStreamSource(0, pMeshData->GetVBBlock(i)
		//		->GetVB(), 0, pMeshData->GetVertexStride(i));			
		//	U2ASSERT(SUCCEEDED(hr));
		//}

		//hr = m_pD3DDevice9->SetIndices(pMeshData->GetIB());		
		//U2ASSERT(SUCCEEDED(hr));


		//if(pMeshData->GetIB())
		//{
		//	uint32 uStartIdx = 0;

		//	for(uint32 i=0; i < pMeshData->GetNumArrays(); ++i)
		//	{
		//		uint32 uiPrimitiveCnt;
		//		const uint16* pusArrayLength =
		//			pMeshData->GetArrayLengths();
		//		if(pusArrayLength)
		//		{
		//			// TriStrip 
		//			uiPrimitiveCnt = pusArrayLength[i] -2;
		//		}
		//		else 
		//			U2ASSERT(pMeshData->GetNumArrays() == 1)
		//			uiPrimitiveCnt = pMeshData->GetTriCount();


		//		hr = m_pD3DDevice9->DrawIndexedPrimitive(pMeshData->GetPrimitiveType(), 
		//			pMeshData->GetBaseVertexIndex(), 0, 
		//			pMeshData->GetVertexCount(), uStartIdx, uiPrimitiveCnt);
		//		U2ASSERT(SUCCEEDED(hr));

		//		uStartIdx += uiPrimitiveCnt + 2;
		//	}
		//}
		//else 
		//{
		//	m_pD3DDevice9->DrawPrimitive(pMeshData->GetPrimitiveType(), 
		//		pMeshData->GetBaseVertexIndex(), pMeshData->GetTriCount());
		//}

		pEffect->EndEffect(pass, this, bPrimaryEffect);
		
	}

	pEffect->End();

	m_pRenderState->RestoreRenderState();


	bPrimaryEffect = false;
}



/**************************************************************************************************
Func name	:	
params		:	
params Desc :
Func Desc	:	 
/*************************************************************************************************/
void U2Dx9Renderer::EnableVertexProgram(U2VertexProgram* pProgram)
{
	if(!pProgram->m_pD3DVS)
	{	
	
	const CHAR* szProgramText;
		
	LPD3DXBUFFER pCompiledShader = 0;
	LPD3DXBUFFER pErrors = 0;
#ifdef UNICODE
	// WCHAR->CHAR 
	U2String buffer = pProgram->GetProgramText();		
	
	size_t len = WideCharToMultiByte(CP_ACP, 0, buffer.c_str(), -1, NULL,
		0, NULL, NULL);

	char* pcProgrmText = U2_ALLOC(char, len);
	WideCharToMultiByte(CP_ACP, 0, buffer.c_str(), -1, pcProgrmText, 
		len, NULL, NULL);	
	szProgramText = pcProgrmText;	
#else 
	szProgramText = pProgram->GetProgramText().c_str();
#endif

	int iProgramLen = int(strlen(szProgramText));
	
	HRESULT hr = D3DXAssembleShader(szProgramText, iProgramLen, 
		0, 0, 0, &pCompiledShader, &pErrors);
	U2ASSERT(SUCCEEDED(hr));
	U2ASSERT(pCompiledShader);

#ifdef UNICODE 
	U2_FREE(pcProgrmText);
	pcProgrmText = NULL;
#endif

	
		hr = m_pD3DDevice9->CreateVertexShader(
			(DWORD*)(pCompiledShader->GetBufferPointer()), &pProgram->m_pD3DVS);
		U2ASSERT(SUCCEEDED(hr));

		SAFE_RELEASE(pCompiledShader);
		SAFE_RELEASE(pErrors);	
	}


	uint32 i;
	for(i=0; i < pProgram->GetSamplerInfoCnt(); ++i)
	{
		U2ASSERT(m_uNumActiveSamplers < GetMaxSimultaneousTextures());
		m_ppActiveSamplers[m_uNumActiveSamplers++] =
			pProgram->GetSamplerInfo(i);
	}

	pProgram->m_uiRegister = 0;

	for(i=0; i < pProgram->GetShaderConstantCnt();++i)
	{
		U2ShaderConstant* pSC = pProgram->GetShaderConstant(i);
		U2ASSERT(pSC);
		SetShaderConstant(pSC->GetSCM(), pSC->GetData());		
		m_pRenderState->SetVertexShaderConstantF(pSC->GetBaseRegister(), 
			pSC->GetData(), pSC->GetRegisterCnt());		
		pProgram->m_uiRegister += pSC->GetBaseRegister();
	}

	for(i=0; i < pProgram->GetNumericalConstantCnt();++i)
	{
		U2NumericalConstant* pNC = pProgram->GetNumericalConstant(i);
		m_pRenderState->SetVertexShaderConstantF(pNC->GetRegister() + pProgram->m_uiRegister, 
			pNC->GetData(), 1);
		pProgram->m_uiRegister += pNC->GetRegister();	
	}

	for(i=0; i < pProgram->GetUserConstantCnt();++i)
	{
		U2ShaderUserConstant* pUC = pProgram->GetUserConstant(i);
		U2ASSERT(pUC);
		m_pRenderState->SetVertexShaderConstantF(pUC->GetBaseRegister() +  pProgram->m_uiRegister,
			pUC->GetData(), pUC->GetRegisterCnt());
		pProgram->m_uiRegister += pUC->GetBaseRegister();	
	}		
}


/************************************************************************
Func name	:	
params		:	
params Desc :
Func Desc	:	 
************************************************************************/
void U2Dx9Renderer::EnablePixelProgram(U2PixelProgram* pProgram)
{
	if(!pProgram->m_pD3DPS)
	{

	
		const CHAR* szProgramText;

		LPD3DXBUFFER pCompiledShader = 0;
		LPD3DXBUFFER pErrors = 0;
	#ifdef UNICODE
		// WCHAR->CHAR 
		U2String buffer = pProgram->GetProgramText();
		size_t len = WideCharToMultiByte(CP_ACP, 0, buffer.c_str(), -1, NULL,
			0, NULL, NULL);

		char* pcProgrmText = U2_ALLOC(char, len);
		WideCharToMultiByte(CP_ACP, 0, buffer.c_str(), -1, pcProgrmText, 
			len, NULL, NULL);	
		szProgramText = pcProgrmText;	
	#else 
		szProgramText = pProgram->GetProgramText().c_str();
	#endif

		int iProgramLen = int(strlen(szProgramText));

		HRESULT hr = D3DXAssembleShader(szProgramText, iProgramLen, 
			0, 0, 0, &pCompiledShader, &pErrors);
		U2ASSERT(SUCCEEDED(hr));
		U2ASSERT(pCompiledShader);

	#ifdef UNICODE 
		U2_FREE(pcProgrmText);
		pcProgrmText = NULL;
	#endif

		hr = m_pD3DDevice9->CreatePixelShader(
			(DWORD*)(pCompiledShader->GetBufferPointer()), &pProgram->m_pD3DPS);
		U2ASSERT(SUCCEEDED(hr));

		SAFE_RELEASE(pCompiledShader);
		SAFE_RELEASE(pErrors);	
	}
	


	pProgram->m_uiRegister = 0;

	uint32 i;
	for(i=0; i < pProgram->GetSamplerInfoCnt(); ++i)
	{
		U2ASSERT(m_uNumActiveSamplers < GetMaxSimultaneousTextures());
		m_ppActiveSamplers[m_uNumActiveSamplers++] =
			pProgram->GetSamplerInfo(i);
	}

	for(i=0; i < pProgram->GetShaderConstantCnt();++i)
	{
		U2ShaderConstant* pSC = pProgram->GetShaderConstant(i);
		U2ASSERT(pSC);
		SetShaderConstant(pSC->GetSCM(), pSC->GetData());		
		m_pRenderState->SetPixelShaderConstantF(pSC->GetBaseRegister() + pProgram->m_uiRegister, 
			pSC->GetData(), pSC->GetRegisterCnt());	
		pProgram->m_uiRegister += pSC->GetBaseRegister();
	}

	for(i=0; i < pProgram->GetNumericalConstantCnt();++i)
	{
		U2NumericalConstant* pNC = pProgram->GetNumericalConstant(i);
		m_pRenderState->SetPixelShaderConstantF(pNC->GetRegister() + pProgram->m_uiRegister,
			pNC->GetData(), 1);
		pProgram->m_uiRegister += pNC->GetRegister();
	}

	for(i=0; i < pProgram->GetUserConstantCnt();++i)
	{
		U2ShaderUserConstant* pUC = pProgram->GetUserConstant(i);
		U2ASSERT(pUC);
		m_pRenderState->SetPixelShaderConstantF(pUC->GetBaseRegister() + pProgram->m_uiRegister,
			pUC->GetData(), pUC->GetRegisterCnt());
		pProgram->m_uiRegister += pUC->GetBaseRegister();
	}		


}


/************************************************************************
Func name	:	
params		:	
params Desc :
Func Desc	:	 
************************************************************************/
void U2Dx9Renderer::EnableTexture(U2Dx9BaseTexture* pTex)
{

	U2ShaderSamplerInfo* pSI = m_ppActiveSamplers[m_uCurrSampler];
	U2ShaderSamplerInfo::SamplerType eSamplerType = pSI->GetSamplerType();
	int iTexUnit = pSI->GetTextureUnit();

#ifdef USE_RENDERSTATE 

	float fAnisotropy = pTex->GetAnisotropyValue();
	if(1.0f < fAnisotropy && fAnisotropy <= m_usHwMaxAnisotropy)
	{
		m_pRenderState->SetSamplerState(iTexUnit, D3DSAMP_MAXANISOTROPY, 
			(DWORD)fAnisotropy);
		
	}
	else 
	{
		m_pRenderState->SetSamplerState(iTexUnit, D3DSAMP_MAXANISOTROPY,
			(DWORD)1.F);		
	}

	U2Dx9BaseTexture::FilterType eFType = pTex->GetFilterType();
	if(eFType == U2Dx9BaseTexture::NEAREST)
	{
		m_pRenderState->SetSamplerState(iTexUnit, D3DSAMP_MAGFILTER, 
			D3DTEXF_POINT);		
	}
	else 
	{
		if(1.0f < fAnisotropy && fAnisotropy <= m_usHwMaxAnisotropy)
		{
			m_pRenderState->SetSamplerState(iTexUnit, 
				D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);			
		}
		else 
		{
			m_pRenderState->SetSamplerState(iTexUnit, 
				D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);			
		}
	}

	// Set MaipMap mode 
	if(1.0f < fAnisotropy && fAnisotropy <= m_usHwMaxAnisotropy)
	{
		m_pRenderState->SetSamplerState(iTexUnit, 
			D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		m_pRenderState->SetSamplerState(iTexUnit, 
			D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);		
	}
	else 
	{
		m_pRenderState->SetSamplerState(iTexUnit, 
			D3DSAMP_MINFILTER, ms_adwTexMinFilter[eFType]);
		m_pRenderState->SetSamplerState(iTexUnit, 
			D3DSAMP_MIPFILTER, ms_adwTexMipFilter[eFType]);					
	}

	const D3DXCOLOR& borderColor = pTex->GetBorderColor();
	m_pRenderState->SetSamplerState(iTexUnit, 
		D3DSAMP_BORDERCOLOR, D3DCOLOR_COLORVALUE(borderColor.r,
		borderColor.g, borderColor.b, borderColor.a));	

	switch(eSamplerType)
	{
	case U2ShaderSamplerInfo::SAMPLER_1D:
		{
			m_pRenderState->SetSamplerState(iTexUnit, 
				D3DSAMP_ADDRESSU, ms_adwTexWrapMode[pTex->GetWrapType(0)]);
			m_pRenderState->SetSamplerState(iTexUnit, 
				D3DSAMP_ADDRESSV, ms_adwTexWrapMode[pTex->GetWrapType(0)]);			
		}
		break;
	case U2ShaderSamplerInfo::SAMPLER_2D:
	case U2ShaderSamplerInfo::SAMPLER_PROJ:
	case U2ShaderSamplerInfo::SAMPLER_CUBE:
		{
			m_pRenderState->SetSamplerState(iTexUnit, 
				D3DSAMP_ADDRESSU, ms_adwTexWrapMode[pTex->GetWrapType(0)]);
			m_pRenderState->SetSamplerState(iTexUnit, 
				D3DSAMP_ADDRESSV, ms_adwTexWrapMode[pTex->GetWrapType(1)]);			
		}
		break;
	case U2ShaderSamplerInfo::SAMPLER_3D:
		{
			m_pRenderState->SetSamplerState(iTexUnit, 
				D3DSAMP_ADDRESSU, ms_adwTexWrapMode[pTex->GetWrapType(0)]);
			m_pRenderState->SetSamplerState(iTexUnit, 
				D3DSAMP_ADDRESSV, ms_adwTexWrapMode[pTex->GetWrapType(1)]);
			m_pRenderState->SetSamplerState(iTexUnit, 
				D3DSAMP_ADDRESSW, ms_adwTexWrapMode[pTex->GetWrapType(2)]);							
		}
		break;
	default:
		U2ASSERT(false);
		break;
	}

	m_pRenderState->SetTexture(iTexUnit, pTex->GetD3DTex());	
#else 
	float fAnisotropy = pTex->GetAnisotropyValue();
	if(1.0f < fAnisotropy && fAnisotropy <= m_usHwMaxAnisotropy)
	{		
		m_pD3DDevice9->SetSamplerState(iTexUnit, D3DSAMP_MAXANISOTROPY, 
			(DWORD)fAnisotropy);
	}
	else 
	{
		m_pD3DDevice9->SetSamplerState(iTexUnit, D3DSAMP_MAXANISOTROPY,
			(DWORD)1.F);
	}

	U2Dx9BaseTexture::FilterType eFType = pTex->GetFilterType();
	if(eFType == U2Dx9BaseTexture::NEAREST)
	{	
		m_pD3DDevice9->SetSamplerState(iTexUnit, D3DSAMP_MAGFILTER, 
			D3DTEXF_POINT);
	}
	else 
	{
		if(1.0f < fAnisotropy && fAnisotropy <= m_usHwMaxAnisotropy)
		{
			m_pD3DDevice9->SetSamplerState(iTexUnit, 
				D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
		}
		else 
		{
			m_pD3DDevice9->SetSamplerState(iTexUnit, 
				D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		}
	}

	// Set MaipMap mode 
	if(1.0f < fAnisotropy && fAnisotropy <= m_usHwMaxAnisotropy)
	{	
		m_pD3DDevice9->SetSamplerState(iTexUnit, 
		D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
			m_pD3DDevice9->SetSamplerState(iTexUnit, 
			D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
	}
	else 
	{		
		m_pD3DDevice9->SetSamplerState(iTexUnit, 
			D3DSAMP_MINFILTER, ms_adwTexMinFilter[eFType]);
		m_pD3DDevice9->SetSamplerState(iTexUnit, 
			D3DSAMP_MIPFILTER, ms_adwTexMipFilter[eFType]);			
	}

	const D3DXCOLOR& borderColor = pTex->GetBorderColor();	
	m_pD3DDevice9->SetSamplerState(iTexUnit, 
		D3DSAMP_BORDERCOLOR, D3DCOLOR_COLORVALUE(borderColor.r,
		borderColor.g, borderColor.b, borderColor.a));
	
	switch(eSamplerType)
	{
	case U2ShaderSamplerInfo::SAMPLER_1D:
		{
			m_pD3DDevice9->SetSamplerState(iTexUnit, 
				D3DSAMP_ADDRESSU, ms_adwTexWrapMode[pTex->GetWrapType(0)]);
			m_pD3DDevice9->SetSamplerState(iTexUnit, 
				D3DSAMP_ADDRESSV, ms_adwTexWrapMode[pTex->GetWrapType(0)]);
		}
		break;
	case U2ShaderSamplerInfo::SAMPLER_2D:
	case U2ShaderSamplerInfo::SAMPLER_PROJ:
	case U2ShaderSamplerInfo::SAMPLER_CUBE:
		{		
			m_pD3DDevice9->SetSamplerState(iTexUnit, 
				D3DSAMP_ADDRESSU, ms_adwTexWrapMode[pTex->GetWrapType(0)]);
			m_pD3DDevice9->SetSamplerState(iTexUnit, 
				D3DSAMP_ADDRESSV, ms_adwTexWrapMode[pTex->GetWrapType(1)]);
		}
		break;
	case U2ShaderSamplerInfo::SAMPLER_3D:
		{			
			m_pD3DDevice9->SetSamplerState(iTexUnit, 
				D3DSAMP_ADDRESSU, ms_adwTexWrapMode[pTex->GetWrapType(0)]);
			m_pD3DDevice9->SetSamplerState(iTexUnit, 
				D3DSAMP_ADDRESSV, ms_adwTexWrapMode[pTex->GetWrapType(1)]);
			m_pD3DDevice9->SetSamplerState(iTexUnit, 
				D3DSAMP_ADDRESSW, ms_adwTexWrapMode[pTex->GetWrapType(2)]);				
		}
		break;
	default:
		U2ASSERT(false);
		break;
	}

	
	m_pD3DDevice9->SetTexture(iTexUnit, pTex->GetD3DTex());
#endif

}

//-------------------------------------------------------------------------------------------------
bool U2Dx9Renderer::PackMeshData(U2MeshData* pModelData, 
								 const U2VertexAttributes& inputAttr, 
								 const U2VertexAttributes& outputAttr)
{

	// 현재 멀티스트림은 미지원 그러므로 stream 값은 0
	if(!pModelData)
		return false;


	// Static Mesh이면 이미 Vertex Buffer를 생성했으로 다시 
	// 만들 필요 없다. 
	// 성능이 엄청나게 다운되므로 주의!!	
	//bool bVBBlockValid = pModelData->IsVBBlockValid();
	//
	//if(bVBBlockValid && 
	//	pModelData->GetMeshState() == U2MeshData::STATIC_MESH)
	//{
	//	return true;
	//}	

	pModelData->SetVertexAttbs((U2VertexAttributes*)&inputAttr);

	unsigned short usVertexCnt = pModelData->GetActiveVertexCount();
	uint16 usMaxVertexCnt = pModelData->GetVertexCount();
	uint16 usTriCnt;
	uint16 usMaxTriCnt;
	uint32 uIndexCnt;
	uint32 uMaxIndexCnt;
	uint16* pusIndexArray = NULL;
	uint16* pusArrayLengths = NULL;
	uint16 usArrayCnt; 

	U2TriStripData* pTSData = DynamicCast<U2TriStripData>(pModelData);
	if(pTSData)
	{
		usTriCnt = pTSData->GetActiveTriangleCount();
		usMaxTriCnt = pTSData->GetTriangleCnt();

		pusIndexArray = pTSData->GetStripLists();
		pusArrayLengths = pTSData->GetStripLengths();
		usArrayCnt = pTSData->GetStripCnt();	// TripStrip Obj Num
		uIndexCnt = usTriCnt + 2 * usArrayCnt;
		uMaxIndexCnt = usMaxTriCnt + 2 * usArrayCnt;
	}
	else 
	{
		U2TriListData* pTLData = DynamicCast<U2TriListData>(pModelData);
		if(pTLData)
		{	
			usTriCnt = pTLData->GetActiveTriangleCount();
			usMaxTriCnt = pTLData->GetTriangleCnt();	
			pusIndexArray = pTLData->GetTriList();
			pusArrayLengths = NULL;	// TriStrip시만 적용
			usArrayCnt = 1;
			uIndexCnt = 3 * usTriCnt;
		}
		else 
		{
			U2ASSERT(false);
		}
	}
	
	
	

	uint32 uNumStream = 1;

	//if(bVBBlockValid)
	//{
	//	if(uNumStream != pModelData->GetStreamCount())
	//	{
	//		pModelData->SetStreamCount(uNumStream);
	//	}
	//}
	//else 
	//{
	//	U2VBGroup* pVBGroup = pModelData->GetVBGroup();
	//	for(uint32 i=0; i < uNumStream; ++i)
	//		pVBGroup->ReleaseVBBlock(pModelData, i);
	//	pModelData->SetStreamCount(uNumStream);
	//	pModelData->SetBaseVertexIndex(0);
	//}
	
	pModelData->SetVertexCount(usVertexCnt, usMaxVertexCnt);
	pModelData->SetIndices(usTriCnt, usMaxTriCnt, pusIndexArray, 
		pusArrayLengths, usArrayCnt);
	
	
	


	unsigned short usDirtyFlags = 0xffff; // 아직 사용안함.. 
	for(uint32 i=0; i < uNumStream; ++i)
	{
		//U2VBBlock* pExistVBBlock = pModelData->GetVBBlock(i);
		//U2VBBlock* pSrcVBBlock = NULL;

		//// Shader
		//pSrcVBBlock = m_pVBMgr->PackShaderModelVB(pModelData, usDirtyFlags, 
		//		pExistVBBlock, i);
		//// FVF 
		///*pSrcVBBlock = m_pVBMgr->PackModelVB(pModelData, usDirtyFlags, pExistVBBlock);*/
		//
		//if(pSrcVBBlock == NULL)
		//{
		//	return false;
		//}
	}	

	if(pusIndexArray != NULL)
	{
		LPDIRECT3DINDEXBUFFER9 pExistIB = pModelData->GetIB();
		uint32 uIBSize = pModelData->GetIBSize();

		LPDIRECT3DINDEXBUFFER9 pSrcIB = m_pIBMgr->PackIB(
			pusIndexArray, uIndexCnt, uIndexCnt, pExistIB, 
			uIBSize, D3DPOOL_MANAGED, (pModelData->GetSoftwareVP() ? 
			D3DUSAGE_SOFTWAREPROCESSING : 0));

		if(!pSrcIB)
		{
			return false;
		}

		if(pExistIB != pSrcIB)
		{
			pModelData->RemoveIB();
			pModelData->SetIB(pSrcIB, uIndexCnt, uIBSize);
		}
	}

	return true;	
}

//-------------------------------------------------------------------------------------------------
void U2Dx9Renderer::SetShaderConstant(U2ShaderConstant::ShaderConstantMapping eSCM, 
					   float* afData)
{
	int scm = (int)eSCM;
	int iFunc;

	if(scm <= (int)U2ShaderConstant::SCM_INVERSE_WORLDVIEWPROJ_TRANSPOSE)
	{		
		int iOperation = scm / 7;
		iFunc =  scm % 7;
		(this->*ms_aSCFun[iFunc])(iOperation, afData);
	}
	else if(scm <= (int)U2ShaderConstant::SCM_PROJECTOR_MATRIX)
	{
		iFunc = 7 + scm - (int)U2ShaderConstant::SCM_MATRIX_COUNT;
		(this->*ms_aSCFun[iFunc])(0, afData);
	}
	else 
	{
		int iDiff = scm - (int)U2ShaderConstant::SCM_LIGHT0_MODEL_POS;
		int iLight = iDiff / 9;
		int iAttribute = iDiff % 9;
		iFunc = 28 + iAttribute;
		(this->*ms_aSCFun[iFunc])(iLight, afData);
	}
}

//-------------------------------------------------------------------------------------------------
void U2Dx9Renderer::DisableTexture()
{
	U2ShaderSamplerInfo* pSI = m_ppActiveSamplers[m_uCurrSampler];
	int iTexUnit = pSI->GetTextureUnit();

	HRESULT hr = m_pD3DDevice9->SetTextureStageState(iTexUnit, 
		D3DTSS_COLOROP, D3DTOP_DISABLE);
	U2ASSERT(SUCCEEDED(hr));

	hr = m_pD3DDevice9->SetTexture(iTexUnit, 0);
	U2ASSERT(SUCCEEDED(hr));
}

//-------------------------------------------------------------------------------------------------
void U2Dx9Renderer::DrawScene(U2VisibleSet& visibleSet)
{
	// 주석 반드시 필요..
	const int iMaxEffect = 64;
	int aaiStack[iMaxEffect][2]; // start. end 
	int iTop = -1;

	const uint32 uVisibleCnt = visibleSet.GetCount();
	U2VisibleObject* pVisible = visibleSet.GetVisible();
	for(uint32 i=0; i < uVisibleCnt; ++i)
	{
		if(pVisible[i].m_pObj)
		{
			if(pVisible[i].m_pGlobalEffect)
			{
				iTop++;
				U2ASSERT(iTop < iMaxEffect);
				aaiStack[iTop][0] = i;
				aaiStack[iTop][1] = i;
			}
			else 
			{
				if(iTop == -1)
				{
					((U2Mesh*)pVisible[i].m_pObj)->Render(this);
				}
				else 
				{
					aaiStack[iTop][1]++;
				}
			}

		}
		else 
		{
			U2ASSERT(!pVisible[i].m_pGlobalEffect);
			int iStart = aaiStack[iTop][0];
			int iEnd = aaiStack[iTop][1];

			pVisible[iStart].m_pGlobalEffect->Render(this, pVisible[iStart].m_pObj, 
				iStart+1, iEnd, pVisible);
			if(--iTop >= 0)
			{
				aaiStack[iTop][1] = iEnd + 1;
			}
		}
	}
}


void U2Dx9Renderer::SetCameraData(U2Camera* pCam) 
{


	m_spCamera = pCam;
	// 1. Update View Matrix
	// 2. Update Projection Matrix 
	// 3. Update Viewport 
	// D3DX Math Function 참조.

	if(!m_bDeviceLost)
	{
		D3DXVECTOR3 vRight = pCam->GetRight();
		D3DXVECTOR3 vUp = pCam->GetUp();
		D3DXVECTOR3 vDir = pCam->GetDir();
		D3DXVECTOR3 eye = pCam->GetEye();

		// 1. Left handed Coordinates View Matrix
		m_viewMat._11 = vRight.x;
		m_viewMat._12 = vUp.x;
		m_viewMat._13 = vDir.x;
		m_viewMat._14 = 0.f;
		m_viewMat._21 = vRight.y;
		m_viewMat._22 = vUp.y;
		m_viewMat._23 = vDir.y;
		m_viewMat._24 = 0.f;
		m_viewMat._31 = vRight.z;
		m_viewMat._32 = vUp.z;
		m_viewMat._33 = vDir.z;		
		m_viewMat._34 = 0.f;
		m_viewMat._41 = -D3DXVec3Dot(&eye, &vRight);
		m_viewMat._42 = -D3DXVec3Dot(&eye, &vUp);
		m_viewMat._43 = -D3DXVec3Dot(&eye, &vDir);
		m_viewMat._44 = 1.f;

		m_invView._11 = vRight.x;
		m_invView._12 = vRight.y;
		m_invView._13 = vRight.z;
		m_invView._14 = 0.f;
		m_invView._21 = vUp.x;
		m_invView._22 = vUp.y;
		m_invView._23 = vUp.z;
		m_invView._24 = 0.f;
		m_invView._31 = vDir.x;
		m_invView._32 = vDir.y;
		m_invView._33 = vDir.z;
		m_invView._34 = 0.f;
		m_invView._41 = eye.x;
		m_invView._42 = eye.y;
		m_invView._43 = eye.z;
		m_invView._44 = 1.f;


		m_pD3DDevice9->SetTransform(D3DTS_VIEW, &m_viewMat);

		U2Frustum frustum = pCam->GetFrustum();

		m_fNearDepth = frustum.m_fNear;
		m_fDepthRange = frustum.m_fFar - frustum.m_fNear;
		
		float fRmL = frustum.m_fRight - frustum.m_fLeft;	// Right minus Left
		float fRpL = frustum.m_fRight + frustum.m_fLeft;	// Right plus Left
		float fTmB = frustum.m_fTop - frustum.m_fBottom;	// Top minus Bottom
		float fTpB = frustum.m_fTop + frustum.m_fBottom;	// Top pluse Bottom
		float fInvFmN = 1.0f / m_fDepthRange;				// Inv m_fDepthRange		

		
		// 2. // Left Hand Coordinates Projection Matrix 		
		if(frustum.m_bOrtho)
		{		
			m_projMat._11 = 2.f / fRmL;
			m_projMat._21 = 0.f;
			m_projMat._31 = 0.f;
			m_projMat._41 = 0.f;

			m_projMat._12 = 0.f;
			m_projMat._22 = 2.0f / fTmB;
			m_projMat._32 = 0.f;
			m_projMat._42 = 0.f;
			
			m_projMat._13 = 0.f;
			m_projMat._23 = 0.f;
			m_projMat._33 = fInvFmN;
			m_projMat._43 = -(frustum.m_fNear * fInvFmN);
						
			m_projMat._14 = 0.f;
			m_projMat._24 = 0.f;
			m_projMat._34 = 0.f;
			m_projMat._44 = 1.f;		
		}
		else 
		{
			m_projMat._11 = 2.0f *  m_fNearDepth/ fRmL;
			m_projMat._21 = 0.f;
			m_projMat._31 = 0.f;
			m_projMat._41 = 0.f;
			m_projMat._12 = 0.f;
			m_projMat._22 = 2.0f * m_fNearDepth / fTmB;
			m_projMat._32 = 0.f;
			m_projMat._42 = 0.f;			
			m_projMat._13 = 0.f;
			m_projMat._23 = 0.f;
			m_projMat._33 = frustum.m_fFar * fInvFmN;
			m_projMat._43 = -(frustum.m_fNear * frustum.m_fFar * fInvFmN);
			
			//Direct3D checks the fourth column of the projection matrix. 
			//If the coefficients are [0,0,0,1] (for an affine projection) 
			//the system will use z-based depth values for fog. In this case, 
			//you must also specify the start and end distances for linear fog effects 
			//in device space, which ranges from 0.0 at the nearest point to the user, 
			//and 1.0 at the farthest point.
			m_projMat._14 = 0.f;
			m_projMat._24 = 0.f;
			m_projMat._34 = 1.f;
			m_projMat._44 = 0.f;
		}

		m_pD3DDevice9->SetTransform(D3DTS_PROJECTION, &m_projMat);

		// 3. Update Viewport
		U2ASSERT(m_pCurrRenderTarget);
		float fW = (float)m_pCurrRenderTarget->GetWidth(0);
		float fH = (float)m_pCurrRenderTarget->GetHeight(0);

		D3DVIEWPORT9 viewPort;

		float fLeft, fRight, fTop, fBottom;
		m_spCamera->GetViewport(fLeft, fRight, fTop, fBottom);

		viewPort.X = unsigned int((fLeft * fW));
		viewPort.Y = unsigned int((1.0f - fTop) * fH);
		viewPort.Width = unsigned int((fRight - fLeft) * fW);
		viewPort.Height = unsigned int((fTop - fBottom) * fH);

		viewPort.MinZ = 0.f;
		viewPort.MaxZ = 1.f;

		m_pD3DDevice9->SetViewport(&viewPort);

		//U2ASSERT(m_pRenderState);
		if(m_pRenderState)		
			m_pRenderState->SetCameraNearAndFar(frustum.m_fNear, frustum.m_fFar);

	}

}

void U2Dx9Renderer::SetCamera(U2Camera* pCam) 
{
		

	m_spCamera = pCam;
	// 1. Update View Matrix
	// 2. Update Projection Matrix 
	// 3. Update Viewport 
	// D3DX Math Function 참조.

	if(!m_bDeviceLost)
	{
		D3DXVECTOR3 vRight = pCam->GetRight();
		D3DXVECTOR3 vUp = pCam->GetUp();
		D3DXVECTOR3 vDir = pCam->GetDir();
		D3DXVECTOR3 eye = pCam->GetEye();
		
		// 1. Left handed Coordinates View Matrix
		m_viewMat._11 = vRight.x;
		m_viewMat._12 = vUp.x;
		m_viewMat._13 = vDir.x;
		m_viewMat._14 = 0.f;
		m_viewMat._21 = vRight.y;
		m_viewMat._22 = vUp.y;
		m_viewMat._23 = vDir.y;
		m_viewMat._24 = 0.f;
		m_viewMat._31 = vRight.z;
		m_viewMat._32 = vUp.z;
		m_viewMat._33 = vDir.z;		
		m_viewMat._34 = 0.f;
		m_viewMat._41 = -D3DXVec3Dot(&eye, &vRight);
		m_viewMat._42 = -D3DXVec3Dot(&eye, &vUp);
		m_viewMat._43 = -D3DXVec3Dot(&eye, &vDir);
		m_viewMat._44 = 1.f;

		m_invView._11 = vRight.x;
		m_invView._12 = vRight.y;
		m_invView._13 = vRight.z;
		m_invView._14 = 0.f;
		m_invView._21 = vUp.x;
		m_invView._22 = vUp.y;
		m_invView._23 = vUp.z;
		m_invView._24 = 0.f;
		m_invView._31 = vDir.x;
		m_invView._32 = vDir.y;
		m_invView._33 = vDir.z;
		m_invView._34 = 0.f;
		m_invView._41 = eye.x;
		m_invView._42 = eye.y;
		m_invView._43 = eye.z;
		m_invView._44 = 1.f;


		//m_pD3DDevice9->SetTransform(D3DTS_VIEW, &m_viewMat);

		//U2Frustum frustum = pCam->GetFrustum();

		//m_fNearDepth = frustum.m_fNear;
		//m_fDepthRange = frustum.m_fFar - frustum.m_fNear;
		//
		//float fRmL = frustum.m_fRight - frustum.m_fLeft;	// Right minus Left
		//float fRpL = frustum.m_fRight + frustum.m_fLeft;	// Right plus Left
		//float fTmB = frustum.m_fTop - frustum.m_fBottom;	// Top minus Bottom
		//float fTpB = frustum.m_fTop + frustum.m_fBottom;	// Top pluse Bottom
		//float fInvFmN = 1.0f / m_fDepthRange;				// Inv m_fDepthRange		

		//
		//// 2. // Left Hand Coordinates Projection Matrix 		
		//if(frustum.m_bOrtho)
		//{		
		//	m_projMat._11 = 2.f / fRmL;
		//	m_projMat._21 = 0.f;
		//	m_projMat._31 = 0.f;
		//	m_projMat._41 = 0.f;

		//	m_projMat._12 = 0.f;
		//	m_projMat._22 = 2.0f / fTmB;
		//	m_projMat._32 = 0.f;
		//	m_projMat._42 = 0.f;
		//	
		//	m_projMat._13 = 0.f;
		//	m_projMat._23 = 0.f;
		//	m_projMat._33 = fInvFmN;
		//	m_projMat._43 = -(frustum.m_fNear * fInvFmN);
		//				
		//	m_projMat._14 = 0.f;
		//	m_projMat._24 = 0.f;
		//	m_projMat._34 = 0.f;
		//	m_projMat._44 = 1.f;		
		//}
		//else 
		//{
		//	m_projMat._11 = 2.0f *  m_fNearDepth/ fRmL;
		//	m_projMat._21 = 0.f;
		//	m_projMat._31 = 0.f;
		//	m_projMat._41 = 0.f;
		//	m_projMat._12 = 0.f;
		//	m_projMat._22 = 2.0f * m_fNearDepth / fTmB;
		//	m_projMat._32 = 0.f;
		//	m_projMat._42 = 0.f;			
		//	m_projMat._13 = 0.f;
		//	m_projMat._23 = 0.f;
		//	m_projMat._33 = frustum.m_fFar * fInvFmN;
		//	m_projMat._43 = -(frustum.m_fNear * frustum.m_fFar * fInvFmN);
		//	
		//	//Direct3D checks the fourth column of the projection matrix. 
		//	//If the coefficients are [0,0,0,1] (for an affine projection) 
		//	//the system will use z-based depth values for fog. In this case, 
		//	//you must also specify the start and end distances for linear fog effects 
		//	//in device space, which ranges from 0.0 at the nearest point to the user, 
		//	//and 1.0 at the farthest point.
		//	m_projMat._14 = 0.f;
		//	m_projMat._24 = 0.f;
		//	m_projMat._34 = 1.f;
		//	m_projMat._44 = 0.f;
		//}

		//m_pD3DDevice9->SetTransform(D3DTS_PROJECTION, &m_projMat);

		//// 3. Update Viewport
		//U2ASSERT(m_pCurrRenderTarget);
		//float fW = (float)m_pCurrRenderTarget->GetWidth(0);
		//float fH = (float)m_pCurrRenderTarget->GetHeight(0);

		//D3DVIEWPORT9 viewPort;

		//float fLeft, fRight, fTop, fBottom;
		//m_spCamera->GetViewport(fLeft, fRight, fTop, fBottom);

		//viewPort.X = unsigned int((fLeft * fW));
		//viewPort.Y = unsigned int((1.0f - fTop) * fH);
		//viewPort.Width = unsigned int((fRight - fLeft) * fW);
		//viewPort.Height = unsigned int((fTop - fBottom) * fH);

		//viewPort.MinZ = 0.f;
		//viewPort.MaxZ = 1.f;

		//m_pD3DDevice9->SetViewport(&viewPort);

		//U2ASSERT(m_pRenderState);
		//if(m_pRenderState)		
		//	m_pRenderState->SetCameraNearAndFar(frustum.m_fNear, frustum.m_fFar);

	}

}


void U2Dx9Renderer::SetViewport(U2Camera* pCam)
{
	if(!m_spCamera)
		m_spCamera = pCam;

	// 3. Update Viewport
	U2ASSERT(m_pCurrRenderTarget);
	float fW = (float)m_pCurrRenderTarget->GetWidth(0);
	float fH = (float)m_pCurrRenderTarget->GetHeight(0);

	D3DVIEWPORT9 viewPort;

	float fLeft, fRight, fTop, fBottom;
	m_spCamera->GetViewport(fLeft, fRight, fTop, fBottom);

	viewPort.X = unsigned int((fLeft * fW));
	viewPort.Y = unsigned int((1.0f - fTop) * fH);
	viewPort.Width = unsigned int((fRight - fLeft) * fW);
	viewPort.Height = unsigned int((fTop - fBottom) * fH);

	viewPort.MinZ = 0.f;
	viewPort.MaxZ = 1.f;

	m_pD3DDevice9->SetViewport(&viewPort);

}


void U2Dx9Renderer::SetProjectionTM(U2Camera* pCam)
{

	// Fix : Frustum update flag need
	if(!m_spCamera)
		m_spCamera = pCam;

	U2Frustum frustum = m_spCamera->GetFrustum();

	m_fNearDepth = frustum.m_fNear;
	m_fDepthRange = frustum.m_fFar - frustum.m_fNear;

	float fRmL = frustum.m_fRight - frustum.m_fLeft;	// Right minus Left
	float fRpL = frustum.m_fRight + frustum.m_fLeft;	// Right plus Left
	float fTmB = frustum.m_fTop - frustum.m_fBottom;	// Top minus Bottom
	float fTpB = frustum.m_fTop + frustum.m_fBottom;	// Top pluse Bottom
	float fInvFmN = 1.0f / m_fDepthRange;				// Inv m_fDepthRange		


	// 2. // Right Hand Coordinates Projection Matrix 		
	if(frustum.m_bOrtho)
	{	
		
		m_projMat._11 = 2.f / fRmL;
		m_projMat._21 = 0.f;
		m_projMat._31 = 0.f;
		m_projMat._41 = 0.f;

		m_projMat._12 = 0.f;
		m_projMat._22 = 2.0f / fTmB;
		m_projMat._32 = 0.f;
		m_projMat._42 = 0.f;

		m_projMat._13 = 0.f;
		m_projMat._23 = 0.f;
		m_projMat._33 = -fInvFmN;
		m_projMat._43 = -(frustum.m_fNear * fInvFmN);

		m_projMat._14 = 0.f;
		m_projMat._24 = 0.f;
		m_projMat._34 = 0.f;
		m_projMat._44 = 1.f;		
	}
	else 
	{
		// DirectX Perspective LH		
		m_projMat._11 = 2.0f *  m_fNearDepth/ fRmL; 
		m_projMat._21 = 0.f;
		m_projMat._31 = 0.f;
		m_projMat._41 = 0.f;
		m_projMat._12 = 0.f;
		m_projMat._22 = 2.0f * m_fNearDepth / fTmB;
		m_projMat._32 = 0.f;
		m_projMat._42 = 0.f;			
		m_projMat._13 = 0.f;
		m_projMat._23 = 0.f;
		m_projMat._33 = frustum.m_fFar * fInvFmN;
		m_projMat._43 = -(frustum.m_fNear * frustum.m_fFar * fInvFmN);

		//Direct3D checks the fourth column of the projection matrix. 
		//If the coefficients are [0,0,0,1] (for an affine projection) 
		//the system will use z-based depth values for fog. In this case, 
		//you must also specify the start and end distances for linear fog effects 
		//in device space, which ranges from 0.0 at the nearest point to the user, 
		//and 1.0 at the farthest point.
		m_projMat._14 = 0.f;
		m_projMat._24 = 0.f;
		m_projMat._34 = 1.f;
		m_projMat._44 = 0.f;

		// Wild Magic Engine DirectX RH		
		//m_projMat._11 = 2.0f *  m_fNearDepth/ fRmL;
		//m_projMat._21 = 0.f;
		//m_projMat._31 = -fRpL / fRmL;
		//m_projMat._41 = 0.f;
		//m_projMat._12 = 0.f;
		//m_projMat._22 = 2.0f * m_fNearDepth / fTmB;
		//m_projMat._32 = -fTpB * fInvFmN;
		//m_projMat._42 = 0.f;			
		//m_projMat._13 = 0.f;
		//m_projMat._23 = 0.f;
		//m_projMat._33 = frustum.m_fFar * fInvFmN;
		//m_projMat._43 = -(frustum.m_fNear * frustum.m_fFar * fInvFmN);		
		//m_projMat._14 = 0.f;
		//m_projMat._24 = 0.f;
		//m_projMat._34 = 1.f;
		//m_projMat._44 = 0.f;
	}

}



int U2Dx9Renderer::LoadFont(const TCHAR* pcFace, int iSize, bool bBold, bool bItalic)
{
	
	uint32 loc;
	for(loc=0; loc < m_fontArray.Size(); ++loc)
	{
		if(!m_fontArray.GetElem(loc))
		{
			break;
		}
	}

	HDC hDC;
	hDC = ::GetDC(NULL);	
	int nLogPixelY = ::GetDeviceCaps( hDC, LOGPIXELSY );	
	ReleaseDC( NULL, hDC );
	
	
	int fontHeight = -MulDiv( iSize, nLogPixelY, 72 );

	LPD3DXFONT pD3DFont;
	DWORD dwWeight = (bBold ? FW_BOLD : FW_REGULAR);

	HRESULT hr = D3DXCreateFont( 
		m_pD3DDevice9, 
		fontHeight,					// Height
		0,					// Width
		dwWeight,				// Weight
		0,						// MpLevels
		(DWORD)bItalic,			// Italic
		DEFAULT_CHARSET,		// CharSet
		OUT_DEFAULT_PRECIS,		// OuputPrecision
		ANTIALIASED_QUALITY,	//Quality
		VARIABLE_PITCH,			// PitchAndFamily
		pcFace,					// FaceName
		&pD3DFont);				//  D3D Font
	U2ASSERT(SUCCEEDED(hr));


	if(loc == m_fontArray.Size())
	{
		m_fontArray.AddElem(pD3DFont);
	}
	else 
	{
		m_fontArray.SetElem(loc, pD3DFont);
	}
	
	return loc;
}

void U2Dx9Renderer::UnloadFont(int iFontID)
{
	if(1 <= iFontID && iFontID < (int)m_fontArray.Size())
	{
		SAFE_RELEASE(m_fontArray.GetElem(0));
	}
}


bool U2Dx9Renderer::SelectFont(int iFontID)
{
	if(0 <= iFontID && iFontID < (int)m_fontArray.Size())
	{
		if(m_fontArray.GetElem(iFontID))
		{
			m_iFontID = iFontID;
			return true;
		}
	}

	return false;	
}

LPD3DXFONT U2Dx9Renderer::GetD3DFont(int iFontID)
{
	if(0 <= iFontID && iFontID < (int)m_fontArray.Size())
	{
		return m_fontArray.GetElem(iFontID);
	}
	return NULL;
}



void U2Dx9Renderer::DrawFont(int x, int y, const D3DXCOLOR& Color, const TCHAR* szText)
{
	U2ASSERT(szText);
	if(!szText)
	{
		return;
	}

	RECT textRect;
	textRect.top =y;
	textRect.bottom = y;
	textRect.left = x;
	textRect.right = x;

	D3DCOLOR color = D3DCOLOR_COLORVALUE(Color.r, Color.g, Color.b, Color.a);

	HRESULT hr = m_fontArray.GetElem(m_iFontID)->DrawText(
		0,				// pSprite
		szText,			// Text to draw
		-1,				// Count
	&textRect,			// text rect to draw
	DT_LEFT | DT_BOTTOM | DT_CALCRECT,	
	Color);
	U2ASSERT(SUCCEEDED(hr));

	hr = m_fontArray.GetElem(m_iFontID)->DrawText(
		0,					// pSprite
		szText,				// Text to draw
		-1,					// Count
		&textRect,			// text rect to draw
		DT_LEFT | DT_BOTTOM ,	
		Color);
	U2ASSERT(SUCCEEDED(hr));
}

//-------------------------------------------------------------------------------------------------
bool U2Dx9Renderer::CreateRenderedTexture(unsigned short usWidth, unsigned short usHeight,		
										  D3DFORMAT pixelFormat)
{
	U2Dx9Texture* pTex = U2Dx9Texture::CreateRendered(usWidth, usHeight, this, pixelFormat);

	if(!pTex)
		return false;

	m_renderedTextures.Insert(pTex->GetName(), pTex);

	return true;
}

//-------------------------------------------------------------------------------------------------
void U2Dx9Renderer::SetScissorRect(const U2Rect<float>& rect)
{
	m_scissorRect = rect;
	UpdateScissorRect();
}

//-------------------------------------------------------------------------------------------------
void U2Dx9Renderer::UpdateScissorRect()
{
	float fW = GetCurrRenderTargets()->GetWidth(0);
	float fH = GetCurrRenderTargets()->GetHeight(0);

	RECT rect;
	rect.left = int(m_scissorRect.m_left * fW);
	rect.right = int(m_scissorRect.m_right * fW);
	rect.top = int(m_scissorRect.m_top * fH);
	rect.bottom = int(m_scissorRect.m_bottom * fH);
	HRESULT hr = m_pD3DDevice9->SetScissorRect(&rect);
	U2ASSERT(SUCCEEDED(hr));
}

//-------------------------------------------------------------------------------------------------
void U2Dx9Renderer::SetUserClipPlanes(U2Plane* plane)
{
	//if(!m_pRenderState)
	//	return;

	if(!plane)
	{
		//m_pRenderState->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
		m_pD3DDevice9->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);		
		return;
	}

	for(int i=0; i < 6; ++i)
		m_clipPlane[i] = plane[i];

	HRESULT hr;
	DWORD clipPlaneMask = 0;
	
	uint32 i;
	for(i=0; i < 6; ++i)
	{
		hr = m_pD3DDevice9->SetClipPlane(i, &(plane[i].m_vNormal.x));
		U2_DXTrace(hr, _T("SetClipPlane() failed"));
		clipPlaneMask |= (1 << i);
	}
	//m_pRenderState->SetRenderState(D3DRS_CLIPPLANEENABLE, clipPlaneMask);
	hr = m_pD3DDevice9->SetRenderState(D3DRS_CLIPPLANEENABLE, clipPlaneMask);
	//U2_DXTrace(hr, _T("Enable clip planes failed!"));

}


//-------------------------------------------------------------------------------------------------
void U2Dx9Renderer::SetupMesh(U2N2Mesh* pMesh)
{	
	HRESULT hr;

	if(pMesh)
	{		
		this->m_pCurrMesh = pMesh;		

		IDirect3DVertexBuffer9* pVB = pMesh->GetVB();
		//uint32 uiStride = pMesh->GetMeshData()->GetVertexStride(0) << 2;
		uint32 uiStride = pMesh->GetVertexStride();
		hr = this->GetD3DDevice()->SetStreamSource(0, pVB, 0, uiStride);
		U2_DXTrace(hr, _T("SetStreamSource() on Failed"));

		hr = this->GetD3DDevice()->SetVertexDeclaration(
			pMesh->GetMeshData()->GetVertexDeclaration());
		U2_DXTrace(hr, _T("SetVertexDeclaration() failed"));

		hr = this->GetD3DDevice()->SetIndices(pMesh->GetMeshData()->GetIB());
		U2_DXTrace(hr, _T("SetIndices() failed"));
	}
	else 
	{
		hr = this->GetD3DDevice()->SetStreamSource(0, 0, 0, 0);
		U2_DXTrace(hr, _T("SetStreamSource() on Failed"));

		hr = this->GetD3DDevice()->SetIndices(0);
		U2_DXTrace(hr, _T("SetIndices() failed"));
	}	
}

//-------------------------------------------------------------------------------------------------
void U2Dx9Renderer::SetupMesh(U2N2Mesh* pVBMesh, U2N2Mesh* pIBMesh)
{	
	HRESULT hr;

	if(pVBMesh)
	{		
		this->m_pCurrMesh = pVBMesh;		

		IDirect3DVertexBuffer9* pVB = pVBMesh->GetVB();
		//uint32 uiStride = pMesh->GetMeshData()->GetVertexStride(0) << 2;
		uint32 uiStride = pVBMesh->GetVertexStride();
		hr = this->GetD3DDevice()->SetStreamSource(0, pVB, 0, uiStride);
		U2_DXTrace(hr, _T("SetStreamSource() on Failed"));

		hr = this->GetD3DDevice()->SetVertexDeclaration(
			pVBMesh->GetMeshData()->GetVertexDeclaration());
		U2_DXTrace(hr, _T("SetVertexDeclaration() failed"));

		hr = this->GetD3DDevice()->SetIndices(pIBMesh->GetMeshData()->GetIB());
		U2_DXTrace(hr, _T("SetIndices() failed"));
	}
	else 
	{
		hr = this->GetD3DDevice()->SetStreamSource(0, 0, 0, 0);
		U2_DXTrace(hr, _T("SetStreamSource() on Failed"));

		hr = this->GetD3DDevice()->SetIndices(0);
		U2_DXTrace(hr, _T("SetIndices() failed"));
	}	
}


//-------------------------------------------------------------------------------------------------
void U2Dx9Renderer::DrawIndexedPrimitive(D3DPRIMITIVETYPE ePrimType)
{

	D3DPRIMITIVETYPE d3dPrimType = ePrimType;

	this->GetCurrEffectShader()->CommitChanges();

	HRESULT hr = S_OK;
	switch(ePrimType)
	{
	case D3DPT_TRIANGLELIST:
		hr = this->GetD3DDevice()->DrawIndexedPrimitive(
			d3dPrimType, 0, m_iVertexRangeFirst, m_iNumVertexRange,
			m_iIndexRangeFirst, m_iNumIndexRange / 3);
		break;
	case D3DPT_TRIANGLESTRIP:
		hr = this->GetD3DDevice()->DrawIndexedPrimitive(
			d3dPrimType, 0, m_iVertexRangeFirst, m_iNumVertexRange,
			m_iIndexRangeFirst, m_iNumIndexRange - 2);
		break;

	}	 

	//U2_DXTrace(hr, _T("DrawIndexedPrimitive() failed"));
}

//-------------------------------------------------------------------------------------------------
void U2Dx9Renderer::DrawPrimitive(D3DPRIMITIVETYPE ePrimType)
{

	D3DPRIMITIVETYPE d3dPrimType = ePrimType;

	this->GetCurrEffectShader()->CommitChanges();

	HRESULT hr = this->GetD3DDevice()->DrawPrimitive(
		d3dPrimType, m_iVertexRangeFirst, m_iNumVertexRange	);
	
}
