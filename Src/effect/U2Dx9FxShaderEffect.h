/**************************************************************************************************
module	:	U2Dx9FxShaderEffect
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_D3D9FXEFFECT_H
#define U2_D3D9FXEFFECT_H


#include "U2Effect.h"
#include <U2_3D/src/Shader/U2D3DXEffectShader.h>
#include <U2_3D/src/Dx9/U2Dx9Frame.h>

class U2Mesh;

class U2_3D U2Dx9FxShaderEffect : public U2Effect
{
	DECLARE_RTTI;

public:
		

	U2Dx9FxShaderEffect(U2D3DXEffectShader* pShader, U2Dx9Renderer *pRenderer);
	virtual ~U2Dx9FxShaderEffect();

	void Initialize();

	virtual void Render(U2Dx9Renderer* pRenderer, U2Spatial* pGlobalObj, 
		int iStart, int iEnd, U2VisibleObject* pVisible);

	virtual int  Begin();
	virtual void End();
	virtual void BeginEffect(int pass, U2Dx9Renderer* pRenerer, bool bPrimary);
	virtual void SetupStage(int pass, U2Dx9Renderer* pRenerer);
	virtual void Commit(int pass, U2Dx9Renderer* pRenerer);
	virtual void EndEffect(int pass, U2Dx9Renderer* pRenerer, bool bPrimary);

	virtual void LoadResoures(U2Dx9Renderer* pRenderer, U2Mesh* pMesh);
	virtual void ReleaseResources(U2Dx9Renderer* pRenderer, U2Mesh* pMesh);
	
	//2010-07-05
	virtual void UpdateShader() {}
	
	U2D3DXEffectShader* GetEffectShader() const;
	void AddShader(U2D3DXEffectShader* pShader);
	
	void SetTexture(U2FxShaderState::Param param, const TCHAR* szTexturename);
	const TCHAR* GetTexture(U2FxShaderState::Param param) const;
	void SetInt(U2FxShaderState::Param param, int val);
	int GetInt(U2FxShaderState::Param param) const;
	void SetBool(U2FxShaderState::Param param, bool val);
	bool GetBool(U2FxShaderState::Param param) const;
	void SetFloat(U2FxShaderState::Param param, float val);
	float GetFloat(U2FxShaderState::Param param) const;
	void SetVector(U2FxShaderState::Param param, const D3DXVECTOR4& val);
	void SetVector(U2FxShaderState::Param param, float r, float g, float b, float a);
	const D3DXVECTOR4& GetVector(U2FxShaderState::Param param) const;

	void SetTexture(const TCHAR* strParam, const TCHAR* szTexturename);
	const TCHAR* GetTexture(const TCHAR* strParam) const;
	void SetInt(const TCHAR* strParam, int val);
	int GetInt(const TCHAR* strParam) const;
	void SetBool(const TCHAR* strParam, bool val);
	bool GetBool(const TCHAR* strParam) const;
	void SetFloat(const TCHAR* strParam, float val);
	float GetFloat(const TCHAR* strParam) const;
	void SetVector(const TCHAR* strParam, const D3DXVECTOR4& val);
	void SetVector(const TCHAR* strParam, float r, float g, float b, float a);
	const D3DXVECTOR4& GetVector(const TCHAR* strParam) const;


	bool HasParam(U2FxShaderState::Param param);

	int GetNumTextures() const;

	//const TCHAR* GetTextureName(uint32 idx) const;
	U2FxShaderState::Param GetTextureParam(uint32 idx) const;

	uint32 GetNumParams() const;
	const TCHAR* GetParamNameByIdx(uint32 idx) const;
	const TCHAR* GetParamTypeByIdx(uint32 idx) const;

	U2ShaderAttribute& GetShaderAttb();


	void SetFrameShaderName(const TCHAR* szFrameShadername);
	const TCHAR* GetFrameShaderName() const;

	uint32 GetShaderIdx();

	// Texture Transform		
	
protected:	
	bool LoadTexture(uint32 idx);
	void UnLoadTexture(uint32 idx);
	
	class TextureNode : public U2MemObj
	{
	public:
		TextureNode();
		TextureNode(U2FxShaderState::Param eSahderParam, const TCHAR* szTexturename);

		U2FxShaderState::Param m_eShaderParam;
		U2DynString m_szTexname;
		U2Dx9BaseTexturePtr m_spTexture;
	};

	U2PrimitiveVec<TextureNode*> m_textureNodes;
	U2ShaderAttribute m_shaderAttb;

	U2DynString m_szFrameShadername;

	U2Dx9Renderer* m_pRenderer;

	//U2ObjVec<U2D3DXEffectShaderPtr> m_d3dShaderArray;
	
	//U2ObjVec<U2LightPtr> m_lights;

	U2String m_szTechnique;	
	
	uint32 m_uiShaderIdx;


	
	U2D3DXEffectShader* m_pShader;

	U2FramePtr m_spFrame;



	bool m_bShaderUpdateEnabled;

	
	
	
};

typedef U2SmartPtr<U2Dx9FxShaderEffect> U2Dx9FxShaderEffectPtr;

#include "U2Dx9FxShaderEffect.inl"

#endif 