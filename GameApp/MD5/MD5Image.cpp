#include "stdafx.h"
#include "MD5Image.h"
#include "GameApp/GameApp.h"
#include "EngineCore/Imaging/Image.h"
#include "CxImage/ximage.h"


TextureID gTex = -1;

TextureID R_LoadImage( const char *cname, byte **pic, int *width, int *height, ID_TIME_T *timestamp, bool makePowerOf2 ) {
	idStr name = cname;

	/*{
	idStr temp = name;
	name = "Models/dds/";
	name += temp;
	}*/
	name.DefaultPath("Doom3/");

	if ( pic ) {
		*pic = NULL;
	}
	if ( timestamp ) {
		*timestamp = 0xFFFFFFFF;
	}
	if ( width ) {
		*width = 0;
	}
	if ( height ) {
		*height = 0;
	}

	name.DefaultFileExtension( ".tga" );

	if (name.Length()<5) {
		return -1;
	}

	name.ToLower();
	idStr ext;
	name.ExtractFileExtension( ext );

	TextureID texID = -1;
	Image img;

	if ( ext == "tga" ) {
		//LoadTGA( name.c_str(), pic, width, height, timestamp );            // try tga first
		img.loadTGA(name.c_str());
		byte* ptr = img.getPixels();		
		pic = &ptr;		
		if ( ( pic && *pic == 0 ) || ( timestamp && *timestamp == -1 ) ) {
			name.StripFileExtension();
			name.DefaultFileExtension( ".jpg" );
			//LoadJPG( name.c_str(), pic, width, height, timestamp );
			Image img;
			img.loadJPEG(name.c_str());
			byte* ptr = img.getPixels();		
			pic = &ptr;
		}
	} else if ( ext == "pcx" ) {
		//LoadPCX32( name.c_str(), pic, width, height, timestamp );
		Image img;
		img.loadPCX(name.c_str());
		byte* ptr = img.getPixels();		
		pic = &ptr;
	} else if ( ext == "bmp" ) {
		//LoadBMP( name.c_str(), pic, width, height, timestamp );
		Image img;
		img.loadBMP(name.c_str());
		byte* ptr = img.getPixels();		
		pic = &ptr;
	} else if ( ext == "jpg" ) {
		//LoadJPG( name.c_str(), pic, width, height, timestamp );
		Image img;
		img.loadJPEG(name.c_str());
		byte* ptr = img.getPixels();		
		pic = &ptr;
		
	}

	int w = img.getWidth();
	int h = img.getHeight();
	width = &w;
	height = &h;

	if ( ( width && *width < 1 ) || ( height && *height < 1 ) ) {
		if ( pic && *pic ) {
			R_StaticFree( *pic );
			*pic = 0;
		}
	}

	//
	// convert to exact power of 2 sizes
	//
	if ( pic && *pic && makePowerOf2 ) {
		int		w, h;
		int		scaled_width, scaled_height;
		byte	*resampledBuffer;

		w = *width;
		h = *height;

		for (scaled_width = 1 ; scaled_width < w ; scaled_width<<=1)
			;
		for (scaled_height = 1 ; scaled_height < h ; scaled_height<<=1)
			;

		if ( scaled_width != w || scaled_height != h ) {
			if ( /*globalImages->image_roundDown.GetBool() &&*/ scaled_width > w ) {
				scaled_width >>= 1;
			}
			if ( /*globalImages->image_roundDown.GetBool() &&*/ scaled_height > h ) {
				scaled_height >>= 1;
			}

			resampledBuffer = R_ResampleTexture( *pic, w, h, scaled_width, scaled_height );
			R_StaticFree( *pic );
			*pic = resampledBuffer;
			*width = scaled_width;
			*height = scaled_height;

			{		
				
				Image img2;
				img2.loadFromMemory(*pic, FORMAT_RGBA8, *width, *height, 1, 1, false);
				texID = GetRenderer()->addTexture(img2);
				//R_StaticFree(temp);
			}
		}
		else 
		{		 
			
			Image img2;			
			img2.loadFromMemory(*pic, img.getFormat(), *width, *height, 1, 1, false);
			texID = GetRenderer()->addTexture(img2);
			//R_StaticFree(temp);

		}		
	}	

	return texID;
}


/*
================
R_ResampleTexture

Used to resample images in a more general than quartering fashion.

This will only have filter coverage if the resampled size
is greater than half the original size.

If a larger shrinking is needed, use the mipmap function 
after resampling to the next lower power of two.
================
*/
#define	MAX_DIMENSION	4096
byte *R_ResampleTexture( const byte *in, int inwidth, int inheight,  
	int outwidth, int outheight ) {
		int		i, j;
		const byte	*inrow, *inrow2;
		unsigned int	frac, fracstep;
		unsigned int	p1[MAX_DIMENSION], p2[MAX_DIMENSION];
		const byte		*pix1, *pix2, *pix3, *pix4;
		byte		*out, *out_p;

		if ( outwidth > MAX_DIMENSION ) {
			outwidth = MAX_DIMENSION;
		}
		if ( outheight > MAX_DIMENSION ) {
			outheight = MAX_DIMENSION;
		}

		out = (byte *)R_StaticAlloc( outwidth * outheight * 4 );
		out_p = out;

		fracstep = inwidth*0x10000/outwidth;

		frac = fracstep>>2;
		for ( i=0 ; i<outwidth ; i++ ) {
			p1[i] = 4*(frac>>16);
			frac += fracstep;
		}
		frac = 3*(fracstep>>2);
		for ( i=0 ; i<outwidth ; i++ ) {
			p2[i] = 4*(frac>>16);
			frac += fracstep;
		}

		for (i=0 ; i<outheight ; i++, out_p += outwidth*4 ) {
			inrow = in + 4 * inwidth * (int)( ( i + 0.25f ) * inheight / outheight );
			inrow2 = in + 4 * inwidth * (int)( ( i + 0.75f ) * inheight / outheight );
			frac = fracstep >> 1;
			for (j=0 ; j<outwidth ; j++) {
				pix1 = inrow + p1[j];
				pix2 = inrow + p2[j];
				pix3 = inrow2 + p1[j];
				pix4 = inrow2 + p2[j];
				out_p[j*4+0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0])>>2;
				out_p[j*4+1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1])>>2;
				out_p[j*4+2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2])>>2;
				out_p[j*4+3] = (pix1[3] + pix2[3] + pix3[3] + pix4[3])>>2;
			}
		}

		return out;
}


/*
================
R_Dropsample

Used to resample images in a more general than quartering fashion.
Normal maps and such should not be bilerped.
================
*/
byte *R_Dropsample( const byte *in, int inwidth, int inheight,  
	int outwidth, int outheight ) {
		int		i, j, k;
		const byte	*inrow;
		const byte	*pix1;
		byte		*out, *out_p;

		out = (byte *)R_StaticAlloc( outwidth * outheight * 4 );
		out_p = out;

		for (i=0 ; i<outheight ; i++, out_p += outwidth*4 ) {
			inrow = in + 4*inwidth*(int)((i+0.25)*inheight/outheight);
			for (j=0 ; j<outwidth ; j++) {
				k = j * inwidth / outwidth;
				pix1 = inrow + k * 4;
				out_p[j*4+0] = pix1[0];
				out_p[j*4+1] = pix1[1];
				out_p[j*4+2] = pix1[2];
				out_p[j*4+3] = pix1[3];
			}
		}

		return out;
}

/*
=================
R_ImageScale
=================
*/
static void R_ImageScale( byte *data, int width, int height, float scale[4] ) {
	int		i, j;
	int		c;

	c = width * height * 4;

	for ( i = 0 ; i < c ; i++ ) {
		j = (byte)(data[i] * scale[i&3]);
		if ( j < 0 ) {
			j = 0;
		} else if ( j > 255 ) {
			j = 255;
		}
		data[i] = j;
	}
}

/*
=================
R_InvertAlpha
=================
*/
static void R_InvertAlpha( byte *data, int width, int height ) {
	int		i;
	int		c;

	c = width * height* 4;

	for ( i = 0 ; i < c ; i+=4 ) {
		data[i+3] = 255 - data[i+3];
	}
}

/*
=================
R_InvertColor
=================
*/
static void R_InvertColor( byte *data, int width, int height ) {
	int		i;
	int		c;

	c = width * height* 4;

	for ( i = 0 ; i < c ; i+=4 ) {
		data[i+0] = 255 - data[i+0];
		data[i+1] = 255 - data[i+1];
		data[i+2] = 255 - data[i+2];
	}
}


/*
===================
R_AddNormalMaps

===================
*/
static void R_AddNormalMaps( byte *data1, int width1, int height1, byte *data2, int width2, int height2 ) {
	int		i, j;
	byte	*newMap;

	// resample pic2 to the same size as pic1
	if ( width2 != width1 || height2 != height1 ) {
		newMap = R_Dropsample( data2, width2, height2, width1, height1 );
		data2 = newMap;
	} else {
		newMap = NULL;
	}

	// add the normal change from the second and renormalize
	for ( i = 0 ; i < height1 ; i++ ) {
		for ( j = 0 ; j < width1 ; j++ ) {
			byte	*d1, *d2;
			noVec3	n;
			float   len;

			d1 = data1 + ( i * width1 + j ) * 4;
			d2 = data2 + ( i * width1 + j ) * 4;

			n[0] = ( d1[0] - 128 ) / 127.0;
			n[1] = ( d1[1] - 128 ) / 127.0;
			n[2] = ( d1[2] - 128 ) / 127.0;

			// There are some normal maps that blend to 0,0,0 at the edges
			// this screws up compression, so we try to correct that here by instead fading it to 0,0,1
			len = n.LengthFast();
			if ( len < 1.0f ) {
				n[2] = noMath::Sqrt(1.0 - (n[0]*n[0]) - (n[1]*n[1]));
			}

			n[0] += ( d2[0] - 128 ) / 127.0;
			n[1] += ( d2[1] - 128 ) / 127.0;
			n.Normalize();

			d1[0] = (byte)(n[0] * 127 + 128);
			d1[1] = (byte)(n[1] * 127 + 128);
			d1[2] = (byte)(n[2] * 127 + 128);
			d1[3] = 255;
		}
	}

	if ( newMap ) {
		R_StaticFree( newMap );
	}
}


/*
=================
R_HeightmapToNormalMap

it is not possible to convert a heightmap into a normal map
properly without knowing the texture coordinate stretching.
We can assume constant and equal ST vectors for walls, but not for characters.
=================
*/
static void R_HeightmapToNormalMap( byte *data, int width, int height, float scale ) {
	int		i, j;
	byte	*depth;

	scale = scale / 256;

	// copy and convert to grey scale
	j = width * height;
	depth = (byte *)R_StaticAlloc( j );
	for ( i = 0 ; i < j ; i++ ) {
		depth[i] = ( data[i*4] + data[i*4+1] + data[i*4+2] ) / 3;
	}

	noVec3	dir, dir2;
	for ( i = 0 ; i < height ; i++ ) {
		for ( j = 0 ; j < width ; j++ ) {
			int		d1, d2, d3, d4;
			int		a1, a2, a3, a4;

			// FIXME: look at five points?

			// look at three points to estimate the gradient
			a1 = d1 = depth[ ( i * width + j ) ];
			a2 = d2 = depth[ ( i * width + ( ( j + 1 ) & ( width - 1 ) ) ) ];
			a3 = d3 = depth[ ( ( ( i + 1 ) & ( height - 1 ) ) * width + j ) ];
			a4 = d4 = depth[ ( ( ( i + 1 ) & ( height - 1 ) ) * width + ( ( j + 1 ) & ( width - 1 ) ) ) ];

			d2 -= d1;
			d3 -= d1;

			dir[0] = -d2 * scale;
			dir[1] = -d3 * scale;
			dir[2] = 1;
			dir.NormalizeFast();

			a1 -= a3;
			a4 -= a3;

			dir2[0] = -a4 * scale;
			dir2[1] = a1 * scale;
			dir2[2] = 1;
			dir2.NormalizeFast();

			dir += dir2;
			dir.NormalizeFast();

			a1 = ( i * width + j ) * 4;
			data[ a1 + 0 ] = (byte)(dir[0] * 127 + 128);
			data[ a1 + 1 ] = (byte)(dir[1] * 127 + 128);
			data[ a1 + 2 ] = (byte)(dir[2] * 127 + 128);
			data[ a1 + 3 ] = 255;
		}
	}


	R_StaticFree( depth );
}

/*
================
R_SmoothNormalMap
================
*/
static void R_SmoothNormalMap( byte *data, int width, int height ) {
	byte	*orig;
	int		i, j, k, l;
	noVec3	normal;
	byte	*out;
	static float	factors[3][3] = {
		{ 1, 1, 1 },
		{ 1, 1, 1 },
		{ 1, 1, 1 }
	};

	orig = (byte *)R_StaticAlloc( width * height * 4 );
	memcpy( orig, data, width * height * 4 );

	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 0 ; j < height ; j++ ) {
			normal = vec3_origin;
			for ( k = -1 ; k < 2 ; k++ ) {
				for ( l = -1 ; l < 2 ; l++ ) {
					byte	*in;

					in = orig + ( ((j+l)&(height-1))*width + ((i+k)&(width-1)) ) * 4;

					// ignore 000 and -1 -1 -1
					if ( in[0] == 0 && in[1] == 0 && in[2] == 0 ) {
						continue;
					}
					if ( in[0] == 128 && in[1] == 128 && in[2] == 128 ) {
						continue;
					}

					normal[0] += factors[k+1][l+1] * ( in[0] - 128 );
					normal[1] += factors[k+1][l+1] * ( in[1] - 128 );
					normal[2] += factors[k+1][l+1] * ( in[2] - 128 );
				}
			}
			normal.Normalize();
			out = data + ( j * width + i ) * 4;
			out[0] = (byte)(128 + 127 * normal[0]);
			out[1] = (byte)(128 + 127 * normal[1]);
			out[2] = (byte)(128 + 127 * normal[2]);
		}
	}

	R_StaticFree( orig );
}


/*
===================
R_ImageAdd

===================
*/
static void R_ImageAdd( byte *data1, int width1, int height1, byte *data2, int width2, int height2 ) {
	int		i, j;
	int		c;
	byte	*newMap;

	// resample pic2 to the same size as pic1
	if ( width2 != width1 || height2 != height1 ) {
		newMap = R_Dropsample( data2, width2, height2, width1, height1 );
		data2 = newMap;
	} else {
		newMap = NULL;
	}


	c = width1 * height1 * 4;

	for ( i = 0 ; i < c ; i++ ) {
		j = data1[i] + data2[i];
		if ( j > 255 ) {
			j = 255;
		}
		data1[i] = j;
	}

	if ( newMap ) {
		R_StaticFree( newMap );
	}
}

// we build a canonical token form of the image program here
static char parseBuffer[MAX_IMAGE_NAME];
/*
===================
AppendToken
===================
*/
static void AppendToken( idToken &token ) {
	// add a leading space if not at the beginning
	if ( parseBuffer[0] ) {
		idStr::Append( parseBuffer, MAX_IMAGE_NAME, " " );
	}
	idStr::Append( parseBuffer, MAX_IMAGE_NAME, token.c_str() );
}

/*
===================
MatchAndAppendToken
===================
*/
static void MatchAndAppendToken( idLexer &src, const char *match ) {
	if ( !src.ExpectTokenString( match ) ) {
		return;
	}
	// a matched token won't need a leading space
	idStr::Append( parseBuffer, MAX_IMAGE_NAME, match );
}

/*
===================
R_ParseImageProgram_r

If pic is NULL, the timestamps will be filled in, but no image will be generated
If both pic and timestamps are NULL, it will just advance past it, which can be
used to parse an image program from a text stream.
===================
*/
static bool R_ParseImageProgram_r( idLexer &src, byte **pic, int *width, int *height,
	ID_TIME_T *timestamps, textureDepth_t *depth ) {
		idToken		token;
		float		scale;
		ID_TIME_T		timestamp;

		src.ReadToken( &token );
		AppendToken( token );

		if ( !token.Icmp( "heightmap" ) ) {
			MatchAndAppendToken( src, "(" );

			if ( !R_ParseImageProgram_r( src, pic, width, height, timestamps, depth ) ) {
				return false;
			}

			MatchAndAppendToken( src, "," );

			src.ReadToken( &token );
			AppendToken( token );
			scale = token.GetFloatValue();

			// process it
			if ( pic ) {
				R_HeightmapToNormalMap( *pic, *width, *height, scale );
				if ( depth ) {
					*depth = TD_BUMP;
				}
			}

			MatchAndAppendToken( src, ")" );
			return true;
		}

		if ( !token.Icmp( "addnormals" ) ) {
			byte	*pic2;
			int		width2, height2;

			MatchAndAppendToken( src, "(" );

			if ( !R_ParseImageProgram_r( src, pic, width, height, timestamps, depth ) ) {
				return false;
			}

			MatchAndAppendToken( src, "," );

			if ( !R_ParseImageProgram_r( src, pic ? &pic2 : NULL, &width2, &height2, timestamps, depth ) ) {
				if ( pic ) {
					R_StaticFree( *pic );
					*pic = NULL;
				}
				return false;
			}

			// process it
			if ( pic ) {
				R_AddNormalMaps( *pic, *width, *height, pic2, width2, height2 );
				R_StaticFree( pic2 );
				if ( depth ) {
					*depth = TD_BUMP;
				}
			}

			MatchAndAppendToken( src, ")" );
			return true;
		}

		if ( !token.Icmp( "smoothnormals" ) ) {
			MatchAndAppendToken( src, "(" );

			if ( !R_ParseImageProgram_r( src, pic, width, height, timestamps, depth ) ) {
				return false;
			}

			if ( pic ) {
				R_SmoothNormalMap( *pic, *width, *height );
				if ( depth ) {
					*depth = TD_BUMP;
				}
			}

			MatchAndAppendToken( src, ")" );
			return true;
		}

		if ( !token.Icmp( "add" ) ) {
			byte	*pic2;
			int		width2, height2;

			MatchAndAppendToken( src, "(" );

			if ( !R_ParseImageProgram_r( src, pic, width, height, timestamps, depth ) ) {
				return false;
			}

			MatchAndAppendToken( src, "," );

			if ( !R_ParseImageProgram_r( src, pic ? &pic2 : NULL, &width2, &height2, timestamps, depth ) ) {
				if ( pic ) {
					R_StaticFree( *pic );
					*pic = NULL;
				}
				return false;
			}

			// process it
			if ( pic ) {
				R_ImageAdd( *pic, *width, *height, pic2, width2, height2 );
				R_StaticFree( pic2 );
			}

			MatchAndAppendToken( src, ")" );
			return true;
		}

		if ( !token.Icmp( "scale" ) ) {
			float	scale[4];
			int		i;

			MatchAndAppendToken( src, "(" );

			R_ParseImageProgram_r( src, pic, width, height, timestamps, depth );

			for ( i = 0 ; i < 4 ; i++ ) {
				MatchAndAppendToken( src, "," );
				src.ReadToken( &token );
				AppendToken( token );
				scale[i] = token.GetFloatValue();
			}

			// process it
			if ( pic ) {
				R_ImageScale( *pic, *width, *height, scale );
			}

			MatchAndAppendToken( src, ")" );
			return true;
		}

		if ( !token.Icmp( "invertAlpha" ) ) {
			MatchAndAppendToken( src, "(" );

			R_ParseImageProgram_r( src, pic, width, height, timestamps, depth );

			// process it
			if ( pic ) {
				R_InvertAlpha( *pic, *width, *height );
			}

			MatchAndAppendToken( src, ")" );
			return true;
		}

		if ( !token.Icmp( "invertColor" ) ) {
			MatchAndAppendToken( src, "(" );

			R_ParseImageProgram_r( src, pic, width, height, timestamps, depth );

			// process it
			if ( pic ) {
				R_InvertColor( *pic, *width, *height );
			}

			MatchAndAppendToken( src, ")" );
			return true;
		}

		if ( !token.Icmp( "makeIntensity" ) ) {
			int		i;

			MatchAndAppendToken( src, "(" );

			R_ParseImageProgram_r( src, pic, width, height, timestamps, depth );

			// copy red to green, blue, and alpha
			if ( pic ) {
				int		c;
				c = *width * *height * 4;
				for ( i = 0 ; i < c ; i+=4 ) {
					(*pic)[i+1] = 
						(*pic)[i+2] = 
						(*pic)[i+3] = (*pic)[i];
				}
			}

			MatchAndAppendToken( src, ")" );
			return true;
		}

		if ( !token.Icmp( "makeAlpha" ) ) {
			int		i;

			MatchAndAppendToken( src, "(" );

			R_ParseImageProgram_r( src, pic, width, height, timestamps, depth );

			// average RGB into alpha, then set RGB to white
			if ( pic ) {
				int		c;
				c = *width * *height * 4;
				for ( i = 0 ; i < c ; i+=4 ) {
					(*pic)[i+3] = ( (*pic)[i+0] + (*pic)[i+1] + (*pic)[i+2] ) / 3;
					(*pic)[i+0] = 
						(*pic)[i+1] = 
						(*pic)[i+2] = 255;
				}
			}

			MatchAndAppendToken( src, ")" );
			return true;
		}

		// if we are just parsing instead of loading or checking,
		// don't do the R_LoadImage
		if ( !timestamps && !pic ) {
			return true;
		}

		// load it as an image
		gTex = R_LoadImage( token.c_str(), pic, width, height, &timestamp, true );

		if ( timestamp == -1 ) {
			return false;
		}

		// add this to the timestamp
		if ( timestamps ) {
			if ( timestamp > *timestamps ) {
				*timestamps = timestamp;
			}
		}

		return true;
}


void R_LoadImageProgram( const char *name, byte **pic, int *width, int *height, ID_TIME_T *timestamps, textureDepth_t *depth ) {
	idLexer src;

	src.LoadMemory( name, strlen(name), name );
	src.SetFlags( LEXFL_NOFATALERRORS | LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );

	parseBuffer[0] = 0;
	if ( timestamps ) {
		*timestamps = 0;
	}

	R_ParseImageProgram_r( src, pic, width, height, timestamps, depth );

	src.FreeSource();
}
/*
===================
R_ParsePastImageProgram
===================
*/
const char *R_ParsePastImageProgram( idLexer &src ) {
	parseBuffer[0] = 0;
	R_ParseImageProgram_r( src, NULL, NULL, NULL, NULL, NULL );
	return parseBuffer;
}



TextureID ImageFromFile( const char *_name, textureFilter_t filter, bool allowDownSize,
	textureRepeat_t repeat, textureDepth_t depth, cubeFiles_t cubeMap ) {

	int		width, height;
	byte	*pic;
	idStr name;
	TextureID output = -1;
	ID_TIME_T timestamp;
		
	if ( !_name || !_name[0] || idStr::Icmp( _name, "default" ) == 0 || idStr::Icmp( _name, "_default" ) == 0 ) {
		declManager->MediaPrint( "DEFAULTED\n" );
		//return globalImages->defaultImage;
		return NULL;
	}

	// strip any .tga file extensions from anywhere in the _name, including image program parameters
	name = _name;
	name.Replace( ".tga", "" );
	name.BackSlashesToSlashes();

	if ( cubeMap != CF_2D ) {
		byte	*pics[6];
		int		i, j;
		char	*cameraSides[6] =  { "_forward.tga", "_back.tga", "_left.tga", "_right.tga", 
			"_up.tga", "_down.tga" };
		char	*axisSides[6] =  { "_px.tga", "_nx.tga", "_py.tga", "_ny.tga", 
			"_pz.tga", "_nz.tga" };
		char	**sides;
		char	fullName[MAX_IMAGE_NAME];
		int		width, height, size = 0;

		if ( cubeMap == CF_CAMERA ) {
			sides = cameraSides;
		} else {
			sides = axisSides;
		}

		// FIXME: precompressed cube map files
		if ( pics ) {
			memset( pics, 0, 6*sizeof(pics[0]) );
		}
		if ( timestamp ) {
			timestamp = 0;
		}
		char** filenames = new char*[6];
		for ( i = 0 ; i < 6 ; i++ ) {
			idStr::snPrintf( fullName, sizeof( fullName ), "%s%s", name, sides[i] );
			filenames[i] = fullName;					
		}

		gTex = GetRenderer()->addCubemap((const char**)filenames, true);
	}
	else 
	{		
		//name.DefaultFileExtension(".tga");
		//output = GetRenderer()->addTexture(name.c_str(), true);

		R_LoadImageProgram(name, &pic, &width, &height, &timestamp, &depth);
	}
}

/*
==================
R_HorizontalFlip

Flip the image in place
==================
*/
void R_HorizontalFlip( byte *data, int width, int height ) {
	int		i, j;
	int		temp;

	for ( i = 0 ; i < height ; i++ ) {
		for ( j = 0 ; j < width / 2 ; j++ ) {
			temp = *( (int *)data + i * width + j );
			*( (int *)data + i * width + j ) = *( (int *)data + i * width + width - 1 - j );
			*( (int *)data + i * width + width - 1 - j ) = temp;
		}
	}
}

void R_VerticalFlip( byte *data, int width, int height ) {
	int		i, j;
	int		temp;

	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 0 ; j < height / 2 ; j++ ) {
			temp = *( (int *)data + j * width + i );
			*( (int *)data + j * width + i ) = *( (int *)data + ( height - 1 - j ) * width + i );
			*( (int *)data + ( height - 1 - j ) * width + i ) = temp;
		}
	}
}

void R_RotatePic( byte *data, int width ) {
	int		i, j;
	int		*temp;

	temp = (int *)R_StaticAlloc( width * width * 4 );

	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 0 ; j < width ; j++ ) {
			*( temp + i * width + j ) = *( (int *)data + j * width + i );
		}
	}

	memcpy( data, temp, width * width * 4 );

	R_StaticFree( temp );
}

