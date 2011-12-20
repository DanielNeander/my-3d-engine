/**************************************************************************************************
module	:	U2AnimFactory
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_ANIMFACTORY_H
#define U2_ANIMFACTORY_H

#include <U2Lib/src/U2RefObject.h>


class U2Animation;

class U2_3D U2AnimFactory : public U2RefObject
{
public:
	U2AnimFactory();
	virtual ~U2AnimFactory();

	static U2AnimFactory* Create();
	static void Terminate();

	static U2AnimFactory* Instance() { return ms_pSingleton; }

	virtual U2Animation* CreateMemAnim(const U2DynString& resName);

private:
	static U2AnimFactory* ms_pSingleton;

};


#endif