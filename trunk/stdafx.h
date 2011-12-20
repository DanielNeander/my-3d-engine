// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include <direct.h>

typedef unsigned int uint32;

#include "U2Filename.h"
#include "U2SearchPath.h"
#include "U2FilePath.h"
#include "U2TString.h"


#include <U2Lib/Src/U2Log.h>


// TODO: reference additional headers your program requires here
