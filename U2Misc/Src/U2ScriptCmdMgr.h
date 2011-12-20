#pragma once 




#include <U2Lib/Src/Memory/U2MemObj.h>

class U2MISC U2ScriptCmdMgr : public U2MemObj
{
public:
	
	virtual ~U2ScriptCmdMgr();

	static U2ScriptCmdMgr* Instance();
	
	static U2ScriptCmdMgr* Create();

	void Initialize();
	void Terminate();
	
	// Lua
	void RegLuaFunc(lua_CFunction func, const char* name);

	inline LUAMANAGER* GetLuaMgr() const { return m_pLuaMgr; }


	
private:
	U2ScriptCmdMgr();
	
	class LUAMANAGER* m_pLuaMgr;

	static U2ScriptCmdMgr* ms_pInst;
	
	

};

#define  ScriptMgr U2ScriptCmdMgr::Instance()

