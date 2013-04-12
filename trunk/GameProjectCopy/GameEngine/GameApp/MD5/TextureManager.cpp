// TextureManager.h:
// Created by: Aurelio Reis

#include "stdafx.h"

#include "TextureManager.h"


CArTextureManager g_TextureManager;


//////////////////////////////////////////////////////////////////////////
// CArTextureAtlas
//////////////////////////////////////////////////////////////////////////

CArTexture *CArTextureAtlas::CreateAtlasTexture( string strFilePath )
{
	MakeFilePathCanonical( strFilePath );

	// First see if this atlas texture doesn't already exist.
	CArTexture *pTx = FindAtlasTexture( strFilePath.c_str() );

	if ( pTx )
	{
		return pTx;
	}

	pTx = new CArTexture();
	pTx->Create( strFilePath.c_str() );
	m_AtlasTextures.push_back( pTx );

	return pTx;
}

CArTexture *CArTextureAtlas::FindAtlasTexture( const char *strPath )
{
	vector< CArTexture * >::iterator iterTx = m_AtlasTextures.begin();

	for ( ; iterTx != m_AtlasTextures.end(); ++iterTx )
	{
		CArTexture *pTx = *iterTx;

		if ( strcmp( pTx->GetFilePath().c_str(), strPath ) == 0 )
		{
			return pTx;
		}
	}

	return NULL;
}

CArTextureAtlas::CArTextureAtlas() {}

CArTextureAtlas::~CArTextureAtlas() {}

void CArTextureAtlas::Destroy()
{
	vector< CArTexture * >::iterator iterTx = m_AtlasTextures.begin();
	for ( ; iterTx != m_AtlasTextures.end(); ++iterTx )
	{
		SAFE_DELETE( *iterTx );
	}

	m_AtlasTextures.clear();

	vector< CArVirtualTexture * >::iterator iterVtx = m_VirtualTextures.begin();
	for ( ; iterVtx != m_VirtualTextures.end(); ++iterVtx )
	{
		SAFE_DELETE( *iterVtx );
	}

	m_VirtualTextures.clear();
}

CArBaseTexture *CArTextureAtlas::FindTexture( const char *strPath )
{
	vector< CArVirtualTexture * >::iterator iterTx = m_VirtualTextures.begin();
	for ( ; iterTx != m_VirtualTextures.end(); ++iterTx )
	{
		CArBaseTexture *pTx = *iterTx;

		if ( stricmp( pTx->GetFilePath().c_str(), strPath ) == 0 )
		{
			return pTx;
		}
	}

	return NULL;
}

void CArTextureAtlas::LoadAtlas( const char *strPath )
{
	char strFile[ MAX_PATH ];
	_snprintf( strFile, MAX_PATH, "%s.tai", strPath );
	FILE *pFile = fopen( strFile, "rb" );

	if ( !pFile )
	{
		assert( 0 );

		return;
	}

	fseek( pFile, 0, SEEK_END );
	unsigned long ulSize = ftell( pFile );
	fseek( pFile, 0, SEEK_SET );

	char *pData = new char[ ulSize + 1 ];
	fread( pData, 1, ulSize, pFile );
	pData[ ulSize ] = '\0';

	fclose( pFile );

	//////////////////////////////////////////////////////////////////////////
	// Parse the data.
	//////////////////////////////////////////////////////////////////////////
	
	char *pCur = pData;

	// Skip initial comments.
	while ( *pCur != '\n' )
	{
		if  ( *pCur == '#' )
		{
			while ( *pCur != '\n' )
			{
				pCur++;
			}
		}

		pCur++;
	}

	const char *strSeps = ", \n\r\t=";
	const char *pTok = strtok( pCur, strSeps );

	while ( pTok )
	{
		CArVirtualTexture *pVtex = new CArVirtualTexture();
		m_VirtualTextures.push_back( pVtex );

		// Grab texture name.
		pVtex->m_strFilePath = VarArg( "Data/%s", pTok );
		MakeFilePathCanonical( pVtex->m_strFilePath );

		// Grab atlas name.
		pTok = strtok( NULL, strSeps );
		pVtex->m_pAtlasTexture = CreateAtlasTexture( VarArg( "Data/%s", pTok ) );

		// Skip atlas type.
		pTok = strtok( NULL, strSeps );

		// Make sure we only support 2D textures.
		pTok = strtok( NULL, strSeps );

		if ( strcmp( pTok, "2D" ) != 0 )
		{
			assert( 0 );
		}

		// Grab offset x.
		pTok = strtok( NULL, strSeps );
		pVtex->m_vOffset.x = (float)atof( pTok );

		// Grab offset y.
		pTok = strtok( NULL, strSeps );
		pVtex->m_vOffset.y = (float)atof( pTok );

		// Skip offset depth.
		pTok = strtok( NULL, strSeps );

		// Grab width.
		pTok = strtok( NULL, strSeps );
		pVtex->m_vSize.x = (float)atof( pTok );

		// Grab height.
		pTok = strtok( NULL, strSeps );
		pVtex->m_vSize.y = (float)atof( pTok );

		pTok = strtok( NULL, strSeps );
	}

	SAFE_DELETE_ARRAY( pData );
}


//////////////////////////////////////////////////////////////////////////
// CArTextureManager
//////////////////////////////////////////////////////////////////////////

void CArTextureManager::Initialize() {}
void CArTextureManager::Shutdown() { PurgeTextureCache(); }

void CArTextureManager::PurgeTextureCache()
{
	vector< CArBaseTexture * >::iterator iterTx = m_TextureCache.begin();
	for ( ; iterTx != m_TextureCache.end(); ++iterTx )
	{
		SAFE_DELETE( *iterTx );
	}

	m_TextureCache.clear();

	//////////////////////////////////////////////////////////////////////////
	// Destroy the texture atlases.
	//////////////////////////////////////////////////////////////////////////

	vector< CArTextureAtlas * >::iterator iterAtx = m_AtlasList.begin();
	for ( ; iterAtx != m_AtlasList.end(); ++iterAtx )
	{
		(*iterAtx)->Destroy();
		SAFE_DELETE( *iterAtx );
	}

	m_AtlasList.clear();
}

CArBaseTexture *CArTextureManager::CreateTexture( string strFilePath )
{
	MakeFilePathCanonical( strFilePath );

	// Look in the atlas first.
	if ( m_bUseAtlasVirtualTextures )
	{
		vector< CArTextureAtlas * >::iterator iterAt = m_AtlasList.begin();
		for ( ; iterAt != m_AtlasList.end(); ++iterAt )
		{
			CArTextureAtlas *pAt = *iterAt;
			CArBaseTexture *pTx = pAt->FindTexture( strFilePath.c_str() );

			if ( pTx )
			{
				return pTx;
			}
		}
	}

	// Manage these textures internally.
	CArTexture *pNewTx = new CArTexture();
	pNewTx->Create( strFilePath.c_str() );
	m_TextureCache.push_back( pNewTx );

	return pNewTx;
}

void CArTextureManager::DestroyTexture( CArBaseTexture *pTx )
{
	/*TODO*/
}

void CArTextureManager::AddAtlas( CArTextureAtlas *pAtlas )
{
	m_AtlasList.push_back( pAtlas );
}
