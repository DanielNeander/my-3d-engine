#pragma once 
//--------------------------------------------------------------------------------------
// File: TerrainMesh.h
//
// Copyright (C) 2005 - Harald Vistnes (harald@vistnes.org)
// All rights reserved worldwide.
//
// This software is provided "as is" without express or implied
// warranties. You may freely copy and compile this source into
// applications you distribute provided that the copyright text
// below is included in the resulting source code, for example:
// "Portions Copyright (C) Harald Vistnes, 2005"
//--------------------------------------------------------------------------------------
//#include <windows.h>	// InterlockedIncrement Call
#include <U2_3D/Src/main/U2Object.h>
#include <U2_3D/Src/main/U2Node.h>
#include <U2_3D/Src/dx9/U2N2Mesh.h>

#include <U2Misc/Src/U2Misc.h>

class Terrain;


class TerrainMeshGroup : public nMeshGroup
{
public:
	virtual bool Render(U2SceneMgr* pSceneMgr, U2RenderContext* pCxt);

};

class TerrainMesh : public U2N2Mesh 
{
	DECLARE_RTTI;
public:
	TerrainMesh(U2TriangleData* pData, Terrain* pTerrain);
	~TerrainMesh();

	bool Initialize(int iNumRows, int iNumCols, bool bSkirt);

	inline int GetNumRows() const { return m_iNumRows; }
	inline int GetNumCols() const { return m_iNumCols; }
	inline bool HasSkirt() const { return m_bSkirt; }

	virtual void GetVisibleSet(U2Culler& culler, bool bNoCull);

	Terrain* GetTerrain() const { return m_pTerrain; }
	
private:
	bool InitNoSkirt();
	bool InitSkirt();

	bool FillVertices(float *vPtr);
	bool FillSkirtVertices(float *vPtr);

private:
	//U2N2MeshPtr	m_spMesh;
	Terrain* m_pTerrain;

	int	m_iNumRows;
	int m_iNumCols;
	bool	m_bSkirt;
	bool	m_bCreated;
};

