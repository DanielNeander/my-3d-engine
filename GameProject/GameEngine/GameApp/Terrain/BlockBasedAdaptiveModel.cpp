// Copyright 2011 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
#include "stdafx.h"
#include "BlockBasedAdaptiveModel.h"
#include "EngineCore/TBB/TaskMgrTBB.h"

CTaskBase::CTaskBase()
    : m_bTaskComplete(false)
    , m_TaskHandle(TASKSETHANDLE_INVALID)
{
}

CTaskBase :: ~CTaskBase()
{
    assert( CheckCompletionStatus() );
    // Release handle
    if( TASKSETHANDLE_INVALID != m_TaskHandle )
        gTaskMgr.ReleaseHandle( m_TaskHandle );
}

bool CTaskBase::CheckCompletionStatus()
{ 
    return ( m_bTaskComplete && ( TASKSETHANDLE_INVALID == m_TaskHandle || gTaskMgr.IsSetComplete(m_TaskHandle)) );
}

void CTaskBase::WaitForTaskCompletion()
{ 
    if( TASKSETHANDLE_INVALID != m_TaskHandle )
        gTaskMgr.WaitForSet(m_TaskHandle); 
}

///////////////////////////////////////////////////////////////////////////////

CIncreaseLODTask::CIncreaseLODTask(std::auto_ptr<CPatchQuadTreeNode> pDescendants[4],
	                               CElevationDataSource *pDataSource,
	                               CTriangDataSource *pTriangDataSource,
                                   const class CBlockBasedAdaptiveModel *pBlockBasedModel)
    : CTaskBase()
    , m_pDataSource(pDataSource)
    , m_pTriangDataSource(pTriangDataSource)
    , m_pBlockBasedModel(pBlockBasedModel)
{
    std::copy(pDescendants, pDescendants + 4, m_pFloatingDescendantNodes);
}

CIncreaseLODTask::~CIncreaseLODTask()
{
    // It is essentially important to wait for task completion before destroying the object
    // If other thread is accessing the data while main thread releases it, a crash will occur
    // Note that we can't do this in base class destructor, becuase all inherited class data
    // is already destroyed there
    WaitForTaskCompletion();
}

void CIncreaseLODTask::DetachFloatingDescendants(std::auto_ptr<CPatchQuadTreeNode> &pLBDescendant,
                                                 std::auto_ptr<CPatchQuadTreeNode> &pRBDescendant,
                                                 std::auto_ptr<CPatchQuadTreeNode> &pLTDescendant,
                                                 std::auto_ptr<CPatchQuadTreeNode> &pRTDescendant)
{
    pLBDescendant = m_pFloatingDescendantNodes[0];
    pRBDescendant = m_pFloatingDescendantNodes[1];
    pLTDescendant = m_pFloatingDescendantNodes[2];
    pRTDescendant = m_pFloatingDescendantNodes[3];
}

void STDMETHODCALLTYPE CIncreaseLODTask::Execute()
{
    // Do all work required to refine the model
    for(int iChildNum = 0; iChildNum < 4; iChildNum++)
    {
        // Get child height maps
        m_pFloatingDescendantNodes[iChildNum]->GetData().m_pElevData.reset( m_pDataSource->GetElevData( m_pFloatingDescendantNodes[iChildNum]->GetPos() ) );
    }

    if( m_pTriangDataSource )
    {
        // Get triangulations
        for(int iChildNum = 0; iChildNum < 4; iChildNum++)
        {
            m_pFloatingDescendantNodes[iChildNum]->GetData().m_pAdaptiveTriangulation.reset(
                    m_pTriangDataSource->DecodeTriangulation( m_pFloatingDescendantNodes[iChildNum]->GetPos() ) );
        }
    }

    // Create patches
    for(int iChildNum = 0; iChildNum < 4; iChildNum++)
    {
        CPatchQuadTreeNode &CurrChildNode = *m_pFloatingDescendantNodes[iChildNum];

        CurrChildNode.GetData().pPatch = 
            m_pBlockBasedModel->CreatePatch( CurrChildNode.GetData().m_pElevData.get(), 
                                             CurrChildNode.GetData().m_pAdaptiveTriangulation.get() );

		
        float fPatchElevDataErrorBound = m_pDataSource->GetPatchElevDataErrorBound(CurrChildNode.GetPos()) * m_pBlockBasedModel->m_Params.m_fElevationScale;
        float fTriangulationError = 0.f;
        if( m_pTriangDataSource )
            fTriangulationError = m_pTriangDataSource->GetTriangulationErrorBound(CurrChildNode.GetPos());
        CurrChildNode.GetData().m_fGuaranteedPatchErrorBound = (fPatchElevDataErrorBound + fTriangulationError) * m_pBlockBasedModel->m_Params.m_fElevationScale ;
        m_pBlockBasedModel->CalculatePatchBoundingBox( CurrChildNode.GetPos(), m_pDataSource, CurrChildNode.GetData().BoundBox);
    }
	m_bTaskComplete = true;
}

CDecreaseLODTask::CDecreaseLODTask( CPatchQuadTreeNode &Node,
                                    const class CBlockBasedAdaptiveModel *pBlockBasedModel):
    m_pNode(&Node),
    m_pBlockBasedModel(pBlockBasedModel),
    m_pDataSource(pBlockBasedModel->m_pDataSource),
    m_pTriangDataSource(pBlockBasedModel->m_pTriangDataSource)
{
}

CDecreaseLODTask::~CDecreaseLODTask()
{
    // It is essentially important to wait for task completion before destroying the object
    // If other thread is accessing the data while main thread releases it, a crash will occur
    // Note that we can't do this in base class destructor, becuase all inherited class data
    // is already destroyed there
    WaitForTaskCompletion();
}

void STDMETHODCALLTYPE CDecreaseLODTask::Execute()
{
    // Do nothing at this moment
    m_bTaskComplete = true;
}


///////////////////////////////////////////////////////////////////////////////

CBlockBasedAdaptiveModel::CBlockBasedAdaptiveModel(void) : 
    m_iTotalTrianglesRendered(0)
{
    D3DXMATRIX mDummyProj;
    D3DXMatrixIdentity(&mDummyProj);
    SetViewFrustumParams(1024, 768, mDummyProj);
	InitializeSRWLock(&m_srwLock);
}

CBlockBasedAdaptiveModel::~CBlockBasedAdaptiveModel(void)
{
    // Task manager must be destroyed after all tasks are completed!
    gTaskMgr.Shutdown();
}

HRESULT CBlockBasedAdaptiveModel::Init(const SRenderingParams &Params,
                                       CElevationDataSource *pDataSource,
                                       CTriangDataSource *pTriangDataSource)
{
    m_Params = Params;
    m_pDataSource = pDataSource;
    m_pTriangDataSource = pTriangDataSource;

    m_iNumLevelsInPatchHierarchy = m_pDataSource->GetNumLevelsInHierarchy();
    m_iPatchSize = m_pDataSource->GetPatchSize();
    m_iNumLevelsInLocalPatchQT = 0;
    while( (1 << m_iNumLevelsInLocalPatchQT) <= m_iPatchSize )m_iNumLevelsInLocalPatchQT++;

    if( m_pTriangDataSource )
    {
        if( m_iNumLevelsInPatchHierarchy != m_pTriangDataSource->GetNumLevelsInHierarchy() )
            CHECK_HR_RET(E_FAIL, _T("Number of levels in compressed elevation data (%d) is not equal to the number of levels in triangulation data source(%d)"),  m_iNumLevelsInPatchHierarchy, m_pTriangDataSource->GetNumLevelsInHierarchy() );
        if( m_iPatchSize != m_pTriangDataSource->GetPatchSize() )
            CHECK_HR_RET(E_FAIL, _T("Patch size in compressed elevation data (%d) is not equal to the patch size in triangulation data source(%d)"),  m_iPatchSize, m_pTriangDataSource->GetPatchSize() );
    }

    // Initialize the model in coarsest state
    RestartAdaptiveModel();

    return S_OK;
}

void CBlockBasedAdaptiveModel::SetViewFrustumParams(int iViewPortWidth, int iViewPortHeight, 
                                                    const D3DXMATRIX &CameraProjMatr)
{
    m_fViewportWidth = (float)iViewPortWidth;
    m_fViewportHeight = (float)iViewPortHeight;
    m_CameraProjMatrix = CameraProjMatr;
    float dX = iViewPortWidth*CameraProjMatr._11; // CameraProjMatr._11 == cot(Camera Horz Field of View/2)
    float dY = iViewPortHeight*CameraProjMatr._22;// CameraProjMatr._22 == cot(Camera Vert Field of View/2)
    m_fViewportStretchConst = max(dX, dY)/2.f;
    // The following method significantly overestimates the error:
    //m_fViewportStretchConst = sqrtf(dX*dX + dY*dY)/2.f;
}


void CBlockBasedAdaptiveModel::CalculatePatchBoundingBox(const SQuadTreeNodeLocation &pos, CElevationDataSource *pElev,
                                                         SPatchBoundingBox &PatchBoundingBox)const
{
    // Get XY coordinates scale for the current hierarchy level
    float fXYCoordsScale = (float)(1 << ((m_iNumLevelsInPatchHierarchy-1) - pos.level) );
    // Calculate min/max XY coordinates
    float fMinX = (float)( pos.horzOrder*m_iPatchSize)    * fXYCoordsScale;
    float fMaxX = (float)((pos.horzOrder+1)*m_iPatchSize) * fXYCoordsScale;
    float fMinY = (float)( pos.vertOrder*m_iPatchSize)    * fXYCoordsScale;
    float fMaxY = (float)((pos.vertOrder+1)*m_iPatchSize) * fXYCoordsScale;
    float fHeightFieldSpacing = m_Params.m_fElevationSamplingInterval;
    // Multiply by original height map spacing interval length
    PatchBoundingBox.fMinX = fMinX * fHeightFieldSpacing;
    PatchBoundingBox.fMaxX = fMaxX * fHeightFieldSpacing;
    PatchBoundingBox.fMinY = fMinY * fHeightFieldSpacing;
    PatchBoundingBox.fMaxY = fMaxY * fHeightFieldSpacing;

    // Get patch min/max height
    UINT16 MinZ, MaxZ;
	pElev->GetPatchMinMaxElevation(pos, MinZ, MaxZ);
    PatchBoundingBox.fMinZ = (float)(MinZ * m_Params.m_fElevationScale);
    PatchBoundingBox.fMaxZ = (float)(MaxZ * m_Params.m_fElevationScale);

    // Swizzle XY
    if( m_Params.m_UpAxis == SRenderingParams::UP_AXIS_Y )
    {
        std::swap( PatchBoundingBox.fMinY, PatchBoundingBox.fMinZ );
        std::swap( PatchBoundingBox.fMaxY, PatchBoundingBox.fMaxZ );
    }

	::AcquireSRWLockShared(&m_srwLock);
    PatchBoundingBox.bIsBoxValid = true;
	::ReleaseSRWLockShared(&m_srwLock);
}

static float GetDistanceToBox(const SPatchBoundingBox &BoundBox, 
                              const D3DXVECTOR3 &Pos)
{
    assert(BoundBox.fMaxX >= BoundBox.fMinX && 
           BoundBox.fMaxY >= BoundBox.fMinY && 
           BoundBox.fMaxZ >= BoundBox.fMinZ);
    float fdX = (Pos.x > BoundBox.fMaxX) ? (Pos.x - BoundBox.fMaxX) : ( (Pos.x < BoundBox.fMinX) ? (BoundBox.fMinX - Pos.x) : 0.f );
    float fdY = (Pos.y > BoundBox.fMaxY) ? (Pos.y - BoundBox.fMaxY) : ( (Pos.y < BoundBox.fMinY) ? (BoundBox.fMinY - Pos.y) : 0.f );
    float fdZ = (Pos.z > BoundBox.fMaxZ) ? (Pos.z - BoundBox.fMaxZ) : ( (Pos.z < BoundBox.fMinZ) ? (BoundBox.fMinZ - Pos.z) : 0.f );
    assert(fdX >= 0 && fdY >= 0 && fdZ >= 0);

    D3DXVECTOR3 RangeVec(fdX, fdY, fdZ);
    return D3DXVec3Length( &RangeVec );
}

void CBlockBasedAdaptiveModel::ExtractViewFrustumPlanesFromMatrix(const D3DXMATRIX &Matrix, SViewFrustum &ViewFrustum)
{
    // For more details, see Gribb G., Hartmann K., "Fast Extraction of Viewing Frustum Planes from the 
    // World-View-Projection Matrix" (the paper is available at 
    // http://www2.ravensoft.com/users/ggribb/plane%20extraction.pdf)

	// Left clipping plane 
    ViewFrustum.LeftPlane.Normal.x = Matrix._14 + Matrix._11; 
	ViewFrustum.LeftPlane.Normal.y = Matrix._24 + Matrix._21; 
	ViewFrustum.LeftPlane.Normal.z = Matrix._34 + Matrix._31; 
	ViewFrustum.LeftPlane.Distance = Matrix._44 + Matrix._41;

	// Right clipping plane 
	ViewFrustum.RightPlane.Normal.x = Matrix._14 - Matrix._11; 
	ViewFrustum.RightPlane.Normal.y = Matrix._24 - Matrix._21; 
	ViewFrustum.RightPlane.Normal.z = Matrix._34 - Matrix._31; 
	ViewFrustum.RightPlane.Distance = Matrix._44 - Matrix._41;

	// Top clipping plane 
	ViewFrustum.TopPlane.Normal.x = Matrix._14 - Matrix._12; 
	ViewFrustum.TopPlane.Normal.y = Matrix._24 - Matrix._22; 
	ViewFrustum.TopPlane.Normal.z = Matrix._34 - Matrix._32; 
	ViewFrustum.TopPlane.Distance = Matrix._44 - Matrix._42;

	// Bottom clipping plane 
	ViewFrustum.BottomPlane.Normal.x = Matrix._14 + Matrix._12; 
	ViewFrustum.BottomPlane.Normal.y = Matrix._24 + Matrix._22; 
	ViewFrustum.BottomPlane.Normal.z = Matrix._34 + Matrix._32; 
	ViewFrustum.BottomPlane.Distance = Matrix._44 + Matrix._42;

	// Near clipping plane 
	ViewFrustum.NearPlane.Normal.x = Matrix._13; 
	ViewFrustum.NearPlane.Normal.y = Matrix._23; 
	ViewFrustum.NearPlane.Normal.z = Matrix._33; 
	ViewFrustum.NearPlane.Distance = Matrix._43;

	// Far clipping plane 
	ViewFrustum.FarPlane.Normal.x = Matrix._14 - Matrix._13; 
	ViewFrustum.FarPlane.Normal.y = Matrix._24 - Matrix._23; 
	ViewFrustum.FarPlane.Normal.z = Matrix._34 - Matrix._33; 
	ViewFrustum.FarPlane.Distance = Matrix._44 - Matrix._43; 
}

bool CBlockBasedAdaptiveModel::IsBoxVisible(const SPatchBoundingBox &Box)
{
    SPlane3D *pPlanes = (SPlane3D *)&m_CameraViewFrustum;
    // If bounding box is "behind" some plane, then it is invisible
    // Otherwise it is treated as visible
    for(int iViewFrustumPlane = 0; iViewFrustumPlane < 6; iViewFrustumPlane++)
    {
        SPlane3D *pCurrPlane = pPlanes + iViewFrustumPlane;
        D3DXVECTOR3 *pCurrNormal = &pCurrPlane->Normal;
        D3DXVECTOR3 MaxPoint;
        
        MaxPoint.x = (pCurrNormal->x > 0) ? Box.fMaxX : Box.fMinX;
        MaxPoint.y = (pCurrNormal->y > 0) ? Box.fMaxY : Box.fMinY;
        MaxPoint.z = (pCurrNormal->z > 0) ? Box.fMaxZ : Box.fMinZ;
        
        float DMax = D3DXVec3Dot( &MaxPoint, pCurrNormal ) + pCurrPlane->Distance;

        if( DMax < 0 )
            return false;
    }

    return true;
}

// Adds current patch to the list of optimal patches in current model
inline void CBlockBasedAdaptiveModel::AddPatchToOptimalPatchesList(CPatchQuadTreeNode *pPatchQTNode)
{
    if( pPatchQTNode->GetData().BoundBox.bIsBoxValid )
    {
	    assert( SPatchQuadTreeNodeData::OPTIMAL_PATCH == pPatchQTNode->GetData().Label );
        SOptimalPatchInfo info;
	    info.pPatchQuadTreeNode = pPatchQTNode;
	    info.bIsPatchVisible = false; // Patch visibility is determined when patches are rendered
	    m_OptimalPatchesList.push_back(info);
    }
}

float CBlockBasedAdaptiveModel::CalculatePatchScrSpaceErrorBound(const SPatchBoundingBox &PatchBoundBox,
                                                                 float fGuaranteedPatchErrorBound)
{
	AcquireSRWLockShared(&m_srwLock);
    float fDistanceToCamera = GetDistanceToBox(PatchBoundBox, m_vCameraPos);
	ReleaseSRWLockShared(&m_srwLock);

    
    if( fDistanceToCamera == 0.f || fGuaranteedPatchErrorBound >= +FLT_MAX/2)
        return +FLT_MAX;

    float fPatchScrSpaceError = fGuaranteedPatchErrorBound / fDistanceToCamera * m_fViewportStretchConst;

    return fPatchScrSpaceError;
}

static void DoTask(VOID* pvInfo, INT iContext, UINT uTaskId, UINT uTaskCount)
{
    ITask *pTask = static_cast<ITask *>(pvInfo);
    // Execute the task
    pTask->Execute();
}

// Adds async taks for scheduling
bool CBlockBasedAdaptiveModel::AddTask(ITask *pTask)
{
    if(m_Params.m_bAsyncExecution)
    {
        assert( TASKSETHANDLE_INVALID == static_cast<CTaskBase*>(pTask)->m_TaskHandle );
        // Create the task
        return gTaskMgr.CreateTaskSet(
	            DoTask, //  Function pointer to the taskset callback function
	            pTask,  //  App data pointer (can be NULL)
	            1,      //  Number of tasks to create 
	            NULL,   //  Array of TASKSETHANDLEs that this taskset depends on
	            0,      //  Count of the depends list
	            "Change LOD Task",
                &static_cast<CTaskBase*>(pTask)->m_TaskHandle ) ? 
                true : false;
    }
    else
    {
        // In synchronous mode immediately execute the task
	    pTask->Execute();
        return true;
    }
}

// Recursively traverses the whole hierarchy and builds adaptive terrain model
//
// The method operates as follows:
// 1. If current patch is labeled as TOO_COARSE_PATCH (which means that
//    this patch provides insufficient resolution for the underlying terrain 
//    region (for previous camera position) and contains children), the following 
//    steps are done:
//      1.a If patch node contains DecreaseLOD task (which means that it was previously
//          identifyied that this region can be coarsened), then:
//          * If the task is completed, patch children are destroyed and the patch is 
//            labeled as OPTIMAL_PATCH
//          * If the task is still being executed, level-of-detail is left unchanged
//      1.b If patch node does not contain DecreaseLOD task, then it is checked if
//          this terrain region can be coarsened (which means deleting patch children)
//          In order for the patch to be coarsened, the following conditions must be met:
//          - All 4 patch offsprings are labeled as OPTIMAL_PATCH
//          - Patch screen space error does not exceed the threshold
//          If these conditions are met, a DecreaseLOD task is created for the node,
//          otherwise recursive traversal continues
// 2. If current patch is labeled as OPTIMAL_PATCH (which means that its resolution is
//    optimal (for previous camera position), the following is done:
//      2.a If patch node contains IncreaseLOD task (which means that the patch must be
//          further refined), it is checked if the task is comleted
//          * If the task is completed, patch children are inserted into the tree and
//            labeled as OPTIMAL_PATCH, while the patch itself is labeled with
//            TOO_COARSE_PATCH. After that recursive traversal continues
//          * If the task is not completed, patch children are added to the 
//            optimal patches list
//      2.b If patch node does not contain IncreaseLOD task, it is checked if further 
//          refinement is required:
//          * If the patch screen space error exceeds the threshold, an IncreaseLOD task
//            is created and attached to the node
//          * Otherwise nothing needs to be done
//          In both cases the patch is added to the optinal patches list
void CBlockBasedAdaptiveModel::RecursiveDetermineOptimalPatches(CPatchQuadTreeNode &PatchNode)
{
    int iLevel = PatchNode.GetPos().level;

    SPatchQuadTreeNodeData &data = PatchNode.GetData();

	AcquireSRWLockShared(&m_srwLock);
    if( !data.BoundBox.bIsBoxValid ) {
		ReleaseSRWLockShared(&m_srwLock);
        return;
	}	
    data.m_fDistanceToCamera = GetDistanceToBox(data.BoundBox, m_vCameraPos);
	ReleaseSRWLockShared(&m_srwLock);

	AcquireSRWLockShared(&m_srwLock);
    data.m_fPatchScrSpaceError = CalculatePatchScrSpaceErrorBound(data.BoundBox, data.m_fGuaranteedPatchErrorBound);
	ReleaseSRWLockShared(&m_srwLock);
    if( SPatchQuadTreeNodeData::TOO_COARSE_PATCH == data.Label )
    {
		assert(!data.m_pIncreaseLODTask.get());

        CPatchQuadTreeNode *pDescendantNode[4];
        PatchNode.GetDescendants(pDescendantNode[0], pDescendantNode[1], pDescendantNode[2], pDescendantNode[3]);

        // If there is pending decrease LOD task, we must complete it first before performing any other tasks
        if( data.m_pDecreaseLODTask.get() )
        {
            // If there is pending decrease LOD task, all children must be optimal patches
            for(int iChild=0; iChild<4; iChild++)
                assert( pDescendantNode[iChild]->GetData().Label == SPatchQuadTreeNodeData::OPTIMAL_PATCH );

            // Check the task's completion status
			AcquireSRWLockShared(&m_srwLock);
			bool state = data.m_pDecreaseLODTask->CheckCompletionStatus();
			ReleaseSRWLockShared(&m_srwLock);

		    if( state )
	        {
                // If task is completed, destroy the node's descendants
	            PatchNode.DestroyDescendants();
                
                data.Label = SPatchQuadTreeNodeData::OPTIMAL_PATCH;
                AddPatchToOptimalPatchesList(&PatchNode);
                // Init the patch if it was updated
                if( data.m_bUpdateRequired )
                {
                    // Insert patch into hierarchy
                    CreatePatchForNode(PatchNode, NULL, NULL, data.m_pDecreaseLODTask->GetNewPatch() );
                    data.pPatch->UpdateDeviceResources();
                    data.m_bUpdateRequired = false;
                }

                data.pPatch->BindChildren(NULL, NULL, NULL, NULL);

                // Release the task
                data.m_pDecreaseLODTask.reset();
            }
            else
            {
                // If task is not completed, add the node's children to optimal patch list
                // WE CAN NOT CONTINUE RECURSIVE TRAVERSAL UNTIL TASK IS COMPLETED!
                for(int iChild=0; iChild<4; iChild++)
                    AddPatchToOptimalPatchesList( pDescendantNode[iChild] );
            }
        }
        else
        {
            // If there is executing recompress tasks, we need to wait for them
            // If there is no pending decrease LOD task, check patch screen space error and child patch labels
            if( data.m_fPatchScrSpaceError < m_Params.m_fScrSpaceErrorBound && iLevel > 0  &&
                SPatchQuadTreeNodeData::OPTIMAL_PATCH == pDescendantNode[0]->GetData().Label && 
                SPatchQuadTreeNodeData::OPTIMAL_PATCH == pDescendantNode[1]->GetData().Label && 
                SPatchQuadTreeNodeData::OPTIMAL_PATCH == pDescendantNode[2]->GetData().Label && 
                SPatchQuadTreeNodeData::OPTIMAL_PATCH == pDescendantNode[3]->GetData().Label )
            {
                // We can decrease LOD if the following THREE conditions are met:
                // 1. Patch screen space error < the threshold
                // 2. All children are marked as optimal patches
                // 3. Children are not executing recompress tasks
                // NOTE: IF SOME CHILD IS NOT MARKED AS OPTIMAL_PATCH AND WE PERFORM THE DECREASE
                // LOD TASK, IT COULD CAUSE ERROR because all decrease LOD tasks must be completed 
                // for all descendants first
				AcquireSRWLockShared(&m_srwLock);
                data.m_pDecreaseLODTask.reset( new CDecreaseLODTask(PatchNode, this) );
				ReleaseSRWLockShared(&m_srwLock);
                // Register task in the manager
                if( !AddTask(data.m_pDecreaseLODTask.get()) )
                    // If task failed to create, release it and repeat attempt next time
                    data.m_pDecreaseLODTask.reset();
            
                // Add the node's children to optimal patch list
                for(int iChild=0; iChild<4; iChild++)
                {
                    assert( pDescendantNode[iChild]->GetData().Label == SPatchQuadTreeNodeData::OPTIMAL_PATCH );
                    AddPatchToOptimalPatchesList( pDescendantNode[iChild] );
                }
            }
            else
            {
                // If some child is not optimal patch or screen space threshold is exceeded and there is 
                // no pending decrease LOD task, continue recursive tree traversal
                for(int iChild=0; iChild<4; iChild++)
                    RecursiveDetermineOptimalPatches( *(pDescendantNode[iChild]) );
            }
        }
    }
    else
    {
        // If this block must be refined, there is an optimal Increase LOD task. In this case we have 
        // to check if it is completed and insert new nodes into the tree.
        // If there is no need to refine the block, m_pIncreaseLODTask is null
        if( data.m_pIncreaseLODTask.get() )
	    {
            // Check if the task is completed
			AcquireSRWLockShared(&m_srwLock);
			bool state = data.m_pIncreaseLODTask->CheckCompletionStatus();
			ReleaseSRWLockShared(&m_srwLock);
		    if(state)
		    {
			    std::auto_ptr<CPatchQuadTreeNode> descendantNodes[4];
			    data.m_pIncreaseLODTask->DetachFloatingDescendants(descendantNodes[0], descendantNodes[1], descendantNodes[2], descendantNodes[3]);
			    data.m_pIncreaseLODTask.reset();

                // It is important to bind children before calling UpdateDeviceResources()!
                if( data.pPatch.get() )
                    data.pPatch->BindChildren(descendantNodes[0]->GetData().pPatch.get(), 
                                              descendantNodes[1]->GetData().pPatch.get(),
                                              descendantNodes[2]->GetData().pPatch.get(), 
                                              descendantNodes[3]->GetData().pPatch.get());

			    for( int iChild = 0; iChild < 4; iChild++ )
                {
				    descendantNodes[iChild]->GetData().pPatch->UpdateDeviceResources();
                }

			    PatchNode.CreateDescendants(descendantNodes[0], descendantNodes[1], descendantNodes[2], descendantNodes[3]);
			    data.Label = SPatchQuadTreeNodeData::TOO_COARSE_PATCH;

                CPatchQuadTreeNode *pDescendantNode[4];
                PatchNode.GetDescendants(pDescendantNode[0], pDescendantNode[1], pDescendantNode[2], pDescendantNode[3]);

			    for(int iChild = 0; iChild < 4; iChild++)
			    {
				    pDescendantNode[iChild]->GetData().Label = SPatchQuadTreeNodeData::OPTIMAL_PATCH;
				    RecursiveDetermineOptimalPatches(*pDescendantNode[iChild]);
			    }
		    }
		    else
		    {
                // The task has not yet been completed. Add current node to the 
                // optimal patches list
			    AddPatchToOptimalPatchesList( &PatchNode );
		    }
	    }
	    else
	    {
		    if( iLevel < m_iNumLevelsInPatchHierarchy - 1 &&
			    ( iLevel == 0 || data.m_fPatchScrSpaceError > m_Params.m_fScrSpaceErrorBound ) )
		    {
			    std::auto_ptr<CPatchQuadTreeNode> pDescendants[4];
			    PatchNode.CreateFloatingDescendants(pDescendants[0], pDescendants[1], pDescendants[2], pDescendants[3]);
				
				AcquireSRWLockShared(&m_srwLock);
                data.m_pIncreaseLODTask.reset( new CIncreaseLODTask(pDescendants, m_pDataSource, m_pTriangDataSource, this) );
				ReleaseSRWLockShared(&m_srwLock);
			    if( !AddTask(data.m_pIncreaseLODTask.get()) )
                    // If task failed to create, release it and repeat attempt next time
                    data.m_pIncreaseLODTask.reset();
		    }
		    AddPatchToOptimalPatchesList( &PatchNode );
	    }
    }
}

// Updates the model with respect to new camera position
void CBlockBasedAdaptiveModel::UpdateModel(const D3DXVECTOR3 &vCameraPosition,
                                           const D3DXMATRIX &CameraViewMatrix)
{
    m_vCameraPos = vCameraPosition;
    m_CameraViewMatrix = CameraViewMatrix;
    D3DXMatrixMultiply(&m_CameraViewProjMatrix, &m_CameraViewMatrix, &m_CameraProjMatrix); 
    ExtractViewFrustumPlanesFromMatrix(m_CameraViewProjMatrix, m_CameraViewFrustum);
    // Clear optimal patches list
    m_OptimalPatchesList.clear();
    RecursiveDetermineOptimalPatches( m_PatchQuadTreeRoot );
}


void CBlockBasedAdaptiveModel::SetScreenSpaceErrorBound(float fScreenSpaceErrorBound)
{
    m_Params.m_fScrSpaceErrorBound = fScreenSpaceErrorBound;
}

void CBlockBasedAdaptiveModel::GetLastFrameComplexity( int &iOptimalPatchesCount, int &iVisiblePatchesCount, int &iTotalTrianglesRendered )
{
    iOptimalPatchesCount = (int) m_OptimalPatchesList.size();
    iVisiblePatchesCount = (int) m_PatchRenderingInfo.size();
    iTotalTrianglesRendered = m_iTotalTrianglesRendered;
}


// Builds adaptive triangulations for the whole hierarchy
void CBlockBasedAdaptiveModel::ConstructPatchAdaptiveTriangulations()
{
    m_AdaptiveTriangulationStat.resize( m_iNumLevelsInPatchHierarchy );

    float fFinestLevelTriangErrorThreshold = m_pTriangDataSource->GetFinestLevelTriangErrorThreshold();

    std::auto_ptr<CRQTTriangulation> pDummyTriang;
    RecursiveBuildPatchTriangulations(SQuadTreeNodeLocation(), fFinestLevelTriangErrorThreshold * (float)(1 << (m_iNumLevelsInPatchHierarchy-1)), NULL, pDummyTriang);

    // Output statistics
    FILE *pStatFile;
    if( _tfopen_s(&pStatFile, _T("TriangStat.txt"), _T("wt")) == 0)
    {
        size_t TotalCompressedDataSize = 0;
        LONGLONG llTotalTriangles = 0;
        LONGLONG llTotalTrianglesInFullResInAllLevels = 0;
        for(int iLevel = 1; iLevel < m_iNumLevelsInPatchHierarchy; iLevel++)
        {
            int iLevelDim = 1<<iLevel;
            int iNumTrisInFullRes = (m_iPatchSize+3 - 1) * (m_iPatchSize+3 - 1) * 2;
            LONGLONG llNumTrisInFullResLevel = (LONGLONG)iNumTrisInFullRes * iLevelDim*iLevelDim;
            llTotalTrianglesInFullResInAllLevels += llNumTrisInFullResLevel;
            float fTriangleFraction = (float)m_AdaptiveTriangulationStat[iLevel].m_llTotalTriangles / (float)llNumTrisInFullResLevel;
            float fBitsPerTri = (float)m_AdaptiveTriangulationStat[iLevel].TotalCompressedDataSize / (float)m_AdaptiveTriangulationStat[iLevel].m_llTotalTriangles * 8.f;
            _ftprintf_s(pStatFile, _T("Level %d: %.1lf%% total # triangles; bits/tri: %.3lf\n"), iLevel, fTriangleFraction * 100.f, fBitsPerTri );
            
            TotalCompressedDataSize += m_AdaptiveTriangulationStat[iLevel].TotalCompressedDataSize;
            llTotalTriangles += m_AdaptiveTriangulationStat[iLevel].m_llTotalTriangles;
        }
        float fAverageTriangleFraction = (float)llTotalTriangles / (float)llTotalTrianglesInFullResInAllLevels;
        float fAverageBitsPerTri = (float)TotalCompressedDataSize / (float)llTotalTriangles * 8.f;
        _ftprintf_s(pStatFile, _T("\nAverage: %.1lf%% total # triangles; bits/tri: %.3lf\n"),  fAverageTriangleFraction * 100.f, fAverageBitsPerTri );
        
        _ftprintf_s(pStatFile, _T("Total compressed tri size: %ld bytes\n"),  TotalCompressedDataSize );
        LONGLONG TotalSamples = (1 << 2*(m_iNumLevelsInPatchHierarchy+m_iNumLevelsInLocalPatchQT-2) );
        float fCompressedTriBPS = (float)TotalCompressedDataSize / (float)TotalSamples * 8.f;
        _ftprintf_s(pStatFile, _T("Compressed tri bps: %.3f\n"),  fCompressedTriBPS);
        

        fclose(pStatFile);
    }

    m_AdaptiveTriangulationStat.clear();
}

// Recursively traverses the whole hierarchy and builds adaptive triangulation for each node
void CBlockBasedAdaptiveModel::RecursiveBuildPatchTriangulations(const SQuadTreeNodeLocation &pos,
                                                                 float fTriangulationErrorThreshold,
                                                                 CPatchElevationData *pElevData, 
                                                                 std::auto_ptr<class CRQTTriangulation> &pAdaptiveTriangulation)
{
    float fElevDataErrorBound = m_pDataSource->GetPatchElevDataErrorBound(pos) * m_Params.m_fElevationScale;
    float fTriangulationError = 0.f;
    UINT uiNumTriangles = 0;

    // If there are finer levels, process them first
    std::auto_ptr<CRQTTriangulation> pChildTriangulation[4];
    if( pos.level < m_iNumLevelsInPatchHierarchy-1 )
    {
        for(int iChild = 0; iChild < 4; iChild++)
        {
            SQuadTreeNodeLocation ChildPos = GetChildLocation(pos, iChild);
            std::auto_ptr<CPatchElevationData> pChildElevData( m_pDataSource->GetElevData( ChildPos ) );
            RecursiveBuildPatchTriangulations(ChildPos, fTriangulationErrorThreshold/2.f, pChildElevData.get(), pChildTriangulation[iChild]);
        }
    }

    // Build triangulation for current patch
    if( pos.level > 0 )
    {
        pAdaptiveTriangulation.reset( 
            m_pTriangDataSource->CreateAdaptiveTriangulation(pElevData,
                                                      pChildTriangulation[0].get(), 
                                                      pChildTriangulation[1].get(), 
                                                      pChildTriangulation[2].get(), 
                                                      pChildTriangulation[3].get(),
                                                      max(fTriangulationErrorThreshold, fElevDataErrorBound/4.f) / m_Params.m_fElevationScale,
                                                      fTriangulationError, uiNumTriangles) );
    }

    // Encode the triangulation
    if( pAdaptiveTriangulation.get() )
        m_pTriangDataSource->EncodeTriangulation( pos, *pAdaptiveTriangulation, fTriangulationError );

    // Update statistics
    if( pos.level < m_iNumLevelsInPatchHierarchy-1 )
        m_AdaptiveTriangulationStat[pos.level+1].TotalCompressedDataSize += m_pTriangDataSource->GetEncodedTriangulationsSize(pos);

    m_AdaptiveTriangulationStat[pos.level].m_llTotalTriangles += uiNumTriangles;
}

// Recursively traverses the tree and waits while each async taks (if any) is completed
void CBlockBasedAdaptiveModel::RecursiveWaitForAsyncTaks(CPatchQuadTreeNode &PatchNode)
{
    if( PatchNode.GetData().m_pDecreaseLODTask.get() )
        PatchNode.GetData().m_pDecreaseLODTask->WaitForTaskCompletion();
    if( PatchNode.GetData().m_pIncreaseLODTask.get() )
        PatchNode.GetData().m_pIncreaseLODTask->WaitForTaskCompletion();
    // Process children
    CPatchQuadTreeNode *pDescendantNode[4];
    PatchNode.GetDescendants(pDescendantNode[0], pDescendantNode[1], pDescendantNode[2], pDescendantNode[3]);
    for(int iChild=0; iChild<4; iChild++)
    {
        if(pDescendantNode[iChild])
            RecursiveWaitForAsyncTaks( *pDescendantNode[iChild] );
    }
}

// Waits while all async tasks are completed
void CBlockBasedAdaptiveModel::WaitForAsyncTasks()
{
    RecursiveWaitForAsyncTaks(m_PatchQuadTreeRoot);
}

// Initializes the model in corsest representation
void CBlockBasedAdaptiveModel::RestartAdaptiveModel()
{
    WaitForAsyncTasks();
    m_PatchQuadTreeRoot.GetData().Label = SPatchQuadTreeNodeData :: OPTIMAL_PATCH;
    m_PatchQuadTreeRoot.GetData().m_fGuaranteedPatchErrorBound = FLT_MAX/2;
    CalculatePatchBoundingBox(m_PatchQuadTreeRoot.GetPos(), m_pDataSource, m_PatchQuadTreeRoot.GetData().BoundBox);
    m_PatchQuadTreeRoot.DestroyDescendants();
}

void CBlockBasedAdaptiveModel::GetTerrainBoundingBox(SPatchBoundingBox &BoundBox)
{
    CalculatePatchBoundingBox(m_PatchQuadTreeRoot.GetPos(), m_pDataSource, BoundBox);
}

template <typename T>
static bool TestIntersectFragments(T MinX1, T MaxX1, T MinX2, T MaxX2)
{
    if( MinX1 > MaxX1)std::swap(MinX1, MaxX1);
    if( MinX2 > MaxX2)std::swap(MinX2, MaxX2);
    
    // There can be only two cases when fragments do not intersect:
    //
    //      MinX1         MaxX1      MinX2      MaxX2
    //   -----[-------------]-----------[---------]----------
    //
    //      MinX2         MaxX2      MinX1      MaxX1
    //   -----[-------------]-----------[---------]----------
    //
    // Otherwise they do intersect
    if( MaxX1 < MinX2 || MaxX2 < MinX1)
        return false;
    else
        return true;
}

// Creates a patch for the specified quad tree node
std::auto_ptr<CTerrainPatch> CBlockBasedAdaptiveModel::CreatePatchForNode(CPatchQuadTreeNode &PatchNode,
                                                                          class CPatchElevationData *pPatchElevData,
                                                                          class CRQTTriangulation *pAdaptiveTriangulation,
                                                                          std::auto_ptr<CTerrainPatch> pCreatedPatch)
{
    std::auto_ptr<CTerrainPatch> &pPatch = PatchNode.GetData().pPatch;
    std::auto_ptr<CTerrainPatch> pOldPatch = pPatch;
    if( pCreatedPatch.get() )
        pPatch = pCreatedPatch;
    else
        pPatch = CreatePatch(pPatchElevData, pAdaptiveTriangulation);

    CPatchQuadTreeNode *pDescendantNode[4];
    PatchNode.GetDescendants(pDescendantNode[0], pDescendantNode[1], pDescendantNode[2], pDescendantNode[3]);
    CTerrainPatch *pChildren[4] = {NULL};
    for(int iChild = 0; iChild < 4; iChild++)
        if(pDescendantNode[iChild])
            pChildren[iChild] = pDescendantNode[iChild]->GetData().pPatch.get();
    pPatch->BindChildren(pChildren[0], pChildren[1], pChildren[2], pChildren[3]);

    CPatchQuadTreeNode *pParent = PatchNode.GetAncestor();
    if( pParent && pParent->GetData().pPatch.get() )
    {
        CPatchQuadTreeNode *pSiblings[4];
        pParent->GetDescendants(pSiblings[0], pSiblings[1], pSiblings[2], pSiblings[3]);
        CTerrainPatch *pSiblingPatches[4] = {NULL};
        for(int iSibling = 0; iSibling < 4; iSibling++)
            pSiblingPatches[iSibling] = pSiblings[iSibling]->GetData().pPatch.get();
        pParent->GetData().pPatch->BindChildren(pSiblingPatches[0], pSiblingPatches[1], pSiblingPatches[2], pSiblingPatches[3]);
    }

    return pOldPatch;
}



//-----------------------------------------------------------------------------
// Returns TRUE if any of the elements of a 3 vector are equal to 0xffffffff.
// Slightly more efficient than using XMnoVec3EqualInt.
//-----------------------------------------------------------------------------
static inline BOOL XMnoVec3AnyTrue( FXMVECTOR V )
{
    XMVECTOR C;

    // Duplicate the fourth element from the first element.
    C = XMVectorSwizzle( V, 0, 1, 2, 0 );

    return XMComparisonAnyTrue( XMVector4EqualIntR( C, XMVectorTrueInt() ) );
}


//-----------------------------------------------------------------------------
// Compute the intersection of a ray (Origin, Direction) with an axis aligned 
// box using the slabs method.
//-----------------------------------------------------------------------------
static BOOL IntersectRayAxisAlignedBox( FXMVECTOR Origin, FXMVECTOR Direction, const SPatchBoundingBox &volume, FLOAT* pEnter, FLOAT* pExit )
{
    XMASSERT( pEnter );
    XMASSERT( pExit );
//    XMASSERT( XMnoVec3IsUnit( Direction ) );

    static const XMVECTOR Epsilon =
    {
        1e-20f, 1e-20f, 1e-20f, 1e-20f
    };
    static const XMVECTOR FltMin =
    {
        -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX
    };
    static const XMVECTOR FltMax =
    {
        FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX
    };

    // Load the box.
    XMFLOAT3 TmpCenter((volume.fMaxX+volume.fMinX)/2, (volume.fMaxY+volume.fMinY)/2, (volume.fMaxZ+volume.fMinZ)/2);
    XMFLOAT3 TmpExtents((volume.fMaxX-volume.fMinX)/2, (volume.fMaxY-volume.fMinY)/2, (volume.fMaxZ-volume.fMinZ)/2);
    XMVECTOR Center = XMLoadFloat3( &TmpCenter );
    XMVECTOR Extents = XMLoadFloat3( &TmpExtents );

    // Adjust ray origin to be relative to center of the box.
    XMVECTOR TOrigin = Center - Origin;

    // Compute the dot product against each axis of the box.
    // Since the axii are (1,0,0), (0,1,0), (0,0,1) no computation is necessary.
    XMVECTOR AxisDotOrigin = TOrigin;
    XMVECTOR AxisDotDirection = Direction;

    // if (fabs(AxisDotDirection) <= Epsilon) the ray is nearly parallel to the slab.
    XMVECTOR IsParallel = XMVectorLessOrEqual( XMVectorAbs( AxisDotDirection ), Epsilon );

    // Test against all three axii simultaneously.
    XMVECTOR InverseAxisDotDirection = XMVectorReciprocal( AxisDotDirection );
    XMVECTOR t1 = ( AxisDotOrigin - Extents ) * InverseAxisDotDirection;
    XMVECTOR t2 = ( AxisDotOrigin + Extents ) * InverseAxisDotDirection;

    // Compute the max of min(t1,t2) and the min of max(t1,t2) ensuring we don't
    // use the results from any directions parallel to the slab.
    XMVECTOR t_min = XMVectorSelect( XMVectorMin( t1, t2 ), FltMin, IsParallel );
    XMVECTOR t_max = XMVectorSelect( XMVectorMax( t1, t2 ), FltMax, IsParallel );

    // t_min.x = maximum( t_min.x, t_min.y, t_min.z );
    // t_max.x = minimum( t_max.x, t_max.y, t_max.z );
    t_min = XMVectorMax( t_min, XMVectorSplatY( t_min ) );  // x = max(x,y)
    t_min = XMVectorMax( t_min, XMVectorSplatZ( t_min ) );  // x = max(max(x,y),z)
    t_max = XMVectorMin( t_max, XMVectorSplatY( t_max ) );  // x = min(x,y)
    t_max = XMVectorMin( t_max, XMVectorSplatZ( t_max ) );  // x = min(min(x,y),z)

    // if ( t_min > t_max ) return FALSE;
    XMVECTOR NoIntersection = XMVectorGreater( XMVectorSplatX( t_min ), XMVectorSplatX( t_max ) );

    // if ( t_max < 0.0f ) return FALSE;
    NoIntersection = XMVectorOrInt( NoIntersection, XMVectorLess( XMVectorSplatX( t_max ), XMVectorZero() ) );

    // if (IsParallel && (-Extents > AxisDotOrigin || Extents < AxisDotOrigin)) return FALSE;
    XMVECTOR ParallelOverlap = XMVectorInBounds( AxisDotOrigin, Extents );
    NoIntersection = XMVectorOrInt( NoIntersection, XMVectorAndCInt( IsParallel, ParallelOverlap ) );

    if( !XMnoVec3AnyTrue( NoIntersection ) )
    {
        // Store the x-components to *pEnter and *pExit
        XMStoreFloat( pEnter, t_min );
        XMStoreFloat( pExit, t_max );
        return TRUE;
    }

    return FALSE;
}


void Intersect(const D3DXVECTOR3 vert[3], const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, float &t)
{
	static const float EPSILON = 1e-10f;

	// find vectors for two edges sharing vert0
	D3DXVECTOR3 edge1 = vert[1] - vert[0];
	D3DXVECTOR3 edge2 = vert[2] - vert[0];

	// begin calculating determinant - also used to calculate U parameter
	D3DXVECTOR3 pvec;
	D3DXVec3Cross(&pvec, &dir, &edge2);

	// if determinant is near zero, ray lies in plane of triangle
	float det = D3DXVec3Dot(&edge1, &pvec);

    // no culling is performed
    // to implement backface culling, leave only one-tailed condition:
    // if( det >= EPSILON )
	if( det <= -EPSILON || det >= EPSILON )
	{
		// calculate distance from vert0 to ray origin
		D3DXVECTOR3 tvec = orig - vert[0];

		// calculate U parameter and test bounds
		float u = D3DXVec3Dot(&tvec, &pvec) / det;
		if( u >= 0.0 && u <= 1 )
		{
			// prepare to test V parameter
			D3DXVECTOR3 qvec;
			D3DXVec3Cross(&qvec, &tvec, &edge1);

			// calculate V parameter and test bounds
			float v = D3DXVec3Dot(&dir, &qvec) / det;
			if( v >= 0.0 && u + v <= 1 )
			{
				// calculate t, ray intersects triangle
				t = D3DXVec3Dot(&edge2, &qvec) / det;
			}
		}
	}
}

// Intersects a ray casted from "origin" in "direction" and returns true if ray hits the terrain
bool CBlockBasedAdaptiveModel::RayCast(const D3DXVECTOR3 &origin, const D3DXVECTOR3 &direction, float *outDistance) const
{
	XMVECTORF32 xmOrigin = { origin.x, origin.y, origin.z, 1 };
	XMVECTORF32 xmDirection = { direction.x, direction.y, direction.z, 1 };

	typedef std::pair<float, float> EnterExitPair;
	typedef std::pair<const CPatchQuadTreeNode*, EnterExitPair> StackFrame;
	std::vector<StackFrame> traversalStack;

	StackFrame current(&m_PatchQuadTreeRoot, EnterExitPair(FLT_MAX, FLT_MAX));
	if( !IntersectRayAxisAlignedBox(xmOrigin, xmDirection, current.first->GetData().BoundBox, &current.second.first, &current.second.second) )
		return false;

	D3DXVECTOR3 tmpOrigin = origin;
	D3DXVECTOR3 tmpDir = direction;
	if( SRenderingParams::UP_AXIS_Y == m_Params.m_UpAxis )
	{
		std::swap(tmpOrigin.y, tmpOrigin.z);
		std::swap(tmpDir.y, tmpDir.z);
	}

	for(;;)
	{
		if( SPatchQuadTreeNodeData::TOO_COARSE_PATCH == current.first->GetData().Label )
		{
			// traverse through the quad tree
			StackFrame children[4] = {
				StackFrame(StackFrame::first_type(), EnterExitPair(FLT_MAX, FLT_MAX)),
				StackFrame(StackFrame::first_type(), EnterExitPair(FLT_MAX, FLT_MAX)),
				StackFrame(StackFrame::first_type(), EnterExitPair(FLT_MAX, FLT_MAX)),
				StackFrame(StackFrame::first_type(), EnterExitPair(FLT_MAX, FLT_MAX)),
			};
			
			current.first->GetDescendants(children[0].first, children[1].first, children[2].first, children[3].first);
			
			for( int i = 0; i < 4; ++i )
			{
				assert(children[i].first);
				IntersectRayAxisAlignedBox(xmOrigin, xmDirection, children[i].first->GetData().BoundBox,
					&children[i].second.first, &children[i].second.second);
				// sort by enter distance
				for( int j = i; j > 0; --j )
					if( children[j].second.first < children[j-1].second.first )
						std::swap(children[j], children[j-1]);
			}
			
			// push children sorted by distance - nearest goes last
			for( int i = 4; i--; )
				if( children[i].second.first < FLT_MAX )
					traversalStack.push_back(children[i]);
		}
		else
		{
			// intersect with height map
			const UINT16 *elev = NULL;
			size_t pitch = 0;
			if( current.first->GetData().m_pElevData.get() )
				current.first->GetData().m_pElevData->GetDataPtr(elev, pitch, 0, 0, 1, 1);
			if( elev )
			{
				assert(pitch);

				SPatchBoundingBox localBB = current.first->GetData().BoundBox;
				if( SRenderingParams::UP_AXIS_Y == m_Params.m_UpAxis )
				{
					std::swap(localBB.fMinY, localBB.fMinZ);
					std::swap(localBB.fMaxY, localBB.fMaxZ);
				}

				D3DXVECTOR3 localEnter = tmpOrigin + tmpDir * current.second.first;
				localEnter.x = localEnter.x - localBB.fMinX;
				localEnter.y = localEnter.y - localBB.fMinY;

				D3DXVECTOR3 localExit = tmpOrigin + tmpDir * current.second.second;
				localExit.x = localExit.x - localBB.fMinX;
				localExit.y = localExit.y - localBB.fMinY;

				float scaleX = (float) (m_iPatchSize - 1) / (localBB.fMaxX - localBB.fMinX);
				float scaleY = (float) (m_iPatchSize - 1) / (localBB.fMaxY - localBB.fMinY);

				int endH = int(localExit.x * scaleX);
				int endV = int(localExit.y * scaleY);
				int localH = int(localEnter.x * scaleX);
				int localV = int(localEnter.y * scaleY);
				int dh = tmpDir.x > 0 ? 1 : -1;
				int dv = tmpDir.y > 0 ? 1 : -1;

				const float p = tmpDir.y * localEnter.x - tmpDir.x * localEnter.y;
				const float tx = p - tmpDir.y * (float) dh / scaleX;
				const float ty = p + tmpDir.x * (float) dv / scaleY;

				for(;;)
				{
					// intersect with two triangles; choose the nearest
					float dist0 = FLT_MAX, dist1 = FLT_MAX;
					D3DXVECTOR3 vert[3];
					vert[0].x = localBB.fMinX + (float) (localH + 1) / scaleX;
					vert[0].y = localBB.fMinY + (float) localV / scaleY;
                    vert[0].z = elev[localH + localV * pitch + 1] * m_Params.m_fElevationScale; // z10
					vert[1].x = localBB.fMinX + (float) localH / scaleX;
					vert[1].y = localBB.fMinY + (float) (localV + 1) / scaleY;
					vert[1].z = elev[localH + localV * pitch + pitch] * m_Params.m_fElevationScale; // z01
					
					vert[2].x = localBB.fMinX + (float) localH / scaleX;
					vert[2].y = localBB.fMinY + (float) localV / scaleY;
					vert[2].z = elev[localH + localV * pitch] * m_Params.m_fElevationScale; // z00
					Intersect(vert, tmpOrigin, tmpDir, dist0);
					
					vert[2].x = localBB.fMinX + (float) (localH + 1) / scaleX;
					vert[2].y = localBB.fMinY + (float) (localV + 1) / scaleY;
					vert[2].z = elev[localH + localV * pitch + pitch + 1] * m_Params.m_fElevationScale; // z11
					Intersect(vert, tmpOrigin, tmpDir, dist1);

					if( dist0 >= 0 && dist0 < FLT_MAX || dist1 >= 0 && dist1 < FLT_MAX )
					{
						if( outDistance )
						{
							if( dist0 > dist1 ) // sort
								std::swap(dist0, dist1);
							*outDistance = dist0 >= 0 ? dist0 : dist1;
						}
						return true;
					}

					if( localH == endH && localV == endV )
					{
						// no intersection found
						break;
					}
					else
					{
						// step to the next cell
						float t = tmpDir.x * ((float) localV + 0.5f) / scaleY - tmpDir.y * ((float) localH + 0.5f) / scaleX;
						if( fabs(t + tx) < fabs(t + ty) )
							localH += dh;
						else
							localV += dv;
					}
				}
			}
			else
			{
				// workaround: register intersection with current bounding box since there is no height map
				if( outDistance )
					*outDistance = current.second.first;
				return true;
			}
		}

		// pop next node or exit if stack is empty
		if( traversalStack.empty() )
		{
			break;
		}
		else
		{
			current = traversalStack.back();
			traversalStack.pop_back();
		}
	}

	return false;
}

// end of file
