#pragma once 



#ifdef U2_3D_EXPORTS
// DLL library project uses this
#define U2_3D __declspec(dllexport)
#else
#ifdef U2_3D_IMPORTS
// client of DLL uses this
#define U2_3D __declspec(dllimport)
#else
// static library project uses this
#define U2_3D
#endif
#endif

#ifdef WIN32
#pragma warning( disable : 4251 )
#endif