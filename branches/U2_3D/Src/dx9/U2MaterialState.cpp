#include <U2_3D/src/U23DLibPCH.h>
#include "U2MaterialState.h"

int U2MaterialState::ms_iNextIndex = 1;


IMPLEMENT_RTTI(U2MaterialState, U2RenderState);
IMPLEMENT_INITIALIZE(U2MaterialState);
IMPLEMENT_TERMINATE(U2MaterialState);

void U2MaterialState::Initialize()
{
	ms_aspDefault[MATERIAL] = U2_NEW U2MaterialState;
	FILE_LOG(logDEBUG) << _T("U2MaterialState::Initialize - Success...");
}

void U2MaterialState::Terminate()
{
	ms_aspDefault[MATERIAL] = 0;
	FILE_LOG(logDEBUG) << _T("U2MaterialState::Terminate - Success...");
}


U2MaterialState::U2MaterialState()
{
	m_ambient.r = m_ambient.g = m_ambient.b = 0.5f;
	m_diffuse.r = m_diffuse.g = m_diffuse.b = 0.5f;
	m_specular.r = m_specular.g = m_specular.b = 0.0f;
	m_emissive.r = m_emissive.g = m_emissive.b = 0.0f;
	m_fShininess = 4.0f;

	m_fAlpha = 1.0f;

	m_iIndex = ms_iNextIndex++;    

	m_uiRevID = 1;

	m_pvRenderData = 0;
}


U2MaterialState::~U2MaterialState()
{

}


void U2MaterialState::SetAmbientColor (const D3DXCOLOR& c)
{
	m_ambient = c;
	SetChanged(true);
}


const D3DXCOLOR& U2MaterialState::GetAmbientColor () const
{
	return m_ambient;
}


void U2MaterialState::SetDiffuseColor (const D3DXCOLOR& c)
{
	m_diffuse = c;
	SetChanged(true);
}


const D3DXCOLOR& U2MaterialState::GetDiffuseColor () const
{
	return m_diffuse;
}


void U2MaterialState::SetSpecularColor (const D3DXCOLOR& c)
{
	m_specular = c;
	SetChanged(true);
}


const D3DXCOLOR& U2MaterialState::GetSpecularColor () const
{
	return m_specular;
}

void U2MaterialState::SetEmittance (const D3DXCOLOR& c)
{
	m_emissive = c;
	SetChanged(true);
}


const D3DXCOLOR& U2MaterialState::GetEmittance () const
{
	return m_emissive;
}

void U2MaterialState::SetShineness (float c)
{
	m_fShininess = c;
	SetChanged(true);
}


float U2MaterialState::GetShineness () const
{
	return m_fShininess;
}

void U2MaterialState::SetAlpha (float c)
{
	m_fAlpha = c;
	SetChanged(true);
}


float U2MaterialState::GetAlpha () const
{
	return m_fAlpha;
}


void U2MaterialState::SetChanged (bool bChanged)
{
	m_uiRevID++;
}

int U2MaterialState::GetIndex () const
{
	return m_iIndex;
}


 void U2MaterialState::SetRevisionID(unsigned int uiRev)
{
	m_uiRevID = uiRev;
}
//---------------------------------------------------------------------------
unsigned int U2MaterialState::GetRevisionID() const
{
	return m_uiRevID;
}


void* U2MaterialState::GetRendererData() const
{
	return m_pvRenderData;
}


void U2MaterialState::SetRendererData(void* pvRendererData)
{
	m_pvRenderData = pvRendererData;
}

U2MaterialStatePtr U2MaterialState::GetDefaultMaterial()
{
	return SmartPtrCast(U2MaterialState, 
		ms_aspDefault[U2RenderState::MATERIAL]);
}
