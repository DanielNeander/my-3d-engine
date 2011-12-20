/************************************************************************
module	:	U2Dx9FramePhase
Author	:	Yun sangyong
Desc	:	Gamebryo 기반..  Renderer 
			Create Cubemap ..등 추가 리소스 관리 필요..
			현재 쉐이더는 Asm 쉐이더와 Fx 쉐이더를 사용한다.. 
			구분하기 쉽게 cpp 파일을 나눌 필요가 있다.
			2010-03-08
			Default 좌표계 : Left Hand 좌표계
************************************************************************/
#pragma once 
#ifndef	U2_DX9_RENDERER_H
#define	U2_DX9_RENDERER_H


#include <d3dx9.h>



#include <U2_3D/Src/U2_3DLibType.h>
#include <U2Lib/Src/U2RefObject.h>
#include <U2Lib/Src/U2SmartPtr.h>
#include <U2Lib/Src/U2THashTable.h>
#include <U2Lib/Src/U2TStringHashTable.h>
#include <U2_3D/Src/Main/U2Rect.h>
#include <U2_3D/Src/dx9/U2RenderTarget.h>


class U2Dx9LightMgr;
class U2Dx9TextureMgr;
class U2Dx9RenderStateMgr;
class U2Dx9RenderTarget;
class U2Dx9VertexBufferMgr;
class U2Dx9IndexBufferMgr;
class U2TriList;
class U2TriStrip;
class U2PolyLine;
class U2Dx9VBGroupMgr;
class U2VisibleSet;
class U2RenderTargets;
class U2D3DXEffectShader;
U2SmartPointer(U2Camera);
class U2ShaderEffect;
class U2VertexProgram;
class U2PixelProgram;
class U2VBGroup;
class U2N2Mesh;




enum { U2_DX9_RENDERID = 1 };

#undef SelectFont

U2SmartPointer(U2Dx9Renderer);

class U2_3D U2Dx9Renderer : public U2Object
{

public:
	// Flags for creation options.
	enum FlagType
	{
		USE_NOFLAGS								= 0x00000000,
		USE_MANUALDEPTHSTENCIL           = 0x00000001,
		USE_FULLSCREEN							= 0x00000004,
		USE_STENCIL								= 0x00000008,
		USE_16BITBUFFERS						 = 0x00000010,
		USE_MULTITHREADED					 = 0x00000020,
		USE_FPU_PRESERVE						= 0x00000040
	};


	enum DeviceDesc
	{
		DEVDESC_PURE,
		DEVDESC_HAL_HWVERTEX,
		DEVDESC_HAL_MIXEDVERTEX,
		DEVDESC_HAL_SWVERTEX,
		DEVDESC_REF,
		DEVDESC_REF_HWVERTEX,
		DEVDESC_REF_MIXEDVERTEX,
		DEVDESC_NUM
	};

	//  DX9 supported frame buffer formats
	enum FrameBufferFormat
	{
		FBFMT_UNKNOWN = 0,
		FBFMT_R8G8B8,
		FBFMT_A8R8G8B8,
		FBFMT_X8R8G8B8,
		FBFMT_R5G6B5,
		FBFMT_X1R5G5B5,
		FBFMT_A1R5G5B5,
		FBFMT_A4R4G4B4,
		FBFMT_R3G3B2,
		FBFMT_A8,
		FBFMT_A8R3G3B2,
		FBFMT_X4R4G4B4,
		FBFMT_R16F,
		FBFMT_G16R16F,
		FBFMT_A16B16G16R16F,
		FBFMT_R32F,
		FBFMT_G32R32F,
		FBFMT_A32B32G32R32F,
		FBFMT_NUM
	};

	//  DX9 supported depth/stencil buffer formats
	enum DepthStencilFormat
	{
		DSFMT_UNKNOWN      =   0,
		DSFMT_D16_LOCKABLE =  70,
		DSFMT_D32          =  71,
		DSFMT_D15S1        =  73,
		DSFMT_D24S8        =  75,
		DSFMT_D16          =  80,
		DSFMT_D24X8        =  77,
		DSFMT_D24X4S4      =  79,
	};

	// Presentation Interval
	enum PresentationInterval
	{
		PRESENT_INTERVAL_IMMEDIATE = 0,
		PRESENT_INTERVAL_ONE = 1,
		PRESENT_INTERVAL_TWO = 2,
		PRESENT_INTERVAL_THREE = 3,
		PRESENT_INTERVAL_FOUR = 4,
		PRESENT_INTERVAL_NUM
	};

	// Swap Effect
	enum SwapEffect
	{
		SWAPEFFECT_DEFAULT,
		SWAPEFFECT_DISCARD,
		SWAPEFFECT_FLIP,
		SWAPEFFECT_COPY,
		SWAPEFFECT_NUM
	};

	enum FramebufferMode
	{
		FBMODE_DEFAULT,
		FBMODE_LOCKABLE,
		FBMODE_MULTISAMPLES_2           = 0x00010000,
		FBMODE_MULTISAMPLES_3           = 0x00020000,
		FBMODE_MULTISAMPLES_4           = 0x00030000,
		FBMODE_MULTISAMPLES_5           = 0x00040000,
		FBMODE_MULTISAMPLES_6           = 0x00050000,
		FBMODE_MULTISAMPLES_7           = 0x00060000,
		FBMODE_MULTISAMPLES_8           = 0x00070000,
		FBMODE_MULTISAMPLES_9           = 0x00080000,
		FBMODE_MULTISAMPLES_10          = 0x00090000,
		FBMODE_MULTISAMPLES_11          = 0x000a0000,
		FBMODE_MULTISAMPLES_12          = 0x000b0000,
		FBMODE_MULTISAMPLES_13          = 0x000c0000,
		FBMODE_MULTISAMPLES_14          = 0x000d0000,
		FBMODE_MULTISAMPLES_15          = 0x000e0000,
		FBMODE_MULTISAMPLES_16          = 0x000f0000,
		FBMODE_MULTISAMPLES_NONMASKABLE = 0x80000000,
		FBMODE_QUALITY_MASK             = 0x0000FFFF,
		FBMODE_NUM = 18
	};

	// Refresh Rate
	enum RefreshRate
	{
		REFRESHRATE_DEFAULT   = 0
	};

	enum TexFormat
	{
		TEX_RGB555,
		TEX_RGB565,
		TEX_RGB888,
		TEX_RGBA5551,
		TEX_RGBA4444,
		TEX_RGBA8888,
		TEX_PAL8,
		TEX_PALA8,
		TEX_DXT1,
		TEX_DXT3,
		TEX_DXT5,
		TEX_BUMP88,
		TEX_BUMPLUMA556,
		TEX_BUMPLUMA888,
		TEX_L8,
		TEX_A8,
		TEX_R16F,
		TEX_RG32F,
		TEX_RGBA64F,
		TEX_R32F,
		TEX_RG64F,
		TEX_RGBA128F,
		TEX_DEFAULT,
		TEX_NUM         = TEX_DEFAULT
	};

	enum TexUsage
	{
		TEXUSE_TEX,
		TEXUSE_RENDERED_TEX,
		TEXUSE_CUBE,
		TEXUSE_RENDERED_CUBE,
		TEXUSE_NUM
	};

	enum FramebufferStatus
	{
		FBSTATUS_OK,
		FBSTATUS_NOT_FOUND,
		FBSTATUS_NEEDS_RESET
	};

	enum RecreateStatus
	{
		RECREATESTATUS_FAILED,
		RECREATESTATUS_RESTORED,
		RECREATESTATUS_OK,
	};

	enum
	{
		HW_BONE_LIMIT = 4
	};

	enum ClearFlags
	{
		CLEAR_BACKBUFFER = 0x1,
		CLEAR_STENCIL = 0x2,
		CLEAR_ZBUFFER = 0x4,
		CLEAR_NONE = 0, 
		CLEAR_ALL = CLEAR_BACKBUFFER | CLEAR_STENCIL | CLEAR_ZBUFFER
	}; 

	// Frame이 제대로 업데이트 되는지 확인... 
	// Direct3D 장치객체의 BeginScene, EndScene Pair의 확실한 보장을 위해 체크
	// 내가 현재 텍스쳐에 그리는지, 백버퍼에 그리는지 체크
	enum FrameState
	{
		FS_OUTSIDE_FRAME,
		FS_INSIDE_FRAME,
		FS_INSIDE_OFFSCREEN_FRAME,
		FS_WAITING_FOR_DISPLAY
	};

	enum D3DResourceType
	{
		D3DRT_GENERIC,
		D3DRT_VERTEXBUFFER,
		D3DRT_INDEXBUFFER,
		D3DRT_TEXTURE,
		D3DRT_SWAPCHAIN,
		D3DRT_DEVICE,
		D3DRT_VERTEXDECL,

		D3DRT_COUNT
	};

	// start
	// moved date : 2010-03-08 (U2SceneMgr -> U2Dx9Renderer)
	enum TransformType
	{
		TMT_WORLD = 0,              ///< model -> world matrix (read/write)
		TMT_VIEW,                   ///< world -> view matrix (read/write)
		TMT_PROJ,					///< view  -> projection matrix (read/write)
		TMT_INVWORLD,               ///< world -> model matrix (read only)
		TMT_INVVIEW,					///< view  -> world matrix (read only)
		TMT_WORLDVIEW,              ///< model -> view matrix (read only)
		TMT_INVWORLDVIEW,           ///< view -> model matrix (read only)
		TMT_WORLDVIEWPROJ,    ///< model -> projection matrix (read only)
		TMT_VIEWPROJ,         ///< current view * projection matrix
		TMT_TEXTURE0,               ///< texture transform for layer 0 (read/write)
		TMT_TEXTURE1,
		TMT_TEXTURE2,
		TMT_TEXTURE3,
		TMT_LIGHT,                  ///< the current light's matrix in world space
		TMT_WORLDLight,
		TMT_INVWORLDLight,
		TMT_SHADOWPROJ,

		MAX_TRANSFORM_TYPES,
	};

	enum Hint 
	{
		MODEL_VIEW_PROJ_ONLY = (1 << 0),	 // only update the ModelViewProjection matrix in shaders
		COUNT_STATS = (1 << 1),		// statics counting currently active
	};

	enum 
	{
		MAX_TRANSFORM_STACK_DEPTH = 4,
	};

	/// 현재 프레임에 실제 사용되는 EffectShader Access
	void SetCurrEffectShader(U2D3DXEffectShader* pShd);
	U2D3DXEffectShader* GetCurrEffectShader() const;

	void SetTransform(TransformType eType, const D3DXMATRIX& mat);
	void GenerateTansform(TransformType eType, const D3DXMATRIX& mat);	
	D3DXMATRIX& GetTransform(TransformType eType);
	void PushTransform(TransformType eType, const D3DXMATRIX& mat);
	const D3DXMATRIX& PopTransform(TransformType eType);

	/// get a vector4 of an int shadowLightIndex to set as shader parameter
	static const D3DXVECTOR4 GetShadowLightIndexVector(int shadowLightIndex, float value);


	void SetHint(Hint hint, bool enable);
	/// get a render hint
	bool GetHint(Hint hint) const;

	U2D3DXEffectShaderPtr m_spSharedShader;	// shader file name : "shared.fx"
	U2D3DXEffectShaderPtr m_spShapeShader;	// shader file name : "shapes.fx"

	int m_aiTMTopOfStack[MAX_TRANSFORM_TYPES];	
	D3DXMATRIX m_aTM[MAX_TRANSFORM_TYPES];
	D3DXMATRIX m_aaTMStack[MAX_TRANSFORM_TYPES][MAX_TRANSFORM_STACK_DEPTH];

	// end
	
	
	virtual	~U2Dx9Renderer();

	static void Terminate();

	// Renderer creation
	static U2Dx9Renderer*							Create(unsigned int uiWidth, unsigned int uiHeight,
										unsigned int uiUseFlags, HWND kWndDevice, 
										HWND kWndFocus, bool useFixedFuncRS = false, unsigned int uiAdapter = D3DADAPTER_DEFAULT, 
										DeviceDesc eDesc = DEVDESC_PURE,
										FrameBufferFormat eFBFormat = FBFMT_UNKNOWN, 
										DepthStencilFormat eDSFormat = DSFMT_UNKNOWN, 
										PresentationInterval ePresentationInterval = PRESENT_INTERVAL_IMMEDIATE,
										SwapEffect eSwapEffect = SWAPEFFECT_DEFAULT,
										unsigned int uiFBMode = FBMODE_DEFAULT,
										unsigned int uiBackBufferCount = 1, 
										unsigned int uiRefreshRate = REFRESHRATE_DEFAULT);

	void											GetCreationParameters(unsigned int& uiWidth, unsigned int& uiHeight,
										unsigned int& uiUseFlags, HWND& kWndDevice, 
										HWND& kWndFocus, unsigned int& uiAdapter, DeviceDesc& eDesc, 
										FrameBufferFormat& eFBFormat, DepthStencilFormat& eDSFormat, 
										PresentationInterval& ePresentationInterval, SwapEffect& eSwapEffect,
										unsigned int& uiFBMode, unsigned int& uiBackBufferCount, 
										unsigned int& uiRefreshRate) const;

	// Renderer recreation
	RecreateStatus									Recreate(unsigned int uiWidth, unsigned int uiHeight,
										unsigned int uiUseFlags, HWND kWndDevice = 0, 
										FrameBufferFormat eFBFormat = FBFMT_UNKNOWN, 
										DepthStencilFormat eDSFormat = DSFMT_UNKNOWN, 
										PresentationInterval ePresentationInterval = PRESENT_INTERVAL_IMMEDIATE,
										SwapEffect eSwapEffect = SWAPEFFECT_DEFAULT,
										unsigned int uiFBMode = FBMODE_DEFAULT,
										unsigned int uiBackBufferCount = 1, 
										unsigned int uiRefreshRate = REFRESHRATE_DEFAULT);

	RecreateStatus									Recreate(unsigned int uiWidth, unsigned int uiHeight);

	bool											Recreate();


	// Respond to D3D device reset
	inline unsigned int								GetResetCounter() const;
	inline bool										IsDeviceLost() const;

	// Callbacks for improved response to D3D device reset
	typedef bool									(*RESETNOTIFYFUNC)(bool bBeforeReset, void* pvData);
	typedef bool									 (*LOSTDEVICENOTIFYFUNC)(void* pvData);

	// Handle Shader Func

	// Access D3D interfaces
	inline LPDIRECT3DDEVICE9						GetD3DDevice() const;
	inline unsigned int								GetAdapter() const;
	inline D3DDEVTYPE								GetDevType() const;

	inline TCHAR* GetDevTypeString() const;
	inline TCHAR* GetBehaviorString() const;



	//inline D3DPRESENT_PARAMETERS* GetPresentParams(
	//	const NiRenderTargetGroup* pkTarget = NULL) const;
	inline D3DFORMAT								GetAdapterFormat() const;
	inline const D3DCAPS9*							GetDeviceCaps() const;

	// Query D3D device capabilities
	inline unsigned int								GetMaxStreams() const;
	inline unsigned int								GetPixelShaderVersion() const;
	inline unsigned int								GetVertexShaderVersion() const;
	inline bool										GetMipmapCubeMaps() const;
	inline bool										IsDynamicTexturesCapable() const;
	bool											GetMRTPostPixelShaderBlendingCapability() const;
	bool											GetFormatPostPixelShaderBlendingCapability(
													D3DFORMAT eFormat) const;

	// Software Vertex Processing Related
	bool											GetSWVertexCapable() const;
	bool											GetSWVertexSwitchable() const;



	static LPDIRECT3D9								GetDirect3D();

	// Access Dx9 Resources(Light, Texture, Render State) Manager



	// Translate between D3D and Gamebryo values
	static unsigned char							GetD3DFormatSize(D3DFORMAT eFormat);
	static D3DFORMAT								GetD3DFormat(FrameBufferFormat eFBFormat);
	static D3DFORMAT								GetD3DAdapterFormat(FrameBufferFormat eFBFormat);	
	static D3DFORMAT								GetD3DFormat(DepthStencilFormat eDSFormat);	
	static D3DMULTISAMPLE_TYPE						GetMultiSampleType(unsigned int uiMode);
	static unsigned int								GetMultiSampleQuality(unsigned int uiMode);
	static D3DSWAPEFFECT							GetSwapEffect(SwapEffect eMode, 
															unsigned int uiFBMode, unsigned int uiUseFlags);
	static unsigned int								GetRefreshRate(unsigned int uiRefreshRateHz);
	static unsigned int								GetPresentInterval(
															PresentationInterval ePresentationInterval);

	static unsigned int								GetMaxActiveLights();
	
	// String describing renderer
	const TCHAR*									GetDriverInfo() const;
	// Renderer capabilities
	unsigned int									GetFlags() const;

	void SetDepthClear(const float fZClear);
	float GetDepthClear() const;	
	void SetBackgroundColor(const D3DXCOLOR& kColor);
	void GetBackgroundColor(D3DXCOLOR& kColor) const;
	void SetStencilClear(unsigned int uiClear);
	unsigned int GetStencilClear() const;

	
	// Render target Clearing Operations 
	// Depth Buffer Clear, Set Background Color, Stencil Buttfer Clear

	// Render target	Access And Management
	// Access Default RenderTarget, DepthStencilBuffer
	bool BeginScene();
	bool EndScene();


	U2RenderTargets* GetDefaultRenderTarget() const;


	bool BeginRenderTarget(U2RenderTargets* pTarget, unsigned int uClearMode );
	bool BeginDefaultRenderTarget(unsigned int uClearMode );

	bool EndRenderTarget();

	void ClearBuffers(unsigned int uClearMode);
	bool DisplayBackBuffer();



	//------------------------------------------------------------------------------
	/**
	Updates the current scissor rectangle. This must be called from
	SetScissorRect() and SetRenderTarget() (since SetRenderTarget() resets
	the current scissor rectangle).
	*/
	
	void SetUserClipPlanes(U2Plane* plane);
	void SetScissorRect(const U2Rect<float>& rect);
	void UpdateScissorRect();


	 static U2Dx9Renderer*							GetRenderer();	 


	 //inline const D3DMATRIX&						GetD3DMat() const;
	 //inline const D3DMATRIX&						GetD3DView() const;
	 //inline const D3DMATRIX&						GetD3DProj() const;
	 //inline const D3DMATRIX&						GetInvView() const;
	 
	 U2Dx9VertexBufferMgr*							GetVBMgr() const;
	 U2Dx9IndexBufferMgr*							GetIBMgr() const;
	 U2Dx9LightMgr*									GetLightMgr() const;

	 U2Camera*										GetCurrCamera() const;


	 U2RenderTargets*								GetCurrRenderTargets() const;

	 // Threading information
	 inline unsigned long							GetDeviceThreadID() const;

	 // 장치 타입 설정
	 static	bool									SelectD3DDevicetype(DeviceDesc eDesc, 
												D3DDEVTYPE& EdEVtYPE, uint32& uBehaviorFlags);


	 bool											IsTextureFormatOk( D3DFORMAT pixelFormat, 
													DWORD dwUsage,
												D3DRESOURCETYPE eResType );

	 unsigned int									GetMaxRenderTargets() const;
	 bool											GetIndependentBufferBitDepths() const;

	bool											IsDSBufferCompatible(U2Dx9FrameBuffer* pBackBuffer, 
												U2Dx9FrameBuffer* pDSBuffer);


	//static void SetMaxTextureBlendStages(
	//	unsigned int uiMaxTextureBlendStages);
	//static void SetMaxSimultaneousTextures(
	//	unsigned int uiMaxSimultaneousTextures);
	//static void SetMaxSamplers(unsigned int uiMaxSamplers);

	static uint32									GetMaxTextureBlendStages();
	//	unsigned int uiMaxTextureBlendStages);
	static uint32									GetMaxSimultaneousTextures();
	//	unsigned int uiMaxSimultaneousTextures);
	static uint32									GetMaxSamplers();

	inline unsigned int								GetProjectedTextureFlags();


	// Make View Matrix And Set
	// Modified 2010-03-10
	// 기존의 SetCamera를 SetCameraData로 변경
	void SetCameraData(U2Camera* pCam);	

	// Added 2010-03-08 
	// SetCamera에서 Projection, Viewport 세팅을 한 번에 하는 것을 분리한다.
	void SetCamera(U2Camera* pCam);
	void SetViewport(U2Camera* pCam);
	void SetProjectionTM(U2Camera* pCam);



	void DrawScene(U2VisibleSet& visibleSet);

	void RenderLines(U2PolyLine* pLines);
	void RenderTriStrip(U2TriStrip* pStrip);
	void RenderTriList(U2TriList* pTriListMesh);

	void DrawLines(U2PolyLine* line, U2ShaderEffect* pEffect, bool &bPrimaryEffect);
	void DrawPrimitves(U2Mesh* pMesh, U2MeshData *pMeshData, 
		U2ShaderEffect* pEffect,bool& bPrimaryEffect);

	bool PackMeshData(U2MeshData* pModelData, 
		const U2VertexAttributes& inputAttr, 
		const U2VertexAttributes& outputAttr);

	void EnableVertexProgram(U2VertexProgram* pProgram);

	void EnableTexture(U2Dx9BaseTexture* pTex);

	enum { LineVertexFVF = D3DFVF_XYZ };
	struct LineVertex : public U2MemObj
	{
		float x, y, z;
	};

	enum { TexturedLineFVF = D3DFVF_XYZ | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEX1 };
	struct TexturedLineVertex : public U2MemObj
	{
	
		float x, y, z;
		float tu, tv;
	};

	enum { ColoredLineFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE };
	struct ColoredLineVertex : public U2MemObj
	{
		float x,y,z;
		D3DCOLOR color;
	};

	enum { BaseVertexFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | 
	D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEX1 };
	struct BaseVertex : public U2MemObj
	{
		float x, y, z;
		D3DCOLOR color;
		float tu, tv;		
	};

	enum { BaseNVertexFVF = D3DFVF_XYZ | D3DFVF_NORMAL | 
	D3DFVF_DIFFUSE | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEX1 };
	class BaseNVertex : public U2MemObj
	{
		float x, y, z;
		float nx, ny, nz;
		D3DCOLOR color;
		float tu, tv;
	};

	// Font 
	int LoadFont(const TCHAR* pcFace, int iSize, bool bBold = false, bool bItalic = false);
	void UnloadFont(int iFontID);
	bool SelectFont(int iFontID);
	void DrawFont(int x, int y, const D3DXCOLOR& Color, const TCHAR* szText);
	LPD3DXFONT GetD3DFont(int iFontID);

	ID3DXEffectPool*										GetD3DEffectPool() const;

	const D3DXMATRIXA16& GetProjMat() const;
	const D3DXMATRIXA16& GetViewMat() const;
	const D3DXMATRIXA16& GetInvViewMat() const;



	// 2010/06/05 
	// Moved from U2SceneMgr
	void SetupMesh(U2N2Mesh* pMesh);
	void SetupMesh(U2N2Mesh* pVBMesh, U2N2Mesh* pIBMesh);
	void SetVertexRange(int firstVertex, int numVertices);
	void SetIndexRange(int firstIdx, int numIndices);
	void DrawIndexedPrimitive(D3DPRIMITIVETYPE ePrimType);
	void DrawPrimitive(D3DPRIMITIVETYPE ePrimType);

	
private:
	U2Dx9Renderer();

	bool													Initialize(unsigned int uiWidth, unsigned int uiHeight,
															unsigned int uiUseFlags, HWND kWndDevice, 
															HWND kWndFocus, bool useFixedFuncRS, unsigned int uiAdapter, DeviceDesc eDesc,
															FrameBufferFormat eFBFormat, DepthStencilFormat eDSFormat, 
															PresentationInterval ePresentationInterval, SwapEffect eSwapEffect,
															unsigned int uiFBMode, unsigned int uiBackBufferCount,
															unsigned int uiRefreshRate);

	bool													InitializeRendererFromDevice(
															D3DPRESENT_PARAMETERS kD3DPresentParams);

	void													InitDefault();
	static bool												IsD3D9Created();
	static int												CreateDirect3D9();

	// LostDevice handler
	bool													RestoreLostDevice();
	// 'Check' functions
	bool													InitializePresentParams(HWND kWndDevice, unsigned int uiWidth,
														unsigned int uiHeight, unsigned int uiUseFlags, 
														unsigned int uiFBMode, 
														FrameBufferFormat eFBFormat, DepthStencilFormat eDSFormat,
														unsigned int uiBackBufferCount, SwapEffect eSwapEffect, 
														unsigned int uiRefreshRate, 
														PresentationInterval ePresentationInterval,
														D3DPRESENT_PARAMETERS& kD3DPresentParams) const;

	bool													InitDeviceCaps(
															const D3DPRESENT_PARAMETERS& kPresentParams);
		

	bool												CreateRenderedTexture(unsigned short															usWidth, unsigned short usHeight,																D3DFORMAT pixelFormat);
	//bool CreateCubeMap();
	//bool CreateRenderedCubeMap();
	//bool CreateDynamicTexture();
	




	// ShaderConstant Function 
	void GetTransform (const D3DXMATRIX& rkMat, int iOperation,
		float* pafData);

	void SetConstantWMatrix (int iOperation, float* afData);
	void SetConstantVMatrix (int iOperation, float* afData);
	void SetConstantPMatrix (int iOperation, float* afData);
	void SetConstantWVMatrix (int iOperation, float* afData);
	void SetConstantVPMatrix (int iOperation, float* afData);
	void SetConstantWVPMatrix (int iOperation, float* afData);

	void SetConstantMaterialEmissive (int, float* afData);
	void SetConstantMaterialAmbient (int, float* afData);
	void SetConstantMaterialDiffuse (int, float* afData);
	void SetConstantMaterialSpecular (int, float* afData);
	void SetConstantCameraModelPosition (int, float* afData);
	void SetConstantCameraModelDirection (int, float* afData);
	void SetConstantCameraModelUp (int, float* afData);
	void SetConstantCameraModelRight (int, float* afData);
	void SetConstantCameraWorldPosition (int, float* afData);
	void SetConstantCameraWorldDirection (int, float* afData);
	void SetConstantCameraWorldUp (int, float* afData);
	void SetConstantCameraWorldRight (int, float* afData);
	void SetConstantProjectorModelPosition (int, float* afData);
	void SetConstantProjectorModelDirection (int, float* afData);
	void SetConstantProjectorModelUp (int, float* afData);
	void SetConstantProjectorModelRight (int, float* afData);
	void SetConstantProjectorWorldPosition (int, float* afData);
	void SetConstantProjectorWorldDirection (int, float* afData);
	void SetConstantProjectorWorldUp (int, float* afData);
	void SetConstantProjectorWorldRight (int, float* afData);
	void SetConstantProjectorMatrix (int, float* afData);


	// These functions set the light state.  The index iLight is between 0
	// and 7 (eight lights are currently supported).
	void SetConstantLightModelPosition (int iLight, float* afData);
	void SetConstantLightModelDirection (int iLight, float* afData);
	void SetConstantLightWorldPosition (int iLight, float* afData);
	void SetConstantLightWorldDirection (int iLight, float* afData);
	void SetConstantLightAmbient (int iLight, float* afData);
	void SetConstantLightDiffuse (int iLight, float* afData);
	void SetConstantLightSpecular (int iLight, float* afData);
	void SetConstantLightSpotCutoff (int iLight, float* afData);
	void SetConstantLightAttenuation (int iLight, float* afData);

	enum { SC_QUANTITY = 37 };
	typedef void (U2Dx9Renderer::*SetConstantFunc)(int, float*);
	static SetConstantFunc ms_aSCFun[SC_QUANTITY];
	void SetShaderConstant(U2ShaderConstant::ShaderConstantMapping eSCM, 
		float* pafData);

	U2Light* GetLight (int i);
	void SetLight (int i, U2Light* pkLight);

	


	void EnablePixelProgram(U2PixelProgram* pProgram);
	void DisableTexture();
	
	bool CheckFrameState(const char* pcCallingFunction,
		FrameState eExpectedValue) const;

	
	
	static	 LPDIRECT3D9							ms_pD3D9;
	LPDIRECT3DDEVICE9								m_pD3DDevice9;
	D3DCAPS9										m_d3dCaps9;

	HWND											m_hDevice;
	HWND											m_hFocus;
	mutable TCHAR m_acDriverDesc[512];

	unsigned int									m_uAdapter;
	D3DDEVTYPE										m_eD3DDevType;
	unsigned int									m_uBehaviorFlags;
	D3DFORMAT										m_eD3DAdapterFormat;
	bool m_bSWVertexCapable;
	bool m_bSWVertexSwitchable;


	unsigned int									m_uBG;
	float											m_fZClear;
	unsigned int									m_uStencilClear;
	unsigned int									m_uRenderFlags;
	mutable TCHAR									m_acBehavior[32];


	D3DXVECTOR3										m_camRight;
	D3DXVECTOR3										m_camUp;
	D3DXVECTOR3										m_modelCamRight;
	D3DXVECTOR3										m_modelCamUp;

	float											m_fNearDepth, m_fDepthRange;
	D3DXMATRIX										m_matIdentity;
	D3DVIEWPORT9									m_d3dPort;

	unsigned int									m_uHWBones;
	unsigned int									m_uMaxStreams;
	unsigned int									m_uMaxPixelShaderVer;
	unsigned int									m_uMaxVertexShaderVer;
	bool m_bMipmapCubeMaps;
	bool m_bDynamicTexturesCapable;
	bool m_bIndependentBitDepths;
	bool m_bMRTPostPixelShaderBlending;

	unsigned int									m_uiResetCount;
	bool											m_bDeviceLost;

	unsigned int m_uMaxNumRenderTargets;

	enum 
	{
		HW_MAX_ANISOTROPY	= 16
	};

	unsigned short									m_usHwMaxAnisotropy;


	U2Dx9RenderStateMgr*							m_pRenderState;
	U2Dx9TextureMgr*								m_pTexMgr;
	U2Dx9LightMgr*									m_pLightMgr;
	U2Dx9VertexBufferMgr* m_pVBMgr;
	U2Dx9IndexBufferMgr* m_pIBMgr;
	

	U2RenderTargets*									m_pCurrRenderTarget;
	U2RenderTargetPtr									m_spDefaultRenderTarget;
	U2THashTable<HWND, U2RenderTargetPtr>*			m_pRenderTargets;
	U2TStringHashTable<U2Dx9Texture*>				m_renderedTextures;
	U2TStringHashTable<U2Dx9Texture*>				m_renderedCubeTextures;
	U2TStringHashTable<U2Dx9Texture*>				m_dynamicTextures;

	U2TPointerList<U2Dx9FrameBuffer*>				m_frameBuffersToPresent;	

	U2TPointerList<U2Shader*>						m_shaders;

	// Temporary members - used to speed up code
	//D3DXALIGNEDMATRIX m_kD3DMat;
	D3DXMATRIXA16									m_viewMat;
	D3DXMATRIXA16									m_projMat;
	D3DXMATRIXA16									m_invView;

	D3DXMATRIX m_worldMat;



	// Screen texture members
	D3DXVECTOR2*									m_pScreenTextureVerts;
	D3DXCOLOR*										m_pScreenTextureColors;
	D3DXVECTOR2*									m_pScreenTextureTexCoords;
	unsigned short									m_usNumScreenTextureVerts;
	unsigned short*									m_pusScreenTextureIndices;
	unsigned int									m_uiNumScreenTextureIndices;

	unsigned int									m_uBgColor;

	// Creation parameters
	unsigned int 									m_uInitWidth;
	unsigned int 									m_uInitHeight;
	unsigned int 									m_uInitUseFlags;
	HWND											m_hInitDevice;
	HWND											m_hInitFocus;
	unsigned int									m_uInitAdapter;
	DeviceDesc										m_eInitDesc;
	FrameBufferFormat								m_eInitFBFormat;
	DepthStencilFormat 								m_eInitDSFormat;
	PresentationInterval							m_eInitPresentationInterval;
	SwapEffect 										m_eInitSwapEffect;
	unsigned int 									m_uInitFBMode;
	unsigned int 									m_uInitBackBufferCnt;
	unsigned int 									m_uInitRefreshRate;

	unsigned int									m_uNumActiveSamplers;	
	uint32											m_uCurrSampler;
	U2ShaderSamplerInfo**							m_ppActiveSamplers;

	U2RenderStateCollection*						m_pCurrRSC;

	U2CameraPtr										m_spCamera;

	// Proejctor for various Effects such as Projected Texture and Shadow map
	U2CameraPtr										m_spProjCamera;

	U2Mesh*											m_pCurrMesh;

	U2LightPtr*										m_aspLights;
	
	// Device thread ID
	unsigned long									m_uDeviceThreadId;	// thread id

	static U2Dx9Renderer*							ms_pRenderer;		// singleton

	U2PrimitiveVec<LPD3DXFONT>						m_fontArray;		// LPD3DXFONT Array
	int												m_iFontID;			// Font ID

	ID3DXEffectPool*								m_pD3DEffectPool;	// D3DEffect Pool

	// Light 
	U2Rect<float>									m_scissorRect;	// Scissor Rect
	U2Plane											m_clipPlane[6];
	unsigned int									m_uiMaxClipPlanes;

	bool m_bWvpMatOnly;		// Update Only World , View, Proj Matrix ?
	int m_iHints;


	U2D3DXEffectShader* m_pCurrShader; 

	
	int m_iVertexRangeFirst, m_iNumVertexRange;
	int m_iIndexRangeFirst, m_iNumIndexRange;
	


	
	//  Max number of blending stages supported.
	//  Should correspond to the number of pixel shader instructions allowed.
	static unsigned int ms_uiMaxTextureBlendStages;
	//  Max number of textures that can be bound to the blending stages.
	//  Should correspond to the number of texture registers supported by the
	//  pixel shaders.
	static unsigned int ms_uiMaxSimultaneousTextures;
	//  Max number of samplers supported.
	//  Can only be greater than ms_uiMaxTextureBlendStages under DX9 when
	//  PS2.0 (or greater) hardware is supported.
	static unsigned int ms_uiMaxSamplers;

	static unsigned int ms_uMaxActiveLights;

	static unsigned int ms_uProjectedTextureFlags; //  Projected flags

	static DWORD ms_adwTexMinFilter[];
	static DWORD ms_adwTexMipFilter[];
	static DWORD ms_adwTexWrapMode[];

	friend class U2TextureEffect;	
	friend class U2LightEffect;
	friend class U2ShaderEffect;
	friend class U2Dx9FxShaderEffect;	
	friend class U2SceneMgr;
	
	FrameState m_eFrameState;
};


#define		ASSERT_D3DDEVICE_THREAD
			
#include "U2Dx9Renderer.inl"





#endif


