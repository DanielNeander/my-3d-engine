#include "stdafx.h"
#include "AnimTree.h"
#include "AnimClasses.h"


#define USE_SLERP 0

/** Each CompresedTranslationData track's ByteStream will be byte swapped in chunks of this size. */
static const INT CompressedTranslationStrides[ACF_MAX] =
{
	sizeof(FLOAT),	// ACF_None					(FVectors are serialized per element hence sizeof(FLOAT) rather than sizeof(FVector).
	sizeof(FLOAT),	// ACF_Float96NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	sizeof(FLOAT),	// ACF_Fixed48NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	sizeof(FLOAT),	// ACF_IntervalFixed32NoW	(Translation data currently uncompressed, hence same size as ACF_None).
	sizeof(FLOAT),	// ACF_Fixed32NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	sizeof(FLOAT),	// ACF_Float32NoW			(Translation data currently uncompressed, hence same size as ACF_None).
};

/** Number of swapped chunks per element. */
static const INT CompressedTranslationNum[ACF_MAX] =
{
	3,	// ACF_None					(FVectors are serialized per element hence sizeof(FLOAT) rather than sizeof(FVector).
	3,	// ACF_Float96NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	3,	// ACF_Fixed48NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	3,	// ACF_IntervalFixed32NoW	(Translation data currently uncompressed, hence same size as ACF_None).
	3,	// ACF_Fixed32NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	3,	// ACF_Float32NoW			(Translation data currently uncompressed, hence same size as ACF_None).
};

/** Each CompresedRotationData track's ByteStream will be byte swapped in chunks of this size. */
static const INT CompressedRotationStrides[ACF_MAX] =
{
	sizeof(FLOAT),						// ACF_None					(FQuats are serialized per element hence sizeof(FLOAT) rather than sizeof(FQuat).
	sizeof(FLOAT),						// ACF_Float96NoW			(FQuats with one component dropped and the remaining three uncompressed at 32bit floating point each
	sizeof(WORD),						// ACF_Fixed48NoW			(FQuats with one component dropped and the remaining three compressed to 16-16-16 fixed point.
	//sizeof(FQuatIntervalFixed32NoW),	// ACF_IntervalFixed32NoW	(FQuats with one component dropped and the remaining three compressed to 11-11-10 per-component interval fixed point.
	//sizeof(FQuatFixed32NoW),			// ACF_Fixed32NoW			(FQuats with one component dropped and the remaining three compressed to 11-11-10 fixed point.
	//sizeof(FQuatFloat32NoW),			// ACF_Float32NoW			(FQuats with one component dropped and the remaining three compressed to 11-11-10 floating point.
};

/** Number of swapped chunks per element. */
static const INT CompressedRotationNum[ACF_MAX] =
{
	4,	// ACF_None					(FQuats are serialized per element hence sizeof(FLOAT) rather than sizeof(FQuat).
	3,	// ACF_Float96NoW			(FQuats with one component dropped and the remaining three uncompressed at 32bit floating point each
	3,	// ACF_Fixed48NoW			(FQuats with one component dropped and the remaining three compressed to 16-16-16 fixed point.
	1,	// ACF_IntervalFixed32NoW	(FQuats with one component dropped and the remaining three compressed to 11-11-10 per-component interval fixed point.
	1,	// ACF_Fixed32NoW			(FQuats with one component dropped and the remaining three compressed to 11-11-10 fixed point.
	1,  // ACF_Float32NoW			(FQuats with one component dropped and the remaining three compressed to 11-11-10 floating point.
};


void UAnimSequence::SeparateRawDataToTracks(const TArray<FRawAnimSequenceTrack>& RawAnimData,
	FLOAT SequenceLength,
	TArray<FTranslationTrack>& OutTranslationData,
	TArray<FRotationTrack>& OutRotationData)
{
	const INT NumTracks = RawAnimData.Num();

	OutTranslationData.Empty( NumTracks );
	OutRotationData.Empty( NumTracks );
	OutTranslationData.AddZeroed( NumTracks );
	OutRotationData.AddZeroed( NumTracks );

	for ( INT TrackIndex = 0 ; TrackIndex < NumTracks ; ++TrackIndex )
	{
		const FRawAnimSequenceTrack& RawTrack	= RawAnimData(TrackIndex);
		FTranslationTrack&	TranslationTrack	= OutTranslationData(TrackIndex);
		FRotationTrack&		RotationTrack		= OutRotationData(TrackIndex);

		const INT PrevNumPosKeys = RawTrack.PosKeys.Num();
		const INT PrevNumRotKeys = RawTrack.RotKeys.Num();

		// Do nothing if the data for this track is empty.
		if( PrevNumPosKeys == 0 || PrevNumRotKeys == 0 )
		{
			continue;
		}

		// Copy over position keys.
		for ( INT PosIndex = 0 ; PosIndex < RawTrack.PosKeys.Num() ; ++PosIndex )
		{
			TranslationTrack.PosKeys.AddItem( RawTrack.PosKeys(PosIndex) );
		}

		// Copy over rotation keys.
		for ( INT RotIndex = 0 ; RotIndex < RawTrack.RotKeys.Num() ; ++RotIndex )
		{
			RotationTrack.RotKeys.AddItem( RawTrack.RotKeys(RotIndex) );
		}

		// Set times for the translation track.
		if ( TranslationTrack.PosKeys.Num() > 1 )
		{
			const FLOAT PosFrameInterval = SequenceLength / static_cast<FLOAT>(TranslationTrack.PosKeys.Num()-1);
			for ( INT PosIndex = 0 ; PosIndex < TranslationTrack.PosKeys.Num() ; ++PosIndex )
			{
				TranslationTrack.Times.AddItem( PosIndex * PosFrameInterval );
			}
		}
		else
		{
			TranslationTrack.Times.AddItem( 0.f );
		}

		// Set times for the rotation track.
		if ( RotationTrack.RotKeys.Num() > 1 )
		{
			const FLOAT RotFrameInterval = SequenceLength / static_cast<FLOAT>(RotationTrack.RotKeys.Num()-1);
			for ( INT RotIndex = 0 ; RotIndex < RotationTrack.RotKeys.Num() ; ++RotIndex )
			{
				RotationTrack.Times.AddItem( RotIndex * RotFrameInterval );
			}
		}
		else
		{
			RotationTrack.Times.AddItem( 0.f );
		}

		// Trim unused memory.
		TranslationTrack.PosKeys.Shrink();
		TranslationTrack.Times.Shrink();
		RotationTrack.RotKeys.Shrink();
		RotationTrack.Times.Shrink();
	}
}

/**
 * Reconstructs a bone atom from key-reduced tracks.
 */
void UAnimSequence::ReconstructBoneAtom(FBoneAtom& OutAtom,
										const FTranslationTrack& TranslationTrack,
										const FRotationTrack& RotationTrack,
										FLOAT SequenceLength,
										FLOAT Time,
										UBOOL bLooping)
{
	OutAtom.Scale = 1.f;

	// Bail out (with rather wacky data) if data is empty for some reason.
	if( TranslationTrack.PosKeys.Num() == 0 || TranslationTrack.Times.Num() == 0 || 
		RotationTrack.RotKeys.Num() == 0 || RotationTrack.Times.Num() == 0 )
	{
		//debugf( TEXT("UAnimSequence::ReconstructBoneAtom(reduced) : No anim data in AnimSequence!") );
		OutAtom.Rotation = FQuat::Identity;
		OutAtom.Translation = FVector(0.f, 0.f, 0.f);
		return;
	}

	// Check for before-first-frame case.
	if( Time <= 0.f )
	{
		OutAtom.Translation = TranslationTrack.PosKeys(0);
		OutAtom.Rotation	= RotationTrack.RotKeys(0);
		return;
	}

	// Check for after-last-frame case.
	const INT LastPosIndex	= TranslationTrack.PosKeys.Num() - 1;
	const INT LastRotIndex	= RotationTrack.RotKeys.Num() - 1;
	if( Time >= SequenceLength )
	{
		OutAtom.Translation = TranslationTrack.PosKeys(LastPosIndex);
		OutAtom.Rotation	= RotationTrack.RotKeys(LastRotIndex);
		return;
	}

	// Find the "starting" key indices.
	const INT PosIndex0 = FindKeyIndex( Time, TranslationTrack.Times );
	const INT RotIndex0 = FindKeyIndex( Time, RotationTrack.Times );

	///////////////////////
	// Translation.

	INT PosIndex1;
	FLOAT PosAlpha;

	// If we have gone over the end, do different things in case of looping.
	if ( PosIndex0 == LastPosIndex )
	{
		// If looping, interpolate between last and first frame.
		if( bLooping )
		{
			PosIndex1 = 0;
			// @todo DB: handle looping with variable-length keys.
			PosAlpha = 0.5f;
		}
		// If not looping - hold the last frame.
		else
		{
			PosIndex1 = PosIndex0;
			PosAlpha = 1.f;
		}
	}
	else
	{
		// Find the "ending" key index and alpha.
		PosIndex1 = PosIndex0+1;
		const FLOAT DeltaTime = TranslationTrack.Times(PosIndex1) - TranslationTrack.Times(PosIndex0);
		PosAlpha = (Time - TranslationTrack.Times(PosIndex0))/DeltaTime;
	}

	OutAtom.Translation = Lerp( TranslationTrack.PosKeys(PosIndex0), TranslationTrack.PosKeys(PosIndex1), PosAlpha );

	///////////////////////
	// Rotation.

	INT RotIndex1;
	FLOAT RotAlpha;

	// If we have gone over the end, do different things in case of looping.
	if ( RotIndex0 == LastRotIndex )
	{
		// If looping, interpolate between last and first frame.
		if( bLooping )
		{
			RotIndex1 = 0;
			 // @todo DB: handle looping with variable-length keys.
			RotAlpha = 0.5f;
		}
		// If not looping - hold the last frame.
		else
		{
			RotIndex1 = RotIndex0;
			RotAlpha = 1.f;
		}
	}
	else
	{
		// Find the "ending" key index and alpha.
		RotIndex1 = RotIndex0+1;
		const FLOAT DeltaTime = RotationTrack.Times(RotIndex1) - RotationTrack.Times(RotIndex0);
		RotAlpha = (Time - RotationTrack.Times(RotIndex0))/DeltaTime;
	}

#if !USE_SLERP
	// Fast linear quaternion interpolation.
	// To ensure the 'shortest route', we make sure the dot product between the two keys is positive.
	if( (RotationTrack.RotKeys(RotIndex0) | RotationTrack.RotKeys(RotIndex1)) < 0.f )
	{
		// To clarify the code here: a slight optimization of inverting the parametric variable as opposed to the quaternion.
		OutAtom.Rotation = (RotationTrack.RotKeys(RotIndex0) * (1.f-RotAlpha)) + (RotationTrack.RotKeys(RotIndex1) * -RotAlpha);
	}
	else
	{
		OutAtom.Rotation = (RotationTrack.RotKeys(RotIndex0) * (1.f-RotAlpha)) + (RotationTrack.RotKeys(RotIndex1) * RotAlpha);
	}
#else
	OutAtom.Rotation = SlerpQuat( RotationTrack.RotKeys(RotIndex0), RotationTrack.RotKeys(RotIndex1), RotAlpha );
#endif

	OutAtom.Rotation.Normalize();
}
