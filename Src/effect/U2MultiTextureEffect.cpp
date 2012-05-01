#include <U2_3D/Src/U23DLibPCH.h>
#include "U2MultiTextureEffect.h"

IMPLEMENT_RTTI(U2MultiTextureEffect, U2ShaderEffect);

U2MultiTextureEffect::U2MultiTextureEffect(uint32 uTextureCnt)
:U2ShaderEffect(1)

{	
	m_ppTextureNameArray = NULL;
	SetTextureCnt(uTextureCnt);	
}

U2MultiTextureEffect::U2MultiTextureEffect()
{
	m_uTextureCnt = 0;
	m_ppTextureNameArray = 0;
}

U2MultiTextureEffect::~U2MultiTextureEffect()
{
	U2_FREE(m_ppTextureNameArray);
	m_ppTextureNameArray = NULL;
}


void U2MultiTextureEffect::SetTextureCnt(uint32 uTextureCnt)
{
	U2ASSERT(uTextureCnt > 0);
	m_uTextureCnt = uTextureCnt;
	U2_FREE(m_ppTextureNameArray);
	m_ppTextureNameArray = NULL;
	m_ppTextureNameArray =  U2_ALLOC(U2DynString*, m_uTextureCnt);

	memset(m_ppTextureNameArray, 0, sizeof(U2DynString*) * m_uTextureCnt);

	m_alphaStateArray.Resize(m_uTextureCnt);
	SetDefaultAlphaState();
}


uint32 U2MultiTextureEffect::GetTextureCnt() const
{
	return m_uTextureCnt;
}


void U2MultiTextureEffect::SetTextureName(int i, const U2DynString& texName)
{
	U2ASSERT(0 <= i && i < m_uTextureCnt);
	m_ppTextureNameArray[i] = U2_NEW U2DynString(texName);
}


const U2DynString& U2MultiTextureEffect::GetTextureName(int i) const
{
	U2ASSERT(0 <= i && i < m_uTextureCnt);
	return *m_ppTextureNameArray[i];	
}


void U2MultiTextureEffect::Configure()
{
	if(m_uTextureCnt == 1)
	{
		m_vsArray.SetElem(0, U2_NEW U2VertexShader(_T("v_Texture.dx9.wmsp")));
		m_psArray.SetElem(0, U2_NEW U2PixelShader(_T("p_Texture.dx9.wmsp")));
		m_psArray.GetElem(0)->SetTexture(0, *m_ppTextureNameArray[0]);
		return;
	}

	const size_t uNumberSize= 4;
	TCHAR szNumber[uNumberSize];

	// In a single-effect drawing pass, texture 0 is a source to be blended
	// with a nonexistent destination.  As such, we think of the source mode
	// as SBF_ONE and the destination mode as SDF_ZERO.
	U2DynString vShaderName(_T("v_T0d2"));	
	U2DynString pShaderName(_T("p_T0s1d0"));
	int i;
	for (i = 1; i < m_uTextureCnt; i++)
	{
		vShaderName.Concatenate(_T("T"));		
		pShaderName.Concatenate(_T("T"));		
		_sntprintf_s(szNumber,uNumberSize,_T("%d"),i);				
		vShaderName.Concatenate(szNumber);
		vShaderName.Concatenate(_T("d2"));
		pShaderName.Concatenate(U2DynString(szNumber));

		U2AlphaState* pkAS = m_alphaStateArray.GetElem(i);

		// Source blending mode.
		pShaderName.Concatenate(_T("s"));
		_sntprintf_s(szNumber,uNumberSize,_T("%d"),(int)pkAS->GetSrcBlendMode());
		pShaderName.Concatenate(U2DynString(szNumber));

		// Destination blending mode.
		pShaderName.Concatenate(_T("d"));
		_sntprintf_s(szNumber,uNumberSize,_T("%d"),(int)pkAS->GetDestBlendMode());
		pShaderName.Concatenate(U2DynString(szNumber));
	}
	vShaderName.Concatenate(U2DynString(_T("PassThrough.dx9.wmsp")));
	pShaderName.Concatenate(U2DynString(_T(".dx9.wmsp")));

	m_vsArray.SetElem(0, U2_NEW U2VertexShader(vShaderName));
	m_psArray.SetElem(0, U2_NEW U2PixelShader(pShaderName));
	for(i=0; i < m_uTextureCnt; ++i)
	{
		m_psArray.GetElem(0)->SetTexture(i, *m_ppTextureNameArray[i]);
	}


}

