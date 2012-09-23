#include "DXUT.h"
#include <Core/StringHelper.h>
#include "MeshBundle.h"


void CMeshBundle::createResource()
{
	const TStringVector& ext = getExtensions();
	const TStringVector& dir = getDirectories();
	int ne = ext.size();
	int nd = dir.size();

	// reload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		noMesh& res = *it->second;
		//ASSERT( !res.isCreated() );
		// try all directories
		for( int d = 0; d < nd; ++d ) {
			// try all extensions
			for( int e = 0; e < ne; ++e ) {
				CResourceId fullid( dir[d] + it->first.getUniqueName() + ext[e] );
				bool ok = loadMesh( it->first, fullid, res );
				if( ok )
					break;
			}
		}
		// maybe ID was full name already
		loadMesh( it->first, it->first, res );
//		ASSERT( res.isCreated() );
	}
}

void CMeshBundle::deleteResource()
{
	// unload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		noMesh& res = *it->second;
//		ASSERT( res.isCreated() );
	//	res.deleteResource();
	}

}

noMesh* CMeshBundle::loadResourceById( const CResourceId& id, const CResourceId& fullName )
{
	noMesh* mesh = new noMesh();
	bool ok = loadMesh( id, fullName, *mesh );
	if( !ok ) {
		delete mesh;
		return NULL;
	}
	return mesh;

}

CMeshBundle::CMeshBundle()
{
	addExtension( ".psk" );
	addExtension( ".md2" ); // legacy
}

bool CMeshBundle::loadMesh( const CResourceId& id, const CResourceId& fullName, noMesh& mesh ) const
{
	if( CStringHelper::endsWith( fullName.getUniqueName(), ".psk" ) || CStringHelper::endsWith( fullName.getUniqueName(), ".PSK" ) ) {

	}
	else 
	if( CStringHelper::endsWith( fullName.getUniqueName(), ".md2" ) || CStringHelper::endsWith( fullName.getUniqueName(), ".MD2" ) ) {

	}

	return true;
}