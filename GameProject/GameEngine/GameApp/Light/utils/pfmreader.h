#ifndef __PFMREADER_H__
#define __PFMREADER_H__

//#include "image.h"
#include <stdio.h>

class PfmReader {
public:
	PfmReader();
	virtual ~PfmReader();

	int ReadHeader(FILE* source);
	void Rewind();
	float* ReadRgb(FILE* source);
	int getWidth() {return m_width;}
	int getHeight() {return m_height;}

private:
	int m_width; //image width
	int m_height; //image height
	float m_scale; //scale/endianness
};

#endif