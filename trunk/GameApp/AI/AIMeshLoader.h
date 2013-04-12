#pragma once 

#include "../ObjLoader.h"

class rcMeshLoaderObj : public SceneNode
{
public:
	rcMeshLoaderObj();
	~rcMeshLoaderObj();

	CMeshLoader10 objLoader;
		
	inline const float* getVerts() const { return verts; }
	inline const float* getNormals() const { return m_normals; }
	inline const int* getTris() const { return tris; }
	inline int getVertCount() const { return nverts; }
	inline int getTriCount() const { return ntris; }
	inline const char* getFileName() const { return m_filename; }


	bool Load(const char* filename);
	bool Load(Mesh* pMesh);
	bool LoadTerrain();

	virtual bool CreateBuffers(void) ;
	virtual void Draw(void) ;
	bool LoadMesh(Mesh* pMesh);
	char m_filename[260];
	
	float* m_verts;
	int* m_tris;
	float* m_normals;
	int m_vertCount;
	int m_triCount;


	int ntris;//number of total triangles
	int *tris;//list of trinagles
	float *verts;//list of verticies
	int nverts;//number of verticies
	unsigned int numEnt;
};