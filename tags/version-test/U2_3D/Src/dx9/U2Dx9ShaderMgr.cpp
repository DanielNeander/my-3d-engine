#include <U2_3D/Src/U23DLibPCH.h>

#include "U2Dx9Renderer.h"
#include <U2_3D/Src/U2_3DDefine.h>

#include "U2Dx9ShaderMgr.h"


LPDIRECT3DDEVICE9 U2Dx9ShaderMgr::ms_pD3DDev = NULL;
U2Dx9ShaderMgr* U2Dx9ShaderMgr::ms_pInstance = NULL;

U2Dx9ShaderMgr::U2Dx9ShaderMgr()
{
	

}

U2Dx9ShaderMgr::~U2Dx9ShaderMgr()
{

}


void U2Dx9ShaderMgr::Create()
{
	ms_pInstance = U2_NEW U2Dx9ShaderMgr();
	U2ASSERT(ms_pInstance);
}


void U2Dx9ShaderMgr::Terminate()
{
	size_t i;
	HRESULT hr;

	if(ms_pD3DDev)
	{
		ms_pD3DDev->SetPixelShader( 0 );
		ms_pD3DDev->SetVertexShader( NULL );
		ms_pD3DDev->SetFVF( D3DFVF_XYZ );

		for(i = 1; i < GetInstance()->m_shaders.size() ; ++i)
		{
			hr = FreeShader( &(GetInstance()->m_shaders[i]));
		}
		SAFE_RELEASE(ms_pD3DDev);		
		ms_pRenderer = NULL;
		
	}
	

	U2_DELETE ms_pInstance;
	ms_pInstance = NULL;	
}


U2Dx9ShaderMgr* U2Dx9ShaderMgr::GetInstance()
{
	return ms_pInstance;
}


HRESULT	U2Dx9ShaderMgr::FreeShader(U2Dx9ShaderDesc *pDesc)
{
	FAIL_IF_NULL( pDesc );
	LPDIRECT3DVERTEXSHADER9 pVS;
	LPDIRECT3DPIXELSHADER9 pPS;

	switch( pDesc->m_type )
	{
	case U2Dx9ShaderDesc::SHADERTYPE_VERTEX:
		pVS = (LPDIRECT3DVERTEXSHADER9)pDesc->m_pShader;
		SAFE_RELEASE( pVS );
		pDesc->m_pShader = NULL;
		break;

	case U2Dx9ShaderDesc::SHADERTYPE_PIXEL:
		pPS = (LPDIRECT3DPIXELSHADER9)pDesc->m_pShader;
		SAFE_RELEASE(pPS);
		pDesc->m_pShader = NULL;
		break;
	}
	return ( S_OK );

}



HRESULT U2Dx9ShaderMgr::Init( U2Dx9Renderer *pRenderer)
{
	if(ms_pRenderer)
		return E_FAIL;
	
	U2ASSERT(ms_pRenderer = pRenderer);

	SAFE_ADDREF(ms_pRenderer->GetD3DDevice());

	U2Dx9ShaderDesc* pDesc = U2_NEW U2Dx9ShaderDesc;
	pDesc->m_szShaderDefFile = _T("Null Shader");
	pDesc->m_szShaderObjFile = _T("Null Shader");
	pDesc->m_type			 = U2Dx9ShaderDesc::SHADERTYPE_VERTEX;
	pDesc->m_pShader		 = NULL;
	m_shaders.push_back(*pDesc);
	

	return S_OK;

}


U2String U2Dx9ShaderMgr::GetShaderObjName( ShaderIndex idx)
{
	U2String name;
	if( idx < m_shaders.size() && idx >= 0)
	{
		return m_shaders.at(idx).m_szShaderObjFile;
	}
	else 
	{
		return _T("Indx out of bounds!");
	}

	return _T("");
}


void	U2Dx9ShaderMgr::ListAllShaders()
{
	uint32 i;
	for( i = 0; i < m_shaders.size() ; ++i)
	{
		FMsg("Shader %3.3d   %s\n", i, GetShaderObjName(i).c_str() );
	}
}



HRESULT U2Dx9ShaderMgr::CreateShader( LPD3DXBUFFER pbufShader,
									 U2Dx9ShaderDesc::ShaderType ShaderType,
									U2Dx9ShaderDesc * pDesc )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pbufShader );
	FAIL_IF_NULL( pDesc );
	FAIL_IF_NULL( ms_pD3DDev );

	pDesc->m_type			= ShaderType;

	if( ShaderType == U2Dx9ShaderDesc::SHADERTYPE_VERTEX )
	{
		hr = ms_pD3DDev->CreateVertexShader( (DWORD*) pbufShader->GetBufferPointer(),
			(IDirect3DVertexShader9**) & pDesc->m_pShader );
		SAFE_RELEASE( pbufShader );
		BREAK_AND_RET_VAL_IF_FAILED(hr);		
	}
	else
	{
		hr = ms_pD3DDev->CreatePixelShader( (DWORD*) pbufShader->GetBufferPointer(),
			(IDirect3DPixelShader9**) & pDesc->m_pShader );
		SAFE_RELEASE( pbufShader );		
		BREAK_AND_RET_VAL_IF_FAILED(hr);				
	}
	return( hr );
}



HRESULT U2Dx9ShaderMgr::LoadAndCreateShaderFromMemory( const char * program_asm_code,	// ASCII assembly code
													 const TCHAR * shader_name,
													 U2Dx9ShaderDesc::ShaderType eShaderType,
													 ShaderIndex * outIndex )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( program_asm_code );
	FAIL_IF_NULL( outIndex );

	//@@@@@ check if shader exists!

	U2Dx9ShaderDesc*	pDesc = U2_NEW U2Dx9ShaderDesc;
	pDesc->m_szShaderDefFile = TEXT("");
	pDesc->m_type					= eShaderType;
	pDesc->m_szShaderObjFile		= shader_name;

	//	typedef struct D3DXMACRO {
	//		LPCSTR Name;
	//		LPCSTR Definition;	};
	LPD3DXBUFFER pbufShader, pbufErrors;

	hr = D3DXAssembleShader( program_asm_code, 
		(UINT) strlen( program_asm_code ),
		NULL,		// D3DXMACRO preprocessor definitions
		NULL,		// include directives
		NULL,		// compile option flags
		& pbufShader,
		& pbufErrors );
	if( FAILED( hr ))
	{
		char * pfunc = "D3DXAssembleShader";
		switch( hr )
		{
		case D3DERR_INVALIDCALL : 
			FMsg("%s failed with HRESULT = D3DERR_INVALIDCALL\n", pfunc );
			break;
		case D3DXERR_INVALIDDATA : 
			FMsg("%s failed with HRESULT = D3DXERR_INVALIDDATA\n", pfunc );
			break;
		case E_OUTOFMEMORY : 
			FMsg("%s failed with HRESULT = E_OUTOFMEMORY\n", pfunc );
			break;
		default : 
			FMsg("Unknown HRESULT : %u\n", hr );
			break;
		}

		FMsg("U2Dx9ShaderMgr::D3DXAssembleShader Errors:\n");
		if( pbufErrors != NULL )
		{
			HandleErrorString( (TCHAR*) pbufErrors->GetBufferPointer() );
		}
		else
		{
			FMsg("Error buffer is NULL!\n");
		}
		SAFE_RELEASE( pbufShader );
		BREAK_AND_RET_VAL_IF_FAILED( hr );
	}

	hr = CreateShader( pbufShader, eShaderType, pDesc );

	// Add shader description to the array, set its index and return the index
	//@@@@ make function for this!!	
	m_shaders.push_back( *pDesc );
	* outIndex = (ShaderIndex) m_shaders.size() - 1;
	m_shaderIndices.push_back( *outIndex );
	return( hr );
}


// Assemble shader using DX runtime
HRESULT U2Dx9ShaderMgr::LoadAndAssembleShader( U2Dx9ShaderDesc * pDesc )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDesc );
	FreeShader( pDesc );
	LPD3DXBUFFER pbufShader, pbufErrors;

	//flags:
	//	D3DXSHADER_DEBUG
	//	D3DXSHADER_SKIPVALIDATION
	//	D3DXSHADER_SKIPOPTIMIZATION
	hr = D3DXAssembleShaderFromFile( pDesc->m_szShaderDefFile.c_str(),
		NULL,			// D3DXMACRO defines
		NULL,			// LPD3DXINcLUDE include, NULL = do #include
		0,				// flags
		&pbufShader,	
		&pbufErrors );
	if( FAILED( hr ))
	{
		FMsg("U2Dx9ShaderMgr::LoadAndAssembleShader( U2Dx9ShaderDesc * pDesc ) Errors:\n");
		if( pbufErrors != NULL )
		{
			HandleErrorString( (TCHAR*) pbufErrors->GetBufferPointer() );
		}
		else
		{
			FMsg("Error buffer is NULL!\n");
		}
		SAFE_RELEASE( pbufShader );
		BREAK_AND_RET_VAL_IF_FAILED( hr );
	}

	hr = CreateShader( pbufShader, pDesc->m_type, pDesc );
	if( FAILED(hr) )
	{
		FMsg("LoadAndAssembleShader( U2Dx9ShaderDesc * pDesc ) couldn't create shader : %s\n", pDesc->m_szShaderDefFile.c_str() );
	}

	return( hr );	
}

HRESULT U2Dx9ShaderMgr::LoadAndAssembleShader( const TCHAR * file_name,
											 U2Dx9ShaderDesc::ShaderType ShaderType,
											 ShaderIndex * outIndex,
											 bool bVerbose )


{
	// assemble shader using DX runtime
	HRESULT hr = S_OK;
	MSG_BREAK_AND_RET_VAL_IF( ms_pD3DDev==NULL, TEXT("U2Dx9ShaderMgr::LoadAndAssembleShader(..) U2Dx9ShaderMgr not initialized!\n"), E_FAIL );
	
	/*
	//@@@@@  this is mixing up the shaders
	// Check to see if shader has already been loaded & created
	ShaderIndex index;
	if( IsShaderLoaded( strFilePath, & index ))
	{
	FDebug("Shader [%s] already assembled and has index %u\n", strFilePath.c_str(), index );	
	*outIndex = index;
	return( S_OK );
	}
	//*/

	U2Dx9ShaderDesc* pDesc = U2_NEW U2Dx9ShaderDesc();
	LPD3DXBUFFER pbufShader, pbufErrors;

	//flags:
	//	D3DXSHADER_DEBUG
	//	D3DXSHADER_SKIPVALIDATION
	//	D3DXSHADER_SKIPOPTIMIZATION
	hr = D3DXAssembleShaderFromFile( file_name,
		NULL,			// D3DXMACRO defines
		NULL,			// LPD3DXINcLUDE include, NULL = do #include
		0,				// flags
		&pbufShader,	
		&pbufErrors );
	if( FAILED( hr ))
	{
		FMsg("U2Dx9ShaderMgr::LoadAndAssembleShader Errors:\n");
		if( pbufErrors != NULL )
		{
			HandleErrorString( (TCHAR*) pbufErrors->GetBufferPointer() );
		}
		else
		{
			FMsg("Error buffer is NULL!\n");
		}
		SAFE_RELEASE( pbufShader );
		BREAK_AND_RET_VAL_IF_FAILED( hr );
	}

	hr = CreateShader( pbufShader, ShaderType, pDesc );

	pDesc->m_szShaderDefFile = file_name;
	pDesc->m_szShaderObjFile		= TEXT("");
	pDesc->m_type					= ShaderType;

	m_shaders.push_back( *pDesc );
	* outIndex = (ShaderIndex) m_shaders.size() - 1;
	m_shaderIndices.push_back( *outIndex );
	return( hr );
}


HRESULT U2Dx9ShaderMgr::SetShader( ShaderIndex index )
{
	// sets device's vertex or pixel shader
	HRESULT hr = S_OK;
	MSG_BREAK_AND_RET_VAL_IF( ms_pD3DDev==NULL, TEXT("U2Dx9ShaderMgr::SetShader() U2Dx9ShaderMgr not initialized!\n"), E_FAIL );

	if( index < m_shaders.size() )
	{
		U2Dx9ShaderDesc::ShaderType ShaderType;
		ShaderType	= m_shaders.at(index).m_type;

		switch( ShaderType )
		{
		case U2Dx9ShaderDesc::SHADERTYPE_VERTEX :
			ms_pD3DDev->SetVertexShader( (IDirect3DVertexShader9*) (m_shaders.at(index).m_pShader) );
			break;
		case U2Dx9ShaderDesc::SHADERTYPE_PIXEL :
			ms_pD3DDev->SetPixelShader( (IDirect3DPixelShader9*) (m_shaders.at(index).m_pShader) );
			break;
		default:
			U2ASSERT( false );
			hr = E_FAIL;
			break;
		}
	}
	else
	{
		FMsg("U2Dx9ShaderMgr Index out of bounds! %u\n", index );
		U2ASSERT( false );
		hr = E_FAIL;
	}
	return( hr );
}

HRESULT U2Dx9ShaderMgr::HandleErrorString( const U2String & strError )
{
	HRESULT hr = S_OK;
	U2String::size_type i;
	i = strError.find( TEXT("error") );
	U2String err;
	int column_width = 90;

	if( i > 0 && i < strError.size() )
	{
		err = strError.substr( i, strError.size() );
		i = err.find( TEXT(" "), column_width );
		while( i > 0 && i < err.size() )
		{
			U2String spacer = TEXT("\n     ");
			err.insert( i, spacer );
			i = err.find( TEXT(" "), i + 2 + spacer.size() + column_width );
		}
	}
	else
	{
		err = strError;
	}
	FMsg("[\n%s]\n", err.c_str() );
	return( hr );
}



bool U2Dx9ShaderMgr::IsShaderLoaded( const U2String & strFilePath, ShaderIndex * out_index )
{
	// Checks if shader has already been loaded and created from the source file
	//  and returns the index of the created shader in out_index if it has.
	bool is_loaded = false;
	U2String filename;
	unsigned int i;

	for( i=0; i < m_shaders.size(); i++ )
	{
		filename = m_shaders.at(i).m_szShaderObjFile;
		if( filename == strFilePath )
		{
			is_loaded = true;

			// find the index in m_ShaderIndices
			unsigned int ind;
			for( ind=0; ind < m_shaderIndices.size(); ind++ )
			{
				if( m_shaderIndices.at(ind) == i )
				{
					*out_index = ind;
					return( is_loaded );
				}
			}
			// if not found, keep searching
			is_loaded = false;
		}
	}
	return( false );
}


// Reload all shaders from disk
HRESULT U2Dx9ShaderMgr::ReloadAllShaders( bool bVerbose )
{
	HRESULT hr = S_OK;	
	size_t i;
	for( i=0; i < m_shaders.size(); i++ )
	{
		ReloadShader( i, bVerbose );
	}
	return( hr );
}

HRESULT U2Dx9ShaderMgr::ReloadShader( size_t index, bool bVerbose )
{
	HRESULT hr = S_OK;
	if( index >= m_shaders.size() )
		return( E_FAIL );
	if( index == 0 )
		return( S_OK );

	size_t	defname_length, objname_length;
	defname_length = m_shaders.at(index).m_szShaderDefFile.length();
	objname_length = m_shaders.at(index).m_szShaderObjFile.length();

	if( bVerbose )
		FMsg("defname_length = %u   objname_length = %u\n", defname_length, objname_length );

	if( defname_length > 0 && objname_length > 0 )
	{
		if( bVerbose )
			FMsg("Shader %u has both a text source name and binary object name.  Can't tell whether to compile or create from binary!\n", index );
		U2ASSERT( false );
		return( E_FAIL );
	}

	if( defname_length > 0 )
	{
		hr = LoadAndAssembleShader( &(m_shaders[index]) );
		if( bVerbose )
			FMsg("Loaded shader %s : %s\n", SUCCEEDED(hr) ? "SUCCEEDED" : "FAILED   ", m_shaders[index].m_szShaderDefFile.c_str() );
	}	

	return( hr );
}


