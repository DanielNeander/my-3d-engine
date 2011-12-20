#pragma once 

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include <string>
#include <sstream>
#include <vector>


extern IDirect3DDevice9* gd3dDevice;

#define ReleaseCOM(x) { if(x){ x->Release();x = 0; } }


extern void GenerateGrid(unsigned short numVertRow, unsigned short numVertCols, 
				  float dx, float dz, const D3DXVECTOR3& vCenter,
				  std::vector<D3DXVECTOR3>& verts, std::vector<DWORD>& indices);


// Colors and Materials

const D3DXCOLOR WHITE(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR BLACK(0.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR RED(1.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR GREEN(0.0f, 1.0f, 0.0f, 1.0f);
const D3DXCOLOR BLUE(0.0f, 0.0f, 1.0f, 1.0f);

struct Mtrl
{
	Mtrl()
		:ambient(WHITE), diffuse(WHITE), spec(WHITE), specPower(8.0f){}
	Mtrl(const D3DXCOLOR& a, const D3DXCOLOR& d, 
		const D3DXCOLOR& s, float power)
		:ambient(a), diffuse(d), spec(s), specPower(power){}

	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR spec;
	float specPower;
};

struct DirLight
{
	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR spec;
	D3DXVECTOR3 dirW;
};

//===============================================================
// .X Files

void LoadXFile(
			   const std::string& filename, 
			   ID3DXMesh** meshOut, 
			   std::vector<Mtrl>& mtrls, 
			   std::vector<IDirect3DTexture9*>& texs);

//===============================================================
// Debug

#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                      \
{                                                  \
	HRESULT hr = x;                                \
	if(FAILED(hr))                                 \
{                                              \
	DXTrace(__FILE__, __LINE__, hr, #x, TRUE); \
}                                              \
}
#endif

#else
#ifndef HR
#define HR(x) x;
#endif
#endif 


extern class U2N2Mesh* ConvertXToMesh(ID3DXMesh* pIn, IDirect3DDevice9* pd3dDevice);
