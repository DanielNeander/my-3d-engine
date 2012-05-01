/************************************************************************
module	:	U2Dx9BaseTexture
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_DX9_BASETEXTURE_H
#define	U2_DX9_BASETEXTURE_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2_3D/src/main/U2Object.h>
#include <U2Lib/Src/U2SmartPtr.h>
#include <U2Lib/Src/U2RefObject.h>


class U2Dx9Renderer;
class U2_3D U2Dx9BaseTexture : public U2Object
{
	DECLARE_RTTI;
	
public:		
	virtual ~U2Dx9BaseTexture();

	enum TextureType
	{
		TT_TEX, 
		TT_RENDERED,
		TT_DYNAMIC_TEX
	};

	// pixel formats
	enum Format
	{
		NOFORMAT,
		X8R8G8B8,
		A8R8G8B8,
		R5G6B5,
		A1R5G5B5,
		A4R4G4B4,
		P8,
		G16R16,
		DXT1,
		DXT2,
		DXT3,
		DXT4,
		DXT5,
		R16F,                       // 16 bit float, red only
		G16R16F,                    // 32 bit float, 16 bit red, 16 bit green
		A16B16G16R16F,              // 64 bit float, 16 bit rgba each
		R32F,                       // 32 bit float, red only
		G32R32F,                    // 64 bit float, 32 bit red, 32 bit green
		A32B32G32R32F,              // 128 bit float, 32 bit rgba each
		A8,
	};

	// usage flags
	enum Usage
	{
		CreateEmpty = (1<<0),               // don't load from disk, instead create empty texture
		CreateFromRawCompoundFile = (1<<1), // create from a compound file as raw ARGB pixel chunk
		CreateFromDDSCompoundFile = (1<<2), // create from dds file inside a compound file
		RenderTargetColor = (1<<3),         // is render target, has color buffer
		RenderTargetDepth = (1<<4),         // is render target, has depth buffer
		RenderTargetStencil = (1<<5),       // is render target, has stencil buffer
		Dynamic = (1<<6),                   // is a dynamic texture (for write access with CPU)
		Video = (1<<7)                      // is a Video
	};

	enum FilterType
	{
		NEAREST,          // nearest neighbor
		LINEAR,           // linear filtering
		NEAREST_NEAREST,  // nearest within image, nearest across images
		NEAREST_LINEAR,   // nearest within image, linear across images
		LINEAR_NEAREST,   // linear within image, nearest across images
		LINEAR_LINEAR,    // linear within image, linear across images
		MAX_FILTER_TYPES
	};

	enum WrapType
	{
		CLAMP,
		REPEAT,
		MIRRORED_REPEAT,
		CLAMP_BORDER,
		CLAMP_EDGE,
		MAX_WRAP_TYPES
	};

	static D3DFORMAT GetD3DFmtFromU2PixelFormat(Format pixelFmt);
	static U2Dx9BaseTexture::Format StringToPixelFormat(const char* str);

	

	bool IsRenderedTexture() const;
	bool IsNormalTexture() const;
	bool IsDynamicTexture() const;

	/// set combination of usage flags
	void SetUsage(int useFlags);
	/// get usage flags combination
	int GetUsage() const;

	unsigned short GetWidth() const;
	unsigned short GetHeight() const;

	unsigned short GetMipLevels() const;
	LPDIRECT3DBASETEXTURE9 GetD3DTex() const;


	// Access to filter modes.  The default is LINEAR.
	virtual void SetFilterType (FilterType eFType);
	FilterType GetFilterType () const;

	// Access to wrap modes.  The input i to SetWrapType and GetWrapType must
	// satisfy 0 <= i < GetDimension().  The defaults are CLAMP_TO_EDGE.
	void SetWrapType (int i, WrapType eWType);
	WrapType GetWrapType (int i) const;

	// Access to the border color used for sampling outside the texture image.
	// The default is ColorRGBA(0,0,0,1).
	void SetBorderColor (const D3DXCOLOR& rkBorderColor);
	D3DXCOLOR GetBorderColor () const;

	// Access to the anisotropy value for image filtering.
	void SetAnisotropyValue (float fAnisotropy);
	float GetAnisotropyValue () const;

	U2Dx9BaseTexture* GetNext();
	U2Dx9BaseTexture* GetPRev();

	static U2Dx9BaseTexture* GetHead();
	static U2Dx9BaseTexture* GetTail();

	void Release();

	void OnLostDevice();
	void OnResetDevice();
				
protected:
	void AddTexToList();
	void RemoveTexFromList();

	U2Dx9BaseTexture();	

	bool LoadFile(const TCHAR* szFilename, U2Dx9Renderer* pRenderer);
	bool InitParamsFromD3DTexture(LPDIRECT3DBASETEXTURE9 pD3DTex);

	unsigned short m_usWidth;
	unsigned short m_usHeight;
	unsigned short m_usMipMapLevels;
	unsigned short m_usUsage;
	bool	m_bMipmapped;

	U2Dx9BaseTexture* m_pNext;
	U2Dx9BaseTexture* m_pPrev;

	static U2Dx9BaseTexture* ms_pHead;
	static U2Dx9BaseTexture* ms_pTail;


	enum TextureTypeFlags
	{
		TEXTURETYPE_NORMAL	    = 1 << 0,
		TEXTURETYPE_RENDERED    = 1 << 1,
		TEXTURETYPE_DYNAMIC     = 1 << 2
	};
	unsigned short m_usTextureType;

	FilterType m_eFType;       // default = LINEAR
	WrapType m_aeWType[3];     // default = CLAMP_EDGE
	D3DXCOLOR m_borderColor;  // default = BLACK
	float m_fAnisotropy;       // default = 1.0f

	
	
	LPDIRECT3DBASETEXTURE9	m_pD3DTex;
};

typedef U2SmartPtr<U2Dx9BaseTexture> U2Dx9BaseTexturePtr;

#endif 