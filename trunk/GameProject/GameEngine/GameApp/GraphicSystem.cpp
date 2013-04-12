#include "stdafx.h"
#include <shellAPI.h>
#include "EngineCore/Direct3D11/Direct3D11Renderer.h"
#include "ColorValue.h"
#include "Common.h"

//
// core includes
//
#include ".\BaseTypes\BaseTypes.h"
#include ".\Interfaces\Interface.h"

#include "GraphicSystem.h"
#include "GraphicsScene.h"


ManagerInterfaces       g_GraphicsManagers;

//
// Static member variables
//
pcstr GraphicsSystem::sm_kapszPropertyNames[] =
{
	"ResourceLocation",
	"WindowName",
	"Resolution",
	"ShadowTextureCount",
	"ShadowTextureSize",
	"FullScreen",
	"VerticalSync",
	"FSAntiAliasing",
};

const Properties::Property GraphicsSystem::sm_kaDefaultProperties[] =
{
	Properties::Property( sm_kapszPropertyNames[ Property_ResourceLocation ],
	VALUE4( Properties::Values::String,
	Properties::Values::String,
	Properties::Values::String,
	Properties::Values::Boolean ),
	Properties::Flags::Valid | Properties::Flags::InitOnly |
	Properties::Flags::Multiple,
	"Path", "Type", "Group", "Recursive",
	"", "", "", 0 ),

	Properties::Property( sm_kapszPropertyNames[ Property_WindowName ],
	VALUE1( Properties::Values::String ),
	Properties::Flags::Valid | Properties::Flags::InitOnly,
	NULL, NULL, NULL, NULL,
	"" ),
	Properties::Property( sm_kapszPropertyNames[ Property_Resolution ],
	VALUE1x2( Properties::Values::Int32 ),
	Properties::Flags::Valid,
	NULL, NULL, NULL, NULL,
	1024, 768 ),
	Properties::Property( sm_kapszPropertyNames[ Property_ShadowTextureCount ],
	VALUE1( Properties::Values::Int32 ),
	Properties::Flags::Valid,
	NULL, NULL, NULL, NULL,
	1 ),
	Properties::Property( sm_kapszPropertyNames[ Property_ShadowTextureSize ],
	VALUE1( Properties::Values::Int32 ),
	Properties::Flags::Valid,
	NULL, NULL, NULL, NULL,
	1024 ),
	Properties::Property( sm_kapszPropertyNames[ Property_FullScreen ],
	VALUE1( Properties::Values::Int32 ),
	Properties::Flags::Valid,
	NULL, NULL, NULL, NULL,
	0 ),
	Properties::Property( sm_kapszPropertyNames[ Property_VerticalSync ],
	VALUE1( Properties::Values::Int32 ),
	Properties::Flags::Valid,
	NULL, NULL, NULL, NULL,
	0 ),
	Properties::Property( sm_kapszPropertyNames[ Property_FSAntiAliasing ],
	VALUE1x2( Properties::Values::String),
	Properties::Flags::Valid,
	"Type", "Quality", NULL, NULL,
	"0" /* D3DMULTISAMPLE_NONE */,
	"0" /* Default Quality level offered by driver */ ),
};


GraphicsSystem::GraphicsSystem( void )     : ISystem()
	, m_pRenderer(NULL)
	, m_uShadowTextureSize( 0 )
	, m_uShadowTextureCount( 0 )
{
	ASSERT( Property_Count == (sizeof sm_kapszPropertyNames / sizeof sm_kapszPropertyNames[ 0 ]) );
	ASSERT( Property_Count == (sizeof sm_kaDefaultProperties / sizeof sm_kaDefaultProperties[ 0 ]) );
}

GraphicsSystem::~GraphicsSystem( void )
{

}

pcstr GraphicsSystem::GetName( void )
{
	return System::Names::Graphics;
}

System::Type GraphicsSystem::GetSystemType( void )
{
	return System::Types::Graphics;
}

Error GraphicsSystem::Initialize( Properties::Array Properties )
{
	ASSERT( !m_bInitialized );

	// Read in the properties required to initialize Ogre.
	//
	char    szWindowName[ 256 ] = "Window";
	u32		Width = 1280;
	u32		Height = 1024;
	Bool    bFullScreen = False;
	Bool    bVerticalSync = True;
	std::string     dFSAAType    = "0";  //D3DMULTISAMPLE_NONE;
	std::string     dFSAAQuality = "0";

	for ( Properties::Iterator it=Properties.begin(); it != Properties.end(); it++ )
	{
		//
		// Make sure this property is valid.
		//
		if ( it->GetFlags() & Properties::Flags::Valid )
		{
			std::string sName = it->GetName();

			if ( sName == sm_kapszPropertyNames[ Property_ResourceLocation ] )
			{
				pcstr pszName = it->GetStringPtr( 0 );
				pcstr pszLocationType = it->GetStringPtr( 1 );
				pcstr pszResourceGroup = it->GetStringPtr( 2 );
				Bool  bRecursive = it->GetBool( 3 ); 

				/*m_pResourceGroupManager->addResourceLocation(
					pszName, pszLocationType, pszResourceGroup, (bRecursive == True) );

				m_pResourceGroupManager->loadResourceGroup( pszResourceGroup );*/
			}
			else if ( sName == sm_kapszPropertyNames[ Property_WindowName ] )
			{
				strcpy_s( szWindowName, sizeof szWindowName, it->GetStringPtr( 0 ) );
			}
			else if ( sName == sm_kapszPropertyNames[ Property_Resolution ] )
			{
				Width  = static_cast<u32>(it->GetInt32( 0 ));
				Height  = static_cast<u32>(it->GetInt32( 1 ));
			}
			else if ( sName == sm_kapszPropertyNames[ Property_ShadowTextureCount ] )
			{
				m_uShadowTextureCount  = static_cast<u16>(it->GetInt32( 0 ));
			}
			else if ( sName == sm_kapszPropertyNames[ Property_ShadowTextureSize ] )
			{
				m_uShadowTextureSize  = static_cast<u16>(it->GetInt32( 0 ));
			}
			else if ( sName == sm_kapszPropertyNames[ Property_FullScreen ] )
			{
				bFullScreen = it->GetBool( 0 );
			}
			else if ( sName == sm_kapszPropertyNames[ Property_VerticalSync ] )
			{
				bVerticalSync = it->GetBool( 0 );
			}
			else if ( sName == sm_kapszPropertyNames[ Property_FSAntiAliasing ] )
			{                
				dFSAAType    = it->GetString( 0 );
				dFSAAQuality = it->GetString( 1 );
			}
			//
			// Set this property to invalid since it's already been read.
			//
			it->ClearFlag( Properties::Flags::Valid );
		}
	}

	//
	// Command line overrides
	//
	{
		USES_CONVERSION;

		WCHAR**				argv;
		int					argc, iArg;

		argv = CommandLineToArgvW( GetCommandLineW(), &argc);
		for(iArg=0; iArg<argc; iArg++)
		{
			switch (*argv[iArg])
			{
			case '-':
			case '/':
				{
					if (0==_wcsicmp(argv[iArg]+1, L"windowed" ))	bFullScreen = False;
				}	break;

			default:
				{
				}
			}
		}

		LocalFree(argv);
	}

	//
	// Intialize the render system and render window.
	//
	// Setup the Full-screen Anti-Aliasing mode
	std::map<std::string, std::string> params;
	params[ "FSAA" ]        = dFSAAType;
	params[ "FSAAQuality" ] = dFSAAQuality;


	m_bInitialized = True;

	//
	// Set the remaining properties.
	//
	SetProperties( Properties );

	return Errors::Success;
}

void GraphicsSystem::GetProperties( Properties::Array& Properties )
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
	
}

void GraphicsSystem::SetProperties( Properties::Array Properties )
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

			if ( sName == sm_kapszPropertyNames[ Property_Resolution ] )
			{
				u32 Width  = static_cast<u32>(it->GetInt32( 0 ));
				u32 Height = static_cast<u32>(it->GetInt32( 1 ));

				//m_pRenderWindow->resize( Width, Height );
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

ISystemScene* GraphicsSystem::CreateScene( void )
{
	return new GraphicsScene( this );
}

Error GraphicsSystem::DestroyScene( ISystemScene* pSystemScene )
{
	ASSERT( pSystemScene != NULL );

	GraphicsScene* pScene = reinterpret_cast<GraphicsScene*>(pSystemScene);
	SAFE_DELETE( pScene );

	return Errors::Success;
}


