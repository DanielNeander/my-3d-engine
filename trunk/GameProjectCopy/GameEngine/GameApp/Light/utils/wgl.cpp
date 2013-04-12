
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "wgl.h"
#include "opengl.h"


void* WGL::GetProcAddress( const char* apiname )
{
	return wglGetProcAddress(apiname);
}

void* WGL::GetCurrentContext()
{
	return wglGetCurrentContext();
}

static bool MatchExtension( const char* supported, const char* extension )
{
	const size_t extlen = strlen(extension);
	// Begin Examination At Start Of String, Increment By 1 On False Match
	for (const char* p = supported; ; p++)
	{
		// Advance p Up To The Next Possible Match
		p = strstr(p, extension);

		if (p == NULL)
			return false;															// No Match

		// Make Sure That Match Is At The Start Of The String Or That
		// The Previous Char Is A Space, Or Else We Could Accidentally
		// Match "wglFunkywglExtension" With "wglExtension"

		// Also, Make Sure That The Following Character Is Space Or NULL
		// Or Else "wglExtensionTwo" Might Match "wglExtension"
		if ((p==supported || p[-1]==' ') && (p[extlen]=='\0' || p[extlen]==' '))
			return true;															// Match
	}
}

// WGLisExtensionSupported: This Is A Form Of The Extension For WGL
bool WGL::IsExtensionSupported(const char *extension)
{
	const char *supported = NULL;

	// Try To Use wglGetExtensionStringARB On Current DC, If Possible
	PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");

	if (wglGetExtString)
		supported = ((char*(__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());

	if (supported != 0) {
		bool match = MatchExtension( supported, extension );

		if (!match) {
			supported = (char*)glGetString(GL_EXTENSIONS);
			if (supported != 0) {
				match = MatchExtension( supported, extension );
			}
		}
		return match;
	}
	return false;
}