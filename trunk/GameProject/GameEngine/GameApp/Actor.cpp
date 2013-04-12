#include "stdafx.h"
#include "GameApp/GameApp.h"
#include "GameObjectUtil.h"
#include "GameObjectAttribute.h"
#include "AnimationComponent.h"
#include "GameObjectManager.h"
#include "ModelLib/Model_M2Format.h"
#include "ModelLib/M2AnimationManager.h"
#include "AnimationSystem.h"

#include "AI/AIConfig.h"
#include "AI/SteeringBehavior.h"
#include "AI/AIPath.h"
#include "AI/GameWorld.h"
#include "DebugDraw.h"
#include "AI/NavDebugDraw.h"


#include "SceneNode.h"
#include "TestObjects.h"
#include "Actor.h"

//treats a window as a toroid that can contain negative values
inline void WrapAround(noVec2 &pos, int MinX, int MinY, int MaxX, int MaxY)
{
	if (pos.x > MaxX) {pos.x = MinX;}

	if (pos.x < MinX)    {pos.x = (double)MaxX;}

	if (pos.y < MinY)    {pos.y = (double)MaxY;}

	if (pos.y > MaxY) {pos.y = MinY;}
}


noImplementRTTI(Actor,GameObject);
Actor::Actor( const std::string filename ) : GameObject( filename)
{

}

void Actor::update( float elapsedTime )
{
}

ActorController::ActorController( GameObject* pkGameObject, GameWorld* pWorld, noVec3 position, double rotation, noVec3 velocity, double mass, double max_force, double max_speed, double max_turn_rate)
	: StateMachine( pkGameObject )
	, m_vHeading(noVec3(sin(rotation), 0.0f, -cos(rotation))), m_vVelocity(velocity),
	m_dMass(mass), m_vSide(m_vHeading.Perp()), m_dMaxSpeed(max_speed),
	m_dMaxTurnRate(max_turn_rate), m_dMaxForce(max_force)
	,m_vSmoothedHeading(vec3_zero), m_bSmoothingOn(true)
{
	m_World = pWorld;
	m_vPos = position;
	m_pSteering = new SteeringBehavior(this);

	m_pHeadingSmoother = new Smoother<noVec3>(AICON.NumSamplesForSmoothing, vec3_zero);
	m_pFrameSmoother = new Smoother<double>(AICON.NumFrameSamplesForSmoothing, 0.0);

	mHasPath = false;
	mIsWalking = false;
	mFindingPath = false;	

	mPathEnd = vec3_zero;
	mPathStart = vec3_zero;
	//mNextPosition = vec3_zero;
	//mCurrentPosition = vec3_zero;
	mIdleTimerToChange = 100.0f;
	mIdleTimerCurrent = 0.0f;

	dd_ = new NavDebugDraw;
	dd_->CreateVertexBuffer(NULL, 1000, sizeof(DebugVertex));
	dd_->setOffset(0.15f);
	//ddMain->CreateIndexBuffer(geom->getMesh()->getTris(), geom->getMesh()->getTriCount() * 3);
	dd_->SetupShader();

	dd2_ = new NavDebugDraw;
	dd2_->CreateVertexBuffer(NULL, 1000, sizeof(DebugVertex));
	dd2_->setOffset(0.35f);
	//ddMain->CreateIndexBuffer(geom->getMesh()->getTris(), geom->getMesh()->getTriCount() * 3);
	dd2_->SetupShader();

	mAttackRange = 10.f;
		
	m_StateMgr_ = new SimpleStateManager;
	m_StateMgr_->RegisterState(STATE_Idle, new IdleState);
	m_StateMgr_->RegisterState(STATE_Move, new MoveState);
	m_StateMgr_->RegisterState(STATE_Attack, new AttackState);
	m_StateMgr_->RegisterState(STATE_Jump, new JumpState);
	m_StateMgr_->RegisterState(STATE_Wound, new WoundState);
	m_StateMgr_->RegisterState(STATE_Die, new DeathState);	

	pathfinding_ = new NavigationMeshAdapter(pkGameObject);

}

ActorController::~ActorController()
{
	SAFE_DELETE(m_StateMgr_);
	SAFE_DELETE(m_pSteering);
	delete m_pHeadingSmoother;
	delete m_pFrameSmoother;
}

void ActorController::update( float elapsedTime )
{
	if (GetState() == STATE_Initialize || 
		GetState() == STATE_Die || GetState() == STATE_Idle || GetState() == STATE_Attack || GetState() == STATE_Wound)
		return;
	
	float time_elapsed = m_pFrameSmoother->Update(elapsedTime) * 0.5f;
	//Actor::Update(elapsedTime);
	noVec3 oldPos = m_owner->GetTranslation();
	
	if(!m_pSteering->PathDone() && !mFindingPath)
	{
		//update the time elapsed
		m_dTimeElapsed = time_elapsed;

		//keep a record of its old position so we can update its cell later
		//in this method
		noVec3 OldPos = Pos();

		noVec3 SteeringForce;

		//calculate the combined force from each steering behavior in the 
		//vehicle's list
		SteeringForce = m_pSteering->Calculate();

		//Acceleration = Force/Mass
		noVec3 acceleration = SteeringForce / m_dMass;

		//update velocity
		m_vVelocity += acceleration * time_elapsed; 

		//make sure vehicle does not exceed maximum velocity
		m_vVelocity.Truncate(m_dMaxSpeed);

		//update the position
		m_vPos += m_vVelocity * time_elapsed * 0.1f;

		//update the heading if the vehicle has a non zero velocity
		if (m_vVelocity.LengthSqr() > 0.00000001)
		{    
			noVec3 temp=m_vVelocity;
			temp.Normalize();
			m_vHeading = temp;
			m_vSide = m_vHeading.Perp();
		}

		EnforceNonPenetrationConstraint(this, World()->Agents());

		//treat the screen as a toroid
		//WrapAround(m_vPos, m_World->cxClientMin(), m_World->cyClientMin(), m_World->cxClient(), m_World->cyClient());

		//update the vehicle's current cell if space partitioning is turned on
		if (m_pSteering->isSpacePartitioningOn())
		{
			World()->CellSpace()->UpdateEntity(this, OldPos);
		}

		m_owner->SetTranslation(m_vPos);

		if (m_vVelocity != vec3_zero)
		{
			if (isSmoothingOn())
			{
				m_vSmoothedHeading = m_pHeadingSmoother->Update(Heading());
				//mBodyNode->setDirection(noVec3(m_vSmoothedHeading.x, 0, m_vSmoothedHeading.y), Node::TransformSpace::TS_WORLD, noVec3::UNIT_Z);
				noMat3 rot;
				MathHelpers::FaceDirection(rot, noVec3(m_vSmoothedHeading.x, 0, m_vSmoothedHeading.z));
				noRotation rot2(vec3_zero, noVec3(0, 1, 0), 180);
				rot = rot * rot2.ToMat3();
				m_owner->SetRotation(rot);			
			}
			else
			{
				//mBodyNode->setDirection(noVec3(m_vHeading.x, 0, m_vHeading.y), Node::TransformSpace::TS_WORLD, noVec3::UNIT_Z);
				noMat3 rot;
				MathHelpers::FaceDirection(rot, noVec3(m_vHeading.x, 0, m_vHeading.z));			
				noRotation rot2(vec3_zero, noVec3(0, 1, 0), 180);
				rot = rot * rot2.ToMat3();
				m_owner->SetRotation(rot);			
			}
		}

		//mHasMoved = true;
	}
	else if(m_pSteering->PathDone() && !mFindingPath)
	{
		m_pSteering->FollowPathOff();
		SetVelocity(vec3_zero);
		mFindingPath = true;
		//sendFindNewPathMessage();
	}
	else
	{
		SetVelocity(vec3_zero);
	}

	// -- RUN ANIMS LOGIC	
	if ( m_owner->GetTranslation()  == oldPos)	
	{		
		SendMsgToMe(MSG_Idle);
		mIsWalking = false;
	}
	else if ( m_owner->GetTranslation() != oldPos && !mIsWalking)
	{
		SimpleStateManager* pStateMgr = this->GetStateMgr();
		MoveState* move = (MoveState*)pStateMgr->GetState(STATE_Move);

		//SendMsgToMe(MSG_Move);
		AnimationComponent* pAnimComp = GetGameObjectComponent(m_owner, AnimationComponent);		
		move->WalkForward(m_owner);

		mIsWalking = true;
	}
	/*if(mBodyNode->getPosition() == OldBodyPos)
	{
		setRunningAnimEnd();
		mIsWalking = false;
	}
	else if(mBodyNode->getPosition() != OldBodyPos && !mIsWalking)
	{
		setRunningAnimStart();
		mIsWalking = true;
	}*/	
	if (pathfinding_)
		pathfinding_->Update(time_elapsed);
}

void ActorController::DrawDebug()
{
	//pickPos_ = PickUtil::GetPickVert();	
	int col = duRGBA(0,192,255,64);
	noVec3 trans(m_vPos.x, m_owner->GetTranslation().y, m_vPos.y);	
	//dd_->vertex(trans.TofloatPtr(), col);	
	noVec3 worldPos = trans;

	//noVec3 targetPos = trans + noVec3(m_vHeading.x, 0, m_vHeading.y) * 3;
	//dd_->vertex(targetPos.TofloatPtr(), col);
	duDebugDrawCircle(dd_, worldPos.x, m_owner->GetTranslation().y, worldPos.z, m_dBoundingRadius, col, 1.0f);	

	dd2_->begin(DU_DRAW_LINES_STRIP);
	dd2_->vertex(trans.ToFloatPtr(), col);
	noVec3 targetPos = trans + noVec3(Steering()->Force().x, 0, Steering()->Force().y) * 4;
	//targetPos = rtrans;
	dd2_->vertex(targetPos.ToFloatPtr(), col);
	dd2_->end();
}

void ActorController::findStartEndPositions()
{
	//srand(g_time.GetCurTime());
	// randomly generate an endpoint for a path from present location
	setPathStart(m_owner->GetTranslation().x, m_owner->GetTranslation().y, m_owner->GetTranslation().z);

	int negpos = RandDelay(0.0f, 10.0f);

	if(negpos > 5)
		negpos = 1;
	else
		negpos = -1;

	float XVal = 0.0f;
	float ZVal = 0.0f;

	/*if(SharedData::getSingleton().m_AppMode == APPMODE_TERRAINSCENE)
	{
		setPathStart(mBodyNode->getPosition().x, mGndHgt, mBodyNode->getPosition().z);

		XVal = (mBodyNode->getPosition().x + (RandDelay(100.0f, 5500.0f) / negpos));
		ZVal = (mBodyNode->getPosition().z + (RandDelay(100.0f, 5500.0f) / negpos));

		float gndAtEnd = m_sample->getInputGeom()->getMesh()->mTerrainGroup->getHeightAtWorldPosition(noVec3(XVal, 5000, ZVal));

		setPathEnd(XVal, gndAtEnd, ZVal);
	}*/

	//if( (SharedData::getSingleton().m_AppMode != APPMODE_TERRAINSCENE) )
	//{
		//XVal = (m_owner->GetTranslation().x + (RandDelay(7.5f, 1000.0f) / negpos));
		//ZVal = (m_owner->GetTranslation().z + (RandDelay(7.5f, 1000.0f) / negpos));

		

	//	Ray ray;
	//	ray.setOrigin(noVec3(XVal, 5000, ZVal));
	//	ray.setDirection(noVec3::NEGATIVE_UNIT_Y);

	//	float tt = 0;
	//	float rays[3];
	//	float raye[3];
	//	float pos[3];
	//	memset(pos,  0, sizeof(pos));
	//	memset(rays, 0, sizeof(rays));
	//	memset(raye, 0, sizeof(raye));
	//	rays[0] = (float)ray.getOrigin().x; rays[1] = (float)ray.getOrigin().y; rays[2] = (float)ray.getOrigin().z;
	//	raye[0] = (float)ray.getPoint(5000.0).x; raye[1] = (float)ray.getPoint(5000.0).y; raye[2] = (float)ray.getPoint(5000.0).z;

	//	if (m_sample->getInputGeom()->raycastMesh(rays, raye, tt))
	//	{
	//		pos[0] = rays[0] + (raye[0] - rays[0])*tt;
	//		pos[1] = rays[1] + (raye[1] - rays[1])*tt;
	//		pos[2] = rays[2] + (raye[2] - rays[2])*tt;
	//	}

		//setPathEnd(XVal, 0.f, ZVal);
	//}

	recalc();
	//// if path valid change state STATE_WalkPath and handle walking the path

	if(GetState() == STATE_FindPath)
	{
		mHasPath = true;
		mFindingPath = false;
		Steering()->ArriveOn();
		ChangeState( STATE_Move );
	}
	else
	{
		mHasPath = false;
		mFindingPath = true;
	}
	
	// if path invalid wait for update and try again till we get a valid one
}

void ActorController::SetStateFindPath()
{		
	mHasPath = true;
	mFindingPath = false;
	Steering()->ArriveOn();
	ChangeState( STATE_Move );	
}

//------------------------------------------------------------------------------------
noVec3 ActorController::findValidSpawnPosition(float _rayHeight)
{

	//int negpos = RandDelay(0.0f, 10.0f);
	//if(negpos > 5)
	//	negpos = 1;
	//else
	//	negpos = -1;

	//float borderZone = 10.0f;
	//float XVal = 0.0f;
	//float ZVal = 0.0f;
	//float YVal = 0.0f;
	//noVec3 checkPos = noVec3::ZERO;

	///*if(SharedData::getSingleton().m_AppMode == APPMODE_TERRAINSCENE)
	//{
	//	if(negpos == -1)
	//	{
	//		XVal = (RandDelay(m_tool->cxClientMin() + borderZone, m_tool->cxClient() - borderZone));
	//		ZVal = (RandDelay(m_tool->cyClientMin() + borderZone, m_tool->cyClient() - borderZone));
	//	}
	//	else
	//	{
	//		XVal = (RandDelay(m_tool->cxClientMin() + borderZone, m_tool->cxClient() - borderZone));
	//		ZVal = (RandDelay(m_tool->cyClientMin() + borderZone, m_tool->cyClient() - borderZone));
	//	}

	//	YVal = m_sample->getInputGeom()->getMesh()->mTerrainGroup->getHeightAtWorldPosition(noVec3(XVal, _rayHeight, ZVal));
	//	checkPos = noVec3(XVal, YVal, ZVal);
	//}
	//else if( (SharedData::getSingleton().m_AppMode != APPMODE_TERRAINSCENE) )*/
	//{
	//	if(negpos == -1)
	//	{
	//		XVal = (RandDelay(m_tool->cxClientMin() + borderZone, m_tool->cxClient() - borderZone));
	//		ZVal = (RandDelay(m_tool->cyClientMin() + borderZone, m_tool->cyClient() - borderZone));
	//	}
	//	else
	//	{
	//		XVal = (RandDelay(m_tool->cxClientMin() + borderZone, m_tool->cxClient() - borderZone));
	//		ZVal = (RandDelay(m_tool->cyClientMin() + borderZone, m_tool->cyClient() - borderZone));
	//	}

	//	Ray ray;
	//	ray.setOrigin(noVec3(XVal, _rayHeight, ZVal));
	//	ray.setDirection(noVec3::NEGATIVE_UNIT_Y);

	//	float tt = 0;
	//	float rays[3];
	//	float raye[3];
	//	float pos[3];
	//	memset(pos,  0, sizeof(pos));
	//	memset(rays, 0, sizeof(rays));
	//	memset(raye, 0, sizeof(raye));
	//	rays[0] = (float)ray.getOrigin().x; rays[1] = (float)ray.getOrigin().y; rays[2] = (float)ray.getOrigin().z;
	//	raye[0] = (float)ray.getPoint((_rayHeight * 2)).x; raye[1] = (float)ray.getPoint((_rayHeight * 2)).y; raye[2] = (float)ray.getPoint((_rayHeight * 2)).z;

	//	if (m_sample->getInputGeom()->raycastMesh(rays, raye, tt))
	//	{
	//		pos[0] = rays[0] + (raye[0] - rays[0])*tt;
	//		pos[1] = rays[1] + (raye[1] - rays[1])*tt;
	//		pos[2] = rays[2] + (raye[2] - rays[2])*tt;
	//	}
	//	YVal = pos[1];
	//	checkPos = noVec3(XVal, YVal, ZVal);
	//}

	//float spos[3];
	//float vpos[3];
	//float polyPickExtent[3];
	//dtPolyRef startRef = 0;
	//memset(spos,  0, sizeof(spos));
	//memset(vpos,  0, sizeof(vpos));
	//memset(polyPickExtent, 0, sizeof(polyPickExtent));
	//spos[0] = checkPos.x;
	//spos[1] = checkPos.y;
	//spos[2] = checkPos.z;
	//// increase the size of the bounding box for our spawn point search to maximize the chances
	//// of actually finding a valid place.
	//polyPickExtent[0] = 75;
	//polyPickExtent[1] = 25; // shorter Y height value for extents box as we want to be able to pick different levels of 3d geom
	//polyPickExtent[2] = 75;
	//startRef = m_sample->getNavMesh()->findNearestPoly(spos, polyPickExtent, &m_filter, vpos);

	//if(startRef == 0)
	//{
	//	// didn't find a valid position
	//	return noVec3::ZERO;
	//}
	//else if(startRef != 0 && vpos)
	//{
	//	// found a valid position on the navmesh
	//	return noVec3(vpos[0], vpos[1], vpos[2]);
	//}

	//// if we get here return a zero value, shouldnt get here, but to be safe, if we do get here
	//// we return a zero value as for whatever reason we didnt find a valid spawn point
	return vec3_zero;
}

//------------------------------------------------------------------------------------
void ActorController::recalc(void)
{
	if (!GetApp()->getNavMesh())
		return;

	mWalkList.resize(0);
	if (m_sposSet)
	{
		m_spos[0] = mPathStart.x;
		m_spos[1] = mPathStart.y;
		m_spos[2] = mPathStart.z;
		m_startRef = GetApp()->getNavMesh()->findNearestPoly(m_spos, m_polyPickExt, &m_filter, 0);
	}
	else
		m_startRef = 0;

	if (m_eposSet)
	{
		m_epos[0] = mPathEnd.x;
		m_epos[1] = mPathEnd.y;
		m_epos[2] = mPathEnd.z;
		m_endRef = GetApp()->getNavMesh()->findNearestPoly(m_epos, m_polyPickExt, &m_filter, 0);
	}
	else
		m_endRef = 0;

	if (m_sposSet && m_eposSet && m_startRef && m_endRef)
	{
		if(m_pPath)
			delete m_pPath;
		m_pPath = new AIPath();
		m_npolys = GetApp()->getNavMesh()->findPath(m_startRef, m_endRef, m_spos, m_epos, &m_filter, m_polys, MAX_POLYS);
		m_nstraightPath = 0;
		if (m_npolys)
		{
			m_nstraightPath = GetApp()->getNavMesh()->findStraightPath(m_spos, m_epos, m_polys, m_npolys,
				m_straightPath, m_straightPathFlags,
				m_straightPathPolys, MAX_POLYS);
			if(m_nstraightPath)
			{
				for(unsigned int i = 0; i < m_nstraightPath; ++i)
				{
					
					mWalkList.push_back(noVec3(m_straightPath[i * 3], m_straightPath[i * 3 + 1], m_straightPath[i * 3 + 2]));
				}
				m_pSteering->SetPath(m_pPath->GetPath());
				m_pSteering->SetPathLoopOff();
				m_pSteering->SetPathDone(false);
				m_pSteering->FollowPathOn();
			}	
		}
		m_sposSet = false;
		m_eposSet = false;
	}
	else
	{
		m_npolys = 0;
		m_nstraightPath = 0;
		SetVelocity(vec3_zero);
	}
}

bool ActorController::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
	BeginStateMachine

		OnMsg( MSG_ModeChange )
			SetVelocity(vec3_zero);
			ChangeState( STATE_ModeChange );

		OnMsg( MSG_FindPath )
				SetVelocity(vec3_zero);
			ChangeState( STATE_FindPath );
		OnMsg( MSG_Move)
			ChangeState( STATE_Move );
							
		OnMsg( MSG_Idle)
			if (GetState() != STATE_Jump)
				ChangeState( STATE_Idle );

		OnMsg( MSG_Think )
				// CURRENTLY UNUSED - very low call cycle - for low level state changes, sensory memory updating etc		
	DeclareState( STATE_Initialize )
		OnEnter
			if(m_owner->GetHealth() <= 99) m_owner->SetHealth(100);
		OnMsg( MSG_Wander)
			ChangeState( STATE_Wander );
		OnMsg( MSG_Idle)	
			ChangeStateDelayed( 2.0f, STATE_Idle );
		OnUpdate
	DeclareState( STATE_Move )
		OnEnter
			MoveState* move = (MoveState*)m_StateMgr_->GetState(STATE_Move);
			move->Enter(m_owner);
			SendMsgDelayedToMe( RandDelay( 2.0f, 5.0f ), MSG_Attack );
			OnMsg( MSG_Damaged )
				ChangeStateDelayed(0.5f, STATE_Wound);

		OnUpdate

			if ((mPathEnd - m_owner->GetTranslation()).Length() <= 0.1f)
				ChangeState(STATE_Idle);

			MoveState* move = (MoveState*)m_StateMgr_->GetState(STATE_Move);
			move->Execute(m_owner);			
			OnMsg( MSG_Attack )
				GameObject* target = NULL;
			float dist = FLT_MAX;
			GetClosestPlayer(target, dist);
			if( target && target->IsAlive() )
			{
				m_curTarget = target->GetID();

				IdleState* idle = (IdleState*)m_StateMgr_->GetState(STATE_Idle);				
				idle->Enter(m_owner);
				Steering()->WanderOff();
				Steering()->FollowPathOff();
				SetVelocity(vec3_zero);
				ChangeState( STATE_Attack );
			}
			else
			{
				SendMsgDelayedToMe( RandDelay( 2.0f, 5.0f ), MSG_Attack );
			}

		OnExit			
			SetVelocity(vec3_zero);

	DeclareState( STATE_Jump )		
		OnEnter
			JumpState* jump = (JumpState*)m_StateMgr_->GetState(STATE_Jump);			
			jump->Enter(m_owner);

		OnUpdate
			JumpState* jump = (JumpState*)m_StateMgr_->GetState(STATE_Jump);
			
			jump->Execute(m_owner);

		OnExit
			JumpState* jump = (JumpState*)m_StateMgr_->GetState(STATE_Jump);			
			jump->Exit(m_owner);

	DeclareState( STATE_Wander ) /* we should never get here yet */

		OnEnter
			//Steering()->ObstacleAvoidanceOn();
			Steering()->WanderOn();
			SendMsgDelayedToMe( RandDelay( 2.0f, 5.0f ), MSG_Attack );
			//SendMsgToMe( MSG_Move,  SCOPE_TO_STATE );
			MoveState* move = (MoveState*)m_StateMgr_->GetState(STATE_Move);
			move->Enter(m_owner);
		OnUpdate
			//Wander around
			
			
				

			OnMsg( MSG_Attack )
				GameObject* target = NULL;
				float dist = FLT_MAX;
				GetClosestPlayer(target, dist);
				if( target && target->IsAlive() )
				{
					m_curTarget = target->GetID();

					IdleState* idle = (IdleState*)m_StateMgr_->GetState(STATE_Idle);				
					idle->Enter(m_owner);
					Steering()->WanderOff();
					Steering()->FollowPathOff();
					SetVelocity(vec3_zero);
					ChangeState( STATE_Attack );
				}
				else
				{
					SendMsgDelayedToMe( RandDelay( 2.0f, 5.0f ), MSG_Attack );
				}
	DeclareState( STATE_Attack ) /* we should never get here yet */

		OnEnter
			//SendMsgToMe( MSG_Attack );

			AttackState* attack = (AttackState*)m_StateMgr_->GetState(STATE_Attack);			
			attack->Enter(m_owner);			
		OnUpdate
			AttackState* attack = (AttackState*)m_StateMgr_->GetState(STATE_Attack);			
			attack->Execute(m_owner);			
		OnExit
			AttackState* attack = (AttackState*)m_StateMgr_->GetState(STATE_Attack);			
			attack->Exit(m_owner);			
		/*OnMsg( MSG_Attack )
			GameObject* target = g_database.Find(m_curTarget);
		if( target && target->IsAlive() )
		{				
			AttackState* attack = (AttackState*)m_StateMgr_->GetState(STATE_Attack);			
			attack->Enter(m_owner);			

			SendMsg( MSG_Damaged, m_curTarget, (rand()%20) ); 
			SendMsgDelayedToMe( RandDelay( 2.0f, 4.0f ), MSG_Attack, SCOPE_TO_STATE );
		}
		else
		{
			if (!(m_owner->GetType() && OBJECT_Player))
				ChangeState( STATE_Wander );
		}*/
	DeclareState( STATE_Die ) /* we shoud NEVER EVER get here*/ 
		
		OnEnter
			//Play die animation
			DeathState* death = (DeathState*)m_StateMgr_->GetState(STATE_Die);
			death->Enter(m_owner);
			
		OnUpdate
			//ChangeStateDelayed( 10.0f, STATE_Initialize );	//Respawn after 10 seconds
			//DeathState* death = (DeathState*)m_StateMgr_->GetState(STATE_Die);
			//death->Execute(m_owner);

			//Do nothing
		OnExit
			DeathState* death = (DeathState*)m_StateMgr_->GetState(STATE_Die);
			death->Exit(m_owner);

	DeclareState( STATE_ModeChange )
			
		OnEnter
			//if(m_EntityAIMode == ENTITY_MODE_IDLE)
			{
				//ChangeState(STATE_Idle);
			}
			//else if(m_EntityAIMode == ENTITY_MODE_FINDPATH)
			{
				//ChangeState(STATE_FindPath);
			}
			//else if(m_EntityAIMode == ENTITY_MODE_AUTOMATED)
			{

			}
			//else if(m_EntityAIMode == ENTITY_MODE_NONE)
			{
				//ChangeState(STATE_Idle);
			}

			///////////////////////////////////////////////////////////////
	DeclareState( STATE_FindPath )

		OnEnter
			//setIdlingAnim();
			//mFindingPath = true;
			m_pSteering->FollowPathOff();
			SetVelocity(vec3_zero);
			//findStartEndPositions();	
		OnUpdate
			//mFindingPath = true;
			SetVelocity(vec3_zero);
			//findStartEndPositions();			

		OnExit
			SetVelocity(vec3_zero);

			///////////////////////////////////////////////////////////////
	DeclareState( STATE_WalkPath )

		OnEnter
			//BasicCharacterAnimStateAttribute    *pkAnimAttrib = GetGameObjectAttribute(m_owner,BasicCharacterAnimStateAttribute);
			//if( pkAnimAttrib )
			//	pkAnimAttrib->Walk();
			SendMsgDelayedToMe( RandDelay( 2.0f, 5.0f ), MSG_Attack );			
			
			

		OnUpdate
			

		OnMsg( MSG_FindPath )
			SetVelocity(vec3_zero);
			ChangeState( STATE_FindPath );
		OnMsg( MSG_Damaged )
			ChangeStateDelayed(1.0f, STATE_Wound);


		OnExit
			SetVelocity(vec3_zero);

	DeclareState( STATE_Wound )
		OnEnter
			WoundState* wound = (WoundState*)m_StateMgr_->GetState(STATE_Wound);
			wound->Enter(m_owner);
			m_owner->SetHealth(m_owner->GetHealth()-30);
		OnUpdate
			WoundState* wound = (WoundState*)m_StateMgr_->GetState(STATE_Wound);
			wound->Execute(m_owner);			
			if (m_owner->GetHealth() <= 0)
				ChangeStateDelayed(0.1f, STATE_Die);

		OnExit			
			

	DeclareState( STATE_Idle )

		OnEnter

			//setIdlingAnim();
			/*AnimationComponent* pAnimComp = GetGameObjectComponent(m_owner, AnimationComponent);			
			BasicCharacterAnimStateAttribute    *pkAnimAttrib = GetGameObjectAttribute(m_owner,BasicCharacterAnimStateAttribute);
			if( pkAnimAttrib )
				pkAnimAttrib->Idle();*/
			IdleState* idleState = (IdleState*)m_StateMgr_->GetState(STATE_Idle);
			idleState->Enter(m_owner);
		m_pSteering->FollowPathOff();
		SetVelocity(vec3_zero);
		mIdleTimerToChange = RandDelay(5.0f, 20.0f);
		//SendMsgDelayedToMe( mIdleTimerToChange, MSG_IdleChange, SCOPE_TO_STATE);
		OnMsg( MSG_Damaged )
			ChangeStateDelayed(0.5f, STATE_Wound);

		OnMsg( MSG_Jump )
			ChangeState(STATE_Jump);

		OnUpdate
			OnMsg( MSG_Jump )
				ChangeState(STATE_Jump);
			SetVelocity(vec3_zero);
		//if((RandDelay(0.0f, 100.0f) <= 1.0f) && (mBaseAnimID != ANIM_JUMP_LOOP))
		//	setJumpingAnim();

		//OnMsg( MSG_IdleChange )
		//	ChangeStateDelayed(0.25f, selectIdleAnimset());
			OnMsg( MSG_Wander)
				ChangeState( STATE_Wander );

		OnExit
			IdleState* idleState = (IdleState*)m_StateMgr_->GetState(STATE_Idle);
			idleState->Exit(m_owner);
			SetVelocity(vec3_zero);

	DeclareState( STATE_Think )

		OnEnter

			///////////////////////////////////////////////////////////////
	EndStateMachine	
}

void ActorController::sendThinkMessage( void )
{
	SendMsgToMe( MSG_Think );

}

void ActorController::sendModeChangeMessage( void )
{
	SendMsgToMe( MSG_ModeChange );

}

void ActorController::sendFindNewPathMessage( void )
{
	mFindingPath = true;
	SendMsgToMe( MSG_FindPath );
}

void ActorController::GetClosestTarget( const noVec3& pos, GameObject*& output)
{
	dbCompositionList list;
	g_database.ComposeList( list, OBJECT_Enemy | OBJECT_Character );

	if( list.empty() )
	{
		output = NULL;
		return;
	}
	else
	{		
		float minDist = FLT_MAX;
		GameObject* target = NULL;
		for(dbCompositionList::iterator it = list.begin(); it != list.end(); ++it)
		{
			// TODO : add proximity check code for closest check here
			if ((*it)->GetID() == m_owner->GetID())
				continue;
			noVec3 enemyPos = (*it)->GetTranslation();
			float dist = (pos - enemyPos).LengthSqr();
			if ( dist < minDist)
			{
				minDist = dist;
				target = (*it);
			}
		}
		
		output = target;		
	}
}


void ActorController::GetClosestPlayer( GameObject*& gameobj, float& dist )
{
	dbCompositionList list;
	g_database.ComposeList( list, OBJECT_Enemy | OBJECT_Character );

	if( list.empty() )
	{
		gameobj = NULL;
		return;
	}
	else
	{
		noVec3 thisPos = m_owner->GetTranslation();
		float minDist = FLT_MAX;
		GameObject* target;
		for(dbCompositionList::iterator it = list.begin(); it != list.end(); ++it)
		{
			// TODO : add proximity check code for closest check here
			if ((*it)->GetID() == m_owner->GetID())
				continue;
			noVec3 enemyPos = (*it)->GetTranslation();
			float dist = (thisPos - enemyPos).LengthSqr();
			if ( dist < minDist)
			{
				minDist = dist;
				target = (*it);
			}
		}
		if (minDist <= mAttackRange)
		{
			gameobj = target;	
			dist = minDist;
		}
	}
}

NavigationMeshAdapter::NavigationMeshAdapter(GameObject* owner)
	: m_PathActive(false),
	m_CurrentCell(0),
	m_Parent(NULL),
	m_Movement(0.0f, 0.0f, 0.0f),
	m_Position(0.0f, 0.0f, 0.0f),
	m_MaxSpeed(5.0f),
	m_owner(owner)
{

}

NavigationCell* NavigationMeshAdapter::CurrentCell() const
{
	return m_CurrentCell;
}

bool NavigationMeshAdapter::PathIsActive() const
{
	return m_PathActive;
}

void NavigationMeshAdapter::Update( float elapsedTime /*= 1.0f*/ )
{	
	WowActor* pActor = (WowActor*)m_owner;
	ActorController* pActrl = (ActorController*)pActor->GetStateMachine();
		

	float	distance;
	float	max_distance = m_MaxSpeed * elapsedTime;

	

	/* if we have no parent mesh, return. We are simply a static object. */
	if(!m_Parent)
	{
		return;
	}

	/*
	 * ;
	 * Figure out where we are going ;
	 */
	if(m_PathActive)
	{

		/*
		 * ;
		 * Move along the waypoint path ;
		 */
		if(m_NextWaypoint != m_Path.WaypointList().end())
		{

			/* determine our desired movement vector */
			m_Movement = (*m_NextWaypoint).Position;
			m_Movement -= m_Position;	
		}
		else
		{

			/* we have reached the end of the path */
			m_PathActive = false;
			//m_Movement.x = m_Movement.y = m_Movement.z = 0.0f;
			pActrl->ChangeState(STATE_Idle);
			pActrl->SetVelocity(vec3_zero);
		}
	}
	else
	{

		/* apply some friction to our current movement (if any) */
		m_Movement.x *= 0.75f;
		m_Movement.y *= 0.75f;
		m_Movement.z *= 0.75f;
	}

	/*
	 * ;
	 * Adjust Movement ;
	 * ;
	 * scale back movement by our max speed if needed
	 */
	distance = m_Movement.Length();
	if(distance > max_distance)
	{
		m_Movement.Normalize();
		m_Movement *= max_distance;
	}

	m_Movement = pActrl->Velocity();

	/* come to a full stop when we go below a certain threshold */
	if(fabs(pActrl->Velocity().x) < 0.005f) m_Movement.x = 0.0f;
	if(fabs(pActrl->Velocity().y) < 0.005f) m_Movement.y = 0.0f;
	if(fabs(pActrl->Velocity().z) < 0.005f) m_Movement.z = 0.0f;

	/*
	 * ;
	 * Constrain any remaining Horizontal movement to the parent navigation rink ;
	 */
	if(m_Movement.x || m_Movement.z)
	{

		/*
		 * ;
		 * Constrain any remaining Horizontal movement to the parent navigation rink ;
		 */
		if(m_Parent)
		{

			/* compute the next desired location */
			noVec3			NextPosition = pActrl->Pos();
			NavigationCell	*NextCell;

			/* test location on the NavigationMesh and resolve collisions */
			m_Parent->ResolveMotionOnMesh(m_Position, m_CurrentCell, NextPosition, &NextCell);

			m_Position = NextPosition;
			pActor->SetTranslation(m_Position);
			m_CurrentCell = NextCell;			
		}
	}
	else if(m_PathActive)
	{

		/*
		 * If we have no remaining movement, but the path is active, ;
		 * we have arrived at our desired waypoint. ;
		 * Snap to it's position and figure out where to go next
		 */
		m_Position = (*m_NextWaypoint).Position;		

		
		if (pActor)
		{
			m_Movement.x = m_Movement.y = m_Movement.z = 0.0f;
			distance = 0.0f;
			m_NextWaypoint = m_Path.GetFurthestVisibleWayPoint(m_NextWaypoint);

			if(m_NextWaypoint == m_Path.WaypointList().end())
			{
				m_PathActive = false;
				m_Movement.x = m_Movement.y = m_Movement.z = 0.0f;

				
				pActrl->ChangeState(STATE_Idle);
				pActrl->SetVelocity(vec3_zero);
				pActrl->ExitFindPath();
			}
			else 
			{			
				if (pActrl)
				{				
					pActrl->setPathStart(m_Position);
					pActrl->setPathEnd(m_NextWaypoint->Position);				
				}
			}
		}
	}
}

void NavigationMeshAdapter::GotoLocation( const noVec3 &Position, NavigationCell *Cell )
{
	if(m_Parent)
	{		
		m_Movement.Zero();

		m_PathActive = m_Parent->BuildNavigationPath(m_Path, m_CurrentCell, m_Position, Cell, Position);

		if(m_PathActive)
		{
			m_NextWaypoint = m_Path.WaypointList().begin();
			WowActor* pActor = (WowActor*)m_owner;
			ActorController* pActrl = (ActorController*)pActor->GetStateMachine();
			if (pActrl)
			{				
				pActrl->SendMsgToMe(MSG_FindPath);
				pActrl->setPathStart(m_owner->GetTranslation());
				pActrl->setPathEnd(m_NextWaypoint->Position);
				pActrl->SetStateFindPath();
			}
		}
	}
}

void NavigationMeshAdapter::GotoRandomLocation()
{
	if(m_Parent)
	{

		/* pick a random cell and go there */
		int				index = rand() % m_Parent->TotalCells();
		NavigationCell	*pCell = m_Parent->Cell(index);

		GotoLocation(pCell->CenterPoint(), pCell);
	}
}

void NavigationMeshAdapter::AddMovement( const noVec3 &Movement )
{

}

void NavigationMeshAdapter::SetMovement( const noVec3 &Movement )
{

}

void NavigationMeshAdapter::SetMovementX( float X )
{

}

void NavigationMeshAdapter::SetMovementY( float Y )
{

}

void NavigationMeshAdapter::SetMovementZ( float Z )
{

}

void NavigationMeshAdapter::SetMaxSpeed( float speed )
{

}

void NavigationMeshAdapter::Create( NavigationMesh *Parent, const noVec3 &Position, NavigationCell *CurrentCell )
{
	m_Parent = Parent;
	m_Position = Position;
	m_CurrentCell = CurrentCell;

	m_Movement.x = m_Movement.y = m_Movement.z = 0.0f;
	

	if(m_Parent)
	{

		/* if no starting cell is provided, find one by searching the mesh */
		if(!m_CurrentCell)
		{
			m_CurrentCell = m_Parent->FindClosestCell(m_Position);
		}

		/* make sure our position is within the current cell */
		m_Position = m_Parent->SnapPointToCell(m_CurrentCell, m_Position);
		m_owner->SetTranslation(m_Position);
	}
}
