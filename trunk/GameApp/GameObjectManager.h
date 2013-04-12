#pragma once 

#include "GameObject.h"
#include "BaseCamera.h"

#define MAX_PLAYER_INPUTS 4



typedef std::vector<GameObject*>	dbCompositionList;

class GameObjectManager : public RefCounter
{
public:

	~GameObjectManager();

	static GameObjectManager *Create();
	static void Destroy();	
	static GameObjectManager *Get();
	
	GameObject* Find( objectID id );
	objectID GetIDByName( const char* name );
	objectID GetNewObjectID( void );

	void ComposeList( dbCompositionList & list, unsigned int type = 0 );

	bool GetGameObjectsWithAttribute(const noRTTI &type,dbCompositionList &list);


	void InitializeGameObject(GameObject *pkGameObject);

	void Update(float fDelta);
	bool ProcessInput(float fDelta);

	void AddGameObject(GameObject *pkGameObject);
	void RemoveGameObject(GameObject *pkGameObject);

	void DestroyGameObject(GameObject *pkGameObject);

	void SetCamera(BaseCamera *pkCamera);
	BaseCamera *GetCamera();

	bool IsShuttingDown()               { return m_bShuttingDown; }

	size_t Size() const { return m_allEntities.size(); }

protected:
	static GameObjectManager                *s_pkGameObjectManager;
	bool                                    m_bPaused;
	bool                                    m_bShuttingDown;

	//GameObjectList                          m_updateEntities;
	//GameObjectList                          m_newEntities;
	std::map<int, GameObject*>                 m_allEntities;

	BaseCamera*	                            m_pkGameCamera;
	//GameObjectWPtr                          m_pkPlayerInputObject[MAX_PLAYER_INPUTS];

	GameObjectManager();


	void RemoveGameObjectInternal(GameObject *pkGameObject);
	void InitializeGameObjectInternal(GameObject *pkGameObject);
private:
	objectID m_nextFreeID;

};


MSmartPointer(GameObjectManager);