#pragma once 


enum EStats
{
	STAT_AnimFirstStat					= 2300,

};

/**
 * Below this weight threshold, animations won't be blended in.
 */
#define ZERO_ANIMWEIGHT_THRESH (0.00001f)  

/**
* Engine stats
*/
enum EAnimStats
{
	/** Skeletal stats */
	STAT_AnimBlendTime = STAT_AnimFirstStat,
	STAT_SkelComposeTime,
	STAT_UpdateSkelPose,
	STAT_AnimTickTime,
	STAT_AnimSyncGroupTime,
	STAT_SkelControlTickTime,
	STAT_SkelComponentTickTime,
	STAT_GetAnimationPose,
	STAT_MirrorBoneAtoms,
	STAT_UpdateFaceFX,
	STAT_SkelControl,
	STAT_UpdateChildComponents,
	STAT_BlendInPhysics,
	STAT_MeshObjectUpdate,
	STAT_SkelCompUpdateTransform,
	STAT_UpdateRBBones,
	STAT_UpdateSkelMeshBounds
};

/** Controls compilation of per-node GetBoneAtom stats. */
#define ENABLE_GETBONEATOM_STATS 0

#define START_GETBONEATOM_TIMER
#define EXCLUDE_CHILD_TIME

struct FBoneAtom
{
	FQuat	Rotation;
	FVector	Translation;
	FLOAT	Scale;

	static const FBoneAtom Identity;

	FBoneAtom() {};

	FBoneAtom(const FQuat& InRotation, const FVector& InTranslation, FLOAT InScale) : 
		Rotation(InRotation), 
		Translation(InTranslation), 
		Scale(InScale)
	{}

	/**
	 * Constructor for converting a Matrix into a bone atom. InMatrix should not contain any scaling info.
	 */
	FBoneAtom(const FMatrix& InMatrix)
		:	Translation( InMatrix.GetOrigin() )
		,	Scale( 1.f )
	{
		//Rotation = InMatrix.ToQuat();
	}

	/**
	 * Does a debugf of the contents of this BoneAtom.
	 */
	void DebugPrint() const;

	/**
	 * Convert this Atom to a transformation matrix.
	 */
	void ToTransform(FMatrix& OutMatrix) const
	{
		OutMatrix.mat[3][0] = Translation.x;
		OutMatrix.mat[3][1] = Translation.y;
		OutMatrix.mat[3][2] = Translation.z;

		const FLOAT x2 = Rotation.x + Rotation.x;	
		const FLOAT y2 = Rotation.y + Rotation.y;  
		const FLOAT z2 = Rotation.z + Rotation.z;
		{
			const FLOAT xx2 = Rotation.x * x2;
			const FLOAT yy2 = Rotation.y * y2;			
			const FLOAT zz2 = Rotation.z * z2;

			OutMatrix.mat[0][0] = (1.0f - (yy2 + zz2)) * Scale;	
			OutMatrix.mat[1][1] = (1.0f - (xx2 + zz2)) * Scale;
			OutMatrix.mat[2][2] = (1.0f - (xx2 + yy2)) * Scale;
		}
		{
			const FLOAT yz2 = Rotation.y * z2;   
			const FLOAT wx2 = Rotation.w * x2;	

			OutMatrix.mat[2][1] = (yz2 - wx2) * Scale;
			OutMatrix.mat[1][2] = (yz2 + wx2) * Scale;
		}
		{
			const FLOAT xy2 = Rotation.x * y2;
			const FLOAT wz2 = Rotation.w * z2;

			OutMatrix.mat[1][0] = (xy2 - wz2) * Scale;
			OutMatrix.mat[0][1] = (xy2 + wz2) * Scale;
		}
		{
			const FLOAT xz2 = Rotation.x * z2;
			const FLOAT wy2 = Rotation.w * y2;   

			OutMatrix.mat[2][0] = (xz2 + wy2) * Scale;
			OutMatrix.mat[0][2] = (xz2 - wy2) * Scale;
		}

		OutMatrix.mat[0][3] = 0.0f;
		OutMatrix.mat[1][3] = 0.0f;
		OutMatrix.mat[2][3] = 0.0f;
		OutMatrix.mat[3][3] = 1.0f;
	}

	/** Set this atom to the weighted blend of the supplied two atoms. */
	FORCEINLINE void Blend(const FBoneAtom& Atom1, const FBoneAtom& Atom2, FLOAT Alpha)
	{
		if( Alpha <= ZERO_ANIMWEIGHT_THRESH )
		{
			// if blend is all the way for child1, then just copy its bone atoms
			(*this) = Atom1;
		}
		else if( Alpha >= 1.f - ZERO_ANIMWEIGHT_THRESH )
		{
			// if blend is all the way for child2, then just copy its bone atoms
			(*this) = Atom2;
		}
		else
		{
			// Simple linear interpolation for translation and scale.
			Translation = Lerp(Atom1.Translation, Atom2.Translation, Alpha);
			Scale		= Lerp(Atom1.Scale, Atom2.Scale, Alpha);
			Rotation	= LerpQuat(Atom1.Rotation, Atom2.Rotation, Alpha);

			// ..and renormalize
			Rotation.Normalize();
		}
	}

	/**
	 * Quaternion addition is wrong here. This is just a special case for linear interpolation.
	 * Use only within blends!!
	 * Rotation part is NOT normalized!!
	 */
	FORCEINLINE FBoneAtom operator+(const FBoneAtom& Atom) const
	{
		return FBoneAtom(Rotation + Atom.Rotation, Translation + Atom.Translation, Scale + Atom.Scale);
	}

	/** 
	 * For quaternions, delta angles is done by multiplying the conjugate.
	 * Result is normalized.
	 */
	FORCEINLINE FBoneAtom operator-(const FBoneAtom& Atom) const
	{
		return FBoneAtom(Rotation * (-Atom.Rotation), Translation - Atom.Translation, Scale - Atom.Scale);
	}

	FORCEINLINE FBoneAtom& operator+=(const FBoneAtom& Atom)
	{
		Translation += Atom.Translation;

		Rotation.x += Atom.Rotation.x;
		Rotation.y += Atom.Rotation.y;
		Rotation.z += Atom.Rotation.z;
		Rotation.w += Atom.Rotation.w;

		Scale += Atom.Scale;

		return *this;
	}

	FORCEINLINE FBoneAtom& operator*=(FLOAT Mult)
	{
		Translation *= Mult;
		Rotation.x	*= Mult;
		Rotation.y	*= Mult;
		Rotation.z	*= Mult;
		Rotation.w	*= Mult;
		Scale		*= Mult;

		return *this;
	}

	FORCEINLINE FBoneAtom operator*(FLOAT Mult) const
	{
		return FBoneAtom(Rotation * Mult, Translation * Mult, Scale * Mult);
	}
};

template <> struct TIsPODType<FBoneAtom> { enum { Value = true }; };

inline FLOAT UnWindNormalizedAimAngleGT(FLOAT Angle)
{
	while( Angle > 2.f )
	{
		Angle -= 4.f;
	}

	while( Angle < -2.f )
	{
		Angle += 4.f;
	}

	return Angle;
}

inline FLOAT UnWindNormalizedAimAngleGE(FLOAT Angle)
{
	while( Angle >= 2.f )
	{
		Angle -= 4.f;
	}

	while( Angle < -2.f )
	{
		Angle += 4.f;
	}

	return Angle;
}


