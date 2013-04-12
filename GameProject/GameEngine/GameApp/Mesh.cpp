#include "stdafx.h"

#include "Common.h"
#include "LWOFile.h"
#include "SceneNode.h"
#include "Mesh.h"

Mesh::Mesh()
{
  m_iVertexSize = 0;
  m_iNumVertices = 0;
  m_iNumTris = 0;
  m_iVertexSize = 0;
  m_pIndices = NULL;
  m_pIndices32 = NULL;

  m_pVertices = NULL;
  m_PrimitiveType = PRIM_TRIANGLES;

  m_iFirstSplit = INT_MAX;
  m_iLastSplit = INT_MIN;
  m_bOnlyReceiveShadows = false;
}

Mesh::~Mesh()
{
  //SAFE_DELETE_ARRAY(m_pVertices);
  //SAFE_DELETE_ARRAY(m_pIndices);
}

bool Mesh::LoadFromLWO(const char *strFile)
{
  LWOFile lwo;

  // try to load
  if(!lwo.LoadFromFile(strFile))
  {
    // failed, print all errors
    std::string strErrors = "Loading LWO failed:\n";
    std::list<std::string>::iterator it;
    for(it = lwo.m_Errors.begin(); it != lwo.m_Errors.end(); ++it) strErrors += (*it)+"\n";
    strErrors += "\nIf you launched the demo from Visual Studio, set the 'Working Directory' in project settings correctly.";
    MessageBoxA(NULL,strErrors.c_str(),"Error!",MB_OK);
    return false;
  }

  // calculate normals
  lwo.CalculateNormals();

  // get first layer only
  LWOFile::Layer *pLayer = lwo.m_Layers[0];

  // make vertex buffer
  //
  // 9 floats per vertex (3 pos, 3 norm, 3 color)
  m_pVertices = new float[pLayer->m_iPoints*9];
  m_iVertexSize = sizeof(float)*9;
  m_iNumVertices = pLayer->m_iPoints;
  for(unsigned int i = 0; i < pLayer->m_iPoints; i++)
  {
    // copy pos
    memcpy(&m_pVertices[i*9+0], &pLayer->m_Points[i*3], sizeof(float)*3);

    // copy normal
    memcpy(&m_pVertices[i*9+3], &pLayer->m_Normals[i*3], sizeof(float)*3);

    // color
    m_pVertices[i*9+6] = 1.0f;
    m_pVertices[i*9+7] = 1.0f;
    m_pVertices[i*9+8] = 1.0f;
  }

  // make index buffer
  //
  m_pIndices = new unsigned short[pLayer->m_Polygons.size()*3];
  m_iNumTris = 0;
  for(unsigned int i = 0; i < pLayer->m_Polygons.size(); i++)
  {
    const LWOFile::Polygon &poly = pLayer->m_Polygons[i];

    // skip non-triangles
    if(poly.m_Vertices.size() != 3) continue;

    // find surface
    const LWOFile::Surface *pSurf = NULL;
    for(unsigned int j = 0; j < lwo.m_Surfaces.size(); j++)
    {
      const LWOFile::Surface *pSurf = &lwo.m_Surfaces[j];
      if(pSurf->m_strName != lwo.m_StringTable[poly.m_iSurface]) continue;

      // apply base color
      for(int j = 0; j < 3; j++)
      {
        m_pVertices[poly.m_Vertices[j]*9 + 6] = pSurf->m_vBaseColor[0];
        m_pVertices[poly.m_Vertices[j]*9 + 7] = pSurf->m_vBaseColor[1];
        m_pVertices[poly.m_Vertices[j]*9 + 8] = pSurf->m_vBaseColor[2];
      }
      break;
    }

    m_pIndices[m_iNumTris*3+0] = poly.m_Vertices[0];
    m_pIndices[m_iNumTris*3+1] = poly.m_Vertices[1];
    m_pIndices[m_iNumTris*3+2] = poly.m_Vertices[2];
    m_iNumTris++;
  }

  // apply ambient occlusion from texture coordinates, if available
  //
  if(pLayer->m_UVMaps.size() > 0)
  {
    LWOFile::UVMap *pUVMap = pLayer->m_UVMaps[0];
    for(unsigned int i = 0; i < m_iNumVertices; i++)
    {
      m_pVertices[i*9+6] *= pUVMap->m_Values[i*2];
      m_pVertices[i*9+7] *= pUVMap->m_Values[i*2];
      m_pVertices[i*9+8] *= pUVMap->m_Values[i*2];
    }
  }


  CalculateOOBB();

  if(!CreateBuffers()) return false;
  return true;
}

void Mesh::CalculateOOBB(void)
{
  m_OOBB.Set(m_pVertices, m_iNumVertices, m_iVertexSize);
}

uint32 Mesh::GetFaceCount()
{	
	return m_iNumTris;
}

void Mesh::GetTriangle( uint32 index, noVec3& v0, noVec3& v1, noVec3& v2 )
{
	int numElems = m_iVertexSize / sizeof(float);

	switch(m_PrimitiveType)
	{
	case PRIM_TRIANGLES:
		v0 = noVec3(m_pVertices[m_pIndices[index*3] * numElems], m_pVertices[m_pIndices[index*3] * numElems+1],
			m_pVertices[m_pIndices[index*3] * numElems+2]);
		v1 = noVec3(m_pVertices[m_pIndices[index*3+1] * numElems], m_pVertices[m_pIndices[index*3+1] * numElems+1],
			m_pVertices[m_pIndices[index*3+1] * numElems+2]);
		v2 = noVec3(m_pVertices[m_pIndices[index*3+2] * numElems], m_pVertices[m_pIndices[index*3+2] * numElems+1],
			m_pVertices[m_pIndices[index*3+2] * numElems+2]);
		break;
	case PRIM_TRIANGLE_STRIP:
		{
			unsigned short i0, i1, i2;
			GetTriangleIndices(index, i0, i1, i2);
			v0 = noVec3(m_pVertices[i0 * numElems],m_pVertices[i0 * numElems+1],m_pVertices[i0 * numElems+2]);
			v1 = noVec3(m_pVertices[i1 * numElems],m_pVertices[i1 * numElems+1],m_pVertices[i1 * numElems+2]);
			v2 = noVec3(m_pVertices[i2 * numElems],m_pVertices[i2 * numElems+1],m_pVertices[i2 * numElems+2]);
		}
		break;
	}
}

void Mesh::GetTriangleIndices( unsigned short i, unsigned short& i0, unsigned short& i1, unsigned short& i2 ) const
{
	switch(m_PrimitiveType)
	{
	case PRIM_TRIANGLES:
		{		
			unsigned int j0 = 3 * i;
			unsigned int j1 = j0 + 1;
			unsigned int j2 = j1 + 1;
			i0 = m_pIndices[j0];
			i1 = m_pIndices[j1];
			i2 = m_pIndices[j2];
		}
		break;
	case PRIM_TRIANGLE_STRIP:
		if(i & 1)
		{
			i0 = m_pIndices[i + 1];
			i1 = m_pIndices[i];
		}
		else 
		{
			i0 = m_pIndices[i];
			i1 = m_pIndices[i + 1];
		}
		i2 = m_pIndices[i + 2];	
		break;
	}

	
}

void Mesh::CalculateAABB(void)
{
	// transform OOBB points to world space
	noVec4 vTransformed[8];
	for(int i=0;i<8;i++)
	{
		vTransformed[i] = noVec4(worldTM_ * m_OOBB.m_pPoints[i], 1.f);
	}
	// set new AABB
	m_AABB.Set(vTransformed, 8, sizeof(noVec4));
}