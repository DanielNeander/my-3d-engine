#include <U2_3D/Src/U23DLibPCH.h>
#include "luabindfunctions.h"

using namespace luabind;




int LuaCmd_NewN2Mesh(lua_State* L)
{
	// takes 2 strings as arguments
	// returns 1 thunk on success or nil on failure
	if ((2 != lua_gettop(L)) || !lua_isstring(L, -1) || !lua_isstring(L, -2))
	{
		lua_settop(L, 0);
		lua_pushnil(L);
		return 1;
	}

	const char* meshClassName = lua_tostring(L, -2);
	const char* dataClassName = lua_tostring(L, -1);

	U2TriListData* pData = 0;
	U2N2Mesh* pMesh = 0;

	if (_stricmp(dataClassName, "U2TriListData"))
	{
		pData = U2_NEW U2TriListData;
	}


	if (_stricmp(meshClassName, "U2N2Mesh"))
	{
		U2ASSERT(pData);
		pMesh = U2_NEW U2N2Mesh(pData);
	}

	lua_settop(L, 0);	
	lua_pushboolean(L, true);

	return 1;	
}





