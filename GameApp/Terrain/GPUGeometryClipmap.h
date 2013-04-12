/*
?Felgueiras Filipe
fsnt@msn.com

This software is a computer program whose purpose is to show an 
implementation of GPU Geometry Clipmaps and it's features described 
in "Terrain rendering using GPU-based geometry clipmaps" paper by 
A. Asirvatham, H. Hoppe.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/

/*
	You will find :
	- View Frustum Culling
	- Instancing
	- Deactivation of levels for which the grid extent is smaller than 2.5h
	- Only rendered levels are updated

	You will not find :
	- use of tensor-product
	- compression, PTC codec, etc...
	- LoadTerrainFromBMP function 

	Know issues :
	- Frustum Culling floating point precision problem if you move far far far away from origin will produce wrong cull
	- if you want to use N > 255, you must change INDEX16 bits to INDEX32 bits and index buffer filling 
	  when creating geometry
	- i just added the "update of rendered levels only", don't know if it's 100% safe yet. 

	Questions, Errors, Optimisations, ...
	fsnt@msn.com
*/

#pragma once

// Grad & Perm lookup texture for GPU noise
VOID WINAPI FillPermTexture (noVec4* pOut, const noVec2* pTexCoord, 
							 const noVec2* pTexelClipSize, LPVOID pData)
{
	char value = (char) perm[( static_cast<int>(pTexCoord->y * 255.0f) + perm[ static_cast<int>(pTexCoord->x * 255.0f) ]) & 0xFF];

	*pOut = noVec4(value/255.0f,
		(grad3[value & 0x0F][0] * 64 + 64)/255.0f,
		(grad3[value & 0x0F][1] * 64 + 64)/255.0f,
		(grad3[value & 0x0F][2] * 64 + 64)/255.0f);
}

VOID WINAPI FillGradTexture (noVec4* pOut, const noVec2* pTexCoord, 
							 const noVec2* pTexelClipSize, LPVOID pData)
{
	char value = (char) perm[(static_cast<int>(pTexCoord->y * 255.0f) + perm[static_cast<int>(pTexCoord->x * 255.0f)]) & 0xFF];

	*pOut = noVec4( (grad4[value & 0x1F][3] * 64 + 64)/255.0f,
		(grad4[value & 0x1F][0] * 64 + 64)/255.0f,
		(grad4[value & 0x1F][1] * 64 + 64)/255.0f,
		(grad4[value & 0x1F][2] * 64 + 64)/255.0f);

}

struct VECTOR2
{
	short x;
	short y;
};

struct Clipmap
{
	LPDIRECT3DTEXTURE9      ElevationMap;		// Elevation data
	LPDIRECT3DTEXTURE9      NormalMap;		    // Normal data 

	CComPtr<ID3D11ShaderResourceView> m_ptex2DElevDataSRV;
	CComPtr<ID3D11RenderTargetView> m_ptex2DElevDataRTV;
	CComPtr<ID3D11ShaderResourceView> m_ptex2DNormalMapSRV;
	CComPtr<ID3D11RenderTargetView> m_ptex2DNormalMapRTV;
	
	ID3D11ShaderResourceView* GetElevDataSRV()const;
	ID3D11RenderTargetView*   GetElevDataRTV()const;
	ID3D11ShaderResourceView* GetNormalMapSRV()const;
	ID3D11RenderTargetView*   GetNormalMapRTV()const;

	noVec4             VecScale; 			// Scale (x,y) & translate (z,w) vertices
	noVec2             TexScale,			// translate (x,y) tex coordinates, use to generate terrain
							LastTexScale,		
							AABBDiff;			// AABB size

	VECTOR2				    TexTorusOrigin,     // Bottom Right Corner of the terrain on the ElevationMap (u, v)
							LastTexTorusOrigin;

	float                   VecShift;			// Grid unit of (level + 1), use to update vertices translation

	short                   Hpos,				// L-Shape
							Vpos;				// L-Shape

	DWORD					NumInstances;		// Number of MxM blocks of finest level
	bool					ShortC;				// Special case when TexTorusOrigin > 32767 or < -32767 => fully update level when it will be render 
};

class RTT
{
public:
	short x;
	short y;
	short u; 
	short v;

	RTT() {}

	void SetXY( short a, short b ) 
	{	
		x = a;
		y = b; 
	}

	void SetUV( short a, short b ) 
	{	
		u = a;
		v = b; 
	}

	void SetXYUV( short a, short b, short c, short d )
	{
		x = a;
		y = b;
		u = c;
		v = d; 
	}


	const RTT & operator=(const RTT & rtt)
	{
		x = rtt.x;
		y = rtt.y;
		u = rtt.u;
		v = rtt.v; 
		return *this;
	}
};

struct RTTQuad
{
	RTT Quad[6];
};

// Ground textures
extern LPDIRECT3DTEXTURE9 G1, G2, G3;

class GPUGeometryClipmap
{
	// Effects
	/*ID3DXEffect * m_pUpdate,
		* m_pRender,
		* m_pComputeNormals;*/
	ID3DX11Effect * m_pUpdate,
		* m_pRender,
		* m_pComputeNormals;

	ID3DX11EffectTechnique* m_pComputeNormalsTec;
	ID3DX11EffectShaderResourceVariable*	m_pElevationSRVVar;
	ID3DX11EffectShaderResourceVariable*	m_pCoarseNormalSRVVar;
	ID3DX11EffectVectorVariable*	m_pCoarserTexOffsetVar2;
	ID3DX11EffectVectorVariable*	m_pScaleFac;
	ID3DX11EffectScalarVariable*	m_pTexelClipVar2;
	ID3DX11EffectScalarVariable*	m_pZScaleOverFp;	
	ID3DX11EffectMatrixVariable*	m_pClipmapRTTVar2;
	
	ID3D11InputLayout*				m_pComputeNormalLayout;

	ID3DX11EffectTechnique* m_pUpdateTec;
	ID3DX11EffectVectorVariable*	m_pCoarserTexOffsetVar;
	ID3DX11EffectVectorVariable*	m_pTexScale;
	ID3DX11EffectScalarVariable*	m_pOneOverTextureSizeVar;
	ID3DX11EffectScalarVariable*	m_pFloatPrecisonVar;
	ID3DX11EffectMatrixVariable*	m_pClipmapRTTVar;
	ID3DX11EffectShaderResourceVariable* m_ptCoarserElevationSRVVar;
	ID3DX11EffectShaderResourceVariable*	m_pPermTexSRVVar;
	ID3DX11EffectShaderResourceVariable*	m_pGradTexSRVVar;

	ID3D11InputLayout*				m_pUpdateLayout;


	CComPtr<ID3D11ShaderResourceView> m_ptex2DPermSRV;
	CComPtr<ID3D11ShaderResourceView> m_ptex2DGradSRV;
	
	// Shader Perlin Noise
	LPDIRECT3DTEXTURE9 PermTexture;
	LPDIRECT3DTEXTURE9 GradTexture;

	// Vertex & Index Buffer
	//LPDIRECT3DVERTEXDECLARATION9 m_pVertexDeclaration,	// Render declaration ( 2 x Short POSITION, (4 x Float TEXCOORD0, 4 x Float TEXCOORD1) -> Instancing )
	//	m_pUpdateDeclaration;							// Update declaration ( 2 x Short POSITION, 2 x Short TEXCOORD0)


	LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer,			// Vertices coordinates (x,z), 0 to M
		m_pInstanceBuffer;								// Instance buffer

	LPDIRECT3DINDEXBUFFER9 m_pIndexBufferMxM,			// MxM index buffer             
		m_pIndexBufferMx3,								// Mx3 index buffer
		m_pIndexBufferLShape[4],						// 4 L-Shapes index buffer
		m_pIndexBufferOuterDegenerateTriangles;			// Degenerate triangles to remove T-Junctions artefacts 

	// Store number of primitives and vertices per index buffer
	DWORD NumVerticesMxMIndex,
		PrimitiveCountMxMIndex;

	DWORD NumVerticesMx3Index,
		PrimitiveCountMx3Index;

	DWORD NumVerticesOuterDegenerateTrianglesIndex,
		PrimitiveCountOuterDegenerateTrianglesIndex;

	DWORD NumVerticesLShapeIndex[4],
		PrimitiveCountLShapeIndex[4];

	// Number of instances of MxM blocks for the finest level ring interior
	DWORD NumInstancesFinest;

	// Constants
	short M, N, L,						// Same as paper, M = (N + 1) / 4, N = (2^X) - 1, L levels
		NP1, NM1,						// for N = 255, NP1 = 255, NM1 = -1 use for update clipmaps ( by default tex clipmap size is N + 1 )
										// for N = 127 and texture size = 256, NP1 = 192, NM1 = -64
		TexClipSize,					// Texture size of elevationMap, by default N + 1. if you are planning on rewriting update function 
										// to prevent the update each time the user move ( ex : if user move, and you set update to be done 
										// only once per 2 frames you need a larger TexClipSize to avoid errors ). You can directly *2, *4,
										// etc...  but if you keep this update function setting TexClipSize > N + 1 is useless.
		TexNormalSize,					// Size of normalmaps, must be >= TexClipSize. you can directly *2, *4, etc... but higher resolution 
										// of normal maps are ugly.
		MinLevel;						// Deactivate levels that are not rendered. if viewer is high in the sky, finer levels are not rendered. (but still updated) 
	float TexelClip,					// Texel size, 1.0f / TexClipSize
		TexClipShift,					// Texel shifting unit, same for all levels. 2 * TexelClip
		TexCoarserOffset[2],			// Depends on L-Shape case, (M - 0.5f) * TexelClip or (M + 0.5f) * TexelClip. Needed to get level + 1 (coarser) data for morphing
		ZScale,							// Max height of terrain
		FloatPrecision,					// Precision used to store terrain. if too low you will get a step effect. by default, ZScale * 20.0f is enough.
		ZScaleOverFp,					// Precompute for shader
		OneOverFp;						// Precompute for shader
	D3DXPLANE FrustumPlanes[6];			// Frustum planes for frustum cull
	noVec2 BlockOrigin[12];		// Block Origin of 12 MxM blocks
	noVec2 BlockOriginFinest[4];	// Block Origin of 4 MxM blocks of finest level interior ring
	D3DXVECTOR3 Viewer, LastViewer;		// Current and last viewer position. Needed for MinLevel and (x,z) displacement
	D3DXMATRIX  m_mClipmapRTT;			// Render To Texture projection matrix
	Clipmap * Levels;

	ID3D11DeviceContext* context;

	void FindFrustumPlanes(const D3DXMATRIX & MVP)
	{
		D3DXVECTOR3 v3FrustumCorners[8];
		D3DXMATRIX InvMVP;

		D3DXMatrixInverse(&InvMVP,NULL,&MVP); 

		v3FrustumCorners[0] = D3DXVECTOR3 ( -1.0f, -1.0f, 0.0f ); // xyz 
		v3FrustumCorners[1] = D3DXVECTOR3 (  1.0f, -1.0f, 0.0f ); // Xyz 
		v3FrustumCorners[2] = D3DXVECTOR3 ( -1.0f,  1.0f, 0.0f ); // xYz 
		v3FrustumCorners[3] = D3DXVECTOR3 (  1.0f,  1.0f, 0.0f ); // XYz 
		v3FrustumCorners[4] = D3DXVECTOR3 ( -1.0f, -1.0f, 1.0f ); // xyZ 
		v3FrustumCorners[5] = D3DXVECTOR3 (  1.0f, -1.0f, 1.0f ); // XyZ 
		v3FrustumCorners[6] = D3DXVECTOR3 ( -1.0f,  1.0f, 1.0f ); // xYZ 
		v3FrustumCorners[7] = D3DXVECTOR3 (  1.0f,  1.0f, 1.0f ); // XYZ 

		D3DXVec3TransformCoord(&v3FrustumCorners[0], &v3FrustumCorners[0], &InvMVP );
		D3DXVec3TransformCoord(&v3FrustumCorners[1], &v3FrustumCorners[1], &InvMVP );
		D3DXVec3TransformCoord(&v3FrustumCorners[2], &v3FrustumCorners[2], &InvMVP );
		D3DXVec3TransformCoord(&v3FrustumCorners[3], &v3FrustumCorners[3], &InvMVP );
		D3DXVec3TransformCoord(&v3FrustumCorners[4], &v3FrustumCorners[4], &InvMVP );
		D3DXVec3TransformCoord(&v3FrustumCorners[5], &v3FrustumCorners[5], &InvMVP );
		D3DXVec3TransformCoord(&v3FrustumCorners[6], &v3FrustumCorners[6], &InvMVP );
		D3DXVec3TransformCoord(&v3FrustumCorners[7], &v3FrustumCorners[7], &InvMVP );

		D3DXPlaneFromPoints ( &FrustumPlanes[0], &v3FrustumCorners[0], &v3FrustumCorners[1], &v3FrustumCorners[2] ); // Near
		D3DXPlaneFromPoints ( &FrustumPlanes[1], &v3FrustumCorners[6], &v3FrustumCorners[7], &v3FrustumCorners[5] ); // Far 
		D3DXPlaneFromPoints ( &FrustumPlanes[2], &v3FrustumCorners[2], &v3FrustumCorners[6], &v3FrustumCorners[4] ); // Left 
		D3DXPlaneFromPoints ( &FrustumPlanes[3], &v3FrustumCorners[7], &v3FrustumCorners[3], &v3FrustumCorners[5] ); // Right 
		D3DXPlaneFromPoints ( &FrustumPlanes[4], &v3FrustumCorners[2], &v3FrustumCorners[3], &v3FrustumCorners[6] ); // Top 
		D3DXPlaneFromPoints ( &FrustumPlanes[5], &v3FrustumCorners[1], &v3FrustumCorners[0], &v3FrustumCorners[4] ); // Bottom
	}

	float GridShifting(short Level, float LastLevel)
	{
		// (x,z) vertices translation at beginning
		if ( Level > 2) return (2 * fabs(LastLevel) + ((Level % 2) == 0 ? -1.0f : 1.0f )) * ((Level % 2) == 0 ? -1.0f : 1.0f );
		else
		{
			if ( Level == 1 ) return 1.0f;
			return -1.0f;
		}
	}

	void Destroy()
	{
		for (int l = 0; l < L; l++)
		{
			SAFE_RELEASE( Levels[l].ElevationMap );
			SAFE_RELEASE( Levels[l].NormalMap );
		}

		delete[] Levels;

		SAFE_RELEASE( PermTexture );
		SAFE_RELEASE( GradTexture );

		SAFE_RELEASE( m_pVertexBuffer );
		SAFE_RELEASE( m_pInstanceBuffer );
		SAFE_RELEASE( m_pIndexBufferMxM );
		SAFE_RELEASE( m_pIndexBufferMx3 );
		SAFE_RELEASE( m_pIndexBufferLShape[0] );
		SAFE_RELEASE( m_pIndexBufferLShape[1] );
		SAFE_RELEASE( m_pIndexBufferLShape[2] );
		SAFE_RELEASE( m_pIndexBufferLShape[3] );
		SAFE_RELEASE( m_pIndexBufferOuterDegenerateTriangles );
	}



	D3DXVECTOR3 getViewer()
	{
		return Viewer;
	}
};
