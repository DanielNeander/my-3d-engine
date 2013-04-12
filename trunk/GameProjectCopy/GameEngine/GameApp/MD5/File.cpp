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

#include "stdafx.h"
#pragma hdrstop
#include "Dict.h"
#include "BitMsg.h"
#include "File.h"
#include "Unzip.h"

#define	MAX_PRINT_MSG		4096

/*
=================
FS_WriteFloatString
=================
*/
int FS_WriteFloatString( char *buf, const char *fmt, va_list argPtr ) {
	long i;
	unsigned long u;
	double f;
	char *str;
	int index;
	idStr tmp, format;

	index = 0;

	while( *fmt ) {
		switch( *fmt ) {
			case '%':
				format = "";
				format += *fmt++;
				while ( (*fmt >= '0' && *fmt <= '9') ||
						*fmt == '.' || *fmt == '-' || *fmt == '+' || *fmt == '#') {
					format += *fmt++;
				}
				format += *fmt;
				switch( *fmt ) {
					case 'f':
					case 'e':
					case 'E':
					case 'g':
					case 'G':
						f = va_arg( argPtr, double );
						if ( format.Length() <= 2 ) {
							// high precision floating point number without trailing zeros
							sprintf( tmp, "%1.10f", f );
							tmp.StripTrailing( '0' );
							tmp.StripTrailing( '.' );
							index += sprintf( buf+index, "%s", tmp.c_str() );
						}
						else {
							index += sprintf( buf+index, format.c_str(), f );
						}
						break;
					case 'd':
					case 'i':
						i = va_arg( argPtr, long );
						index += sprintf( buf+index, format.c_str(), i );
						break;
					case 'u':
						u = va_arg( argPtr, unsigned long );
						index += sprintf( buf+index, format.c_str(), u );
						break;
					case 'o':
						u = va_arg( argPtr, unsigned long );
						index += sprintf( buf+index, format.c_str(), u );
						break;
					case 'x':
						u = va_arg( argPtr, unsigned long );
						index += sprintf( buf+index, format.c_str(), u );
						break;
					case 'X':
						u = va_arg( argPtr, unsigned long );
						index += sprintf( buf+index, format.c_str(), u );
						break;
					case 'c':
						i = va_arg( argPtr, long );
						index += sprintf( buf+index, format.c_str(), (char) i );
						break;
					case 's':
						str = va_arg( argPtr, char * );
						index += sprintf( buf+index, format.c_str(), str );
						break;
					case '%':
						index += sprintf( buf+index, format.c_str() );
						break;
					default:
						//common->Error( "FS_WriteFloatString: invalid format %s", format.c_str() );
						break;
				}
				fmt++;
				break;
			case '\\':
				fmt++;
				switch( *fmt ) {
					case 't':
						index += sprintf( buf+index, "\t" );
						break;
					case 'v':
						index += sprintf( buf+index, "\v" );
						break;
					case 'n':
						index += sprintf( buf+index, "\n" );
						break;
					case '\\':
						index += sprintf( buf+index, "\\" );
						break;
					default:
						//common->Error( "FS_WriteFloatString: unknown escape character \'%c\'", *fmt );
						break;
				}
				fmt++;
				break;
			default:
				index += sprintf( buf+index, "%c", *fmt );
				fmt++;
				break;
		}
	}

	return index;
}

/*
=================================================================================

idFile

=================================================================================
*/

/*
=================
idFile::GetName
=================
*/
const char *idFile::GetName( void ) {
	return "";
}

/*
=================
idFile::GetFullPath
=================
*/
const char *idFile::GetFullPath( void ) {
	return "";
}

/*
=================
idFile::Read
=================
*/
int idFile::Read( void *buffer, int len ) {
	//common->FatalError( "idFile::Read: cannot read from idFile" );
	return 0;
}

/*
=================
idFile::Write
=================
*/
int idFile::Write( const void *buffer, int len ) {
	//common->FatalError( "idFile::Write: cannot write to idFile" );
	return 0;
}

/*
=================
idFile::Length
=================
*/
int idFile::Length( void ) {
	return 0;
}

/*
=================
idFile::Timestamp
=================
*/
time_t idFile::Timestamp( void ) {
	return 0;
}

/*
=================
idFile::Tell
=================
*/
int idFile::Tell( void ) {
	return 0;
}

/*
=================
idFile::ForceFlush
=================
*/
void idFile::ForceFlush( void ) {
}

/*
=================
idFile::Flush
=================
*/
void idFile::Flush( void ) {
}

/*
=================
idFile::Seek
=================
*/
int idFile::Seek( long offset, fsOrigin_t origin ) {
	return -1;
}

/*
=================
idFile::Rewind
=================
*/
void idFile::Rewind( void ) {
	Seek( 0, FS_SEEK_SET );
}

/*
=================
idFile::Printf
=================
*/
int idFile::Printf( const char *fmt, ... ) {
	char buf[MAX_PRINT_MSG];
	int length;
	va_list argptr;

	va_start( argptr, fmt );
	length = idStr::vsnPrintf( buf, MAX_PRINT_MSG-1, fmt, argptr );
	va_end( argptr );

	// so notepad formats the lines correctly
  	idStr	work( buf );
 	work.Replace( "\n", "\r\n" );
  
  	return Write( work.c_str(), work.Length() );
}

/*
=================
idFile::VPrintf
=================
*/
int idFile::VPrintf( const char *fmt, va_list args ) {
	char buf[MAX_PRINT_MSG];
	int length;

	length = idStr::vsnPrintf( buf, MAX_PRINT_MSG-1, fmt, args );
	return Write( buf, length );
}

/*
=================
idFile::WriteFloatString
=================
*/
int idFile::WriteFloatString( const char *fmt, ... ) {
	char buf[MAX_PRINT_MSG];
	int len;
	va_list argPtr;

	va_start( argPtr, fmt );
	len = FS_WriteFloatString( buf, fmt, argPtr );
	va_end( argPtr );

	return Write( buf, len );
}

/*
 =================
 idFile::ReadInt
 =================
 */
int idFile::ReadInt( int &value ) {
	int result = Read( &value, sizeof( value ) );
	//value = LittleLong(value);
	return result;
}

/*
 =================
 idFile::ReadUnsignedInt
 =================
 */
int idFile::ReadUnsignedInt( unsigned int &value ) {
	int result = Read( &value, sizeof( value ) );
	//value = LittleLong(value);
	return result;
}

/*
 =================
 idFile::ReadShort
 =================
 */
int idFile::ReadShort( short &value ) {
	int result = Read( &value, sizeof( value ) );
	//value = LittleShort(value);
	return result;
}

/*
 =================
 idFile::ReadUnsignedShort
 =================
 */
int idFile::ReadUnsignedShort( unsigned short &value ) {
	int result = Read( &value, sizeof( value ) );
	//value = LittleShort(value);
	return result;
}

/*
 =================
 idFile::ReadChar
 =================
 */
int idFile::ReadChar( char &value ) {
	return Read( &value, sizeof( value ) );
}

/*
 =================
 idFile::ReadUnsignedChar
 =================
 */
int idFile::ReadUnsignedChar( unsigned char &value ) {
	return Read( &value, sizeof( value ) );
}

/*
 =================
 idFile::ReadFloat
 =================
 */
int idFile::ReadFloat( float &value ) {
	int result = Read( &value, sizeof( value ) );
	//value = LittleFloat(value);
	return result;
}

/*
 =================
 idFile::ReadBool
 =================
 */
int idFile::ReadBool( bool &value ) {
	unsigned char c;
	int result = ReadUnsignedChar( c );
	value = c ? true : false;
	return result;
}

/*
 =================
 idFile::ReadString
 =================
 */
int idFile::ReadString( idStr &string ) {
	int len;
	int result = 0;
	
	ReadInt( len );
	if ( len >= 0 ) {
		string.Fill( ' ', len );
		result = Read( &string[ 0 ], len );
	}
	return result;
}

/*
 =================
 idFile::ReadVec2
 =================
 */
int idFile::ReadVec2( noVec2 &vec ) {
	int result = Read( &vec, sizeof( vec ) );
	//LittleRevBytes( &vec, sizeof(float), sizeof(vec)/sizeof(float) );
	return result;
}

/*
 =================
 idFile::ReadVec3
 =================
 */
int idFile::ReadVec3( noVec3 &vec ) {
	int result = Read( &vec, sizeof( vec ) );
	//LittleRevBytes( &vec, sizeof(float), sizeof(vec)/sizeof(float) );
	return result;
}

/*
 =================
 idFile::ReadVec4
 =================
 */
int idFile::ReadVec4( noVec4 &vec ) {
	int result = Read( &vec, sizeof( vec ) );
	//LittleRevBytes( &vec, sizeof(float), sizeof(vec)/sizeof(float) );
	return result;
}

/*
 =================
 idFile::ReadVec6
 =================
 */
int idFile::ReadVec6( idVec6 &vec ) {
	int result = Read( &vec, sizeof( vec ) );
	//LittleRevBytes( &vec, sizeof(float), sizeof(vec)/sizeof(float) );
	return result;
}

/*
 =================
 idFile::ReadMat3
 =================
 */
int idFile::ReadMat3( noMat3 &mat ) {
	int result = Read( &mat, sizeof( mat ) );
	//LittleRevBytes( &mat, sizeof(float), sizeof(mat)/sizeof(float) );
	return result;
}

/*
 =================
 idFile::WriteInt
 =================
 */
int idFile::WriteInt( const int value ) {
	//int v = LittleLong(value);
	int v = value;
	return Write( &v, sizeof( v ) );
}

/*
 =================
 idFile::WriteUnsignedInt
 =================
 */
int idFile::WriteUnsignedInt( const unsigned int value ) {
	//unsigned int v = LittleLong(value);
	unsigned int v = value;
	return Write( &v, sizeof( v ) );
}

/*
 =================
 idFile::WriteShort
 =================
 */
int idFile::WriteShort( const short value ) {
	//short v = LittleShort(value);
	short v = value;
	return Write( &v, sizeof( v ) );
}

/*
 =================
 idFile::WriteUnsignedShort
 =================
 */
int idFile::WriteUnsignedShort( const unsigned short value ) {
	//unsigned short v = LittleShort(value);
	unsigned short v = value;
	return Write( &v, sizeof( v ) );
}

/*
 =================
 idFile::WriteChar
 =================
 */
int idFile::WriteChar( const char value ) {
	return Write( &value, sizeof( value ) );
}

/*
 =================
 idFile::WriteUnsignedChar
 =================
 */
int idFile::WriteUnsignedChar( const unsigned char value ) {
	return Write( &value, sizeof( value ) );
}

/*
 =================
 idFile::WriteFloat
 =================
 */
int idFile::WriteFloat( const float value ) {
	//float v = LittleFloat(value);
	float v =value;
	return Write( &v, sizeof( v ) );
}

/*
 =================
 idFile::WriteBool
 =================
 */
int idFile::WriteBool( const bool value ) {
	unsigned char c = value;
	return WriteUnsignedChar( c );
}

/*
 =================
 idFile::WriteString
 =================
 */
int idFile::WriteString( const char *value ) {
	int len;
	
	len = strlen( value );
	WriteInt( len );
    return Write( value, len );
}

/*
 =================
 idFile::WriteVec2
 =================
 */
int idFile::WriteVec2( const noVec2 &vec ) {
	noVec2 v = vec;
	//LittleRevBytes( &v, sizeof(float), sizeof(v)/sizeof(float) );
	return Write( &v, sizeof( v ) );
}

/*
 =================
 idFile::WriteVec3
 =================
 */
int idFile::WriteVec3( const noVec3 &vec ) {
	noVec3 v = vec;
	//LittleRevBytes( &v, sizeof(float), sizeof(v)/sizeof(float) );
	return Write( &v, sizeof( v ) );
}

/*
 =================
 idFile::WriteVec4
 =================
 */
int idFile::WriteVec4( const noVec4 &vec ) {
	noVec4 v = vec;
	//LittleRevBytes( &v, sizeof(float), sizeof(v)/sizeof(float) );
	return Write( &v, sizeof( v ) );
}

/*
 =================
 idFile::WriteVec6
 =================
 */
int idFile::WriteVec6( const idVec6 &vec ) {
	idVec6 v = vec;
	//LittleRevBytes( &v, sizeof(float), sizeof(v)/sizeof(float) );
	return Write( &v, sizeof( v ) );
}

/*
 =================
 idFile::WriteMat3
 =================
 */
int idFile::WriteMat3( const noMat3 &mat ) {
	noMat3 v = mat;
	//LittleRevBytes(&v, sizeof(float), sizeof(v)/sizeof(float) );
	return Write( &v, sizeof( v ) );
}

/*
=================================================================================

idFile_Memory

=================================================================================
*/


/*
=================
idFile_Memory::idFile_Memory
=================
*/
idFile_Memory::idFile_Memory( void ) {
	name = "*unknown*";
	maxSize = 0;
	fileSize = 0;
	allocated = 0;
	granularity = 16384;

	mode = ( 1 << FS_WRITE );
	filePtr = NULL;
	curPtr = NULL;
}

/*
=================
idFile_Memory::idFile_Memory
=================
*/
idFile_Memory::idFile_Memory( const char *name ) {
	this->name = name;
	maxSize = 0;
	fileSize = 0;
	allocated = 0;
	granularity = 16384;

	mode = ( 1 << FS_WRITE );
	filePtr = NULL;
	curPtr = NULL;
}

/*
=================
idFile_Memory::idFile_Memory
=================
*/
idFile_Memory::idFile_Memory( const char *name, char *data, int length ) {
	this->name = name;
	maxSize = length;
	fileSize = 0;
	allocated = length;
	granularity = 16384;

	mode = ( 1 << FS_WRITE );
	filePtr = data;
	curPtr = data;
}

/*
=================
idFile_Memory::idFile_Memory
=================
*/
idFile_Memory::idFile_Memory( const char *name, const char *data, int length ) {
	this->name = name;
	maxSize = 0;
	fileSize = length;
	allocated = 0;
	granularity = 16384;

	mode = ( 1 << FS_READ );
	filePtr = const_cast<char *>(data);
	curPtr = const_cast<char *>(data);
}

/*
=================
idFile_Memory::~idFile_Memory
=================
*/
idFile_Memory::~idFile_Memory( void ) {
	if ( filePtr && allocated > 0 && maxSize == 0 ) {
		Mem_Free( filePtr );
	}
}

/*
=================
idFile_Memory::Read
=================
*/
int idFile_Memory::Read( void *buffer, int len ) {

	if ( !( mode & ( 1 << FS_READ ) ) ) {
		//common->FatalError( "idFile_Memory::Read: %s not opened in read mode", name.c_str() );
		return 0;
	}

	if ( curPtr + len > filePtr + fileSize ) {
		len = filePtr + fileSize - curPtr;
	}
	memcpy( buffer, curPtr, len );
	curPtr += len;
	return len;
}

/*
=================
idFile_Memory::Write
=================
*/
int idFile_Memory::Write( const void *buffer, int len ) {

	if ( !( mode & ( 1 << FS_WRITE ) ) ) {
		//common->FatalError( "idFile_Memory::Write: %s not opened in write mode", name.c_str() );
		return 0;
	}

	int alloc = curPtr + len + 1 - filePtr - allocated; // need room for len+1
	if ( alloc > 0 ) {
		if ( maxSize != 0 ) {
			//common->Error( "idFile_Memory::Write: exceeded maximum size %d", maxSize );
			return 0;
		}
		int extra = granularity * ( 1 + alloc / granularity );
		char *newPtr = (char *) Mem_Alloc( allocated + extra );
		if ( allocated ) {
			memcpy( newPtr, filePtr, allocated );
		}
		allocated += extra;
		curPtr = newPtr + ( curPtr - filePtr );		
		if ( filePtr ) {
			Mem_Free( filePtr );
		}
		filePtr = newPtr;
	}
	memcpy( curPtr, buffer, len );
	curPtr += len;
	fileSize += len;
	filePtr[ fileSize ] = 0; // len + 1
	return len;
}

/*
=================
idFile_Memory::Length
=================
*/
int idFile_Memory::Length( void ) {
	return fileSize;
}

/*
=================
idFile_Memory::Timestamp
=================
*/
time_t idFile_Memory::Timestamp( void ) {
	return 0;
}

/*
=================
idFile_Memory::Tell
=================
*/
int idFile_Memory::Tell( void ) {
	return ( curPtr - filePtr );
}

/*
=================
idFile_Memory::ForceFlush
=================
*/
void idFile_Memory::ForceFlush( void ) {
}

/*
=================
idFile_Memory::Flush
=================
*/
void idFile_Memory::Flush( void ) {
}

/*
=================
idFile_Memory::Seek

  returns zero on success and -1 on failure
=================
*/
int idFile_Memory::Seek( long offset, fsOrigin_t origin ) {

	switch( origin ) {
		case FS_SEEK_CUR: {
			curPtr += offset;
			break;
		}
		case FS_SEEK_END: {
			curPtr = filePtr + fileSize - offset;
			break;
		}
		case FS_SEEK_SET: {
			curPtr = filePtr + offset;
			break;
		}
		default: {
			//common->FatalError( "idFile_Memory::Seek: bad origin for %s\n", name.c_str() );
			return -1;
		}
	}
	if ( curPtr < filePtr ) {
		curPtr = filePtr;
		return -1;
	}
	if ( curPtr > filePtr + fileSize ) {
		curPtr = filePtr + fileSize;
		return -1;
	}
	return 0;
}

/*
=================
idFile_Memory::MakeReadOnly
=================
*/
void idFile_Memory::MakeReadOnly( void ) {
	mode = ( 1 << FS_READ );
	Rewind();
}

/*
=================
idFile_Memory::Clear
=================
*/
void idFile_Memory::Clear( bool freeMemory ) {
	fileSize = 0;
	granularity = 16384;
	if ( freeMemory ) {
		allocated = 0;
		Mem_Free( filePtr );
		filePtr = NULL;
		curPtr = NULL;
	} else {
		curPtr = filePtr;
	}
}

/*
=================
idFile_Memory::SetData
=================
*/
void idFile_Memory::SetData( const char *data, int length ) {
	maxSize = 0;
	fileSize = length;
	allocated = 0;
	granularity = 16384;

	mode = ( 1 << FS_READ );
	filePtr = const_cast<char *>(data);
	curPtr = const_cast<char *>(data);
}


/*
=================================================================================

idFile_BitMsg

=================================================================================
*/

/*
=================
idFile_BitMsg::idFile_BitMsg
=================
*/
idFile_BitMsg::idFile_BitMsg( idBitMsg &msg ) {
	name = "*unknown*";
	mode = ( 1 << FS_WRITE );
	this->msg = &msg;
}

/*
=================
idFile_BitMsg::idFile_BitMsg
=================
*/
idFile_BitMsg::idFile_BitMsg( const idBitMsg &msg ) {
	name = "*unknown*";
	mode = ( 1 << FS_READ );
	this->msg = const_cast<idBitMsg *>(&msg);
}

/*
=================
idFile_BitMsg::~idFile_BitMsg
=================
*/
idFile_BitMsg::~idFile_BitMsg( void ) {
}

/*
=================
idFile_BitMsg::Read
=================
*/
int idFile_BitMsg::Read( void *buffer, int len ) {

	if ( !( mode & ( 1 << FS_READ ) ) ) {
		//common->FatalError( "idFile_BitMsg::Read: %s not opened in read mode", name.c_str() );
		return 0;
	}

	return msg->ReadData( buffer, len );
}

/*
=================
idFile_BitMsg::Write
=================
*/
int idFile_BitMsg::Write( const void *buffer, int len ) {

	if ( !( mode & ( 1 << FS_WRITE ) ) ) {
		//common->FatalError( "idFile_Memory::Write: %s not opened in write mode", name.c_str() );
		return 0;
	}

	msg->WriteData( buffer, len );
	return len;
}

/*
=================
idFile_BitMsg::Length
=================
*/
int idFile_BitMsg::Length( void ) {
	return msg->GetSize();
}

/*
=================
idFile_BitMsg::Timestamp
=================
*/
time_t idFile_BitMsg::Timestamp( void ) {
	return 0;
}

/*
=================
idFile_BitMsg::Tell
=================
*/
int idFile_BitMsg::Tell( void ) {
	if ( mode & FS_READ ) {
		return msg->GetReadCount();
	} else {
		return msg->GetSize();
	}
}

/*
=================
idFile_BitMsg::ForceFlush
=================
*/
void idFile_BitMsg::ForceFlush( void ) {
}

/*
=================
idFile_BitMsg::Flush
=================
*/
void idFile_BitMsg::Flush( void ) {
}

/*
=================
idFile_BitMsg::Seek

  returns zero on success and -1 on failure
=================
*/
int idFile_BitMsg::Seek( long offset, fsOrigin_t origin ) {
	return -1;
}


/*
=================================================================================

idFile_Permanent

=================================================================================
*/

/*
=================
idFile_Permanent::idFile_Permanent
=================
*/
idFile_Permanent::idFile_Permanent( void ) {
	name = "invalid";
	o = NULL;
	mode = 0;
	fileSize = 0;
	handleSync = false;
}

/*
=================
idFile_Permanent::~idFile_Permanent
=================
*/
idFile_Permanent::~idFile_Permanent( void ) {
	if ( o ) {
		fclose( o );
	}
}

/*
=================
idFile_Permanent::Read

Properly handles partial reads
=================
*/
int idFile_Permanent::Read( void *buffer, int len ) {
	int		block, remaining;
	int		read;
	byte *	buf;
	int		tries;

	if ( !(mode & ( 1 << FS_READ ) ) ) {
		//common->FatalError( "idFile_Permanent::Read: %s not opened in read mode", name.c_str() );
		return 0;
	}

	if ( !o ) {
		return 0;
	}

	buf = (byte *)buffer;

	remaining = len;
	tries = 0;
	while( remaining ) {
		block = remaining;
		read = fread( buf, 1, block, o );
		if ( read == 0 ) {
			// we might have been trying to read from a CD, which
			// sometimes returns a 0 read on windows
			if ( !tries ) {
				tries = 1;
			}
			else {
				//fileSystem->AddToReadCount( len - remaining );
				return len-remaining;
			}
		}

		if ( read == -1 ) {
			//common->FatalError( "idFile_Permanent::Read: -1 bytes read from %s", name.c_str() );
		}

		remaining -= read;
		buf += read;
	}
	//fileSystem->AddToReadCount( len );
	return len;
}

/*
=================
idFile_Permanent::Write

Properly handles partial writes
=================
*/
int idFile_Permanent::Write( const void *buffer, int len ) {
	int		block, remaining;
	int		written;
	byte *	buf;
	int		tries;

	if ( !( mode & ( 1 << FS_WRITE ) ) ) {
		//common->FatalError( "idFile_Permanent::Write: %s not opened in write mode", name.c_str() );
		return 0;
	}

	if ( !o ) {
		return 0;
	}

	buf = (byte *)buffer;

	remaining = len;
	tries = 0;
	while( remaining ) {
		block = remaining;
		written = fwrite( buf, 1, block, o );
		if ( written == 0 ) {
			if ( !tries ) {
				tries = 1;
			}
			else {
				//common->Printf( "idFile_Permanent::Write: 0 bytes written to %s\n", name.c_str() );
				return 0;
			}
		}

		if ( written == -1 ) {
			//common->Printf( "idFile_Permanent::Write: -1 bytes written to %s\n", name.c_str() );
			return 0;
		}

		remaining -= written;
		buf += written;
		fileSize += written;
	}
	if ( handleSync ) {
		fflush( o );
	}
	return len;
}

/*
=================
idFile_Permanent::ForceFlush
=================
*/
void idFile_Permanent::ForceFlush( void ) {
	setvbuf( o, NULL, _IONBF, 0 );
}

/*
=================
idFile_Permanent::Flush
=================
*/
void idFile_Permanent::Flush( void ) {
	fflush( o );
}

/*
=================
idFile_Permanent::Tell
=================
*/
int idFile_Permanent::Tell( void ) {
	return ftell( o );
}

/*
================
idFile_Permanent::Length
================
*/
int idFile_Permanent::Length( void ) {
	return fileSize;
}

/*
================
idFile_Permanent::Timestamp
================
*/
time_t idFile_Permanent::Timestamp( void ) {
	//return Sys_FileTimeStamp( o );
	return 0;
}

/*
=================
idFile_Permanent::Seek

  returns zero on success and -1 on failure
=================
*/
int idFile_Permanent::Seek( long offset, fsOrigin_t origin ) {
	int _origin;

	switch( origin ) {
		case FS_SEEK_CUR: {
			_origin = SEEK_CUR;
			break;
		}
		case FS_SEEK_END: {
			_origin = SEEK_END;
			break;
		}
		case FS_SEEK_SET: {
			_origin = SEEK_SET;
			break;
		}
		default: {
			_origin = SEEK_CUR;
			//common->FatalError( "idFile_Permanent::Seek: bad origin for %s\n", name.c_str() );
			break;
		}
	}

	return fseek( o, offset, _origin );
}


/*
=================================================================================

idFile_InZip

=================================================================================
*/

/*
=================
idFile_InZip::idFile_InZip
=================
*/
idFile_InZip::idFile_InZip( void ) {
	name = "invalid";
	zipFilePos = 0;
	fileSize = 0;
	memset( &z, 0, sizeof( z ) );
}

/*
=================
idFile_InZip::~idFile_InZip
=================
*/
idFile_InZip::~idFile_InZip( void ) {
	unzCloseCurrentFile( z );
	unzClose( z );
}

/*
=================
idFile_InZip::Read

Properly handles partial reads
=================
*/
int idFile_InZip::Read( void *buffer, int len ) {
	int l = unzReadCurrentFile( z, buffer, len );
	//fileSystem->AddToReadCount( l );
	return l;
}

/*
=================
idFile_InZip::Write
=================
*/
int idFile_InZip::Write( const void *buffer, int len ) {
	//common->FatalError( "idFile_InZip::Write: cannot write to the zipped file %s", name.c_str() );
	return 0;
}

/*
=================
idFile_InZip::ForceFlush
=================
*/
void idFile_InZip::ForceFlush( void ) {
	//common->FatalError( "idFile_InZip::ForceFlush: cannot flush the zipped file %s", name.c_str() );
}

/*
=================
idFile_InZip::Flush
=================
*/
void idFile_InZip::Flush( void ) {
	//common->FatalError( "idFile_InZip::Flush: cannot flush the zipped file %s", name.c_str() );
}

/*
=================
idFile_InZip::Tell
=================
*/
int idFile_InZip::Tell( void ) {
	return unztell( z );
}

/*
================
idFile_InZip::Length
================
*/
int idFile_InZip::Length( void ) {
	return fileSize;
}

/*
================
idFile_InZip::Timestamp
================
*/
time_t idFile_InZip::Timestamp( void ) {
	return 0;
}

/*
=================
idFile_InZip::Seek

  returns zero on success and -1 on failure
=================
*/
#define ZIP_SEEK_BUF_SIZE	(1<<15)

int idFile_InZip::Seek( long offset, fsOrigin_t origin ) {
	int res, i;
	char *buf;

	switch( origin ) {
		case FS_SEEK_END: {
			offset = fileSize - offset;
		}
		case FS_SEEK_SET: {
			// set the file position in the zip file (also sets the current file info)
			unzSetCurrentFileInfoPosition( z, zipFilePos );
			unzOpenCurrentFile( z );
			if ( offset <= 0 ) {
				return 0;
			}
		}
		case FS_SEEK_CUR: {
			buf = (char *) _alloca16( ZIP_SEEK_BUF_SIZE );
			for ( i = 0; i < ( offset - ZIP_SEEK_BUF_SIZE ); i += ZIP_SEEK_BUF_SIZE ) {
				res = unzReadCurrentFile( z, buf, ZIP_SEEK_BUF_SIZE );
				if ( res < ZIP_SEEK_BUF_SIZE ) {
					return -1;
				}
			}
			res = i + unzReadCurrentFile( z, buf, offset - i );
			return ( res == offset ) ? 0 : -1;
		}
		default: {
			//common->FatalError( "idFile_InZip::Seek: bad origin for %s\n", name.c_str() );
			break;
		}
	}
	return -1;
}


/*
   MD5 Message Digest Algorithm. (RFC1321)
*/

/*

This code implements the MD5 message-digest algorithm.
The algorithm is due to Ron Rivest.  This code was
written by Colin Plumb in 1993, no copyright is claimed.
This code is in the public domain; do with it what you wish.

Equivalent code is available from RSA Data Security, Inc.
This code has been tested against that, and is equivalent,
except that you don't need to include two pages of legalese
with every copy.

To compute the message digest of a chunk of bytes, declare an
MD5Context structure, pass it to MD5Init, call MD5Update as
needed on buffers full of bytes, and then call MD5Final, which
will fill a supplied 16-byte array with the digest.

*/

/* MD5 context. */
typedef struct
{
	unsigned int	state[4];
    unsigned int	bits[2];
    unsigned char	in[64];
} MD5_CTX;

/* The four core functions - F1 is optimized somewhat */
/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) ( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

/*
=================
MD5_Transform

The core of the MD5 algorithm, this alters an existing MD5 hash to
reflect the addition of 16 longwords of new data.  MD5Update blocks
the data and converts bytes into longwords for this routine.
=================
*/
void MD5_Transform( unsigned int state[4], unsigned int in[16] ) {
    register unsigned int a, b, c, d;

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];

	//x86
	//LittleRevBytes( in, sizeof(unsigned int), 16 );

    MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

	// x86
	//LittleRevBytes( in, sizeof(unsigned int), 16 );

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

/*
==================
MD5_Init

MD5 initialization. Begins an MD5 operation, writing a new context.
==================
*/
void MD5_Init( MD5_CTX *ctx ) {
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;

    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}

/*
===================
MD5_Update

MD5 block update operation. Continues an MD5 message-digest operation,
processing another message block, and updating the context.
===================
*/
void MD5_Update( MD5_CTX *ctx, unsigned char const *buf, unsigned int len ) {
    unsigned int t;

    /* Update bitcount */

    t = ctx->bits[0];
	if ( ( ctx->bits[0] = t + ( (unsigned int) len << 3 ) ) < t ) {
        ctx->bits[1]++;         /* Carry from low to high */
	}
    ctx->bits[1] += len >> 29;

    t = ( t >> 3 ) & 0x3f;        /* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */

    if ( t ) {
        unsigned char *p = (unsigned char *) ctx->in + t;

        t = 64 - t;
        if ( len < t ) {
            memcpy( p, buf, len );
            return;
        }
        memcpy( p, buf, t );
        MD5_Transform( ctx->state, (unsigned int *) ctx->in );
        buf += t;
        len -= t;
    }
    /* Process data in 64-byte chunks */

    while( len >= 64 ) {
        memcpy( ctx->in, buf, 64 );
        MD5_Transform( ctx->state, (unsigned int *) ctx->in );
        buf += 64;
        len -= 64;
    }

    /* Handle any remaining bytes of data. */
    memcpy( ctx->in, buf, len );
}

/*
===============
MD5_Final

MD5 finalization. Ends an MD5 message-digest operation,
writing the message digest and zeroizing the context.
===============
*/
void MD5_Final( MD5_CTX *ctx, unsigned char digest[16] ) {
    unsigned count;
    unsigned char *p;

    /* Compute number of bytes mod 64 */
    count = ( ctx->bits[0] >> 3 ) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    p = ctx->in + count;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    count = 64 - 1 - count;

    /* Pad out to 56 mod 64 */
    if ( count < 8 ) {
        /* Two lots of padding:  Pad the first block to 64 bytes */
        memset( p, 0, count );
        MD5_Transform( ctx->state, (unsigned int *) ctx->in );

        /* Now fill the next block with 56 bytes */
        memset( ctx->in, 0, 56 );
    } else {
        /* Pad block to 56 bytes */
        memset( p, 0, count - 8 );
    }

    /* Append length in bits and transform */
	unsigned int val0 = ctx->bits[0];
	unsigned int val1 = ctx->bits[1];
	
    ((unsigned int *) ctx->in)[14] = val0;//LittleLong( val0 );
    ((unsigned int *) ctx->in)[15] = val1;//LittleLong( val1 );

    MD5_Transform( ctx->state, (unsigned int *) ctx->in );
    memcpy( digest, ctx->state, 16 );
    memset( ctx, 0, sizeof( ctx ) );        /* In case it's sensitive */
}

/*
===============
MD5_BlockChecksum
===============
*/
unsigned long MD5_BlockChecksum( const void *data, int length ) {
	unsigned long	digest[4];
	unsigned long	val;
	MD5_CTX			ctx;

	MD5_Init( &ctx );
	MD5_Update( &ctx, (unsigned char *)data, length );
	MD5_Final( &ctx, (unsigned char *)digest );

	val = digest[0] ^ digest[1] ^ digest[2] ^ digest[3];

	return val;
}
