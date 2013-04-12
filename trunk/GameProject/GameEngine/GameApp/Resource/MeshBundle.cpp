#include "stdafx.h"
#include <GameApp/Util/StringHelper.h>
#include <GameApp/GameApp.h>
#include <GameApp/Utility.h>
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
		Mesh& res = *it->second;
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
		//ASSERT( res.isCreated() );
	}
}

void CMeshBundle::deleteResource()
{
	// unload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		Mesh& res = *it->second;
//		ASSERT( res.isCreated() );
		//res.deleteResource();
	}

}

Mesh* CMeshBundle::loadResourceById( const CResourceId& id, const CResourceId& fullName )
{
	Mesh* mesh = NULL;
	if( CStringHelper::endsWith( fullName.getUniqueName(), ".psk" ) || CStringHelper::endsWith( fullName.getUniqueName(), ".PSK" ) ) {
	}
	else if( CStringHelper::endsWith( fullName.getUniqueName(), ".m2" ) || CStringHelper::endsWith( fullName.getUniqueName(), ".M2" ) ) {
	   mesh = new M2Mesh();
		
	}else if( CStringHelper::endsWith( fullName.getUniqueName(), ".sdkmesh" ) || CStringHelper::endsWith( fullName.getUniqueName(), ".SDKMESH" ) ) {
		mesh = new CDXUTSDKMesh;
	}	
	bool ok = loadMesh( id, fullName, *mesh );
	if( !ok ) {
		delete mesh;
		return NULL;
	}
	return mesh;

}

CMeshBundle::CMeshBundle()
{
	addExtension( ".sdkmesh" );
	addExtension( ".psk" );
	addExtension( ".m2" ); // legacy
}

bool CMeshBundle::loadMesh( const CResourceId& id, const CResourceId& fullName, Mesh& mesh ) const
{
	if( CStringHelper::endsWith( fullName.getUniqueName(), ".psk" ) || CStringHelper::endsWith( fullName.getUniqueName(), ".PSK" ) ) {

	}
	else 
	if( CStringHelper::endsWith( fullName.getUniqueName(), ".m2" ) || CStringHelper::endsWith( fullName.getUniqueName(), ".M2" ) ) {
		//M2Mesh* m2_mesh = static_cast<M2Mesh*>(mesh);		
	}
	else if( CStringHelper::endsWith( fullName.getUniqueName(), ".sdkmesh" ) || CStringHelper::endsWith( fullName.getUniqueName(), ".SDKMESH" ) ) {
		CDXUTSDKMesh* sdkMesh = static_cast<CDXUTSDKMesh*>(&mesh);
		sdkMesh->Create(D3D11Dev(), AnsiToWString(fullName.getUniqueName().c_str()).c_str());
	}

	return true;
}


void CMeshBundle::activateResource()
{
}

void CMeshBundle::passivateResource()
{
}
