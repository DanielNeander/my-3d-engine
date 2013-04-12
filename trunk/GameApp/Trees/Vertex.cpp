//=============================================================================
// Vertex.cpp by Frank Luna (C) 2005 All Rights Reserved.
//=============================================================================
#include "stdafx.h"
#include "Vertex.h"

// Initialize static variables.
//IDirect3DVertexDeclaration9* VertexPos::Decl = 0;
//IDirect3DVertexDeclaration9* VertexCol::Decl = 0;
//IDirect3DVertexDeclaration9* VertexPN::Decl  = 0;
//IDirect3DVertexDeclaration9* VertexPNT::Decl = 0;
//IDirect3DVertexDeclaration9* Particle::Decl  = 0;

ID3D11InputLayout* VertexPos::Decl = 0;
ID3D11InputLayout* VertexCol::Decl = 0;
ID3D11InputLayout* VertexPN::Decl  = 0;
ID3D11InputLayout* VertexPNT::Decl = 0;
ID3D11InputLayout* Particle::Decl  = 0;

#if 0
void InitAllVertexDeclarations()
{
	//===============================================================
	// VertexPos

	D3DVERTEXELEMENT9 VertexPosElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		D3DDECL_END()
	};	
	//HR(gd3dDevice->CreateVertexDeclaration(VertexPosElements, &VertexPos::Decl));


	//===============================================================
	// VertexCol

	D3DVERTEXELEMENT9 VertexColElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		D3DDECL_END()
	};	
	//HR(gd3dDevice->CreateVertexDeclaration(VertexColElements, &VertexCol::Decl));

	//===============================================================
	// VertexPN

	D3DVERTEXELEMENT9 VertexPNElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		D3DDECL_END()
	};	
	//HR(gd3dDevice->CreateVertexDeclaration(VertexPNElements, &VertexPN::Decl));

	//===============================================================
	// VertexPNT

	D3DVERTEXELEMENT9 VertexPNTElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};	
	//HR(gd3dDevice->CreateVertexDeclaration(VertexPNTElements, &VertexPNT::Decl));


  //VertexDeclaration *decl = mRenderOp.vertexData->vertexDeclaration;
  //decl->addElement(POSITION_BINDING, 0, VET_FLOAT3, VES_POSITION);

	//===============================================================
	// Particle

	D3DVERTEXELEMENT9 ParticleElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		{0, 24, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
		{0, 28, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},
		{0, 32, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},
		{0, 36, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4},
		{0, 40, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		D3DDECL_END()
	};	
	//HR(gd3dDevice->CreateVertexDeclaration(ParticleElements, &Particle::Decl));
}
#endif

/*void DestroyAllVertexDeclarations()
{
	ReleaseCOM(VertexPos::Decl);
	ReleaseCOM(VertexCol::Decl);
	ReleaseCOM(VertexPN::Decl);
	ReleaseCOM(VertexPNT::Decl);
	ReleaseCOM(Particle::Decl);
}*/
