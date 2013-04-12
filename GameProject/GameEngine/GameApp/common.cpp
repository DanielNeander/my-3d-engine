#include "stdafx.h"
#include "EngineCore/Util/GameLog.h"
#include "GameApp/GameApp.h"
#include "Mesh.h"
#include "SceneNode.h"
#include "M2Mesh.h"
#include "Mesh_D3D11.h"
#include "SDKMeshLoader.h"
#include "ModelLib/M2Loader.h"
#include "Recast.h"
#include "ObjLoader.h"
#include "Common.h"


extern Renderer* gRenderer;
//--------------------------------------------------------------------------------------
// Given a ray origin (orig) and direction (dir), and three vertices of a triangle, this
// function returns TRUE and the interpolated texture coordinates if the ray intersects 
// the triangle
//--------------------------------------------------------------------------------------
bool IntersectTriangle( const noVec3& orig, const noVec3& dir,
	noVec3& v0, noVec3& v1, noVec3& v2,
	FLOAT* t, FLOAT* u, FLOAT* v )
{
	// Find vectors for two edges sharing vert0
	noVec3 edge1 = v1 - v0;
	noVec3 edge2 = v2 - v0;

	// Begin calculating determinant - also used to calculate U parameter
	noVec3 pvec;
	pvec = dir.Cross( edge2 );

	// If determinant is near zero, ray lies in plane of triangle
	FLOAT det = edge1 * ( pvec );

	noVec3 tvec;
	if( det > 0 )
	{
		tvec = orig - v0;
	}
	else
	{
		tvec = v0 - orig;
		det = -det;
	}

	if( det < 0.0001f )
		return FALSE;

	// Calculate U parameter and test bounds
	*u = tvec * pvec ;
	if( *u < 0.0f || *u > det )
		return FALSE;

	// Prepare to test V parameter
	noVec3 qvec;
	qvec = tvec.Cross( edge1 );

	// Calculate V parameter and test bounds
	*v = dir * qvec ;
	if( *v < 0.0f || *u + *v > det )
		return FALSE;

	// Calculate t, scale parameters, ray intersects triangle
	*t = edge2 * qvec ;
	FLOAT fInvDet = 1.0f / det;
	*t *= fInvDet;
	*u *= fInvDet;
	*v *= fInvDet;

	return TRUE;
}

int IntersectTriangle( const noVec3& orig, const noVec3& dir, noVec3& v0, noVec3& v1, noVec3& v2,
	noVec3* l, float* fDist)
{
	noVec3    u, v, n;              // triangle vectors
	noVec3    w0, w;           // ray vectors
	float     r, a, b;              // params to calc ray-plane intersect

	// get triangle edge vectors and plane normal
	u = v1 - v0;
	v = v2 - v0;
	n = u.Cross(v);              // cross product
	if (n == vec3_zero)             // triangle is degenerate
		return -1;                  // do not deal with this case
		
	w0 = orig - v0;
	a = -(n * w0);
	b = n * dir;
	if (noMath::Fabs(b) < SMALL_NUMBER) {     // ray is  parallel to triangle plane
		if (a == 0)                 // ray lies in triangle plane
			return 2;
		else return 0;              // ray disjoint from plane
	}

	// get intersect point of ray with triangle plane
	r = a / b;
	if (r < 0.0)                    // ray goes away from triangle
		return 0;                   // => no intersect
	// for a segment, also test if (r > 1.0) => no intersect

	*l= orig + r * dir;            // intersect point of ray and plane
	*fDist = r;

	// is I inside T?
	float    uu, uv, vv, wu, wv, D;
	uu = u * u;
	uv = u * v;
	vv = v * v;
	w = *l - v0;
	wu = w * u;
	wv = w * v;
	D = uv * uv - uu * vv;

	// get and test parametric coords
	float s, t;
	s = (uv * wv - vv * wu) / D;
	if (s < 0.0 || s > 1.0)         // I is outside T
		return 0;
	t = (uv * wu - uu * wv) / D;
	if (t < 0.0 || (s + t) > 1.0)  // I is outside T
		return 0;

	return 1;                       // I is in T
}



bool IntersectTriangle(const noVec3& kOrigin, 
	const noVec3& kDir, const noVec3& kV1, const noVec3& kV2, 
	const noVec3& kV3, bool bCull, noVec3& kIntersect, float& r, float& s, 
	float& t)
{
	// All input quantities are in model space of the NiMesh object.
	// Input:
	//     ray (kOrigin+T*kDir), 
	//     triangle vertices (kV1, kV2, kV3)
	//     backface culling is performed when bCull=true
	// Return value:  true iff the ray intersects the triangle
	// Output (valid when return value is true):
	//     intersection of ray and triangle (kIntersect)
	//     kIntersect = kOrigin+r*kDir
	//     kIntersect = kV1+s*(kV2-kV1)+t*(kV3-kV1)

	// From "Real-Time Rendering" which references Moller, Tomas & Trumbore, 
	// "Fast, Minimum Storage Ray-Triangle Intersection", Journal of Graphics 
	// Tools, vol. 2, no. 1, pp 21-28, 1997. With some modifications to the 
	// non-culling case by Michael Mounier.

	const float fTolerance = 1e-05f;

	noVec3 kEdge1 = kV2 - kV1;
	noVec3 kEdge2 = kV3 - kV1;

	noVec3 kPt = kDir.Cross(kEdge2);

	float fDet = kEdge1 * kPt;
	if (fDet > fTolerance) // Determinant is positive.
	{
		noVec3 kS = kOrigin - kV1;
		s = kS * kPt;

		if (s < 0.0f || s > fDet)
			return false;

		noVec3 q = kS.Cross(kEdge1);
		t = kDir * q;

		if (t < 0.0f || s + t > fDet)
			return false;

		r = kEdge2 * q;

		if (r < 0.0f)
			return false;
	}
	else if (fDet <= -fTolerance && !bCull) // Determinant is negative.
	{
		LOG_ERR << "fDet : " << fDet;

		noVec3 kS = kOrigin - kV1;
		s = kS * kPt;

		if (s > 0.0f || s < fDet)
		{
			LOG_ERR << "s : " << s;
			return false;
		}

		noVec3 q = kS.Cross(kEdge1);
		t = kDir * q;

		if (t > 0.0f || s + t < fDet)
		{
			LOG_ERR << "t :" << t <<" s + t : " <<  s+ t;
			return false;
		}

		r = kEdge2 * q;

		if (r > 0.0f)
		{
			LOG_ERR << "r : " << r;
			return false;
		}
	}
	else    // Parallel ray or culled.
	{
		LOG_ERR << "Parallel";
		return false;
	}

	float inv_det = 1.0f / fDet;

	s *= inv_det;
	t *= inv_det;
	r *= inv_det;

	LOG_INFO << "Pick Success r : " << r;

	kIntersect = kOrigin + r * kDir;

	return true;
}

struct PNCVertex {
	noVec3 pos;
	noVec3 norm;	
	noVec3 color;
};

struct GridVertex {
	noVec3 pos;
	noVec3 norm;
	noVec3 tangent;
	noVec3 binormal;
	noVec2 uv;
	//noVec4 color;
};
struct GridVertex2 {
	noVec3 pos;
	noVec3 norm;
	noVec2 uv;
	noVec4 color;
};

//
// given ray from pos in direction dir
// returns (t,u,v) where t is distance to the plane and u,v are 
// the coords within the triangle
// so the point of intersection will be v0 + u*(v1-v0) + v*(v2-v0)
//
// (from:	Fast,Minimum Storage Ray-Triangle Intersection
//			Journal of Graphics Tools, 2(1):21-28, 1997 )
#define HKG_TRIANGLE_EPSILON 0.000001f
bool triangleIntersect( const noVec3& kOrigin, 
	const noVec3& kDir, const noVec3& v0, const noVec3& v1, 	const noVec3& v2,
	float &t, float &u, float &v) 
{
	float det,inv_det;
	noVec3 edge1 = v1 - v0;
	noVec3 edge2 = v2 - v0;

	/* begin calculating determinant - also used to calculate U parameter */
	noVec3 pvec;
	pvec = kDir.Cross(edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = edge1 * pvec;

	if (det < HKG_TRIANGLE_EPSILON)
		return false;
	noVec3 tvec;

	/* calculate distance from vert0 to ray origin */   
	tvec = kOrigin - v0;
	/* calculate U parameter and test bounds */   	
	u = tvec * pvec;
	if (u < 0.0f || u > det)      
		return false;   
	noVec3 qvec;
	qvec = tvec.Cross(edge1);

	/* calculate V parameter and test bounds */
	v = kDir * qvec;

	if (v < 0.0f || (u + v) > det)      
		return false;

	/* calculate t, scale parameters, ray intersects triangle */
	t = edge2 * qvec;

	inv_det = 1.0f / det;   
	t *= inv_det;
	u *= inv_det; 
	v *= inv_det;

	return true;
}


Mesh* makeEditGrid(RenderData& rd, const noVec3& corner, const noVec3& sideA, const noVec3& sideB, int resA /*= 1*/, int resB /*= 1*/, int numTexCoords /*= 1*/, bool vcolor /*= false*/, bool tangents /*= false*/ )
{
	int rowsX = resA +1;
	int colsY = resB +1;

	int numVertices = rowsX * colsY; 
	int numIndices = (2*rowsX) * resB;

	noVec3 normal;
	normal = sideB.Cross(sideA);
	normal.Normalize();

	GridVertex2* pVerts = new GridVertex2[numVertices];
	uint16 *pIndices = new uint16[numIndices];

	Mesh* pMesh = new Mesh_D3D11;
	pMesh->m_PrimitiveType = PRIM_TRIANGLE_STRIP;

	pMesh->m_pVertices = (float *)pVerts;
	pMesh->m_pIndices	= pIndices;
	pMesh->m_iNumVertices = numVertices;
	pMesh->m_iNumTris = numIndices - 2;
	pMesh->m_iVertexSize = sizeof(GridVertex2);



	noVec2 tempTex;
	noVec4 tempColor4f(1, 1, 1, 1 );

	int i, j;
	int curVertIndex = 0;
	for (i = 0; i < colsY; i++)
	{
		noVec3 vy;
		vy =  sideB * ( i/(float)(colsY-1) );
		vy += corner;

		for (j = 0; j < rowsX; j++)
		{
			noVec3 vxy;
			vxy =  sideA * ( j/(float)(rowsX-1) );
			vxy += vy ;

			pVerts[curVertIndex].pos = vxy;
			pVerts[curVertIndex].norm = normal;

			if (numTexCoords)
			{
				tempTex[0] = j/(float)rowsX; 
				tempTex[1] = i/(float)colsY; 
				for (int tc=0; tc < numTexCoords; ++tc)
				{
					pVerts[curVertIndex].uv = tempTex;
					//m_vertices->setVertexComponentData( HKG_VERTEX_COMPONENT(HKG_VERTEX_COMPONENT_TEX0 + tc), curVertIndex, tempTex);	
				}
			}
			pVerts[curVertIndex].color = colorCyan;
			curVertIndex++;
		}
	}

	pMesh->CalculateOOBB();
	pMesh->CalculateAABB();

	unsigned short* conn = pIndices;

	bool dirUp = true;
	int c = 0;

	// fill strip info
	for (i = 0; i < resB; i++) // res not cols
	{
		int start0 = rowsX*i;
		int start1 = rowsX*(i+1);

		if (dirUp)
		{
			for (j = 0; j< rowsX; j++) // rows not res
			{
				conn[c++] = (unsigned short)( start0 + j );
				conn[c++] = (unsigned short)( start1 + j );
			}
		}
		else 
		{
			for (j = rowsX -1; j>= 0; j--) // rows not res
			{
				conn[c++] = (unsigned short)( start1 + j );
				conn[c++] = (unsigned short)( start0 + j );
			}
		}	

		dirUp = !dirUp;
	}

	//if (tangents)
	//	computeTangents(0);		
	FormatDesc vbFmt[] = {
		0, TYPE_VERTEX, FORMAT_FLOAT,  3,
		0, TYPE_NORMAL, FORMAT_FLOAT,   3,		
		0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,					
		0, TYPE_COLOR, FORMAT_FLOAT, 4,				
	};

	
	rd.shader = gRenderer->addShader("Data/Shaders/SimpleColor.hlsl");

	rd.vf = gRenderer->addVertexFormat(vbFmt, elementsOf(vbFmt), rd.shader);

	rd.vb = gRenderer->addVertexBuffer(sizeof(GridVertex2) * numVertices	, STATIC, pVerts);
	rd.ib = gRenderer->addIndexBuffer(numIndices, sizeof(WORD), STATIC, pIndices);

	//rd.baseTex = gRenderer->addTexture("Terrain/ground/sand.dds", true);
	//rd.diffuseSampler = gRenderer->addSamplerState(LINEAR, WRAP, WRAP, WRAP);


	rd.startIndex = 0;
	rd.endIndex = numIndices;
	rd.vertexStart = 0;
	rd.vertexEnd = numVertices;
	rd.cull = gRenderer->addRasterizerState(CULL_NONE, WIREFRAME);
	rd.depthMode = gRenderer->addDepthState(true, false);
	rd.prim = PRIM_TRIANGLE_STRIP;	

	return pMesh;
}



Mesh* makeGrid(RenderData& rd, const noVec3& corner, const noVec3& sideA, const noVec3& sideB, int resA /*= 1*/, int resB /*= 1*/, int numTexCoords /*= 1*/, bool vcolor /*= false*/, bool tangents /*= false*/ )
{
	int rowsX = resA +1;
	int colsY = resB +1;

	int numVertices = rowsX * colsY; 
	int numIndices = (2*rowsX) * resB;

	noVec3 normal;
	normal = sideB.Cross(sideA);
	normal.Normalize();
		
	GridVertex* pVerts = new GridVertex[numVertices];
	uint16 *pIndices = new uint16[numIndices];

	Mesh* pMesh = new Mesh_D3D11;
	pMesh->m_PrimitiveType = PRIM_TRIANGLE_STRIP;
	
	pMesh->m_pVertices = (float *)pVerts;
	pMesh->m_pIndices	= pIndices;
	pMesh->m_iNumVertices = numVertices;
	pMesh->m_iNumTris = numIndices - 2;
	pMesh->m_iVertexSize = sizeof(GridVertex);
	
			

	noVec2 tempTex;
	noVec4 tempColor4f(1, 1, 1, 1 );
	
	int i, j;
	int curVertIndex = 0;
	for (i = 0; i < colsY; i++)
	{
		noVec3 vy;
		vy =  sideB * ( i/(float)(colsY-1) );
		vy += corner;

		for (j = 0; j < rowsX; j++)
		{
			noVec3 vxy;
			vxy =  sideA * ( j/(float)(rowsX-1) );
			vxy += vy ;

			pVerts[curVertIndex].pos = vxy;
			pVerts[curVertIndex].norm = normal;

			if (numTexCoords)
			{
				tempTex[0] = j/(float)rowsX; 
				tempTex[1] = i/(float)colsY; 
				for (int tc=0; tc < numTexCoords; ++tc)
				{
					pVerts[curVertIndex].uv = tempTex;
					//m_vertices->setVertexComponentData( HKG_VERTEX_COMPONENT(HKG_VERTEX_COMPONENT_TEX0 + tc), curVertIndex, tempTex);	
				}
			}
			//pVerts[curVertIndex].color = tempColor4f;
			curVertIndex++;
		}
	}

	pMesh->CalculateOOBB();
	pMesh->CalculateAABB();

	unsigned short* conn = pIndices;

	bool dirUp = true;
	int c = 0;

	// fill strip info
	for (i = 0; i < resB; i++) // res not cols
	{
		int start0 = rowsX*i;
		int start1 = rowsX*(i+1);

		if (dirUp)
		{
			for (j = 0; j< rowsX; j++) // rows not res
			{
				conn[c++] = (unsigned short)( start0 + j );
				conn[c++] = (unsigned short)( start1 + j );
			}
		}
		else 
		{
			for (j = rowsX -1; j>= 0; j--) // rows not res
			{
				conn[c++] = (unsigned short)( start1 + j );
				conn[c++] = (unsigned short)( start0 + j );
			}
		}	

		dirUp = !dirUp;
	}

	//if (tangents)
	//	computeTangents(0);		
	FormatDesc vbFmt[] = {
		0, TYPE_VERTEX, FORMAT_FLOAT,  3,
		0, TYPE_NORMAL, FORMAT_FLOAT,   3,
		0, TYPE_TANGENT, FORMAT_FLOAT, 3,
		0, TYPE_BINORMAL,FORMAT_FLOAT, 3,
		0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,					
		//0, TYPE_COLOR, FORMAT_FLOAT, 4,				
	};

	switch (AppSettings::ShadowModeEnum)
	{
	case SHADOW_NONE:
		rd.shader =  gColor;
	case SHADOW_PSSM:
		rd.shader = gColorPssm;			
	case SHADOW_SSAO:
		rd.shader = gColorSsao;			
	}		
	//rd.shader = gRenderer->addShader("Data/Shaders/BlinnPhong.hlsl");
	
	rd.vf = gRenderer->addVertexFormat(vbFmt, elementsOf(vbFmt), rd.shader);

	rd.vb = gRenderer->addVertexBuffer(sizeof(GridVertex) * numVertices	, STATIC, pVerts);
	rd.ib = gRenderer->addIndexBuffer(numIndices, sizeof(WORD), STATIC, pIndices);
		
	rd.baseTex = gRenderer->addTexture("Terrain/ground/sand.dds", true);
	rd.diffuseSampler = gRenderer->addSamplerState(LINEAR, WRAP, WRAP, WRAP);
	
		
	rd.startIndex = 0;
	rd.endIndex = numIndices;
	rd.vertexStart = 0;
	rd.vertexEnd = numVertices;
	//rd.cull = gRenderer->addRasterizerState(CULL_BACK);
	rd.depthMode = gRenderer->addDepthState(true, true);
	rd.prim = PRIM_TRIANGLE_STRIP;	
	

	return pMesh;
}



class Mesh* makeMesh( RenderData& rd, const char* file )
{
	Mesh* pMesh = new Mesh_D3D11;
	pMesh->LoadFromLWO(file);
	pMesh->m_PrimitiveType = PRIM_TRIANGLES;
	pMesh->CalculateAABB();
	
	FormatDesc vbFmt[] = {
		0, TYPE_VERTEX, FORMAT_FLOAT,  3,		
		0, TYPE_NORMAL, FORMAT_FLOAT,  3,		
		0, TYPE_COLOR, FORMAT_FLOAT, 3,				
	};

	switch (AppSettings::ShadowModeEnum)
	{
	case SHADOW_NONE:
		rd.shader =  gColor;		
	case SHADOW_PSSM:
		rd.shader = gColorPssm2;					
	case SHADOW_SSAO:
		rd.shader = gColorSsao;			
	}		
		
	rd.vf = gRenderer->addVertexFormat(vbFmt, elementsOf(vbFmt), rd.shader);

	rd.vb = gRenderer->addVertexBuffer(sizeof(PNCVertex) * pMesh->m_iNumVertices	, STATIC, pMesh->m_pVertices);
	rd.ib = gRenderer->addIndexBuffer(pMesh->m_iNumTris * 3, sizeof(WORD), STATIC, pMesh->m_pIndices);

	rd.startIndex = 0;
	rd.endIndex = pMesh->m_iNumTris * 3;
	rd.vertexStart = 0;
	rd.vertexEnd = pMesh->m_iNumVertices;	
	rd.cull = gRenderer->addRasterizerState(CULL_BACK, SOLID);
	rd.depthMode = gRenderer->addDepthState(true, true);	
	rd.prim = PRIM_TRIANGLES;	

	return pMesh;
}


struct WireBoxVertex {
	noVec3 pos;
	noVec4 col;
};

class Mesh* makeMesh( RenderData& rd, noVec3* verts, uint32 numVerts, uint16* indices, uint32 numIndices, noVec4 col )
{
	WireBoxVertex* pVerts = new WireBoxVertex[numVerts];
	uint16 *pIndices = new uint16[numIndices];

	Mesh* pMesh = new Mesh_D3D11;
	pMesh->m_PrimitiveType = PRIM_TRIANGLES;

	for (int i = 0; i < numVerts; ++i)
	{
		pVerts[i].pos = verts[i];
		pVerts[i].col = col;
	}

	memcpy(pIndices, indices, numIndices * sizeof(uint16));

	pMesh->m_pVertices = (float *)pVerts;
	pMesh->m_pIndices	= indices;
	pMesh->m_iNumVertices = numVerts;
	pMesh->m_iNumTris = numIndices / 3;
	pMesh->m_iVertexSize = sizeof(WireBoxVertex);

	FormatDesc vbFmt[] = {
		0, TYPE_VERTEX, FORMAT_FLOAT,  3,		
		0, TYPE_COLOR, FORMAT_FLOAT, 4,				
	};

	rd.shader = gRenderer->addShader("Data/Shaders/WireColor.hlsl");
	rd.vf = gRenderer->addVertexFormat(vbFmt, elementsOf(vbFmt), rd.shader);

	rd.vb = gRenderer->addVertexBuffer(sizeof(WireBoxVertex) * numVerts	, STATIC, pVerts);
	rd.ib = gRenderer->addIndexBuffer(numIndices, sizeof(WORD), STATIC, pIndices);

	rd.startIndex = 0;
	rd.endIndex = numIndices;
	rd.vertexStart = 0;
	rd.vertexEnd = numVerts;	
	rd.cull = gRenderer->addRasterizerState(CULL_BACK, WIREFRAME);
	rd.depthMode = gRenderer->addDepthState(true, true);	
	rd.prim = PRIM_TRIANGLES;	
	
	return pMesh;
}

#define MAX_INTERSECTIONS 16
PickData g_IntersectionArray[MAX_INTERSECTIONS];
int g_nNumIntersections = 0;
bool g_bAllHits = true;

PickData PickUtil::ms_pickData;

bool PickUtil::Intersect( CDXUTSDKMesh* pMesh, const noVec3& vPickRayOrig, const noVec3& vPickRayDir )
{
	bool bSuccess = false;	

	noVec3 kModelOrigin, kModelDir;
	ConvertRayFromWorldToLocal(pMesh->worldTM_, vPickRayOrig, vPickRayDir, kModelOrigin, kModelDir);

	for( UINT i =0; i < pMesh->GetNumMeshes( ); ++i ) 
	{
		DWORD dwNumFaces = (DWORD)pMesh->GetNumIndices(i) / 3;

		uint16* pIndices = (uint16*)pMesh->GetRawIndicesAt(i);
		noVec3* pVertices = (noVec3*)pMesh->GetRawVerticesAt(i);
		
		FLOAT fBary1, fBary2;
		FLOAT fDist;
		uint16* iThisTri;
		noVec3 v0, v1, v2;

		ms_pickData.vert = noVec3(noMath::INFINITY, noMath::INFINITY, noMath::INFINITY);
		g_nNumIntersections = 0;

		for( DWORD j = 0; j < dwNumFaces; j++ )
		{
			/*noVec3 v0 = pMesh->worldTM_ * pVertices[pIndices[3 * j + 0]];
			noVec3 v1 = pMesh->worldTM_ * pVertices[pIndices[3 * j + 1]];
			noVec3 v2 = pMesh->worldTM_ * pVertices[pIndices[3 * j + 2]];*/
			noVec3 v0 = pVertices[pIndices[3 * j + 0]];
			noVec3 v1 = pVertices[pIndices[3 * j + 1]];
			noVec3 v2 = pVertices[pIndices[3 * j + 2]];
			
			// Check if the pick ray passes through this point
			noVec3 kIntersect; 
			float fLineParam, fTriParam1, fTriParam2;		
			//if( IntersectTriangle( vPickRayOrig, vPickRayDir, v0, v1, v2, &kIntersect, &fDist) == 1)
			//if (rayIntersectsTriangle( kModelOrigin.ToFloatPtr(), kModelDir.ToFloatPtr(), 
			//	v0.ToFloatPtr(), v1.ToFloatPtr(), v2.ToFloatPtr(), fDist))		
			if (IntersectTriangle(kModelOrigin, kModelDir, v0, v1, v2, false, kIntersect, 
				fLineParam, fTriParam1, fTriParam2))
				//if (triangleIntersect(kModelOrigin, kModelDir, v0, v1, v2,  
				//	fLineParam, fTriParam1, fTriParam2))
			{
				kIntersect = kModelOrigin + fLineParam * kModelDir;
				fDist = fLineParam * pMesh->worldTM_.scale_;

				bSuccess = true;
				if( g_bAllHits || g_nNumIntersections == 0 || abs(fDist) < abs(g_IntersectionArray[0].fDist) )
				{
					if( !g_bAllHits )
						g_nNumIntersections = 0;								
					g_IntersectionArray[g_nNumIntersections].dwFace = i;
					//g_IntersectionArray[g_nNumIntersections].fBary1 = fBary1;
					//g_IntersectionArray[g_nNumIntersections].fBary2 = fBary2;
					g_IntersectionArray[g_nNumIntersections].fDist = fDist;
					//iThisTri = &pIndices[3 * i];
					// World ÁÂÇ¥(Local ÁÂÇ¥ÀÏ½Ã´Â kIntersect¸¸ ¹ÝÈ¯)
					ms_pickData.vert =  pMesh->worldTM_.scale_ * (pMesh->worldTM_.rotate_ *	kIntersect)
						+ pMesh->worldTM_.trans_; // vPickRayOrig + vPickRayDir * fDist;				
					//ms_pickData.vert = vPickRayOrig + vPickRayDir * fDist;
					g_nNumIntersections++;
					if( g_nNumIntersections == MAX_INTERSECTIONS )
						break;
				}
			}		
		}
		if( g_nNumIntersections == MAX_INTERSECTIONS )
			break;
	}
	return bSuccess;
}

bool PickUtil::FindBoundIntersect( const BoundingBox& kWorldBB, const noVec3& kOrigin, const noVec3& kDir) {
	// Determine if ray intersects bounding sphere of object. If sphere is
	// (X-C)*(X-C) = R^2 and ray is X = t*D+L for t >= 0, then intersection
	// is obtained by plugging X into sphere equation to get quadratic
	// (D*D)t^2 + 2*(D*(L-C))t + (L-C)*(L-C)-R^2 = 0.  Define a = D*D, 
	// b = D*(L-C), and c = (L-C)*(L-C)-R^2.  Intersection occurs whenever
	// b*b >= a*c (quadratic has at least one real root).  This is clearly
	// true if c <= 0, so that test is made for a quick out.  If c > 0 and
	// b >= 0, then the sphere center is "behind" the ray (so to speak) and
	// the sphere cannot intersect the ray. Otherwise, b*b >= a*c is checked.

	noVec3 kDiff = kOrigin - kWorldBB.GetCenter();
	float fRadius = kWorldBB.BoundingRadiusFromAABB();
	float c = kDiff * kDiff - fRadius * fRadius;
	if (c <= 0.0f)
		return true;

	float b = kDir * kDiff;
	if (b >= 0.0f)
		return false;

	float a = kDir * kDir;
	return (b * b >= a * c);
}

bool PickUtil::Intersect( Mesh* pMesh, const noVec3& vPickRayOrig, const noVec3& vPickRayDir )
{	
	bool bSuccess = false;
		
	DWORD dwNumFaces = (DWORD)pMesh->GetFaceCount();

	uint16* pIndices = pMesh->GetIndices();
	float* pVertices = pMesh->GetVertices();

	noVec3 kModelOrigin, kModelDir;
	ConvertRayFromWorldToLocal(pMesh->worldTM_, vPickRayOrig, vPickRayDir, kModelOrigin, kModelDir);
		
	FLOAT fBary1, fBary2;
	FLOAT fDist;
	uint16* iThisTri;
	noVec3 v0, v1, v2;

	ms_pickData.vert = noVec3(noMath::INFINITY, noMath::INFINITY, noMath::INFINITY);
	g_nNumIntersections = 0;
		
	for( DWORD i = 0; i < dwNumFaces; i++ )
	{
		pMesh->GetTriangle(i, v0, v1, v2);

		
		/*v0 = pMesh->worldTM_ * v0;
		v1 = pMesh->worldTM_ * v1;
		v2 = pMesh->worldTM_ * v2;*/
		
		// Check if the pick ray passes through this point
		//if( IntersectTriangle( vPickRayOrig, vPickRayDir, v0, v1, v2,
		//	&fDist, &fBary1, &fBary2 ) )		
		noVec3 kIntersect; 
		float fLineParam, fTriParam1, fTriParam2;		
		//if( IntersectTriangle( vPickRayOrig, vPickRayDir, v0, v1, v2, &kIntersect, &fDist) == 1)
		//if (rayIntersectsTriangle( kModelOrigin.ToFloatPtr(), kModelDir.ToFloatPtr(), 
		//	v0.ToFloatPtr(), v1.ToFloatPtr(), v2.ToFloatPtr(), fDist))		
		if (IntersectTriangle(kModelOrigin, kModelDir, v0, v1, v2, false, kIntersect, 
			fLineParam, fTriParam1, fTriParam2))
		//if (triangleIntersect(kModelOrigin, kModelDir, v0, v1, v2,  
		//	fLineParam, fTriParam1, fTriParam2))
		{
			kIntersect = kModelOrigin + fLineParam * kModelDir;
			fDist = fLineParam * pMesh->worldTM_.scale_;

			bSuccess = true;
			if( g_bAllHits || g_nNumIntersections == 0 || abs(fDist) < abs(g_IntersectionArray[0].fDist) )
			{
				if( !g_bAllHits )
					g_nNumIntersections = 0;								
				g_IntersectionArray[g_nNumIntersections].dwFace = i;
				//g_IntersectionArray[g_nNumIntersections].fBary1 = fBary1;
				//g_IntersectionArray[g_nNumIntersections].fBary2 = fBary2;
				g_IntersectionArray[g_nNumIntersections].fDist = fDist;
				//iThisTri = &pIndices[3 * i];
				// World ÁÂÇ¥(Local ÁÂÇ¥ÀÏ½Ã´Â kIntersect¸¸ ¹ÝÈ¯)
				ms_pickData.vert =  pMesh->worldTM_.scale_ * (pMesh->worldTM_.rotate_ *	kIntersect)
					 + pMesh->worldTM_.trans_; // vPickRayOrig + vPickRayDir * fDist;				
				//ms_pickData.vert = vPickRayOrig + vPickRayDir * fDist;
				g_nNumIntersections++;
				if( g_nNumIntersections == MAX_INTERSECTIONS )
					break;
			}
		}		
	}

	return bSuccess;
}


bool PickUtil::Intersect( CMeshLoader10* pMesh, const noVec3& vPickRayOrig, const noVec3& vPickRayDir )
{
	bool bSuccess = false;
	
	DWORD dwNumFaces = (DWORD)pMesh->GetIndexCount()/3;

	DWORD* pIndices = (DWORD*)pMesh->GetIndices();
	const VERTEX* pVertices =  pMesh->GetVertices();

	FLOAT fBary1, fBary2;
	FLOAT fDist;
	DWORD* iThisTri;

	ms_pickData.vert = noVec3(noMath::INFINITY, noMath::INFINITY, noMath::INFINITY);
	g_nNumIntersections = 0;

	float MinDist = 1.0f;
	for( DWORD i = 0; i < dwNumFaces; i++ )
	{
		noVec3 v0 = pMesh->worldTM_ * pVertices[pIndices[3 * i + 0]].position;
		noVec3 v1 = pMesh->worldTM_ * pVertices[pIndices[3 * i + 1]].position;
		noVec3 v2 = pMesh->worldTM_ * pVertices[pIndices[3 * i + 2]].position;

		// Check if the pick ray passes through this point
		if( IntersectTriangle( vPickRayOrig, vPickRayDir, v0, v1, v2,
			&fDist, &fBary1, &fBary2 ) )		
		{
			bSuccess = true;
			if( g_bAllHits || g_nNumIntersections == 0 || abs(fDist) < abs(g_IntersectionArray[0].fDist) )			
			{				
				if( !g_bAllHits )
					g_nNumIntersections = 0;				
				ms_pickData = g_IntersectionArray[g_nNumIntersections];
				g_IntersectionArray[g_nNumIntersections].dwFace = i;
				g_IntersectionArray[g_nNumIntersections].fBary1 = fBary1;
				g_IntersectionArray[g_nNumIntersections].fBary2 = fBary2;
				g_IntersectionArray[g_nNumIntersections].fDist = fDist;
				iThisTri = &pIndices[3 * i];
				ms_pickData.vert = vPickRayOrig + vPickRayDir * fDist;												
				//pickDatas.push_back(g_IntersectionArray[g_nNumIntersections]);
				g_nNumIntersections++;
				if( g_nNumIntersections == MAX_INTERSECTIONS )
					break;				
			}
		}

	}
	return bSuccess;
}

void PickUtil::Clear()
{
	pickDatas.clear(); 
	ms_pickData.dwFace = -1;
	ms_pickData.fBary1 = -1.0f;
	ms_pickData.fBary2 = -1.0f;
	ms_pickData.fDist = FLT_MAX;
	ms_pickData.vert = noVec3(FLT_MAX, FLT_MAX, FLT_MAX);
	ms_pickData.pUserData = NULL;
}

bool PickUtil::Picking( SceneNode* RootNode )
{
	PickUtil::Clear();
	noVec3 from;
	noVec3 dir;

	D3DXVECTOR3 vorig, vdir;
	GetApp()->GetRay(vdir, vorig);


	GetApp()->rayOrig_ = noVec3(vorig.x, vorig.y, vorig.z);

	//pcamera_->unProject(x, y, 0.0f, getWidth(), getHeight(), from);
	//pcamera_->unProject(x, y, 1.0f, getWidth(), getHeight(), to);
	from = noVec3(vorig.x , vorig.y, vorig.z);
	dir = noVec3(vdir.x, vdir.y, vdir.z) ;	

	GetApp()->pickPos_ = GetApp()->rayOrig_ + 1000 * dir;
	//dir = rayOrig_ + 1000 * dir;;

	if(GetApp()->Pick(RootNode, from, dir))
	{
		if (!pickDatas.empty())
		{
			std::sort(pickDatas.begin(), pickDatas.end(), PickSort());
			for (size_t i=0; i < pickDatas.size(); ++i)
				LOG_INFO << "Pick : " << i << " Pick Dist : " << pickDatas[i].fDist;
		}
		return true;
	}

	return false;
}

void PickUtil::ConvertRayFromWorldToLocal( const noTransform& kWorld, const noVec3& kWorldOrigin, const noVec3& kWordDir, noVec3& kLocalOrig, noVec3& klocalDir ) {
	noVec3 kWorldDiff = kWorldOrigin - kWorld.trans_;
	float fInvWorldScale = 1.0f/ kWorld.scale_;
	kLocalOrig = (kWorldDiff * kWorld.rotate_) * fInvWorldScale;
	klocalDir = (kWordDir * kWorld.rotate_);
}

std::vector<PickData> PickUtil::pickDatas;

