SHLightList
#include "stdafx.h"
#include <EngineCore/Util/BSP.h>

#include ".\BaseTypes\BaseTypes.h"
#include ".\Interfaces\Interface.h"

#include "ColorValue.h"
#include "Common.h"

#include "GraphicSystem.h"
#include "GraphicsScene.h"
#include "GraphicsTask.h"
#include "GraphicsObject.h"
#include "ObjectMesh.h"

#include "GameObjectManager.h"
#include "Mesh.h"
#include "M2Mesh.h"
#include "ModelLib/M2Loader.h"



extern ManagerInterfaces       g_GraphicsManagers;

// We use SM2.0 instancing. Since we do normal mapping on the instanced geometry, we 
// need to pass both the world matrix and the inverse world matrix for each instance.
// Given our limit of 256 float4 constants, we set the Objects per batch to 40.
// 40 float3x3 world matrices           = 120 float3/4 constants
// 40 float3x3 world inverse matrices   = 120 float3/4 constants
// we have used 240 float4 constants. The rest 16 are saved to pass other information
// to the instancing shader.
static const u32 maxObjectsPerBatch = 40;

// Prototypes
void ProcessObjects( void* Data );

static const u32   UpdateGrainSize = 120;

DEFINE_SPIN_MUTEX(GraphicsScene::m_mutex);

pcstr GraphicsScene::sm_kapszPropertyNames[] =
{
	"ResourceLocation", "DelResourceLocation",
	"AmbientLight", "Shadows", "ShadowColor", 
	"DrawBoundingBox", "ShowNormals", "ShowTangents",
	"UseStaticGeom", "UseInstancedGeom", "FogColor", "Fog", "Font",
	"PagedGeometry", "PagedGeometryTerrain", "PagedGeometryTerrainOffset"
};

const Properties::Property GraphicsScene::sm_kaDefaultProperties[] =
{
	Properties::Property( sm_kapszPropertyNames[ Property_ResourceLocation ],
	VALUE4( Properties::Values::String,
	Properties::Values::String,
	Properties::Values::String,
	Properties::Values::Boolean ),
	Properties::Flags::Valid | Properties::Flags::Multiple | Properties::Flags::InitOnly,
	"Path", "Type", "Group", "Recursive",
	"", "", "", 0 ),

	Properties::Property( sm_kapszPropertyNames[ Property_DelResourceLocation ],
	VALUE1x2( Properties::Values::String ),
	Properties::Flags::Valid | Properties::Flags::Multiple | Properties::Flags::InitOnly,
	"Path", "Group", NULL, NULL,
	"", "" ),

	Properties::Property( sm_kapszPropertyNames[ Property_AmbientLight ],
	Properties::Values::Color3,
	Properties::Flags::Valid,
	"R", "G", "B", NULL,
	Math::Color3::Black ),
	Properties::Property( sm_kapszPropertyNames[ Property_Shadows ],
	VALUE1x2( Properties::Values::Boolean ),
	Properties::Flags::Valid,
	NULL, NULL, NULL, NULL,
	0, 0 ),
	Properties::Property( sm_kapszPropertyNames[ Property_ShadowColor ],
	Properties::Values::Color3,
	Properties::Flags::Valid,
	"R", "G", "B", NULL,
	Math::Color3::Black ),
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
	Properties::Property( sm_kapszPropertyNames[ Property_UseStaticGeom ],
	Properties::Values::Boolean,
	Properties::Flags::Valid | Properties::Flags::InitOnly,
	NULL, NULL, NULL, NULL,
	1 ),
	Properties::Property( sm_kapszPropertyNames[ Property_UseInstancedGeom ],
	Properties::Values::Boolean,
	Properties::Flags::Valid | Properties::Flags::InitOnly,
	NULL, NULL, NULL, NULL,
	0 ),
	Properties::Property( sm_kapszPropertyNames[ Property_FogColor ],
	Properties::Values::Color3,
	Properties::Flags::Valid | Properties::Flags::InitOnly,
	"R", "G", "B", NULL,
	0 ),
	Properties::Property( sm_kapszPropertyNames[ Property_Fog ],
	VALUE4(
	Properties::Values::Int32,
	Properties::Values::Float32,
	Properties::Values::Float32,
	Properties::Values::Float32),
	Properties::Flags::Valid | Properties::Flags::InitOnly,
	"Mode", "Density", "Start", "Stop",
	0 ),
	Properties::Property( sm_kapszPropertyNames[ Property_Font ],
	VALUE1x2(Properties::Values::String),
	Properties::Flags::Valid | Properties::Flags::InitOnly,
	"FontPath", "FontName", NULL, NULL,
	"", "" ),
	Properties::Property( sm_kapszPropertyNames[ Property_PagedGeometry ],
	VALUE4(Properties::Values::String,
	Properties::Values::String,
	Properties::Values::Float32,
	Properties::Values::Float32),
	Properties::Flags::Valid | Properties::Flags::InitOnly,
	"Heightmap", "ResourceGroup", "Pagesize", "Drawdistance",
	"", "", 100.0, 1000.0),
	Properties::Property( sm_kapszPropertyNames[ Property_PagedGeometryTerrain ],
	VALUE1x3(Properties::Values::Float32),
	Properties::Flags::Valid | Properties::Flags::InitOnly,
	"Width", "Length", "Height", NULL,
	100.0, 100.0, 100.0),
	Properties::Property( sm_kapszPropertyNames[ Property_PagedGeometryTerrainOffset ],
	VALUE1x3(Properties::Values::Float32),
	Properties::Flags::Valid | Properties::Flags::InitOnly,
	"Width", "Length", "Height", NULL,
	0.0, 0.0, 0.0),
};

void GraphicsScene::storeLcoeffs( SHLight* shlight )
{
	m_shLights.push_back(shlight);

	m_currentLight = m_shLights.size() - 1;		//make this the current light to use for rendering
	//load the SH light into the shader:
	SetActiveLight(m_currentLight);
}

void GraphicsScene::SetActiveLight( int lid )
{
	const SHLight* shlight = GetCurrentLight();
	//update uniform constants
	//if (m_vertexShader) {
	//	shlight->bindToShader( m_vertexShader );
	//}
}

bool GraphicsScene::Trace( HitInfo& minHit, const Ray& ray, float tMin /*= 0.0f*/, float tMax /*= MIRO_TMAX*/ ) const
{
	float3 point;
	const BTri *tri;

	noVec3 intersectPos;
	vec3 v1((ray.o + ray.d).ToFloatPtr());
	if (bsp.intersects(vec3(ray.o.ToFloatPtr()), v1, &point, &tri))
	{
		vec3 newPos = point + tri->plane.xyz();
		intersectPos.Set(newPos.x, newPos.y, newPos.z);
		minHit.P = intersectPos;
		return true;
	}
	
	return false;
}

void GraphicsScene::BuildKDTree()
{
	dbCompositionList list;
	g_database.ComposeList( list, OBJECT_Character );

	for(dbCompositionList::iterator it = list.begin(); it != list.end(); ++it)
	{		
		M2Mesh* pMesh = (M2Mesh*)(*it)->GetNode();
		unsigned short i0, i1, i2;
		noVec3 v0, v1, v2;
		for (int i =0; i < pMesh->m2_->nIndices; i+=3)
		{
			i0 = pMesh->m2_->indices[i];
			i1 = pMesh->m2_->indices[i+1];
			i2 = pMesh->m2_->indices[i+2];
			v0 = pMesh->m2_->vertDatas[i0].pos;
			v1 = pMesh->m2_->vertDatas[i1].pos;	
			v2 = pMesh->m2_->vertDatas[i2].pos;
			bsp.addTriangle(v0.ToFloatPtr(), v1.ToFloatPtr(), v2.ToFloatPtr());
		}
	}

	bsp.build();

}

void GraphicsScene::LoadOBJ( char* filename, noVec3& a_Pos, float a_Scale, bool a_Static )
{
	
}


GraphicsScene::GraphicsScene( ISystem* pSystem ) : ISystemScene( pSystem )
	, m_pTask( NULL )
	, m_Extends(BoundingBox( vec3_zero, vec3_zero ))
	, m_bUseStaticGeom( 1 )
	, m_bUseInstancedGeom( 0 )
	, m_bParallelize(False)
{
	ASSERT( Property_Count == sizeof sm_kapszPropertyNames / sizeof sm_kapszPropertyNames[ 0 ] );
	ASSERT( Property_Count == sizeof sm_kaDefaultProperties / sizeof sm_kaDefaultProperties[ 0 ] );
}

GraphicsScene::~GraphicsScene()
{
	SAFE_DELETE( m_pTask );

}

void GraphicsScene::Update( f32 DeltaTime )
{
	Bool bPaused = g_GraphicsManagers.pEnvironment->Runtime().GetStatus() ==
		IEnvironment::IRuntime::Status::Paused;
	m_bPause = bPaused;
	m_fDeltaTime = DeltaTime;

	if( m_bUseInstancedGeom )
	{

	}

	u32         size = (u32)m_Objects.size();

	if (m_bParallelize && ( g_GraphicsManagers.pTask != NULL ) && ( UpdateGrainSize < size ))
	{
		g_GraphicsManagers.pTask->ParallelFor( m_pTask, UpdateCallback, this, 0, size, UpdateGrainSize );
	}
	else
	{
		ProcessRange( 0, size );
	}

	// Terrain Update
}

System::Type GraphicsScene::GetSystemType( void )
{
	return System::Types::Graphics;
}

void GraphicsScene::GlobalSceneStatusChanged( GlobalSceneStatus Status )
{
	// Create the StaticGeom and InstancedGeom objects after we finish loading all
	// the separate graphic objects.
	if( Status == GlobalSceneStatus::PostLoadingObjects )
	{
		if( m_bUseStaticGeom )
		{
			// Check each object for Static options
			for ( ObjectsList::iterator it = m_Objects.begin(); it != m_Objects.end(); it++ )
			{
				// If Mesh is specified as part of a StaticGeom group
				if( ((*it)->GetType() == GraphicsObject::Type_Mesh) &&
					((dynamic_cast<GraphicsObjectMesh*>(*it)->m_strStaticGrpName).length() != 0) )
				{
					GraphicsObjectMesh* pObjMesh = dynamic_cast<GraphicsObjectMesh*> (*it);
				}
			}
		}

		if( m_bUseInstancedGeom )
		{


		}
	}
}
			

Error GraphicsScene::Initialize( Properties::Array Properties )
{
	Error Err = Errors::Success;

	ASSERT( !m_bInitialized );

	m_pTask = new GraphicsTask( this );
	ASSERT( m_pTask != NULL );

	//
	// Set this set as initialized.
	//
	m_bInitialized = True;
	//
	// Read the properties.
	//
	for ( Properties::Iterator it=Properties.begin(); it != Properties.end(); it++ )
	{
		if ( it->GetFlags() & Properties::Flags::Valid &&
			it->GetFlags() & Properties::Flags::InitOnly)
		{
			std::string sName = it->GetName();
			if ( sName == sm_kapszPropertyNames[ Property_ResourceLocation ] )
			{
				pcstr pszName = it->GetStringPtr( 0 );
				pcstr pszLocationType = it->GetStringPtr( 1 );
				pcstr pszResourceGroup = it->GetStringPtr( 2 );
				Bool  bRecursive = it->GetBool( 3 ); 
			}
			else if ( sName == sm_kapszPropertyNames[ Property_DelResourceLocation ] )
			{
				pcstr pszName = it->GetStringPtr( 0 );
				pcstr pszResourceGroup = it->GetStringPtr( 1 );

			}
			else if ( sName == sm_kapszPropertyNames[ Property_UseStaticGeom ] )
			{
				m_bUseStaticGeom = it->GetBool( 0 );
			}
			else if ( sName == sm_kapszPropertyNames[ Property_UseInstancedGeom ] )
			{
				m_bUseInstancedGeom = it->GetBool( 0 );
			}
			else if ( sName == sm_kapszPropertyNames[ Property_FogColor ] )
			{
				m_FogColor.r = it->GetColor3().r;
				m_FogColor.g = it->GetColor3().g;
				m_FogColor.b = it->GetColor3().b;
			}
			else if ( sName == sm_kapszPropertyNames[ Property_Fog ] )
			{
				i32 mode = it->GetInt32(0);
				switch ( mode )
				{
				case 1:
					m_FogMode = ::FOG_EXP;
					break;
				case 2:
					m_FogMode = ::FOG_EXP2;
					break;
				case 3:
					m_FogMode = ::FOG_LINEAR;
					break;
				default:
					m_FogMode = ::FOG_NONE;
					break;
				}
				m_ExpDensity  = it->GetFloat32(1);
				m_LinearStart = it->GetFloat32(2);
				m_LinearEnd   = it->GetFloat32(3);
			}
			else if ( sName == sm_kapszPropertyNames[ Property_Font ] )
			{
				//-------------------------------------------------------------------------
				// Here is where we load the "default" font into the  resource system. We don't need
				// to keep any references to the resource around as it will be accessed by name by anyone
				// who needs the resource.
				//
				std::string sPath = it->GetString(0);
				std::string sFile = it->GetString(1);
			}
			//
			// Load the paged geometry information. This is broken up between three areas. Here, the camera,
			// and the individual layers in ObjectPagedGeometryLayer.cpp. Here, we initialize the system and
			// load general information that will be shared between all layers. In ObjectCamera the camera
			// needs to be associated with PagedGeometry. There can be multiple layers (e.g. grass, rocks, flowers)
			// so they are taken care of as their own objects.
			//
			else if ( sName == sm_kapszPropertyNames[ Property_PagedGeometry ] )
			{
				// Terrain Setting
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

	//
	// Intialize any resource groups added by the scene
	//
	//
	// Set the properties for this scene.
	//
	SetProperties( Properties );

	//
	// Set some of the shadow properties even though shadows might be disabled.
	//
	// Shadow Setting


	//-------------------------------------------------------------------------
	// This is where we "create" the overlay manager by retrieving its pointer. We
	// also create one overlay which is currently the only one in the system. All
	// overlay elements are currently attached to it.
	//

	//-------------------------------------------------------------------------
	// Here we set the fog properties for the scene
	//
	return Err;
}

void GraphicsScene::GetProperties( Properties::Array& Properties )
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

}

void GraphicsScene::SetProperties( Properties::Array Properties )
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


			if ( sName == sm_kapszPropertyNames[ Property_AmbientLight ] )
			{
				//
				// Set the scene's ambient light.
				//
				ColourValue AmbientLight;

				AmbientLight.a = 1.0f;
				AmbientLight.r = it->GetColor3().r;
				AmbientLight.g = it->GetColor3().g;
				AmbientLight.b = it->GetColor3().b;
			}
			else if ( sName == sm_kapszPropertyNames[ Property_Shadows ] )
			{
				//
				// Enable/disable shadows.
				//
			}
			else if ( sName == sm_kapszPropertyNames[ Property_ShadowColor ] )
			{
				//
				// Set the scene's shadow color.
				//
				ColourValue ShadowColor;

				ShadowColor = ColourValue( it->GetColor3().r,
					it->GetColor3().g,
					it->GetColor3().b );
			}
			else if ( sName == sm_kapszPropertyNames[ Property_DrawBoundingBox ] )
			{
				//
				// Enable/disable the drawing of Object BoundingBoxes.
				//
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

pcstr* GraphicsScene::GetObjectTypes( void )
{
	return GraphicsObject::sm_kapszTypeNames;
}

ISystemObject* GraphicsScene::CreateObject( pcstr pszName, pcstr pszType )
{
	ASSERT( m_bInitialized );

	GraphicsObject* pObject = NULL;

	if ( strcmp( pszType,
		GraphicsObject::sm_kapszTypeNames[ GraphicsObject::Type_Mesh ] ) == 0 )
	{
		//
		// Create and return the  graphics object.
		//
		pObject = new GraphicsObjectMesh( this, pszName );
	}
	else if ( strcmp( pszType,
		GraphicsObject::sm_kapszTypeNames[ GraphicsObject::Type_PointList ] ) == 0 )
	{
		//
		// Create and return the  graphics object.
		//
		//pObject = new GraphicsObjectParticles( this, pszName );
	}
	else if ( strcmp( pszType,
		GraphicsObject::sm_kapszTypeNames[ GraphicsObject::Type_Light ] ) == 0 )
	{
		//
		// Create and return the  graphics object.
		//
		//pObject = new GraphicsObjectLight( this, pszName );
	}
	else if ( strcmp( pszType,
		GraphicsObject::sm_kapszTypeNames[ GraphicsObject::Type_LightFire ] ) == 0 )
	{
		//
		// Create and return the  graphics object.
		//
		//pObject = new GraphicsObjectLightFire( this, pszName );
	}
	else if ( strcmp( pszType,
		GraphicsObject::sm_kapszTypeNames[ GraphicsObject::Type_Camera ] ) == 0 )
	{
		//
		// Create and return the  graphics object.
		//
		//pObject = new GraphicsObjectCamera( this, pszName );
	}
	else if ( strcmp( pszType,
		GraphicsObject::sm_kapszTypeNames[ GraphicsObject::Type_MeshAnimated ] ) == 0 )
	{
		//
		// Create and return the  graphics object.
		//
		//pObject = new GraphicsObjectMeshAnimated( this, pszName );
	}
	else
	{
		ASSERT( False );
	}

	//
	//  Store the newly created object for future access
	//
	if ( pObject != NULL )
	{
		m_Objects.push_back( pObject );
	}
	else
	{
		ASSERT( False );
	}

	return pObject;
}

Error GraphicsScene::DestroyObject( ISystemObject* pSystemObject )
{
	ASSERT( m_bInitialized );
	ASSERT( pSystemObject != NULL );

	//
	// Cast to a GraphicsObject so that the correct destructor will be called.
	//
	GraphicsObject* pObject =
		reinterpret_cast<GraphicsObject*>(pSystemObject);

	if ( pObject != NULL )
	{
		//
		// Remove the object from the list and delete it.
		//
		m_Objects.erase( std::find(m_Objects.begin(), m_Objects.end(), pObject) );

		delete pObject;
	}

	return Errors::Success;
}

ISystemTask* GraphicsScene::GetSystemTask( void )
{
	return m_pTask;
}

System::Changes::BitMask GraphicsScene::GetPotentialSystemChanges( void )
{
	return System::Changes::None;
}

const void* GraphicsScene::GetSystemChangeData( System::Change SystemChange )
{
	UNREFERENCED_PARAM( SystemChange );

	return NULL;
}

void GraphicsScene::UpdateCallback( void *param, u32 begin, u32 end )
{
	//    ASSERT ( dynamic_cast<GraphicsScene*>(param));
	GraphicsScene* pThis = static_cast<GraphicsScene*>(param);

	pThis->ProcessRange( begin, end );
}

void GraphicsScene::ProcessRange( u32 begin, u32 end )
{
	static const u32 nonPausable = 
		GraphicsObject::Type_Camera |
		GraphicsObject::Type_Window |
		GraphicsObject::Type_StatWindow |
		GraphicsObject::Type_Chart |
		GraphicsObject::Type_CPUChart |
		GraphicsObject::Type_WorkloadWindow;

	for ( size_t i = begin; i < end; ++i )
	{
		GraphicsObject* pObject = m_Objects[i];

		// Update objects based on paused state
		if ( !m_bPause  || pObject->GetType() & nonPausable )
		{
			// Process this object
			pObject->Update( m_fDeltaTime );
		}
	}
}

