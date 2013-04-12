#include "stdafx.h"
#include "GameApp.h"
#include "SceneNode.h"
#include "Mesh.h"
#include "M2Mesh.h"
#include "Mesh_D3D11.h"
#include "Common.h"

#include "ModelLib/M2Loader.h"
#include "ModelLib/M2Particle.h"
#include "M2Object.h"
#include "M2EffectRender.h"
#include "M2Equipment.h"
#include "TestObjects.h"
#include "GameApp/Effects/TrailEffect.h"

#include "DebugDraw.h"
#include "AI/NavDebugDraw.h"

#include "Animation/IK.h"
#include "Font/Dx11TextHelper.h"
#include "U2Light.h"

extern U2Light gSun;



void M2Skeleton::extractSkeleton()
{
	pBoneMatrixPtrs = new noMat4*[model->header.nBones];
	pBoneOffsetMatrices = new noMat4[model->header.nBones];
	bones_ = new M2Bone[model->header.nBones];
	ZeroMemory(bones_, sizeof(M2Bone) * model->header.nBones);

	rootBone_ = &bones_[model->GetBoneId(BONE_ROOT)];

	for(int i=0; i < BONE_MAX; ++i)
	{
		pBoneOffsetMatrices[i].Identity();
		pBoneMatrixPtrs[i] = NULL;		
		if (model->GetBoneId(i) != -1)
		{			
			CreateBoneTree(model->GetBoneId(i), NULL);		
		}
	}
}

void M2Skeleton::CreateBoneTree(int bone, M2Bone* childBone)
{	
	if (model->GetBoneId(bone) == -1) 
		return;
	
	if(model->bones[bone].parent > -1)
	{		
		bones_[model->bones[bone].parent].pFirstChild = &bones_[bone];
		CreateBoneTree(model->bones[bone].parent, &bones_[bone]);
	}

	if (!bones_[bone].pFirstChild) bones_[bone].pFirstChild = childBone;	
	bones_[bone].mat = &model->bones[bone].mat;
	bones_[bone].rdmat = &model->bones[bone].rdmat;
	bones_[bone].boneId = bone;
}



const Bone& M2Mesh::GetBone(int bone) const
{
	return m2_->GetBone(bone);
}

Bone& M2Mesh::GetBone( int bone )
{
	return m2_->GetBone(bone);
}
static std::vector<bool> boneFlags;

void M2Mesh::UpdateBoneMatrix(int parent, const noMat4& parentMat)
{

	//if (boneFlags.empty()) {
		boneFlags.resize(m2_->header.nBones);		
	//}	

	for (size_t i=0; i< BONE_MAX; i++) {
		if (m2_->bones[i].parent == parent)		
		{			
			m2_->bones[i].mat = parentMat * m2_->bones[i].mat;
			memcpy(&m2_->bones[i].rdmat, &m2_->bones[i].mat.Transpose(), sizeof(float) * 16);
			boneFlags[i] = true;
		}
	}		

	/*for (size_t i=0; i<m2_->header.nBones; i++) { 
	if (!boneFlags[i]) {
	if (m2_->bones[i].parent > -1) {
	m2_->bones[i].mat = m2_->bones[m2_->bones[i].parent].mat * m2_->bones[i].mat;
	memcpy(&m2_->bones[i].rdmat, &m2_->bones[i].mat.Transpose(), sizeof(float) * 16);
	boneFlags[i] = true;
	}
	}
	}		*/
}

void M2Mesh::DrawBoundingVolumes(RenderData& rd)
{
	Renderer* renderer = GetApp()->getRenderer();
	renderer->reset();	
	renderer->setVertexFormat(rd.vf);
	renderer->setVertexBuffer(0, rd.vb);
	renderer->setIndexBuffer(rd.ib);
	renderer->setRasterizerState(rd.cull);
	renderer->setDepthState(rd.depthMode);
	renderer->setShader(rd.shader);	
	//renderer->setTexture("txDiffuse", rd.baseTex);
	//renderer->setSamplerState("samLinear", rd.linear);	
	noMat4 tm(pDebugBBox_->worldTM_.rotate_ * pDebugBBox_->worldTM_.scale_, pDebugBBox_->worldTM_.trans_);
	tm.TransposeSelf();
	renderer->setShaderConstant4x4f("World", ToMat4(tm));
	renderer->setShaderConstant4x4f("View", ViewMat());
	renderer->setShaderConstant4x4f("Projection", ProjMat());	
	renderer->apply();
	renderer->drawElements(rd.prim, rd.startIndex, rd.endIndex, 0, rd.vertexEnd);
}

void M2Mesh::UpdateIK()
{
	if (!m_pIK) return; 

	//m_pIK->UpdateHeadIK();
	//boneFlags.clear();
}

bool M2Mesh::CreateBuffers( void )
{	
	if (m2_->modelType == MT_CHAR)			
		m_pIK = new InverseKinematics(this);
	/*if(m2_->IsSKinned())
	{
		gSkel = new M2Skeleton(m2_);
		gSkel->extractSkeleton();
	}*/		
	if (m2_->bounds)
	{
		if (m2_->modelType == MT_CHAR)
			m_OOBB.Set(m2_->bounds, m2_->header.nBoundingVertices, sizeof(noVec3));
		else 
		{
			if (m2_->IsSKinned())
			{
				std::vector<noVec3> vertices;
				vertices.resize(m2_->numVertices);
				for ( int i=0; i < m2_->numVertices; ++i)
					vertices[i] = m2_->vertDatas[i].pos;				
				m_OOBB.Set(&vertices[0], m2_->numVertices, sizeof(noVec3));
			}
			else
				m_OOBB.Set(m2_->vertices, m2_->numVertices, sizeof(noVec3));
		}
		CalculateAABB();
		Mesh* pMesh = new Mesh_D3D11;
		pMesh->m_PrimitiveType = PRIM_TRIANGLES;
		pMesh->m_pVertices = (float *)m2_->bounds;
		pMesh->m_pIndices	= m2_->boundIndices;
		pMesh->m_iNumVertices =  m2_->header.nBoundingVertices;
		pMesh->m_iNumTris =  m2_->header.nBoundingTriangles / 3;
		pMesh->m_iVertexSize = sizeof(noVec3);
		pBBox_ = pMesh;
		AddChild(pBBox_);				
		pDebugBBox_ = makeMesh(box_rd, m2_->bounds, m2_->header.nBoundingVertices, m2_->boundIndices, m2_->header.nBoundingTriangles);
		AddChild(pDebugBBox_);		
	}

	if (m2_->particleSystems || m2_->ribbons)
	{
		EffectRender_ = new M2EffectRender(m2_);
		AddChild(EffectRender_);
		EffectRender_->CreateResources();
	}

#if 1
	dd_ = new NavDebugDraw;
	dd_->setOffset(0.15f);
	dd_->CreateVertexBuffer(NULL, 2, sizeof(DebugVertex));
	//ddMain->CreateIndexBuffer(geom->getMesh()->getTris(), geom->getMesh()->getTriCount() * 3);
	dd_->SetupShader();
#endif
	return true;
}



void M2Mesh::DrawShadow( void )
{
	noMat4 tm(worldTM_.rotate_ * worldTM_.scale_, worldTM_.trans_);
	tm.TransposeSelf();	
	m2_->DrawShadow(ToMat4(tm), ToMat4(gSun.m_mView), ToMat4(gSun.m_mProj));	
}


void M2Mesh::DrawCascadedShadow( void )
{
	noMat4 tm(worldTM_.rotate_ * worldTM_.scale_, worldTM_.trans_);
	tm.TransposeSelf();	
	m2_->DrawZOnly(tm, ToMat4(gSun.m_mView), ToMat4(gSun.m_mProj));	


	// 와우 모델 뷰어에서 파티클을 렌더링 하지 않아도 렌더 스테이트 때문에 
	// 모델의 색깔이 변하는 것 주의... (이거 찾는데.. 시간 오래걸림.)		

	if (EffectRender_) 	
	{
		EffectRender_->UpdateEffects();
		EffectRender_->Render();		
	}
}


void M2Mesh::DrawZOnly( void )
{
	if (!m2_) NULL;

	noMat4 tm(worldTM_.rotate_ * worldTM_.scale_, worldTM_.trans_);
	tm.TransposeSelf();	
	m2_->DrawZOnly(tm, ToMat4(gSun.m_mView), ToMat4(gSun.m_mProj));	

	if (drawDebug_)
	{	

		if (m2_->IsSKinned())
			DrawBones();

		if (pDebugBBox_)
			DrawBoundingVolumes(box_rd);

		if (dd_)
		{	
			dd_->begin(DU_DRAW_LINES_STRIP);

			//pickPos_ = PickUtil::GetPickVert();	
			int col = duRGBA(0,192,255,64);

			// forward vector
			dd_->vertex(worldTM_.trans_.x, worldTM_.trans_.y, worldTM_.trans_.z, col );

			noVec3 target = worldTM_.trans_ + worldTM_.rotate_.mat[0] * 3.f;
			dd_->vertex(target.ToFloatPtr(), col);

			dd_->end();

			dd_->begin(DU_DRAW_LINES_STRIP);

			// right vector
			dd_->vertex(worldTM_.trans_.x, worldTM_.trans_.y, worldTM_.trans_.z, col );
			target = worldTM_.trans_ + worldTM_.rotate_.mat[2] * 3.f;
			dd_->vertex(target.ToFloatPtr(), col);

			dd_->end();	
		}
	}

	if (m_pIK) m_pIK->DrawDebug();

	for (size_t i=0; i < childNodes_.size(); ++i)
	{
		M2Mesh* pMesh = dynamic_cast<M2Mesh*>(childNodes_[i]);
		if (pMesh)
			pMesh->DrawZOnly();
		TrailMesh* pTrail = dynamic_cast<TrailMesh*>(childNodes_[i]);
		if (pTrail)
			pTrail->Draw();
	}
}


void M2Mesh::Draw( void )
{
	if (!m2_) NULL;
	
		
	noMat4 tm(worldTM_.rotate_ * worldTM_.scale_, worldTM_.trans_);
	tm.TransposeSelf();	

	switch (AppSettings::ShadowModeEnum)
	{
	case SHADOW_NONE:
		m2_->DrawModel(ToMat4(tm), ViewMat(), ProjMat());		
		break;		
	case SHADOW_PSSM:		
		m2_->DrawShadowModel(ToMat4(tm), ViewMat(), ProjMat());	
		break;
	case SHADOW_CASCADED:		
		m2_->DrawCascaded();
	case SHADOW_SSAO:		
		m2_->DrawSSAO(ToMat4(tm), ViewMat(), ProjMat());
	}		

	
	
	// 와우 모델 뷰어에서 파티클을 렌더링 하지 않아도 렌더 스테이트 때문에 
	// 모델의 색깔이 변하는 것 주의... (이거 찾는데.. 시간 오래걸림.)		

	if (EffectRender_) 	
	{
		EffectRender_->UpdateEffects();
		EffectRender_->Render();		
	}
			
	if (drawDebug_)
	{	
	
		if (m2_->IsSKinned())
			DrawBones();

		if (pDebugBBox_)
			DrawBoundingVolumes(box_rd);

		if (dd_)
		{	
			dd_->begin(DU_DRAW_LINES_STRIP);

			//pickPos_ = PickUtil::GetPickVert();	
			int col = duRGBA(0,192,255,64);
			
			// forward vector
			dd_->vertex(worldTM_.trans_.x, worldTM_.trans_.y, worldTM_.trans_.z, col );

			noVec3 target = worldTM_.trans_ + worldTM_.rotate_.mat[0] * 3.f;
			dd_->vertex(target.ToFloatPtr(), col);
		
			dd_->end();

			dd_->begin(DU_DRAW_LINES_STRIP);

			// right vector
			dd_->vertex(worldTM_.trans_.x, worldTM_.trans_.y, worldTM_.trans_.z, col );
			target = worldTM_.trans_ + worldTM_.rotate_.mat[2] * 3.f;
			dd_->vertex(target.ToFloatPtr(), col);

			dd_->end();	
		}
	}

	//if (m_pIK) m_pIK->DrawDebug();

	for (size_t i=0; i < childNodes_.size(); ++i)
	{
		M2Mesh* pMesh = dynamic_cast<M2Mesh*>(childNodes_[i]);
		if (pMesh)
			pMesh->Draw();
		TrailMesh* pTrail = dynamic_cast<TrailMesh*>(childNodes_[i]);
		if (pTrail)
			pTrail->Draw();
	}
}

M2Mesh::M2Mesh() : m2_(NULL), m_pIK(NULL)
	,dd_(NULL),gSkel(NULL), id_(-1), EffectRender_(NULL),drawDebug_(false)
{
	ZeroMemory(showBones_, sizeof(bool) * 164);
	pBBox_ = pDebugBBox_ = 0; 
	owner_ = NULL;
}

M2Mesh::~M2Mesh()
{
	Destory();
}

void M2Mesh::Destory()
{
	SAFE_DELETE(EffectRender_);	
	SAFE_DELETE(m_pIK);
	SAFE_DELETE(dd_);
	SAFE_DELETE(m2_);
}


void M2Mesh::DrawBones()
{
	for (size_t i=0; i< BONE_MAX; i++) 
	{		
		if (m2_->bones[i].parent != -1) {
			
			int x;
			/*for (x=0; x < BONE_MAX ; ++x)
			{
				if (i == m2_->GetBoneId(x))
					break;
			}*/
			if (showBones_[i])
			{	
				dd_->begin(DU_DRAW_LINES_STRIP);


				//pickPos_ = PickUtil::GetPickVert();	
				int col = duRGBA(0,192,255,64);

				noVec3 v0 = worldTM_ * m2_->bones[i].transPivot;
				dd_->vertex(v0.x, v0.y, v0.z, col );

				noVec3 v1 = worldTM_ * m2_->bones[m2_->bones[i].parent].transPivot;
				dd_->vertex(v1.ToFloatPtr(), col);

				dd_->end();

				GetApp()->TextRender()->Begin();
				GetApp()->TextRender()->SetForegroundColor(D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );	

				noVec3 v;
				GetCamera()->project(v0.x, v0.y, v0.z, GetApp()->getWidth(), GetApp()->getHeight(), v);

				RECT rc;
				rc.left = v.x; rc.top = v.y;			

				GetApp()->TextRender()->DrawFormattedTextLine(rc, 0, BoneTableNames[i]);
				GetApp()->TextRender()->End();
			}
		}
	}


}

void M2Mesh::ToggleBones(int index)
{
	showBones_[index] = !showBones_[index];	
		
}

void M2Mesh::ToggleDebug()
{
	drawDebug_ = !drawDebug_;
}

void M2Mesh::UpdateTM()
{
	Mesh::UpdateTM();

	if( owner_)
	{

		if (id_ == 1)
		{				
			owner_->meshEdit_->Equip_->RightHandBone_->SetTrans(worldTM_.trans_);
			owner_->meshEdit_->Equip_->RightHandBone_->SetRotate(worldTM_.rotate_);
			owner_->meshEdit_->Equip_->RightHandBone_->UpdateTM();
		}
		else if (id_ == 2)
		{				
			owner_->meshEdit_->Equip_->LeftHandBone_->SetTrans(worldTM_.trans_);
			owner_->meshEdit_->Equip_->LeftHandBone_->SetRotate(worldTM_.rotate_);
			owner_->meshEdit_->Equip_->LeftHandBone_->UpdateTM();
		}

	}

	CalculateAABB();
}

void M2Mesh::UpdateData()
{	

	if (m2_->IsSKinned() || EffectRender_)
		m2_->update(GetApp()->ddt, GetApp()->globalTime);
		

	if (!parent_)
		return;

	M2Mesh* parent = dynamic_cast<M2Mesh*>(parent_);

	if(id_ > -1 && parent && parent->m2_ )
	{
		int i = parent->m2_->attLookup[id_];
		if (i > -1)
		{
			int bone = parent->m2_->atts[i].bone;
			noMat4 m = parent->m2_->bones[bone].mat;											
			noMat4 t;			
			t.Translation(parent->m2_->atts[i].pos);									
			noMat4 res = m * t;
			res.TransposeSelf();

			if (localTM_.isDirty)
			{
				noTransform tm;
				tm.MakeIdentity();
				tm.rotate_.mat[0] = noVec3(res.mat[0].x, res.mat[0].y, res.mat[0].z);
				tm.rotate_.mat[1] = noVec3(res.mat[1].x, res.mat[1].y, res.mat[1].z);
				tm.rotate_.mat[2] = noVec3(res.mat[2].x, res.mat[2].y, res.mat[2].z);																			
				tm.trans_ = noVec3(res.mat[3].x, res.mat[3].y, res.mat[3].z);													

				localTM_ = tm * localTM_;
			}
			else 
			{						
				localTM_.rotate_.mat[0] = noVec3(res.mat[0].x, res.mat[0].y, res.mat[0].z);
				localTM_.rotate_.mat[1] = noVec3(res.mat[1].x, res.mat[1].y, res.mat[1].z);
				localTM_.rotate_.mat[2] = noVec3(res.mat[2].x, res.mat[2].y, res.mat[2].z);																			
				localTM_.trans_ = noVec3(res.mat[3].x, res.mat[3].y, res.mat[3].z);													
				localTM_.isDirty = true;
			}
			
			
			
		}		
	}	
	
	UpdateIK();
}

void M2Mesh::DrawCascaded( void )
{
	m2_->DrawCascaded();
}

void M2Mesh::PreUpdate()
{

}

void M2Mesh::PostUpdate()
{

}
