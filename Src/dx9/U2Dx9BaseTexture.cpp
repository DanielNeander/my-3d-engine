#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9BaseTexture.h"


IMPLEMENT_RTTI(U2Dx9BaseTexture, U2Object);



U2Dx9BaseTexture* U2Dx9BaseTexture::ms_pHead;
U2Dx9BaseTexture* U2Dx9BaseTexture::ms_pTail;
U2Dx9BaseTexture::U2Dx9BaseTexture()
	:m_bMipmapped(false),
	m_usWidth(0),
	m_usHeight(0),
	m_usMipMapLevels(0),
	m_pD3DTex(0),
	m_borderColor(D3DXCOLOR(0.f, 0.f, 0.f, 1.f)),
	m_pNext(0),
	m_pPrev(0),
	m_usTextureType(0),
	m_usUsage(0)
{
	
	m_eFType = LINEAR;
	m_aeWType[0] = CLAMP_EDGE;
	m_aeWType[1] = CLAMP_EDGE;
	m_aeWType[2] = CLAMP_EDGE;
	m_fAnisotropy = 1.F;

	AddTexToList();
}


U2Dx9BaseTexture::~U2Dx9BaseTexture()
{
	Release();
	RemoveTexFromList();
}

//-------------------------------------------------------------------------------------------------
void U2Dx9BaseTexture::Release()
{
	//int refCnt = m_pD3DTex->Release();
	//if(refCnt)
	//	refCnt = m_pD3DTex->Release();
	//U2ASSERT(refCnt == 0);
	SAFE_RELEASE(m_pD3DTex);
	
}

//-------------------------------------------------------------------------------------------------
bool U2Dx9BaseTexture::LoadFile(const TCHAR* szFilename, 
								U2Dx9Renderer* pRenderer)
{
	if(!pRenderer)
		return false;

	LPDIRECT3DDEVICE9 pD3DDev = pRenderer->GetD3DDevice();

//	TCHAR texPath[MAX_PATH];
//	_tcscpy_s(texPath, MAX_PATH, szFilename);
	U2Filename texFilename(szFilename);
	const TCHAR* szExt = texFilename.GetExt();	// 텍스쳐 파일의 확장자

	D3DXIMAGE_FILEFORMAT eFormat = D3DXIFF_FORCE_DWORD;

	if(_tcscmp(szExt, _T(".bmp")) == 0)
		eFormat = D3DXIFF_BMP;
	else if(_tcscmp(szExt, _T(".tga")) == 0)
		eFormat = D3DXIFF_TGA;
	else if(_tcscmp(szExt, _T(".dds")) == 0)
		eFormat = D3DXIFF_DDS;
	else if(_tcscmp(szExt, _T(".jpg")) == 0)
		eFormat = D3DXIFF_JPG;

	if(eFormat == D3DXIFF_FORCE_DWORD)
		return false;

	U2File* pFile = U2File::GetFile(szFilename, U2File::READ_ONLY);
	if(!pFile)
	{
		U2_DELETE pFile;
		return false;
	}
	
	uint32 uBuffLen = pFile->GetfileSize();
	if(uBuffLen == 0)
	{
		U2_DELETE pFile;
		return false;
	}

	TBYTE* ptbBuff = U2_ALLOC(TBYTE, uBuffLen);
	pFile->Read(ptbBuff, uBuffLen);

	U2_DELETE pFile;

	D3DXIMAGE_INFO imgInfo;
	HRESULT hr = D3DXGetImageInfoFromFileInMemory((VOID*)ptbBuff, 
		uBuffLen, &imgInfo);
	if(FAILED(hr))
	{
		U2_FREE(ptbBuff);
		ptbBuff = NULL;
		return false;
	}

	LPDIRECT3DBASETEXTURE9 pD3DTex = NULL;

	

	if(imgInfo.ResourceType == D3DRTYPE_TEXTURE)
	{
		LPDIRECT3DTEXTURE9 p2DTex = NULL;
		hr = D3DXCreateTextureFromFileInMemory(pD3DDev, 
			(VOID*)ptbBuff, uBuffLen, &p2DTex);
		pD3DTex = (LPDIRECT3DBASETEXTURE9)p2DTex;
	}
	else if(imgInfo.ResourceType == D3DRTYPE_CUBETEXTURE)
	{
		U2ASSERT(eFormat == D3DXIFF_DDS);
		LPDIRECT3DCUBETEXTURE9 pCubeTex = NULL;
		hr = D3DXCreateCubeTextureFromFileInMemory(pD3DDev, 
			(VOID*)ptbBuff, uBuffLen, &pCubeTex);
		pD3DTex = (LPDIRECT3DBASETEXTURE9)pCubeTex;
	}
	else if(imgInfo.ResourceType == D3DRTYPE_VOLUMETEXTURE)
	{
		U2ASSERT(eFormat == D3DXIFF_DDS);
		LPDIRECT3DVOLUMETEXTURE9 p3DTex = NULL;
		hr = D3DXCreateVolumeTextureFromFileInMemory(pD3DDev,
			(VOID*)ptbBuff, uBuffLen, &p3DTex);
		pD3DTex = (LPDIRECT3DBASETEXTURE9)p3DTex;
	}
	else 
		hr = D3DERR_INVALIDCALL;

	U2_FREE(ptbBuff);
	ptbBuff = NULL;

	if(FAILED(hr) || pD3DTex == NULL)
		return false;

	bool bSuccess = InitParamsFromD3DTexture(pD3DTex);
	if(bSuccess == false)
		pD3DTex->Release();

	return bSuccess;
}


bool U2Dx9BaseTexture::InitParamsFromD3DTexture(
	LPDIRECT3DBASETEXTURE9 pD3DTex)
{
	m_usMipMapLevels = (unsigned short)pD3DTex->GetLevelCount();
	m_bMipmapped = (m_usMipMapLevels != 1);

	D3DRESOURCETYPE eType = pD3DTex->GetType();
	if(eType == D3DRTYPE_TEXTURE)
	{
		LPDIRECT3DTEXTURE9 p2DTex = (LPDIRECT3DTEXTURE9)pD3DTex;

		D3DSURFACE_DESC desc;
		for(unsigned short i = 0; i < m_usMipMapLevels; ++i)
		{
			HRESULT hr = p2DTex->GetLevelDesc(i, &desc);
			if(FAILED(hr))
				return false;

			if(i == 0)
			{
				m_usWidth = desc.Width;
				m_usHeight = desc.Height;

				// Currently do not support palettized D3D textures
				if (desc.Format == D3DFMT_A8P8 ||
					desc.Format == D3DFMT_P8)
				{
					return false;
				}

			}
		}

	}
	else if(eType == D3DRTYPE_CUBETEXTURE)
	{
		LPDIRECT3DCUBETEXTURE9 pCubeTex =
			(LPDIRECT3DCUBETEXTURE9)pD3DTex;

		D3DSURFACE_DESC desc;
		for(unsigned short i = 0; i < m_usMipMapLevels; ++i)
		{
			HRESULT hr = pCubeTex->GetLevelDesc(i, &desc);
			if(FAILED(hr))
				return false;
			if(i == 0)
			{
				m_usWidth = desc.Width;
				m_usHeight = desc.Height;
				// Currently do not support palettized D3D textures
				if (desc.Format == D3DFMT_A8P8 ||
					desc.Format == D3DFMT_P8)
				{
					return false;
				}

			}
		}

	}
	else if(eType == D3DRTYPE_VOLUMETEXTURE)
	{
		LPDIRECT3DVOLUMETEXTURE9 p3DTex = 
			(LPDIRECT3DVOLUMETEXTURE9)pD3DTex;

		D3DVOLUME_DESC desc;
		for(unsigned short i = 0; i < m_usMipMapLevels; ++i)
		{

			HRESULT hr = p3DTex->GetLevelDesc(i, &desc);
			if(FAILED(hr))
				return false;
			if(i == 0)
			{
				m_usWidth = desc.Width;
				m_usHeight = desc.Height;
				// Currently do not support palettized D3D textures
				if (desc.Format == D3DFMT_A8P8 ||
					desc.Format == D3DFMT_P8)
				{
					return false;
				}

			}
		}
	}

	m_pD3DTex = pD3DTex;
	return true;
}


bool U2Dx9BaseTexture::IsRenderedTexture() const
{
	return (m_usTextureType & TEXTURETYPE_RENDERED) != 0;
}
//---------------------------------------------------------------------------
bool U2Dx9BaseTexture::IsNormalTexture() const
{
	return (m_usTextureType & TEXTURETYPE_NORMAL) != 0;
}
//---------------------------------------------------------------------------
bool U2Dx9BaseTexture::IsDynamicTexture() const
{
	return (m_usTextureType & TEXTURETYPE_DYNAMIC) != 0;
}

unsigned short U2Dx9BaseTexture::GetMipLevels() const
{
	return m_usMipMapLevels;
}


LPDIRECT3DBASETEXTURE9 U2Dx9BaseTexture::GetD3DTex() const
{
	return m_pD3DTex;
}



unsigned short U2Dx9BaseTexture::GetWidth() const
{
	return m_usWidth;
}


unsigned short U2Dx9BaseTexture::GetHeight() const
{
	return m_usHeight;
}



U2Dx9BaseTexture* U2Dx9BaseTexture::GetNext()
{
	return m_pNext;
}

U2Dx9BaseTexture* U2Dx9BaseTexture::GetPRev()
{
	return m_pPrev;
}


U2Dx9BaseTexture* U2Dx9BaseTexture::GetHead()
{
	return ms_pHead;
}


U2Dx9BaseTexture* U2Dx9BaseTexture::GetTail()
{
	return ms_pTail;
}


void U2Dx9BaseTexture::AddTexToList()
{
	if (!ms_pHead)
		ms_pHead = this;
	if (ms_pTail)
	{
		ms_pTail->m_pNext = this;
		m_pPrev = ms_pTail;
	}
	else
	{
		m_pPrev = 0;
	}
	ms_pTail = this;
	m_pNext = 0;
}


void U2Dx9BaseTexture::RemoveTexFromList()
{
	if (ms_pHead == this)
		ms_pHead = m_pNext;
	if (ms_pTail == this)
		ms_pTail = m_pPrev;

	if (m_pPrev)
		m_pPrev->m_pNext = m_pNext;
	if (m_pNext)
		m_pNext->m_pPrev = m_pPrev;

}


void U2Dx9BaseTexture::SetFilterType (FilterType eFType)
{
	m_eFType = eFType;
}


U2Dx9BaseTexture::FilterType U2Dx9BaseTexture::GetFilterType () const
{
	return m_eFType;
}

void U2Dx9BaseTexture::SetWrapType (int i, WrapType eWType)
{
	U2ASSERT(0 <= i && i < 3);
	m_aeWType[i] = eWType;
}


U2Dx9BaseTexture::WrapType U2Dx9BaseTexture::GetWrapType (int i) const
{
	U2ASSERT(0 <= i && i < 3);
	return m_aeWType[i];
}


void U2Dx9BaseTexture::SetBorderColor (const D3DXCOLOR& borderColor)
{
	m_borderColor = borderColor;
}


D3DXCOLOR U2Dx9BaseTexture::GetBorderColor () const
{
	return m_borderColor;
}


void U2Dx9BaseTexture::SetAnisotropyValue (float fAnisotropy)
{
	m_fAnisotropy = fAnisotropy;
}


float U2Dx9BaseTexture::GetAnisotropyValue () const
{
	return m_fAnisotropy;
}

void U2Dx9BaseTexture::SetUsage(int useFlags)
{
	m_usUsage = useFlags;
}

int U2Dx9BaseTexture::GetUsage() const
{
	return m_usUsage;
}

D3DFORMAT U2Dx9BaseTexture::GetD3DFmtFromU2PixelFormat(Format pixelFmt)
{
	D3DFORMAT colorFormat;
	switch (pixelFmt)
	{
	case X8R8G8B8:      colorFormat = D3DFMT_X8R8G8B8;      break;
	case A8R8G8B8:      colorFormat = D3DFMT_A8R8G8B8;      break;
	case G16R16:        colorFormat = D3DFMT_G16R16;        break;
	case R16F:          colorFormat = D3DFMT_R16F;          break;
	case G16R16F:       colorFormat = D3DFMT_G16R16F;       break;
	case A16B16G16R16F: colorFormat = D3DFMT_A16B16G16R16F; break;
	case R32F:          colorFormat = D3DFMT_R32F;          break;
	case G32R32F:       colorFormat = D3DFMT_G32R32F;       break;
	case A32B32G32R32F: colorFormat = D3DFMT_A32B32G32R32F; break;
	case R5G6B5:        colorFormat = D3DFMT_R5G6B5;        break;
	case A1R5G5B5:      colorFormat = D3DFMT_A1R5G5B5;      break;
	case A4R4G4B4:      colorFormat = D3DFMT_A4R4G4B4;      break;
	default:
		FDebug("nD3D9Texture: invalid render target pixel format!\n");
		colorFormat = D3DFMT_X8R8G8B8;
	}

	return colorFormat;
}

U2Dx9BaseTexture::Format U2Dx9BaseTexture::StringToPixelFormat(const char* str)
{
	U2ASSERT(str);
	if (0 == strcmp("NOFORMAT", str))           return NOFORMAT;
	else if (0 == strcmp("X8R8G8B8", str))      return X8R8G8B8;
	else if (0 == strcmp("A8R8G8B8", str))      return A8R8G8B8;
	else if (0 == strcmp("R5G6B5", str))        return R5G6B5;
	else if (0 == strcmp("A1R5G5B5", str))      return A1R5G5B5;
	else if (0 == strcmp("A4R4G4B4", str))      return A4R4G4B4;
	else if (0 == strcmp("P8", str))            return P8;
	else if (0 == strcmp("G16R16", str))        return G16R16;
	else if (0 == strcmp("DXT1", str))          return DXT1;
	else if (0 == strcmp("DXT2", str))          return DXT2;
	else if (0 == strcmp("DXT3", str))          return DXT3;
	else if (0 == strcmp("DXT4", str))          return DXT4;
	else if (0 == strcmp("DXT5", str))          return DXT5;
	else if (0 == strcmp("R16F", str))          return R16F;
	else if (0 == strcmp("G16R16F", str))       return G16R16F;
	else if (0 == strcmp("A16B16G16R16F", str)) return A16B16G16R16F;
	else if (0 == strcmp("R32F", str))          return R32F;
	else if (0 == strcmp("G32R32F", str))       return G32R32F;
	else if (0 == strcmp("A32B32G32R32F", str)) return A32B32G32R32F;
	else if (0 == strcmp("A8", str))            return A8;
	else
	{
		FDebug("nTexture2::StringToFormat(): invalid string '%s'", str);
		return NOFORMAT;
	}
}


void U2Dx9BaseTexture::OnLostDevice()
{
	Release();	
}

void U2Dx9BaseTexture::OnResetDevice()
{
	IsRendererOK;

	// Load Texture
	U2ASSERT(m_szName);

	 this->LoadFile(m_szName, pRenderer);
	
}