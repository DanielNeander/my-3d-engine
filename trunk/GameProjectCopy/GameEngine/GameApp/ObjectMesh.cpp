#include "stdafx.h"

#pragma warning( push, 0 )
#pragma warning( disable : 4244 6211 6386 )

#include "Common.h"
#include "ColorValue.h"
//
// core includes
//
#include ".\BaseTypes\BaseTypes.h"
#include ".\Interfaces\Interface.h"

#include "./ProceduralTrees/TreeObject.h"

#include "GraphicsScene.h"
#include "GraphicSystem.h"
#include "GraphicsObject.h"
#include "ObjectMesh.h"

#include "GameApp/GameApp.h"

#include "./Framework/IttNotify.h"

__ITT_DEFINE_STATIC_EVENT(g_tpeChangeOccurred, "Graphics Mesh: ChangeOccurred", 29 );

pcstr GraphicsObjectMesh::sm_kapszPropertyNames[] =
{
	"Mesh", "ProceduralMesh", "Material", "CastShadows", 
	"DrawBoundingBox", "ShowNormals", "ShowTangents",
	"StaticGeom", "Instance",
	"Position", "Orientation", "Scale"
};

const Properties::Property GraphicsObjectMesh::sm_kaDefaultProperties[] =
{
	Properties::Property( sm_kapszPropertyNames[ Property_Mesh ],
	VALUE1( Properties::Values::String ),
	Properties::Flags::Valid | Properties::Flags::InitOnly,
	NULL, NULL, NULL, NULL,
	"" ),
	Properties::Property( sm_kapszPropertyNames[ Property_ProceduralMesh ],
	VALUE1( Properties::Values::Boolean ),
	Properties::Flags::Valid | Properties::Flags::InitOnly,
	NULL, NULL, NULL, NULL,
	False ),
	Properties::Property( sm_kapszPropertyNames[ Property_Material ],
	VALUE1( Properties::Values::String ),
	Properties::Flags::Valid | Properties::Flags::WriteOnly,
	NULL, NULL, NULL, NULL,
	"" ),
	Properties::Property( sm_kapszPropertyNames[ Property_CastShadows ],
	VALUE1( Properties::Values::Int32 ),
	Properties::Flags::Valid,
	NULL, NULL, NULL, NULL,
	0 ),
	Properties::Property( sm_kapszPropertyNames[ Property_DrawBoundingBox ],
	Properties::Values::Boolean,
	Properties::Flags::Valid,
	NULL, NULL, NULL, NULL,
	0 ),
	Properties::Property( sm_kapszPropertyNames[ Property_ShowNormals ],
	Properties::Values::Boolean,
	Properties::Flags::Valid,
	NULL, NULL, NULL, NULL,
	0 ),
	Properties::Property( sm_kapszPropertyNames[ Property_ShowTangents ],
	Properties::Values::Boolean,
	Properties::Flags::Valid,
	NULL, NULL, NULL, NULL,
	0 ),
	Properties::Property( sm_kapszPropertyNames[ Property_StaticGeom ],
	Properties::Values::String,
	Properties::Flags::Valid  | Properties::Flags::InitOnly,
	"Group", NULL, NULL, NULL,
	"" ),
	Properties::Property( sm_kapszPropertyNames[ Property_Instance ],
	Properties::Values::String,
	Properties::Flags::Valid,
	"Group", NULL, NULL, NULL,
	"" ),
	Properties::Property( sm_kapszPropertyNames[ Property_Position ],
	Properties::Values::Vector3,
	Properties::Flags::Valid,
	NULL, NULL, NULL, NULL,
	Math::Vector3::Zero ),
	Properties::Property( sm_kapszPropertyNames[ Property_Orientation ],
	Properties::Values::Quaternion,
	Properties::Flags::Valid,
	NULL, NULL, NULL, NULL,
	Math::Quaternion::Zero ),
	Properties::Property( sm_kapszPropertyNames[ Property_Scale ],
	Properties::Values::Vector3,
	Properties::Flags::Valid,
	NULL, NULL, NULL, NULL,
	Math::Vector3::Zero ),
};


GraphicsObjectMesh::GraphicsObjectMesh( ISystemScene* pSystemScene, pcstr pszName ) 
	: GraphicsObject( pSystemScene, pszName )	
	, isProcedural( false )
	, m_strStaticGrpName ( "" )
	, m_Position( Math::Vector3::Zero )
	, m_Orientation( Math::Quaternion::Zero )
	, m_Scale( Math::Vector3::One )	
	, m_Dirty( True )    // Force Instanced Geom update initially
{
	ASSERT( Property_Count == sizeof sm_kapszPropertyNames / sizeof sm_kapszPropertyNames[ 0 ] );
	ASSERT( Property_Count == sizeof sm_kaDefaultProperties / sizeof sm_kaDefaultProperties[ 0 ] );

	m_Type = GraphicsObject::Type_Mesh;
}

GraphicsObjectMesh::~GraphicsObjectMesh( void )
{
	
}

Error GraphicsObjectMesh::Initialize( std::vector<Properties::Property> Properties )
{
	Error Err = Errors::Failure;

	ASSERT( !m_bInitialized );

	//
	// Call the base class.
	//
	GraphicsObject::Initialize( Properties );


	//
	// Read in the initialization only properties.
	//
	pcstr pszMeshName = NULL;

	for ( Properties::Iterator it=Properties.begin(); it != Properties.end(); it++ )
	{
		if ( it->GetFlags() & Properties::Flags::Valid &&
			it->GetFlags() & Properties::Flags::InitOnly )
		{
			std::string sName = it->GetName();

			if ( sName == sm_kapszPropertyNames[ Property_Mesh ] )
			{
				pszMeshName = it->GetStringPtr( 0 );

				PostChanges( System::Changes::Graphics::AllMesh |
					System::Changes::Custom);
			}
			else if ( sName == sm_kapszPropertyNames[ Property_ProceduralMesh ] )
			{
				isProcedural = true;
			}
			else if ( sName == sm_kapszPropertyNames[ Property_StaticGeom ] )
			{
				m_strStaticGrpName = it->GetString( 0 );
			}
			else
			{
				ASSERT( False );
			}
			//
			// Set this property to invalid since it's already been read.
			//
			it->ClearFlag( Properties::Flags::Valid );
		}
	}

	ASSERT( pszMeshName != NULL  ||  isProcedural );

	if ( pszMeshName != NULL )
	{
		//
		// Use the UObject's name for this entity.
		//
		/*m_pEntity = PSCENEMGR->createEntity( m_pszName, pszMeshName );
		ASSERT( m_pEntity != NULL );

		m_pNode->attachObject( m_pEntity );*/
	}

	if ( /*m_pEntity != NULL  ||*/  isProcedural )
	{
		//
		// Set this set as initialized.
		//
		m_bInitialized = True;

		//
		// Set the remaining properties for this object.
		//
		SetProperties( Properties );

		Err = Errors::Success;
	}

	return Err;
}

void GraphicsObjectMesh::GetProperties( Properties::Array& Properties )
{
	//
	// Get the index of our first item.
	//
	i32 iProperty = static_cast<i32>(Properties.size());

	//
	// Add all the properties.
	//
	Properties.reserve( Properties.size() + Property_Count );

	for ( i32 i=0; i < Property_Count; i++ )
	{
		Properties.push_back( sm_kaDefaultProperties[ i ] );
	}


	//
	// Modify the default values.
	//
	Properties[ iProperty+Property_Position ].SetValue( m_Position );
	Properties[ iProperty+Property_Orientation ].SetValue( m_Orientation );
	Properties[ iProperty+Property_Scale ].SetValue( m_Scale );

	/*if ( m_pEntity != NULL )
	{
	Properties[ iProperty+Property_Mesh ].SetValue( 0, m_pEntity->getMesh()->getName() );

	Properties[ iProperty+Property_CastShadows ].SetValue( 0, m_pEntity->getCastShadows() );
	}
	else*/ if ( isProcedural )
	{
		Properties[ iProperty+Property_Mesh ].SetValue(
			0, sMaterialName.c_str()
			);

		Properties[ iProperty+Property_CastShadows ].SetValue(
			0, bCastShadows
			);
	}
}

void GraphicsObjectMesh::SetProperties( Properties::Array Properties )
{
	ASSERT( m_bInitialized );

	//
	// Read in the properties.
	//
	for ( Properties::Iterator it=Properties.begin(); it != Properties.end(); it++ )
	{
		if ( it->GetFlags() & Properties::Flags::Valid )
		{
			std::string sName = it->GetName();

			if ( sName == sm_kapszPropertyNames[ Property_Material ] )
			{
				/*if ( m_pEntity != NULL )
				{
				m_pEntity->setMaterialName( it->GetStringPtr( 0 ) );
				}
				else*/ if ( isProcedural )
				{
					sMaterialName = it->GetStringPtr( 0 );
				}
				else
				{
					ASSERT( False );
				}
			}
			else if ( sName == sm_kapszPropertyNames[ Property_CastShadows ] )
			{
				/*if ( m_pEntity != NULL )
				{
				m_pEntity->setCastShadows( it->GetBool( 0 ) != False );
				}
				else*/ if ( isProcedural )
				{
					bCastShadows = it->GetBool( 0 ) != False;
				}
				else
				{
					ASSERT( False );
				}
			}
			else if ( sName == sm_kapszPropertyNames[ Property_Position ] )
			{
				m_Position = it->GetVector3();
			}
			else if ( sName == sm_kapszPropertyNames[ Property_Orientation ] )
			{
				m_Orientation = it->GetQuaternion();
			}
			else if ( sName == sm_kapszPropertyNames[ Property_Scale ] )
			{
				m_Scale = it->GetVector3();
			}
			else
			{
				ASSERT( False );
			}

			//
			// Set this property to invalid since it's already been read.
			//
			it->ClearFlag( Properties::Flags::Valid );
		}
	}
}

System::Types::BitMask GraphicsObjectMesh::GetDesiredSystemChanges( void )
{
	return System::Changes::Custom | System::Changes::Graphics::All | GraphicsObject::GetDesiredSystemChanges();
}

namespace {

	// Provides access to protected members of the ::SceneNode class
	// in order to check if the locking is necessary
	class AccessSceneNode : public SceneNode
	{
		// Never use this class
		AccessSceneNode ( const AccessSceneNode& );
		AccessSceneNode ();
		const AccessSceneNode& operator= ( const AccessSceneNode& );
	public:

		bool NeedsLocking ()
		{
			return parent_ /*&& !mParentNotified*/;
		}
	};

	inline 
		bool NeedsLocking ( SceneNode* pNode )
	{
		return reinterpret_cast<AccessSceneNode*>(pNode)->NeedsLocking();
	}

} // anonymous namespace

Error GraphicsObjectMesh::ChangeOccurred( ISubject* pSubject, System::Changes::BitMask ChangeType )
{
	// THREAD SAFETY NOTE
	// Currently some of the notifications do not arrive concurrently.
	// If they ever do, calls marked as "Lock it" must be protected by locks.

	__ITT_EVENT_START(g_tpeChangeOccurred, PROFILE_MESH);

	if ( ChangeType & (System::Changes::Graphics::IndexDecl |
		System::Changes::Graphics::VertexDecl) )
	{
		//
		// Get the vertex information from the graphics object.
		//
		IGraphicsObject* pGfxObj = dynamic_cast<IGraphicsObject*>(pSubject);
		ASSERT( pGfxObj != NULL );

		if( pGfxObj ) {
			u32 IndexDecl = pGfxObj->GetIndexDeclaration();
			u32 VertexDeclCount = pGfxObj->GetVertexDeclarationCount();
			//VertexDecl::Element* pVertexDecl = new VertexDecl::Element[ VertexDeclCount ];
			//ASSERT( pVertexDecl != NULL );
			//pGfxObj->GetVertexDeclaration( pVertexDecl );						
			u32 IndexCount = pGfxObj->GetIndexCount();
			u32 VertexCount = pGfxObj->GetVertexCount();
			
			//
			// Create a unique mesh name.
			//
			char szMeshName[ 256 ];
			sprintf_s( szMeshName, sizeof szMeshName, "%s_ProceduralMesh", m_pszName );
			

			FormatDesc vbFmt[] = {
				0, TYPE_VERTEX, FORMAT_FLOAT,  3,		
				0, TYPE_NORMAL, FORMAT_FLOAT,  3,		
				0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,				
			};

			ShaderID shader = -1;
			if (dynamic_cast<TreeObject*>(pGfxObj))  {
				shader = gTreeShader;
			}
			else {
				shader = gFireShader;
			}

			struct ProcVertex {
				noVec3 Pos;
				noVec3 Normal;
				noVec2 Texcoord;
			};

			if (shader != -1) {
				m_Mesh = new Mesh_D3D11;
				m_Mesh->SetName(szMeshName);

				int vertexStride = 0;
				for (int i = 0; i < elementsOf(vbFmt); ++i)
				{
					vertexStride +=vbFmt[i].size * GetRenderer()->getFormatSize(vbFmt[i].format);
				}
				m_Mesh->m_rd.shader = shader;
				m_Mesh->m_iVertexSize = vertexStride;			
				m_Mesh->m_iNumTris = IndexCount / 3;
				m_Mesh->m_iNumVertices = VertexCount;
				m_Mesh->m_rd.vf = GetRenderer()->addVertexFormat(vbFmt, elementsOf(vbFmt), shader);
				ProcVertex* pVB = new ProcVertex[VertexCount];
				pGfxObj->GetVertices(pVB, 0, 0);
				m_Mesh->m_rd.vb = GetRenderer()->addVertexBuffer(sizeof(ProcVertex) * VertexCount, STATIC, pVB);
				m_Mesh->m_pVertices = (float*)pVB;

				idStr name = m_pszName;
				if (name.Find("Tree"))
					m_Mesh->m_rd.baseTex = GetRenderer()->addTexture("Textures/smoke_maple_yellow.tga", true);
				else if (name.Find("Canopy"))
					m_Mesh->m_rd.baseTex = GetRenderer()->addTexture("Textures/sd_bark.jpg", true);
				m_Mesh->m_rd.vertexEnd = VertexCount;
				m_Mesh->m_rd.endIndex = IndexCount;			

				if (IndexCount > 0 ) {
					
					u16* indices = new u16[IndexCount];
					pGfxObj->GetIndices(indices, 0);
					m_Mesh->m_rd.ib = GetRenderer()->addIndexBuffer(IndexCount, sizeof(WORD), STATIC,  indices);
					m_Mesh->m_pIndices = indices;										
				}			
								
				GetApp()->m_ActorRoot->AddChild(m_Mesh);
			}
		}

		if ( ChangeType & System::Changes::Graphics::AABB ) // Lock it, see THREAD SAFETY NOTE above
		{
			SCOPED_SPIN_LOCK(GraphicsScene::m_mutex);

			/*ASSERTMSG( !pMesh.isNull(),
				"The index/vertex decl needs to happen before or at the same time as this." );*/

			//
			// Get the AABB information from the graphics object.
			//
			IGraphicsObject* pGfxObj = dynamic_cast<IGraphicsObject*>(pSubject);
			ASSERT( pGfxObj != NULL );

			Math::Vector3 AABBMin;
			Math::Vector3 AABBMax;
			pGfxObj->GetAABB( AABBMin, AABBMax );

			// Set the mesh's bounding box.
			//
			/*::AxisAlignedBox AABox;
			AABox.setMaximum( ::Vector3( AABBMax.x, AABBMax.y, AABBMax.z ) );
			AABox.setMinimum( ::Vector3( AABBMin.x, AABBMin.y, AABBMin.z ) );
			pMesh->_setBounds( AABox );

			m_pNode->_updateBounds();*/
		}

		if ( ChangeType & System::Changes::Graphics::IndexBuffer )
		{
			//ASSERTMSG( !pMesh.isNull(),
			//	"The index/vertex decl needs to happen before or at the same time as this." );

			//
			// Get the index buffer from the graphics object.
			//
			IGraphicsObject* pGfxObj = dynamic_cast<IGraphicsObject*>(pSubject);
			ASSERT (pGfxObj != NULL );

			u32 nSubMeshCount = pGfxObj->GetSubMeshCount();

			for( u32 i = 0; i < nSubMeshCount; i++ ) // Lock it, see THREAD SAFETY NOTE above?
			{

			}
		}

		if ( ChangeType & System::Changes::Graphics::VertexBuffer )
		{
			//ASSERTMSG( !pMesh.isNull(),
			//	"The index/vertex decl needs to happen before or at the same time as this." );

			//
			// Get the vertex buffer from the graphics object.
			//
			IGraphicsObject* pGfxObj = dynamic_cast<IGraphicsObject*>(pSubject);
			ASSERT( pGfxObj != NULL );

			u32 nSubMeshCount = pGfxObj->GetSubMeshCount();

			// Figure out which vertex streams changed.
			u32 nStreamMask = pGfxObj->GetStreamsChanged();

			for( u32 i = 0; i < nSubMeshCount; i++ )
			{


			}
		}
	}

	if ( ChangeType & (System::Changes::Geometry::Position |
		System::Changes::Geometry::Orientation |
		System::Changes::Geometry::Scale)
		)
	{
		IGeometryObject* pGeometryObject = dynamic_cast<IGeometryObject*>(pSubject);
		ASSERT( pGeometryObject != NULL );

		if(m_pNode)
		{
			if ( NeedsLocking(m_pNode) )
			{
				SCOPED_SPIN_LOCK(GraphicsScene::m_mutex);
				GeometryChanged( ChangeType, pGeometryObject );
			}
			else
			{
				GeometryChanged( ChangeType, pGeometryObject );
			}
		}
		else
		{
			if ( ChangeType & System::Changes::Geometry::Position )
			{
				m_Position = *pGeometryObject->GetPosition();
				m_Dirty = True;
			}
			if ( ChangeType & System::Changes::Geometry::Orientation )
			{
				m_Orientation = *pGeometryObject->GetOrientation();
				m_Dirty = True;
			}
			if ( ChangeType & System::Changes::Geometry::Scale )
			{
				m_Scale = *pGeometryObject->GetScale();
				m_Dirty = True;
			}
		}
	}
	__ITT_EVENT_END(g_tpeChangeOccurred, PROFILE_MESH);

	return Errors::Success;
}

System::Changes::BitMask GraphicsObjectMesh::GetPotentialSystemChanges( void )
{
	return System::Changes::Graphics::All | GraphicsObject::GetPotentialSystemChanges();
}

u32 GraphicsObjectMesh::GetSubMeshCount( void )
{
	//return m_pEntity->getMesh()->getNumSubMeshes();
	return 0;
}

u32 GraphicsObjectMesh::GetIndexDeclaration( In u16 nSubMeshIndex /*= 0 */ )
{
	return 0;
}

u32 GraphicsObjectMesh::GetVertexDeclarationCount( In u16 nSubMeshIndex /*= 0 */ )
{
	return 0;
}

void GraphicsObjectMesh::GetVertexDeclaration( Out VertexDecl::Element* pVertexDecl, In u16 nSubMeshIndex /*= 0 */ )
{
	
}

u32 GraphicsObjectMesh::GetIndexCount( In u16 nSubMeshIndex /*= 0 */ )
{
	return 0;
}

u32 GraphicsObjectMesh::GetVertexCount( In u16 nSubMeshIndex /*= 0 */ )
{
	return 0;
}

void GraphicsObjectMesh::GetIndices( Out void* pIndices, In u16 nSubMeshIndex /*= 0 */ )
{

}

void GraphicsObjectMesh::GetVertices( Out void* pVertices, In u16 nSubMeshIndex /*= 0*/, In u16 nStreamIndex /*= 0*/, In u32 nVertexDeclCount /*= 0*/, In VertexDecl::Element* pVertexDecl /*= NULL */ )
{

}

u32 GraphicsObjectMesh::GetStreamsChanged( void )
{
	// Always returns 1 to indicate change to stream 0 only.
	// Will have to be changed if and when we expose the Mesh streams
	// to other systems.
	return 1;
}

void GraphicsObjectMesh::GetAABB( Out Math::Vector3& Min, Out Math::Vector3& Max )
{

}

void GraphicsObjectMesh::SetupNormalsAndTangentsDisplay( void )
{

}

void GraphicsObjectMesh::SetupCaptions( void )
{

}

void GraphicsObjectMesh::GeometryChanged( System::Changes::BitMask ChangeType, IGeometryObject* pGeometryObject )
{
	if ( ChangeType & System::Changes::Geometry::Position )
	{
		m_Position = *pGeometryObject->GetPosition();
		//m_pNode->setPosition( m_Position.x, m_Position.y, m_Position.z );
		m_Dirty = True;
	}

	if ( ChangeType & System::Changes::Geometry::Orientation )
	{
		m_Orientation = *pGeometryObject->GetOrientation();
		//m_pNode->setOrientation( m_Orientation.w, m_Orientation.x,m_Orientation.y, m_Orientation.z );
		m_Dirty = True;
	}

	if ( ChangeType & System::Changes::Geometry::Scale )
	{
		m_Scale = *pGeometryObject->GetScale();
		//m_pNode->setScale(m_Scale.x, m_Scale.y, m_Scale.z );
		m_Dirty = True;
	}
}

template<class IdxType>
void GraphicsObjectMesh::BuildNormalsTemplate( u32 nSubMesh )
{

}

#pragma warning( pop )
