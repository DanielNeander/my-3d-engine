#include "stdafx.h"
#include "GameApp/GameApp.h"
#include "ModelLib/M2Loader.h"
#include "../Mesh.h"
#include "../M2Mesh.h"

#include "DebugDraw.h"
#include "../AI/NavDebugDraw.h"

#include "IK.h"

InverseKinematics::InverseKinematics( M2Mesh* pSkinnedMesh )
	:m_pHeadBone(pSkinnedMesh->m2_->GetBone(BONE_HEAD)),
	m_pSkinnedMesh(pSkinnedMesh)
{
	
	dd_ = new NavDebugDraw;
	dd_->setOffset(0.15f);
	dd_->CreateVertexBuffer(NULL, 2, sizeof(DebugVertex));
	//ddMain->CreateIndexBuffer(geom->getMesh()->getTris(), geom->getMesh()->getTriCount() * 3);
	dd_->SetupShader();
	//// Calculate the local forward vector for the head bone

	//// Remove translation
	//noMat4 headMat = m_pHeadBone.mat;
	//headMat.mat[0][3] = 0.f;
	//headMat.mat[1][3] = 0.f;
	//headMat.mat[2][3] = 0.f;
	//
	//noMat4 toHeadSpace;;
	//toHeadSpace = headMat.Inverse();
	//
	//// Our model is looking towards -z in the content
	//noVec3 v;
	////D3DXVec3Transform(&vec, &D3DXVECTOR3(0, 0, -1), &toHeadSpace);
	//v = toHeadSpace * noVec3(1.f, 0.f, 0.f);
	//m_headForward = v;
}


void InverseKinematics::UpdateHeadIK()
{
	//Get look at point from cursor location
	POINT p;
	::GetCursorPos(&p);
	LONG width = GetApp()->getWidth();
	LONG height = GetApp()->getHeight();
	p.x = Max(Min(p.x, width), (LONG)0);
	p.y = Max(Min(p.y, height), (LONG)0);
	float x = (p.x - 0.5f * width) / width;
	float y = (p.y - 0.5f * height) / height;	
	noVec3 lookAt(1.f, 1.5f - y, x);

	ApplyLookAtIK(lookAt, DEG2RAD(60.0f));
}

// Uses shortest arc rotation algorithm to turn the head bone towards a position
void InverseKinematics::ApplyLookAtIK( noVec3 &lookAtTarget, float maxAngle )
{
	// Calculate the local forward vector for the head bone

	static bool bFirst = false;

	if (!bFirst)
	{
		// Remove translation
		noMat4 headMat = m_pHeadBone.mat;
		headMat.mat[0][3] = 0.f;
		headMat.mat[1][3] = 0.f;
		headMat.mat[2][3] = 0.f;
		headMat.TransposeSelf();

		noMat4 toHeadSpace;;
		toHeadSpace = headMat.Inverse();

		// Our model is looking towards -z in the content
		noVec3 v;
		//D3DXVec3Transform(&vec, &D3DXVECTOR3(0, 0, -1), &toHeadSpace);			
		v = toHeadSpace * noVec3(1.f, 0.f, 0.f);
		m_headForward = v;
		

		bFirst = true;
	}

	// Start by transforming to local space
	noMat4 trans;
	trans.Translation(m_pSkinnedMesh->m2_->GetBone(BONE_HEAD).pivot);
	noMat4 temp = m_pHeadBone.mat * trans;	
	temp.TransposeSelf();
	
	noMat4 mtxToLocal = temp.Inverse();

	

	
	// Footnote: the matrix inverse could be avoided if we would ignore to update the local transform
	noVec3 localLookAt;
	localLookAt = mtxToLocal * lookAtTarget;
	// Normalize local look at target
	localLookAt.Normalize();
	m_localLookAt = localLookAt;


	// Get rotation angle and angle
	noVec3 localRotationAxis;
	localRotationAxis = m_headForward.Cross(localLookAt);
	localRotationAxis.Normalize();
	m_localRotationAxis = localRotationAxis;


	float localAngle = noMath::ACos(m_headForward * localLookAt);

	// Limit Angle
	localAngle = min( localAngle, maxAngle );

	// Apply the transformation to the bone
	noMat4 rotation;
	noRotation rot(vec3_zero, localRotationAxis, RAD2DEG(localAngle));
	rotation = rot.ToMat4();
	m_pHeadBone.mat = m_pHeadBone.mat * rotation;
	memcpy(&m_pHeadBone.rdmat, &m_pHeadBone.mat.Transpose(), sizeof(float) * 16);			
	m_pSkinnedMesh->UpdateBoneMatrix(BONE_HEAD, m_pHeadBone.mat);
}

void InverseKinematics::DrawDebug()
{
	dd_->begin(DU_DRAW_LINES_STRIP);

	//pickPos_ = PickUtil::GetPickVert();	
	int col = duRGBA(255,0,0,64);

	noVec3 orig(m_pSkinnedMesh->m2_->GetBone(BONE_HEAD).transPivot);
	orig =  m_pSkinnedMesh->worldTM_  * orig;

	noTransform tm;
	tm.MakeIdentity();
	tm.rotate_ = m_pSkinnedMesh->worldTM_.rotate_;
			
	dd_->vertex(orig.ToFloatPtr(), col);	
	m_localLookAt = orig + tm * m_localLookAt;
	dd_->vertex(m_localLookAt.x, m_localLookAt.y, m_localLookAt.z, col );
	dd_->end();


	col = duRGBA(255,255,0,64);
	m_localRotationAxis = orig +  tm * m_localRotationAxis * 3;
	noVec3 forward = orig + tm * m_headForward;
	
	dd_->begin(DU_DRAW_LINES_STRIP);
	dd_->vertex(orig.ToFloatPtr(), col );	
	dd_->vertex(forward.ToFloatPtr(), col);
	dd_->end();	

	//col = duRGBA(0,255,255,64);	
	dd_->begin(DU_DRAW_LINES_STRIP);
	dd_->vertex(orig.ToFloatPtr(), col );	
	dd_->vertex(m_localRotationAxis.ToFloatPtr(), col);
	dd_->end();	
}

