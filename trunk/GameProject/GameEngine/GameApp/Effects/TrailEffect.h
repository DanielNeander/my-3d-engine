#pragma once 

#define NUM_QUADS 75

class SceneNode;
class Mesh;

struct PNTCVertex 
{
	noVec3 pos;
	//noVec3 normal;
	//noVec2 texcoord;
	noVec4 color;
};

class TrailMesh : public Mesh
{
public:
	TrailMesh();

	virtual bool CreateBuffers(void);
	virtual void Draw(void);

	void SamplePoints(float fTime);
	bool Init(SceneNode* pkScene, SceneNode* pkBottom, float fInitialTime );
protected:
	void UpdateData();

	noVec3 m_kVertices[NUM_QUADS * 2];
	float m_afVelocityScales[NUM_QUADS * 2];
	noVec2 m_kTexCoords[NUM_QUADS * 2];

	PNTCVertex m_kRenderVertices[NUM_QUADS * 2];

	int m_iStartingIndex;
	int m_iNumberOfQuads;

	//Values needed to calculate the velocity/opacity scaling.
	float m_fLastTime;
	noVec3 m_kLastPoint;
	float m_fVelocityMinimum;
	float m_fVelocityMaximum;
	int	m_kVerexStride;

	SceneNode* m_pkBottomObj;	
};
