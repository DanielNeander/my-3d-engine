// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <assert.h>
#include <tchar.h>
//#include <commctrl.h> // for InitCommonControls() 

#define D3D_DEBUG_INFO 


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


#include <U2Application/U2WinApp.h>
#include <U2_3D/Src/main/U2Scene.h>



// TODO: reference additional headers your program requires here
#include "AppMisc.h"
#include "Vertex.h"
#include "PolygonUtil.h"