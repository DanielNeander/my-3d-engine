#pragma once

#include "Vector.h"

class BoundingBox
{
public:
  inline BoundingBox();

  // create from minimum and maximum vectors
  inline BoundingBox(const noVec3 &vMin, const noVec3 &vMax);
  // create from set of points
  inline BoundingBox(const void *pPoints, int iNumPoints, int iStride);

  // set from minimum and maximum vectors
  inline void Set(const noVec3 &vMin, const noVec3 &vMax);

  // set from set of points
  inline void Set(const void *pPoints, int iNumPoints, int iStride);

  // returns size of bounding box
  inline noVec3 GetSize(void) const { return m_vMax - m_vMin; }

  inline float	BoundingRadiusFromAABB() const;

  const float w() const { return m_vMax.x - m_vMin.x; }
  const float h() const { return m_vMax.y - m_vMin.y; }
  const float d() const { return m_vMax.z - m_vMin.z; }

  // compute union
  inline void Union(const BoundingBox &bb2);

  inline float GetDistanceSqr(const noVec3 &pos) const
  {
	  noVec3 clamped(pos);
	  clamped.Min(m_vMax);
	  clamped.Max(m_vMin);
	  clamped -= pos;
	  return clamped*clamped;
  }

  /// \brief
  ///   Get the squared distance of a point to the bounding box (closest distance), but ignoring
  ///   the difference in z (height)
  inline float GetDistanceSqrXY(const noVec3 &pos) const
  {
	  noVec2 clamped(__min(pos.x,m_vMax.x),__min(pos.y,m_vMax.y));
	  clamped.x = __max(clamped.x,m_vMin.x) - pos.x;
	  clamped.y = __max(clamped.y,m_vMin.y) - pos.y;
	  return clamped.x*clamped.x + clamped.y*clamped.y;
  }

  const noVec3 GetCenter() const { return (m_vMin + m_vMax) * 0.5f; }

  inline float GetDistance(const noVec3 &pos) const {return sqrtf(GetDistanceSqr(pos));}

  inline float GetDistanceXY(const noVec3 &pos) const {return sqrtf(GetDistanceSqrXY(pos));}

  noVec3 m_pPoints[8];
  noVec3 m_vMin, m_vMax;
};

/////////////////////

BoundingBox::BoundingBox()
{
  m_vMin = noVec3( FLT_MAX, FLT_MAX, FLT_MAX);
  m_vMax = noVec3(-FLT_MAX,-FLT_MAX,-FLT_MAX);
}

// compute union of two bounding boxes
inline BoundingBox Union(const BoundingBox &bb1, const BoundingBox &bb2)
{
  noVec3 vMin, vMax;
  vMin.x = Min(bb1.m_vMin.x, bb2.m_vMin.x);
  vMin.y = Min(bb1.m_vMin.y, bb2.m_vMin.y);
  vMin.z = Min(bb1.m_vMin.z, bb2.m_vMin.z);
  vMax.x = Max(bb1.m_vMax.x, bb2.m_vMax.x);
  vMax.y = Max(bb1.m_vMax.y, bb2.m_vMax.y);
  vMax.z = Max(bb1.m_vMax.z, bb2.m_vMax.z);
  return BoundingBox(vMin, vMax);
}

// compute union
void BoundingBox::Union(const BoundingBox &bb2)
{
  (*this) = ::Union(*this, bb2);
}

// create from minimum and maximum vectors
inline BoundingBox::BoundingBox(const noVec3 &vMin, const noVec3 &vMax)
{
  Set(vMin, vMax);
}

// create from set of points
inline BoundingBox::BoundingBox(const void *pPoints, int iNumPoints, int iStride)
{
  Set(pPoints, iNumPoints, iStride);
}


// create from minimum and maximum vectors
inline void BoundingBox::Set(const noVec3 &vMin, const noVec3 &vMax)
{
  // calculate points
  m_pPoints[0] = noVec3(vMin.x, vMin.y, vMin.z);
  m_pPoints[1] = noVec3(vMax.x, vMin.y, vMin.z);
  m_pPoints[2] = noVec3(vMin.x, vMin.y, vMax.z);
  m_pPoints[3] = noVec3(vMax.x, vMin.y, vMax.z);
  m_pPoints[4] = noVec3(vMin.x, vMax.y, vMin.z);
  m_pPoints[5] = noVec3(vMax.x, vMax.y, vMin.z);
  m_pPoints[6] = noVec3(vMin.x, vMax.y, vMax.z);
  m_pPoints[7] = noVec3(vMax.x, vMax.y, vMax.z);
  m_vMin = vMin;
  m_vMax = vMax;
}

// create from set of points
inline void BoundingBox::Set(const void *pPoints, int iNumPoints, int iStride)
{
  // calculate min and max vectors
  m_vMin = noVec3( FLT_MAX, FLT_MAX, FLT_MAX);
  m_vMax = noVec3(-FLT_MAX,-FLT_MAX,-FLT_MAX);
  char *pData = (char *)pPoints;
  for(int i=0; i<iNumPoints; i++)
  {
    const noVec3 &vPoint = *(noVec3*)pData;
    if(vPoint.x < m_vMin.x) m_vMin.x = vPoint.x;
    if(vPoint.y < m_vMin.y) m_vMin.y = vPoint.y;
    if(vPoint.z < m_vMin.z) m_vMin.z = vPoint.z;

    if(vPoint.x > m_vMax.x) m_vMax.x = vPoint.x;
    if(vPoint.y > m_vMax.y) m_vMax.y = vPoint.y;
    if(vPoint.z > m_vMax.z) m_vMax.z = vPoint.z;

    // next position
    pData += iStride;
  }
  // create from vectors
  Set(m_vMin, m_vMax);
}

float BoundingBox::BoundingRadiusFromAABB() const
{
	noVec3 magnitude = m_vMax;
	magnitude.Max(-m_vMax);
	magnitude.Max(m_vMin);
	magnitude.Max(-m_vMin);

	return magnitude.Length();
}

