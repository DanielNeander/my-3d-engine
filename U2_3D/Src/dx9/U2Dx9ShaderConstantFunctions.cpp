#include <U2_3D/Src/U23DLibPCH.h>
#include "U2Dx9Renderer.h"


U2Dx9Renderer::SetConstantFunc U2Dx9Renderer::ms_aSCFun[SC_QUANTITY] =
{
	NULL,
	&U2Dx9Renderer::SetConstantWMatrix,
	&U2Dx9Renderer::SetConstantVMatrix,
	&U2Dx9Renderer::SetConstantPMatrix,
	&U2Dx9Renderer::SetConstantWVMatrix,
	&U2Dx9Renderer::SetConstantVPMatrix,
	&U2Dx9Renderer::SetConstantWVPMatrix,				// 6

	&U2Dx9Renderer::SetConstantMaterialEmissive,
	&U2Dx9Renderer::SetConstantMaterialAmbient,
	&U2Dx9Renderer::SetConstantMaterialDiffuse,
	&U2Dx9Renderer::SetConstantMaterialSpecular,
	&U2Dx9Renderer::SetConstantCameraModelPosition,
	&U2Dx9Renderer::SetConstantCameraModelDirection,
	&U2Dx9Renderer::SetConstantCameraModelUp,
	&U2Dx9Renderer::SetConstantCameraModelRight,
	&U2Dx9Renderer::SetConstantCameraWorldPosition,
	&U2Dx9Renderer::SetConstantCameraWorldDirection,
	&U2Dx9Renderer::SetConstantCameraWorldUp,
	&U2Dx9Renderer::SetConstantCameraWorldRight,
	&U2Dx9Renderer::SetConstantProjectorModelPosition,
	&U2Dx9Renderer::SetConstantProjectorModelDirection,
	&U2Dx9Renderer::SetConstantProjectorModelUp,
	&U2Dx9Renderer::SetConstantProjectorModelRight,
	&U2Dx9Renderer::SetConstantProjectorWorldPosition,
	&U2Dx9Renderer::SetConstantProjectorWorldDirection,
	&U2Dx9Renderer::SetConstantProjectorWorldUp,
	&U2Dx9Renderer::SetConstantProjectorWorldRight,
	&U2Dx9Renderer::SetConstantProjectorMatrix,			// 27


	// These functions set the light state.  The index iLight is between 0
	// and 7 (eight lights are currently supported).
	&U2Dx9Renderer::SetConstantLightModelPosition, 
	&U2Dx9Renderer::SetConstantLightModelDirection,
	&U2Dx9Renderer::SetConstantLightWorldPosition,
	&U2Dx9Renderer::SetConstantLightWorldDirection,
	&U2Dx9Renderer::SetConstantLightAmbient,
	&U2Dx9Renderer::SetConstantLightDiffuse,
	&U2Dx9Renderer::SetConstantLightSpecular,
	&U2Dx9Renderer::SetConstantLightSpotCutoff,
	&U2Dx9Renderer::SetConstantLightAttenuation			// 36
};

void U2Dx9Renderer::GetTransform (const D3DXMATRIX& rkMat, int iOperation,
								  float* pafData)
{
	const size_t uSize = 16 * sizeof(float);

	if(iOperation == 0)
	{
		memcpy_s(pafData, uSize, (const float*)rkMat, uSize);
	}
	else if(iOperation == 1)
	{
		D3DXMATRIX matTranspose;
		D3DXMatrixTranspose(&matTranspose, &rkMat);
		memcpy_s(pafData, uSize, (const float*)matTranspose, uSize);
	}
	else if(iOperation == 2)
	{
		D3DXMATRIX matInverse;
		D3DXMatrixInverse(&matInverse, 0, &rkMat);
		memcpy_s(pafData, uSize, (const float*)matInverse, uSize);

	}
	else if(iOperation == 3)
	{
		D3DXMATRIX matIT;
		D3DXMATRIX matInverse;

		D3DXMatrixInverse(&matInverse, 0, &rkMat);	
		D3DXMatrixTranspose(&matIT, &matInverse);

		memcpy_s(pafData, uSize, (const float*)matIT, uSize);
	}
}

void U2Dx9Renderer::SetConstantWMatrix (int iOperation, float* afData)
{

	GetTransform(m_worldMat, iOperation, afData);
}


void U2Dx9Renderer::SetConstantVMatrix (int iOperation, float* afData)
{

	GetTransform(m_viewMat, iOperation, afData)	;
}


void U2Dx9Renderer::SetConstantPMatrix (int iOperation, float* afData)
{
	GetTransform(m_projMat, iOperation, afData);
}

void U2Dx9Renderer::SetConstantWVMatrix (int iOperation, float* afData)
{
	D3DXMATRIX wvMat = m_worldMat * m_viewMat;
	GetTransform(wvMat, iOperation, afData);
}


void U2Dx9Renderer::SetConstantVPMatrix (int iOperation, float* afData)
{
	D3DXMATRIX vpMat = m_viewMat * m_projMat;
	GetTransform(vpMat, iOperation, afData);
}


void U2Dx9Renderer::SetConstantWVPMatrix (int iOperation, float* afData)
{
	D3DXMATRIX wvpMat = m_worldMat * m_viewMat * m_projMat;
	GetTransform(wvpMat, iOperation, afData);

}


void U2Dx9Renderer::SetConstantProjectorMatrix (int, float* afData)
{
	U2ASSERT(m_spProjCamera);

	const D3DXVECTOR3& eye = m_spProjCamera->GetEye();
	const D3DXVECTOR3& right = m_spProjCamera->GetRight();
	const D3DXVECTOR3& up = m_spProjCamera->GetUp();
	const D3DXVECTOR3& dir = m_spProjCamera->GetDir();

	float fRdotE = D3DXVec3Dot(&right, &eye);
	float fUdotE = D3DXVec3Dot(&up, &eye);
	float fDdotE = D3DXVec3Dot(&dir, &eye);
	D3DXMATRIX projVMat(
		right.x, up.x, dir.x, 0.f,
		right.y, up.y, dir.y, 0.f,
		right.z, up.z, dir.z, 0.f,
		-fRdotE, -fUdotE, -fDdotE, 1.0f);


	U2Frustum frustum = m_spProjCamera->GetFrustum();
	float fInvLRDiff = 1.0f/(frustum.m_fRight - frustum.m_fLeft);
	float fInvTBDiff = 1.0f/(frustum.m_fTop - frustum.m_fBottom);
	float fInvFNDiff = 1.0f/(frustum.m_fFar - frustum.m_fNear);
	float fRTerm0 = frustum.m_fNear * fInvLRDiff;
	float fUTerm0 = frustum.m_fNear * fInvTBDiff;
	float fDTerm0 = frustum.m_fNear * fInvFNDiff;

	float fRTerm1 = -(frustum.m_fLeft + frustum.m_fRight) * fInvLRDiff;
	float fUTerm1 = -(frustum.m_fBottom + frustum.m_fTop) * fInvTBDiff;
	float fDTerm1 = frustum.m_fFar * fInvFNDiff;
	D3DXMATRIX projPMat(
		2.0f * fRTerm1, 0.f, 0.f, 0.f,
		0.f, 2.0f * fUTerm1, 0.f, 0.f,
		fRTerm1, fUTerm1, fDTerm1, 1.0f,
		0.f, 0.f, -frustum.m_fFar * fDTerm0, 0.f);

	// Set up the bias and scale matrix for the projector.
	D3DXMATRIX projBSMatrix(
		0.5f,0.0f,0.0f,0.0f,
		0.0f,0.5f,0.0f,0.0f,
		0.0f,0.0f,1.0f,0.0f,
		0.5f,0.5f,0.0f,1.0f);

	D3DXMATRIX projectorMat = 
		m_worldMat * projVMat * projPMat * projBSMatrix;

	GetTransform(projectorMat, 0, afData);
}


void U2Dx9Renderer::SetConstantMaterialEmissive (int, float* afData)
{
	U2MaterialStatePtr pMaterial = m_pCurrRSC->GetMaterial();
	afData[0] = pMaterial->m_emissive.r;
	afData[1] = pMaterial->m_emissive.g;
	afData[2] = pMaterial->m_emissive.b;
	afData[3] = 1.f;	
}


void U2Dx9Renderer::SetConstantMaterialAmbient (int, float* afData)
{
	U2MaterialState* pMaterial = m_pCurrRSC->GetMaterial();
	afData[0] = pMaterial->m_ambient.r;
	afData[1] = pMaterial->m_ambient.g;
	afData[2] = pMaterial->m_ambient.b;
	afData[3] = 1.f;
}


void U2Dx9Renderer::SetConstantMaterialDiffuse (int, float* afData)
{
	U2MaterialState* pMaterial = m_pCurrRSC->GetMaterial();
	afData[0] = pMaterial->m_diffuse.r;
	afData[1] = pMaterial->m_diffuse.g;
	afData[2] = pMaterial->m_diffuse.b;
	afData[3] = pMaterial->m_fAlpha;
}



void U2Dx9Renderer::SetConstantMaterialSpecular (int, float* afData)
{

	U2MaterialState* pMaterial = m_pCurrRSC->GetMaterial();
	afData[0] = pMaterial->m_specular.r;
	afData[1] = pMaterial->m_specular.g;
	afData[2] = pMaterial->m_specular.b;
	afData[3] = pMaterial->m_fShininess;
}

void U2Dx9Renderer::SetConstantCameraModelPosition (int, float* afData)
{


	D3DXVECTOR3 modelSpaceLoc = 
		m_pCurrMesh->m_tmWorld.ApplyInverse(m_spCamera->GetEye());

	afData[0] = modelSpaceLoc.x;
	afData[1] = modelSpaceLoc.y;
	afData[2] = modelSpaceLoc.z;
	afData[3] = 1.f;
}


void U2Dx9Renderer::SetConstantCameraModelDirection (int, float* afData)
{

	D3DXVECTOR3 modelSpaceDir = 
		m_pCurrMesh->m_tmWorld.InverseVector(m_spCamera->GetDir());

	afData[0] = modelSpaceDir.x;
	afData[1] = modelSpaceDir.y;
	afData[2] = modelSpaceDir.z;
	afData[3] = 0.f;
}


void U2Dx9Renderer::SetConstantCameraModelUp (int, float* afData)
{

	D3DXVECTOR3 modelSpaceUp = 
		m_pCurrMesh->m_tmWorld.InverseVector(m_spCamera->GetUp());

	afData[0] = modelSpaceUp.x;
	afData[1] = modelSpaceUp.y;
	afData[2] = modelSpaceUp.z;
	afData[3] = 0.f;
}

void U2Dx9Renderer::SetConstantCameraModelRight (int, float* afData)
{
	D3DXVECTOR3 modelSpaceRight = 
		m_pCurrMesh->m_tmWorld.InverseVector(m_spCamera->GetRight());

	afData[0] = modelSpaceRight.x;
	afData[1] = modelSpaceRight.y;
	afData[2] = modelSpaceRight.z;
	afData[3] = 0.f;	
}


void U2Dx9Renderer::SetConstantCameraWorldPosition (int, float* afData)
{
	D3DXVECTOR3 WorldPos = m_spCamera->GetEye();	

	afData[0] = WorldPos.x;
	afData[1] = WorldPos.y;
	afData[2] = WorldPos.z;
	afData[3] = 1.f;	
}


void U2Dx9Renderer::SetConstantCameraWorldDirection (int, float* afData)
{

	D3DXVECTOR3 WorldDir = m_spCamera->GetDir();	

	afData[0] = WorldDir.x;
	afData[1] = WorldDir.y;
	afData[2] = WorldDir.z;
	afData[3] = 0.f;	

}


void U2Dx9Renderer::SetConstantCameraWorldUp (int, float* afData)
{
	D3DXVECTOR3 WorldUp = m_spCamera->GetUp();	

	afData[0] = WorldUp.x;
	afData[1] = WorldUp.y;
	afData[2] = WorldUp.z;
	afData[3] = 0.f;	

}


void U2Dx9Renderer::SetConstantCameraWorldRight (int, float* afData)
{

	D3DXVECTOR3 WorldRight = m_spCamera->GetRight();	

	afData[0] = WorldRight.x;
	afData[1] = WorldRight.y;
	afData[2] = WorldRight.z;
	afData[3] = 0.f;	

}


void U2Dx9Renderer::SetConstantProjectorModelPosition (int, float* afData)
{
	U2ASSERT(m_spProjCamera);

	D3DXVECTOR3 modelPos = m_pCurrMesh->m_tmWorld.ApplyInverse(
		m_spProjCamera->GetEye());

	afData[0] = modelPos.x;
	afData[1] = modelPos.y;
	afData[2] = modelPos.z;
	afData[3] = 1.f;
}


void U2Dx9Renderer::SetConstantProjectorModelDirection (int, float* afData)
{
	U2ASSERT(m_spProjCamera);

	D3DXVECTOR3 modelDir = m_pCurrMesh->m_tmWorld.InverseVector(
		m_spProjCamera->GetDir());

	afData[0] = modelDir.x;
	afData[1] = modelDir.y;
	afData[2] = modelDir.z;
	afData[3] = 0.f;
}


void U2Dx9Renderer::SetConstantProjectorModelUp (int, float* afData)
{
	U2ASSERT(m_spProjCamera);

	D3DXVECTOR3 modelUp = m_pCurrMesh->m_tmWorld.InverseVector(
		m_spProjCamera->GetUp());

	afData[0] = modelUp.x;
	afData[1] = modelUp.y;
	afData[2] = modelUp.z;
	afData[3] = 0.f;

}


void U2Dx9Renderer::SetConstantProjectorModelRight (int, float* afData)
{
	U2ASSERT(m_spProjCamera);

	D3DXVECTOR3 modelRight = m_pCurrMesh->m_tmWorld.InverseVector(
		m_spProjCamera->GetRight());

	afData[0] = modelRight.x;
	afData[1] = modelRight.y;
	afData[2] = modelRight.z;
	afData[3] = 0.f;
}


void U2Dx9Renderer::SetConstantProjectorWorldPosition (int, float* afData)
{
	U2ASSERT(m_spProjCamera);


	D3DXVECTOR3 WorldPos = m_spProjCamera->GetEye();	

	afData[0] = WorldPos.x;
	afData[1] = WorldPos.y;
	afData[2] = WorldPos.z;
	afData[3] = 1.f;	
}


void U2Dx9Renderer::SetConstantProjectorWorldDirection (int, float* afData)
{
	U2ASSERT(m_spProjCamera);

	D3DXVECTOR3 WorldDir = m_spProjCamera->GetDir();	

	afData[0] = WorldDir.x;
	afData[1] = WorldDir.y;
	afData[2] = WorldDir.z;
	afData[3] = 0.f;	

}


void U2Dx9Renderer::SetConstantProjectorWorldUp (int, float* afData)
{
	U2ASSERT(m_spProjCamera);

	D3DXVECTOR3 WorldUp = m_spProjCamera->GetUp();	

	afData[0] = WorldUp.x;
	afData[1] = WorldUp.y;
	afData[2] = WorldUp.z;
	afData[3] = 0.f;	

}


void U2Dx9Renderer:: SetConstantProjectorWorldRight (int, float* afData)
{
	U2ASSERT(m_spProjCamera);
	D3DXVECTOR3 WorldRight = m_spProjCamera->GetRight();	

	afData[0] = WorldRight.x;
	afData[1] = WorldRight.y;
	afData[2] = WorldRight.z;
	afData[3] = 0.f;	
}





// These functions set the light state.  The index iLight is between 0
// and 7 (eight lights are currently supported).
void U2Dx9Renderer::SetConstantLightModelPosition (int iLight, float* afData)
{	

	U2Light* pLight = GetLight(iLight);
	if(pLight)
	{
		D3DXVECTOR3 modelPos = m_pCurrMesh->m_tmWorld.ApplyInverse(
			pLight->m_vLitPos);

		afData[0] = modelPos.x;
		afData[1] = modelPos.y;
		afData[2] = modelPos.z;
		afData[3] = 1.f;
	}
	else 
	{
		afData[0] = 0.f;
		afData[1] = 0.f;
		afData[2] = 0.f;
		afData[3] = 1.f;
	}
}
void U2Dx9Renderer::SetConstantLightModelDirection (int iLight, float* afData)
{

	U2Light* pLight = GetLight(iLight);
	if(pLight)
	{
		D3DXVECTOR3 modelDir = m_pCurrMesh->m_tmWorld.InverseVector(
			pLight->m_vLitDir);

		D3DXVECTOR3 normDir;
		D3DXVec3Normalize(&normDir, &modelDir);

		afData[0] = normDir.x;
		afData[1] = normDir.y;
		afData[2] = normDir.z;
	}
	else 
	{
		afData[0] = 0.f;
		afData[1] = 0.f;
		afData[2] = 0.f;
		afData[3] = 0.f;
	}
}
void U2Dx9Renderer::SetConstantLightWorldPosition (int iLight, float* afData)
{
	U2Light* pLight = GetLight(iLight);
	if(pLight)
	{
		afData[0] = pLight->m_vLitPos.x;
		afData[1] = pLight->m_vLitPos.y;
		afData[2] = pLight->m_vLitPos.z;
		afData[3] = 1.f;
	}
	else 
	{
		afData[0] = 0.f;
		afData[1] = 0.f;
		afData[2] = 0.f;
		afData[3] = 1.f;
	}

}
void U2Dx9Renderer::SetConstantLightWorldDirection (int iLight, float* afData)
{
	U2Light* pLight = GetLight(iLight);
	if(pLight)
	{
		// If the light's direction was set via Light::SetDirection(), then
		// DVector should be unit length.  If the light is managed by a
		// LightNode, then LightNode::UpdateWorldData() guarantees that
		// DVector is unit length.  However, to ensure we have a unit-length
		// vector, normalize anyway.
		D3DXVECTOR3 normDir; 

		D3DXVec3Normalize(&normDir, &pLight->m_vLitDir);
		afData[0] = normDir.x;
		afData[1] = normDir.y;
		afData[2] = normDir.z;
		afData[3] = 0.f;
	}
	else 
	{
		afData[0] = 0.f;
		afData[1] = 0.f;
		afData[2] = 0.f;
		afData[3] = 0.f;
	}


}


void U2Dx9Renderer::SetConstantLightAmbient (int iLight, float* afData)
{

	U2Light* pLight = GetLight(iLight);
	U2ASSERT(pLight);
	afData[0] = pLight->m_ambientColor.r;
	afData[1] = pLight->m_ambientColor.g;
	afData[2] = pLight->m_ambientColor.b;
	afData[3] = 1.f;
}
void U2Dx9Renderer::SetConstantLightDiffuse (int iLight, float* afData)
{

	U2Light* pLight = GetLight(iLight);	
	if(pLight)
	{	
		afData[0] = pLight->m_diffuseColor.r;
		afData[1] = pLight->m_diffuseColor.g;
		afData[2] = pLight->m_diffuseColor.b;
		afData[3] = 1.f;
	}
	else 
	{
		afData[0] = 0.f;
		afData[1] = 0.f;
		afData[2] = 0.f;
		afData[3] = 1.f;
	}
}
void U2Dx9Renderer::SetConstantLightSpecular (int iLight, float* afData)
{

	U2Light* pLight = GetLight(iLight);
	U2ASSERT(pLight);
	afData[0] = pLight->m_specularColor.r;
	afData[1] = pLight->m_specularColor.g;
	afData[2] = pLight->m_specularColor.b;
	afData[3] = 1.f;

}


void U2Dx9Renderer::SetConstantLightSpotCutoff (int iLight, float* afData)
{

	U2Light* pLight = GetLight(iLight);	
	if(pLight)
	{	
		afData[0] = pLight->m_fAngle;
		afData[1] = pLight->m_fCosAngle;
		afData[2] = pLight->m_fSinAngle;
		afData[3] = pLight->m_fExponent;
	}
	else 
	{
		afData[0] = U2_PI;
		afData[1] = -1.f;
		afData[2] = 0.f;
		afData[3] = 1.f;
	}

}


void U2Dx9Renderer::SetConstantLightAttenuation (int iLight, float* afData)
{

	U2Light* pLight = GetLight(iLight);	
	if(pLight)
	{	
		afData[0] = pLight->m_fConstCofficient;
		afData[1] = pLight->m_fLinearCofficient;
		afData[2] = pLight->m_fQuadraticCofficient;
		afData[3] = pLight->m_fIntensityFactor;
	}
	else 
	{
		afData[0] = U2_PI;
		afData[1] = -1.f;
		afData[2] = 0.f;
		afData[3] = 1.f;
	}
}
