#ifndef U2_LIBTYPE_H
#define U2_LIBTYPE_H

#ifdef U2LIB_EXPORT
// DLL library project uses this
#define U2LIB __declspec(dllexport)
#else
#ifdef U2LIB_IMPORT
// client of DLL uses this
#define U2LIB __declspec(dllimport)
#else
// static library project uses this
#define U2LIB
#endif
#endif

#ifdef WIN32
#pragma warning( disable : 4251 )
#endif

#endif

