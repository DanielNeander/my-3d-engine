#ifndef WGL_H
#define WGL_H

/*
-- This provides a wrapper for Window's 'wgl' layer.  This file exist mostly to avoid having to include
-- the ugly Windows.h (even with LEAN_AND_MEAN).  It pollutes the global namespace with things like 'min' etc.
*/

namespace WGL
{
	void* GetProcAddress( const char* apiname );
	void* GetCurrentContext();
	bool IsExtensionSupported(const char *extension);

};

#endif
