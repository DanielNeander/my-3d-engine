#include "stdafx.h"
#include "GameObject.h"
#include "GameObjectAttribute.h"
#include "GameObjectManager.h"
#include "SceneNode.h"

#include "AI/MsgRoute.h"
#include "AI/StateMachine.h"

class GameObjectAttachment 
{
public:
	GameObjectWPtr          m_pkGameObject;
	noVec3                m_offset;
	bool                    m_bRotation;

	GameObjectAttachment(GameObject *pkGameObject, const noVec3 &offset, bool bRotation)
	{
		m_pkGameObject  = pkGameObject;
		m_offset        = offset;
		m_bRotation     = bRotation;
	}
};

noImplementRootRTTI(GameObject);
GameObject::GameObject( const std::string& filename )
	:m_filename(filename),
	m_stateMachineChange(NO_STATE_MACHINE_CHANGE),
	m_newStateMachine(0),
	m_health(100)
{
	m_weakPointer.SetupWeakPointer(this);

	m_spNode = NULL;
	m_bPaused = false;
	m_name = m_filename.c_str();
	
	CreateTransformComponent();
}

GameObject::~GameObject()
{
	m_newComponents.clear();
	m_components.clear();

	m_attributes.clear();

	std::vector<GameObjectAttachment*>::iterator it = m_attachedObjects.begin();
	for ( ; it != m_attachedObjects.end(); ++it)
		delete *it;
	m_attachedObjects.clear();

	GameObjectManager *pkGameObjectMan = GameObjectManager::Get();

	if( !pkGameObjectMan->IsShuttingDown() )
	{
		GameObjectManager::Get()->RemoveGameObject(this);
	}
	
}

void GameObject::AttachComponent( IGameObjectComponent *pkComponent )
{
	m_components.push_back(pkComponent);
	m_newComponents.push_back(pkComponent);
}

void GameObject::AttachAttribute( IGameObjectAttribute *pkAttribute )
{	
	m_attributes.push_back(pkAttribute);
}

void GameObject::RemoveAttribute( IGameObjectAttribute *pkAttribute )
{	

}

void GameObject::RemoveComponent( IGameObjectComponent *pkComponent )
{
	IGameObjectComponentPtr spComponent = pkComponent;
}

bool GameObject::Update( float fDelta )
{
	//InitNewComponents();

	if( m_bPaused )
		return false;
		
	bool bUpdateOccurred = UpdateComponents(fDelta);

	TickState();
	if( !m_stateMachineList.empty() ) {
		ProcessStateMachineChangeRequests();
		m_stateMachineList.back()->Update();
	}
	
	UpdateInternal(fDelta);

	if( PostUpdateComponents(fDelta) )
	{
		bUpdateOccurred = true;
	}

	RemoveDeadComponents();

	UpdateAttachments();

	return bUpdateOccurred;
}

bool GameObject::ProcessInput()
{
	return true;
}

noVec3 GameObject::GetTranslation()
{
	return m_spTransformComponent->GetTranslation();
}

noMat3 GameObject::GetRotation()
{
	return m_spTransformComponent->GetRotation();
}

float GameObject::GetScale()
{
	return m_spTransformComponent->GetScale();
}

noVec3 GameObject::GetVelocity()
{
	return m_velocity;
}

void GameObject::SetRotation( const noMat3 &rotation )
{
	m_spTransformComponent->SetRotation( rotation );

}

void GameObject::SetTranslation( const noVec3 &translation )
{
	m_spTransformComponent->SetTranslation( translation );

}

void GameObject::SetHeight( float height )
{
	m_spTransformComponent->SetHeight( height );
}

void GameObject::SetScale( float fScale )
{
	m_spTransformComponent->SetScale( fScale );

}

void GameObject::SetVelocity( const noVec3 &velocity )
{
	m_velocity = velocity;
}

const char* GameObject::GetName()
{
	return m_name.c_str();
}

void GameObject::SetName( const std::string &name )
{
	m_name = name;
}

float GameObject::GetUpdateTime()
{
	return 0.0f;
}

void GameObject::AttachGameObject( GameObject *pkGameObject, const noVec3 &offset /*= vec3_zero*/, bool bRotation/*=true*/ )
{
	GameObjectAttachment    *pkAttachment = new GameObjectAttachment(pkGameObject,offset,bRotation);
	m_attachedObjects.push_back( pkAttachment );
}

void GameObject::DetachGameObject( GameObject *pkGameObject )
{
	std::vector<GameObjectAttachment*>::iterator it = m_attachedObjects.begin();
	for (; it != m_attachedObjects.end(); ++it)
	{
		if ((*it)->m_pkGameObject == pkGameObject)
		{
			m_attachedObjects.erase(it);
			break;
		}
	}
}

void GameObject::ResetTranslation( const noVec3 &position )
{
	SetTranslation(position);

	OnTeleported();
}

void GameObject::CreateTransformComponent()
{
	ITransformComponent	*pkComponent = NULL;	
	pkComponent = new DefaultTransformComponent;
	
	if( pkComponent )
		SetTransformComponent(pkComponent);
}

ITransformComponent * GameObject::GetTransformComponent()
{
	return m_spTransformComponent;
}

void GameObject::Initialize()
{

	AttachComponents();
	InitNewComponents();
}

bool GameObject::UpdateComponents( float fDelta )
{
	bool bUpdateOccurred = false;

	if( m_spTransformComponent->IsEnabled() )
		m_spTransformComponent->Update(fDelta);

	std::vector<IGameObjectComponentPtr>::iterator it = m_components.begin();
	for(; it != m_components.end(); ++it)
	{
		IGameObjectComponent *pkComponent = *it;
		if( pkComponent->IsEnabled() )
		{
			if( pkComponent->Update( fDelta ) )
				bUpdateOccurred = true;
		}
	}
	return bUpdateOccurred;
}

bool GameObject::PostUpdateComponents( float fDelta )
{
	bool bUpdateOccurred = false;

	std::vector<IGameObjectComponentPtr>::iterator it = m_components.begin();
	for(; it != m_components.end(); ++it)
	{
		IGameObjectComponent *pkComponent = *it;
		if( pkComponent->IsEnabled() )
		{
			if( pkComponent->PostUpdate( fDelta ) )
				bUpdateOccurred = true;
		}
	}
	if( m_spTransformComponent->IsEnabled() )
		m_spTransformComponent->PostUpdate(fDelta);

	return bUpdateOccurred;
}

void GameObject::UpdateInternal( float fDelta )
{

}

void GameObject::InitNewComponents()
{
	std::vector<IGameObjectComponent*>  badComponents;

	std::vector<IGameObjectComponentPtr>::iterator it = m_newComponents.begin();
	for(; it != m_newComponents.end(); ++it)
	{
		IGameObjectComponent *pkComponent = *it;
	
		if( !pkComponent->Initialize() )
		{
			// remove components that fail to initialize
			badComponents.push_back(pkComponent);
		}
	}

	{	
		std::vector<IGameObjectComponent*>::iterator it = badComponents.begin();
		for(; it != badComponents.end(); ++it)
		{
			RemoveComponent( *it );
		}

		m_newComponents.clear();
	}

}

void GameObject::UpdateAttachments()
{
	noVec3    myPos = GetTranslation();
	noVec3    attachPos;

	std::vector<GameObjectAttachment*>::iterator it = m_attachedObjects.begin();

	std::vector<GameObjectAttachment*> deadAttachments;

	for (; it != m_attachedObjects.end(); it++)
	{
		GameObjectAttachment    *pkAttachment = *it;

		attachPos = myPos + pkAttachment->m_offset;

		if( pkAttachment->m_pkGameObject )
		{
			pkAttachment->m_pkGameObject->SetTranslation(attachPos);
			if( pkAttachment->m_bRotation )
				pkAttachment->m_pkGameObject->SetRotation(GetRotation());
		}
		else
		{
			// need to remove the attachment...
			deadAttachments.push_back( pkAttachment );
		}
	}
	{
		std::vector<GameObjectAttachment*>::iterator it = deadAttachments.begin();
	
		for (; it != deadAttachments.end(); it++)
		{
			delete *it;
		}
		deadAttachments.clear();
	}
}


void GameObject::RemoveDeadComponents()
{

}

void GameObject::SetTransformComponent( ITransformComponent *pkComponent )
{
	if( m_spTransformComponent )
	{
		pkComponent->SetTranslation( m_spTransformComponent->GetTranslation() );
		pkComponent->SetRotation( m_spTransformComponent->GetRotation() );
		pkComponent->SetScale( m_spTransformComponent->GetScale() );
		pkComponent->SetTranslationOffset( pkComponent->GetTranslationOffset() );
	}

	m_spTransformComponent = pkComponent;
	m_newComponents.push_back( pkComponent );
}

void GameObject::OnTeleported()
{
	std::vector<IGameObjectComponentPtr>::iterator it = m_components.begin();
	for(; it != m_components.end(); ++it)
	{
		IGameObjectComponent *pkComponent = *it;
		pkComponent->OnTeleported();
	}

}

IGameObjectComponent * GameObject::GetComponentOfType( const noRTTI &type )
{
	uint32 count = m_components.size();
	IGameObjectComponent *pkCurr;

	if( m_spTransformComponent->IsKindOf(&type) )
		return m_spTransformComponent;

	for(uint32 ui=0;ui<count;ui++)
	{
		pkCurr = m_components.at(ui);
		if( pkCurr->IsKindOf(&type) )
			return pkCurr;
	}

	return NULL;
}

IGameObjectAttribute * GameObject::GetAttributeOfType( const noRTTI &type )
{
	uint32 count = m_attributes.size();
	IGameObjectAttribute *pkCurr;

	for(uint32 ui=0;ui<count;ui++)
	{
		pkCurr = m_attributes.at(ui);
		if( pkCurr->IsKindOf(&type) )
			return pkCurr;
	}

	return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         RequestStateMachineChange

  Description:  Requests that a state machine change take place.

  Arguments:    mch    : the new state machine
                change : the change to take place

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GameObject::RequestStateMachineChange( StateMachine * mch, StateMachineChange change )
{
	ASSERT_MSG( m_stateMachineChange == NO_STATE_MACHINE_CHANGE, "GameObject::RequestStateMachineChange - Change already requested." );
	m_newStateMachine = mch;
	m_stateMachineChange = change;
}

/*---------------------------------------------------------------------------*
  Name:         ResetStateMachine

  Description:  Resets the current state machine.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GameObject::ResetStateMachine( void )
{
	
		ASSERT_MSG( m_stateMachineList.size() > 0, "GameObject::ResetStateMachine - No existing state machine to reset." );
		if( m_stateMachineList.size() > 0 ) {
			StateMachine * mch = m_stateMachineList.back();
			mch->Reset();
		}
}

/*---------------------------------------------------------------------------*
  Name:         ReplaceStateMachine

  Description:  Replaces the current state machine with the provided one
                by popping off the current state machine and pushing the 
				new one.

  Arguments:    mch : the new state machine

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GameObject::ReplaceStateMachine( StateMachine & mch )
{
	ASSERT_MSG( m_stateMachineList.size() > 0, "GameObject::ReplaceStateMachine - No existing state machine to replace." );
	if( m_stateMachineList.size() > 0 ) {
		StateMachine * temp = m_stateMachineList.back();
		m_stateMachineList.pop_back();
		delete( temp );
	}
	PushStateMachine( mch );
}

/*---------------------------------------------------------------------------*
  Name:         QueueStateMachine

  Description:  Queues a state machine behind all others, except for the
                very last one. The last state machine is the "default" and
				should always remain the last.

  Arguments:    mch : the new state machine

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GameObject::QueueStateMachine( StateMachine & mch )
{
	//Insert state machine one up from bottom
	if( m_stateMachineList.size() <= 1 ) {
		PushStateMachine( mch );
	}
	else {
		stateMachineListContainer::iterator i;
		i = m_stateMachineList.begin();
		i++;	//Move iterator past the first entry
		m_stateMachineList.insert(i, &mch);
		//Purposely do not "Reset" state machine until it is active
	}
}

/*---------------------------------------------------------------------------*
  Name:         RequeueStateMachine

  Description:  Requeues the current state machine behind all others, except
                for the very last one.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GameObject::RequeueStateMachine( void )
{
	ASSERT_MSG( m_stateMachineList.size() > 0, "GameObject::RequeueStateMachine - No existing state machines to requeue." );
	if( m_stateMachineList.size() > 1 ) {
		StateMachine * mch = m_stateMachineList.back();
		QueueStateMachine( *mch );
		m_stateMachineList.pop_back();

		//Initialize new state machine
		mch = m_stateMachineList.back();
		mch->Reset();
	}
	else if( m_stateMachineList.size() == 1 ) {
		//Just reinitialize
		StateMachine * mch = m_stateMachineList.back();
		mch->Reset();	
	}
}

void GameObject::PushStateMachine( StateMachine & mch )
{
	m_stateMachineList.push_back( &mch );
	mch.Reset();
}

void GameObject::PopStateMachine( void )
{
	ASSERT_MSG( m_stateMachineList.size() > 1, "GameObject::PopStateMachine - Can't pop last state machine." );
	if( m_stateMachineList.size() > 1 ) {
		StateMachine * mch = m_stateMachineList.back();
		m_stateMachineList.pop_back();
		delete( mch );

		//Initialize new state machine
		mch = m_stateMachineList.back();
		mch->Reset();
	}
}

void GameObject::DestroyStateMachineList( void )
{
	while( m_stateMachineList.size() > 0 ) {
		StateMachine * mch = m_stateMachineList.back();
		m_stateMachineList.pop_back();
		delete( mch );
	}
}

/*---------------------------------------------------------------------------*
  Name:         ProcessStateMachineChangeRequests

  Description:  Checks if a state machine should be changed. It will loop
                until no more state machine change requests have been made.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GameObject::ProcessStateMachineChangeRequests( void )
{
	int safetyCount = 20;
	StateMachineChange change = m_stateMachineChange;
	StateMachine * tempStateMachine = m_newStateMachine;

	while( change != NO_STATE_MACHINE_CHANGE && (--safetyCount >= 0) )
	{
		ASSERT_MSG( safetyCount > 0, "GameObject::ProcessStateMachineChangeRequests - State machiens are flip-flopping in an infinite loop." );

		m_stateMachineChange = NO_STATE_MACHINE_CHANGE;			//Reset
		m_newStateMachine = 0;									//Reset

		switch( change )
		{
		case STATE_MACHINE_RESET:
			g_msgroute.PurgeScopedMsg( m_id ); //Remove all delayed messages addressed to me that are scoped
			ResetStateMachine();
			break;

		case STATE_MACHINE_REPLACE:
			g_msgroute.PurgeScopedMsg( m_id ); //Remove all delayed messages addressed to me that are scoped
			ReplaceStateMachine( *tempStateMachine );
			break;

		case STATE_MACHINE_QUEUE:
			QueueStateMachine( *tempStateMachine );
			break;

		case STATE_MACHINE_REQUEUE:
			g_msgroute.PurgeScopedMsg( m_id ); //Remove all delayed messages addressed to me that are scoped
			RequeueStateMachine();
			break;

		case STATE_MACHINE_PUSH:
			g_msgroute.PurgeScopedMsg( m_id ); //Remove all delayed messages addressed to me that are scoped
			PushStateMachine( *tempStateMachine );
			break;

		case STATE_MACHINE_POP:
			g_msgroute.PurgeScopedMsg( m_id ); //Remove all delayed messages addressed to me that are scoped
			PopStateMachine();
			break;

		default:
			ASSERT_MSG( 0, "GameObject::ProcessStateMachineChangeRequests - invalid StateMachineChange request." );
		}

		//Check if new change
		change = m_stateMachineChange;
		tempStateMachine = m_newStateMachine;
	}
}
