/************************************************************************
module	:	U2Dx9FramePass
Author	:	Yun sangyong
Desc	:	Nebula Device 2 
			Begin 미구현

			4.28 Update UpdateMeshCoords 함수 제거 또는 호출 방법 변경
************************************************************************/
#pragma once 
#ifndef U2_DX9_FRAMEPASS_H
#define U2_DX9_FRAMEPASS_H

#include "U2Dx9FramePhase.h"
#include "U2Dx9FrameShader.h"
#include "U2Lib/src/U2StopWatch.h"
#include <U2_3D/Src/Dx9/U2Dx9Renderer.h>

#include "U2N2Mesh.h"


class U2Frame;
class U2FrameSection;

class U2_3D U2FramePass : public U2Object
{

public:
	enum ShadowTechnique
	{
		NO_SHADOW,
		SIMPLE,
		MULTI_LIGHT,	// draw multi-light shadows
	};

	enum PixelFlag
	{
		PIXEL_SIZE = 0
	};


	U2FramePass();
	~U2FramePass();

	//void operator=(const U2Dx9FramePass);


	void	SetFrame(U2Frame* pFrame);
	U2Frame* GetFrame() const;

	void SetShaderAlias(const U2DynString& p);
	const U2DynString& GetShaderAlias() const;

	U2D3DXEffectShader* GetEffectShader() const;

	void SetTechnique(const U2DynString& tec);
	const U2DynString& GetTechnique() const;

	void SetClearFlags(unsigned int uiClearFlags);
	uint32 GetClearFlags() const;

	void SetOcclusionQuery(bool b);
	bool GetOcculsionQuery() const;

	/// set the "shadow enabled condition" flag
	void SetShadowEnabled(bool b);
	/// get the "shadow enabled condition" flag
	bool GetShadowEnabled() const;

	/// set the "draw gui" flag
	void SetDrawGui(bool b);
	/// get the "draw gui" flag
	bool GetDrawGui() const;

	/// set the draw full-screen quad flag
	void SetDrawFullscreenQuad(bool b);
	/// get the draw full-screen quad flag
	bool GetDrawFullscreenQuad() const;

	void AddConstantShaderParam(U2FxShaderState::Param eParam, const U2ShaderArg& arg);

	void AddVariableShaderParam(const U2DynString& szVar, U2FxShaderState::Param eParam, 
		const U2ShaderArg& arg);

	const U2ShaderAttribute& GetShaderAttb() const;

	void AddFramePhase(U2FramePhase* pPhase);

	const U2ObjVec<U2FramePhasePtr>& GetFramePhases() const;

	U2FramePhase* GetFramePhase(unsigned int idx) const;

	void SetDepthClear(const float fZClear);
	float GetDepthClear() const;	
	void SetBackgroundColor(const D3DXCOLOR& kColor);
	void GetBackgroundColor(D3DXCOLOR& kColor) const;
	void SetStencilClear(unsigned int uiClear);
	unsigned int GetStencilClear() const;

	unsigned int Begin();	

	void End();

	void SetRenderTargetName(uint32 idx, const U2DynString& szName);
	const U2DynString& GetRenderTargetName(uint32 idx) const;

	U2RenderTargetPtr GetRenderTarget() const;

	void SetShadowTechnique(ShadowTechnique t);

	inline ShadowTechnique GetShadowTechnique() const;

	static ShadowTechnique StringToShadowTechnique(const TCHAR* str);

	void SetDepthStencil(bool bDepthStencil);



	void DrawFullScreenQuad();

	void CreateScreenQuad();
	void RecreateScreenQuad();

	
private:
	friend class U2FrameSection;

	void Validate();

	void UpdateVariableShaderParams();

	void UpdateMeshCoords();

	struct ShaderParam : public U2MemObj
	{
		U2FxShaderState::Type m_eType;
		U2DynString m_szStatename;
		U2DynString m_szVal;
	};

	U2PrimitiveVec<ShaderParam*> m_constShaderParams;
	U2PrimitiveVec<ShaderParam*> m_varShaderParams;

	U2FixedObjArray<U2DynString> m_renderTargetNames;
	U2ObjVec<U2FramePhasePtr> m_framePhases;
	U2VariableContext m_varContext;

	U2DynString m_szShaderAlias;
	U2DynString m_szTechnique;
	U2FourCC m_shaderFourCC;
	unsigned int m_uiFrameShaderIdx;
	bool m_bFrameInBegin;

	U2ShaderAttribute m_shaderAttb;
	U2N2MeshPtr	m_spScreenQuad;

	uint32 m_uiClearFlags;
	
	ShadowTechnique m_eShadowTeq;

	bool m_bDrawFullscreenQuad;
	bool m_bDrawGui;
	bool m_bShadowEnabled;
	bool m_bOcclusionQuery;
	bool m_bDepthStencil;

	D3DXCOLOR m_bgColor;
	float m_fClearDepth;
	int		m_iClearStencil;
	

	UINT m_currRTWidth;
	UINT m_currRTHeight;

	U2RenderTargetPtr m_spRenderTarget;
	U2Frame* m_pOwnerFrame;

};

typedef U2SmartPtr<U2FramePass> U2FramePassPtr;

#include "U2Dx9FramePass.inl"

#endif