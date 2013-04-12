/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/
#include "stdafx.h"
#include <stdio.h>
#include <malloc.h>
#include <climits>
#include <conio.h>

#include "pfmreader.h"
//#include "image.h"
//#include "miro.h"
#include "GameApp/Light/prefilter.h"


#pragma warning (disable: 4996)

#define MAXWIDTH 2000 

float* envpixels;

PfmReader::PfmReader()
{
}

PfmReader::~PfmReader() {}

int PfmReader::ReadHeader(FILE* source)
{
	char buf[32];

	if (source == NULL) {
		_cprintf("Error! File not found! Press enter to quit...");
		return false;
	}

	fscanf(source, "%s", buf); //throw away, already assuming pfm format

	bool valid = false;

	/*if (strncmp(buf, "Pf", 2) == 0) {
		m_channels = 1;
		valid = true;
	}
	if (strncmp(buf, "PF", 2) == 0) {
		m_channels = 3;
		valid = true;
	}*/
	fscanf(source, "%d %d", &m_width, &m_height );
	fscanf(source, "%f", &m_scale );

	envpixels = new float[m_width*m_height*3];

	// Save the rewind position.
    //m_rewindPos = m_stream.Tell();

	// set up the Info structure on the parent
	/*m_info.availableTypes = ImageReader::Info::kTypeImage;
	m_info.width = m_width;
	m_info.height = m_height;
	m_info.planes = m_channels;
	m_info.layers = -1;
	m_info.clutEntries = -1;
	m_info.mips = -1;*/

	return m_width;
}

/*void PfmReader::Rewind()
{
    m_stream.Seek(m_rewindPos, SEEK_SET);
}*/

/*void PfmReader::ReadGrayScale(Image *image )
{
    BinaryReader<ICETOOLS_HOST_ENDIAN != ICETOOLS_LITTLE_ENDIAN> br(&m_stream);
	float* scanline = (float*)ALLOCA( sizeof(float) * m_width );
    
	for (unsigned int y = 0; y < m_height; ++y) {
		float* p = scanline;

		for (unsigned int x = 0; x != m_width; ++x) {
            *p++ = br.ReadF32();
		}
		image->SetScanline( scanline, m_width, y, 0 );

    }
}*/

float* PfmReader::ReadRgb(FILE* source)
{
    //BinaryReader<ICETOOLS_HOST_ENDIAN != ICETOOLS_LITTLE_ENDIAN> br(&m_stream);
	//float* redscanline = (float*)_alloca( sizeof(float) * m_width );
	//float* grnscanline = (float*)_alloca( sizeof(float) * m_width );
	//float* bluscanline = (float*)_alloca( sizeof(float) * m_width );
	int temp = fgetc(source); //eat up newline

	float* pixel = envpixels; //intiate
    for (int i = 0; i < m_height*m_width*3; ++i) {
		fread((void*)pixel, sizeof(float), 1, source);
		envpixels[i] = *pixel;
		++pixel;
    }
	fclose(source);
	
	return envpixels;
}
