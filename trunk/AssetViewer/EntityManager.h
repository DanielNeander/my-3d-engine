#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <U2Lib/src/memory/U2MemObj.h>
#include <vector>

U2SmartPointer(U2Entity);

class U2EntityManager 
{
public:
	static U2EntityManager* m_pInst;

	std::vector< U2EntityPtr > m_vEntities;

	U2EntityManager();

	~U2EntityManager();

	// Add an entity to be managed.
	void AddEntity( U2Entity *pEnt );

	// Remove this Entity from the manager.
	void RemoveEntity( U2Entity *pEnt );

	// Release all the managed entities.
	void ReleaseEntities();

	//void RenderScene();
	void UpdateScene();
};

#endif