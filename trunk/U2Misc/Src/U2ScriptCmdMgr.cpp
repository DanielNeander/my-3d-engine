#include "stdafx.h"
#include "U2ScriptCmdMgr.h"


U2ScriptCmdMgr* U2ScriptCmdMgr::ms_pInst = NULL;

U2ScriptCmdMgr::U2ScriptCmdMgr()
:m_pLuaMgr(NULL)
{

}

U2ScriptCmdMgr::~U2ScriptCmdMgr()
{
	Terminate();
}

void U2ScriptCmdMgr::Terminate()
{

}

U2ScriptCmdMgr* U2ScriptCmdMgr::Instance()
{
	return ms_pInst;
}




U2ScriptCmdMgr* U2ScriptCmdMgr::Create()
{
	if (!ms_pInst)
	{
		ms_pInst = U2_NEW U2ScriptCmdMgr();	
		ms_pInst->Initialize();
	}
	return ms_pInst;
}

void U2ScriptCmdMgr::RegLuaFunc(lua_CFunction func, const char* name)
{
	lua_pushstring(this->m_pLuaMgr->masterState, name);
	lua_pushlightuserdata(m_pLuaMgr->masterState, this);
	lua_pushcclosure(m_pLuaMgr->masterState, func, 1);
	lua_settable(m_pLuaMgr->masterState, LUA_GLOBALSINDEX);
	lua_settop(m_pLuaMgr->masterState, 0);
}

void U2ScriptCmdMgr::Initialize()
{
	m_pLuaMgr = new LUAMANAGER;

}

