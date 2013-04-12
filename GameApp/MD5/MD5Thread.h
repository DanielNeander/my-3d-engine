#pragma once 


class idThread : public idClass {
private:
	static idThread				*currentThread;

	idThread					*waitingForThread;
	int							waitingFor;
	int							waitingUntil;
	//idInterpreter				interpreter;

	idDict						spawnArgs;

	int 						threadNum;
	idStr 						threadName;

	int							lastExecuteTime;
	int							creationTime;

	bool						manualControl;

	static int					threadIndex;
	static idList<idThread *>	threadList;

	static trace_t				trace;

	void						Init( void );
	void						Pause( void );

public:							
	CLASS_PROTOTYPE( idThread );

	idThread();

	virtual						~idThread();

	// tells the thread manager not to delete this thread when it ends
	void						ManualDelete( void );

	void						WaitMS( int time );
	void						WaitSec( float time );
	void						WaitFrame( void );

	static idThread				*GetThread( int num );
	static void					ListThreads_f( const idCmdArgs &args );

	static idList<idThread*>&	GetThreads ( void );

	bool						IsDoneProcessing ( void );
	bool						IsDying			 ( void );	

	void						End( void );
	static void					KillThread( const char *name );
	static void					KillThread( int num );

	void						ManualControl( void ) { manualControl = true; CancelEvents( &EV_Thread_Execute ); };

	void						Error( const char *fmt, ... ) const id_attribute((format(printf,2,3)));
	void						Warning( const char *fmt, ... ) const id_attribute((format(printf,2,3)));

	static idThread				*CurrentThread( void );
	static int					CurrentThreadNum( void );

	static void					ReturnString( const char *text );
	static void					ReturnFloat( float value );
	static void					ReturnInt( int value );
	static void					ReturnVector( idVec3 const &vec );
	static void					ReturnEntity( idEntity *ent );
};
