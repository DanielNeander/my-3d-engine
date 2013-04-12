#ifndef GLUTUTIL_H
#define GLUTUTIL_H

#include "vector.h"

namespace GLUT
{
	// print text using GLUT
	void glutPrintf ( const char * format, ... );
	// setup for text output
	void TextMode(int w, int h, const Vector3* textcolor = 0);

	int printOglError(char *file, int line);

};

#endif

