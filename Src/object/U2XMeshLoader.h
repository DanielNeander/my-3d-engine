/************************************************************************
module	:	U2XMeshLoader
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_XMESHLOADER_H
#define U2_XMESHLOADER_H

#include "U2MeshLoader.h"

class U2_3D U2XMeshLoader : public U2MeshLoader
{
public:
	

	U2XMeshLoader();

	virtual ~U2XMeshLoader();

	virtual bool Open();
	virtual void Close();

	virtual bool ReadVertices(void* pvBuffer, int buffSize);
	virtual bool ReadIndices(void* pvBuffer, int buffSize);
	virtual bool ReadEdges(void* pvBuffer, int buffSize);
	
	
};

typedef U2SmartPtr<U2XMeshLoader> U2XMeshLoaderPtr;

#endif