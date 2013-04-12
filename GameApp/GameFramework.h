#pragma once 

#include "Framework/Framework.h"



class GameFramework : public Framework {

public:

	virtual Error Initialize( pcstr pszGDF );

	virtual void Shutdown( void );
	
	virtual Error Execute( void ) throw ( ... );

};

