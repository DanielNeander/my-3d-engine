//--------------------------------------------------------------------------------
// GeometryLoaderDX11
//
// Copyright (C) 2003-2009 Jason Zink
//--------------------------------------------------------------------------------
#ifndef GeometryLoaderDX11_h
#define GeometryLoaderDX11_h
//--------------------------------------------------------------------------------
#include "GeometryDX11.h"
#include <string>
//--------------------------------------------------------------------------------
class GeometryLoaderDX11
{
public:
	static GeometryDX11* loadMS3DFile2( std::string filename );		

private:
	GeometryLoaderDX11();
	
};
#endif // GeometryLoaderDX11_h


