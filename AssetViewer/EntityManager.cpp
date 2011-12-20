#include "stdafx.h"
#include "EntityManager.h"
#include "Entity.h"
using namespace std;

U2EntityManager::U2EntityManager()
{

}

// Add an entity to be managed.
void U2EntityManager::AddEntity( U2Entity *pEnt )
{
	m_vEntities.push_back( pEnt );
}

// Remove this Entity from the manager.
void U2EntityManager::RemoveEntity( U2Entity *pEnt )
{
	vector< U2EntityPtr >::iterator iterEnt = m_vEntities.begin();
	for ( ; iterEnt != m_vEntities.end(); ++iterEnt )
	{
		if ( (*iterEnt) == pEnt )
		{
			m_vEntities.erase( iterEnt, iterEnt );
		}
	}
}

// Release all the managed entities.
void U2EntityManager::ReleaseEntities()
{
	vector< U2EntityPtr >::iterator iterEnt = m_vEntities.begin();
	for ( ; iterEnt != m_vEntities.end(); ++iterEnt )
	{
		(*iterEnt) = 0;
	}
	m_vEntities.clear();
}