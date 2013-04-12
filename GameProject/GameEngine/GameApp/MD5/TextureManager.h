// TextureManager.h:
// Created by: Aurelio Reis

#ifndef __AR__TEXTURE__MANAGER__H__
#define __AR__TEXTURE__MANAGER__H__


//////////////////////////////////////////////////////////////////////////
// CArBaseTexture
//////////////////////////////////////////////////////////////////////////

class CArBaseTexture
{
public:
	CArBaseTexture() {}
	virtual ~CArBaseTexture() = 0 {}

	const string &GetFilePath() const { return m_strFilePath; }
	virtual LPDIRECT3DTEXTURE9 GetD3dTexture() = 0;
	virtual const D3DXVECTOR2 &GetOffset() const = 0;
	virtual const D3DXVECTOR2 &GetSize() const = 0;
	
	friend class CArTextureAtlas;

protected:
	string m_strFilePath;
};


//////////////////////////////////////////////////////////////////////////
// CArTexture
//////////////////////////////////////////////////////////////////////////

class CArTexture : public CArBaseTexture
{
public:
	CArTexture() : m_pTexture( NULL ) {}
	~CArTexture() { SAFE_RELEASE( m_pTexture ); }

	bool Create( const char *strFilePath )
	{
		m_strFilePath = strFilePath;

		bool bRet = S_OK == D3DXCreateTextureFromFileExA( g_pd3dDevice, strFilePath, D3DX_DEFAULT, D3DX_DEFAULT,
			D3DX_FROM_FILE, 0, D3DFMT_FROM_FILE, D3DPOOL_MANAGED, D3DX_FILTER_NONE,
			D3DX_DEFAULT, 0, NULL, NULL, &m_pTexture );

		assert( bRet );
		return bRet;
	}

	virtual LPDIRECT3DTEXTURE9 GetD3dTexture() { return m_pTexture; }
	virtual const D3DXVECTOR2 &GetOffset() const { return VEC2_ZERO; }
	virtual const D3DXVECTOR2 &GetSize() const { return VEC2_ONE; }

private:
	LPDIRECT3DTEXTURE9 m_pTexture;
};


//////////////////////////////////////////////////////////////////////////
// CArVirtualTexture
//////////////////////////////////////////////////////////////////////////

class CArVirtualTexture : public CArBaseTexture
{
public:
	D3DXVECTOR2 m_vOffset, m_vSize;
	CArTexture *m_pAtlasTexture;

	CArVirtualTexture() {}
	~CArVirtualTexture() {}

	virtual LPDIRECT3DTEXTURE9 GetD3dTexture() { return m_pAtlasTexture->GetD3dTexture(); }
	virtual const D3DXVECTOR2 &GetOffset() const { return m_vOffset; }
	virtual const D3DXVECTOR2 &GetSize() const { return m_vSize; }
};


//////////////////////////////////////////////////////////////////////////
// CArTextureAtlas
//////////////////////////////////////////////////////////////////////////

class CArTextureAtlas
{
public:
	CArTextureAtlas();
	~CArTextureAtlas();

	void Destroy();
	CArBaseTexture *FindTexture( const char *strPath );
	void LoadAtlas( const char *strPath );

private:
	vector< CArTexture * > m_AtlasTextures;
	vector< CArVirtualTexture * > m_VirtualTextures;

	CArTexture *CreateAtlasTexture( string strFilePath );
	CArTexture *FindAtlasTexture( const char *strPath );
};


//////////////////////////////////////////////////////////////////////////
// CArTextureManager
//////////////////////////////////////////////////////////////////////////

class CArTextureManager
{
public:
	CArTextureManager() : m_bUseAtlasVirtualTextures( true ) {}
	~CArTextureManager() {}

	void Initialize();
	void Shutdown();
	void PurgeTextureCache();
	CArBaseTexture *CreateTexture( string strFilePath );
	void DestroyTexture( CArBaseTexture *pTx );
	void AddAtlas( CArTextureAtlas *pAtlas );

	void SetUseAtlasVirtualTextures( bool bUseAtlasVirtualTextures ) { m_bUseAtlasVirtualTextures = bUseAtlasVirtualTextures; }

private:
	vector< CArTextureAtlas * > m_AtlasList;
	bool						m_bUseAtlasVirtualTextures;
	vector< CArBaseTexture * >	m_TextureCache;
};


extern CArTextureManager g_TextureManager;


#endif // __AR__TEXTURE__MANAGER__H__
