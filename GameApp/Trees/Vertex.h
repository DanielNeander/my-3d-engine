//=============================================================================
// Vertex.h by Frank Luna (C) 2005 All Rights Reserved.
//
// This file contains all of our vertex structures.  We will add to it
// as needed throughout the book.
//=============================================================================
//#include <d3dx9.h>

#ifndef VERTEX_H
#define VERTEX_H



// Call in constructor and destructor, respectively, of derived application class.
void InitAllVertexDeclarations();
//void DestroyAllVertexDeclarations();
enum VertexType {VP, VC, VPN, VPNT};

//===============================================================
struct VertexPos
{
	VertexPos():pos(0.0f, 0.0f, 0.0f){}
	VertexPos(float x, float y, float z):pos(x,y,z){}
	VertexPos(const noVec3& v):pos(v){}

	noVec3 pos;
	//static IDirect3DVertexDeclaration9* Decl;
	static ID3D11InputLayout* Decl;
};

//===============================================================
struct VertexCol
{
	VertexCol():pos(0.0f, 0.0f, 0.0f),col(0x00000000){}
	//VertexCol(float x, float y, float z, DWORD c):pos(x,y,z), col(c){}
	//VertexCol(const noVec3& v, DWORD c):pos(v),col(c){}
	VertexCol(float x, float y, float z, DWORD c):pos(x,y,z), col(c){}
	VertexCol(const noVec3& v, DWORD c):pos(v),col(c){}

	noVec3 pos;
	DWORD    col;
	
	//static IDirect3DVertexDeclaration9* Decl;
	static ID3D11InputLayout* Decl;
};

//===============================================================
struct VertexPN
{
	VertexPN()
		:pos(0.0f, 0.0f, 0.0f),
		normal(0.0f, 0.0f, 0.0f){}
	VertexPN(float x, float y, float z, 
		float nx, float ny, float nz):pos(x,y,z), normal(nx,ny,nz){}
	VertexPN(const noVec3& v, const noVec3& n)
		:pos(v),normal(n){}

	noVec3 pos;
	noVec3 normal;
	//static IDirect3DVertexDeclaration9* Decl;
	static ID3D11InputLayout* Decl;
};

//===============================================================
struct VertexPNT
{
	VertexPNT()
		:pos(0.0f, 0.0f, 0.0f),
		normal(0.0f, 0.0f, 0.0f),
		tex0(0.0f, 0.0f){}
	VertexPNT(float x, float y, float z, 
		float nx, float ny, float nz,
		float u, float v):pos(x,y,z), normal(nx,ny,nz), tex0(u,v){}
	VertexPNT(const noVec3& v, const noVec3& n, const noVec2& uv)
		:pos(v),normal(n), tex0(uv){}

	noVec3 pos;
	noVec3 normal;
	noVec2 tex0;

	//static IDirect3DVertexDeclaration9* Decl;
	static ID3D11InputLayout* Decl;

};

//===============================================================
struct Particle
{
	noVec3 initialPos;
	noVec3 initialVelocity;
	float       initialSize; // In pixels.
	float       initialTime;
	float       lifeTime;
	float       mass;
	DWORD    initialColor;

	//static IDirect3DVertexDeclaration9* Decl;
	static ID3D11InputLayout* Decl;
};

#endif // VERTEX_H