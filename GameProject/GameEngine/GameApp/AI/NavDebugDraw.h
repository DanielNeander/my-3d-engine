#pragma once 

class Mesh_D3D11;


struct DebugVertex {

	noVec3 pos;
	noVec4 color;
};


struct NavDebugDraw : public duDebugDraw
{
public:
	
	NavDebugDraw();
	virtual ~NavDebugDraw();

	
	bool CreateVertexBuffer(const void* pVertexData, unsigned int uiNumVertices, unsigned int uiVertexSize);


	virtual void depthMask(bool state);

	// Begin drawing primitives.
	// Params:
	//  prim - (in) primitive type to draw, one of rcDebugDrawPrimitives.
	//  nverts - (in) number of vertices to be submitted.
	//  size - (in) size of a primitive, applies to point size and line width only.
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);

	// Submit a vertex
	// Params:
	//  pos - (in) position of the verts.
	//  color - (in) color of the verts.
	virtual void vertex(const float* pos, unsigned int color);

	// Submit a vertex
	// Params:
	//  x,y,z - (in) position of the verts.
	//  color - (in) color of the verts.
	virtual void vertex(const float x, const float y, const float z, unsigned int color);
	
	// End drawing primitives.
	virtual void end();
	
	void setOffset(float _offset) { mYOffset = _offset; }

	
	bool CreateIndexBuffer(const void* pIndexData, unsigned int uiNumIndices);
	void FindMaxIndex(const void* pIndexData, unsigned int uiNumIndices);
	void SetupShader();
	DebugVertex* LockVB( int offset);
	void UnlockVB( );
	WORD* LockIB16( );
	void UnlockIB16( );
	void Draw();
	unsigned int						m_uiMaxIndex;
	unsigned int						m_uiCurrentVertexBufferSize;	// in bytes
	unsigned int						m_uiCurrentIndexBufferSize;	// # of indices
	float mYOffset;


	D3D_PRIMITIVE_TOPOLOGY	m_Prim;
	D3D11_MAPPED_SUBRESOURCE m_MappedResource;		
	
	unsigned int m_BaseVertex;
	unsigned int m_BaseIndex;
	unsigned int m_CurrVertex;
	ID3D11Buffer *m_pVertexBuffer;
	ID3D11Buffer *m_pIndexBuffer;
	ID3D11InputLayout *m_pVertexLayout;

	ID3DX11Effect*                       gpEffect;
	ID3DX11EffectTechnique*              gpDiffuse;
	ID3DX11EffectMatrixVariable*		 mpWorldViewProjection;
	bool m_bLock;
};