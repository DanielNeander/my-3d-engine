// Global.h:
// Created by: Aurelio Reis

#ifndef __AR__GLOBAL__H__
#define __AR__GLOBAL__H__


////////////////////////////////////////////////////////////////////
// AReis:
////////////////////////////////////////////////////////////////////

// Proper C++ made easy.
#define DEFINE_GETSET_ACCESSOR( type, var, name )										\
	type Get##name() { return var; }													\
	void Set##name( type otherVar ) { var = otherVar; }

#define DEFINE_GETSET_ACCESSOR_REFERENCE( type, var, name )								\
	type &Get##name() { return var; }													\
	void Set##name( type &otherVar ) { var = otherVar; }

#define DEFINE_GETSET_ACCESSOR_POINTER( type, var, name )								\
	type *Get##name() { return var; }													\
	void Set##name( type *otherVar ) { var = otherVar; }

#define SETBIT( x ) ( 1 << x )

#define SQUARE( x )		( ( x ) * ( x ) )
const float TWO_PI = 3.14f * 2.0f;
const float invRandMax = 1.0f / RAND_MAX;
const float invRandMaxTimesPI = TWO_PI * invRandMax;

__forceinline float RandFloat( float fLow, float fHigh ) { return fLow + ( fHigh - fLow ) * ( (float)rand() ) / RAND_MAX; }
__forceinline int RandInt( int iLow, int iHigh ) { return iLow + rand() % ( iHigh - iLow + 1 ); }

#define		DEG_TO_RAD( x )		( x ) * 0.0174532925f

// Skip warning about truncating void pointer (I meant to do that).
#pragma warning( disable : 4311 )

// Skip warning about depreciated functions (thanks anyways dx).
#pragma warning( disable : 4995 )

#pragma warning( disable : 4996 )

// STL warnings.
#pragma warning( disable : 4530 )
#pragma warning( disable : 80 )
#pragma warning( disable : 83 )
#pragma warning( disable : 804 )

extern HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, const WCHAR * strFileName, 
						const D3DVERTEXELEMENT9* aMeshDecl,
						LPDIRECT3DVERTEXBUFFER9* ppVB,LPDIRECT3DINDEXBUFFER9* ppIB, 
						int *pNumPolys, int *pNumVerts, int *pNumIndices );

extern IDirect3DDevice9 *g_pd3dDevice;
extern IDirect3DVertexDeclaration9 *g_pDecl;

extern DXUTDeviceSettings		*g_pDeviceSettings;

#include <vector>
#include <algorithm>
using namespace std;

// Send a message to the debugger.
void PrintMessage( const char *strFormat, ... );

// Format and copy a string with variable argument into a temp buffer.
const char *VarArg( char *strFormat, ... );

#if 1
	#define PERF_BEGIN( _Name )		D3DPERF_BeginEvent( 0xFFFFFFFF, L##_Name )
	#define PERF_END()				D3DPERF_EndEvent()
	#define PERF_SCOPE( _Name )		CArPerfScopeGuard g_PerfGuard( L##_Name );
#else
	#define PERF_BEGIN( _Name )
	#define PERF_END()
	#define PERF_SCOPE( _Name )
#endif


class CArPerfScopeGuard
{
public:
	CArPerfScopeGuard( LPCWSTR strName ) { D3DPERF_BeginEvent( 0xFFFFFFFF, strName ); }
	~CArPerfScopeGuard() { PERF_END(); }
};


class CArPerfCounters
{
public:
	int m_iNumVerticesDrawn;
	int m_iNumBatchsDrawn;
	int m_iNumTrisDrawn;
	int m_iNumQuadsDrawn;
	int m_iRenderMs;

	void FrameReset()
	{
		m_iNumVerticesDrawn = 0;
		m_iNumBatchsDrawn = 0;
		m_iNumTrisDrawn = 0;
		m_iNumQuadsDrawn = 0;
		m_iRenderMs = 0;
	}
};


extern CArPerfCounters g_PC;

extern D3DXVECTOR4 g_vTexelSize;
extern D3DXVECTOR4 g_vInvScreenSize;

extern const D3DXVECTOR2 VEC2_ZERO;
extern const D3DXVECTOR2 VEC2_ONE;

extern const D3DXVECTOR4 VEC4_ZERO;
extern const D3DXVECTOR4 VEC4_ONE;

extern void MakeFilePathCanonical( string &strFilePath );
extern void ProjectScreenToWorld( D3DXVECTOR3 &vResult, int x, int y, float fDepth );

extern double g_fTime;

extern HWND g_hWnd;

extern LPD3DXEFFECT g_pEffect;
extern IDirect3DVertexDeclaration9 *g_pDecl;
class CModelViewerCamera;
extern CModelViewerCamera g_Camera;

extern float g_fElapsedTime;
extern UINT32 g_uiFrameNum;

extern class CArBaseTexture *g_pTexture;

extern bool g_bDrawSkeleton;
extern bool g_bDrawDamageZones;
extern bool g_bDrawWireframe;
extern bool g_bTintSurfaces;
extern bool g_bSkipOptimizedLimbs;
extern float g_fGlobalAnimScale;

extern void CalcQuaternionW( D3DXQUATERNION &Q );

extern D3DXMATRIXA16 FlipMatrix;
extern D3DXQUATERNION FlipQuat;

enum { COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_ORANGE, COLOR_PURPLE, COLOR_CYAN, MAX_COLORS };
extern UINT32 g_Colors[ MAX_COLORS ];

extern void DrawAxis( float fAxisLen, const D3DXMATRIXA16 &Transform, bool bDrawAlways = false );
extern void AddUniqueJoint( vector< int > &JointList, int iJoint );

#endif // __AR__GLOBAL__H__
