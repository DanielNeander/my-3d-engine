#include "stdafx.h"

#include "ColorValue.h"
#include "Common.h"

//
// core includes
//
#include ".\BaseTypes\BaseTypes.h"
#include ".\Interfaces\Interface.h"

#include "GraphicsScene.h"
#include "GraphicsObject.h"

#define PROOTNODE (reinterpret_cast<GraphicsScene*>(m_pSystemScene)->GetRootSceneNode())


pcstr GraphicsObject::sm_kapszTypeNames[] =
{
	"Light", "LightFire", "Camera", "Mesh", "MeshAnimated", "PointList", "Window", "StatWindow", "Chart", "CPUChart", "WorkloadWindow", 
	"ParticleSystem", "PagedGeometryLayer",
	NULL
};


void GraphicsObject::Update( f32 DeltaTime )
{
	UNREFERENCED_PARAM( DeltaTime );
	return;
}

GraphicsObject::GraphicsObject( ISystemScene* pSystemScene, pcstr pszName )
	: ISystemObject( pSystemScene, pszName )
	, m_pNode(NULL)
{
	m_pszName = pszName;
}

GraphicsObject::~GraphicsObject( void )
{

}

System::Type GraphicsObject::GetSystemType( void )
{
	return System::Types::Graphics;
}

Error GraphicsObject::Initialize( std::vector<Properties::Property> Properties )
{
	ASSERT( !m_bInitialized );

	//
	// Create the scene node for this object.
	//
	//m_pNode = POGREROOTNODE->createChildSceneNode( std::string( m_pszName ) + "_SceneNode" );
	//ASSERT( m_pNode != NULL );

	return Errors::Success;
}

System::Types::BitMask GraphicsObject::GetDesiredSystemChanges( void )
{
	return System::Changes::Geometry::Position |
		System::Changes::Geometry::Orientation |
		System::Changes::Geometry::Scale |
		System::Changes::Graphics::GUI;
}

namespace {

	// Provides access to protected members of the Ogre::SceneNode class
	// in order to check if the locking is necessary
	class AccessOgreSceneNode : public SceneNode
	{
		// Never use this class
		AccessOgreSceneNode ( const AccessOgreSceneNode& );
		AccessOgreSceneNode ();
		const AccessOgreSceneNode& operator= ( const AccessOgreSceneNode& );
	public:

		bool NeedsLocking ()
		{
			return parent_;//&& !mParentNotified;
		}
	};

	inline 
		bool NeedsLocking ( SceneNode* pNode )
	{
		return reinterpret_cast<AccessOgreSceneNode*>(pNode)->NeedsLocking();
	}

} // anonymous namespace

inline
	void UpdateGeometry( SceneNode* pNode, System::Changes::BitMask ChangeType, IGeometryObject* pGeometryObject )
{
	if ( ChangeType & System::Changes::Geometry::Position )
	{
		const Math::Vector3& Position = *pGeometryObject->GetPosition();
		pNode->SetTrans( noVec3(Position.x, Position.y, Position.z) );
	}

	if ( ChangeType & System::Changes::Geometry::Orientation )
	{
		const Math::Quaternion& Orientation = *pGeometryObject->GetOrientation();
		//pNode->setOrientation( Orientation.w, Orientation.x, Orientation.y, Orientation.z );
	}

	if ( ChangeType & System::Changes::Geometry::Scale )
	{
		const Math::Vector3& Scale = *pGeometryObject->GetScale();
		//pNode->SetScale( Scale.x, Scale.y, Scale.z );
		pNode->SetScale( Scale.x );
	}
}


Error GraphicsObject::ChangeOccurred( ISubject* pSubject, System::Changes::BitMask ChangeType )
{
	ASSERT( m_bInitialized );
#if 0
	if ( !m_pNode )
	{
		// StaticGeom and InstancedGeom objects are abstract groupings and 
		// are not globally attached to any scene node
		return Errors::Success;
	}

	if ( ChangeType & (System::Changes::Geometry::Position |
		System::Changes::Geometry::Orientation |
		System::Changes::Geometry::Scale)
		)
	{
		IGeometryObject* pGeometryObject = dynamic_cast<IGeometryObject*>(pSubject);

		if ( NeedsLocking(m_pNode) )
		{
			SCOPED_SPIN_LOCK(OGREGraphicsScene::m_mutex);
			UpdateGeometry( m_pNode, ChangeType, pGeometryObject );
		}
		else
		{
			UpdateGeometry( m_pNode, ChangeType, pGeometryObject );
		}
	}
#endif
	return Errors::Success;
}

System::Changes::BitMask GraphicsObject::GetPotentialSystemChanges( void )
{
	return System::Changes::None;

}


