#ifndef MESH_BUNDLE_H
#define MESH_BUNDLE_H

#include "StorageResourceBundle.h"
#include "Proxies.h"
#include <Core/Singleton.h>
#include <Renderer/noMesh.h>
#include <Renderer/noRenderSystemDX9.h>

class CMeshBundle :	public CStorageResourceBundle<noMesh>,
						public CSingleton<CMeshBundle>,
						public noD3DResource 
{
public:
	virtual void createResource();	
	virtual void deleteResource();

protected:
	virtual noMesh* loadResourceById( const CResourceId& id, const CResourceId& fullName );
	virtual void deleteResource( noMesh& resource ) {
		//if( resource.isCreated() )
		//resource.deleteResource();
		delete &resource;
	}

private:
	IMPLEMENT_SIMPLE_SINGLETON(CMeshBundle);
	CMeshBundle();
	virtual ~CMeshBundle() { clear(); };

	/// @return false on not found
	bool loadMesh( const CResourceId& id, const CResourceId& fullName, noMesh& mesh ) const;



};

/// Shortcut macro
#define RGET_MESH(rid) CMeshBundle::GetInstance().getResourceById(rid)


#endif