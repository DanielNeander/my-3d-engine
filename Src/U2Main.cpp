#include <U2_3D/Src/U23DLibPCH.h>
#include "U2Main.h"


extern int LuaCmd_NewN2Mesh(lua_State* pLua);

U2Main::InitializerArray* U2Main::ms_Initializers = 0;
U2Main::TerminatorArray*  U2Main::ms_Terminators = 0;
bool U2Main::ms_bInitAllocator = false;
U2Allocator* U2Main::ms_pAllocator = 0;

void U2Main::AddInitializer(Initializer oInitialize)
{	
	if(-1 == ms_Initializers->Find(oInitialize))
	{	
		ms_Initializers->AddElem(oInitialize);
	}
}

void U2Main::Initialize()
{
	
	static int counter = 0;
	if(counter++ == 0)
	{
		U2MemoryMgr::Init();

		FILELog::Init();
		FILELog::ReportingLevel() = FILELog::FromString(_T("DEBUG"));

		ms_Initializers = U2_NEW InitializerArray;
		ms_Terminators = U2_NEW TerminatorArray;		

		//ms_pAllocator = new U2DefaultAllocator();
		ms_bInitAllocator = true;

		U2ScriptCmdMgr::Create();

		::RegisterAllWithLua(ScriptMgr->GetLuaMgr()->masterState);

		ScriptMgr->RegLuaFunc(LuaCmd_NewN2Mesh, "NewN2Mesh");
		

 
		U2_NEW U2VariableMgr;
				
		U2FileMgr::Create();

		U2AnimFactory::Create();


		U2AlphaState::RegisterInitialize();
		U2MaterialState::RegisterInitialize();
		U2StencilState::RegisterInitialize();
		U2ZBufferState::RegisterInitialize();
		U2WireframeState::RegisterInitialize();

		U2AlphaState::RegisterTerminate();
		U2ZBufferState::RegisterTerminate();
		U2MaterialState::RegisterTerminate();
		U2StencilState::RegisterTerminate();
		U2WireframeState::RegisterTerminate();

		U2ShaderConstant::RegisterInitialize();
		U2ShaderConstant::RegisterTerminate();
		

		for(uint32 i=0; i < ms_Initializers->Size(); ++i)
		{
			(*ms_Initializers->GetElem(i))();
		}

		ms_Initializers->RemoveAll();
	}

}

void U2Main::AddTerminator(Terminator oTerminate)
{
	if(-1 == ms_Terminators->Find(oTerminate))	
	{
		ms_Terminators->AddElem(oTerminate);
	}
}



void U2Main::Terminate ()
{		
	//if(ms_bInitAllocator)	
	//	delete ms_pAllocator;		
	for(uint32 i=0; i < ms_Terminators->Size(); ++i)
	{
		(*ms_Terminators->GetElem(i))();
	}

	ms_Terminators->RemoveAll();

	U2_DELETE ms_Initializers;
	ms_Initializers = 0;
	U2_DELETE ms_Terminators;
	ms_Terminators = 0;

	U2FileMgr::Terminate();	
	U2VariableMgr::Terminate();

	U2AnimFactory::Terminate();

	//U2PoolShutdown(size_t);

	

	U2MemoryMgr::Terminate();

}
