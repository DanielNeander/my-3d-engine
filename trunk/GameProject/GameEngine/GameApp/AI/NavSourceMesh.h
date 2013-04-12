#pragma once 

#include <EngineCore/Math/BoundingBox.h>
#include "AICommon.h"
#include "ChunkyTriMesh.h"
#include "AIMeshLoader.h"


static const int MAX_CONVEXVOL_PTS = 12;
struct ConvexVolume
{
	float verts[MAX_CONVEXVOL_PTS*3];
	float hmin, hmax;
	int nverts;
	int area;
};

class NavSourceMesh 
{
	BoundingBox bbox;

	rcChunkyTriMesh* m_chunkyMesh;
	rcMeshLoaderObj* m_mesh;

	// Off-Mesh connections.
	static const int MAX_OFFMESH_CONNECTIONS = 256;
	float m_offMeshConVerts[MAX_OFFMESH_CONNECTIONS*3*2];
	float m_offMeshConRads[MAX_OFFMESH_CONNECTIONS];
	unsigned char m_offMeshConDirs[MAX_OFFMESH_CONNECTIONS];
	unsigned char m_offMeshConAreas[MAX_OFFMESH_CONNECTIONS];
	unsigned short m_offMeshConFlags[MAX_OFFMESH_CONNECTIONS];
	int m_offMeshConCount;

	// Convex Volumes.
	static const int MAX_VOLUMES = 256;
	ConvexVolume m_volumes[MAX_VOLUMES];
	int m_volumeCount;

public:
	NavSourceMesh();
	~NavSourceMesh();

	bool loadMesh(const char* filename);
	bool load(class Mesh* pMesh);
	bool loadTerrain();
		

	inline const rcMeshLoaderObj* getMesh() const { return m_mesh; }
	inline const float* getMeshBoundsMin() const { return bbox.m_vMin.ToFloatPtr(); }
	inline const float* getMeshBoundsMax() const { return bbox.m_vMax.ToFloatPtr(); }
	inline rcMeshLoaderObj* getMeshObject() { return m_mesh; }

	inline const rcChunkyTriMesh* getChunkyMesh() const { return m_chunkyMesh; }

	// Off-Mesh connections.
	int getOffMeshConnectionCount() const { return m_offMeshConCount; }
	const float* getOffMeshConnectionVerts() const { return m_offMeshConVerts; }
	const float* getOffMeshConnectionRads() const { return m_offMeshConRads; }
	const unsigned char* getOffMeshConnectionDirs() const { return m_offMeshConDirs; }
	const unsigned char* getOffMeshConnectionAreas() const { return m_offMeshConAreas; }
	const unsigned short* getOffMeshConnectionFlags() const { return m_offMeshConFlags; }
	void addOffMeshConnection(const float* spos, const float* epos, const float rad,
		unsigned char bidir, unsigned char area, unsigned short flags);
	void deleteOffMeshConnection(int i);

	// Box Volumes.
	int getConvexVolumeCount() const { return m_volumeCount; }
	const ConvexVolume* getConvexVolumes() const { return m_volumes; }
	void addConvexVolume(const float* verts, const int nverts,
		const float minh, const float maxh, unsigned char area);
	void deleteConvexVolume(int i);
	bool raycastMesh(float* src, float* dst, float& tmin);
};