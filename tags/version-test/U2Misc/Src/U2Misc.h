#pragma once 

#pragma warning(disable : 4231)
#pragma warning(disable : 4275)


#include "API.h"
#include "U2FileTime_Win32.h"
#include "U2WinFile.h"
#include "U2FileMgr.h"
#include <tinyxml/tinyxml.h>
#include "U2XmlStream.h"
#include "DebugConsole.h"

// Lua requires it's headers to be explicitly extern'd
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include    "LuaScriptLib.h"
#include    "LuaScript.h"
#include    "LuaManager.h"


#include <luabind/luabind.hpp>

#include "U2ScriptCmdMgr.h"