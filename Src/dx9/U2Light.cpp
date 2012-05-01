#include <U2_3D/src/U23DLibPCH.h>
#include "U2Light.h"
#include <U2_3D/src/Main/U2Math.h>

const TCHAR* U2Light::ms_aacLitType[LT_MAX_COUNT] =
{
	_T("LT_AMBIENT"),
	_T("LT_DIRECTIONAL"),
	_T("LT_POINT"),
	_T("LT_SPOT")
};

U2Light::U2Light()
	:m_eLitType(LT_AMBIENT), 
	m_ambientColor(0.f, 0.f, 0.f, 0.f),
	m_diffuseColor(0.f, 0.f, 0.f, 0.f),
	m_specularColor(0.f, 0.f, 0.f, 0.f),
	m_vLitPos(0.f, 0.f, 0.f),
	m_vLitDir(0.f, 0.f, 0.f),
	m_vLitUp(0.f, 0.f, 0.f),
	m_vLitRight(0.f, 0.f, 0.f)
{
	m_fConstCofficient = 0.f;
	m_fLinearCofficient = 0.f;
	m_fQuadraticCofficient = 0.f;
	m_fIntensityFactor = 0.f;

	m_fAngle = 0.f;
	m_fCosAngle = 0.f;
	m_fSinAngle = 0.f;
	m_fExponent = 0.f;

	m_fLitRange = 1.f;
	
	m_bCastShadows = false;
 
}


U2Light::U2Light(LightType eLitType)
	:m_eLitType(eLitType),
	m_ambientColor(0.f, 0.f, 0.f, 0.f),
	m_diffuseColor(0.f, 0.f, 0.f, 0.f),
	m_specularColor(0.f, 0.f, 0.f, 0.f),
	m_vLitPos(0.f, 0.f, 0.f),
	m_vLitDir(0.f, 0.f, 1.f),
	m_vLitUp(0.f, 1.f, 0.f),
	m_vLitRight(1.f, 0.f, 0.f)

{
	m_fConstCofficient = 1.f;
	m_fLinearCofficient = 0.f;
	m_fQuadraticCofficient = 0.f;
	m_fIntensityFactor = 1.f;

	m_fAngle = D3DX_PI;
	m_fCosAngle = 0.f;
	m_fSinAngle = 0.f;
	m_fExponent = 0.f;

	m_fLitRange = 1.f;
	m_bCastShadows = false;

}

U2Light::~U2Light()
{


}


void U2Light::SetAngle(float angle)
{
	U2ASSERT(0.f < angle && angle <= D3DX_PI);
	m_fAngle = angle;
	m_fCosAngle = U2Math::Cos(angle);
	m_fSinAngle = U2Math::Sin(angle);
}


void U2Light::SetDir(const D3DXVECTOR3 &vLitDir)
{
	m_vLitDir = vLitDir;
}


bool U2Light::IsValidFrame() const
{
	return true;
}

