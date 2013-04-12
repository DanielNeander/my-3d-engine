/*
	This file implements a crc checksum object.
*/
#ifndef CRC_H
#define CRC_H

#include <stdio.h>
#include <string>

#include "basictypes.h"

	/*!
	-- Crc32Generator: holds a table for fast crc generation.
	*/
	class Crc32Generator {
	public:
		explicit Crc32Generator();

		/// starting point for a crc:
		U32		Start() const { return 0xFFFFFFFF; }
		/// add 'byte' to the running crc code:
		U32		AddByte( const Byte byte, U32 crc ) const {
			return ((crc) >> 8) ^ m_crcTable[(byte) ^ ((crc) & 0x000000FF)];
		}
		/// sum 2 crc codes
		U32		SumCrc( U32 crc1, U32 crc2 ) const {
			return (crc1 ^ crc2);
		}
		/// finish with 'crc'
		U32		Done(U32 crc) const { return ~crc; }
	protected:
			U32		m_crcTable[256];
	};	//end of Crc32Generator

	/// generates a crc code for the entire content of the stream:
	U32	GenerateCrc( const std::string& filename );
	/// generates a crc code for the entire content of the stream:
	U32	GenerateCrc( FILE* stream );
	/// generates a crc code for the entire content of the stream:
	U32 GenerateCrc( void* data, int nbytes );


#endif
