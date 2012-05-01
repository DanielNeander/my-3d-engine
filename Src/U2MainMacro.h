#ifndef U2_MACRO_H
#define U2_MACRO_H

#define DECLARE_INITIALIZE \
public: \
	static bool RegisterInitialize (); \
	static void Initialize (); \
private: \
	static bool ms_bInitializeRegistered

//----------------------------------------------------------------------------
#define IMPLEMENT_INITIALIZE(classname) \
	bool classname::ms_bInitializeRegistered = false; \
	bool classname::RegisterInitialize () \
{ \
	if (!ms_bInitializeRegistered) \
	{ \
	U2Main::AddInitializer(classname::Initialize); \
	ms_bInitializeRegistered = true; \
	} \
	return ms_bInitializeRegistered; \
}

//----------------------------------------------------------------------------
#define DECLARE_TERMINATE \
public: \
	static bool RegisterTerminate (); \
	static void Terminate (); \
private: \
	static bool ms_bTerminateRegistered
//----------------------------------------------------------------------------
#define IMPLEMENT_TERMINATE(classname) \
	bool classname::ms_bTerminateRegistered = false; \
	bool classname::RegisterTerminate () \
{ \
	if (!ms_bTerminateRegistered) \
	{ \
	U2Main::AddTerminator(classname::Terminate); \
	ms_bTerminateRegistered = true; \
	} \
	return ms_bTerminateRegistered; \
}


#endif