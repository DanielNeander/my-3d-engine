#include <U2_3D/Src/U23DLibPCH.h>
#include "U2D3DXEffectShaderInclude.h"
#include "U2D3DXEffectShader.h"
#include <dxerr.h>


U2D3DXEffectShader* U2D3DXEffectShader::ms_pHead = 0;
U2D3DXEffectShader* U2D3DXEffectShader::ms_pTail = 0;

U2D3DXEffectShader* U2D3DXEffectShader::GetNext()
{
	return m_pNext;
}

U2D3DXEffectShader* U2D3DXEffectShader::GetPRev()
{
	return m_pPrev;
}

U2D3DXEffectShader* U2D3DXEffectShader::GetHead()
{
	return ms_pHead;
}

U2D3DXEffectShader* U2D3DXEffectShader::GetTail()
{
	return ms_pTail;
}

void U2D3DXEffectShader::AddShaderToList()
{
	if (!ms_pHead)
		ms_pHead = this;
	if (ms_pTail)
	{
		ms_pTail->m_pNext = this;
		m_pPrev = ms_pTail;
	}
	else
	{
		m_pPrev = 0;
	}
	ms_pTail = this;
	m_pNext = 0;
}


void U2D3DXEffectShader::RemoveShaderFromList()
{
	if (ms_pHead == this)
		ms_pHead = m_pNext;
	if (ms_pTail == this)
		ms_pTail = m_pPrev;

	if (m_pPrev)
		m_pPrev->m_pNext = m_pNext;
	if (m_pNext)
		m_pNext->m_pPrev = m_pPrev;

}


U2D3DXEffectShader::U2D3DXEffectShader(U2Dx9Renderer* pRenderer) 
	:m_pD3DEffect(0),
	m_bBeginPass(false),
	m_bValidated(false),
	m_bNotValidate(false),
	m_bNeedSoftwareVertexProcessing(false),
	m_pRenderer(pRenderer)	
{
	memset(m_ahParamHandles, 0, sizeof(m_ahParamHandles));

	AddShaderToList();
}

U2D3DXEffectShader::~U2D3DXEffectShader()
{
	UnloadResource();
	RemoveShaderFromList();		
}

void U2D3DXEffectShader::SetFilename(const U2DynString& szFilename)
{
	m_szFilename = szFilename;
}


const U2DynString& U2D3DXEffectShader::GetFilename() const
{
	return m_szFilename;
}


bool U2D3DXEffectShader::LoadResource()
{
	U2ASSERT(0 == m_pD3DEffect);

	HRESULT hr;
	
	IDirect3DDevice9* pD3DDev = m_pRenderer->GetD3DDevice();
	U2ASSERT(pD3DDev);

	U2FilePath fPath;		
	TCHAR fullPath[MAX_PATH];
	// StackString Memory Leak...
	U2DynString includePath(FX_SHADER_PATH);
	includePath += _T("\\2.0");

	fPath.ConvertToAbs(fullPath, MAX_PATH * sizeof(TCHAR) , m_szFilename.Str(), includePath);		

	U2File* pFile = U2File::GetFile(fullPath, U2File::READ_ONLY);
	if(!pFile)
	{
		U2_DELETE pFile;
		return false;
	}

	uint32 uBuffLen = pFile->GetfileSize();
	if(uBuffLen == 0)
	{
		U2_DELETE pFile;
		return false;
	}

	unsigned char* pBuffer = U2_ALLOC(unsigned char, uBuffLen);
	pFile->Read(pBuffer, uBuffLen);

	U2_DELETE pFile;

	ID3DXBuffer* pErrorBuffer = NULL;

#ifdef DEBUG_SHADER
	DWORD compileFlags = D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION | D3DXSHADER_USE_LEGACY_D3DX9_31_DLL
		| D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT | D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT ;	
#else
	DWORD compileFlags = D3DXSHADER_USE_LEGACY_D3DX9_31_DLL;
#endif

	//U2DynString includePath(FX_SHADER_PATH);
	//includePath += _T("\\2.0");
	
	U2D3DXEffectShaderInclude includeHandler(includePath);

	ID3DXEffectPool* pEffectPool = m_pRenderer->GetD3DEffectPool();
	U2ASSERT(pEffectPool);

	// get the highest supported shader profiles
//	LPCSTR vsProfile, psProfile;
//#ifdef UNICODE 
//	 vsProfile = ToUnicode( D3DXGetVertexShaderProfile(pD3DDev) );
//	 psProfile = ToUnicode( D3DXGetPixelShaderProfile(pD3DDev) );
//#else 
//	vsProfile = D3DXGetVertexShaderProfile(pD3DDev);
//	psProfile = D3DXGetPixelShaderProfile(pD3DDev);
//#endif
	LPCSTR vsProfile = D3DXGetVertexShaderProfile(pD3DDev);
	LPCSTR psProfile = D3DXGetPixelShaderProfile(pD3DDev);

	if (0 == vsProfile)
	{
		FDebug("Invalid Vertex Shader profile! Fallback to vs_2_0!\n");
		vsProfile = "vs_2_0";
	}

	if (0 == psProfile)
	{
		FDebug("Invalid Pixel Shader profile! Fallback to ps_2_0!\n");
		psProfile = "ps_2_0";
	}

	// create macro definitions for shader compiler
	D3DXMACRO defines[] = {
		{ "VS_PROFILE", vsProfile },
		{ "PS_PROFILE", psProfile },
		{ 0, 0 },
	};

	// create effect
	if (compileFlags)
	{
		hr = D3DXCreateEffectFromFile(
			pD3DDev,            // pDevice
			fullPath,   // File name
			defines,            // pDefines
			&includeHandler,    // pInclude
			compileFlags,       // Flags
			pEffectPool,         // pPool
			&m_pD3DEffect,    // ppEffect
			&pErrorBuffer);      // ppCompilationErrors
	}
	else
	{
		hr = D3DXCreateEffect(
			pD3DDev,            // pDevice
			pBuffer,             // pFileData
			uBuffLen,           // DataSize
			defines,            // pDefines
			&includeHandler,    // pInclude
			compileFlags,       // Flags
			pEffectPool,         // pPool
			&(m_pD3DEffect),    // ppEffect
			&pErrorBuffer);      // ppCompilationErrors
	}

	U2_FREE(pBuffer);
	pBuffer = NULL;

	if (FAILED(hr))
	{
		FDebug("nD3D9Shader: failed to load fx file '%s' with:\n\n%s\n",
			fullPath,
			pErrorBuffer ? pErrorBuffer->GetBufferPointer() : "No D3DX error message.");
		if (pErrorBuffer)
		{
			pErrorBuffer->Release();
		}
		return false;
	}
	U2ASSERT(m_pD3DEffect);

	m_bValidated = false;
	m_bNotValidate = false;

	this->ValidateEffect();

	return true;
}

void U2D3DXEffectShader::UnloadResource()
{	
	SAFE_RELEASE(m_pD3DEffect);	

	m_currAttb.Clear();	
}


void U2D3DXEffectShader::SetBool(U2FxShaderState::Param eParam, bool val)
{
	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));
	m_currAttb.SetArg(eParam, U2ShaderArg(val));
	HRESULT hr = m_pD3DEffect->SetBool(m_ahParamHandles[eParam], val);	

	U2_DXTrace(hr, _T("SetBool() on shader failed!"));
}


void U2D3DXEffectShader::SetBoolArray(U2FxShaderState::Param eParam, 
									  const bool* pArray, int count)
{
	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));	
	HRESULT hr = m_pD3DEffect->SetBoolArray(m_ahParamHandles[eParam], 
		(const BOOL*)pArray, count);	
	
	U2_DXTrace(hr, _T("SetBoolArray() on shader failed!"));
}


void U2D3DXEffectShader::SetInt(U2FxShaderState::Param eParam, int val)
{
	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));
	m_currAttb.SetArg(eParam, U2ShaderArg(val));
	HRESULT hr = m_pD3DEffect->SetInt(m_ahParamHandles[eParam], val);			

	U2_DXTrace(hr, _T("SetInt() on shader failed!"));
}

void U2D3DXEffectShader::SetIntArray(U2FxShaderState::Param eParam,
									 const int* pArray, int count)
{

	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));	
	HRESULT hr = m_pD3DEffect->SetIntArray(m_ahParamHandles[eParam], 
		pArray, count);	

	U2_DXTrace(hr, _T("SetIntArray() on shader failed!"));
}


void U2D3DXEffectShader::SetFloat(U2FxShaderState::Param eParam, float val)
{
	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));
	m_currAttb.SetArg(eParam, U2ShaderArg(val));
	HRESULT hr = m_pD3DEffect->SetFloat(m_ahParamHandles[eParam], val);			

	U2_DXTrace(hr, _T("SetFloat() on shader failed!"));	
}



void U2D3DXEffectShader::SetFloatArray(U2FxShaderState::Param eParam, 
									   const float* pArray, int count)
{
	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));	
	HRESULT hr = m_pD3DEffect->SetFloatArray(m_ahParamHandles[eParam], 
		pArray, count);	

	U2_DXTrace(hr, _T("SetFloatArray() on shader failed!"));
}


//void U2D3DXEffectShader::SetVector3(U2FxShaderState::Param eParam, const D3DXVECTOR3& val)
//{	
//	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));
//	static D3DXVECTOR4 v;
//	v.x = val.x; v.y = val.y; v.z = val.z; v.w = 1.f;
//
//	m_currAttb.SetArg(eParam, U2ShaderArg(v));
//	HRESULT hr = m_pD3DEffect->SetVector(m_ahParamHandles[eParam], &v);			
//}

void U2D3DXEffectShader::SetFloat4(U2FxShaderState::Param eParam, const U2Float4& val)
{
	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));
	m_currAttb.SetArg(eParam, U2ShaderArg(val));
	HRESULT hr = m_pD3DEffect->SetVector(m_ahParamHandles[eParam], (CONST D3DXVECTOR4*)&val);		
}


void U2D3DXEffectShader::SetFloat4Array(U2FxShaderState::Param eParam, 
										const U2Float4* pArray, int count)
{
	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));	
	HRESULT hr = m_pD3DEffect->SetVectorArray(m_ahParamHandles[eParam], 
		(CONST D3DXVECTOR4*)pArray, count);	

	U2_DXTrace(hr, _T("SetFloat4Array() on shader failed!"));
}


void U2D3DXEffectShader::SetVector4(U2FxShaderState::Param eParam, const D3DXVECTOR4& val)
{
	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));
	m_currAttb.SetArg(eParam, U2ShaderArg(val));
	HRESULT hr = m_pD3DEffect->SetVector(m_ahParamHandles[eParam], &val);			

	U2_DXTrace(hr, _T("SetVector4() on shader failed!"));	
}


void U2D3DXEffectShader::SetVector4Array(U2FxShaderState::Param eParam, 
										 const D3DXVECTOR4* pArray, int count)
{
	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));	
	HRESULT hr = m_pD3DEffect->SetVectorArray(m_ahParamHandles[eParam], 
		pArray, count);	

	U2_DXTrace(hr, _T("SetVectorArray() on shader failed!"));
}


//void U2D3DXEffectShader::SetMatrix(U2FxShaderState::Param eParam, const D3DXMATRIX& val)
//{
//	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));
//	m_currAttb.SetArg(eParam, U2ShaderArg(val));
//	HRESULT hr = m_pD3DEffect->SetMatrix(m_ahParamHandles[eParam], &val);			
//}


void U2D3DXEffectShader::SetMatrixArray(U2FxShaderState::Param eParam, 
										const D3DXMATRIX* pArray, int count)
{
	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));	
	HRESULT hr = m_pD3DEffect->SetMatrixArray(m_ahParamHandles[eParam], 
		pArray, count);	
}


void U2D3DXEffectShader::SetMatrixPointerArray(U2FxShaderState::Param eParam, 
											   const D3DXMATRIX** ppArray, int count)
{
	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));	
	HRESULT hr = m_pD3DEffect->SetMatrixPointerArray(m_ahParamHandles[eParam], 
		ppArray, count);	
}


void U2D3DXEffectShader::SetTexture(U2FxShaderState::Param eParam, 
									const U2Dx9BaseTexture* pTex)
{
	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));	

	if(0 == pTex)
	{
		HRESULT hr = m_pD3DEffect->SetTexture(m_ahParamHandles[eParam], 0);
		m_currAttb.SetArg(eParam, U2ShaderArg(NULL));
	}
	else 
	{
		uint32 curTexUniqueId = 0;
		if(m_currAttb.IsValid(eParam))
		{
			U2Dx9BaseTexture* pCurTex = this->m_currAttb.GetArg(eParam).GetTexture();
			if(pCurTex)
			{
				curTexUniqueId = pCurTex->GetUniqueID();
			}
		}

		if(!m_currAttb.IsValid(eParam) || (curTexUniqueId != pTex->GetUniqueID()))
		{
			m_currAttb.SetArg(eParam, U2ShaderArg(pTex));
			HRESULT hr = m_pD3DEffect->SetTexture(m_ahParamHandles[eParam], 
				pTex->GetD3DTex());			
		}
	}

}


void U2D3DXEffectShader::SetAttribute(const U2ShaderAttribute& attb)
{
	int i;
	HRESULT hr;

	int numValidAttbEntries = attb.GetNumValidAttbEntries();

//#ifdef _DEBUG
//	for(i=0; i < m_currAttb.GetNumValidAttbEntries(); ++i)
//	{
//		U2FxShaderState::Param eCurParam = m_currAttb.GetParam(i);
//
//		U2DynString str;
//		str.Format(_T("Curr Param Type: %s\n"), U2FxShaderState::ParamToString(eCurParam));
//		FILE_LOG(logDEBUG) << str.Str();
//	}
//
//	for(i=0; i < numValidAttbEntries; ++i)
//	{
//		U2FxShaderState::Param eCurParam = attb.GetParam(i);
//
//		U2DynString str;
//		str.Format(_T("arg Param Type: %s\n"), U2FxShaderState::ParamToString(eCurParam));
//		FILE_LOG(logDEBUG) << str.Str();
//	}
//#endif 

	for(i=0; i < numValidAttbEntries; ++i)
	{
		U2FxShaderState::Param eCurParam = attb.GetParam(i);

		D3DXHANDLE handle = m_ahParamHandles[eCurParam];
		if(handle != 0)
		{
			const U2ShaderArg& curArg = attb.GetArg(i);

			if(curArg.GetType() == U2ShaderArg::AT_VOID)
			{
				continue;
			}

			if(!m_currAttb.IsValid(eCurParam) || 
				!(curArg == m_currAttb.GetArg(eCurParam)))
			{
				m_currAttb.SetArg(eCurParam, curArg);			

				switch(curArg.GetType())
				{
				case U2ShaderArg::AT_BOOL:
					hr = m_pD3DEffect->SetBool(handle, curArg.GetBool());
					break;
				case U2ShaderArg::AT_INT:
					hr = m_pD3DEffect->SetInt(handle, curArg.GetInt());
					break;
				case U2ShaderArg::AT_FLOAT:
					hr = m_pD3DEffect->SetFloat(handle, curArg.GetFloat());
					break;
				case U2ShaderArg::AT_FLOAT4:
					hr = m_pD3DEffect->SetVector(handle, (CONST D3DXVECTOR4*)&curArg.GetFloat4());
					break;

				case U2ShaderArg::AT_MATRIX44:
					hr = m_pD3DEffect->SetMatrix(handle, curArg.GetMatrix44());
					break;

				case U2ShaderArg::AT_TEXTURE:
					hr = m_pD3DEffect->SetTexture(handle, curArg.GetTexture()->GetD3DTex());
					break;
				}
			}
		}
	}
}


// Create or append an instance stream declaration for this shader
int U2D3DXEffectShader::UpdateInstatnceStreamDecl(U2StreamInstance::Declaration& decl)
{
	U2ASSERT(m_pD3DEffect);

	int numAppended = 0;

	HRESULT hr;
	D3DXEFFECT_DESC fxDesc;
	hr = m_pD3DEffect->GetDesc(&fxDesc);

	// for each parameter...
	UINT32 paramIdx;
	for(paramIdx = 0; paramIdx < fxDesc.Parameters; ++paramIdx)
	{
		D3DXHANDLE paramHandle = m_pD3DEffect->GetParameter(NULL, paramIdx);
		U2ASSERT(paramHandle);

		D3DXHANDLE annHandle = m_pD3DEffect->GetAnnotationByName(paramHandle, "Instance");
		if(annHandle)
		{
			BOOL bVal;
			hr = m_pD3DEffect->GetBool(annHandle, &bVal);
			SUCCEEDED(hr);
			if(bVal)
			{
				U2FxShaderState::Param eParam = 
					D3DXParamFromUserParam(paramHandle);

				D3DXPARAMETER_DESC paramDesc;
				hr = m_pD3DEffect->GetParameterDesc(paramHandle, &paramDesc);

				U2ShaderArg::ArgType type = U2ShaderArg::AT_VOID;
				if(paramDesc.Type == D3DXPT_FLOAT)
				{
					switch(paramDesc.Class)
					{
					case D3DXPC_SCALAR:
						type = U2ShaderArg::AT_FLOAT;
						break;
					case D3DXPC_VECTOR:
						type =U2ShaderArg::AT_FLOAT4;
						break;

					case D3DXPC_MATRIX_ROWS:
					case D3DXPC_MATRIX_COLUMNS:
						type =U2ShaderArg::AT_MATRIX44;
						break;
	
					}
				}
				if(U2ShaderArg::AT_VOID == type)
				{
					FDebug("nShader2: Invalid data type for instance parameter '%s' in shader '%s'!",
						paramDesc.Name, m_szFilename.Str());

					return 0;
				}

				// append instance stream component (if not exists yet)
				int i; 
				bool bParamExists = false;
				for(i=0; i < (int)decl.Size(); ++i)
				{
					if(decl[i]->GetSCM() == eParam)
					{
						bParamExists = true;
						break;
					}
				}
				if(!bParamExists)
				{
					U2StreamInstance::Component* pStreamComponent 
						= U2_NEW U2StreamInstance::Component(type, eParam);
					decl.AddElem(pStreamComponent);
					numAppended++;
				}			
			}
		}
	}
	return numAppended;
}


bool U2D3DXEffectShader::HasTechnique(const char* szTechnique) const	
{
	U2ASSERT(szTechnique);
	U2ASSERT(m_pD3DEffect);
	D3DXHANDLE h = m_pD3DEffect->GetTechniqueByName(szTechnique);
	return (0 != h);
}


bool U2D3DXEffectShader::SetTechnique(const char* szTechnique)
{
	U2ASSERT(szTechnique);
	U2ASSERT(m_pD3DEffect);

	D3DXHANDLE hTechnique = m_pD3DEffect->GetTechniqueByName(szTechnique);
	if(0 == hTechnique)
	{
		FDebug("nD3D9Shader::SetTechnique(%s): technique not found in shader file %s!\n", 
			szTechnique, m_szFilename.Str());

		return false;
	}

	m_bNeedSoftwareVertexProcessing = false;
	HRESULT hr;
	
	if(m_pRenderer->GetSWVertexCapable())
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

	}

	hr = m_pD3DEffect->SetTechnique(hTechnique);
	
	if(FAILED(hr))
	{
		FDebug("nD3D9Shader::SetTechnique(%s) on shader %s failed! : Error Desc : %s\n", szTechnique, m_szName.Str(), DXGetErrorDescription(hr));
		return false;
	}

	return true;
}

// Get Current Technique
const char* U2D3DXEffectShader::GetTechnique() const
{
	U2ASSERT(m_pD3DEffect);
	return m_pD3DEffect->GetCurrentTechnique();
}

bool U2D3DXEffectShader::IsParamUsed(U2FxShaderState::Param eParam)
{
	U2ASSERT(m_pD3DEffect && (0 <= eParam && eParam < U2FxShaderState::NumParameters));
	return (0 != m_ahParamHandles[eParam]);
}


int U2D3DXEffectShader::Begin(bool bSaveState)
{
	U2ASSERT(m_pD3DEffect);

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

	HRESULT hr = m_pD3DEffect->Begin(&numPasses, dwFlags);
	SUCCEEDED(hr);

	return numPasses;
}

void U2D3DXEffectShader::BeginPass(int pass)
{
	U2ASSERT(m_pD3DEffect);
	U2ASSERT(m_bValidated && !m_bNotValidate);
	HRESULT hr = m_pD3DEffect->BeginPass(pass);
	SUCCEEDED(hr);
}

void U2D3DXEffectShader::CommitChanges()
{
	U2ASSERT(m_pD3DEffect);
	U2ASSERT(m_bValidated && !m_bNotValidate);
	HRESULT hr = m_pD3DEffect->CommitChanges();
	SUCCEEDED(hr);

}

void U2D3DXEffectShader::End()
{
	U2ASSERT(m_pD3DEffect);
	U2ASSERT(m_bValidated && !m_bNotValidate);
	
	if(!m_bNotValidate)
	{	
		 HRESULT hr = m_pD3DEffect->End();
		SUCCEEDED(hr);
		U2_DXTrace(hr, _T("End() failed on effect"));

	}
}


void U2D3DXEffectShader::EndPass()
{
	U2ASSERT(m_pD3DEffect);
	U2ASSERT(m_bValidated && !m_bNotValidate);
	HRESULT hr = m_pD3DEffect->EndPass();
	SUCCEEDED(hr);
}


void U2D3DXEffectShader::OnLostDevice()
{
	U2ASSERT(m_pD3DEffect);
	HRESULT hr = m_pD3DEffect->OnLostDevice();
	SUCCEEDED(hr);

	
	// flush my current parameters (important! otherwise, seemingly redundant
	// state will not be set after OnRestore())!
	m_currAttb.Clear();
}


void U2D3DXEffectShader::OnResetDevice()
{
	U2ASSERT(m_pD3DEffect);
	HRESULT hr = m_pD3DEffect->OnResetDevice();
	SUCCEEDED(hr);
}

	
void U2D3DXEffectShader::ValidateEffect()
{
	U2ASSERT(!m_bValidated);
	U2ASSERT(m_pD3DEffect);
	U2ASSERT(m_pRenderer->GetD3DDevice());

	LPDIRECT3DDEVICE9 pD3DDev = m_pRenderer->GetD3DDevice();
	U2ASSERT(pD3DDev);

	HRESULT hr;

	// set on first technique that validates correctly
	D3DXHANDLE hTechnique = NULL;
	hr = m_pD3DEffect->FindNextValidTechnique(0, &hTechnique);

	// NOTE: DON'T change this to SUCCEEDED(), since FindNextValidTechnique() may
	// return S_FALSE, which the SUCCEEDED() macro interprets as a success code!
	if(D3D_OK == hr)
	{
		// technique could be validated
		D3DXTECHNIQUE_DESC desc;
		m_pD3DEffect->GetTechniqueDesc(m_pD3DEffect->GetTechnique(0), &desc);
		this->SetTechnique(desc.Name);		
		this->m_bValidated = true;
		this->m_bNotValidate = false;
		this->UpdateAttbHandles();
	}
}

	
void U2D3DXEffectShader::UpdateAttbHandles()
{
	U2ASSERT(m_pD3DEffect);
	HRESULT hr;

	memset(m_ahParamHandles, 0, sizeof(m_ahParamHandles));

	D3DXEFFECT_DESC effectDesc = { 0 } ;
	hr = m_pD3DEffect->GetDesc(&effectDesc);

	uint32 curParamIdx;
	for(curParamIdx =0; curParamIdx < effectDesc.Parameters; ++curParamIdx)
	{
		D3DXHANDLE curParamHandle = m_pD3DEffect->GetParameter(NULL, curParamIdx);
		U2ASSERT(NULL != curParamHandle);

		D3DXPARAMETER_DESC paramDesc = { 0 };
		hr = m_pD3DEffect->GetParameterDesc(curParamHandle, &paramDesc);
		
		U2String strSCM;
#ifdef UNICODE 
		strSCM.assign(ToUnicode(paramDesc.Name));
#else 
		strSCM.assign(paramDesc.Name);
#endif 
		U2FxShaderState::Param eParam = U2FxShaderState::StringToParam(paramDesc.Name);
			if(eParam != U2FxShaderState::InvalidParameter)
		{
			m_ahParamHandles[eParam] = curParamHandle;
		}

	}

}

U2FxShaderState::Param U2D3DXEffectShader::D3DXParamFromUserParam(D3DXHANDLE handle)
{
	int i;
	for(i=0; i < U2FxShaderState::NumParameters; ++i)
	{
		if(m_ahParamHandles[i] == handle)
		{
			return (U2FxShaderState::Param)i;
		}
	}

	return U2FxShaderState::InvalidParameter;
}

