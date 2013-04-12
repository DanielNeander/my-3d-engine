#pragma once 

#include <EngineCore/RefCount.h>
#include "WeakPointer.h"
#include "FSM.h"

#include "AI/AICommon.h"

MSmartPointer(ITransformComponent);
MSmartPointer(IGameObjectComponent);
MSmartPointer(SceneNode);

class IGameObjectComponent;
MSmartPointer(IGameObjectAttribute);


//Add new object types here (bitfield mask - objects can be combinations of types)
#define OBJECT_Ignore_Type  (0)
#define OBJECT_Gameflow     (1<<1)
#define OBJECT_Character    (1<<2)
#define OBJECT_NPC          (1<<3)
#define OBJECT_Player       (1<<4)
#define OBJECT_Enemy        (1<<5)
#define OBJECT_Weapon       (1<<6)
#define OBJECT_Item         (1<<7)
#define OBJECT_Projectile   (1<<8)

enum StateMachineChange {
	NO_STATE_MACHINE_CHANGE,
	STATE_MACHINE_RESET,
	STATE_MACHINE_REPLACE,
	STATE_MACHINE_QUEUE,
	STATE_MACHINE_REQUEUE,
	STATE_MACHINE_PUSH,
	STATE_MACHINE_POP
};

class StateMachine;

class IGameObjectMessage 
{
public:
	virtual ~IGameObjectMessage()   {} 
	virtual uint32 GetId()=0;
};

class GameObject : public FSMObject, public RefCounter
{
public:
	noDeclareRootRTTI(GameObject);


	GameObject(const std::string& filename);
	//GameObject(M2Loader* pLoader);	


	virtual ~GameObject();

	void AttachComponent(IGameObjectComponent *pkComponent);

	void AttachAttribute(IGameObjectAttribute *pkAttribute);
	void RemoveAttribute(IGameObjectAttribute *pkAttribute);
	void RemoveComponent(IGameObjectComponent *pkComponent);

	virtual void PreUpdate()                                    { }
	virtual bool Update(float fDelta);
	virtual void PostUpdate()                                   { }
	virtual bool ProcessInput();

	WeakPointerData* GetWeakPointerData()                       { return m_weakPointer.GetWeakPointerData(); }

	virtual noVec3 GetTranslation();
	virtual noMat3 GetRotation();
	virtual float GetScale();
	virtual noVec3 GetVelocity();

	virtual void SetRotation(const noMat3 &rotation);
	virtual void SetTranslation(const noVec3 &translation);
	virtual void SetHeight(float height);
	virtual void SetScale(float fScale);
	virtual void SetVelocity(const noVec3 &velocity);

	/*void Hide();
	void Show();
	bool IsHidden();

	void Pause();
	void Resume();
	void TogglePause();*/

	IGameObjectComponent *GetComponentOfType(const noRTTI &type);
	IGameObjectAttribute *GetAttributeOfType(const noRTTI &type);


	const char* GetName();
	void SetName(const std::string &name);

	virtual bool HandleMessage(IGameObjectMessage *pkMessage)           { return false; }

	virtual float GetUpdateTime();
	
	void AttachGameObject(GameObject *pkGameObject, const noVec3 &offset = vec3_zero, bool bRotation=true);
	void DetachGameObject(GameObject *pkGameObject);

	virtual void ResetTranslation(const noVec3 &position);

	virtual void CreateTransformComponent();

	ITransformComponent *GetTransformComponent();

	//virtual bool OnCollision(const GameObjectCollision &collisionArray);

	SceneNode *GetNode() { return m_spNode; }

	void	 SetID(objectID id)						{ m_id = id; }
	objectID GetID( void )							{ return( m_id ); }
	void SetType(unsigned int type)					{ m_type = type; }
	unsigned int GetType( void )					{ return( m_type ); }
	//State Machine access
	StateMachine* GetStateMachine( void )			{ if( m_stateMachineList.empty() ) { return( 0 ); } else { return( m_stateMachineList.back() ); } }
	void RequestStateMachineChange( StateMachine * mch, StateMachineChange change );
	void ResetStateMachine( void );
	void ReplaceStateMachine( StateMachine & mch );
	void QueueStateMachine( StateMachine & mch );
	void RequeueStateMachine( void );
	void PushStateMachine( StateMachine & mch );
	void PopStateMachine( void );
	void DestroyStateMachineList( void );

	int GetHealth( void )							{ return( m_health ); }
	void SetHealth( int health )					{ if( health > 0 ) { m_health = health; } else { m_health = 0; } }
	bool IsAlive( void )							{ return( m_health > 0 ); }

protected:
	virtual void Initialize();
	virtual void AttachComponents()     { } // convenient place for subclasses to attach components

	virtual bool UpdateComponents(float fDelta);
	virtual bool PostUpdateComponents(float fDelta);
	virtual void UpdateInternal(float fDelta);

	virtual void LoadModel() {}

	void InitNewComponents();

	virtual void UpdateAttachments();

	void RemoveDeadComponents();        // remove unwanted components

	void SetTransformComponent(ITransformComponent *pkComponent);

	virtual void OnTeleported();
	
protected:
	WeakPointerMaster                           	m_weakPointer;
	bool                               m_bPaused;
	
	ITransformComponentPtr		           m_spTransformComponent;	
	std::vector<IGameObjectComponentPtr>      m_components;
	std::vector<IGameObjectComponentPtr>      m_newComponents;
	std::vector<IGameObjectAttributePtr>      m_attributes;
	std::vector<class GameObjectAttachment*> m_attachedObjects;

	std::string								m_name;
	std::string								m_filename;

	noVec3	m_velocity;
	int m_health;

	
	SceneNodePtr							m_spNode;

	friend class GameObjectManager;

private:
	typedef std::list<StateMachine*> stateMachineListContainer;
	objectID m_id;									//Unique id of object (safer than a pointer).
	unsigned int m_type;							//Type of object (can be combination).
	
	stateMachineListContainer m_stateMachineList;	//List of state machines. Top one is active.

	StateMachineChange m_stateMachineChange;		//Directions for any pending state machine changes
	StateMachine * m_newStateMachine;				//A state machine that will be added to the queue later

	void ProcessStateMachineChangeRequests( void );
};
noWeakPointer(GameObject);
