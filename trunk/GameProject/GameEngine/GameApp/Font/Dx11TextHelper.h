#pragma once 

struct DXUTSpriteVertex
{
	D3DXVECTOR3 vPos;
	D3DXCOLOR vColor;
	D3DXVECTOR2 vTex;
};


class Dx11TextHelper : public Singleton<Dx11TextHelper>
{
public:
	//CDXUTTextHelper( ID3DXFont* pFont9 = NULL, ID3DXSprite* pSprite9 = NULL,
	//	int nLineHeight = 15 );
	Dx11TextHelper( ID3D11Device* pd3d11Device, ID3D11DeviceContext* pd3dDeviceContext, int nLineHeight = 15);
	~Dx11TextHelper();

	//void    Init( ID3DXFont* pFont9 = NULL, ID3DXSprite* pSprite9 = NULL, 
	//	int nLineHeight = 15 );
	void    Init(int nLineHeight = 15 );
	void	InitFont();

	void    SetInsertionPos( int x, int y )
	{
		m_pt.x = x; m_pt.y = y;
	}
	void    SetForegroundColor( D3DXCOLOR clr )
	{
		m_clr = clr;
	}

	void    Begin();
	HRESULT DrawFormattedTextLine( const CHAR* strMsg, ... );
	HRESULT DrawTextLine( const CHAR* strMsg );
	HRESULT DrawFormattedTextLine( RECT& rc, DWORD dwFlags, const CHAR* strMsg, ... );
	HRESULT DrawTextLine( RECT& rc, DWORD dwFlags, const CHAR* strMsg );
	void    End();

protected:

	void StoreD3D11State( ID3D11DeviceContext* pd3dImmediateContext );
	void RestoreD3D11State( ID3D11DeviceContext* pd3dImmediateContext );
	void EndText11( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3d11DeviceContext );
	void DrawText11DXUT( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3d11DeviceContext, LPCSTR strText, RECT rcScreen, D3DXCOLOR vFontColor, float fBBWidth, float fBBHeight, bool bCenter );
	
	ID3D11DepthStencilState* m_pDepthStencilStateUI11;
	ID3D11RasterizerState* m_pRasterizerStateUI11;
	ID3D11BlendState* m_pBlendStateUI11;
	ID3D11SamplerState* m_pSamplerStateUI11;

	// Stored states
	ID3D11DepthStencilState* m_pDepthStencilStateStored11;
	UINT m_StencilRefStored11;
	ID3D11RasterizerState* m_pRasterizerStateStored11;
	ID3D11BlendState* m_pBlendStateStored11;
	float m_BlendFactorStored11[4];
	UINT m_SampleMaskStored11;
	ID3D11SamplerState* m_pSamplerStateStored11;

	ID3DX11Effect* g_pFontEffect;
	ID3DX11EffectTechnique* g_pFontTec;
	ID3DX11EffectShaderResourceVariable*	m_pFontTex;

	//ID3DXFont* m_pFont9;
	//ID3DXSprite* m_pSprite9;
	D3DXCOLOR m_clr;
	POINT m_pt;
	int m_nLineHeight;

	// D3D11 font 
	ID3D11InputLayout* m_pInputLayout11;
	ID3D11Device* m_pd3d11Device;
	ID3D11DeviceContext* m_pd3d11DeviceContext;	
};
void DrawText11DXUT( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3d11DeviceContext, LPCSTR strText, RECT rcScreen, D3DXCOLOR vFontColor, float fBBWidth, float fBBHeight, bool bCenter );
void EndText11( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3d11DeviceContext );