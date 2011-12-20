/*----------------------------------------------------------------------------
    LUA SCRIPT LIB.H
    Small library of Lua glue routines to support script management.

    contact: Matthew Harmon matt@matthewharmon.com
*/

/*--------------------------------------------------------------------------*/
/*                           Compilation Control                            */
/*--------------------------------------------------------------------------*/

// signal header has been included
#ifndef _LUASCRIPTLIB_H
#define _LUASCRIPTLIB_H


/*--------------------------------------------------------------------------*/
/*                              Data Structures                             */
/*--------------------------------------------------------------------------*/

typedef struct lua_State lua_State;

/*--------------------------------------------------------------------------*/
/*                            Public Prototypes                             */    
/*--------------------------------------------------------------------------*/


void LuaOpenScriptLib (lua_State* l);

/*--------------------------------------------------------------------------*/
/*                           Compilation Control                            */
/*--------------------------------------------------------------------------*/
//----------------------------- PopLuaNumber-----------------------------------
//
//  a function template to retrieve a number from the lua stack
//-----------------------------------------------------------------------------
template <class T>
inline T PopLuaNumber(lua_State* pL, const char* name)
{
	lua_settop(pL, 0);

	lua_getglobal(pL, name);

	//check that the variable is the correct type. If it is not throw an
	//exception
	if (!lua_isnumber(pL, 1))
	{
		std::string err("<PopLuaNumber> Cannot retrieve: ");

		throw std::runtime_error(err + name);
	}

	//grab the value, cast to the correct type and return
	T val = (T)lua_tonumber(pL, 1);

	//remove the value from the stack
	lua_pop(pL, 1);

	return val;
}

//--------------------------- PopLuaString ------------------------------------
//-----------------------------------------------------------------------------
inline std::string PopLuaString(lua_State* pL, const char* name)
{
	lua_settop(pL, 0);

	lua_getglobal(pL, name);

	//check that the variable is the correct type. If it is not throw an
	//exception
	if (!lua_isstring(pL, 1))
	{
		std::string err("<PopLuaString> Cannot retrieve: ");

		throw std::runtime_error(err + name);
	}

	//grab the value, cast to the correct type and return
	std::string s = lua_tostring(pL, 1);

	//remove the value from the stack
	lua_pop(pL, 1);

	return s;
}

//--------------------------- PopLuaBool ------------------------------------
//-----------------------------------------------------------------------------
inline bool PopLuaBool(lua_State* pL, const char* name)
{
	lua_settop(pL, 0);

	lua_getglobal(pL, name);

	//check that the variable is the correct type. If it is not throw an
	//exception
	if (!lua_isstring(pL, 1))
	{
		std::string err("<PopLuaBool> Cannot retrieve: ");

		throw std::runtime_error(err + name);
	}

	//grab the value, cast to the correct type and return
	bool b = (lua_toboolean(pL, 1) ? true : false);

	//remove the value from the stack
	lua_pop(pL, 1);

	return b;
}

//------------------------- LuaPopStringFieldFromTable ------------------------
//-----------------------------------------------------------------------------
inline std::string LuaPopStringFieldFromTable(lua_State* L, const char* key)
{

	//push the key onto the stack
	lua_pushstring(L, key);

	//table is now at -2 (key is at -1). lua_gettable now pops the key off
	//the stack and then puts the data found at the key location on the stack
	lua_gettable(L, -2);

	//check that the variable is the correct type. If it is not throw an
	//exception
	if (!lua_isstring(L, -1))
	{
		std::string err("<LuaPopStringFieldFromTable> Cannot retrieve: ");

		throw std::runtime_error(err + key);
	}

	//grab the data
	std::string s = lua_tostring(L, -1);

	lua_pop(L, 1);

	return s;
}

//----------------------------- LuaPopNumberFieldFromTable --------------------
//-----------------------------------------------------------------------------
template <class T>
inline T LuaPopNumberFieldFromTable(lua_State* L, const char* key)
{
	//push the key onto the stack
	lua_pushstring(L, key);

	//table is now at -2 (key is at -1). lua_gettable now pops the key off
	//the stack and then puts the data found at the key location on the stack
	lua_gettable(L, -2);

	//check that the variable is the correct type. If it is not throw an
	//exception
	if (!lua_isnumber(L, -1))
	{
		std::string err("<LuaPopNumberFieldFromTable> Cannot retrieve: ");

		throw std::runtime_error(err + key);
	}

	//grab the data
	T val = (T)lua_tonumber(L, -1);

	lua_pop(L, 1);

	return val;
}


#endif // _LUASCRIPTLIB_H
