#pragma once 
#include "ShadowMap.h"


class PSSMShadowMapDx11 : public ShadowMap 
{
public:
	PSSMShadowMapDx11();
	~PSSMShadowMapDx11();

	bool Create(int iSize);
	bool CreateAsTextureArray(int iSize, int iArraySize);
	bool CreateAsTextureCube(int iSize);
	void Destroy(void);

	// start rendering to texture
	void EnableRendering(void);
	// stop rendering to texture
	void DisableRendering(void);

	int GetMemoryInMB(void);
	bool CreateAsTextureCube2( int iSize );

	bool CreateConstantBuffer(CComPtr<ID3DX11Effect> pEffect11);

	struct PSSMConstant 
	{
		mat4	TextureMatrixArray[4];
		Float4Align float	SplitPlane[4];
	};
	PSSMConstant mPssmConstants;
	ID3D11BufferPtr m_pD3DBuffer;
	ID3DX11EffectConstantBuffer* m_pPSSMConstBuffer;
	int m_ibufferSize;
public:
	int m_iArraySize;

	ID3D11Texture2D *m_pTexture;
	ID3D11DepthStencilView *m_pDSV;
	ID3D11ShaderResourceView *m_pSRV;

	
};