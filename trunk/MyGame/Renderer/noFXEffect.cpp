///////////////////////////////////////////////////////////////////////
// File : SXEffect.cpp
// Description : D3DX Effects Parameter Manipulation Class
//				 (SXEffect) class implementation.
//				 (SXEffVariant) class implementation.
//				 (SXEffParam) class implementation.
// Author : Wessam Bahnassi, In|Framez
//
///////////////////////////////////////////////////////////////////////

///////////////// #includes /////////////////
//#include <d3dx9.h>
#include "DXUT.h"
#include "noRenderSystemDX9.h"
#include "noD3DXEffectShaderInclude.h"

#include "noFXEffect.h"


///////////////// Macros /////////////////
//#define SAFE_RELEASE(ptr) {if(ptr){(ptr)->Release();(ptr)=NULL;}}
//#define SAFE_DELETE(ptr) {if(ptr){delete(ptr);(ptr)=NULL;}}
//#define SAFE_DELETE_ARRAY(ptr) {if(ptr){delete[](ptr);(ptr)=NULL;}}
//#ifdef _DEBUG
//#define SXTRACE(text) OutputDebugString(text)
//#else
//#define SXTRACE(text) __noop
//#endif // _DEBUG

#define IMPLEMENT_ANIM(bColor) SAFE_DELETE(m_pAnim);	\
	m_pAnim = new noEffAnimation(*this,bColor);	\
	if (!m_pAnim) return;	\
	if (!m_pAnim->m_pfnLerper) {SAFE_DELETE(m_pAnim);return;}


//////////////////////////////// SXEffAnimation Class Declaration ////////////////////////////////
noEffAnimation::noEffAnimation(const noEffVariant& varValue,bool bColor) :
	m_pfnLerper(NULL),
	m_fWeight(0.0f),
	m_vec4Key1(0,0,0,0),
	m_vec4Key2(1,1,1,1)
{
	if (bColor)
	{
		if (!varValue.IsA(D3DXPT_INT))
			return;
		m_pfnLerper = LerpColor;
		return;
	}

	// Assign LERPer
	switch (varValue.GetClass())
	{
	case D3DXPC_SCALAR:
		{
			D3DXPARAMETER_TYPE paramType = varValue.GetType();
			if (paramType == D3DXPT_FLOAT)
				m_pfnLerper = LerpFloat1;
			else if (paramType == D3DXPT_INT)
				m_pfnLerper = LerpInteger;
		}
		return;

	case D3DXPC_VECTOR:
		if (!varValue.IsA(D3DXPT_FLOAT))
			return;
		else
		{
			static LERPFUNC aFuncs[] = {NULL,LerpFloat1,LerpFloat2,LerpFloat3,LerpFloat4};
			UINT uSize = varValue.GetVectorSize();
			if (uSize >= sizeof(aFuncs)/sizeof(aFuncs[0]))
				return;
			m_pfnLerper = aFuncs[uSize];
		}
	}
}

noEffAnimation::~noEffAnimation()
{
}

void noEffAnimation::LerpFloat1(noEffAnimation& animThis,noEffVariant& varOwner)
{
	float fVal = animThis.m_vec4Key1.x + animThis.m_fWeight * (animThis.m_vec4Key2.x-animThis.m_vec4Key1.x);
	varOwner = fVal;
}

void noEffAnimation::LerpFloat2(noEffAnimation& animThis,noEffVariant& varOwner)
{
	D3DXVECTOR2 vec2Val;
	varOwner = *D3DXVec2Lerp(&vec2Val,(D3DXVECTOR2*)&animThis.m_vec4Key1,(D3DXVECTOR2*)&animThis.m_vec4Key2,animThis.m_fWeight);
}

void noEffAnimation::LerpFloat3(noEffAnimation& animThis,noEffVariant& varOwner)
{
	D3DXVECTOR3 vec3Val;
	varOwner = *D3DXVec3Lerp(&vec3Val,(D3DXVECTOR3*)&animThis.m_vec4Key1,(D3DXVECTOR3*)&animThis.m_vec4Key2,animThis.m_fWeight);
}

void noEffAnimation::LerpFloat4(noEffAnimation& animThis,noEffVariant& varOwner)
{
	D3DXVECTOR4 vec4Val;
	varOwner = *D3DXVec4Lerp(&vec4Val,&animThis.m_vec4Key1,&animThis.m_vec4Key2,animThis.m_fWeight);
}

void noEffAnimation::LerpInteger(noEffAnimation& animThis,noEffVariant& varOwner)
{
	int iVal = (int)(animThis.m_vec4Key1.x + animThis.m_fWeight * (animThis.m_vec4Key2.x-animThis.m_vec4Key1.x));
	varOwner = iVal;
}

void noEffAnimation::LerpColor(noEffAnimation& animThis,noEffVariant& varOwner)
{
	D3DXCOLOR clrVal;
	D3DXColorLerp(&clrVal,(D3DXCOLOR*)&animThis.m_vec4Key1,(D3DXCOLOR*)&animThis.m_vec4Key2,animThis.m_fWeight);
	varOwner = (int)(D3DCOLOR)clrVal;
}


//////////////////////////////// SXEffVariant Class Implementation ////////////////////////////////
noEffVariant::noEffVariant(noFXEffect *pOwnerEffect,D3DXHANDLE hParam) :
	m_pOwnerEffect(pOwnerEffect),
	m_hParam(hParam),
	m_pAnim(NULL)
{
}

noEffVariant::~noEffVariant()
{
	// Clear animation
	SAFE_DELETE(m_pAnim);

	// Clear off any variants allocated through this array variant
	noEffVariantElemHash::const_iterator itElem = m_hashElems.begin();
	while (itElem != m_hashElems.end())
	{
		noEffVariant *pElem = (itElem++)->second;
		SAFE_DELETE(pElem);
	}
	m_hashElems.clear();
}

D3DXPARAMETER_TYPE noEffVariant::GetType(void) const
{
	D3DXPARAMETER_DESC descParam;
	HRESULT hRetval = m_pOwnerEffect->GetD3DXEffect()->GetParameterDesc(m_hParam,&descParam);
	return SUCCEEDED(hRetval)?descParam.Type:(D3DXPARAMETER_TYPE)D3DXEDT_FORCEDWORD;
}

D3DXPARAMETER_CLASS noEffVariant::GetClass(void) const
{
	D3DXPARAMETER_DESC descParam;
	HRESULT hRetval = m_pOwnerEffect->GetD3DXEffect()->GetParameterDesc(m_hParam,&descParam);
	return SUCCEEDED(hRetval)?descParam.Class:(D3DXPARAMETER_CLASS)D3DXEDT_FORCEDWORD;
}

noEffVariant& noEffVariant::operator[] (int iIndex)
{
	noEffVariantElemHash::const_iterator itElem = m_hashElems.find(iIndex);
	if (itElem != m_hashElems.end())
		return *(itElem->second);

	D3DXHANDLE hParam = m_pOwnerEffect->GetD3DXEffect()->GetParameterElement(m_hParam,iIndex);
	if (!hParam)
		return m_pOwnerEffect->m_paramInvalid;

	noEffVariant *pRetval = new noEffVariant(m_pOwnerEffect,hParam);
	if (!pRetval)
		return m_pOwnerEffect->m_paramInvalid;

	m_hashElems[iIndex] = pRetval;
	return *pRetval;
}

UINT noEffVariant::GetElemsCount(void) const
{
	D3DXPARAMETER_DESC descParam;
	HRESULT hRetval = m_pOwnerEffect->GetD3DXEffect()->GetParameterDesc(m_hParam,&descParam);
	return SUCCEEDED(hRetval)?descParam.Elements:0;
}

UINT noEffVariant::GetVectorSize(void) const
{
	D3DXPARAMETER_DESC descParam;
	HRESULT hRetval = m_pOwnerEffect->GetD3DXEffect()->GetParameterDesc(m_hParam,&descParam);
	return SUCCEEDED(hRetval)?descParam.Columns:0;
}

noEffVariant::operator bool (void)
{
	BOOL bValue = FALSE;
	m_pOwnerEffect->GetD3DXEffect()->GetBool(m_hParam,&bValue);
	return (bValue == TRUE);
}

noEffVariant::operator int (void)
{
	int iValue = 0;
	m_pOwnerEffect->GetD3DXEffect()->GetInt(m_hParam,&iValue);
	return iValue;
}

noEffVariant::operator float (void)
{
	float fValue = 0.0f;
	m_pOwnerEffect->GetD3DXEffect()->GetFloat(m_hParam,&fValue);
	return fValue;
}

noEffVariant::operator D3DXVECTOR2 (void)
{
	D3DXVECTOR2 vec2Value(0,0);
	m_pOwnerEffect->GetD3DXEffect()->GetFloatArray(m_hParam,vec2Value,2);
	return vec2Value;
}

noEffVariant::operator D3DXVECTOR3 (void)
{
	D3DXVECTOR3 vec3Value(0,0,0);
	m_pOwnerEffect->GetD3DXEffect()->GetFloatArray(m_hParam,vec3Value,3);
	return vec3Value;
}

noEffVariant::operator D3DXVECTOR4 (void)
{
	D3DXVECTOR4 vec4Value(0,0,0,1);
	m_pOwnerEffect->GetD3DXEffect()->GetVector(m_hParam,&vec4Value);
	return vec4Value;
}

noEffVariant::operator D3DXMATRIX (void)
{
	D3DXMATRIX matValue(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
	m_pOwnerEffect->GetD3DXEffect()->GetMatrix(m_hParam,&matValue);
	return matValue;
}

noEffVariant::operator PCSTR (void)
{
	PCSTR pszString = NULL;
	m_pOwnerEffect->GetD3DXEffect()->GetString(m_hParam,&pszString);
	return pszString;
}

noEffVariant::operator PDIRECT3DVERTEXSHADER9 (void)
{
	PDIRECT3DVERTEXSHADER9 pVertexShader = NULL;
	m_pOwnerEffect->GetD3DXEffect()->GetVertexShader(m_hParam,&pVertexShader);
	if (pVertexShader)
		pVertexShader->Release();	// Decrease reference count
	return pVertexShader;
}

noEffVariant::operator PDIRECT3DPIXELSHADER9 (void)
{
	PDIRECT3DPIXELSHADER9 pPixelShader = NULL;
	m_pOwnerEffect->GetD3DXEffect()->GetPixelShader(m_hParam,&pPixelShader);
	if (pPixelShader)
		pPixelShader->Release();	// Decrease reference count
	return pPixelShader;
}

noEffVariant::operator PDIRECT3DBASETEXTURE9 (void)
{
	PDIRECT3DBASETEXTURE9 pTexture = NULL;
	m_pOwnerEffect->GetD3DXEffect()->GetTexture(m_hParam,&pTexture);
	if (pTexture)
		pTexture->Release();	// Decrease reference count
	return pTexture;
}

noEffVariant& noEffVariant::operator= (const noEffVariant& varVal)
{
	// Copy value blindly
	LPD3DXEFFECT pEffect = m_pOwnerEffect->GetD3DXEffect();
	D3DXPARAMETER_DESC descThisParam,descThatParam;
	HRESULT hRetval = pEffect->GetParameterDesc(m_hParam,&descThisParam);
	hRetval |= pEffect->GetParameterDesc(varVal.m_hParam,&descThatParam);
	if (FAILED(hRetval))
	{
		SXTRACE("SXEffect ERROR: Failed to get value for copy\n");
		return *this;
	}

	BYTE *pData = new BYTE[descThatParam.Bytes];
	if (!pData)
		return *this;	// Out of memory

	pEffect->GetValue(varVal.m_hParam,pData,descThatParam.Bytes);
	pEffect->SetValue(m_hParam,pData,descThatParam.Bytes);
	SAFE_DELETE_ARRAY(pData);
	return *this;
}

void noEffVariant::Animate(float fKey1,float fKey2)
{
	IMPLEMENT_ANIM(false);
	m_pAnim->m_vec4Key1.x = fKey1;
	m_pAnim->m_vec4Key2.x = fKey2;
}

void noEffVariant::Animate(const D3DXVECTOR2& vec2Key1,const D3DXVECTOR2& vec2Key2)
{
	IMPLEMENT_ANIM(false);
	*((D3DXVECTOR2*)&m_pAnim->m_vec4Key1) = vec2Key1;
	*((D3DXVECTOR2*)&m_pAnim->m_vec4Key2) = vec2Key2;
}

void noEffVariant::Animate(const D3DXVECTOR3& vec3Key1,const D3DXVECTOR3& vec3Key2)
{
	IMPLEMENT_ANIM(false);
	*((D3DXVECTOR3*)&m_pAnim->m_vec4Key1) = vec3Key1;
	*((D3DXVECTOR3*)&m_pAnim->m_vec4Key2) = vec3Key2;
}

void noEffVariant::Animate(const D3DXVECTOR4& vec4Key1,const D3DXVECTOR4& vec4Key2)
{
	IMPLEMENT_ANIM(false);
	m_pAnim->m_vec4Key1 = vec4Key1;
	m_pAnim->m_vec4Key2 = vec4Key2;
}

void noEffVariant::Animate(int iKey1,int iKey2)
{
	IMPLEMENT_ANIM(false);
	m_pAnim->m_vec4Key1.x = (float)iKey1;
	m_pAnim->m_vec4Key2.x = (float)iKey2;
}

void noEffVariant::AnimateAsColor(D3DCOLOR clrKey1,D3DCOLOR clrKey2)
{
	IMPLEMENT_ANIM(true);
	*((D3DXCOLOR*)&m_pAnim->m_vec4Key1) = D3DXCOLOR(clrKey1);
	*((D3DXCOLOR*)&m_pAnim->m_vec4Key2) = D3DXCOLOR(clrKey2);
}


//////////////////////////////// SXEffParam Class Implementation ////////////////////////////////
noEffParam::noEffParam(noFXEffect *pOwnerEffect,D3DXHANDLE hParam) : noEffVariant(pOwnerEffect,hParam)
{
}

noEffParam::~noEffParam()
{
	// Clear off any annotations allocated through this parameter
	SXEffVariantsHash::const_iterator itVars = m_hashAnnotations.begin();
	while (itVars != m_hashAnnotations.end())
	{
		noEffVariant *pVar = (itVars++)->second;
		SAFE_DELETE(pVar);
	}
	m_hashAnnotations.clear();
}

noEffVariant& noEffParam::GetPassAnnotation(PCSTR pszPassName,PCSTR pszAnnName)
{
	// Was it requested before? If so, then it's stored in our hash table
	SXEffVariantsHash::const_iterator itAnn = m_hashAnnotations.find(pszAnnName);
	if (itAnn != m_hashAnnotations.end())
		return *(itAnn->second);

	// Nope, try to get it from ID3DXEffect
	D3DXHANDLE hPass = m_pOwnerEffect->GetD3DXEffect()->GetPassByName(m_hParam,pszPassName);
	if (!hPass)
		return m_pOwnerEffect->m_paramInvalid;

	D3DXHANDLE hAnn = m_pOwnerEffect->GetD3DXEffect()->GetAnnotationByName(hPass,pszAnnName);
	if (!hAnn)
		return m_pOwnerEffect->m_paramInvalid;

	noEffVariant *pAnn = new noEffVariant(m_pOwnerEffect,hAnn);
	if (!pAnn)
		return m_pOwnerEffect->m_paramInvalid;

	// Push it into the hash table for faster access next time
	m_hashAnnotations[pszAnnName] = pAnn;
	return *pAnn;
}

noEffVariant& noEffParam::Annotation(PCSTR pszAnnName)
{
	SXEffVariantsHash::const_iterator itVariant = m_hashAnnotations.find(pszAnnName);
	if (itVariant != m_hashAnnotations.end())
		return *(itVariant->second);

	D3DXHANDLE hParam = m_pOwnerEffect->GetD3DXEffect()->GetAnnotationByName(m_hParam,pszAnnName);
	if (!hParam)
		return m_pOwnerEffect->m_paramInvalid;

	noEffVariant *pRetval = new noEffVariant(m_pOwnerEffect,hParam);
	if (!pRetval)
		return m_pOwnerEffect->m_paramInvalid;

	m_hashAnnotations[pszAnnName] = pRetval;
	return *pRetval;
}


//////////////////////////////// SXEffect Class Implementation ////////////////////////////////
noFXEffect::noFXEffect() :
	m_pEffect(NULL),
	m_paramInvalid(NULL,NULL),
	m_bBeginPass(false),
	m_bValidated(false),
	m_bNotValidate(false),
	m_bNeedSoftwareVertexProcessing(false)
{
	m_paramInvalid.m_pOwnerEffect = this;
}

noFXEffect::~noFXEffect()
{
	Clear();
}

HRESULT noFXEffect::CreateFromFile(PDIRECT3DDEVICE9 pDevice,PCTSTR pszFileName)
{
	
#ifdef _DEBUG
	DWORD compileFlags = D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION | D3DXSHADER_USE_LEGACY_D3DX9_31_DLL
		| D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT | D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT ;	
#else
	DWORD compileFlags = D3DXSHADER_USE_LEGACY_D3DX9_31_DLL;
#endif

	ID3DXEffectPool* pEffectPool = g_renderSystem->GetD3DEffectPool();
	ASSERT(pEffectPool);

	LPCSTR vsProfile = D3DXGetVertexShaderProfile(g_renderSystem->GetD3DDevice());
	LPCSTR psProfile = D3DXGetPixelShaderProfile(g_renderSystem->GetD3DDevice());

	if (0 == vsProfile)
	{
		ASSERT("Invalid Vertex Shader profile! Fallback to vs_2_0!\n");
		vsProfile = "vs_2_0";
	}

	if (0 == psProfile)
	{
		ASSERT("Invalid Pixel Shader profile! Fallback to ps_2_0!\n");
		psProfile = "ps_2_0";
	}

	// create macro definitions for shader compiler
	D3DXMACRO defines[] = {
		{ "VS_PROFILE", vsProfile },
		{ "PS_PROFILE", psProfile },
		{ 0, 0 },
	};

	Clear();
	LPD3DXBUFFER pCompileErrors = NULL;

#ifdef UNICODE
	std::wstring includePath;
#else 
	std::string includePath;
#endif

	noD3DXEffectShaderInclude includeHandler(includePath.c_str());

	HRESULT hRetval;
	if (compileFlags)
	{	
		hRetval = D3DXCreateEffectFromFile(pDevice,
			pszFileName,	// File name
			defines,	// Macro definitions
			&includeHandler,	// Includes
			compileFlags,	// Flags
			pEffectPool,	// Effect Pool Object
			&m_pEffect,
			&pCompileErrors);
	}
	else {
		hRetval = D3DXCreateEffectFromFile(pDevice,
			pszFileName,	// File name
			defines,	// Macro definitions
			&includeHandler,	// Includes
			compileFlags,	// Flags
			pEffectPool,	// Effect Pool Object
			&m_pEffect,
			&pCompileErrors);
	}

	if (pCompileErrors)
	{
		SXTRACE("SXEffect WARN: D3DX Effect Compiler said:\n");
		SXTRACE((PCTSTR)pCompileErrors->GetBufferPointer());
	}
	SAFE_RELEASE(pCompileErrors);

	if (SUCCEEDED(hRetval))
	{	
		m_bValidated = false;
		m_bNotValidate = false;
		this->ValidateEffect();
	}
	return hRetval;
}

HRESULT noFXEffect::AttachToExisting(LPD3DXEFFECT pEffect)
{
	if (!pEffect)
		return E_POINTER;
	pEffect->AddRef();
	Clear();
	m_pEffect = pEffect;
	return S_OK;
}

void noFXEffect::Clear(void)
{
	// Free all cached parameters
	noEffParamsHash::const_iterator itParams = m_hashParams.begin();
	while (itParams != m_hashParams.end())
	{
		noEffParam *pParam = (itParams++)->second;
		SAFE_DELETE(pParam);
	}
	m_hashParams.clear();
	SAFE_RELEASE(m_pEffect);
}

noEffParam& noFXEffect::operator () (PCSTR pszParamName,D3DXHANDLE hParent)
{
	noEffParamsHash::const_iterator itParam = m_hashParams.find(pszParamName);
	if (itParam != m_hashParams.end())
		return *(itParam->second);

	D3DXHANDLE hParam = m_pEffect->GetParameterByName(hParent,pszParamName);
	if (!hParam)
		return m_paramInvalid;

	noEffParam *pRetval = new noEffParam(this,hParam);
	if (!pRetval)
		return m_paramInvalid;

	m_hashParams[pszParamName] = pRetval;
	return *pRetval;
}

noEffParam& noFXEffect::GetTechnique(PCSTR pszTechniqueName)
{
	// Was it requested before? If so, then it's stored in our hash table
	noEffParamsHash::const_iterator itTec = m_hashParams.find(pszTechniqueName);
	if (itTec != m_hashParams.end())
		return *(itTec->second);

	// Nope, try to get it from ID3DXEffect
	D3DXHANDLE hTechnique = m_pEffect->GetTechniqueByName(pszTechniqueName);
	if (!hTechnique)
		return m_paramInvalid;

	noEffParam *pParam = new noEffParam(this,hTechnique);
	if (!pParam)
		return m_paramInvalid;

	m_hashParams[pszTechniqueName] = pParam;
	return *pParam;
}

void noFXEffect::SetAnimWeight(float fTime)
{
	// Loop on all parameters, asking them to set their time
	// to the new value which should be in the range [0,1]
	noEffParamsHash::const_iterator itParam = m_hashParams.begin();
	while (itParam != m_hashParams.end())
	{
		noEffParam *pParam = (itParam++)->second;
		if (pParam->m_pAnim)
			pParam->m_pAnim->Update(fTime,*pParam);
	}
}

HRESULT noFXEffect::OnDeviceLoss()
{
	if (!m_pEffect)
		return S_OK;
	return m_pEffect->OnLostDevice();
}

HRESULT noFXEffect::OnDeviceRestore()
{
	if (!m_pEffect)
		return S_OK;
	return m_pEffect->OnResetDevice();
}

int noFXEffect::Begin( bool bSaveState )
{
	ASSERT(m_pEffect);

	if(!m_bValidated)
	{
		ValidateEffect();
	}

	if(m_bNotValidate)
	{
		return 0;
	}

	UINT numPasses;
	DWORD dwFlags;
	if(bSaveState)
	{
		// save all state
		dwFlags = 0;
	}
	else 
	{
		// save no state
		dwFlags = D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE;
	}

	HRESULT hr = m_pEffect->Begin(&numPasses, dwFlags);
	SUCCEEDED(hr);

	return numPasses;

}

void noFXEffect::BeginPass( int pass )
{
	ASSERT(m_pEffect);
	ASSERT(m_bValidated && !m_bNotValidate);
	HRESULT hr = m_pEffect->BeginPass(pass);
	SUCCEEDED(hr);
}

void noFXEffect::CommitChanges()
{
	ASSERT(m_pEffect);
	ASSERT(m_bValidated && !m_bNotValidate);
	HRESULT hr = m_pEffect->CommitChanges();
	SUCCEEDED(hr);
}

void noFXEffect::End()
{
	ASSERT(m_pEffect);
	ASSERT(m_bValidated && !m_bNotValidate);

	if(!m_bNotValidate)
	{	
		HRESULT hr = m_pEffect->End();
		SUCCEEDED(hr);
		//DXTrace(_T("End() failed on effect"));
	}
}

void noFXEffect::EndPass()
{
	ASSERT(m_pEffect);
	ASSERT(m_bValidated && !m_bNotValidate);
	HRESULT hr = m_pEffect->EndPass();
	SUCCEEDED(hr);
}

bool noFXEffect::SetTechnique(const char* szTechnique)
{
	ASSERT(szTechnique);
	ASSERT(m_pEffect);

	D3DXHANDLE hTechnique = m_pEffect->GetTechniqueByName(szTechnique);
	if(0 == hTechnique)
	{
		LOG_ERR << _T("nD3D9Shader::SetTechnique(%s): technique not found in shader file %s!\n") 
			<< szTechnique ;

		return false;
	}

	m_bNeedSoftwareVertexProcessing = false;
	HRESULT hr;

	/*if(m_pRenderer->GetSWVertexCapable())
	{
		D3DXHANDLE hPass = m_pD3DEffect->GetPass(hTechnique, 0);
		U2ASSERT(0 != hPass);
		D3DXPASS_DESC passDesc = { 0 };
		hr = m_pD3DEffect->GetPassDesc(hPass, &passDesc);
		U2ASSERT(SUCCEEDED(hr));
		if(passDesc.pVertexShaderFunction)
		{
			m_bNeedSoftwareVertexProcessing = true;
		}

	}*/

	hr = m_pEffect->SetTechnique(hTechnique);

	if(FAILED(hr))
	{
		LOG_ERR << _T("nD3D9Shader::SetTechnique(%s) on shader %s failed! : Error Desc : %s\n") << szTechnique << DXGetErrorDescription(hr);
		return false;
	}

	return true;
}

void noFXEffect::ValidateEffect()
{
	ASSERT(!m_bValidated);
	ASSERT(m_pEffect);
	ASSERT(g_renderSystem->GetD3DDevice());

	LPDIRECT3DDEVICE9 pD3DDev = g_renderSystem->GetD3DDevice();
	ASSERT(pD3DDev);

	HRESULT hr;

	// set on first technique that validates correctly
	D3DXHANDLE hTechnique = NULL;
	hr = m_pEffect->FindNextValidTechnique(0, &hTechnique);

	// NOTE: DON'T change this to SUCCEEDED(), since FindNextValidTechnique() may
	// return S_FALSE, which the SUCCEEDED() macro interprets as a success code!
	if(D3D_OK == hr)
	{
		// technique could be validated
		D3DXTECHNIQUE_DESC desc;
		m_pEffect->GetTechniqueDesc(m_pEffect->GetTechnique(0), &desc);
		D3DXHANDLE hTechnique = m_pEffect->GetTechniqueByName(desc.Name);
		this->GetTechnique(desc.Name).Annotation(desc.Name).SetHandle(hTechnique);		
		this->SetTechnique(desc.Name);
		this->m_bValidated = true;
		this->m_bNotValidate = false;
		this->UpdateAttbHandles();
	}

}

void noFXEffect::UpdateAttbHandles()
{
	ASSERT(m_pEffect);
	HRESULT hr;
		
	D3DXEFFECT_DESC effectDesc = { 0 } ;
	hr = m_pEffect->GetDesc(&effectDesc);

	uint32 curParamIdx;
	for(curParamIdx =0; curParamIdx < effectDesc.Parameters; ++curParamIdx)
	{
		D3DXHANDLE curParamHandle = m_pEffect->GetParameter(NULL, curParamIdx);
		ASSERT(NULL != curParamHandle);

		D3DXPARAMETER_DESC paramDesc = { 0 };
		hr = m_pEffect->GetParameterDesc(curParamHandle, &paramDesc);

		std::string strSCM;
		strSCM.assign(paramDesc.Name);

		m_hashParams[strSCM]->Annotation(strSCM.c_str()).SetHandle(curParamHandle);
	}


}
////////////////// End of File : SXEffect.cpp //////////////////