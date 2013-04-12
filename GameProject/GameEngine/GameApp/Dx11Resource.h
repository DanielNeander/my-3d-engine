#pragma once 
#define VC_EXTRALEAN
#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>

#include "GameApp/Terrain/RenderInterface/ForestRI.h"
#include "GameApp/Terrain/RenderInterface/TerrainRI.h"
#include "GameApp/Terrain/RenderInterface/GrassRI.h"
#include "GameApp/Terrain/RenderInterface/SkyRI.h"

namespace SpeedTree {

	///////////////////////////////////////////////////////////////////////  
	//  Forward references

	class CShaderLoaderDirectX10;


	///////////////////////////////////////////////////////////////////////  
	//  Class DX10

	class ST_STORAGE_CLASS DX11
	{
	public:
		// todo: should be inlined
		static	ID3D11Device*			ST_CALL_CONV	Device(void);
		static	ID3D11DeviceContext*			ST_CALL_CONV	Context(void);
		static	void					ST_CALL_CONV	SetDevice(ID3D11Device* pDevice);
		static	void					ST_CALL_CONV	SetContext(ID3D11DeviceContext* pContext);


		static	ID3D11RenderTargetView*	ST_CALL_CONV	MainRenderTargetView(void);
		static	void					ST_CALL_CONV	SetMainRenderTargetView(ID3D11RenderTargetView* pTarget);

		static	ID3D11DepthStencilView* ST_CALL_CONV	MainDepthStencilView(void);
		static	void					ST_CALL_CONV	SetMainDepthStencilView(ID3D11DepthStencilView* pTarget);
	};


	///////////////////////////////////////////////////////////////////////  
	//  Class CRenderStateDirectX9

	class ST_STORAGE_CLASS CRenderStateDirectX10
	{
	public:

		static	void   	   	ST_CALL_CONV 	Initialize(void);
		static	bool		ST_CALL_CONV	NeedsStateApplication(void);
		static	void		ST_CALL_CONV 	ApplyStates(void);
		static	void		ST_CALL_CONV 	Destroy(void);

		static	void   	   	ST_CALL_CONV 	ClearScreen(bool bClearColor = true, bool bClearDepth = true);
		static	void   	   	ST_CALL_CONV 	SetAlphaFunction(EAlphaFunc eFunc, float fValue);	// fValue range = [0.0, 255.0]
		static	void   	   	ST_CALL_CONV 	SetAlphaTesting(bool bFlag);
		static	void   	   	ST_CALL_CONV 	SetAlphaToCoverage(bool bFlag);
		static	void   	   	ST_CALL_CONV 	SetBlending(bool bFlag);
		static	void       	ST_CALL_CONV 	SetColorMask(bool bRed, bool bGreen, bool bBlue, bool bAlpha);
		static	void   	   	ST_CALL_CONV 	SetDepthMask(bool bFlag);
		static	void	   	ST_CALL_CONV 	SetDepthTestFunc(EDepthTestFunc eDepthTestFunc);
		static	void   	   	ST_CALL_CONV 	SetDepthTesting(bool bFlag);
		static	void   	   	ST_CALL_CONV 	SetFaceCulling(ECullType eCullType);
		static	void   	   	ST_CALL_CONV 	SetPointSize(float fSize);
		static	void   	   	ST_CALL_CONV 	SetPolygonOffset(float fFactor = 0.0f, float fUnits = 0.0f);
		static	void   	   	ST_CALL_CONV 	SetMultisampling(bool bMultisample);
		static	void   	   	ST_CALL_CONV 	SetRenderStyle(ERenderStyle eStyle);

	private:
		typedef CMap<D3D11_DEPTH_STENCIL_DESC, ID3D11DepthStencilState*> DepthStencilMap;
		typedef CMap<D3D11_RASTERIZER_DESC, ID3D11RasterizerState*> RasterizerMap;
		typedef CMap<D3D11_BLEND_DESC, ID3D11BlendState*> BlendMap;

		static	bool						m_bDepthChanged;
		static	D3D11_DEPTH_STENCIL_DESC	m_sCurrentDepthStateDesc;
		static	DepthStencilMap*			m_pDepthStates;

		static	bool						m_bRasterizerChanged;
		static	D3D11_RASTERIZER_DESC		m_sCurrentRasterizerStateDesc;
		static	RasterizerMap*				m_pRasterizerStates;

		static	bool						m_bBlendChanged;
		static	D3D11_BLEND_DESC			m_sCurrentBlendDesc;
		static	BlendMap*					m_pBlendStates;

		static	unsigned int					m_uiRefCount;
	};
	typedef CRenderStateRI<CRenderStateDirectX10> CRenderState;


	///////////////////////////////////////////////////////////////////////  
	//  Class CTextureDirectX10

	class ST_STORAGE_CLASS CTextureDirectX10
	{
	public:
		CTextureDirectX10( );
		virtual								~CTextureDirectX10( );

		// loading
		bool						Load(const char* pFilename, int nMaxAnisotropy = 0);
		bool						LoadColor(unsigned int ulColor);
		bool						LoadAlphaNoise(int nWidth, int nHeight);
		bool						LoadSpecularLookup(void);
		bool						LoadCubeNormalizer(int nResolution);
		bool						Unload(void);

		// render
		bool						FixedFunctionBind(void);
		ID3D11ShaderResourceView*	GetTextureObject(void) const;

	private:
		// caching
		ID3D11ShaderResourceView*	SearchCache(const CString& strFilename);
		void						AddToCache(const CString& strFilename, ID3D11ShaderResourceView* pTexture);
		bool						RemoveFromCache(const CString& strFilename);

		ID3D11ShaderResourceView*	m_pTexture;
		CString						m_strFilename;

		struct SCacheEntry
		{
			SCacheEntry( ) :
		m_pTexture(NULL),
			m_nRefCount(0)
		{
		}

		ID3D11ShaderResourceView*	m_pTexture;
		int					m_nRefCount;
		};

		typedef CMap<CString, SCacheEntry> TextureCacheMap;

		static	TextureCacheMap*		m_pCache;
	};
	typedef CTextureRI<CTextureDirectX10> CTexture;

	
	
	///////////////////////////////////////////////////////////////////////  
//  Class CDepthTextureDirectX10

class ST_STORAGE_CLASS CDepthTextureDirectX10
{
public:
	CDepthTextureDirectX10( );
	~CDepthTextureDirectX10( );

	bool						Create(int nWidth, int nHeight);

	void						Release(void);
	void						OnResetDevice(void);
	void						OnLostDevice(void);

	bool						SetAsTarget(void);
	bool						ReleaseAsTarget(void);

	bool						BindAsTexture(void);
	bool						UnBindAsTexture(void);

	void						EnableShadowMapComparison(bool bFlag);

	ID3D11ShaderResourceView*	GetTextureObject(void) const;

protected:
	ID3D11ShaderResourceView*	m_pColorTexture;
	ID3D11ShaderResourceView*	m_pDepthTexture;

	ID3D11RenderTargetView*		m_pDepthTextureView;
	ID3D11DepthStencilView*		m_pDepthStencilView;
	ID3D11RenderTargetView*		m_pOldDepthTextureView;
	ID3D11DepthStencilView*		m_pOldDepthStencilView;

	D3D11_VIEWPORT				m_sOldViewport;

	int					m_nBufferWidth;
	int					m_nBufferHeight;
	bool						m_bInitSucceeded;
	bool						m_bBoundAsTarget;
};
typedef CDepthTextureRI<CDepthTextureDirectX10> CDepthTexture;

#pragma once 

class ST_STORAGE_CLASS CShaderTechniqueDirectX10
{
public:
	friend class CShaderLoaderDirectX10;

	CShaderTechniqueDirectX10( );
	virtual							~CShaderTechniqueDirectX10( );

	bool					Bind(unsigned int uiPass);
	bool					UnBind(void);
	bool					CommitConstants(void);
	bool					CommitTextures(void);

	bool					IsValid(void) const;

	ID3DX11EffectTechnique*	GetDX10Technique(void) const;

private:
	ID3DX11EffectTechnique*	m_pTechnique;
};
typedef CShaderTechniqueRI<CShaderTechniqueDirectX10> CShaderTechnique;


///////////////////////////////////////////////////////////////////////  
//  Class CShaderConstantDirectX10

class ST_STORAGE_CLASS CShaderConstantDirectX10
{
public:
	friend class CShaderLoaderDirectX10;

	CShaderConstantDirectX10( );
	virtual							~CShaderConstantDirectX10( );

	bool					IsValid(void) const;

	bool					Set1f(float x) const;
	bool					Set2f(float x, float y) const;
	bool					Set2fv(const float afValues[2]) const;
	bool					Set3f(float x, float y, float z) const;
	bool					Set3fv(const float afValues[3]) const;
	bool					Set3fvPlus1f(const float afValues[3], float w) const;
	bool					Set4f(float x, float y, float z, float w) const;
	bool					Set4fv(const float afValues[4]) const;
	bool					SetArray4f(const float* pArray, int nSize, int nOffset = 0) const;
	bool					SetMatrix(const float afMatrix[16]) const;
	bool					SetMatrixTranspose(const float afMatrix[16]) const;
	bool					SetMatrixIndex(const float afMatrix[16], unsigned int uiIndex) const;
	bool					SetMatrix4x4Array(const float* pMatrixArray, unsigned int uiNumMatrices) const;

	ID3DX11EffectVariable*	m_pParameter;
};


///////////////////////////////////////////////////////////////////////  
//  Class CShaderConstant

class ST_STORAGE_CLASS CShaderConstant : public CShaderConstantRI<CShaderConstantDirectX10>
{
public:
	bool				SetTexture(const CTexture& cTexture) const;
	bool				SetTexture(const CDepthTexture& cDepthTexture) const;

	bool				SetTextureIndex(const CTexture& cTexture, unsigned int uiIndex) const;
	bool				SetTextureIndex(const CDepthTexture& cDepthTexture, unsigned int uiIndex) const;
};


///////////////////////////////////////////////////////////////////////  
//  Class CShaderLoaderDirectX10

class ST_STORAGE_CLASS CShaderLoaderDirectX10
{
public:
	CShaderLoaderDirectX10( );
	virtual						~CShaderLoaderDirectX10( );

	bool				Load(const char* pFilename, const CArray<CString>& aDefines);
	const char*			GetError(void) const;

	bool				GetShader(const char* pName, CShaderTechnique& cShader) const;
	bool				GetConstant(const char* pName, CShaderConstant& cConstant) const;

	void				Release(void);
	void				OnResetDevice(void);
	void				OnLostDevice(void);

private:
	ID3DX11Effect*		m_pEffect;
	CString				m_strError;
};
typedef CShaderLoaderRI<CShaderLoaderDirectX10, CShaderTechnique, CShaderConstant> CShaderLoader;


///////////////////////////////////////////////////////////////////////  
//  Class CGeometryBufferDirectX10

class ST_STORAGE_CLASS CGeometryBufferDirectX10
{
public:
	CGeometryBufferDirectX10( );
	virtual									~CGeometryBufferDirectX10( );

	// vertex buffer
	bool							SetVertexFormat(const SVertexAttribDesc* pAttribDesc, const CShaderTechnique* pTechnique, bool bDynamic);
	bool							CreateVertexBuffer(const void* pVertexData, unsigned int uiNumVertices, unsigned int uiVertexSize);
	bool							ReplaceVertices(const void* pVertexData, unsigned int uiNumVertices, unsigned int uiVertexSize);
	bool							VertexBufferIsValid(void) const;

	bool							EnableFormat(void) const;
	static	bool		       ST_CALL_CONV DisableFormat(void);

	bool							BindVertexBuffer(int nVertexSize) const;
	static	bool		       ST_CALL_CONV UnBindVertexBuffer(void);

	// index buffer
	bool							SetIndexFormat(EIndexFormat eFormat);
	bool							CreateIndexBuffer(const void* pIndexData, unsigned int uiNumIndices);
	bool							ReplaceIndices(const void* pIndexData, unsigned int uiNumIndices);
	bool							IndexBufferIsValid(void) const;

	bool							BindIndexBuffer(void) const;
	bool							UnBindIndexBuffer(void) const;

	// render functions
	bool							RenderIndexed(EPrimitiveType ePrimType, unsigned int uiStartIndex, unsigned int uiNumIndices, unsigned int uiNumVerticesOverride = 0) const;
	bool							RenderArrays(EPrimitiveType ePrimType, unsigned int uiStartVertex, unsigned int uiNumVertices) const;
	static	bool							IsPrimitiveTypeSupported(EPrimitiveType ePrimType);

private:
	bool							IsFormatSet(void) const;
	unsigned int						IndexSize(void) const;
	void							FindMaxIndex(const void* pIndexData, unsigned int uiNumIndices);

	ID3D11InputLayout*				m_pVertexLayout;
	ID3D11Buffer*					m_pVertexBuffer;
	ID3D11Buffer*					m_pIndexBuffer;
	EIndexFormat					m_eIndexFormat;
	unsigned int						m_uiMaxIndex;
	unsigned int						m_uiCurrentVertexBufferSize;	// in bytes
	unsigned int						m_uiCurrentIndexBufferSize;	// # of indices
	bool							m_bDynamicVertexBuffer;
};
typedef CGeometryBufferRI<CGeometryBufferDirectX10, CShaderTechnique> CGeometryBuffer;


///////////////////////////////////////////////////////////////////////  
//  Class CForestRender (DirectX10 specific)

typedef CForestRI<CRenderState, 
	CTexture, 
	CGeometryBuffer, 
	CShaderLoader, 
	CShaderTechnique, 
	CShaderConstant,
	CDepthTexture> CForestRender;

///////////////////////////////////////////////////////////////////////  
//  Class CTreeRender (DirectX10 specific)

typedef CTreeRI<CTexture, 
	CGeometryBuffer,
	CShaderTechnique> CTreeRender;


///////////////////////////////////////////////////////////////////////  
//  Class CTerrainRender (DirectX10 specific)

typedef CTerrainRI<CRenderState, 
	CTexture, 
	CGeometryBuffer, 
	CShaderLoader, 
	CShaderTechnique, 
	CShaderConstant,
	CDepthTexture> CTerrainRender;


///////////////////////////////////////////////////////////////////////  
//  Class CGrassRender (DirectX10 specific)

typedef CGrassRI<CRenderState, 
	CTexture, 
	CGeometryBuffer, 
	CShaderLoader, 
	CShaderTechnique, 
	CShaderConstant,
	CDepthTexture> CGrassRender;


///////////////////////////////////////////////////////////////////////  
//  Class CSkyRender (DirectX10 specific)

typedef CSkyRI<CRenderState, 
	CTexture, 
	CGeometryBuffer, 
	CShaderLoader, 
	CShaderTechnique, 
	CShaderConstant,
	CDepthTexture> CSkyRender;



// include inline functions
#include "RenderState.inl"
#include "Texture.inl"
#include "GeometryBuffer.inl"
#include "Shaders.inl"
#include "DepthTexture.inl"

} // end namespace SpeedTree