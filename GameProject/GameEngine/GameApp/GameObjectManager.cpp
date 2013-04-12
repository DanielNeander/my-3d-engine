#include"stdafx.h"
#include "GameObjectManager.h"


GameObjectManager *GameObjectManager::s_pkGameObjectManager = NULL;

GameObjectManager::~GameObjectManager()
{
	m_bShuttingDown = true;

	std::map<int, GameObject*>::iterator b = m_allEntities.begin();
	std::map<int, GameObject*>::iterator e = m_allEntities.end();
	for ( ; b != e; b++)
		delete (*b).second;

	m_allEntities.clear();
	//m_updateEntities.clear();
	
	s_pkGameObjectManager = NULL;
}

GameObjectManager * GameObjectManager::Create()
{
	s_pkGameObjectManager = new GameObjectManager();
	s_pkGameObjectManager->SetCamera(NULL);
	return s_pkGameObjectManager;
}

void GameObjectManager::Destroy()
{
	delete s_pkGameObjectManager;
	s_pkGameObjectManager = NULL;
}

GameObjectManager * GameObjectManager::Get()
{
	return s_pkGameObjectManager;
}

void GameObjectManager::InitializeGameObject( GameObject *pkGameObject )
{
	InitializeGameObjectInternal(pkGameObject);
}

void GameObjectManager::Update( float fDelta )
{
	GameObject *pkGameObject;
	
	std::map<int, GameObject*>::iterator b = m_allEntities.begin();
	std::map<int, GameObject*>::iterator e = m_allEntities.end();
	for ( ; b != e; b++)
	{		
		pkGameObject = (*b).second;
		pkGameObject->Update(fDelta);
	}

	if( m_pkGameCamera )
		m_pkGameCamera->Update(fDelta);

	b = m_allEntities.begin();
	e = m_allEntities.end();
	for ( ; b != e; b++)
	{		
		pkGameObject = (*b).second;
		pkGameObject->PostUpdate();
	}
}

bool GameObjectManager::ProcessInput( float fDelta )
{
	if (m_bPaused)
	{
		return false;
	}

	GameObject *pkGameObject;
	bool        inputHandled = false;

	std::map<int, GameObject*>::iterator b = m_allEntities.begin();
	std::map<int, GameObject*>::iterator e = m_allEntities.end();
	for ( ; b != e; b++)
	{		
		pkGameObject = (*b).second;
		if( pkGameObject->ProcessInput() )
			inputHandled = true;
	}
}

void GameObjectManager::AddGameObject( GameObject *pkGameObject )
{
	InitializeGameObjectInternal(pkGameObject);
	//m_newEntities.Add(pkGameObject); // 초기화 할 때 사용
}

void GameObjectManager::RemoveGameObject( GameObject *pkGameObject )
{
	std::map<int, GameObject*>::iterator iter = m_allEntities.find(pkGameObject->GetID());			

	if (iter != m_allEntities.end())
		m_allEntities.erase(pkGameObject->GetID());
}

void GameObjectManager::DestroyGameObject( GameObject *pkGameObject )
{

}

void GameObjectManager::SetCamera( BaseCamera *pkCamera )
{
	m_pkGameCamera = pkCamera;
}

BaseCamera * GameObjectManager::GetCamera()
{
	return m_pkGameCamera;
}

GameObjectManager::GameObjectManager()
{
	m_bPaused       = true;
	m_bShuttingDown = false;
	m_nextFreeID = 0;
}

void GameObjectManager::RemoveGameObjectInternal( GameObject *pkGameObject )
{

}

void GameObjectManager::InitializeGameObjectInternal( GameObject *pkGameObject )
{	
	m_allEntities.insert(std::make_pair(pkGameObject->GetID(), pkGameObject));
	pkGameObject->Initialize();
}

GameObject* GameObjectManager::Find( objectID id )
{
	std::map<int, GameObject*>::iterator b = m_allEntities.begin();
	std::map<int, GameObject*>::iterator e = m_allEntities.end();
	for ( ; b != e; b++)
	{		
		GameObject* pkGameObject = (*b).second;
		if (pkGameObject->GetID() == id)
			return pkGameObject;
	}
	return NULL;
}

objectID GameObjectManager::GetIDByName( const char* name )
{
	std::map<int, GameObject*>::iterator b = m_allEntities.begin();
	std::map<int, GameObject*>::iterator e = m_allEntities.end();
	for ( ; b != e; b++)
	{		
		GameObject* pkGameObject = (*b).second;
		if (strcmp(pkGameObject->GetName(), name) == 0)
			return pkGameObject->GetID();
	}

	return (INVALID_OBJECT_ID);
}

objectID GameObjectManager::GetNewObjectID( void )
{
	return( m_nextFreeID++ );
}

/*---------------------------------------------------------------------------*
  Name:         ComposeList

  Description:  Compose a list of objects given certain type.

  Arguments:    list   : the list to fill with the result of the operation
                type   : the type of object to add to the list (optional)

  Returns:      None. (The result is stored in the list argument.)
 *---------------------------------------------------------------------------*/
void GameObjectManager::ComposeList( dbCompositionList & list, unsigned int type /*= 0 */ )
{
	GameObject* pkGameObject;
	std::map<int, GameObject*>::iterator b = m_allEntities.begin();
	std::map<int, GameObject*>::iterator e = m_allEntities.end();
	for ( ; b != e; b++)
	{		
		pkGameObject = (*b).second;
		if( type == OBJECT_Ignore_Type || pkGameObject->GetType() & type )
		{	//Type matches
			list.push_back(pkGameObject);
		}
	}
}


bool GameObjectManager::GetGameObjectsWithAttribute(const noRTTI &type,dbCompositionList &list)
{
	uint32 count = m_allEntities.size();
	GameObject *pkGameObject;

	std::map<int, GameObject*>::iterator b = m_allEntities.begin();
	std::map<int, GameObject*>::iterator e = m_allEntities.end();
	for ( ; b != e; b++)
	{		
		pkGameObject = (*b).second;
		if( pkGameObject->GetAttributeOfType(type) )
			list.push_back( pkGameObject );
	}

	if( list.size()>0 )
		return true;

	return false;
}