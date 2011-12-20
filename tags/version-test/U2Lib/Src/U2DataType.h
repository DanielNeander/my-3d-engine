#pragma once	
#ifndef	U2_TYPE_H
#define	U2_TYPE_H

typedef		__int8						Int8;
typedef		unsigned __int8				uint8;
typedef		__int16						int16;
typedef		unsigned __int16			uint16;
typedef		__int32						int32;
typedef		unsigned __int32			uint32;
typedef		__int64						int64;
typedef		unsigned __int64			uint64;

struct U2Float4 
{
	float x,y,z,w;
};

struct U2Float3 
{
	float x, y, z;
};

struct U2Float2 
{
	float x, y;
};

#define MAKE_FOURCC(c0, c1, c2, c3) ((c0) | (c1 << 8) | (c2 << 16) | (c3 << 24))

typedef unsigned int U2FourCC;


#define U2_WHITESPACE _T(" \r\n\t")


#endif
