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
#pragma once

#include "GameApp/Mesh.h"

#include "DynamicQuadTreeNode.h"
#include "TriangDataSource.h"
#include "ElevationDataSource.h"
#include "RQTTriangulation.h"
#include "TerrainPatch.h"
#include "EngineCore/TBB/TaskMgrTbb.h"


#include <deque>
#include <vector>

class CQuadTreeNodeLabelsMap;
class CIncreaseLODTask;
class CDecreaseLODTask;

struct SPatchBoundingBox
{
    float fMinX, fMaxX, fMinY, fMaxY, fMinZ, fMaxZ;
    bool bIsBoxValid;
};

// Structure storing information about single patch to be rendered
struct SPatchRenderingInfo
{
    CTerrainPatch *pPatch;
    float fFlangeWidth;
    float fDistanceToCamera;
    float fMorphCoeff;
};

// Patch quad tree node
struct SPatchQuadTreeNodeData
{
    std::auto_ptr<CTerrainPatch> pPatch;
    std::auto_ptr<CPatchElevationData> m_pElevData;
    std::auto_ptr<CRQTTriangulation> m_pAdaptiveTriangulation;
    std::auto_ptr<CIncreaseLODTask> m_pIncreaseLODTask;
    std::auto_ptr<CDecreaseLODTask> m_pDecreaseLODTask;

    float m_fGuaranteedPatchErrorBound;// == ElevDataErrorBound + TriangulationErrorBound
    float m_fDistanceToCamera;
    float m_fPatchScrSpaceError;
    bool m_bUpdateRequired; // For future use

    SPatchBoundingBox BoundBox;

    enum PATCH_QUAD_TREE_NODE_LABEL
    {
        TOO_COARSE_PATCH = 0, // Patch level of detail is too coarse to represent underlying terrain
                              // region with the requried precision (for current camera position)
                              // Should be refined
        OPTIMAL_PATCH = 1,    // Level of detail is optimal for current view parameters and
                              // should not be changed
        TOO_DETAILED_PATCH = 2 // Pacth resolution level is too high and should be coarsened
    }Label;

    SPatchQuadTreeNodeData()
		: Label(TOO_COARSE_PATCH)
		, m_bUpdateRequired(false)
	{}
};

typedef CDynamicQuadTreeNode<SPatchQuadTreeNodeData> CPatchQuadTreeNode;



__interface ITask
{
	void STDMETHODCALLTYPE Execute();
};

// Base class for async tasks
class CTaskBase : public ITask
{
public:
    // Returns true if task is completed or false otherwise
    bool CheckCompletionStatus();

    // Waits until the task is completed
    void WaitForTaskCompletion();
    
    TASKSETHANDLE m_TaskHandle;
protected:
    CTaskBase();
	virtual ~CTaskBase();

    volatile bool m_bTaskComplete;
};

// Async task performing model refinement
class CIncreaseLODTask : public CTaskBase
{
public:
	CIncreaseLODTask(std::auto_ptr<CPatchQuadTreeNode> pDescendants[4],
	                 CElevationDataSource *pDataSource,
	                 CTriangDataSource *pTriangDataSource,
                     const class CBlockBasedAdaptiveModel *pBlockBasedModel);

    // Detaches descendants from internal auto pointers, so they can be
    // inserted into the quad tree
	void DetachFloatingDescendants(std::auto_ptr<CPatchQuadTreeNode> &pLBDescendant,
	                               std::auto_ptr<CPatchQuadTreeNode> &pRBDescendant,
	                               std::auto_ptr<CPatchQuadTreeNode> &pLTDescendant,
	                               std::auto_ptr<CPatchQuadTreeNode> &pRTDescendant);

	// Executes the task
	void STDMETHODCALLTYPE Execute();

    ~CIncreaseLODTask();
protected:
	CIncreaseLODTask(); // never implemented
	CIncreaseLODTask(const CIncreaseLODTask &); // no copy
	CIncreaseLODTask& operator = (CIncreaseLODTask &); // no assignment

private:
	std::auto_ptr<CPatchQuadTreeNode> m_pFloatingDescendantNodes[4]; // Internal auto pointers to created descendants
	CElevationDataSource *m_pDataSource; // Pointer to elevation data source
    CTriangDataSource *m_pTriangDataSource; // Pointer to triangulation data source
    const class CBlockBasedAdaptiveModel *m_pBlockBasedModel;

};

// Async task performing model coarsening
class CDecreaseLODTask : public CTaskBase
{
public:
    CDecreaseLODTask( CPatchQuadTreeNode &Node,
                      const class CBlockBasedAdaptiveModel *pBlockBasedModel );
	
    // ITask
	void STDMETHODCALLTYPE Execute();

    std::auto_ptr<CTerrainPatch> GetNewPatch(){return m_pNewPatch;}

    ~CDecreaseLODTask();
protected:
	CDecreaseLODTask(); // to make it work with CComPtr; never implemented
	CDecreaseLODTask(const CDecreaseLODTask &); // no copy
	CDecreaseLODTask& operator = (CDecreaseLODTask &); // no assignment

private:
    CPatchQuadTreeNode *m_pNode;
    CElevationDataSource *m_pDataSource; // Pointer to elevation data source
    CTriangDataSource *m_pTriangDataSource; // Pointer to triangulation data source
    const class CBlockBasedAdaptiveModel *m_pBlockBasedModel;
    std::auto_ptr<CTerrainPatch> m_pNewPatch;
};

// Structure describing a plane
struct SPlane3D
{
    D3DXVECTOR3 Normal;
    float Distance;     //Distance from the coordinate system origin to the plane along normal direction
};

#pragma pack(1)
struct SViewFrustum
{
    SPlane3D LeftPlane, RightPlane, BottomPlane, TopPlane, NearPlane, FarPlane;
};
#pragma pack()


// Structure describing terrain rendering parameters
struct SRenderingParams
{
    float m_fElevationSamplingInterval; // Distance between to height map samples
    float m_fElevationScale;    // Scale of height map samples
    float m_fScrSpaceErrorBound; // Screen space error tolerance for rendering model
    enum UP_AXIS
    {
        UP_AXIS_Y = 0,
        UP_AXIS_Z
    }m_UpAxis;
    int m_iPatchSize;   // Size of the terrain patch (is always 2^n)
    float m_fGlobalMinElevation; // Minimal height of the whole terrain
    float m_fGlobalMaxElevation; // Maximal height of the whole terrain
    int m_iNumLevelsInPatchHierarchy; // Total number of levels in patch quad tree hierarchy
    bool m_bAsyncExecution; // Flag indicating if model should be upadted synchronously
};

// This class constructs adaptive view-dependent terrain model
class CBlockBasedAdaptiveModel : public Mesh
{
    friend class CIncreaseLODTask;
    friend class CDecreaseLODTask;
public:
    CBlockBasedAdaptiveModel(void);
    ~CBlockBasedAdaptiveModel(void);

	virtual bool CreateBuffers(void) { return true; }
	virtual void Draw(void) {}

    // Initializes the object
    HRESULT Init(const SRenderingParams &Params,
                 CElevationDataSource *pDataSource,
                 CTriangDataSource *pTriangDataSource);

    void SetViewFrustumParams(int iViewPortWidth, int iViewPortHeight, 
                              const D3DXMATRIX &CameraProjMatr);

    // Updates the model with respect to new camera position
    void UpdateModel(const D3DXVECTOR3 &vCameraPosition,
                     const D3DXMATRIX &CameraViewMatrix);

    void SetScreenSpaceErrorBound(float fScreenSpaceErrorBound);

    // Returns complexity indicators for the last rendered frame
    void GetLastFrameComplexity( int &iOptimalPatchesCount, int &iVisiblePatchesCount, int &iTotalTrianglesRendered );

    // Builds adaptive triangulations for the whole hierarchy
    void ConstructPatchAdaptiveTriangulations();

    // Sets location of the mini map
	void SetQuadTreePreviewPos(float fX, float fY, float fWidth, float fHeight){m_vQuadTreePreviewScrPos=D3DXVECTOR4(fX, fY, fWidth, fHeight);}

    // Returns bounding box for the whole terrain
    void GetTerrainBoundingBox(SPatchBoundingBox &BoundBox);
    
    // Initializes the model in corsest representation
    void RestartAdaptiveModel();

    // Waits while all async tasks are completed
    void WaitForAsyncTasks();

    // Intersects a ray casted from "origin" in "direction" and returns true if ray hits the terrain
    bool RayCast(const D3DXVECTOR3 &origin, const D3DXVECTOR3 &direction, float *outDistance) const;

    // Enables or disables asynchronous task execution
    void EnableAsyncExecution(bool bAsyncExecution){m_Params.m_bAsyncExecution = bAsyncExecution;}

protected:
    // Creates a new terrain patch
    virtual std::auto_ptr<CTerrainPatch> CreatePatch(class CPatchElevationData *pPatchElevData,
                                                     class CRQTTriangulation *pAdaptiveTriangulation)const = 0;

    // Calculates bounding box for the specified patch
    void CalculatePatchBoundingBox(const SQuadTreeNodeLocation &pos, CElevationDataSource *pElev,
                                   SPatchBoundingBox &PatchBoundingBox)const;

    // Extract view frustum planes from the world-view-projection matrix
    void ExtractViewFrustumPlanesFromMatrix(const D3DXMATRIX &Matrix, SViewFrustum &ViewFrustum);
    
    // Tests if bounding box is visible by the camera
    bool IsBoxVisible(const SPatchBoundingBox &Box);

    // Creates a patch for the specified quad tree node
    std::auto_ptr<CTerrainPatch> CreatePatchForNode(CPatchQuadTreeNode &PatchNode,
                                                    class CPatchElevationData *pPatchElevData,
                                                    class CRQTTriangulation *pAdaptiveTriangulation,
                                                    std::auto_ptr<CTerrainPatch> pPatch = std::auto_ptr<CTerrainPatch>() );

    // Total number of levels in patch quad tree hierarchy
    int m_iNumLevelsInPatchHierarchy;
    
    // Active patch is a leaf node of a current quad tree.
    // Not all active patches are visible
    struct SOptimalPatchInfo
    {
        const CPatchQuadTreeNode *pPatchQuadTreeNode;
        bool bIsPatchVisible;
    };
    typedef std::vector<SOptimalPatchInfo> OptimalPatchesList;
    OptimalPatchesList m_OptimalPatchesList; // List of all optimal patches in a model
    int m_iTotalTrianglesRendered; // Total number of triangles rendered during last frame
    std::vector<SPatchRenderingInfo> m_PatchRenderingInfo;

    int m_iPatchSize;
    int m_iNumLevelsInLocalPatchQT; // Number of levels in patch quad tree.
                                    // If patch size is 128x128, then its quad tree 
                                    // consists of 8 levels
    
    CPatchQuadTreeNode m_PatchQuadTreeRoot; // Root of the quad tree
    SRenderingParams m_Params; // Rendering params

    CElevationDataSource *m_pDataSource; // Pointer to elevation data source

    CTriangDataSource *m_pTriangDataSource; // Pointer to adaptive triangulation data source

    SViewFrustum m_CameraViewFrustum; // View frustum of the main camera
    float m_fViewportStretchConst; // This value is used to calculate patch
                                   // screen-space error approximation

    D3DXVECTOR3 m_vCameraPos;
    D3DXMATRIX m_CameraViewMatrix;
    D3DXMATRIX m_CameraProjMatrix;
    D3DXMATRIX m_CameraViewProjMatrix;

	D3DXVECTOR4 m_vQuadTreePreviewScrPos; // Position of the mini map

	float m_fViewportWidth, m_fViewportHeight; 
		
	mutable SRWLOCK	m_srwLock;
private:
    // Calculates screen-space error bound for the specified patch
    float CalculatePatchScrSpaceErrorBound(const SPatchBoundingBox &PatchBoundBox,
                                           float fGuaranteedPatchErrorBound);

    // Recursively traverses the tree and updates it
    void RecursiveDetermineOptimalPatches(CPatchQuadTreeNode &PatchNode);

    // Recursively traverses the whole hierarchy and builds adaptive 
    // triangulation for each node
    void RecursiveBuildPatchTriangulations(const SQuadTreeNodeLocation &pos,
                                           float fTriangulationError,
                                           CPatchElevationData *pElevData,
                                           std::auto_ptr<class CRQTTriangulation> &pAdaptiveTriangulation);
    
    // Recursively traverses the tree and waits while each async taks (if any)
    // is completed
    void RecursiveWaitForAsyncTaks(CPatchQuadTreeNode &PatchNode);
    
    // Adds async task for scheduling
    bool AddTask(ITask *pTask);

    // Adds current patch to the list of active patches in current model
    void AddPatchToOptimalPatchesList(CPatchQuadTreeNode *pPatchQTNode);
    
    // Triangulation statistics for each level of the hierarchy
    struct SLevelAdaptiveTriangulationStat
    {
        LONGLONG m_llTotalTriangles;
        size_t TotalCompressedDataSize;
        SLevelAdaptiveTriangulationStat() : m_llTotalTriangles(0), TotalCompressedDataSize(0) {}
    };
    std::vector< SLevelAdaptiveTriangulationStat > m_AdaptiveTriangulationStat;


};
