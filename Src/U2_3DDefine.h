#pragma once 



#define DATA_PATH		_T("Data")
#define ASM_SHADER_PATH		_T("Data\\shaders\\asm")
#define FX_SHADER_PATH		_T("Data\\shaders\\fx")

#define TEXTURE_PATH		_T("Data\\texture\\")
#define MODEL_PATH			_T("Data\\models\\")

#define ANIM_DIR			_T("Data\\models\\")
#define MESH_DIR			_T("Data\\models\\")

#define COLOR_WHITE		D3DXCOLOR(1.f, 1.f, 1.f, 1.f)
#define COLOR_BLACK		D3DXCOLOR(0.f, 0.f, 0.f, 1.f)


#if defined(DEBUG) | defined(_DEBUG)
#ifndef V
#define V(x)           { hr = x; if( FAILED(hr) ) { DXUTTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true ); } }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = x; if( FAILED(hr) ) { return DXUTTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true ); } }
#endif
#else
#ifndef V
#define V(x)           { hr = x; }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = x; if( FAILED(hr) ) { return hr; } }
#endif
#endif

#ifndef FAIL_IF_NULL
#define FAIL_IF_NULL( x )								\
{														\
	if( (x) == NULL )	{ return( E_FAIL ); }			\
}

#ifndef BREAK_AND_RET_VAL_IF_FAILED
#define BREAK_AND_RET_VAL_IF_FAILED( hr )						\
{																\
	if( FAILED(hr) )	{ U2ASSERT(false); return( hr ); }		\
}
#endif

#ifndef MSG_BREAK_AND_RET_VAL_IF
#define MSG_BREAK_AND_RET_VAL_IF( expr, msg, retval )					\
{																		\
	if( expr )	{ FMsg(msg); U2ASSERT(false); return(retval);	}			\
}
#endif

#endif

#define SAFE_RELEASE(p)												\
	{																	\
	int refc = -1;													\
	if( (p) != NULL )												\
		{																\
		refc = (int)((p)->Release());								\
		(p) = NULL;													\
		}																\
	}

#define SAFE_ADDREF(p)										\
	{															\
	if( (p) != NULL )										\
		{														\
		(p)->AddRef();										\
		}														\
	}


#define IsRendererOK			U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();	\
										if (0 == pRenderer)															\
											return		

#define IsRendererOK2			U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();	\
										if (0 == pRenderer)															\
											return false
									