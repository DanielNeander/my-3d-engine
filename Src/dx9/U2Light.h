/************************************************************************
module	:	U2Dx9Light
Author	:	Yun sangyong
Desc	:	라이트 클래스를 분할하지 않고 라이트 타입으로 구분.
			멤버 변수의 정확한 정의 필요...
************************************************************************/
#pragma once
#ifndef	U2_DX9_LIGHT_H
#define	U2_DX9_LIGHT_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2Lib/Src/U2RefObject.h>
#include <U2Lib/Src/U2SmartPtr.h>


class U2_3D U2Light : public U2RefObject
{

public:
	// Shader 파일에 영향을 줄 수 있으므로 주의.
	// Nebula와 Wild Magic의 차이가 있을 수 있지만 
	// 현재 사용중인 경우는 Nebula에서 Shader에 
	// 파라미터로 사용중이기 때문에 
	// Nebula type에 맞춘다.
	enum LightType 
	{
		/*LT_AMBIENT = 0,
		LT_DIRECTIONAL,
		LT_POINT,
		LT_SPOT,
		LT_MAX_COUNT*/
		LT_POINT = 0,
		LT_DIRECTIONAL,
		LT_SPOT,
		LT_AMBIENT,					
		LT_MAX_COUNT
	};

	U2Light(LightType eType);
	virtual ~U2Light();

	LightType m_eLitType;

	D3DXCOLOR m_ambientColor;
	D3DXCOLOR m_diffuseColor;
	D3DXCOLOR m_specularColor;


	float m_fConstCofficient;
	float m_fLinearCofficient;
	float m_fQuadraticCofficient;
	float m_fIntensityFactor;

	float m_fAngle;
	float m_fCosAngle;
	float m_fSinAngle;
	float m_fExponent;


	// 
	float m_fLitRange;
	D3DXVECTOR4 m_shadowLightMask;
	bool m_bCastShadows;

	D3DXVECTOR3 m_vLitPos, m_vLitDir, m_vLitUp, m_vLitRight;

	void SetAngle(float angle);

	void SetDir(const D3DXVECTOR3& vLitDir);

	bool IsValidFrame() const;


protected:
	U2Light();

private:
	static const TCHAR* ms_aacLitType[LT_MAX_COUNT];

	
};

typedef U2SmartPtr<U2Light> U2LightPtr;

#endif