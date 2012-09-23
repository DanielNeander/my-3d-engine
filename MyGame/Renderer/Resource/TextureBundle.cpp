#include "DXUT.h"
#include "TextureBundle.h"

void CTextureBundle::createResource()
{
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CD3DTexture& res = *it->second;
		ASSERT( res.isNull() );
		CD3DTexture* n = tryLoadResourceById( it->first );
		ASSERT( n );
		res.setObject( n->getObject() );
		delete n;
		ASSERT( !res.isNull() );
	}
}

void CTextureBundle::deleteResource()
{
	// unload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CD3DTexture& res = *it->second;
		ASSERT( !res.isNull() );
		res.getObject()->Release();
		res.setObject( NULL );
	}

}

CD3DTexture* CTextureBundle::loadResourceById( const CResourceId& id, const CResourceId& fullName )
{
	IDirect3DTexture9* texture = loadTexture( id, fullName );
	if( !texture )
		return NULL;
	return new CD3DTexture( texture );
}

CTextureBundle::CTextureBundle()
{
	addExtension( ".dds" );
	addExtension( ".png" );
	addExtension( ".jpg" );
	addExtension( ".tga" );
	addExtension( ".bmp" );
}

IDirect3DTexture9* CTextureBundle::loadTexture( const CResourceId& id, const CResourceId& fullName ) const
{
	IDirect3DTexture9* texture = NULL;

	// load mips from file in DDS case; construct full mip chain for other
	// formats
	DWORD mipLevels = 0;
	//if( CStringHelper::endsWith( fullName.getUniqueName(), ".dds" ) )
	mipLevels = D3DX_FROM_FILE;

	HRESULT hres = D3DXCreateTextureFromFileEx(
		g_renderSystem->GetD3DDevice(),
#ifdef UNICODE
		ToUnicode(fullName.getUniqueName().c_str()),
#else 
		fullName.getUniqueName().c_str(),
#endif
		D3DX_DEFAULT, D3DX_DEFAULT,
		mipLevels,
		0,
		D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		NULL,
		NULL,
		&texture );

	if( !SUCCEEDED( hres ) )
		return NULL;

	ASSERT( texture );
	LOG_DEBUG << "texture loaded '" <<  id.getUniqueName().c_str()  << "'" ;
	return texture;
}