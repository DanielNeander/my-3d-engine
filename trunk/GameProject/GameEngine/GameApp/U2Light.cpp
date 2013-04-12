#include "stdafx.h"
#include "U2Light.h"
#include "GameObjectManager.h"
#include "EngineCore/Math/IntersectionTests.h"
#include "EngineCore/Math/Frustum.h"
#include "DemoSetup.h"
#include "Mesh.h"
#include "M2Mesh.h"
#include "TestObjects.h"
#include "Common.h"

extern std::vector<Mesh*>	gShadowReceivers;

const TCHAR* U2Light::ms_aacLitType[LT_MAX_COUNT] =
{
	_T("LT_AMBIENT"),
	_T("LT_DIRECTIONAL"),
	_T("LT_POINT"),
	_T("LT_SPOT")
};

U2Light::U2Light()
	:m_eLitType(LT_DIRECTIONAL), 
	m_ambientColor(0.1f, 0.1f, 0.15f),
	m_diffuseColor(1.0f, 1.0f, 1.0f),
	m_specularColor(0.5f, 0.5f, 0.5f),
	m_vLitPos(0.f, 30.f, 0.f),
	m_vLitDir(0.f, 0.f, 0.f),
	m_vLitUp(0.f, 1.f, 0.f),
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

	m_fAspectRatio = 1.f;

	if (m_eLitType == LT_DIRECTIONAL)
		m_Type = TYPE_ORTHOGRAPHIC;
	else 
		m_Type = TYPE_PERSPECTIVE;

	m_vTarget = noVec3(0, 0, 0);
	
	m_bCastShadows = true;
 
	m_fFOV = DegreeToRadian(45.0f);
	//m_diffuseColor = noVec3(0.7f,0.7f,0.7f);
	//m_ambientColor = noVec3(0.25f,0.25f,0.25f);
	m_fNear = 0.1f;
	m_fFar = 1000.0f;
	ZeroMemory(&m_ControlState, sizeof(ControlState));
	m_ControlState.m_vRotation = noVec3(DegreeToRadian(50.0f), -DegreeToRadian(35.0f),0);
}


U2Light::U2Light(LightType eLitType)
	:m_eLitType(eLitType),
	m_ambientColor(0.1f, 0.1f, 0.15f),
	m_diffuseColor(1.0f, 1.0f, 1.0f),
	m_specularColor(0.5f, 0.5f, 0.5f),
	m_vLitPos(0.f, 30.f, 0.f),	
	m_vLitUp(0.f, 1.f, 0.f),
	m_vLitRight(1.f, 0.f, 0.f)

{
	if (eLitType == LT_DIRECTIONAL)
		m_Type = TYPE_ORTHOGRAPHIC;
	else 
		m_Type = TYPE_PERSPECTIVE;
		
	m_vTarget = noVec3(0, 0, 0);
	

	m_fConstCofficient = 1.f;
	m_fLinearCofficient = 0.f;
	m_fQuadraticCofficient = 0.f;
	m_fIntensityFactor = 1.f;

	m_fAngle = noMath::PI;
	m_fCosAngle = 0.f;
	m_fSinAngle = 0.f;
	m_fExponent = 0.f;

	m_fLitRange = 1.f;
	m_bCastShadows = true;

	m_fAspectRatio = 1.f;

	m_fFOV = DegreeToRadian(45.0f);
	//m_diffuseColor = noVec3(0.7f,0.7f,0.7f);
	//m_ambientColor = noVec3(0.25f,0.25f,0.25f);
	m_fNear = 0.1f;
	m_fFar = 1000.0f;
	ZeroMemory(&m_ControlState, sizeof(ControlState));
	m_ControlState.m_vRotation = noVec3(DegreeToRadian(50.0f), -DegreeToRadian(35.0f),0);
}


U2Light::~U2Light()
{


}

void U2Light::SetAngle(float angle)
{
	assert(0.f < angle && angle <= noMath::PI);
	m_fAngle = angle;
	m_fCosAngle = noMath::Cos(angle);
	m_fSinAngle = noMath::Sin(angle);
}


void U2Light::SetDir( const noVec3& vLitDir )
{
	m_vLitDir = vLitDir;
}


bool U2Light::IsValidFrame() const
{
	return true;
}

void U2Light::CalculateMatrices( void )
{
	
	// view matrix  
	m_mView = MatrixLookAtLH(m_vLitPos, m_vTarget, m_vLitUp);
	
	mat4 invView(ToMat4(m_mView));
	m_mInvView = transpose(invView);
	

	// projection matrix
	if(m_Type == TYPE_PERSPECTIVE)
	{
		m_mProj = MatrixPerspectiveFovLH(m_fFOV, m_fAspectRatio, m_fNear, m_fFar);
	}
	else
	{
		// this is just a funny way to calculate a size for the light using FOV
		float fFarPlaneSize = 2 * tanf(m_fFOV * 0.5f) * m_fFar;
		m_mProj = MatrixOrthoLH(fFarPlaneSize * m_fAspectRatio, fFarPlaneSize, m_fNear, m_fFar);
	}
	// view matrix  
	//Matrix tempView = MatrixLookAtLH(m_vLitPos, m_vTarget, m_vLitUp);
	//memcpy(&m_mView, &tempView, SIZE16);

	//// projection matrix
	//if(m_Type == TYPE_PERSPECTIVE)
	//{
	//	Matrix tempProj = MatrixPerspectiveFovLH(m_fFOV, m_fAspectRatio, m_fNear, m_fFar);
	//	memcpy(&m_mProj, &tempProj, SIZE16);
	//}
	//else
	//{
	//	// this is just a funny way to calculate a size for the light using FOV
	//	float fFarPlaneSize = 2 * tanf(m_fFOV * 0.5f) * m_fFar;
	//	Matrix tempProj = MatrixOrthoLH(fFarPlaneSize * m_fAspectRatio, fFarPlaneSize, m_fNear, m_fFar);
	//	memcpy(&m_mProj, &tempProj, SIZE16);

	//}
}


// helper function for computing AABB in clip space
inline BoundingBox CreateClipSpaceAABB(const BoundingBox &bb, const Matrix &mViewProj)
{
	noVec4 vTransformed[8];
	// for each point
	for(int i=0;i<8;i++)
	{
		// transform to projection space
		vTransformed[i] = Transform(bb.m_pPoints[i], mViewProj);

		// compute clip-space coordinates
		vTransformed[i].x /= vTransformed[i].w;
		vTransformed[i].y /= vTransformed[i].w;
		vTransformed[i].z /= vTransformed[i].w;
	}

	return BoundingBox(vTransformed, 8, sizeof(noVec4));
}

Matrix U2Light::CalculateCropMatrix( const Frustum &frustum )
{
	Matrix mViewProj = m_mView * m_mProj;

	BoundingBox cropBB;

	// find boundaries in light’s clip space
	cropBB = CreateClipSpaceAABB(frustum.m_AABB, mViewProj);

	// use default near plane
	cropBB.m_vMin.z = 0.0f;

	// finally, create matrix
	return BuildCropMatrix(cropBB.m_vMin, cropBB.m_vMax);
}

// crops the light volume on given objects, constrained by given frustum
Matrix U2Light::CalculateCropMatrix( const std::vector<Mesh *> &casters, const std::vector<Mesh *> &receivers, const Frustum &frustum )
{
	if(!g_bUseSceneDependentProjection) return CalculateCropMatrix(frustum);

	Matrix mViewProj = m_mView * m_mProj;

	// bounding box limits
	BoundingBox receiversBB, splitBB, castersBB;

	// for each caster
	// find boundaries in light’s clip space
	for(unsigned int i = 0; i < casters.size(); i++)
		castersBB.Union(CreateClipSpaceAABB(casters[i]->m_AABB, mViewProj));

	// for each receiver
	// find boundaries in light’s clip space
	for(unsigned int i = 0; i < receivers.size(); i++)
	{
		receiversBB.Union(CreateClipSpaceAABB(receivers[i]->m_AABB, mViewProj));
	}

	// find frustum boundaries in light’s clip space
	splitBB = CreateClipSpaceAABB(frustum.m_AABB, mViewProj);

	// next we will merge the bounding boxes
	//
	BoundingBox cropBB;
	cropBB.m_vMin.x = Max(Max(castersBB.m_vMin.x, receiversBB.m_vMin.x), splitBB.m_vMin.x);
	cropBB.m_vMax.x = Min(Min(castersBB.m_vMax.x, receiversBB.m_vMax.x), splitBB.m_vMax.x);
	cropBB.m_vMin.y = Max(Max(castersBB.m_vMin.y, receiversBB.m_vMin.y), splitBB.m_vMin.y);
	cropBB.m_vMax.y = Min(Min(castersBB.m_vMax.y, receiversBB.m_vMax.y), splitBB.m_vMax.y);
	cropBB.m_vMin.z = castersBB.m_vMin.z;
	cropBB.m_vMax.z = Min(receiversBB.m_vMax.z, splitBB.m_vMax.z);

	// when there are no casters, the merged
	// bounding box will be infinitely small
	if(casters.size() == 0)
	{
		// it will cause artifacts when rendering receivers,
		// so just use the frustum bounding box instead
		cropBB.m_vMin = splitBB.m_vMin;
		cropBB.m_vMax = splitBB.m_vMax;
	}

	// finally, create matrix
	return BuildCropMatrix(cropBB.m_vMin, cropBB.m_vMax);
}


// build a matrix for cropping light's projection
// given vectors are in light's clip space
Matrix U2Light::BuildCropMatrix( const noVec3 &vMin, const noVec3 &vMax )
{
	float fScaleX, fScaleY, fScaleZ;
	float fOffsetX, fOffsetY, fOffsetZ;

	fScaleX = 2.0f / (vMax.x - vMin.x);
	fScaleY = 2.0f / (vMax.y - vMin.y);

	fOffsetX = -0.5f * (vMax.x + vMin.x) * fScaleX;
	fOffsetY = -0.5f * (vMax.y + vMin.y) * fScaleY;

	fScaleZ = 1.0f / (vMax.z - vMin.z);
	fOffsetZ = -vMin.z * fScaleZ;

	// crop volume matrix
	return Matrix(   fScaleX,     0.0f,     0.0f,   0.0f,
		0.0f,  fScaleY,     0.0f,   0.0f,
		0.0f,     0.0f,  fScaleZ,   0.0f,
		fOffsetX, fOffsetY, fOffsetZ,   1.0f  );
}

// finds scene objects that overlap given frustum from light's view
std::vector<Mesh *> U2Light::FindCasters( const Frustum &frustum )
{
	noVec3 vDir = Normalize(m_vTarget - m_vLitPos);

	std::vector<Mesh *> casters;
	//	
	//casters.reserve(g_database.Size());
	//for(unsigned int i=0; i<g_database.Size(); i++)
	//{
	//	GameObject* pGameObj = g_database.Find(i);
	//	if (!pGameObj) continue;
	//	
	//	WowActor* pActor = (WowActor*)pGameObj;
	//	Mesh *pObject = pActor->mesh_;
	for(unsigned int i=0; i< gShadowReceivers.size(); i++)
	{
		Mesh *pObject = gShadowReceivers[i];
		if(pObject->m_bOnlyReceiveShadows) continue;

		// do intersection test
		// orthogonal light
		if(m_Type == TYPE_ORTHOGRAPHIC)
		{
			// use sweep intersection
			if(g_iVisibilityTest == VISTEST_ACCURATE) {
				// test accurately
				if(!SweepIntersectionTest(pObject->m_AABB, frustum, vDir)) continue;
			} else if(g_iVisibilityTest == VISTEST_CHEAP) {
				// test only with AABB of frustum
				if(!SweepIntersectionTest(pObject->m_AABB, frustum.m_AABB, vDir)) continue;
			}
		}
		// perspective light
		else if(m_Type == TYPE_PERSPECTIVE)
		{
			// the same kind of sweep intersection doesn't really work here, but we can
			// approximate it by using the direction to center of AABB as the sweep direction
			// (note that sometimes this will fail)
			noVec3 vDirToCenter = Normalize(((pObject->m_AABB.m_vMax + pObject->m_AABB.m_vMin) * 0.5f) - m_vLitPos);
			if(g_iVisibilityTest == VISTEST_ACCURATE)
			{
				// test accurately
				if(!SweepIntersectionTest(pObject->m_AABB, frustum, vDirToCenter)) continue;
			} else if(g_iVisibilityTest == VISTEST_CHEAP) {
				// test only with AABB of frustum
				if(!SweepIntersectionTest(pObject->m_AABB, frustum.m_AABB, vDirToCenter)) continue;
			}
		}

		casters.push_back(pObject);
	}
	return casters;
}

void U2Light::DoControls( float fDeltaTime )
{
	// Rotate light
	//
	if(GetKeyDown(VK_LEFT)) m_ControlState.m_vRotation.x += 0.2f * fDeltaTime;
	else if(GetKeyDown(VK_RIGHT)) m_ControlState.m_vRotation.x -= 0.2f * fDeltaTime;
	if(GetKeyDown(VK_UP)) m_ControlState.m_vRotation.y += 0.1f * fDeltaTime;
	else if(GetKeyDown(VK_DOWN)) m_ControlState.m_vRotation.y -= 0.1f * fDeltaTime;

	m_ControlState.m_vRotation.y = Clamp(m_ControlState.m_vRotation.y, DegreeToRadian(-89.9f), DegreeToRadian(0.0f));
	float ch = cosf(m_ControlState.m_vRotation.x);
	float sh = sinf(m_ControlState.m_vRotation.x);
	float cp = cosf(m_ControlState.m_vRotation.y);
	float sp = sinf(m_ControlState.m_vRotation.y);
	noVec3 vDist = m_vTarget - m_vLitPos;
	m_vLitPos = m_vTarget + noVec3(sh*cp, -sp, cp*ch) * vDist.Length();

	// Switch light type
	//
	if(GetKeyDown('T'))
	{
		if(!m_ControlState.m_bSwitchingType)
		{
			m_Type = (m_Type == U2Light::TYPE_ORTHOGRAPHIC) ? U2Light::TYPE_PERSPECTIVE : U2Light::TYPE_ORTHOGRAPHIC;
			m_ControlState.m_bSwitchingType = true;
		}
	}
	else
	{
		m_ControlState.m_bSwitchingType = false;
	}

	CalculateMatrices();
}

 noVec3 U2Light::GetDir( void ) const
{
	m_vLitDir = (m_vTarget - m_vLitPos);
	m_vLitDir.Normalize();
	return m_vLitDir;
}

 void U2Light::RegenerateDebugWireFrustum()
 {

 }

 bool U2Light::Initialize()
 {
	 m_cbLightConstants.Initialize(D3D11Dev());

	 return true;
 }

 void U2Light::Apply(int nslot)
 {
	 m_cbLightConstants.Data.LightAmbient = this->m_ambientColor;
	 m_cbLightConstants.Data.LightDiffuse = this->m_diffuseColor;
	 m_cbLightConstants.Data.LightSpecular = this->m_specularColor;
	 m_cbLightConstants.Data.LightType = this->m_eLitType;
	 m_cbLightConstants.Data.LightRange = this->m_fFar;
	 m_cbLightConstants.Data.LightPos = this->m_vLitPos;

	 m_cbLightConstants.ApplyChanges(D3D11Context());
	 m_cbLightConstants.SetVS(D3D11Context(), nslot);
	 m_cbLightConstants.SetPS(D3D11Context(), nslot);
 }



