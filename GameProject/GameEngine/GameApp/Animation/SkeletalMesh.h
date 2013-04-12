#pragma once 

#include "../GameObjectComponent.h"
#include "../GameObject.h"

/** Struct used to indicate one active morph target that should be applied to this SkeletalMesh when rendered. */
struct FActiveMorph
{
	/** The morph target that we want to apply. */
	class UMorphTarget*	Target;

	/** Strength of the morph target, between 0.0 and 1.0 */
	FLOAT				Weight;

	FActiveMorph(class UMorphTarget* InTarget, FLOAT InWeight):
	Target(InTarget),
		Weight(InWeight)
	{}
	FActiveMorph() {}
};


struct FMeshWedge
{
	WORD			iVertex;		// Vertex index.
	FLOAT			U,V;			// UVs.
	friend FArchive &operator<<( FArchive& Ar, FMeshWedge& T )
	{
		Ar << T.iVertex << T.U << T.V;
		return Ar;
	}
};
template <> struct TIsPODType<FMeshWedge> { enum { Value = true }; };

struct FMeshFace
{
	WORD		iWedge[3];			// Textured Vertex indices.
	WORD		MeshMaterialIndex;	// Source Material (= texture plus unique flags) index.

	/*friend FArchive &operator<<( FArchive& Ar, FMeshFace& F )
	{
		Ar << F.iWedge[0] << F.iWedge[1] << F.iWedge[2];
		Ar << F.MeshMaterialIndex;
		return Ar;
	}*/
};
template <> struct TIsPODType<FMeshFace> { enum { Value = true }; };

// A bone: an orientation, and a position, all relative to their parent.
struct VJointPos
{
	FQuat   	Orientation;  //
	FVector		Position;     //  needed or not ?

	FLOAT       Length;       //  For collision testing / debugging drawing...
	FLOAT       XSize;
	FLOAT       YSize;
	FLOAT       ZSize;

	/*friend FArchive &operator<<( FArchive& Ar, VJointPos& V )
	{
		return Ar << V.Orientation << V.Position;
	}*/
};
template <> struct TIsPODType<VJointPos> { enum { Value = true }; };


// Reference-skeleton bone, the package-serializable version.
struct FMeshBone
{
	FName 		Name;		  // Bone's name.
	DWORD		Flags;        // reserved
	VJointPos	BonePos;      // reference position
	INT         ParentIndex;  // 0/NULL if this is the root bone.  
	INT 		NumChildren;  // children  // only needed in animation ?
	INT         Depth;        // Number of steps to root in the skeletal hierarcy; root=0.

	// DEBUG rendering
	FColor		BoneColor;		// Color to use when drawing bone on screen.

	UBOOL operator==( const FMeshBone& B ) const
	{
		return( Name == B.Name );
	}

	/*friend FArchive &operator<<( FArchive& Ar, FMeshBone& F)
	{
		Ar << F.Name << F.Flags << F.BonePos << F.NumChildren << F.ParentIndex;

		if( Ar.IsLoading() && Ar.Ver() < VER_SKELMESH_DRAWSKELTREEMANAGER )
		{
			F.BoneColor = FColor(255, 255, 255, 255);
		}
		else
		{
			Ar << F.BoneColor;
		}

		return Ar;
	}*/
};
template <> struct TIsPODType<FMeshBone> { enum { Value = true }; };

struct FBoneVertInfo
{
	// Invariant: Arrays should be same length!
	TArray<FVector>	Positions;
	TArray<FVector>	Normals;
};

/** Struct containing information for a particular LOD level, such as materials and info for when to use it. */
struct FSkeletalMeshLODInfo
{
	/**	Indicates when to use this LOD. A smaller number means use this LOD when further away. */
	FLOAT								DisplayFactor;

	/**	Used to avoid 'flickering' when on LOD boundary. Only taken into account when moving from complex->simple. */
	FLOAT								LODHysteresis;

	/** Mapping table from this LOD's materials to the SkeletalMesh materials array. */
	TArray<INT>							LODMaterialMap;

	/** Per-section control over whether to enable shadow casting. */
	TArray<UBOOL>						bEnableShadowCasting;
};

struct FBoneMirrorInfo
{
	/** The bone to mirror. */
	INT		SourceIndex;
	/** Axis the bone is mirrored across. */
	BYTE	BoneFlipAxis;
};
template <> struct TIsPODType<FBoneMirrorInfo> { enum { Value = true }; };


class USkeletalMesh
{
public:
	TArray<FMeshBone>		RefSkeleton;
	INT MatchRefBone(FName StartBoneName) const;

	/** Map from bone name to bone index. Used to accelerate MatchRefBone. */
	std::map<FString,INT>					NameIndexMap;
};



class  USkeletalMeshComponent : public GameObjectComponentBase
{

public:
	USkeletalMesh*						SkeletalMesh;

	USkeletalMeshComponent*				AttachedToSkelComponent;

	class UAnimTree*					AnimTreeTemplate;
	class noAnimNode*					Animations;

	/** Array of all AnimNodes in entire tree, in the order they should be ticked - that is, all parents appear before a child. */
	TArray<noAnimNode*>					AnimTickArray;

	/** Used to scale speed of all animations on this skeletal mesh. */
	FLOAT								GlobalAnimRateScale;


	/** Used to avoid ticking nodes in the tree multiple times. Node will only be ticked if TickTag != NodeTickTag. */
	INT									TickTag;

	/** 
	 *	Used to avoid duplicating work when calling GetBoneAtom. 
	 *	If this is equal to a nodes NodeCachedAtomsTag, cache is up-to-date and can be used. 
	 */
	INT									CachedAtomsTag;

	/** Reference skeleton precomputed bases. */
	TArray<FMatrix>					RefBasesInvMatrix;	// @todo: wasteful ?!
	/** List of bones that should be mirrored. */
	TArray<FBoneMirrorInfo>			SkelMirrorTable;

	// Editor/debugging rendering mode flags.

	/** Force drawing of a specific lodmodel -1 if > 0. */
	INT									ForcedLodModel;
	/** 
	 * This is the min LOD that this component will use.  (e.g. if set to 2 then only 2+ LOD Models will be used.) This is useful to set on
	 * meshes which are known to be a certain distance away and still want to have better LODs when zoomed in on them.
	 **/
	INT									MinLodModel;
	/** 
	 *	Best LOD that was 'predicted' by UpdateSkelPose. 
	 *	This is what bones were updated based on, so we do not allow rendering at a better LOD than this. 
	 */
	INT									PredictedLODLevel;

	/** LOD level from previous frame, so we can detect changes in LOD to recalc required bones. */
	INT									OldPredictedLODLevel;

	/**	High (best) DistanceFactor that was desired for rendering this SkeletalMesh last frame. Represents how big this mesh was in screen space   */
	FLOAT								MaxDistanceFactor;

	/** Forces the mesh to draw in wireframe mode. */
	UBOOL								bForceWireframe;

	/** If true, force the mesh into the reference pose - is an optimisation. */
	UBOOL								bForceRefpose;
	

	/** TRUE if mesh has been recently rendered, FALSE otherwise */
	BITFIELD bRecentlyRendered:1;


	BITFIELD bCacheAnimSequenceNodes:1;
	BITFIELD bForceMeshObjectUpdates:1;

	/** If true, will move the Actors Location to match the root rigid body location when in PHYS_RigidBody. */
	BITFIELD bSyncActorLocationToRootRigidBody:1;

	/** If TRUE, force usage of raw animation data when animating this skeltal mesh; if FALSE, use compressed data. */
	BITFIELD bUseRawData:1;

	/** Disable warning when an AnimSequence is not found. FALSE by default. */
	BITFIELD bDisableWarningWhenAnimNotFound:1;

	/** if set, components that are attached to us have their bOwnerNoSee and bOnlyOwnerSee properties overridden by ours */
	BITFIELD bOverrideAttachmentOwnerVisibility:1;

	/** pauses animations (doesn't tick them) */
	BITFIELD bPauseAnims:1;

public:
	USkeletalMeshComponent(GameObject *pkGameObject);
	~USkeletalMeshComponent();
	void BuildComposePriorityList(TArray<BYTE>& PriorityList);
	INT MatchRefBone( FName BoneName) const;
	class UAnimSequence* FindAnimSequence(FName AnimSeqName);

};
