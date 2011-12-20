/************************************************************************
                                                                      
************************************************************************/
#pragma once 
#ifndef U2_DX9_SHADERMGR_H
#define U2_DX9_SHADERMGR_H

#include <U2Lib/Src/Memory/U2MemObj.h>
#include <U2Lib/Src/U2Util.h>
#include <U2_3D/Src/U2_3DLibType.h>

class U2Dx9Renderer;


class U2Dx9ShaderDesc : public U2MemObj
{
public:
	enum ShaderType
	{
		SHADERTYPE_VERTEX = 0,
		SHADERTYPE_PIXEL = 1,
		SHADERTYPE_FORCEWORD = 0xffffffff
	};
	
	U2String	m_szShaderDefFile;	// 텍스트 파일 이름
	U2String	m_szShaderObjFile;	// 컴파일된 쉐이더 오브젝트 파일

	void*		m_pShader;
	DWORD m_uDecl;
	ShaderType	m_type;

	U2Dx9ShaderDesc() : m_pShader(0),m_uDecl(0), m_type(SHADERTYPE_FORCEWORD)
	{

	}

};

typedef DWORD	ShaderIndex;
typedef std::vector<U2Dx9ShaderDesc, U2PoolAllocator<U2Dx9ShaderDesc> > ShaderDescArray;
typedef std::vector<ShaderIndex, U2PoolAllocator<ShaderIndex> > ShaderIdxArray;

class U2_3D	U2Dx9ShaderMgr : public U2MemObj
{
public:	
	virtual ~U2Dx9ShaderMgr();

	static void Create();
	static void Terminate();

	HRESULT Init( U2Dx9Renderer *pRenderer);

	static U2Dx9ShaderMgr* GetInstance();

	U2String	GetShaderObjName( ShaderIndex idx);
	void		ListAllShaders();

	HRESULT U2Dx9ShaderMgr::CreateShader( LPD3DXBUFFER pbufShader,
		U2Dx9ShaderDesc::ShaderType ShaderType,
		U2Dx9ShaderDesc * pDesc );

	HRESULT LoadAndCreateShaderFromMemory( const char * program_asm_code,	// ASCII assembly code
		const TCHAR * shader_name,
		U2Dx9ShaderDesc::ShaderType eShaderType,
		ShaderIndex * outIndex );

	HRESULT LoadAndAssembleShader( U2Dx9ShaderDesc * pDesc );


	HRESULT LoadAndAssembleShader( const TCHAR * file_name,
		U2Dx9ShaderDesc::ShaderType ShaderType,
		ShaderIndex * outIndex,
		bool bVerbose );

	HRESULT SetShader( ShaderIndex index );


	HRESULT HandleErrorString( const U2String & strError );

	bool IsShaderLoaded( const U2String & strFilePath, ShaderIndex * out_index );

	HRESULT ReloadAllShaders( bool bVerbose );
	HRESULT ReloadShader( size_t index, bool bVerbose );


			
private:
	U2Dx9ShaderMgr();

	static HRESULT	FreeShader(U2Dx9ShaderDesc *pDesc);

	
	ShaderDescArray m_shaders;
	ShaderIdxArray m_shaderIndices;
	

	static U2Dx9ShaderMgr* ms_pInstance;
	static LPDIRECT3DDEVICE9 ms_pD3DDev;
	static U2Dx9Renderer*	ms_pRenderer;

};



#endif