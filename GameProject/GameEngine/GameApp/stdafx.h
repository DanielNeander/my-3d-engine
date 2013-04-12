// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#if defined(DEBUG) || defined(_DEBUG)
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif
#endif

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// MSVC COM Support
#include <comip.h>
#include <comdef.h>
#include <atlcomcli.h> // for CComPtr support


#include <MMSystem.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


#include <dxgi.h>
#include <d3d11.h>
//#include <d3dx11.h>
#include <d3dx11tex.h>
#include <D3dcsx.h>

#include <dxerr.h>
#include <D3dx11effect.h>
#include <D3Dcompiler.h>
#include <d3dx9.h>
//#include <xnamath.h>

#include <dxerr.h>


// wx
#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/string.h>

#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <list>
#include <queue>
#include <string>
#include <algorithm>
#include <iterator>
#include <math.h>

#define WotLK 

#pragma warning( disable : 4100)
#pragma warning( disable : 4101)
#pragma warning( disable : 4189)

// TODO: reference additional headers your program requires here
#include <EngineCore/Platform.h>
#include <EngineCore/Types.h>
#include "EngineCore/Singleton.h"
#include <ModelLib/mpq_libmpq.h>
#include <EngineCore/Util/BSP.h>


#include <EngineCore/RefCount.h>


#include <EngineCore/Heap.h>
#include <EngineCore/Math/Simd.h>
#include <EngineCore/Math/Simd_Generic.h>
#include <EngineCore/Math/Simd_MMX.h>
#include <EngineCore/Math/Simd_3DNow.h>
#include <EngineCore/Math/Simd_AltiVec.h>
#include <EngineCore/Math/Simd_SSE.h>
#include <EngineCore/Math/Simd_SSE2.h>
#include <EngineCore/Math/Simd_SSE3.h>
#include <EngineCore/Util/Str.h>



#include <EngineCore/Math/Curve.h>
#include <EngineCore/Math/Vector.h>
#include <EngineCore/Math/Quaternion.h>
#include <EngineCore/Math/Matrix.h>
#include <EngineCore/Math/Math.h>
#include <EngineCore/Math/Extrapolate.h>
#include <EngineCore/Math/interpolate.h>
#include <EngineCore/Math/Plane.h>
#include <EngineCore/Math/GameMath.h>
#include <EngineCore/Math/noTransform.h>
#include <EngineCore/Math/Rotation.h>
#include <EngineCore/Math/Angles.h>
#include "EngineCore/Math/MD5Bounds.h"
#include "EngineCore/Math/Pluecker.h"
#include "EngineCore/Math/Lcp.h"
#include "EngineCore/Geometry/Winding.h"
#include "EngineCore/Geometry/JointTransform.h"
#include "EngineCore/Geometry/TraceModel.h"
#include "EngineCore/Geometry/DrawVert.h"
#include "EngineCore/Geometry/Surface.h"
#include "EngineCore/Geometry/Surface_Patch.h"
#include "EngineCore/Geometry/Surface_Polytope.h"
#include "EngineCore/Geometry/Surface_SweptSpline.h"
#include <EngineCore/Math/MathTools.h>
#include <EngineCore/Math/BoundingBox.h>


#include <EngineCore/Containers/List.h>
#include <EngineCore/Containers/LinkList.h>
#include <EngineCore/Containers/Queue.h>
#include <EngineCore/Containers/Stack.h>
#include <EngineCore/Containers/StrList.h>
#include <EngineCore/Containers/HashIndex.h>
#include <EngineCore/Containers/HashTable.h>
#include <EngineCore/Containers/StrPool.h>
#include <EngineCore/Containers/BinSearch.h>
#include <EngineCore/Containers/BTree.h>
#include <EngineCore/Containers/Hierarchy.h>


#include <EngineCore/Direct3D11/Direct3D11Renderer.h>
// TODO: reference additional headers your program requires here

#if defined(DEBUG) || defined(_DEBUG)
#ifndef V
#define V(x)           { hr = (x); if( FAILED(hr) ) { return; } }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return; } }
#endif
#else
#ifndef V
#define V(x)           { hr = (x); }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return hr; } }
#endif
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif



// Bullet Physics 
#include "LinearMath/btVector3.h"
#include "LinearMath/btMatrix3x3.h"
#include "LinearMath/btTransform.h"
#include "LinearMath/btQuickprof.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"


#include "Terrain/Errors.h"

#define ID_TIME_T	time_t
#define SIZE16		sizeof(float) * 16

const noVec3 gBlack   (0, 0, 0);
const noVec3 gWhite   (1, 1, 1);

const noVec3 gRed     (1, 0, 0);
const noVec3 gYellow  (1, 1, 0);
const noVec3 gGreen   (0, 1, 0);
const noVec3 gCyan    (0, 1, 1);
const noVec3 gBlue    (0, 0, 1);
const noVec3 gMagenta (1, 0, 1);

const noVec3 gOrange (1, 0.5f, 0);

inline noVec3 grayColor (const float g) {return noVec3 (g, g, g);}


const noVec3 gGray10 = grayColor (0.1f);
const noVec3 gGray20 = grayColor (0.2f);
const noVec3 gGray30 = grayColor (0.3f);
const noVec3 gGray40 = grayColor (0.4f);
const noVec3 gGray50 = grayColor (0.5f);
const noVec3 gGray60 = grayColor (0.6f);
const noVec3 gGray70 = grayColor (0.7f);
const noVec3 gGray80 = grayColor (0.8f);
const noVec3 gGray90 = grayColor (0.9f);

typedef noVec3		FVector;
typedef noVec2		FVector2D;
typedef noMat4		FMatrix;
typedef noQuat		FQuat;

#include "EngineCore/Unreal3/Sorting.h"
#include "EngineCore/Unreal3/Core.h"
#include "EngineCore/Unreal3/UnArc.h"
#include "EngineCore/Unreal3/RefCounting.h"
#include "EngineCore/Unreal3/UnTemplate.h"
#include "EngineCore/Unreal3/Array.h"
#include "EngineCore/Unreal3/UnStringConv.h"
#include "EngineCore/Unreal3/UnString.h"
#include "EngineCore/Unreal3/UnObjBase.h"
#include "EngineCore/Unreal3/Sorting.h"
#include "EngineCore/Unreal3/UnName.h"
#include "EngineCore/Unreal3/UnMathSSE.h"
#include "UnMath.h"
#include "UnColor.h"


#include "RTTI.h"

#pragma comment(lib, "psapi.lib")


#ifdef _DEBUG
#pragma comment(lib, "comsuppwd.lib")
#pragma comment(lib, "D3dcsxd.lib")
#else
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "D3dcsx.lib")
#endif

#include "GameApp/MD5/Md5Timer.h"
#include "GameApp/MD5/CmdArgs.h"
#include "GameApp/MD5/Dict.h"
#include "GameApp/MD5/BitMsg.h"
#include "GameApp/MD5/File.h"
#include "GameApp/MD5/CmdSystem.h"
#include "GameApp/MD5/CVarSystem.h"
#include "GameApp/MD5/Token.h"
#include "GameApp/MD5/Lexer.h"
#include "GameApp/MD5/Parser.h"
#include "GameApp/MD5/DeclManager.h"
#include "GameApp/MD5/MD5SysCVar.h"

#include "GameApp/MD5/Md5Event.h"
#include "GameApp/MD5/Md5Class.h"

// We have expression parsing and evaluation code in multiple places:
// materials, sound shaders, and guis. We should unify them.
const int MAX_EXPRESSION_OPS = 4096;
const int MAX_EXPRESSION_REGISTERS = 4096;

#include <GameApp/MD5/MD5Material.h>
#include "GameApp/MD5/MD5Image.h"
#include "GameApp/MD5/MD5Triangle.h"
#include <GameApp/MD5/MD5MapFile.h>
#include <GameApp/Collision/CollisionModel_local.h>
#include "GameApp/Physics/Physics.h"
#include "GameApp/Physics/Force.h"
#include "GameApp/Physics/Clip.h"
#include "GameApp/MD5/MD5Model.h"
#include "GameApp/Animation/MD5Anim.h"
#include "GameApp/Animation/MD5IK.h"
#include <GameApp/MD5/MD5Game.h>
#include "GameApp/Physics/Physics_Base.h"
#include "GameApp/Physics/Physics_Static.h"
#include "GameApp/Physics/Physics_Actor.h"
#include <GameApp/Physics/Physics_AF.h>
#include <GameApp/Physics/Physics_Player.h>
#include <GameApp/Physics/Physics_Monster.h>
#include <GameApp/Physics/Physics_Parametric.h>
#include <GameApp/MD5/DeclAF.h>
#include <GameApp/MD5/MD5Af.h>
#include "GameApp/MD5/MD5Entity.h"
#include "GameApp/MD5/MD5AFEntity.h"
#include "GameApp/MD5/MD5Actor.h"
//#include "GameApp/MD5/Pvs.h"

#include "GameApp/Collision/RayTriIntersection.h"
#include "GameApp/BaseTypes/BaseTypes.h"

#include "GameApp/Resource/MeshBundle.h"