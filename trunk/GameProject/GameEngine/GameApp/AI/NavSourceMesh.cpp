#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "Recast.h"
#include "RecastLog.h"
#include "RecastDebugDraw.h"
#include "DetourNavMesh.h"
#include "GameApp/SceneNode.h"
#include <ModelLib/M2Loader.h>
#include "../ObjLoader.h"
#include "../Mesh.h"
#include "NavSourceMesh.h"

static bool intersectSegmentTriangle(const float* sp, const float* sq,
	const float* a, const float* b, const float* c,
	float &t)
{
	float v, w;
	float ab[3], ac[3], qp[3], ap[3], norm[3], e[3];
	rcVsub(ab, b, a);
	rcVsub(ac, c, a);
	rcVsub(qp, sp, sq);

	// Compute triangle normal. Can be precalculated or cached if
	// intersecting multiple segments against the same triangle
	rcVcross(norm, ab, ac);

	// Compute denominator d. If d <= 0, segment is parallel to or points
	// away from triangle, so exit early
	float d = rcVdot(qp, norm);
	if (d <= 0.0f) return false;

	// Compute intersection t value of pq with plane of triangle. A ray
	// intersects iff 0 <= t. Segment intersects iff 0 <= t <= 1. Delay
	// dividing by d until intersection has been found to pierce triangle
	rcVsub(ap, sp, a);
	t = rcVdot(ap, norm);
	if (t < 0.0f) return false;
	if (t > d) return false; // For segment; exclude this code line for a ray test

	// Compute barycentric coordinate components and test if within bounds
	rcVcross(e, qp, ap);
	v = rcVdot(ac, e);
	if (v < 0.0f || v > d) return false;
	w = -rcVdot(ab, e);
	if (w < 0.0f || v + w > d) return false;

	// Segment/ray intersects triangle. Perform delayed division
	t /= d;

	return true;
}

static char* parseRow(char* buf, char* bufEnd, char* row, int len)
{
	bool start = true;
	bool done = false;
	int n = 0;
	while (!done && buf < bufEnd)
	{
		char c = *buf;
		buf++;
		// multirow
		switch (c)
		{
		case '\n':
			if (start) break;
			done = true;
			break;
		case '\r':
			break;
		case '\t':
		case ' ':
			if (start) break;
		default:
			start = false;
			row[n++] = c;
			if (n >= len-1)
				done = true;
			break;
		}
	}
	row[n] = '\0';
	return buf;
}


NavSourceMesh::NavSourceMesh() :
	m_chunkyMesh(0),
	m_mesh(0),
	m_offMeshConCount(0),
	m_volumeCount(0)
{

}

NavSourceMesh::~NavSourceMesh()
{
	delete m_chunkyMesh;
	delete m_mesh;
}

void NavSourceMesh::addOffMeshConnection( const float* spos, const float* epos, const float rad, unsigned char bidir, unsigned char area, unsigned short flags )
{
	if (m_offMeshConCount >= MAX_OFFMESH_CONNECTIONS) return;
	float* v = &m_offMeshConVerts[m_offMeshConCount*3*2];
	m_offMeshConRads[m_offMeshConCount] = rad;
	m_offMeshConDirs[m_offMeshConCount] = bidir;
	m_offMeshConAreas[m_offMeshConCount] = area;
	m_offMeshConFlags[m_offMeshConCount] = flags;
	rcVcopy(&v[0], spos);
	rcVcopy(&v[3], epos);
	m_offMeshConCount++;
}

void NavSourceMesh::deleteOffMeshConnection( int i )
{
	m_offMeshConCount--;
	float* src = &m_offMeshConVerts[m_offMeshConCount*3*2];
	float* dst = &m_offMeshConVerts[i*3*2];
	rcVcopy(&dst[0], &src[0]);
	rcVcopy(&dst[3], &src[3]);
	m_offMeshConRads[i] = m_offMeshConRads[m_offMeshConCount];
	m_offMeshConDirs[i] = m_offMeshConDirs[m_offMeshConCount];
	m_offMeshConAreas[i] = m_offMeshConAreas[m_offMeshConCount];
	m_offMeshConFlags[i] = m_offMeshConFlags[m_offMeshConCount];
}

void NavSourceMesh::addConvexVolume( const float* verts, const int nverts, const float minh, const float maxh, unsigned char area )
{
	if (m_volumeCount >= MAX_VOLUMES) return;
	ConvexVolume* vol = &m_volumes[m_volumeCount++];
	memset(vol, 0, sizeof(ConvexVolume));
	memcpy(vol->verts, verts, sizeof(float)*3*nverts);
	vol->hmin = minh;
	vol->hmax = maxh;
	vol->nverts = nverts;
	vol->area = area;
}

void NavSourceMesh::deleteConvexVolume( int i )
{
	m_volumeCount--;
	m_volumes[i] = m_volumes[m_volumeCount];
}

bool NavSourceMesh::loadMesh( const char* filename )
{
	if (m_mesh)
	{
		delete m_chunkyMesh;
		m_chunkyMesh = 0;
		delete m_mesh;
		m_mesh = 0;
	}
	m_offMeshConCount = 0;
	m_volumeCount = 0;

	m_mesh = new rcMeshLoaderObj;
	if (!m_mesh)
	{
		return false;
	}
	m_mesh->objLoader.SetScale(2.f);					
	if (!m_mesh->Load(filename))
		return false;
		
	bbox.Set(m_mesh->getVerts(), m_mesh->getVertCount(), sizeof(noVec3)); 
	rcCalcBounds(m_mesh->getVerts(), m_mesh->getVertCount(), bbox.m_vMin.ToFloatPtr(),  bbox.m_vMax.ToFloatPtr() );
	
	m_chunkyMesh = new rcChunkyTriMesh;
	if (!m_chunkyMesh)
	{
		return false;
	}

	if (!rcCreateChunkyTriMesh(m_mesh->getVerts(), m_mesh->getTris(), m_mesh->getTriCount(), 256, m_chunkyMesh))
	{
		return false;
	}
		
	return true;
}

bool NavSourceMesh::load( Mesh* pMesh )
{
	if (m_mesh)
	{
		delete m_chunkyMesh;
		m_chunkyMesh = 0;
		delete m_mesh;
		m_mesh = 0;
	}
	m_offMeshConCount = 0;
	m_volumeCount = 0;

	m_mesh = new rcMeshLoaderObj;
	if (!m_mesh)
	{
		return false;
	}

	if(!m_mesh->LoadMesh(pMesh))
		return false;

	bbox.Set(pMesh->GetVertices(), pMesh->m_iNumVertices, pMesh->m_iVertexSize); 
	rcCalcBounds(m_mesh->getVerts(), pMesh->m_iNumVertices, bbox.m_vMin.ToFloatPtr(),  bbox.m_vMax.ToFloatPtr() );

	m_chunkyMesh = new rcChunkyTriMesh;
	if (!m_chunkyMesh)
	{
		return false;
	}
		
	if (!rcCreateChunkyTriMesh(m_mesh->getVerts(), (const int*)m_mesh->getTris(), m_mesh->getTriCount(), 256, m_chunkyMesh))
	{
		return false;
	}
}

bool NavSourceMesh::loadTerrain()
{
	if (m_mesh)
	{
		delete m_chunkyMesh;
		m_chunkyMesh = 0;
		delete m_mesh;
		m_mesh = 0;
	}
	m_offMeshConCount = 0;
	m_volumeCount = 0;

	m_mesh = new rcMeshLoaderObj;
	if (!m_mesh)
		return false;

	if (!m_mesh->LoadTerrain())
	{
		return false;
	}

	//rcCalcBounds(m_mesh->getVerts(), m_mesh->getVertCount(), m_meshBMin, m_meshBMax);
	bbox.Set(m_mesh->getVerts(), m_mesh->getVertCount(), sizeof(noVec3)); 

	m_chunkyMesh = new rcChunkyTriMesh;
	if (!m_chunkyMesh)
	{
		return false;
	}

	if (!rcCreateChunkyTriMesh(m_mesh->getVerts(), m_mesh->getTris(), m_mesh->getTriCount(), 256, m_chunkyMesh))
	{
		return false;
	}


	return true;
}

bool NavSourceMesh::raycastMesh(float* src, float* dst, float& tmin)
{
	float dir[3];
	rcVsub(dir, dst, src);

	int nt = m_mesh->getTriCount();
	const float* verts = m_mesh->getVerts();
	const float* normals = m_mesh->getNormals();
	const int* tris = m_mesh->getTris();
	tmin = 1.0f;
	bool hit = false;

	for (int i = 0; i < nt*3; i += 3)
	{
		const float* n = &normals[i];
		if (rcVdot(dir, n) > 0)
			continue;

		float t = 1;
		if (intersectSegmentTriangle(src, dst,
			&verts[tris[i]*3],
			&verts[tris[i+1]*3],
			&verts[tris[i+2]*3], t))
		{
			if (t < tmin)
				tmin = t;
			hit = true;
		}
	}

	return hit;
}


