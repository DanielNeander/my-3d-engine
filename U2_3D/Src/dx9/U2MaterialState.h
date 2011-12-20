/************************************************************************
module	:	U2Dx9MaterialState
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once
#ifndef U2_DX9_MATERIALSTATE_H
#define U2_DX9_MATERIALSTATE_H


#include "U2RenderState.h"

U2SmartPointer(U2MaterialState);

class U2_3D U2MaterialState : public U2RenderState
{
	DECLARE_RTTI;
	DECLARE_INITIALIZE;
	DECLARE_TERMINATE;
	
public :
	U2MaterialState();
	virtual ~U2MaterialState();


	virtual int Type() const { return U2RenderState::MATERIAL; }

	void SetAmbientColor (const D3DXCOLOR& c);
	const D3DXCOLOR& GetAmbientColor () const;

	void SetDiffuseColor (const D3DXCOLOR& c);
	const D3DXCOLOR& GetDiffuseColor () const;

	void SetSpecularColor (const D3DXCOLOR& c);
	const D3DXCOLOR& GetSpecularColor () const;

	void SetEmittance (const D3DXCOLOR& c);
	const D3DXCOLOR& GetEmittance () const;

	void SetShineness (float c);
	float GetShineness () const;

	void SetAlpha (float c);
	float GetAlpha () const;

	void SetChanged (bool bChanged);
	int GetIndex () const;

	void SetRevisionID (unsigned int uiRev);
	unsigned int GetRevisionID () const;

	void* GetRendererData() const;
	void SetRendererData(void* pvRendererData);

	static U2MaterialStatePtr GetDefaultMaterial();

	D3DXCOLOR m_ambient;
	D3DXCOLOR m_diffuse;
	D3DXCOLOR m_specular;
	D3DXCOLOR m_emissive;
	float m_fShininess;
	float m_fAlpha;

protected:
	static int ms_iNextIndex;

	void* m_pvRenderData;

	int m_iIndex;
	

	unsigned int m_uiRevID;

};


#endif


