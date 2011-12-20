/**************************************************************************************************
module	:	U2D3DXEffectShader
Author	:	Yun sangyong
Desc	:	
**************************************************************************************************/
#pragma once
#ifndef U2_D3DXEFFECTSHADER_H
#define U2_D3DXEFFECTSHADER_H

#include "U2Shader.h"
#include "U2StreamInstance.h"
#include "U2ShaderAttribute.h"

class U2_3D U2D3DXEffectShader : public U2Shader 
{
public:
	U2D3DXEffectShader(U2Dx9Renderer* pRenderer);
	virtual ~U2D3DXEffectShader();


	void SetFilename(const U2DynString& szFilename);
	const U2DynString& GetFilename() const;

	// Create or append an instance stream declaration for this shader
	int UpdateInstatnceStreamDecl(U2StreamInstance::Declaration& decl);
	bool HasTechnique(const char* szTechnique) const;	
	bool SetTechnique(const char* szTechnique);

	// Get Current Technique
	const char* GetTechnique() const;
	
	bool IsParamUsed(U2FxShaderState::Param eParam);

	void SetBool(U2FxShaderState::Param eParam, bool val);

	void SetInt(U2FxShaderState::Param eParam, int val);

	void SetFloat(U2FxShaderState::Param eParam, float val);

	void SetVector4(U2FxShaderState::Param eParam, 
		const D3DXVECTOR4& val);

	inline void SetVector3(U2FxShaderState::Param eParam, 
		const D3DXVECTOR3& val)
	{	
		U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));
		static D3DXVECTOR4 v;
		v.x = val.x; v.y = val.y; v.z = val.z; v.w = 1.f;

		m_currAttb.SetArg(eParam, U2ShaderArg(v));
		HRESULT hr = m_pD3DEffect->SetVector(m_ahParamHandles[eParam], &v);			
	}

	void SetFloat4(U2FxShaderState::Param eParam, 
		const U2Float4& val);

	inline void SetMatrix(U2FxShaderState::Param eParam, 
		const D3DXMATRIX& val)
	{
		U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));
		m_currAttb.SetArg(eParam, U2ShaderArg(val));
		HRESULT hr = m_pD3DEffect->SetMatrix(m_ahParamHandles[eParam], &val);			
	}

	void SetTexture(U2FxShaderState::Param eParam, 
		const U2Dx9BaseTexture* pTex);
	
	void SetBoolArray(U2FxShaderState::Param eParam, 
		const bool* pArray, int count);
	
	void SetIntArray(U2FxShaderState::Param eParam, 
		const int* pArray, int count);

	void SetFloatArray(U2FxShaderState::Param eParam, 
		const float* pArray, int count);

	void SetFloat4Array(U2FxShaderState::Param eParam, 
		const U2Float4* pArray, int count);

	void SetVector4Array(U2FxShaderState::Param eParam, 
		const D3DXVECTOR4* pArray, int count);

	void SetMatrixArray(U2FxShaderState::Param eParam, 
		const D3DXMATRIX* pArray, int count);

	void SetMatrixPointerArray(U2FxShaderState::Param eParam, 
		const D3DXMATRIX** ppArray, int count);

	void SetAttribute(const U2ShaderAttribute& attb);

	int Begin(bool saveState);

	void BeginPass(int pass);

	void CommitChanges();

	void End();
	
	void EndPass();

	bool LoadResource();
	void UnloadResource();

	U2D3DXEffectShader* GetNext();
	U2D3DXEffectShader* GetPRev();

	static U2D3DXEffectShader* GetHead();
	static U2D3DXEffectShader* GetTail();

	void OnLostDevice();
	void OnResetDevice();

private:

	void AddShaderToList();
	void RemoveShaderFromList();
	
	U2D3DXEffectShader* m_pNext;
	U2D3DXEffectShader* m_pPrev;

	static U2D3DXEffectShader* ms_pHead;
	static U2D3DXEffectShader* ms_pTail;

	

	void ValidateEffect();

	void UpdateAttbHandles();

	U2FxShaderState::Param D3DXParamFromUserParam(D3DXHANDLE handle);
	
	friend class U2Dx9Renderer;

	U2Dx9Renderer* m_pRenderer;

	bool m_bNeedSoftwareVertexProcessing;	// Curr technique need software processing mode ?

	ID3DXEffect* m_pD3DEffect;
	bool m_bValidated;
	bool m_bNotValidate;
	bool m_bBeginPass;
	D3DXHANDLE m_ahParamHandles[U2FxShaderState::NumParameters];
	U2ShaderAttribute m_currAttb;

	// U2Shader::m_szShaderName으로 대체.. 
	U2DynString m_szFilename;
};

typedef U2SmartPtr<U2D3DXEffectShader> U2D3DXEffectShaderPtr;

#endif 