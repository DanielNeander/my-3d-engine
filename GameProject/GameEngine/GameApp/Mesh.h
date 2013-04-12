#pragma once

#include "EngineCore/Math/BoundingBox.h"
#include "SceneNode.h"


// Base class for meshes
//
// Mesh_D3D9 and Mesh_D3D11 will derive from this
class Mesh : public SceneNode
{
public:
  Mesh();
  virtual ~Mesh();

  // API specific functions
  virtual bool CreateBuffers(void) = 0;
  virtual void Draw(void) = 0;
  virtual void DrawShadow(void) {}
  virtual void DrawCascadedShadow(void) {}
  virtual void DrawCascaded(void) {}


  // generic functions
  bool LoadFromLWO(const char *strFile);
  void CalculateOOBB(void);
  void CalculateAABB(void);

  float* GetVertices() { return m_pVertices; }
  uint16* GetIndices() { return m_pIndices; }
  uint32 GetFaceCount();
    
  void GetTriangleIndices(unsigned short i, unsigned short& i0,
	  unsigned short& i1, unsigned short& i2) const;
  void GetTriangle(uint32 index, noVec3& v0, noVec3& v1, noVec3& v2);
public:
  unsigned int m_iNumVertices;
  unsigned int m_iNumTris;
  unsigned int m_iVertexSize;
  unsigned short *m_pIndices;  
  unsigned int *m_pIndices32;  
  float *m_pVertices;
    
  // used in geometry shader (D3D11)
  int m_iFirstSplit;
  int m_iLastSplit;
  bool m_bOnlyReceiveShadows;

  // object space bounding box
  BoundingBox m_OOBB;
  // world space bounding box (use CalculateAABB to update)
  BoundingBox m_AABB;

  Primitives	m_PrimitiveType;
  struct RenderData	m_rd;
};

