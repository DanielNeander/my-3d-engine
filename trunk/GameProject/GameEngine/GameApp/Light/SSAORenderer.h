#pragma once 

class SSAORenderer 
{
public:
	SSAORenderer();

	bool Init(ID3D11Device* dev, ID3D11DeviceContext* context);

	HRESULT LoadShaders();
	void SSAORenderer::Release();

	void Reset();
	void SSAORenderer::RenderCS();

	void BeginDepthRender();
	
	void EndDepthRender();
	void SetDepthShader(noMat4* world);	
	void SetSkinDepthShader(noMat4* world,  noMat4* boneMat, int numBones);
	ID3D11ShaderResourceView* GetAOSRV() const { return g_pOcclusionBufferSRV; }
private:
	ID3D11DeviceContext* Context;
	ID3D11Device* Device;


	ID3D11Texture2D*			g_pDepthNormalBuffer ;
	ID3D11RenderTargetView*		g_pDepthNormalBufferRTV ;
	ID3D11ShaderResourceView *	g_pDepthNormalBufferSRV ;

	ID3D11Texture2D*			g_pOcclusionBuffer ;
	ID3D11UnorderedAccessView*	g_pOcclusionBufferUAV ;
	ID3D11ShaderResourceView *	g_pOcclusionBufferSRV ;

	
	ID3D11InputLayout*			g_pInputLayout ;

	ID3D11VertexShader*			g_pDepthVS ;
	ID3D11VertexShader*			g_pDepthSkinVS ;
	ID3D11PixelShader*			g_pDepthPS ;

	ID3D11ComputeShader*		g_pAmbientOcclusionCS ;
	ID3D11ComputeShader*		g_pHorizontalBilateralCS; 
	ID3D11ComputeShader*		g_pVerticalBilateralCS ;

	ID3D11VertexShader*			g_pFinalVS ;
	ID3D11PixelShader*			g_pFinalPS ;

	ID3D11Buffer*				g_pConstantBuffer ;
	ID3D11Buffer*				g_pSkinConstantBuffer ;
};