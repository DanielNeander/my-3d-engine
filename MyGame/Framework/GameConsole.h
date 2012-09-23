#ifndef GAMECONSOLE_H
#define GAMECONSOLE_H

class GameConsole {

	void	Init( void );
	void	Shutdown( void );

	void	InitGraphics();

	bool	ProcessEvent( const struct sysEvent_s *event, bool forceeAccept);
	bool	Active( void );
	void	ClearNotifyLines( void );
	void	Close( void );
	void	Draw( bool forceFullScreen );
	void	Print( const char *text );
};

#endif