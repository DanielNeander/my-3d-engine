#pragma once 

#ifdef U2MISC_EXPORTS
// DLL library project uses this
#define U2MISC __declspec(dllexport)
#else
#ifdef U2MISC_IMPORTS
// client of DLL uses this
#define U2MISC __declspec(dllimport)
#else
// static library project uses this
#define U2MISC
#endif
#endif

#ifdef WIN32
#pragma warning( disable : 4251 )
#endif