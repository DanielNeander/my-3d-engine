#include "stdafx.h"
#include "GameApp.h"
#include "CascadedShadowsManager.h"
#include "GameApp/Mesh.h"
#include "GameApp/SceneNode.h"
#include "GameApp/M2Mesh.h"
#include "GameApp/Mesh_D3D11.h"
#include "GameApp/TestObjects.h"
#include "GameApp/GameObjectManager.h"
#include "GameApp/GameObjectUtil.h"
#include "GameApp/Common.h"
#include "ModelLib/M2Loader.h"
#include "GameApp/Actor.h"
#include "GameApp/Font/Dx11TextHelper.h"
#include "SDKMeshLoader.h"

#include "U2Light.h"

extern  std::vector<Mesh*>	gShadowReceivers;

CascadeConfig               g_CascadeConfig;
static CascadedShadowsManager* gCascadedShadow;
CDXUTSDKMesh                g_MeshPowerPlant;
CDXUTSDKMesh                g_MeshTestScene;
CDXUTSDKMesh*				g_pSelectMesh;
extern CDXUTSDKMesh gSample;

void App::LoadCascadedShadows()
{
	g_CascadeConfig.m_nCascadeLevels = 3;
	g_CascadeConfig.m_iBufferSize = 1024;
	g_CascadeConfig.m_ShadowBufferFormat = CASCADE_DXGI_FORMAT_R32_TYPELESS;

	pCascaded_ = new CascadedShadowsManager(&g_CascadeConfig);
	gCascadedShadow =pCascaded_;
	pCascaded_->m_iCascadePartitionsZeroToOne[0] = 5;
	pCascaded_->m_iCascadePartitionsZeroToOne[1] = 15;
	pCascaded_->m_iCascadePartitionsZeroToOne[2] = 60;
	pCascaded_->m_iCascadePartitionsZeroToOne[3] = 100;
	pCascaded_->m_iCascadePartitionsZeroToOne[4] = 100;
	pCascaded_->m_iCascadePartitionsZeroToOne[5] = 100;
	pCascaded_->m_iCascadePartitionsZeroToOne[6] = 100;
	pCascaded_->m_iCascadePartitionsZeroToOne[7] = 100;
	pCascaded_->m_iCascadePartitionsMax = 100;

	pCascaded_->m_bMoveLightTexelSize = true;
	pCascaded_->m_eSelectedCascadesFit = FIT_TO_SCENE;
	pCascaded_->m_eSelectedNearFarFit =  FIT_NEARFAR_SCENE_AABB;
	pCascaded_->m_eSelectedCascadeSelection = CASCADE_SELECTION_MAP;

	
	float eye[] = {  -320.0f, 300.0f, -220.3f };
	float at[] = { 0.0f, 0.0f, 0.f };
	float up[] = {  0.0f, 1.0f, 0.0f };

	DefCam_->setFrom(noVec3(100.f, 5.f, 5.f).ToFloatPtr());
	DefCam_->setTo(vec3_zero.ToFloatPtr());
	DefCam_->setUp(up);
	DefCam_->computeModelView();
	//pShadowCam_ = new ShadowMapCamera(eye, at, up, 45.f);
	//pShadowCam_->setFar(1000.0f);
	//pShadowCam_->setNear(0.1f);
	//pShadowCam_->SetAspect(width / (FLOAT)height);	
	//pShadowCam_->setHandednessMode(CAMERA_HANDEDNESS_LEFT);		
	//pShadowCam_->computeModelView();
	//pShadowCam_->ComputeProjection();		
	BoundingBox unionBB;	
	{
		 for (int i = 0; i < m_WorldRoot->childNodes_.size(); ++i)
		 {
			 Mesh* pMesh = (Mesh*)m_WorldRoot->childNodes_[i];
			 unionBB.Union(pMesh->m_AABB);
		 }	

		 g_MeshPowerPlant.Create( device, L"powerplant\\powerplant.sdkmesh" );
		 //g_MeshTestScene.Create( device, L"ShadowColumns\\testscene.sdkmesh" ) ;
	 	
		 g_pSelectMesh = &g_MeshPowerPlant;
		 m_WorldRoot->AddChild(g_pSelectMesh);
		 //g_pSelectMesh = &g_MeshTestScene;	 

		 noVec3 vMeshMin;
		 noVec3 vMeshMax;

		 noVec3 sceneMin(FLT_MAX, FLT_MAX, FLT_MAX);
		 noVec3 sceneMax(FLT_MIN, FLT_MIN, FLT_MIN);
		 for( UINT i =0; i < g_pSelectMesh->GetNumMeshes( ); ++i ) 
		 {
			 SDKMESH_MESH* msh = g_pSelectMesh->GetMesh( i );
			 vMeshMin.Set(msh->BoundingBoxCenter.x - msh->BoundingBoxExtents.x,
				 msh->BoundingBoxCenter.y - msh->BoundingBoxExtents.y,
				 msh->BoundingBoxCenter.z - msh->BoundingBoxExtents.z );

			 vMeshMax.Set(msh->BoundingBoxCenter.x + msh->BoundingBoxExtents.x,
				 msh->BoundingBoxCenter.y + msh->BoundingBoxExtents.y,
				 msh->BoundingBoxCenter.z + msh->BoundingBoxExtents.z );

			 noVec3 temp = vMeshMin;
			 temp.Min( sceneMin );
			 sceneMin = temp;
			 temp = vMeshMax;
			 temp.Max( sceneMax );		 
			 sceneMax = temp;
		 }
		 unionBB.m_vMin = sceneMin;
		 unionBB.m_vMax = sceneMax;	 
	}
	 
	 pCascaded_->Init(unionBB, DefCam_, pShadowCam_ );
}

//--------------------------------------------------------------------------------------
// Calcaulte the camera based on size of the current scene
//--------------------------------------------------------------------------------------
void App::UpdateViewerCameraNearFar () 
{
	if (!pCascaded_) return;
	noVec4 vMeshExtents = gCascadedShadow->GetSceneAABBMax() - gCascadedShadow->GetSceneAABBMin();
	float vMeshLength = ( vMeshExtents ).Length();	
	GetApp()->GetShadowCam()->SetAspect(GetApp()->GetAspectRatio());
	GetApp()->GetShadowCam()->setNear(0.05f);
	GetApp()->GetShadowCam()->setFar(vMeshLength);
	GetApp()->GetShadowCam()->ComputeProjection();	
}

void App::CasCaded_ShadowPass( float fDeltaTime )
{
	if (!pCascaded_) return;

	pShadowCam_->Update(fDeltaTime);
	
	pCascaded_->InitFrame( device );
		
	pCascaded_->EnableRendering();
	pCascaded_->RenderShadowsForAllCascades();
	pCascaded_->RenderShadow2(g_pSelectMesh);
	pCascaded_->DisableRendering();

}

void App::RenderCascasedShadowedScene()
{	
	if (!pCascaded_) return;

	//FLOAT ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.55f };
	//renderer->changeToMainFramebuffer();	

	//ID3D11RenderTargetView* pBackBufferRTV; 
	//ID3D11DepthStencilView* pBackBufferDSV;

	//context->OMGetRenderTargets(1, &pBackBufferRTV, &pBackBufferDSV);

	//context->ClearRenderTargetView( pBackBufferRTV, ClearColor );
	// Clear the depth stencil
	//context->ClearDepthStencilView( pBackBufferDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	


	//Mesh* pMesh = (Mesh*)m_WorldRoot->childNodes_[0];
	//drawModel(pMesh->m_rd);

	for (size_t i = 0; i < gShadowReceivers.size(); ++i)
		pCascaded_->RenderScene(ActiveCam_, gShadowReceivers[i]);
	
#if 1
	
	pCascaded_->RenderScene2(ActiveCam_, g_pSelectMesh);
#endif
	
	//pCascaded_->RenderScene( pd3dImmediateContext, pRTV, pDSV, g_pSelectedMesh, g_pActiveCamera,  &vp, g_bVisualizeCascades );

}