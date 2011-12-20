/************************************************************************
module	:	U2Dx9RenderStateMgr
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_DX9_RENDERSTATEMGR_H
#define	U2_DX9_RENDERSTATEMGR_H

#include "U2AlphaState.h"
#include "U2MaterialState.h"
#include "U2Dx9TextureState.h"
#include "U2ZBufferState.h"
#include "U2StencilState.h"
#include "U2Dx9MaterialMgr.h"
#include "U2Dx9ShaderConstantMgr.h"

class U2Dx9Renderer;

class U2_3D U2Dx9RenderStateMgr : public U2RefObject
{
	
public:
	virtual ~U2Dx9RenderStateMgr();

	static U2Dx9RenderStateMgr* Create(U2Dx9Renderer* pRenderer, bool bZBuffer);

	void	Reset(const D3DCAPS9& d3dCaps9, bool bZBuffer);

	void UpdateRenderState(const U2RenderStateCollection* pRSC);

	void SetAlphaState(const U2AlphaState* pState);
	void SetMaterialState(U2MaterialState * pState);
	void SetStencilState(const U2StencilState* pState);
	void SetZBufferState(const U2ZBufferState* pState);
	void SetWireframeState(const U2WireframeState* pState);
	

	inline void DisableAlpha();
	inline unsigned int GetSrcAlphaCaps();
	inline unsigned int GetDestAlphaCaps();
	inline unsigned int GetD3DBlendMode(
		U2AlphaState::BlendMode eBlend);
	inline unsigned int GetD3DTestMode(U2AlphaState::TestMode eTest);

	inline void SetBoneCount(unsigned short usNumBones);

	void SetAntiAlias(bool bOn) {}


	void InitRenderState();
	void SaveRenderState();
	void RestoreRenderState();

	inline void SetRenderState(D3DRENDERSTATETYPE eState, 
		unsigned int uiValue, bool bSave);
	inline void SetRenderState(D3DRENDERSTATETYPE eState,
		unsigned int uiValue);
	inline unsigned int GetRenderState(D3DRENDERSTATETYPE eState) const;
	inline void RestoreRenderState(D3DRENDERSTATETYPE eState);


	void GetCameraNearAndFar(float& fNear, float& fFar);
	void SetCameraNearAndFar(float fNear, float fFar);


	void InitShaderState();
	void SaveShaderState();
	void RestoreShaderState();

	inline void SetPixelShader(LPDIRECT3DPIXELSHADER9 hShader, 
		bool bSave = false);
	inline LPDIRECT3DPIXELSHADER9 GetPixelShader() const;
	inline void RestorePixelShader();
	inline void ClearPixelShader(LPDIRECT3DPIXELSHADER9 hShader);

	inline void SetVertexShader(LPDIRECT3DVERTEXSHADER9 hShader, 
		bool bSave = false);
	inline LPDIRECT3DVERTEXSHADER9 GetVertexShader() const;
	inline void RestoreVertexShader();
	inline void ClearVertexShader(LPDIRECT3DVERTEXSHADER9 hShader);

	inline void SetFVF(unsigned int uiFVF, bool bSave = false);
	inline unsigned int GetFVF() const;
	inline void RestoreFVF();
	inline void ClearFVF(unsigned int uiFVF);

	inline void SetVertexDecl(LPDIRECT3DVERTEXDECLARATION9 hDeclaration, 
		bool bSave = false);
	inline LPDIRECT3DVERTEXDECLARATION9 GetVertDecl() const;
	inline void RestoreVertDecl();
	inline void ClearVertDecl(LPDIRECT3DVERTEXDECLARATION9 hDeclaration);

	// Texture Stage and Sampler State management
	void InitTextureStageState();
	void SaveTextureStageState();
	void RestoreTextureStageState();

	inline void SetTextureStageState(unsigned int uiStage, 
		D3DTEXTURESTAGESTATETYPE eState, unsigned int uiValue, 
		bool bSave);
	inline void SetTextureStageState(unsigned int uiStage, 
		D3DTEXTURESTAGESTATETYPE eState, unsigned int uiValue);
	inline unsigned int GetTextureStageState(unsigned int uiStage, 
		D3DTEXTURESTAGESTATETYPE eState) const;
	inline void RestoreTextureStageState(unsigned int uiStage, 
		D3DTEXTURESTAGESTATETYPE eState);
	

	inline void SetSamplerState(unsigned int uiStage,
		D3DSAMPLERSTATETYPE eState, unsigned int uiValue, bool bSave);
	inline void SetSamplerState(unsigned int uiStage,
		D3DSAMPLERSTATETYPE eState, unsigned int uiValue);
	inline unsigned int GetSamplerState(unsigned int uiStage, 
		D3DSAMPLERSTATETYPE eState);
	inline void RestoreSamplerState(unsigned int uiStage,
		D3DSAMPLERSTATETYPE eState);

	// Texture management
	inline void InitTextures();
	inline void SetTexture(unsigned int uiStage, 
		LPDIRECT3DBASETEXTURE9 pkTexture);
	inline LPDIRECT3DBASETEXTURE9 GetTexture(unsigned int uiStage);
	inline void ClearTexture(LPDIRECT3DBASETEXTURE9 pkTexture);

	// Shader Constant management
	inline bool SetVertexShaderConstantB(unsigned int uiStartRegister,
		const BOOL* pbConstantData, unsigned int uiBoolCount, 
		bool bSave = false);
	inline bool GetVertexShaderConstantB(unsigned int uiStartRegister,
		BOOL* pbConstantData, unsigned int uiBoolCount);
	inline bool RestoreVertexShaderConstantB(unsigned int uiStartRegister, 
		unsigned int uiBoolCount);
	inline bool SetVertexShaderConstantF(unsigned int uiStartRegister,
		const float* pfConstantData, unsigned int uiFloat4Count, 
		bool bSave = false);
	inline bool GetVertexShaderConstantF(unsigned int uiStartRegister,
		float* pfConstantData, unsigned int uiFloat4Count);
	inline bool RestoreVertexShaderConstantF(unsigned int uiStartRegister, 
		unsigned int uiFloat4Count);
	inline bool SetVertexShaderConstantI(unsigned int uiStartRegister,
		const int* piConstantData, unsigned int uiInt4Count, 
		bool bSave = false);
	inline bool GetVertexShaderConstantI(unsigned int uiStartRegister,
		int* piConstantData, unsigned int uiInt4Count);
	inline bool RestoreVertexShaderConstantI(unsigned int uiStartRegister, 
		unsigned int uiInt4Count);
	inline bool SetVertexShaderConstant(unsigned int uiStartRegister,
		const float* pfConstantData, unsigned int uiFloat4Count, 
		bool bSave = false);
	inline bool GetVertexShaderConstant(unsigned int uiStartRegister,
		float* pfConstantData, unsigned int uiFloat4Count);
	inline bool RestoreVertexShaderConstant(unsigned int uiStartRegister, 
		unsigned int uiFloat4Count);

	inline bool SetPixelShaderConstantB(unsigned int uiStartRegister,
		const BOOL* pbConstantData, unsigned int uiBoolCount, 
		bool bSave = false);
	inline bool GetPixelShaderConstantB(unsigned int uiStartRegister,
		BOOL* pbConstantData, unsigned int uiBoolCount);
	inline bool RestorePixelShaderConstantB(unsigned int uiStartRegister, 
		unsigned int uiBoolCount);
	inline bool SetPixelShaderConstantF(unsigned int uiStartRegister,
		const float* pfConstantData, unsigned int uiFloat4Count, 
		bool bSave = false);
	inline bool GetPixelShaderConstantF(unsigned int uiStartRegister,
		float* pfConstantData, unsigned int uiFloat4Count);
	inline bool RestorePixelShaderConstantF(unsigned int uiStartRegister, 
		unsigned int uiFloat4Count);
	inline bool SetPixelShaderConstantI(unsigned int uiStartRegister,
		const int* piConstantData, unsigned int uiInt4Count, 
		bool bSave = false);
	inline bool GetPixelShaderConstantI(unsigned int uiStartRegister,
		int* piConstantData, unsigned int uiInt4Count);
	inline bool RestorePixelShaderConstantI(unsigned int uiStartRegister, 
		unsigned int uiInt4Count);
	inline bool SetPixelShaderConstant(unsigned int uiStartRegister,
		const float* pfConstantData, unsigned int uiFloat4Count, 
		bool bSave = false);
	inline bool GetPixelShaderConstant(unsigned int uiStartRegister,
		float* pfConstantData, unsigned int uiFloat4Count);
	inline bool RestorePixelShaderConstant(unsigned int uiStartRegister, 
		unsigned int uiFloat4Count);

	inline void SetForceNormalizeNormals(bool bFlags);

	void	InitAllStates();

	static unsigned int GetSamplerStateMapping(unsigned int uSamplerState);

	inline void CommitShaderConsts();

private:
	U2Dx9RenderStateMgr(U2Dx9Renderer*, const D3DCAPS9& kD3DCaps9);


	// 렌더 상태 값의 기록을 저장.
	class U2Dx9RSBlock : public U2MemObj
	{
	public:
		U2Dx9RSBlock();
		void SetValue(uint32 val, bool bSave);

		void InitValue(uint32 uVal);

		void SaveValue();
		void RestoreValue();

		uint32 GetValue() const;
		uint32 GetPrevValue() const;

	private:
		uint32 m_uCurrVal;
		uint32 m_uPrevVal;
	};

	uint32 m_uFlags;

	enum 
	{ 
		MAX_RENDER_STATES        = 256,
		MAX_TEXSTAGE_STATES = 33,
		MAX_TEXTURE_STAGES       = 16,
		MAX_SAMPLER_STATES       = 14,
		MAX_TEXBLEND_STAGES = 2,
		MAX_SAMPLERS = 2,	
		MAX_D3D_SAMPLERS = 13
	};

	// Initial Values
	// Render States
	struct RenderStateInitValue 
	{
	
		D3DRENDERSTATETYPE m_eType;
		unsigned int m_uVal;
	};

	static RenderStateInitValue 
		ms_initRenderStates[MAX_RENDER_STATES + 1];

	struct TextureStageInitValue
	{
		D3DTEXTURESTAGESTATETYPE m_eType;
		unsigned int m_uVal;
	};

	static TextureStageInitValue 
		ms_initTextureStages[MAX_TEXSTAGE_STATES + 1];

	struct SamplerStateInitValue
	{
		D3DSAMPLERSTATETYPE m_eType;
		unsigned int m_uVal;
	};

	static SamplerStateInitValue 
		ms_initSamplerStates[MAX_SAMPLER_STATES+1];

	// Renser States
	U2Dx9RSBlock m_RSBlocks[MAX_RENDER_STATES];

	// Texture Stages and Sampler States
	U2Dx9RSBlock m_TSRSBlocks[MAX_TEXTURE_STAGES][MAX_TEXSTAGE_STATES];
	U2Dx9RSBlock m_SSRSBlocks[MAX_TEXTURE_STAGES][MAX_TEXSTAGE_STATES];	

	enum VertexBlendMode
	{
		VBMODE_NUM = 5	// Dx 문서 참조.
	};

	
	D3DVERTEXBLENDFLAGS m_d3dVertBlendFlags[VBMODE_NUM];

	// AlphaBlending
	uint32 m_uAlphaBlendModes[U2AlphaState::MAX_BLEND_MODES];
	uint32 m_uAlphaTestModes[U2AlphaState::MAX_TEST_MODES];

	uint32 m_uSrcAlphaCaps;
	uint32 m_uDestAlphaCaps;
	

	// Stencil 
	uint32 m_uStencilTestModes[U2StencilState::MAX_TYPES];
	uint32 m_uSteinlOps[U2StencilState::MAX_OP_MODE];	
	uint32 m_uCullModes[U2StencilState::CM_QUANTITY];

	static unsigned int ms_uSSMappings[MAX_D3D_SAMPLERS];

	LPDIRECT3DBASETEXTURE9 m_apTSTextures[MAX_TEXTURE_STAGES];
	
	U2Dx9MaterialMgr	m_matMgr;
	
	uint32 m_uZBufferModes[U2ZBufferState::MAX_CFS];


	// Fog Members
	float m_fCameraNear;
	float m_fCameraFar;
	float m_fCameraDepthRange;
	float m_fMaxFogFactor;
	float m_fMaxFogValue;
	D3DXCOLOR m_currFogColor;
	unsigned int m_uiCurrFogColor;


	// Shader programms 
	LPDIRECT3DVERTEXSHADER9 m_pCurrVS;
	LPDIRECT3DVERTEXSHADER9 m_pPrevVS;
	LPDIRECT3DPIXELSHADER9 m_pCurrPS;
	LPDIRECT3DPIXELSHADER9 m_pPrevPS;

	// Shader Constant Mgr
	U2Dx9ShaderConstantMgrPtr m_spShaderConstMgr;

	bool	m_bForceNormalizeNormals;

	// Vertex Format
	bool	m_bVertDecl;
	uint32	m_uCurrFVF;
	uint32	m_uPrevFVF;

	LPDIRECT3DVERTEXDECLARATION9 m_pCurrVertDecl;
	LPDIRECT3DVERTEXDECLARATION9 m_pPrevVertDecl;

	D3DCAPS9	m_d3dCaps9;
	LPDIRECT3DDEVICE9 m_pD3DDev;
	U2Dx9Renderer*	m_pRenderer;
};


#include "U2Dx9RenderStateMgr.inl"


#endif
