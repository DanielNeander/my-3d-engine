#pragma once
#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <limits>

typedef unsigned int		uint32;
typedef unsigned int		dword;
typedef unsigned short		uint16;
typedef unsigned char		uint8;

typedef unsigned char			byte;
typedef unsigned short			word;		// 16 bits
typedef unsigned int			dword;		// 32 bits
typedef unsigned int			uint;
typedef unsigned long			ulong;

typedef int						qhandle_t;


//typedef			 int		int8;
//typedef          short		int16;
//typedef          int		int32;


typedef	float				Real;

#define NO_INLINE	__forceinline

#define REAL_MAX	FLT_MAX		

#define BIT(x) (1 << (x))                       ///< Returns value with bit x set (2^x)



#define	MAX_STRING_CHARS		1024		// max length of a string

// maximum world size
#define MAX_WORLD_COORD			( 128 * 1024 )
#define MIN_WORLD_COORD			( -128 * 1024 )
#define MAX_WORLD_SIZE			( MAX_WORLD_COORD - MIN_WORLD_COORD )

#ifdef __GNUC__
#define id_attribute(x) __attribute__(x)
#else
#define id_attribute(x)  
#endif

#define	BUILD_STRING					"win-x86"
#define BUILD_OS_ID						0
#define	CPUSTRING						"x86"
#define CPU_EASYARGS					1

#define ALIGN16( x )					__declspec(align(16)) x
#define PACKED

#define _alloca16( x )					((void *)((((int)_alloca( (x)+15 )) + 15) & ~15))

#define PATHSEPERATOR_STR				"\\"
#define PATHSEPERATOR_CHAR				'\\'

#define ID_INLINE						__forceinline
#define ID_STATIC_TEMPLATE				static

#define assertmem( x, y )				assert( _CrtIsValidPointer( x, y, true ) )


// SpeedTree
typedef bool            st_bool;
typedef char            st_int8;
typedef char            st_char;
typedef short           st_int16;
typedef int             st_int32;
typedef unsigned char   st_uint8;
typedef unsigned char   st_byte;
typedef unsigned char   st_uchar;
typedef unsigned short  st_uint16;
typedef unsigned int    st_uint32;
typedef float           st_float32;
typedef double          st_float64;
template <class T> inline T st_min(const T& a, const T& b)
{
	return (a < b) ? a : b;
}

template <class T> inline T st_max(const T& a, const T& b)
{
	return (a > b) ? a : b;
}

// Unreal 3

typedef unsigned __int64	QWORD;		// 64-bit unsigned.

// Signed base types.
typedef	signed __int8		SBYTE;		// 8-bit  signed.
typedef signed __int16		SWORD;		// 16-bit signed.
typedef signed __int32 		INT;		// 32-bit signed.
typedef long				LONG;		// defined in windows.h

typedef signed __int64		SQWORD;		// 64-bit signed.

// Character types.
typedef char				ANSICHAR;	// An ANSI character. normally a signed type.
typedef wchar_t				UNICHAR;	// A unicode character. normally a signed type.
typedef wchar_t				WCHAR;		// defined in windows.h



typedef unsigned int		UBOOL;		// Boolean 0 (false) or 1 (true).
typedef float				FLOAT;		// 32-bit IEEE floating point.
typedef double				DOUBLE;		// 64-bit IEEE double.

#ifdef _WIN64
typedef SQWORD				PTRINT;		// Integer large enough to hold a pointer.
#else
typedef int					PTRINT;		// Integer large enough to hold a pointer.
#endif

typedef unsigned long       BITFIELD;	// For bitfields.

#define ARRAY_COUNT( array )  ( sizeof(array) / sizeof((array)[0]) )

// Global constants.
//enum {MAXBYTE		= 0xff       };
//enum {MAXWORD		= 0xffffU    };
//enum {MAXDWORD		= 0xffffffffU};
enum {MAXSBYTE		= 0x7f       };
enum {MAXSWORD		= 0x7fff     };
//enum {MAXINT		= 0x7fffffff };
enum {INDEX_NONE	= -1         };
enum {UNICODE_BOM   = 0xfeff     };
enum ENoInit {E_NoInit = 0};

#define SMALL_NUMBER		(1.e-8)
#define KINDA_SMALL_NUMBER	(1.e-4)
#define BIG_NUMBER			(3.4e+38f)
#define EULERS_NUMBER       (2.71828182845904523536)

enum EEventParm            {EC_EventParm};


#endif