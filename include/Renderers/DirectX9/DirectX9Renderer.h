///////////////////////////////////////////////////////////////////////  
//  DirectX9Renderer.h
//
//	*** INTERACTIVE DATA VISUALIZATION (IDV) CONFIDENTIAL AND PROPRIETARY INFORMATION ***
//
//	This software is supplied under the terms of a license agreement or
//	nondisclosure agreement with Interactive Data Visualization, Inc. and
//  may not be copied, disclosed, or exploited except in accordance with 
//  the terms of that agreement.
//
//      Copyright (c) 2003-2009 IDV, Inc.
//      All rights reserved in all media.
//
//      IDV, Inc.
//      http://www.idvinc.com
//
//  *** Release version 5.0 Alpha 1 ***


///////////////////////////////////////////////////////////////////////  
//  Preprocessor

#pragma once
#define VC_EXTRALEAN
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>
#include "RenderInterface/ForestRI.h"
#include "RenderInterface/TerrainRI.h"
#include "RenderInterface/GrassRI.h"
#include "RenderInterface/SkyRI.h"


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

	///////////////////////////////////////////////////////////////////////  
	//  Forward references

	class CShaderLoaderDirectX9;


	///////////////////////////////////////////////////////////////////////  
	//  Class DX9

	class ST_STORAGE_CLASS DX9
	{
	public:
	static	LPDIRECT3DDEVICE9 ST_CALL_CONV	Device(void);
	static	void			  ST_CALL_CONV	SetDevice(LPDIRECT3DDEVICE9 pDevice);
	};


	///////////////////////////////////////////////////////////////////////  
	//  Class CRenderStateDirectX9

	class ST_STORAGE_CLASS CRenderStateDirectX9
	{
	public:

	static	void   	   ST_CALL_CONV Initialize(void);
	static	void   	   ST_CALL_CONV ApplyStates(void);
	static	void   	   ST_CALL_CONV Destroy(void);

	static	void   	   ST_CALL_CONV ClearScreen(st_bool bClearColor = true, st_bool bClearDepth = true);
	static	void   	   ST_CALL_CONV SetAlphaFunction(EAlphaFunc eFunc, st_float32 fValue);	// fValue range = [0.0, 255.0]
	static	void   	   ST_CALL_CONV SetAlphaTesting(st_bool bFlag);
	static	void   	   ST_CALL_CONV SetAlphaToCoverage(st_bool bFlag);
	static	void   	   ST_CALL_CONV SetBlending(st_bool bFlag);
	static	void       ST_CALL_CONV SetColorMask(st_bool bRed, st_bool bGreen, st_bool bBlue, st_bool bAlpha);
	static	void   	   ST_CALL_CONV SetDepthMask(st_bool bFlag);
	static	void	   ST_CALL_CONV	SetDepthTestFunc(EDepthTestFunc eDepthTestFunc);
	static	void   	   ST_CALL_CONV SetDepthTesting(st_bool bFlag);
	static	void   	   ST_CALL_CONV SetFaceCulling(ECullType eCullType);
	static	void   	   ST_CALL_CONV SetPointSize(st_float32 fSize);
	static	void   	   ST_CALL_CONV SetPolygonOffset(st_float32 fFactor = 0.0f, st_float32 fUnits = 0.0f);
	static	void   	   ST_CALL_CONV SetMultisampling(st_bool bMultisample);
	static	void   	   ST_CALL_CONV SetRenderStyle(ERenderStyle eStyle);

	private:
	static	st_bool	   m_bSupportsA2C;
	static	st_bool	   m_bATI;
	};
	typedef CRenderStateRI<CRenderStateDirectX9> CRenderState;


	///////////////////////////////////////////////////////////////////////  
	//  Class CTextureDirectX9

	class ST_STORAGE_CLASS CTextureDirectX9
	{
	public:
									CTextureDirectX9( );
	virtual							~CTextureDirectX9( );

			// loading
			st_bool					Load(const char* pFilename, st_int32 nMaxAnisotropy = 0);
			st_bool					LoadColor(st_uint32 ulColor);
			st_bool					LoadAlphaNoise(st_int32 nWidth, st_int32 nHeight);
			st_bool					LoadSpecularLookup(void);
			st_bool					LoadCubeNormalizer(st_int32 nResolution);
			st_bool					Unload(void);

			// render
			st_bool					FixedFunctionBind(void);
			LPDIRECT3DTEXTURE9		GetTextureObject(void) const;
			LPDIRECT3DCUBETEXTURE9	GetCubeTextureObject(void) const;

	private:
			// caching
			LPDIRECT3DTEXTURE9		SearchCache(const CString& strFilename);
			void					AddToCache(const CString& strFilename, LPDIRECT3DTEXTURE9 pTexture);
			st_bool					RemoveFromCache(const CString& strFilename);

			LPDIRECT3DTEXTURE9		m_pTexture;
			LPDIRECT3DCUBETEXTURE9	m_pCubeTexture;
			CString					m_strFilename;

			struct SCacheEntry
			{
									SCacheEntry( ) :
										m_pTexture(NULL),
										m_nRefCount(0)
									{
									}

				LPDIRECT3DTEXTURE9	m_pTexture;
				st_int32			m_nRefCount;
			};

			typedef CMap<CString, SCacheEntry> TextureCacheMap;

	static	TextureCacheMap*		m_pCache;
	};
	typedef CTextureRI<CTextureDirectX9> CTexture;


	///////////////////////////////////////////////////////////////////////  
	//  Class CDepthTextureDirectX9

	class ST_STORAGE_CLASS CDepthTextureDirectX9
	{
	public:
								CDepthTextureDirectX9( );
								~CDepthTextureDirectX9( );

			st_bool				Create(st_int32 nWidth, st_int32 nHeight);

			void				Release(void);
			void				OnResetDevice(void);
			void				OnLostDevice(void);

			st_bool				SetAsTarget(void);
			st_bool				ReleaseAsTarget(void);

			st_bool				BindAsTexture(void);
			st_bool				UnBindAsTexture(void);

			void				EnableShadowMapComparison(st_bool bFlag);
			
			LPDIRECT3DTEXTURE9	GetTextureObject(void) const;

	protected:
			LPDIRECT3DTEXTURE9	m_pColorTexture;
			LPDIRECT3DTEXTURE9	m_pDepthTexture;

			LPDIRECT3DSURFACE9	m_pColorDepthTexture;
			LPDIRECT3DSURFACE9	m_pDepthStencilSurface;

			LPDIRECT3DSURFACE9	m_pOldColorDepthTexture;
			LPDIRECT3DSURFACE9	m_pOldDepthStencilSurface;

			D3DVIEWPORT9		m_sOldViewport;

			st_int32			m_nBufferWidth;
			st_int32			m_nBufferHeight;
			st_bool				m_bInitSucceeded;
			st_bool				m_bBoundAsTarget;
	};
	typedef CDepthTextureRI<CDepthTextureDirectX9> CDepthTexture;


	///////////////////////////////////////////////////////////////////////  
	//  Class CShaderTechniqueDirectX9

	class ST_STORAGE_CLASS CShaderTechniqueDirectX9
	{
	public:
			friend class CShaderLoaderDirectX9;

								CShaderTechniqueDirectX9( );
	virtual						~CShaderTechniqueDirectX9( );

			st_bool				Bind(st_uint32 uiPass);
			st_bool				UnBind(void);
			st_bool				CommitConstants(void);
			st_bool				CommitTextures(void);

			st_bool				IsValid(void) const;

	private:
			LPD3DXEFFECT		m_pEffect;
			D3DXHANDLE			m_hTechnique;
	};
	typedef CShaderTechniqueRI<CShaderTechniqueDirectX9> CShaderTechnique;


	///////////////////////////////////////////////////////////////////////  
	//  Class CShaderConstantDirectX9

	class ST_STORAGE_CLASS CShaderConstantDirectX9
	{
	public:
			friend class CShaderLoaderDirectX9;

								CShaderConstantDirectX9( );
	virtual						~CShaderConstantDirectX9( );

			st_bool				IsValid(void) const;

			st_bool				Set1f(st_float32 x) const;
			st_bool				Set2f(st_float32 x, st_float32 y) const;
			st_bool				Set2fv(const st_float32 afValues[2]) const;
			st_bool				Set3f(st_float32 x, st_float32 y, st_float32 z) const;
			st_bool				Set3fv(const st_float32 afValues[3]) const;
			st_bool				Set3fvPlus1f(const st_float32 afValues[3], st_float32 w) const;
			st_bool				Set4f(st_float32 x, st_float32 y, st_float32 z, st_float32 w) const;
			st_bool				Set4fv(const st_float32 afValues[4]) const;
			st_bool				SetArray4f(const st_float32* pArray, st_int32 nSize, st_int32 nOffset = 0) const;
			st_bool				SetMatrix(const st_float32 afMatrix[16]) const;
			st_bool				SetMatrixTranspose(const st_float32 afMatrix[16]) const;
			st_bool				SetMatrixIndex(const st_float32 afMatrix[16], st_uint32 uiIndex) const;
			st_bool				SetMatrix4x4Array(const st_float32* pMatrixArray, st_uint32 uiNumMatrices) const;

			LPD3DXEFFECT		m_pEffect;
			D3DXHANDLE			m_hParameter;
	};


	///////////////////////////////////////////////////////////////////////  
	//  Class CShaderConstant

	class ST_STORAGE_CLASS CShaderConstant : public CShaderConstantRI<CShaderConstantDirectX9>
	{
	public:
			st_bool				SetTexture(const CTexture& cTexture) const;
			st_bool				SetTexture(const CDepthTexture& cDepthTexture) const;

			st_bool				SetTextureIndex(const CTexture& cTexture, st_uint32 uiIndex) const;
			st_bool				SetTextureIndex(const CDepthTexture& cDepthTexture, st_uint32 uiIndex) const;
	};


	///////////////////////////////////////////////////////////////////////  
	//  Class CShaderLoaderDirectX9

	class ST_STORAGE_CLASS CShaderLoaderDirectX9
	{
	public:
								CShaderLoaderDirectX9( );
	virtual						~CShaderLoaderDirectX9( );

			st_bool				Load(const char* pFilename, const CArray<CString>& aDefines);
			const char*			GetError(void) const;

			st_bool				GetShader(const char* pName, CShaderTechnique& cShader) const;
			st_bool				GetConstant(const char* pName, CShaderConstant& cConstant) const;

			void				Release(void);
			void				OnResetDevice(void);
			void				OnLostDevice(void);

	private:
			LPD3DXEFFECT		m_pEffect;
			CString				m_strError;
	};
	typedef CShaderLoaderRI<CShaderLoaderDirectX9, CShaderTechnique, CShaderConstant> CShaderLoader;


	///////////////////////////////////////////////////////////////////////  
	//  Class CGeometryBufferDirectX9

	class ST_STORAGE_CLASS CGeometryBufferDirectX9
	{
	public:
											CGeometryBufferDirectX9( );
	virtual									~CGeometryBufferDirectX9( );

			// vertex buffer
			st_bool							SetVertexFormat(const SVertexAttribDesc* pAttribDesc, const CShaderTechnique* pTechnique, st_bool bDynamic);
			st_bool							CreateVertexBuffer(const void* pVertexData, st_uint32 uiNumVertices, st_uint32 uiVertexSize);
			st_bool							ReplaceVertices(const void* pVertexData, st_uint32 uiNumVertices, st_uint32 uiVertexSize);
			st_bool							VertexBufferIsValid(void) const;

			st_bool							EnableFormat(void) const;
	static	st_bool		       ST_CALL_CONV DisableFormat(void);

			st_bool							BindVertexBuffer(st_int32 nVertexSize) const;
	static	st_bool		       ST_CALL_CONV UnBindVertexBuffer(void);

			// index buffer
			st_bool							SetIndexFormat(EIndexFormat eFormat);
			st_bool							CreateIndexBuffer(const void* pIndexData, st_uint32 uiNumIndices);
			st_bool							ReplaceIndices(const void* pIndexData, st_uint32 uiNumIndices);
			st_bool							IndexBufferIsValid(void) const;

			st_bool							BindIndexBuffer(void) const;
			st_bool							UnBindIndexBuffer(void) const;

			// render functions
			st_bool							RenderIndexed(EPrimitiveType ePrimType, st_uint32 uiStartIndex, st_uint32 uiNumIndices, st_uint32 uiNumVerticesOverride = 0) const;
			st_bool							RenderArrays(EPrimitiveType ePrimType, st_uint32 uiStartVertex, st_uint32 uiNumVertices) const;
	static	st_bool							IsPrimitiveTypeSupported(EPrimitiveType ePrimType);

	private:
			st_bool							IsFormatSet(void) const;
			st_uint32						IndexSize(void) const;
			void							FindMaxIndex(const void* pIndexData, st_uint32 uiNumIndices);

			LPDIRECT3DVERTEXDECLARATION9	m_pVertexDeclaration;
			LPDIRECT3DVERTEXBUFFER9			m_pVertexBuffer;
			LPDIRECT3DINDEXBUFFER9			m_pIndexBuffer;
			EIndexFormat					m_eIndexFormat;
			st_uint32						m_uiMaxIndex;
			st_uint32						m_uiCurrentVertexBufferSize;	// in bytes
			st_uint32						m_uiCurrentIndexBufferSize;	// # of indices
	};
	typedef CGeometryBufferRI<CGeometryBufferDirectX9, CShaderTechnique> CGeometryBuffer;


	///////////////////////////////////////////////////////////////////////  
	//  Class CForestRender (DirectX9 specific)

	typedef CForestRI<CRenderState, 
					  CTexture, 
					  CGeometryBuffer, 
					  CShaderLoader, 
					  CShaderTechnique, 
					  CShaderConstant,
					  CDepthTexture> CForestRender;


	///////////////////////////////////////////////////////////////////////  
	//  Class CTreeRender (DirectX9 specific)

	typedef CTreeRI<CTexture, 
					CGeometryBuffer,
					CShaderTechnique> CTreeRender;


	///////////////////////////////////////////////////////////////////////  
	//  Class CTerrainRender (DirectX9 specific)

	typedef CTerrainRI<CRenderState, 
					   CTexture, 
					   CGeometryBuffer, 
					   CShaderLoader, 
					   CShaderTechnique, 
					   CShaderConstant,
					   CDepthTexture> CTerrainRender;


	///////////////////////////////////////////////////////////////////////  
	//  Class CGrassRender (DirectX9 specific)

	typedef CGrassRI<CRenderState, 
					 CTexture, 
					 CGeometryBuffer, 
					 CShaderLoader, 
					 CShaderTechnique, 
					 CShaderConstant,
					 CDepthTexture> CGrassRender;


	///////////////////////////////////////////////////////////////////////  
	//  Class CSkyRender (DirectX9 specific)

	typedef CSkyRI<CRenderState, 
				   CTexture, 
				   CGeometryBuffer, 
				   CShaderLoader, 
				   CShaderTechnique, 
				   CShaderConstant,
				   CDepthTexture> CSkyRender;


	// include inline functions
	#include "Renderers/DirectX9/RenderState.inl"
	#include "Renderers/DirectX9/Texture.inl"
	#include "Renderers/DirectX9/GeometryBuffer.inl"
	#include "Renderers/DirectX9/Shaders.inl"
	#include "Renderers/DirectX9/DepthTexture.inl"

} // end namespace SpeedTree



// Macintosh export control
#ifdef __APPLE__
#pragma export off
#endif
