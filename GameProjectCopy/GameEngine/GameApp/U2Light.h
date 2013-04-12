/************************************************************************
module	:	U2Dx9Light
Author	:	Yun sangyong
Desc	:	라이트 클래스를 분할하지 않고 라이트 타입으로 구분.
			멤버 변수의 정확한 정의 필요...
************************************************************************/
#pragma once
#ifndef	U2_DX11_LIGHT_H
#define	U2_DX11_LIGHT_H

#include "GraphicsTypes.h"
#include "SceneNode.h"

class Mesh;
class Frustum;

class U2Light : public SceneNode
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

	noVec3 m_ambientColor;
	noVec3 m_diffuseColor;
	noVec3 m_specularColor;


	float m_fConstCofficient;
	float m_fLinearCofficient;
	float m_fQuadraticCofficient;
	float m_fIntensityFactor;

	float m_fAngle;
	float m_fCosAngle;
	float m_fSinAngle;
	float m_fExponent;


	enum LightMatType
	{
		TYPE_ORTHOGRAPHIC, // eg. directional light
		TYPE_PERSPECTIVE   // eg. spot light
	};

	// type of light
	LightMatType m_Type;

	// matrices, updated with CalculateMatrices()
	Matrix m_mView;
	Matrix m_mProj;
	
	mat4 m_mInvView;
	

	// settings
	mutable noVec3 m_vLitPos, m_vLitDir, m_vLitUp, m_vLitRight;
	noVec3 m_vTarget;
	float m_fNear;
	float m_fFar;
	float m_fFOV;	
	float m_fAspectRatio;	


	// 
	float m_fLitRange;
	noVec4 m_shadowLightMask;
	bool m_bCastShadows;

	bool Initialize();
	
	void SetAngle(float angle);

	void SetDir(const noVec3& vLitDir);

	bool IsValidFrame() const;

	// finds scene objects that overlap given frustum from light's view
	std::vector<Mesh *> FindCasters(const Frustum &frustum);

	// calculates light noMat4 to contain given visible objects
	void CalculateMatrices(void);

	// crops the light volume on given frustum (scene-independent projection)
	Matrix CalculateCropMatrix(const Frustum &frustum);

	// crops the light volume on given objects, constrained by given frustum
	Matrix CalculateCropMatrix(const std::vector<Mesh *> &casters, const std::vector<Mesh *> &receivers, const Frustum &frustum);

	// retuns direction of light
	noVec3 GetDir(void) const ;
	
	// processes light controls
	void DoControls(float fDeltaTime);

	U2Light();

	void RegenerateDebugWireFrustum();


	struct LightConstants 
	{
		Float4Align noVec3 LightPos;
		Float4Align	noVec3	LightDiffuse;
		Float4Align	noVec3	LightSpecular;
		Float4Align	noVec3	LightAmbient;
		float	LightRange;
		int		LightType;
	};

	ConstantBuffer<LightConstants>	m_cbLightConstants;

	void Apply(int nslot);

private:
	static const TCHAR* ms_aacLitType[LT_MAX_COUNT];

	noVec3	m_FrustumVerts[8];
	//noPlane	m_FrustumPlanes[6];

	// Spotlight parameters
	FLOAT          m_fWidth;
	FLOAT          m_fHeight;

	FLOAT          m_fNearWidth;
	FLOAT          m_fNearHeight;
	FLOAT          m_fVerticalFOV;
	FLOAT          m_fFarWidth;
	FLOAT          m_fFarHeight;

	struct ControlState
	{
		noVec3 m_vRotation;
		bool m_bSwitchingType;
		double m_fLastUpdate;
	};



	ControlState m_ControlState;
private:

	// build a noMat4 for cropping light's projection	
	inline Matrix BuildCropMatrix( const noVec3 &vMin, const noVec3 &vMax );

};

//typedef U2SmartPtr<U2Light> U2LightPtr;
extern U2Light gSun;


#endif