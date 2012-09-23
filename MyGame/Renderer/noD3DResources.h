#ifndef NO_RESOURCES_H
#define NO_RESOURCES_H

#include "noRenderSystemDX9.h"


///////////////// SXGeometryBuffer Class Declaration /////////////////
class noGeometryBuffer : public noD3DResource
{
public:
	noGeometryBuffer();
	~noGeometryBuffer();

	// Initialization
	HRESULT CreateVertexBuffer(DWORD dwVertexSize,DWORD dwVerticesCount,const void *pVertices = NULL);
	HRESULT CreateIndexBuffer(DWORD dwIndicesCount,const WORD* pIndices = NULL);

	// Accessors
	LPDIRECT3DVERTEXBUFFER9 GetVertexBuffer(void) const { return m_pVB; };
	LPDIRECT3DINDEXBUFFER9 GetIndexBuffer(void) const { return m_pIB; };
	DWORD GetVerticesCount(void) const;
	DWORD GetIndicesCount(void) const;
	DWORD GetVertexSize(void) const { return m_dwVertexSize; };
	void* GetVertices(void) const { return m_pVertices; };
	WORD* GetIndices(void) const { return m_pIndices; };

protected:
	// Overrides
	HRESULT OnDeviceLoss(void);
	HRESULT OnDeviceRelease(void);
	HRESULT OnDeviceRestore(void);

	DWORD m_dwIndicesCount;
	DWORD m_dwVerticesCount;
	DWORD m_dwVertexSize;
	void *m_pVertices;
	WORD *m_pIndices;

	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	LPDIRECT3DINDEXBUFFER9 m_pIB;
};

///////////////// SXTexture2D Class Declaration /////////////////
class noTexture2D : public noD3DResource
{
public:
	noTexture2D();
	~noTexture2D();

	// Initialization
	HRESULT SetTextureFile(PCTSTR pszFileName);

	// Accessors
	LPDIRECT3DTEXTURE9 GetTexture(void) const { return m_pTexture; };
	PCTSTR GetSourceFileName(void) const { return m_tszFileName; };

protected:
	// Overrides
	//HRESULT OnDeviceLoss(void);
	HRESULT OnDeviceRelease(void);
	HRESULT OnDeviceRestore(void);

	TCHAR m_tszFileName[MAX_PATH];
	LPDIRECT3DTEXTURE9 m_pTexture;
};

class noHLSLShader : public noD3DResource
{
public:
	noHLSLShader();
	~noHLSLShader();

	// Initialization
	HRESULT SetShaderFile(PCTSTR pszFileName);

	// Accessors
	LPDIRECT3DVERTEXSHADER9 GetVertexShader(void) const { return m_pVertexShader; };
	LPDIRECT3DPIXELSHADER9 GetPixelShader(void) const { return m_pPixelShader; };
	const DWORD* GetFunction(void) const { return m_pShaderFunction; };

protected:
	// Overrides
	//HRESULT OnDeviceLoss(void);
	HRESULT OnDeviceRelease(void);
	HRESULT OnDeviceRestore(void);

	DWORD *m_pShaderFunction;
	LPDIRECT3DVERTEXSHADER9 m_pVertexShader;
	LPDIRECT3DPIXELSHADER9 m_pPixelShader;
};

class noFXShader : public noD3DResource {
public:
	HRESULT OnDeviceRelease(void);
	HRESULT OnDeviceRestore(void);	

};



#endif