/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/
#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <crtdbg.h>
#include <iostream>
#include <conio.h>

//#include "miro.h"
#include "imagePRT.h"
//#include "opengl.h"

#pragma warning (disable: 4996)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ImagePRT::ImagePRT()
{
    m_pPixels = 0;
    m_iWidth = 1;
    m_iHeight = 1;
}

ImagePRT::~ImagePRT()
{
    if (m_pPixels)
        delete [] m_pPixels;
}

void ImagePRT::Resize(int width, int height)
{
    if (m_pPixels)
        delete [] m_pPixels;
    m_pPixels = 0;
    m_pPixels = new Pixel[width*height];
    memset(m_pPixels, 0, width*height*sizeof(Pixel));
    m_iWidth = width;
    m_iHeight = height;
}

// map floating point values to byte values for pixels
unsigned char inline Map(float r)
{
    float rMap = 255*r;
    //unsigned char c = rMap>255?255:(unsigned char)rMap;
	unsigned char c = (unsigned char)rMap;
    return c;
}

ImagePRT::Rgb ToRgb( const Pixel& pix )
{
	Pixel clamppix = pix;
	float gain = 1;
	float exposure = 1;

	clamppix.r = pow(exposure*clamppix.r, 1/gain);
	clamppix.g = pow(exposure*clamppix.g, 1/gain);
	clamppix.b = pow(exposure*clamppix.b, 1/gain);

	float maxchan = clamppix.r;
	if (clamppix.g > maxchan) {
		maxchan = clamppix.g;
	}
	if (clamppix.b > maxchan) {
		maxchan = clamppix.b;
	}
	if (maxchan > 1.0) {
		clamppix.r /= maxchan;
		clamppix.g /= maxchan;
		clamppix.b /= maxchan;
	}

	ImagePRT::Rgb rgb;
	rgb.r = Map(clamppix.r);		//simple tone-mapping - clamp to 255
	rgb.g = Map(clamppix.g);
	rgb.b = Map(clamppix.b);
	return rgb;
}

void ImagePRT::WritePPM(const char* pcFile)
{
	Rgb* rgbpixs = new Rgb[m_iWidth*m_iHeight];
	Rgb* scanline = rgbpixs;
	Pixel* pix = m_pPixels;

	for (int y=0; y < m_iHeight; y++) {
		for (int x=0; x < m_iWidth; x++) {
			*scanline++ = ToRgb( *pix++ );
		}
	}
    WritePPM(pcFile, (unsigned char*)rgbpixs, m_iWidth, m_iHeight);

	delete [] rgbpixs;
}

void ImagePRT::WritePPM(const char *pcFile, unsigned char *data, int width, int height)
{
    FILE *fp = fopen(pcFile, "wb");
    if (!fp)
        _cprintf("Couldn't open PPM file %s for writing\n", pcFile);
    else
    {
        fprintf(fp, "P6\n");
        fprintf(fp, "%d %d\n", width, height );
        fprintf(fp, "255\n" );

        // invert image
        int stride = width*3;
        for (int i=height-1; i>=0; i--)
            fwrite(&data[stride*i], stride, 1, fp);
        fclose(fp);
    }
}