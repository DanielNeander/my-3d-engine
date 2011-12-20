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
#include <windows.h>	// InterlockedIncrement Call
#include <U2_3D/Src/main/U2Object.h>
#include <U2_3D/Src/main/U2Node.h>
#include <U2_3D/Src/dx9/U2N2Mesh.h>

#include <U2Misc/Src/U2Misc.h>

class Terrain;

class QuadMesh : public U2N2Mesh 
{
	DECLARE_RTTI;
public:
	QuadMesh(U2TriangleData *pData);
	bool Initialize(DWORD m, DWORD n, float dx);

private:
	DWORD mNumRows;
	DWORD mNumCols;

};

