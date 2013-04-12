#pragma once 

#include "TestObjects.h"

class  Arrow : public WowActor 
{
public:
	Arrow(const std::string& filename);	

protected:
	virtual void Initialize();	

	class ProjectileAttributes* projectileAttrib_;
};

