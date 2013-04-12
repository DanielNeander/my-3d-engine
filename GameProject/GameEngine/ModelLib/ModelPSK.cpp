#include "stdafx.h"
#include <EngineCore/Types.h>
//#include <Core/GameLog.h>
//#include <Core/U2WinFile.h>
#include <EngineCore/Math/Vector.h>
#include <EngineCore/Math/Quaternion.h>
#include "ModelPSK.h"
//#include <Animation/Interleaved/noaInterleavedAnimation.h>

template< class T > inline void UnSwap( T& A, T& B )
{
	const T Temp = A;
	A = B;
	B = Temp;
}
template< class T > inline void Exchange( T& A, T& B )
{
	UnSwap(A, B);
}

class CompareVRawBoneInfluenceConstRef {
public:
	static inline INT Compare( const VRawBoneInfluence& A, const VRawBoneInfluence& B)
	{
		if		( A.VertexIndex > B.VertexIndex	) return  1;
		else if ( A.VertexIndex < B.VertexIndex	) return -1;
		else if ( A.Weight      < B.Weight		) return  1;
		else if ( A.Weight      > B.Weight		) return -1;
		else if ( A.BoneIndex   > B.BoneIndex	) return  1;
		else if ( A.BoneIndex   < B.BoneIndex	) return -1;
		else									  return  0;
	}
};

static INT FindBoneIndex( const hkArray<FMeshBone>& RefSkeleton, const hkString& BoneName )
{
	for(INT i=0; i<RefSkeleton.getSize(); i++)
	{
		if(RefSkeleton[i].Name == BoneName)
		{
			return i;
		}
	}

	return -1;
}


/**
* Helper function for filling in the various FSkeletalMeshBinaryImport values from buffer of data
* 
* @param DeshkArray - destination array of data to allocate and copy to
* @param BufferReadPtr - source data buffer to read from
* @param BufferEnd - end of data buffer
*/
template<typename DataType>
static void CopyMeshDataAndAdvance( hkArray<DataType>& DeshkArray, BYTE*& BufferReadPtr, const BYTE* BufferEnd )
{
	// assume that BufferReadPtr is positioned at next chunk header 
	const VChunkHeader* ChunkHeader = (const VChunkHeader*) BufferReadPtr;
	// advance buffer ptr to data block
	BufferReadPtr += sizeof(VChunkHeader);	
	// make sure we don't overrun the buffer when reading data
	//check((sizeof(DataType) * ChunkHeader->DataCount + BufferReadPtr) <= BufferEnd);
	// allocate space in import data
	DeshkArray.expandBy( ChunkHeader->DataCount );	
	// copy from buffer
	memcpy( &DeshkArray[0], BufferReadPtr, sizeof(DataType) * ChunkHeader->DataCount );
	// advance buffer
	BufferReadPtr  += sizeof(DataType) * ChunkHeader->DataCount;		
};

void FSkeletalMeshBinaryImport::ImportFromFile( BYTE* BufferReadPtr, const BYTE* BufferEnd, BOOL bShowSummary /*= TRUE */ )
{
	// Skip passed main dummy header. 
	BufferReadPtr  += sizeof(VChunkHeader);

	// Read the temp skin structures..
	// 3d points "vpoints" datasize*datacount....
	CopyMeshDataAndAdvance<FVector>( Points, BufferReadPtr, BufferEnd );

	//  Wedges (VVertex)	
	CopyMeshDataAndAdvance<VVertex>( Wedges, BufferReadPtr, BufferEnd );

	// Faces (VTriangle)
	CopyMeshDataAndAdvance<VTriangle>( Faces, BufferReadPtr, BufferEnd );

	// Materials (VMaterial)
	CopyMeshDataAndAdvance<VMaterial>( Materials, BufferReadPtr, BufferEnd );

	// Reference skeleton (VBones)
	CopyMeshDataAndAdvance<VBone>( RefBonesBinary, BufferReadPtr, BufferEnd );

	// Raw bone influences (VRawBoneInfluence)
	CopyMeshDataAndAdvance<VRawBoneInfluence>( Influences, BufferReadPtr, BufferEnd );

	// Y-flip quaternions and translations from Max/Maya/etc space into Unreal space.
	for( INT b=0; b<RefBonesBinary.getSize(); b++)
	{
		noQuat Bone = RefBonesBinary[b].BonePos.Orientation;
		//Bone.m_vec(1) = - Bone.m_vec(1);
		//// W inversion only needed on the parent - since PACKAGE_FILE_VERSION 133.
		//// @todo - clean flip out of/into exporters
		//if( b==0 ) 
		//{
		//	Bone.m_vec(3) = - Bone.m_vec(3); 
		//}
		RefBonesBinary[b].BonePos.Orientation = Bone;

		FVector Pos = RefBonesBinary[b].BonePos.Position;
		Pos.Y = - Pos.Y;
		RefBonesBinary[b].BonePos.Position = Pos;
	}

	// Y-flip skin, and adjust handedness
	for( INT p=0; p<Points.getSize(); p++ )
	{
		Points[p].Y *= -1;
	}

	for( INT f=0; f<Faces.getSize(); f++)
	{
		Exchange( Faces[f].WedgeIndex[1], Faces[f].WedgeIndex[2] );
	}	

	// Necessary: Fixup face material index from wedge 0 as faces don't always have the proper material index (exporter's task).
	for( INT i=0; i<Faces.getSize(); i++)
	{
		Faces[i].MatIndex		= Wedges[Faces[i].WedgeIndex[0]].MatIndex;
		Faces[i].AuxMatIndex	= 0;
	}

	if( bShowSummary )
	{
		// display summary info
		LOG_INFO << _T(" * Skeletal skin VPoints            : ")  << Points.getSize()			;
		LOG_INFO << _T(" * Skeletal skin VVertices          : ")	<< Wedges.getSize()			;
		LOG_INFO << _T(" * Skeletal skin VTriangles         : ")	<< Faces.getSize()			;
		LOG_INFO << _T(" * Skeletal skin VMaterials         : ")	<< Materials.getSize()		;
		LOG_INFO << _T(" * Skeletal skin VBones             : ")	<< RefBonesBinary.getSize()	;
		LOG_INFO << _T(" * Skeletal skin VRawBoneInfluences : ")	<< Influences.getSize()		;
	}
}

void FSkeletalMeshBinaryImport::CopyLODImportData( hkArray<FVector>& LODPoints, hkArray<FMeshWedge>& LODWedges, hkArray<FMeshFace>& LODFaces, hkArray<FVertInfluence>& LODInfluences )
{

	// Copy vertex data.
	LODPoints.clear();
	LODPoints.setSize( Points.getSize() );	
	for( INT p=0; p < Points.getSize(); p++ )
	{
		LODPoints[p] = Points[p];
	}

	// Copy wedge information to static LOD level.
	LODWedges.clear();
	LODWedges.setSize( Wedges.getSize() );
	for( INT w=0; w < Wedges.getSize(); w++ )
	{
		LODWedges[w].iVertex	= Wedges[w].VertexIndex;
		LODWedges[w].U		= Wedges[w].U;
		LODWedges[w].V		= Wedges[w].V;
	}

	// Copy triangle/ face data to static LOD level.
	LODFaces.clear();
	LODFaces.setSize( Faces.getSize() );
	for( INT f=0; f < Faces.getSize(); f++)
	{
		FMeshFace Face;
		Face.iWedge[0]			= Faces[f].WedgeIndex[0];
		Face.iWedge[1]			= Faces[f].WedgeIndex[1];
		Face.iWedge[2]			= Faces[f].WedgeIndex[2];
		Face.MeshMaterialIndex	= Faces[f].MatIndex;

		LODFaces[f] = Face;
	}			

	// Copy weights/ influences to static LOD level.
	LODInfluences.clear();
	LODInfluences.setSize( Influences.getSize() );
	for( INT i=0; i < Influences.getSize(); i++ )
	{
		LODInfluences[i].Weight		= Influences[i].Weight;
		LODInfluences[i].VertIndex	= Influences[i].VertexIndex;
		LODInfluences[i].BoneIndex	= Influences[i].BoneIndex;
	}
}


hkString FSkeletalMeshBinaryImport::FixupBoneName( char *AnisBoneName )
{
	return hkString(AnisBoneName);
}
bool PSABinaryImport::ImportFromFile( const TCHAR* Filename )
{
	
	FArchiveFileReaderWindows* animFile = 	CreateFile(Filename);				
	
	VChunkHeader ChunkHeader;		
	animFile->Serialize( &ChunkHeader, sizeof(VChunkHeader));
	animFile->Serialize( &ChunkHeader, sizeof(VChunkHeader) );
		

	INT NumPSATracks = ChunkHeader.DataCount; // Number of tracks of animation. One per bone.

	hkArray<FNamedBoneBinary> RawBoneNamesBin;
	RawBoneNamesBin.setSize(NumPSATracks);	
	animFile->Serialize( &RawBoneNamesBin[0], sizeof(FNamedBoneBinary) * ChunkHeader.DataCount);		
			
	hkArray<hkString> RawBoneNames;
	RawBoneNames.setSize(NumPSATracks);	
	for(INT i=0; i<NumPSATracks; i++)
	{
		FNamedBoneBinary& Bone = RawBoneNamesBin[i];
		hkString BoneName = FSkeletalMeshBinaryImport::FixupBoneName( Bone.Name );
		//RawBoneNames[i] = FName( *BoneName );
		RawBoneNames[i] = BoneName;
	}
		
	animFile->Serialize( &ChunkHeader, sizeof(VChunkHeader) );

	INT NumPSASequences = ChunkHeader.DataCount;
	hkArray<AnimInfoBinary> RawAnimSeqInfo; // Array containing per-sequence information (sequence name, key range etc)
	RawAnimSeqInfo.setSize(NumPSASequences);
	animFile->Serialize( &RawAnimSeqInfo[0], sizeof(AnimInfoBinary) * ChunkHeader.DataCount);

	animFile->Serialize( &ChunkHeader, sizeof(VChunkHeader) );

	
	hkArray<VQuatAnimKey> RawAnimKeys;
	RawAnimKeys.setSize(ChunkHeader.DataCount);
	animFile->Serialize( &RawAnimKeys[0], sizeof(VQuatAnimKey) * ChunkHeader.DataCount);	
	
	delete animFile;
	animFile = NULL;

	// Y-flip quaternions and translations from Max/Maya/etc space into Unreal space.
	for( INT i=0; i<RawAnimKeys.getSize(); i++)
	{
		noQuat Bone = RawAnimKeys[i].Orientation;
		Bone.w = - Bone.w;
		Bone.y = - Bone.y;
		RawAnimKeys[i].Orientation = Bone;

		FVector Pos = RawAnimKeys[i].Position;
		Pos.Y = - Pos.Y;
		RawAnimKeys[i].Position = Pos;
	}

		
	m_animSeqs.clear();
	m_animSeqs.resize(NumPSASequences);

	for(INT SeqIdx=0; SeqIdx<NumPSASequences; SeqIdx++)
	{
		AnimInfoBinary& PSASeqInfo = RawAnimSeqInfo[SeqIdx];
									
		noaAnimation* destSeq = new noaInterleavedAnim();
		destSeq->m_seqName = &PSASeqInfo.Name[0];
		destSeq->m_duration = PSASeqInfo.TrackTime / PSASeqInfo.AnimRate; // Time of animation if playback speed was 1.0;
		destSeq->m_numFrames = PSASeqInfo.NumRawFrames;

		destSeq->RawAnimData.setSize( NumPSATracks );

		// Structure of data is this:
		// RawAnimKeys contains all keys. 
		// Sequence info FirstRawFrame and NumRawFrames indicate full-skel frames (NumPSATracks raw keys). Ie number of elements we need to copy from RawAnimKeys is NumRawFrames * NumPSATracks.

		// Import each track.
		for(INT TrackIdx = 0; TrackIdx < NumPSATracks; TrackIdx++)
		{
			FRawAnimSequenceTrack& RawTrack = destSeq->RawAnimData[TrackIdx];
						
			RawTrack.PosKeys.setSize(destSeq->m_numFrames);
			RawTrack.RotKeys.setSize(destSeq->m_numFrames);
			RawTrack.KeyTimes.setSize(destSeq->m_numFrames);

			for(INT KeyIdx = 0; KeyIdx < destSeq->m_numFrames; KeyIdx++)
			{
				INT SrcKeyIdx = ((PSASeqInfo.FirstRawFrame + KeyIdx) * NumPSATracks);
				//check( RawAnimKeys.IsValidIndex(SrcKeyIdx) );

				VQuatAnimKey& RawSrcKey = RawAnimKeys[SrcKeyIdx];

				RawTrack.PosKeys[KeyIdx] = noVec3(RawSrcKey.Position.X,RawSrcKey.Position.Y, RawSrcKey.Position.Z);
				RawTrack.RotKeys[KeyIdx] = RawSrcKey.Orientation;
				RawTrack.KeyTimes[KeyIdx] = ((FLOAT)KeyIdx/(FLOAT)destSeq->m_numFrames) * destSeq->m_duration;
			}
		}			
	}	
	return true;
}

FArchiveFileReaderWindows* CreateFile(const TCHAR* Filename)
{
	INT StatsHandle = 0;
	DWORD  Access    = GENERIC_READ;
	DWORD  WinFlags  = FILE_SHARE_READ;
	DWORD  Create    = OPEN_EXISTING;
	HANDLE Handle    = CreateFileW( Filename, Access, WinFlags, NULL, Create, FILE_ATTRIBUTE_NORMAL, NULL );
	if( Handle==INVALID_HANDLE_VALUE )
	{		
		return NULL;
	}

	FArchiveFileReaderWindows* retArch = new FArchiveFileReaderWindows(Handle,StatsHandle,Filename,GetFileSize(Handle,NULL));
	return retArch;
}

FArchiveFileReaderWindows::FArchiveFileReaderWindows( HANDLE InHandle, INT InStatsHandle, const TCHAR* InFilename, INT InSize )
	:   Handle          ( InHandle )
	,	StatsHandle		( InStatsHandle )
	,	Filename		( InFilename )	
	,   Size            ( InSize )
	,   Pos             ( 0 )
	,   BufferBase      ( 0 )
	,   BufferCount     ( 0 )
	,	ArIsError		( 0 )
{
	
}

FArchiveFileReaderWindows::~FArchiveFileReaderWindows()
{	
	if( Handle )
	{
		Close();
	}
}

void FArchiveFileReaderWindows::Seek( INT InPos )
{
	ASSERT(InPos>=0);
	ASSERT(InPos<=Size);
	if( SetFilePointer( Handle, InPos, 0, FILE_BEGIN )==INVALID_SET_FILE_POINTER )
	{		
		//Error->Logf( TEXT("SetFilePointer Failed %i/%i: %i %s"), InPos, Size, Pos, appGetSystemErrorMessage() );
	}
	Pos         = InPos;
	BufferBase  = Pos;
	BufferCount = 0;

}

INT FArchiveFileReaderWindows::Tell()
{
	return Pos;
}

INT FArchiveFileReaderWindows::TotalSize()
{
	return Size;
}

UINT FArchiveFileReaderWindows::Close()
{
	if( Handle )
	{
		CloseHandle( Handle );
	}
	Handle = NULL;
	return 1;
}

void FArchiveFileReaderWindows::Serialize( void* V, INT Length )
{
	while( Length>0 )
	{
		INT Copy = min( Length, BufferBase+BufferCount-Pos );
		if( Copy<=0 )
		{
			if( Length >= ARRAY_COUNT(Buffer) )
			{
				INT Count=0;
				// Read data from device via Win32 ReadFile API.
				{					
					ReadFile( Handle, V, Length, (DWORD*)&Count, NULL );
				}
				if( Count!=Length )
				{
					ArIsError = 1;
					//Error->Logf( TEXT("ReadFile failed: Count=%i Length=%i Error=%s for file %s"), 
					//	Count, Length, appGetSystemErrorMessage(), *Filename );
				}
				Pos += Length;
				BufferBase += Length;
				return;
			}
			InternalPrecache( Pos, MAXINT );
			Copy = min( Length, BufferBase+BufferCount-Pos );
			if( Copy<=0 )
			{
				ArIsError = 1;
				//Error->Logf( TEXT("ReadFile beyond EOF %i+%i/%i for file %s"), 
				//	Pos, Length, Size, *Filename );
			}
			if( ArIsError )
			{
				return;
			}
		}
		memcpy( V, Buffer+Pos-BufferBase, Copy );
		Pos       += Copy;
		Length    -= Copy;
		V          = (BYTE*)V + Copy;
	}
}

UINT FArchiveFileReaderWindows::InternalPrecache( INT PrecacheOffset, INT PrecacheSize )
{
	// Only precache at current position and avoid work if precaching same offset twice.
	if( Pos == PrecacheOffset && (!BufferBase || !BufferCount || BufferBase != Pos) )
	{
		BufferBase = Pos;
		BufferCount = min( min( PrecacheSize, (INT)(ARRAY_COUNT(Buffer) - (Pos&(ARRAY_COUNT(Buffer)-1))) ), Size-Pos );
		BufferCount = max(BufferCount,0); // clamp to 0
		INT Count=0;

		// Read data from device via Win32 ReadFile API.
		{			
			ReadFile( Handle, Buffer, BufferCount, (DWORD*)&Count, NULL );
		}

		if( Count!=BufferCount )
		{			
			LOG_ERR << TEXT("ReadFile failed: Count" ) <<  Count << TEXT(" BufferCount= ") << BufferCount;
		}
	}
	return TRUE;
}