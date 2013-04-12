#pragma once 

#include <EngineCore/Math/Vector.h>
#include "TerrainConfig.h"

class LargePosition
{
public:
	inline LargePosition() { m_iSectorX = m_iSectorY = 0; }
	inline LargePosition(const LargePosition& vOther) {m_iSectorX=vOther.m_iSectorX;m_iSectorY=vOther.m_iSectorY;m_vSectorOfs=vOther.m_vSectorOfs;}
	inline LargePosition(const TerrainConfig& config, const noVec3& vRenderPos);
	inline LargePosition(int iSectorX, int iSectorY, const noVec3& vRelPos)  {m_iSectorX=iSectorX;m_iSectorY=iSectorY;m_vSectorOfs=vRelPos;}

	inline noVec3   ToRenderSpace(const TerrainConfig& config) const;
	inline void		FromRenderSpace(const TerrainConfig& config, const noVec3& vRenderPos);
	inline noVec3	ToRenderSpace(const TerrainConfig& config, int iOfsSectorX, int iOfsSectorY) const;

	inline void		Validate(const TerrainConfig& cf);
	inline bool		IsValid(const TerrainConfig& cf) const;

	static inline LargePosition Sum(const TerrainConfig& cf, const LargePosition& p1, const LargePosition& p2);
	static inline LargePosition Sum(const TerrainConfig& cf, const LargePosition& p1, const noVec3& p2);

	inline void IncPos(const TerrainConfig &cf, const noVec3& vDelta) {m_vSectorOfs+=vDelta;Validate(cf);}
	inline void ToString(char *szBuffer) const {sprintf(szBuffer,"sector(%i,%i), pos(%.2f,%.2f,%.2f)",(int)m_iSectorX,(int)m_iSectorY,m_vSectorOfs.x,m_vSectorOfs.y,m_vSectorOfs.z);}

	inline static bool IsSmallerThan(int iSector1, float fOfs1, int iSector2, float fOfs2);
	inline static bool IsLargerThan(int iSector1, float fOfs1, int iSector2, float fOfs2);

public:
	noVec3	m_vSectorOfs;
	short m_iSectorX, m_iSectorY;

};

inline noVec3 LargePosition::ToRenderSpace(const TerrainConfig &config) const
{
	return noVec3(
		config.m_vTerrainPos.x + (float)(m_iSectorX/*-g_iCurrentSectorOfs[0]*/)*config.m_vSectorSize.x + m_vSectorOfs.x,
		config.m_vTerrainPos.y + (float)(m_iSectorY/*-g_iCurrentSectorOfs[1]*/)*config.m_vSectorSize.y + m_vSectorOfs.y, 
		config.m_vTerrainPos.z + m_vSectorOfs.z);
}

inline noVec3 LargePosition::ToRenderSpace(const TerrainConfig &config,int iOfsSectorX,int iOfsSectorY) const
{
	return noVec3(
		config.m_vTerrainPos.x + (float)(m_iSectorX-iOfsSectorX)*config.m_vSectorSize.x + m_vSectorOfs.x,
		config.m_vTerrainPos.y + (float)(m_iSectorY-iOfsSectorY)*config.m_vSectorSize.y + m_vSectorOfs.y, 
		config.m_vTerrainPos.z + m_vSectorOfs.z);
}


inline void LargePosition::FromRenderSpace(const TerrainConfig &config,const noVec3 &vRenderPos)
{
	m_iSectorX = 0;//g_iCurrentSectorOfs[0];
	m_iSectorY = 0;//g_iCurrentSectorOfs[1];
	m_vSectorOfs = vRenderPos-config.m_vTerrainPos;
	Validate(config);
}

inline void LargePosition::Validate(const TerrainConfig &config)
{
	if (m_vSectorOfs.x<0.f) 
	{
		int iDelta = (int)(-m_vSectorOfs.x * config.m_vInvSectorSize.x)+1;
		m_iSectorX -= iDelta;
		m_vSectorOfs.x+=config.m_vSectorSize.x*(float)iDelta;
	} 
	else if (m_vSectorOfs.x>=config.m_vInvSectorSize.x)
	{
		int iDelta = (int)(m_vSectorOfs.x * config.m_vInvSectorSize.x);
		m_iSectorX += iDelta;
		m_vSectorOfs.x-=config.m_vSectorSize.x*(float)iDelta;
	}
	if (m_vSectorOfs.y<0.f) 
	{
		int iDelta = (int)(-m_vSectorOfs.y * config.m_vInvSectorSize.y)+1;
		m_iSectorY -= iDelta;
		m_vSectorOfs.y+=config.m_vSectorSize.y*(float)iDelta;
	} 
	else if (m_vSectorOfs.y>=config.m_vInvSectorSize.y)
	{
		int iDelta = (int)(m_vSectorOfs.y * config.m_vInvSectorSize.y);
		m_iSectorY += iDelta;
		m_vSectorOfs.y-=config.m_vSectorSize.y*(float)iDelta;
	}
}

inline LargePosition LargePosition::Sum(const TerrainConfig &config,const LargePosition &p1,const LargePosition &p2)
{
	LargePosition newPos(p1.m_iSectorX+p2.m_iSectorX,p1.m_iSectorY+p2.m_iSectorY,p1.m_vSectorOfs+p2.m_vSectorOfs);
	newPos.Validate(config);
	return newPos;
}

inline LargePosition LargePosition::Sum(const TerrainConfig &config,const LargePosition &p1,const noVec3 &p2)
{
	LargePosition newPos(p1.m_iSectorX,p1.m_iSectorY,p1.m_vSectorOfs+p2);
	newPos.Validate(config);
	return newPos;
}

// true if pos1<pos2
inline bool LargePosition::IsSmallerThan(int iSector1, float fOfs1, int iSector2, float fOfs2)
{
	if (iSector1<iSector2) return true;
	if (iSector1>iSector2) return false;
	return (fOfs1<fOfs2);
}

// true if pos1>pos2
inline bool LargePosition::IsLargerThan(int iSector1, float fOfs1, int iSector2, float fOfs2)
{
	if (iSector1>iSector2) return true;
	if (iSector1<iSector2) return false;
	return (fOfs1>fOfs2);
}

class LargeBoundingBox
{
public:
	inline LargeBoundingBox() { Reset(); }
	inline LargeBoundingBox(const LargePosition& vMin, const LargePosition& vMax);

	inline LargeBoundingBox(const TerrainConfig& cf, const noVec3& vMin, const noVec3 vMax) : m_vMin(cf,vMin), m_vMax(cf,vMax) {}

	inline void Reset();

	inline bool IsValid() const;

	inline bool IsInside(const LargePosition& pos);

	inline bool IntersectsWith(const LargeBoundingBox& other);

	inline void Add(const TerrainConfig& config, const noVec3& pos);

	inline void Add(const LargePosition& pos);

	inline void Add(const LargeBoundingBox& other) {Add(other.m_vMin);Add(other.m_vMax);}

	inline void GetSectorIndices_Clamped(const TerrainConfig &config, int &x1, int &y1, int &x2, int &y2) const;

public:
	LargePosition m_vMin, m_vMax;

};