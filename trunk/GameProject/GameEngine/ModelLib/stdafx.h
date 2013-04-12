// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers


#include <Windows.h>
#include <tchar.h>


// wx
#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/string.h>


#include <map>
#include <vector>
#include <string>

#define WotLK 

// TODO: reference additional headers your program requires here
#include <EngineCore/Platform.h>
#include <EngineCore/Types.h>
#include "mpq_libmpq.h"


#include <EngineCore/Math/Vector.h>
#include <EngineCore/Math/Matrix.h>
#include <EngineCore/Math/Quaternion.h>

#include <EngineCore/Math/MathTools.h>

#include <EngineCore/Direct3D11/Direct3D11Renderer.h>

