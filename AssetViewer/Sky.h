#ifndef SKY_H
#define SKY_H


class CSky : public U2N2Mesh {

public:
	CSky(U2TriangleData *pData);

	HRESULT Init(int divisions = 10,
		float planetRadius = 100.0f,
		float atmosphereRadius = 120.0f,
		float hTileFactor = 1.0f,
		float vTileFactor = 1.0f);
	/*               IawVector pos = IawVector(0.0f),
	IawVector scale = IawVector(1.0f),
	IawVector rotationAxis = IawVector(0.0f,1.0f,0.0f),
	float rotationAngle = 0.0f,
	IawVector velocity = IawVector(0.0f),
	IawVector scaleVelocity = IawVector(0.0f),
	float rotationalVel = 0.0f,
	DWORD flags = 0);
	*/

private:	
	HRESULT SetupVertices(float *pVertices =NULL);
	HRESULT SetupIndices(uint32 uiNumFaces, uint32 uiNumIndices);

private:
	float mPlanetRadius;
	float mAtmosphereRadius;
	float mHTileFactor;
	float mVTileFactor;

};


#endif