#pragma once 

#include <EngineCore/Util/dynarray.h>

#include "TerrainConfig.h"
#include "LargePosition.h"
#include "TerrainVisibilityCollector.h"

class TerrainSectorManager;
class TerrainSector;
class SectorTile;
class Terrain;
class TerrainVisibilityCollector;
class TerrainLockObject;
class TerrainSectorMeshPageInfo;
typedef MSmartPtr<TerrainSector> TerrainSectorPtr;


// two footstep constraints - if this changes, it must also be changed in the shaders
#define MAX_SPHERE_CONSTRAINTS  2


/// \brief
///   Helper structure that contains information about LOD index range
/// \internal
struct SectorMeshRenderRange_t
{
	SectorMeshRenderRange_t() {m_iFirstPrim=m_iPrimCount=0;}
	SectorMeshRenderRange_t operator = (const SectorMeshRenderRange_t& other)
	{
		m_iFirstPrim = other.m_iFirstPrim;
		m_iPrimCount = other.m_iPrimCount;
		return *this;
	}
	inline void SetRange(int iFirstPrim, int iCount) {m_iFirstPrim=iFirstPrim;m_iPrimCount=iCount;}

	int m_iFirstPrim, m_iPrimCount;
};

///   Helper structure that holds all edge permutations of VSectorMeshRenderRange_t
struct SectorMeshLODLevelInfo_t
{

	// indexing order: top,right,bottom,left
	SectorMeshRenderRange_t m_EdgeInfo[2][2][2][2];
};

//LOD constants
const char PAGE_LOD_NOTVISIBLE        = (char)-1;
const char PAGE_LOD_MASK              = 0x0f;
const char PAGE_LOD_CONSTRAINED       = 0x10;
const char PAGE_LOD_FORCEFIXED        = 0x20;


class TerrainSectorManager
{
public:
	TerrainSectorManager(TerrainConfig& config);
	virtual ~TerrainSectorManager();

	void InitTerrain(Renderer* pRenderer);
	void DeInitTerrain();
	static int CreateLODIndexBuffer(const TerrainConfig &cfg, int iLOD, int iUp, int iRight, int iBottom, int iLeft, DynArray_cl<unsigned short> &pDestBuffer , int iStartIndex, const bool* pTileHoleMask);
	void EnsureSharedMeshesCreated();
	void SetAllowPurging(bool bStatus);


	inline TerrainSector *GetSector(int x,int y) const
	{
		assert(m_pSector && x>=0 && x<m_Config.m_iSectorCount[0] && y>=0 && y<m_Config.m_iSectorCount[1]);
		return m_pSector[y*m_Config.m_iSectorCount[0]+x];
	}

	SectorTile *GetTile(int x,int y) const;

	TerrainSectorMeshPageInfo *GetMeshPageInfo(int x, int y);
	bool GetMeshPageInfo(int x, int y, int &iGlobalIndex) const;

	// visibility
	void BeginVisibilityUpdate(TerrainVisibilityCollector &infoComp);
	void EndVisibilityUpdate(TerrainVisibilityCollector &infoComp);

public:
	Terrain *m_pTerrain;           ///< owner terrain
	TerrainConfig& m_Config;
	int m_iSectorCount;
	TerrainSectorPtr*	m_pSector;	
	int m_iPickingMeshDetail;

	TerrainVisibilityCollector* m_pRenderCollector;
	unsigned int m_iVisibleBitmask; ///< terrain's render filter mask

	// LOD
	float m_fLODScaling;
	static float g_fDecorationDistanceScaling;
	static float g_fDecorationDistanceInvScaling;

	// shader
	bool m_bSphereConstraintConstantsDirty, m_bNeedsLightgridUpdate, m_bGlobalsCBDirty, m_bShaderUsesDirectionalLight;
	LargePosition m_vSphereConstraintPos[MAX_SPHERE_CONSTRAINTS];
	float m_fSphereConstraintRadius[MAX_SPHERE_CONSTRAINTS];

	ShaderID terrainShader;

	// shared meshes:
	// IndexBuffer	
	VertexBufferID terrainVB;
	IndexBufferID  terrainIB;
	VertexFormatID terrainVF;

	mat4                World;
	mat4                View;
	mat4                Projection;
		
	SectorMeshLODLevelInfo_t m_LODInfo[MAX_MESH_LOD];
	bool m_bLastMeshHasHoles;

	Renderer* m_pRenderer;
private:

public:
	//static void InitProfiling();



};