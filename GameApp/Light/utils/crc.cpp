/*
	This file implements a crc object.
*/

#include <stdio.h>
#include <malloc.h>

#include "crc.h"

#pragma warning (disable: 4996)

#define ALLOCA(s)	_alloca(s)

Crc32Generator gCrc32Generator;


Crc32Generator::Crc32Generator()
{
	U32 polynomial = 0xEDB88320;	//degree-32 polynomial that is primitive in the Galois field of polynomials modulo 2
	int i, j;

	U32 crc;
	for (i = 0; i < 256; i++) {
		crc = i;
		for (j = 8; j > 0; j--) {
			if (crc & 1)
				crc = (crc >> 1) ^ polynomial;
			else
				crc >>= 1;
		}
		m_crcTable[i] = crc;
	}
}

#define BUFSIZE	1024*4

/// generates a crc code for the entire content of the stream:
U32	GenerateCrc( const std::string& filename )
{
	U32 crc = 0;
	FILE* stream = fopen( filename.c_str(), "rb" );
	if (stream != 0) {
		crc = GenerateCrc(stream);
		fclose(stream);
	}
	return crc;
}

/// generates a crc code for the entire content of the stream:
U32	GenerateCrc( FILE* stream )
{
	U32 crc = gCrc32Generator.Start();
	Byte* buf = (Byte*)ALLOCA( BUFSIZE );

	size_t count = fread( buf, 1, BUFSIZE, stream );
	while (count != 0) {
		Byte* p = (Byte*)buf;
		Byte* e = buf + count;

		while (p != e) {
			Byte b = *p++;
			crc = gCrc32Generator.AddByte(b, crc);
		}
		count = fread( buf, 1, BUFSIZE, stream );
	}
	crc = gCrc32Generator.Done(crc);

	return crc;
}
/// generates a crc code for the entire content of the stream:
U32 GenerateCrc( void* data, int nbytes )
{
	Byte* p = (Byte*)data;
	Byte* e = p + nbytes;
	U32 crc = gCrc32Generator.Start();

	while (p != e) {
		Byte b = *p++;
		crc = gCrc32Generator.AddByte(b, crc);
	}
	crc = gCrc32Generator.Done(crc);

	return crc;
}
