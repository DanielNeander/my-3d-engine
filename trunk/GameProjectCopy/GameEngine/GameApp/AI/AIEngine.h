#ifndef __AIENGINE_H
#define __AIENGINE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <vector>

namespace AI
{
	struct AIObjectInfo
	{
		D3DXMATRIX	m_transformation;
		noVec4		m_extra;
	};

	typedef std::vector<AIObjectInfo> AIOBJECTINFOS;

	typedef std::vector<noVec3> PATH;

	struct Engine {
		virtual ~Engine()=0;
		virtual void setWorldSize( float hsize, float vsize,  int divh, int divv, float scale )=0;
		virtual bool create(unsigned int num_characters, PATH *path, int num_threads=-1 )=0;
		virtual bool destroy()=0;
		virtual bool tick()=0;
		virtual bool getPositions( AIOBJECTINFOS &mobs )=0;
		virtual unsigned getNumMOBS()=0;
		virtual bool getMOBInfo(unsigned index, AIObjectInfo &info)=0;
		virtual bool setMOBInfo(unsigned index, AIObjectInfo &info)=0;
		virtual void updateDynamicsObject( unsigned int index , noVec3 pos, D3DXMATRIX rot, float radius)=0;
		virtual void updateDynamicsObject( unsigned int index , noVec3 pos, D3DXMATRIX rot, float w, float h, float d)=0;
	};

	Engine*factory( unsigned int num_characters , PATH* path);
}

#endif
