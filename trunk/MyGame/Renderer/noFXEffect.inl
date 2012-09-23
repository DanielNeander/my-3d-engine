///////////////////////////////////////////////////////////////////////
// File : SXEffect.inl
// Description : D3DX Effects Parameter Manipulation Class
//				 inline functions implementations.
// Author : Wessam Bahnassi, In|Framez
//
///////////////////////////////////////////////////////////////////////

#ifndef _SXEFFECT_INL
#define _SXEFFECT_INL

//////////////////////////////// SXEffAnimation Inline Functions Implementations ////////////////////////////////
D3DXINLINE void noEffAnimation::Update(float fTime,noEffVariant& varOwner)
{
	m_fWeight = fTime;
	m_pfnLerper(*this,varOwner);
}


//////////////////////////////// SXEffVariant Inline Functions Implementations ////////////////////////////////
D3DXINLINE bool noEffVariant::IsValid(void) const
{
	return m_hParam?true:false;
}

D3DXINLINE bool noEffVariant::IsA(D3DXPARAMETER_CLASS eParamClass) const
{
	return (GetClass() == eParamClass);
}

D3DXINLINE bool noEffVariant::IsA(D3DXPARAMETER_TYPE eParamType) const
{
	return (GetType() == eParamType);
}

D3DXINLINE float noEffVariant::mat(int iCol,int iRow)
{
	return operator D3DXMATRIX()(iCol,iRow);
}

D3DXINLINE noEffVariant& noEffVariant::operator= (bool bVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetBool(m_hParam,bVal?TRUE:FALSE);
	return *this;
}

D3DXINLINE noEffVariant& noEffVariant::operator= (int iVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetInt(m_hParam,iVal);
	return *this;
}

D3DXINLINE noEffVariant& noEffVariant::operator= (float fVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetFloat(m_hParam,fVal);
	return *this;
}

D3DXINLINE noEffVariant& noEffVariant::operator= (const D3DXVECTOR2& vec2Val)
{
	m_pOwnerEffect->GetD3DXEffect()->SetFloatArray(m_hParam,vec2Val,2);
	return *this;
}

D3DXINLINE noEffVariant& noEffVariant::operator= (const D3DXVECTOR3& vec3Val)
{
	m_pOwnerEffect->GetD3DXEffect()->SetFloatArray(m_hParam,vec3Val,3);
	return *this;
}

D3DXINLINE noEffVariant& noEffVariant::operator= (const D3DXVECTOR4& vec4Val)
{
	m_pOwnerEffect->GetD3DXEffect()->SetVector(m_hParam,&vec4Val);
	return *this;
}

D3DXINLINE noEffVariant& noEffVariant::operator= (const D3DXMATRIX& matVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetMatrix(m_hParam,&matVal);
	return *this;
}

D3DXINLINE noEffVariant& noEffVariant::operator= (PCSTR pszVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetString(m_hParam,pszVal);
	return *this;
}

D3DXINLINE noEffVariant& noEffVariant::operator= (PDIRECT3DVERTEXSHADER9 pVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetValue(m_hParam,pVal,sizeof(pVal));
	return *this;
}

D3DXINLINE noEffVariant& noEffVariant::operator= (PDIRECT3DPIXELSHADER9 pVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetValue(m_hParam,pVal, sizeof(pVal));
	return *this;
}

D3DXINLINE noEffVariant& noEffVariant::operator= (PDIRECT3DBASETEXTURE9 pVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetTexture(m_hParam,pVal);
	return *this;
}

D3DXINLINE void noEffVariant::CancelAnimation(void)
{
	if (m_pAnim)
		delete m_pAnim;
	m_pAnim = NULL;
}


//////////////////////////////// SXEffParam Inline Functions Implementations ////////////////////////////////
D3DXINLINE noEffParam& noEffParam::operator () (PCSTR pszParamName)
{
	return m_pOwnerEffect->operator()(pszParamName,m_hParam);
}

D3DXINLINE noEffVariant& noEffParam::operator= (bool bVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetBool(m_hParam,bVal?TRUE:FALSE);
	return *this;
}

D3DXINLINE noEffVariant& noEffParam::operator= (int iVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetInt(m_hParam,iVal);
	return *this;
}

D3DXINLINE noEffVariant& noEffParam::operator= (float fVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetFloat(m_hParam,fVal);
	return *this;
}

D3DXINLINE noEffVariant& noEffParam::operator= (const D3DXVECTOR2& vec2Val)
{
	m_pOwnerEffect->GetD3DXEffect()->SetFloatArray(m_hParam,vec2Val,2);
	return *this;
}

D3DXINLINE noEffVariant& noEffParam::operator= (const D3DXVECTOR3& vec3Val)
{
	m_pOwnerEffect->GetD3DXEffect()->SetFloatArray(m_hParam,vec3Val,3);
	return *this;
}

D3DXINLINE noEffVariant& noEffParam::operator= (const D3DXVECTOR4& vec4Val)
{
	m_pOwnerEffect->GetD3DXEffect()->SetVector(m_hParam,&vec4Val);
	return *this;
}

D3DXINLINE noEffVariant& noEffParam::operator= (const D3DXMATRIX& matVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetMatrix(m_hParam,&matVal);
	return *this;
}

D3DXINLINE noEffVariant& noEffParam::operator= (PCSTR pszVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetString(m_hParam,pszVal);
	return *this;
}

D3DXINLINE noEffVariant& noEffParam::operator= (PDIRECT3DVERTEXSHADER9 pVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetValue(m_hParam,pVal, sizeof(pVal));
	return *this;
}

D3DXINLINE noEffVariant& noEffParam::operator= (PDIRECT3DPIXELSHADER9 pVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetValue(m_hParam,pVal, sizeof(pVal));
	return *this;
}

D3DXINLINE noEffVariant& noEffParam::operator= (PDIRECT3DBASETEXTURE9 pVal)
{
	m_pOwnerEffect->GetD3DXEffect()->SetTexture(m_hParam,pVal);
	return *this;
}


//////////////////////////////// SXEffect Inline Functions Implementations ////////////////////////////////
D3DXINLINE LPD3DXEFFECT noFXEffect::GetD3DXEffect(void) const
{
	return m_pEffect;
}

#endif	// _SXEFFECT_INL

////////////////// End of File : SXEffect.inl //////////////////