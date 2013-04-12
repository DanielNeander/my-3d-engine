#pragma once
/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

/*
====================================================================

IMAGE

idImage have a one to one correspondance with OpenGL textures.

No texture is ever used that does not have a corresponding idImage.

no code outside this unit should call any of these OpenGL functions:

qglGenTextures
qglDeleteTextures
qglBindTexture

qglTexParameter

qglTexImage
qglTexSubImage

qglCopyTexImage
qglCopyTexSubImage

qglEnable( GL_TEXTURE_* )
qglDisable( GL_TEXTURE_* )

====================================================================
*/

typedef enum {
	IS_UNLOADED,	// no gl texture number
	IS_PARTIAL,		// has a texture number and the low mip levels loaded
	IS_LOADED		// has a texture number and the full mip hierarchy
} imageState_t;

static const int	MAX_TEXTURE_LEVELS = 14;

// surface description flags
const unsigned long DDSF_CAPS           = 0x00000001l;
const unsigned long DDSF_HEIGHT         = 0x00000002l;
const unsigned long DDSF_WIDTH          = 0x00000004l;
const unsigned long DDSF_PITCH          = 0x00000008l;
const unsigned long DDSF_PIXELFORMAT    = 0x00001000l;
const unsigned long DDSF_MIPMAPCOUNT    = 0x00020000l;
const unsigned long DDSF_LINEARSIZE     = 0x00080000l;
const unsigned long DDSF_DEPTH          = 0x00800000l;

// pixel format flags
const unsigned long DDSF_ALPHAPIXELS    = 0x00000001l;
const unsigned long DDSF_FOURCC         = 0x00000004l;
const unsigned long DDSF_RGB            = 0x00000040l;
const unsigned long DDSF_RGBA           = 0x00000041l;

// our extended flags
const unsigned long DDSF_ID_INDEXCOLOR	= 0x10000000l;
const unsigned long DDSF_ID_MONOCHROME	= 0x20000000l;

// dwCaps1 flags
const unsigned long DDSF_COMPLEX         = 0x00000008l;
const unsigned long DDSF_TEXTURE         = 0x00001000l;
const unsigned long DDSF_MIPMAP          = 0x00400000l;

#define DDS_MAKEFOURCC(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

typedef struct {
	unsigned long dwSize;
	unsigned long dwFlags;
	unsigned long dwFourCC;
	unsigned long dwRGBBitCount;
	unsigned long dwRBitMask;
	unsigned long dwGBitMask;
	unsigned long dwBBitMask;
	unsigned long dwABitMask;
} ddsFilePixelFormat_t;

typedef struct
{
	unsigned long dwSize;
	unsigned long dwFlags;
	unsigned long dwHeight;
	unsigned long dwWidth;
	unsigned long dwPitchOrLinearSize;
	unsigned long dwDepth;
	unsigned long dwMipMapCount;
	unsigned long dwReserved1[11];
	ddsFilePixelFormat_t ddspf;
	unsigned long dwCaps1;
	unsigned long dwCaps2;
	unsigned long dwReserved2[3];
} ddsFileHeader_t;


// increasing numeric values imply more information is stored
typedef enum {
	TD_SPECULAR,			// may be compressed, and always zeros the alpha channel
	TD_DIFFUSE,				// may be compressed
	TD_DEFAULT,				// will use compressed formats when possible
	TD_BUMP,				// may be compressed with 8 bit lookup
	TD_HIGH_QUALITY			// either 32 bit or a component format, no loss at all
} textureDepth_t;

typedef enum {
	TT_DISABLED,
	TT_2D,
	TT_3D,
	TT_CUBIC,
	TT_RECT
} textureType_t;

typedef enum {
	CF_2D,			// not a cube map
	CF_NATIVE,		// _px, _nx, _py, etc, directly sent to GL
	CF_CAMERA		// _forward, _back, etc, rotated and flipped as needed before sending to GL
} cubeFiles_t;

#define	MAX_IMAGE_NAME	256

const char *R_ParsePastImageProgram( idLexer &src );
byte *R_Dropsample( const byte *in, int inwidth, int inheight, int outwidth, int outheight );
byte *R_ResampleTexture( const byte *in, int inwidth, int inheight, int outwidth, int outheight );
TextureID R_LoadImage( const char *cname, byte **pic, int *width, int *height, ID_TIME_T *timestamp, bool makePowerOf2 );
TextureID ImageFromFile( const char *_name, textureFilter_t filter, bool allowDownSize, textureRepeat_t repeat, textureDepth_t depth, cubeFiles_t cubeMap );
void R_LoadImageProgram( const char *name, byte **pic, int *width, int *height, ID_TIME_T *timestamps, textureDepth_t *depth );

// these operate in-place on the provided pixels
void R_HorizontalFlip( byte *data, int width, int height );
void R_VerticalFlip( byte *data, int width, int height );
void R_RotatePic( byte *data, int width );

extern TextureID gTex;