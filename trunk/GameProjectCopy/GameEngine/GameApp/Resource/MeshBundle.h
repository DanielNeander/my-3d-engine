#ifndef MESH_BUNDLE_H
#define MESH_BUNDLE_H

#include "StorageResourceBundle.h"
#include "DeviceResource.h"
#include "Proxies.h"
#include <EngineCore/Singleton.h>
#include <GameApp/SDKMeshLoader.h>
#include <GameApp/Mesh.h>
#include <GameApp/M2Mesh.h>

class CMeshBundle :	public CStorageResourceBundle<Mesh>,
						public Singleton<CMeshBundle>,
						public IDeviceResource
{
public:
	virtual void createResource();	
	virtual void deleteResource();
	void activateResource();
	void passivateResource();

protected:
	virtual Mesh* loadResourceById( const CResourceId& id, const CResourceId& fullName );
	virtual void deleteResource( Mesh& resource ) {
		//if( resource.isCreated() )
		//resource.deleteResource();
		delete &resource;
	}

private:
	IMPLEMENT_SIMPLE_SINGLETON(CMeshBundle);
	CMeshBundle();
	virtual ~CMeshBundle() { clear(); };

	/// @return false on not found
	bool loadMesh( const CResourceId& id, const CResourceId& fullName, Mesh& mesh ) const;
	

};

/// Shortcut macro
#define RGET_MESH(rid) CMeshBundle::GetInstance().getResourceById(rid)


#endif