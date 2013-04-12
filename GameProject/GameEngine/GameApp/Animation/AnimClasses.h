#pragma once 
#pragma pack (push,4)

#include <EngineCore/Unreal3/Array.h>
#include "AnimTree.h"

enum AnimBlendType
{
	ABT_Linear              =0,
	ABT_Cubic               =1,
	ABT_Sinusoidal          =2,
	ABT_EaseInOutExponent2  =3,
	ABT_EaseInOutExponent3  =4,
	ABT_EaseInOutExponent4  =5,
	ABT_EaseInOutExponent5  =6,
	ABT_MAX                 =7,
};

enum EAimID
{
	EAID_LeftUp             =0,
	EAID_LeftDown           =1,
	EAID_RightUp            =2,
	EAID_RightDown          =3,
	EAID_ZeroUp             =4,
	EAID_ZeroDown           =5,
	EAID_ZeroLeft           =6,
	EAID_ZeroRight          =7,
	EAID_CellLU             =8,
	EAID_CellCU             =9,
	EAID_CellRU             =10,
	EAID_CellLC             =11,
	EAID_CellCC             =12,
	EAID_CellRC             =13,
	EAID_CellLD             =14,
	EAID_CellCD             =15,
	EAID_CellRD             =16,
	EAID_MAX                =17,
};

enum EAnimAimDir
{
	ANIMAIM_LEFTUP          =0,
	ANIMAIM_CENTERUP        =1,
	ANIMAIM_RIGHTUP         =2,
	ANIMAIM_LEFTCENTER      =3,
	ANIMAIM_CENTERCENTER    =4,
	ANIMAIM_RIGHTCENTER     =5,
	ANIMAIM_LEFTDOWN        =6,
	ANIMAIM_CENTERDOWN      =7,
	ANIMAIM_RIGHTDOWN       =8,
	ANIMAIM_MAX             =9,
};
enum ERootRotationOption
{
	RRO_Default             =0,
	RRO_Discard             =1,
	RRO_Extract             =2,
	RRO_MAX                 =3,
};

enum ERootBoneAxis
{
	RBA_Default             =0,
	RBA_Discard             =1,
	RBA_Translate           =2,
	RBA_MAX                 =3,
};
enum EBaseBlendType
{
	BBT_ByActorTag          =0,
	BBT_ByActorClass        =1,
	BBT_MAX                 =2,
};

enum AnimationCompressionFormat
{
	ACF_None                =0,
	ACF_Float96NoW          =1,
	ACF_Fixed48NoW          =2,
	ACF_IntervalFixed32NoW  =3,
	ACF_Fixed32NoW          =4,
	ACF_Float32NoW          =5,
	ACF_MAX                 =6,
};
enum EBoneControlSpace
{
	BCS_WorldSpace          =0,
	BCS_ActorSpace          =1,
	BCS_ComponentSpace      =2,
	BCS_ParentBoneSpace     =3,
	BCS_BoneSpace           =4,
	BCS_OtherBoneSpace      =5,
	BCS_MAX                 =6,
};

enum ESplineControlRotMode
{
	SCR_NoChange            =0,
	SCR_AlongSpline         =1,
	SCR_Interpolate         =2,
	SCR_MAX                 =3,
};

struct FAnimSlotInfo
{
	FName SlotName;
	TArray<FLOAT> ChannelWeights;

	/** Constructors */
	FAnimSlotInfo()
		: SlotName(NAME_None)
	{}
	FAnimSlotInfo(EEventParm)
	{
		appMemzero(this, sizeof(FAnimSlotInfo));
	}
};

struct FAnimSlotDesc
{
	FName SlotName;
	INT NumChannels;

	/** Constructors */
	FAnimSlotDesc()
		: SlotName(NAME_None)
		, NumChannels(0)
	{}
	FAnimSlotDesc(EEventParm)
	{
		appMemzero(this, sizeof(FAnimSlotDesc));
	}
};


class noAnimNode 
{
public:
	BITFIELD bRelevant:1;
	BITFIELD bJustBecameRelevant:1;
	BITFIELD bSkipTickWhenZeroWeight:1;
	BITFIELD bTickDuringPausedAnims:1;
	INT NodeTickTag;
	INT NodeCachedAtomsTag;
	FLOAT NodeTotalWeight;
	FLOAT TotalWeightAccumulator;
	class USkeletalMeshComponent* SkelComponent;
	TArrayNoInit<class noAnimNodeBlendBase*> ParentNodes;
	FName NodeName;
	TArrayNoInit<FBoneAtom> CachedBoneAtoms;
	FBoneAtom CachedRootMotionDelta;
	INT bCachedHasRootMotion;
	INT DrawWidth;
	INT DrawHeight;
	INT NodePosX;
	INT NodePosY;
	INT OutDrawY;
	INT InstanceVersionNumber;
//protected:
	INT SearchTag;


	class noAnimNode* FindAnimNode(FName InNodeName);	
	virtual void PlayAnim(UBOOL bLoop=FALSE,FLOAT Rate=1.000000,FLOAT StartTime=0.000000);
	virtual void StopAnim();

	virtual void InitAnim( class USkeletalMeshComponent* meshComp, class noAnimNodeBlendBase* Parent );

	/** AnimSets have been updated, update all animations */
	virtual void AnimSetsUpdated() {}

	/**
	 *	Update this node, then call TickAnim on all children.
	 *	@param DeltaSeconds		Amount of time to advance this node.
	 *	@param TotalWeight		The eventual weight that this node will have in the final blend. This is the multiplication of weights of all nodes above this one.
	 */
	virtual	void	TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight ) {}

	/** Parent node is requesting a blend out. Give node a chance to delay that. */
	virtual UBOOL	CanBlendOutFrom() { return TRUE; }

	/** parent node is requesting a blend in. Give node a chance to delay that. */
	virtual UBOOL	CanBlendTo() { return TRUE; }

	/** Add this node and all children to array. Node are added so a parent is always before its children in the array. */
	void GetNodes(TArray<noAnimNode*>& Nodes);

	/** Return an array with all noAnimNodeSequence childs, including this node. */
	void GetAnimSeqNodes(TArray<class noAnimNodeSequence*>& Nodes, FName InSynchGroupName=NAME_None);

	/** Used for building array of AnimNodes in 'tick' order - that is, all parents of a node are added to array before it. */
	virtual void BuildTickArray(TArray<noAnimNode*>& OutTickArray) {}

	virtual void GetBoneAtoms(TArray<FBoneAtom>& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);

	/** Save the supplied array of BoneAtoms in the CachedBoneAtoms. */
	virtual UBOOL ShouldSaveCachedResults();
	void SaveCachedResults(const TArray<FBoneAtom>& NewAtoms, const FBoneAtom& NewRootMotionDelta, INT bNewHasRootMotion);

	virtual void SetAnim( FName SequenceName ) {}
	virtual void SetPosition( FLOAT NewTime, UBOOL bFireNotifies ) {}

	// STATIC ANIMTREE UTILS

	/** flag to prevent calling GetNodesInternal() from anywhere besides GetNodes() or another GetNodesInternal(), since
	 * we can't make it private/protected because noAnimNodeBlendBase needs to be able to call it on its children
	 */
	static UBOOL bNodeSearching;
	/** current tag value used for SearchTag on nodes being iterated over. Incremented every time a new search is started */
	static INT CurrentSearchTag;

	/* Fills the Atoms array with the specified skeletal mesh reference pose.
	 *
	 * @param Atoms				[out] Output array of relative bone transforms. Must be the same length as RefSkel when calling function.
	 * @param DesiredBones		Indices of bones we want to modify. Parents must occur before children.
	 * @param RefSkel			Input reference skeleton to create atoms from.
	 */
	static void FillWithRefPose(TArray<FBoneAtom>& Atoms, const TArray<BYTE>& DesiredBones, const TArray<struct FMeshBone>& RefSkel);
	UBOOL GetCachedResults(TArray<FBoneAtom>& OutAtoms, FBoneAtom& OutRootMotionDelta, INT& bOutHasRootMotion);
	INT CountNumParentsTicked();
	UBOOL IsChildOf(noAnimNode* Node);
	void GetNodesInternal(TArray<noAnimNode*>& Nodes);
	void EnsureParentsPresent(TArray<BYTE>& BoneIndices, class USkeletalMesh* SkelMesh);
};

struct FAnimBlendChild
{
	FName Name;
	class noAnimNode* Anim;
	FLOAT Weight;
	FLOAT TotalWeight;
	FLOAT BlendWeight;
	INT bHasRootMotion;
	FBoneAtom RootMotion;
	BITFIELD bMirrorSkeleton:1;
	BITFIELD bIsAdditive:1;
	INT DrawY;

	/** Constructors */
	FAnimBlendChild() {}
	FAnimBlendChild(EEventParm)
	{
		appMemzero(this, sizeof(FAnimBlendChild));
	}
};

class noAnimNodeBlendBase : public noAnimNode
{
public:
	TArrayNoInit<struct FAnimBlendChild> Children;
	BITFIELD bFixNumChildren:1;
	BYTE BlendType;
		
	virtual void PlayAnim(UBOOL bLoop = FALSE, FLOAT Rate=1.000000,FLOAT StartTime=0.000000);
	virtual void StopAnim();

	virtual void InitAnim( class USkeletalMeshComponent* meshComp, noAnimNodeBlendBase* Parent );

	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );

	virtual void BuildTickArray(TArray<noAnimNode*>& OutTickArray);

	FORCEINLINE FLOAT	GetBlendWeight(FLOAT ChildWeight);
	FORCEINLINE void	SetBlendTypeWeights();
	virtual void GetBoneAtoms(TArray<FBoneAtom>& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);

		/**
	 * Get mirrored bone atoms from desired child index.
	 * Bones are mirrored using the SkelMirrorTable.
	 */
	void GetMirroredBoneAtoms(TArray<FBoneAtom>& Atoms, INT ChildIndex, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);

	/** For debugging. Return the sum of the weights of all children nodes. Should always be 1.0. */
	FLOAT GetChildWeightTotal();

	/** Notification to this blend that a child noAnimNodeSequence has reached the end and stopped playing. Not called if child has bLooping set to true or if user calls StopAnim. */
	virtual void OnChildAnimEnd(noAnimNodeSequence* Child, FLOAT PlayedTime, FLOAT ExcessTime);

	/** A child connector has been added */
	virtual void	OnAddChild(INT ChildNum);
	/** A child connector has been removed */
	virtual void	OnRemoveChild(INT ChildNum);

	/** Rename all child nodes upon Add/Remove, so they match their position in the array. */
	virtual void	RenameChildConnectors();

	/** internal code for GetNodes(); should only be called from GetNodes() or from the GetNodesInternal() of this node's parent */
	virtual void GetNodesInternal(TArray<noAnimNode*>& Nodes);
	void SetChildrenTotalWeightAccumulator(const INT Index);
	void OnCeaseRelevant();
};

struct FAimTransform
{
	noQuat Quaternion;
	noVec3 Translation;

	/** Constructors */
	FAimTransform() {}
	*FAimTransform(EEventParm)
	{
		appMemzero(this, sizeof(FAimTransform));
	}
};

struct FAimComponent
{
	FName BoneName;
	struct FAimTransform LU;
	struct FAimTransform LC;
	struct FAimTransform LD;
	struct FAimTransform CU;
	struct FAimTransform CC;
	struct FAimTransform CD;
	struct FAimTransform RU;
	struct FAimTransform RC;
	struct FAimTransform RD;

	/** Constructors */
	FAimComponent() {}
	FAimComponent(EEventParm)
	{
		appMemzero(this, sizeof(FAimComponent));
	}
};

struct FAimOffsetProfile
{
	FName ProfileName;
	noVec2 HorizontalRange;
	noVec2 VerticalRange;
	TArrayNoInit<struct FAimComponent> AimComponents;
	FName AnimName_LU;
	FName AnimName_LC;
	FName AnimName_LD;
	FName AnimName_CU;
	FName AnimName_CC;
	FName AnimName_CD;
	FName AnimName_RU;
	FName AnimName_RC;
	FName AnimName_RD;

	/** Constructors */
	FAimOffsetProfile() {}
	FAimOffsetProfile(EEventParm)
	{
		appMemzero(this, sizeof(FAimOffsetProfile));
	}
};


class noAnimNodeAimOffset : public noAnimNodeBlendBase
{
public:
	//## BEGIN PROPS AnimNodeAimOffset
	noVec2 Aim;
	noVec2 AngleOffset;
	BITFIELD bForceAimDir:1;
	BITFIELD bBakeFromAnimations:1;
	BITFIELD bPassThroughWhenNotRendered:1;
	INT PassThroughAtOrAboveLOD;
	BYTE ForcedAimDir;
	TArrayNoInit<BYTE> RequiredBones;
	TArrayNoInit<INT> BoneToAimCpnt;
	class noAnimNodeAimOffset* TemplateNode;
	TArrayNoInit<struct FAimOffsetProfile> Profiles;
	INT CurrentProfileIndex;
	//## END PROPS AnimNodeAimOffset
	virtual void		InitAnim(USkeletalMeshComponent* meshComp, noAnimNodeBlendBase* Parent);

	/** returns current aim. Override this to pull information from somewhere else, like Pawn actor for example. */
	virtual FVector2D	GetAim() { return Aim; }

	virtual void		GetBoneAtoms(TArray<FBoneAtom>& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);

	/** 
	 * Function called after Aim has been extracted and processed (offsets, range, clamping...).
	 * Gives a chance to PostProcess it before being used by the AimOffset Node.
	 * Note that X,Y range should remain [-1,+1].
	 */
	virtual void		PostAimProcessing(FVector2D &AimOffsetPct) {}

	/** Util for getting the current AimOffsetProfile. */
	FAimOffsetProfile*	GetCurrentProfile();

	/** Update cached list of required bones, use to transform skeleton from parent space to component space. */
	void				UpdateListOfRequiredBones();

	/** Returns TRUE if AimComponents contains specified bone */
	UBOOL				ContainsBone(const FName &BoneName);

	/** Util for grabbing the quaternion on a specific bone in a specific direction. */
	FQuat				GetBoneAimQuaternion(INT CompIndex, EAnimAimDir InAimDir);
	/** Util for grabbing the translation on a specific bone in a specific direction. */
	FVector				GetBoneAimTranslation(INT CompIndex, EAnimAimDir InAimDir);

	/** Util for setting the quaternion on a specific bone in a specific direction. */
	void				SetBoneAimQuaternion(INT CompIndex, EAnimAimDir InAimDir, FQuat InQuat);
	/** Util for setting the translation on a specific bone in a specific direction. */
	void				SetBoneAimTranslation(INT CompIndex, EAnimAimDir InAimDir, FVector InTrans);

	/** Bake in Offsets from supplied Animations. */
	void				BakeOffsetsFromAnimations();
	void				ExtractOffsets(TArray<FBoneAtom>& RefBoneAtoms, TArray<FBoneAtom>& BoneAtoms, EAnimAimDir InAimDir);
	INT					GetComponentIdxFromBoneIdx(const INT BoneIndex, UBOOL bCreateIfNotFound=0);
	/**
	 * Extract Parent Space Bone Atoms from Animation Data specified by Name.
	 * Returns TRUE if successful.
	 */
	UBOOL				ExtractAnimationData(noAnimNodeSequence *SeqNode, FName AnimationName, TArray<FBoneAtom>& BoneAtoms);
	void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
	void PostAnimNodeInstance(noAnimNode* SourceNode);
	FLOAT GetSliderPosition(INT SliderIndex, INT ValueIndex);
	FString GetSliderDrawValue(INT SliderIndex);
	void SetActiveProfileByName(FName ProfileName);
	void SetActiveProfileByIndex(INT ProfileIndex);
};

class noAnimNodeBlend : public noAnimNodeBlendBase
{
public:
	FLOAT Child2Weight;
	FLOAT Child2WeightTarget;
	FLOAT BlendTimeToGo;
	BITFIELD bSkipBlendWhenNotRendered:1;

	void SetBlendTarget(FLOAT BlendTarget,FLOAT BlendTime);
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
};


class noAnimNodeAdditiveBlending : public noAnimNodeBlend
{
public:
	//## BEGIN PROPS AnimNodeAdditiveBlending
	BITFIELD bPassThroughWhenNotRendered:1 GCC_BITFIELD_MAGIC;
	//## END PROPS AnimNodeAdditiveBlending

	//DECLARE_CLASS(noAnimNodeAdditiveBlending,noAnimNodeBlend,0,Engine)
	void GetChildAtoms(INT ChildIndex, TArray<FBoneAtom>& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);
	virtual void GetBoneAtoms(TArray<FBoneAtom>& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);
	virtual void SetChildrenTotalWeightAccumulator(const INT Index);
};

class noAnimNodeBlendPerBone : public noAnimNodeBlend
{
public:
	//## BEGIN PROPS AnimNodeBlendPerBone
	BITFIELD bForceLocalSpaceBlend:1;
	TArrayNoInit<FName> BranchStartBoneName;
	TArrayNoInit<FLOAT> Child2PerBoneWeight;
	TArrayNoInit<BYTE> LocalToCompReqBones;
	void BuildWeightList();
	void SetChildrenTotalWeightAccumulator(const INT Index);
	void InitAnim(USkeletalMeshComponent* meshComp, noAnimNodeBlendBase* Parent);
	void GetBoneAtoms(TArray<FBoneAtom>& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);
	//## END PROPS AnimNodeBlendPerBone
};

class noAnimNodeCrossfader : public noAnimNodeBlend
{
public:
	//## BEGIN PROPS AnimNodeCrossfader
	FName DefaultAnimSeqName;
	BITFIELD bDontBlendOutOneShot:1;
	FLOAT PendingBlendOutTimeOneShot;
	//## END PROPS AnimNodeCrossfader

	FName GetAnimName();
	class noAnimNodeSequence* GetActiveChild();

	// noAnimNode interface
	virtual	void InitAnim( USkeletalMeshComponent* meshComp, noAnimNodeBlendBase* Parent );
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
};

class noAnimNodeSequence : public noAnimNode
{
public:
	//## BEGIN PROPS AnimNodeSequence
	FName AnimSeqName;
	FLOAT Rate;
	BITFIELD bPlaying:1;
	BITFIELD bLooping:1;
	BITFIELD bCauseActorAnimEnd:1;
	BITFIELD bCauseActorAnimPlay:1;
	BITFIELD bZeroRootRotation:1;
	BITFIELD bZeroRootTranslation:1;
	BITFIELD bDisableWarningWhenAnimNotFound:1;
	BITFIELD bNoNotifies:1;
	BITFIELD bForceRefposeWhenNotPlaying:1;
	BITFIELD bIsIssuingNotifies:1;
	BITFIELD bForceAlwaysSlave:1;
	BITFIELD bSynchronize:1;
	BITFIELD bShowTimeLineSlider:1;
	BITFIELD bLoopCameraAnim:1;
	BITFIELD bRandomizeCameraAnimLoopStartTime:1;
	BITFIELD bEditorOnlyAddRefPoseToAdditiveAnimation:1;
	FLOAT CurrentTime;
	FLOAT PreviousTime;
	class UAnimSequence* AnimSeq;
	INT AnimLinkupIndex;
	FLOAT NotifyWeightThreshold;
	FName SynchGroupName;
	FLOAT SynchPosOffset;
	//class UTexture2D* DebugTrack;
	//class UTexture2D* DebugCarat;
	//class UCameraAnim* CameraAnim;
	//class UCameraAnimInst* ActiveCameraAnimInstance;
	FLOAT CameraAnimScale;
	FLOAT CameraAnimPlayRate;
	BYTE RootBoneOption[3];
	BYTE RootRotationOption[3];

	virtual void SetAnim(FName Sequence);
	virtual void PlayAnim(UBOOL bLoop=FALSE,FLOAT InRate=1.000000,FLOAT StartTime=0.000000);
	virtual void StopAnim();
	virtual void SetPosition(FLOAT NewTime,UBOOL bFireNotifies);
	virtual FLOAT GetNormalizedPosition();
	virtual FLOAT FindNormalizedPositionFromGroupRelativePosition(FLOAT GroupRelativePosition);
	virtual FLOAT GetGlobalPlayRate();
	virtual FLOAT GetAnimPlaybackLength();
	virtual FLOAT GetTimeLeft();

	// AnimNode interface
	virtual void InitAnim( USkeletalMeshComponent* meshComp, noAnimNodeBlendBase* Parent );
	virtual UBOOL GetCachedResults(TArray<FBoneAtom>& OutAtoms, FBoneAtom& OutRootMotionDelta, INT& bOutHasRootMotion);
	virtual UBOOL ShouldSaveCachedResults();
	virtual UBOOL ShouldSkipWhenMeshNotRendered();

	/** AnimSets have been updated, update all animations */
	virtual void AnimSetsUpdated();

	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );	 // Progress the animation state, issue AnimEnd notifies.
	virtual void GetBoneAtoms(TArray<FBoneAtom>& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);

	// AnimNodeSequence interface
	void GetAnimationPose(UAnimSequence* InAnimSeq, INT& InAnimLinkupIndex, TArray<FBoneAtom>& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);
	// Extract root motion fro animation.
	void ExtractRootMotion(UAnimSequence* InAnimSeq, const INT &TrackIndex, FBoneAtom& RootBoneAtom, FBoneAtom& RootBoneAtomDeltaMotion, INT& bHasRootMotion);

	/** Advance animation time. Take care of issuing notifies, looping and so on */
	void AdvanceBy(FLOAT MoveDelta, FLOAT DeltaSeconds, UBOOL bFireNotifies);

	/** Issue any notifies tha are passed when moving from the current position to DeltaSeconds in the future. Called from TickAnim. */
	void IssueNotifies(FLOAT DeltaSeconds);

		/**
	 * notification that current animation finished playing.
	 * @param	PlayedTime	Time in seconds of animation played. (play rate independant).
	 * @param	ExcessTime	Time in seconds beyond end of animation. (play rate independant).
	 */
	virtual void OnAnimEnd(FLOAT PlayedTime, FLOAT ExcessTime);
};

struct FAnimInfo
{
	FName AnimSeqName;
	class UAnimSequence* AnimSeq;
	INT AnimLinkupIndex;

	/** Constructors */
	FAnimInfo() {}
	FAnimInfo(EEventParm)
	{
		appMemzero(this, sizeof(FAnimInfo));
	}
};

struct FAnimBlendInfo
{
	FName AnimName;
	struct FAnimInfo AnimInfo;
	FLOAT Weight;

	/** Constructors */
	FAnimBlendInfo() {}
	FAnimBlendInfo(EEventParm)
	{
		appMemzero(this, sizeof(FAnimBlendInfo));
	}
};

class noAnimNodeSequenceBlendBase : public noAnimNodeSequence
{
public:
	//## BEGIN PROPS AnimNodeSequenceBlendBase
	TArrayNoInit<struct FAnimBlendInfo> Anims;
	//## END PROPS AnimNodeSequenceBlendBase
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, noAnimNodeBlendBase* Parent);

	/** AnimSets have been updated, update all animations */
	virtual void AnimSetsUpdated();

	/** make sure animations are up date */
	virtual void CheckAnimsUpToDate();

	/** Lookup animation data for a given animation name */
	void SetAnimInfo(FName InSequenceName, FAnimInfo& InAnimInfo);

	/** Extract animations and do the blend. */
	virtual void GetBoneAtoms(TArray<FBoneAtom>& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);
};

class noAnimNodeSequenceBlendByAim : public noAnimNodeSequenceBlendBase
{
public:
	FVector2D Aim;
	FVector2D PreviousAim;
	FVector2D HorizontalRange;
	FVector2D VerticalRange;
	FVector2D AngleOffset;
	FName AnimName_LU;
	FName AnimName_LC;
	FName AnimName_LD;
	FName AnimName_CU;
	FName AnimName_CC;
	FName AnimName_CD;
	FName AnimName_RU;
	FName AnimName_RC;
	FName AnimName_RD;

	void CheckAnimsUpToDate();

	/** Override this function in a subclass, and return normalized Aim from Pawn. */
	virtual FVector2D GetAim();

	// AnimNode interface
	virtual	void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
};

class noAnimNodePlayCustomAnim : public noAnimNodeBlend
{
public:
	BITFIELD bIsPlayingCustomAnim:1 GCC_BITFIELD_MAGIC;
	FLOAT CustomPendingBlendOutTime;

	FLOAT PlayCustomAnim(FName AnimName,FLOAT Rate,FLOAT BlendInTime=0,FLOAT BlendOutTime=0,UBOOL bLooping=FALSE,UBOOL bOverride=FALSE);
	void PlayCustomAnimByDuration(FName AnimName,FLOAT Duration,FLOAT BlendInTime=0,FLOAT BlendOutTime=0,UBOOL bLooping=FALSE,UBOOL bOverride=FALSE);
	void StopCustomAnim(FLOAT BlendOutTime);

	virtual void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
};

class noAnimNodeBlendDirectional : public noAnimNodeBlendBase
{
public:
	FLOAT DirDegreesPerSecond;
	FLOAT DirAngle;
	INT SingleAnimAtOrAboveLOD;

	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
};

class noAnimNodeBlendList : public noAnimNodeBlendBase
{
public:
	TArrayNoInit<FLOAT> TargetWeight;
	FLOAT BlendTimeToGo;
	INT ActiveChildIndex;
	BITFIELD bPlayActiveChild:1;
	BITFIELD bForceChildFullWeightWhenBecomingRelevant:1;
	BITFIELD bSkipBlendWhenNotRendered:1;
	FLOAT SliderPosition;

	virtual void SetActiveChild(INT ChildIndex,FLOAT BlendTime);

	// AnimNode interface
	virtual void InitAnim( USkeletalMeshComponent* meshComp, noAnimNodeBlendBase* Parent );
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );

	// AnimNodeBlendBase interface
	virtual void OnAddChild(INT ChildNum);
	virtual void OnRemoveChild(INT ChildNum);
	void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
	FString GetSliderDrawValue(INT SliderIndex);
	FLOAT GetSliderPosition(INT SliderIndex, INT ValueIndex);
};

class noAnimNodeBlendByBase : public noAnimNodeBlendList
{
public:
	BYTE Type;
	FName ActorTag;
	//class UClass* ActorClass;
	FLOAT BlendTime;
	//class AActor* CachedBase;
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
};

class noAnimNodeBlendByPhysics : public noAnimNodeBlendList
{
public:
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );

};

class noAnimNodeBlendByPosture : public noAnimNodeBlendList
{
public:
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
};

class noAnimNodeBlendBySpeed : public noAnimNodeBlendList
{
public:
	FLOAT Speed;
	INT LastChannel;
	FLOAT BlendUpTime;
	FLOAT BlendDownTime;
	FLOAT BlendDownPerc;
	TArrayNoInit<FLOAT> Constraints;
	BITFIELD bUseAcceleration:1;

	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );

	virtual FLOAT CalcSpeed();
	void OnBecomeRelevant();
};

struct FRandomAnimInfo
{
	FLOAT Chance;
	BYTE LoopCountMin;
	BYTE LoopCountMax;
	FLOAT BlendInTime;
	FVector2D PlayRateRange;
	BITFIELD bStillFrame:1;
	BYTE LoopCount ;

	/** Constructors */
	FRandomAnimInfo() {}
	FRandomAnimInfo(EEventParm)
	{
		appMemzero(this, sizeof(FRandomAnimInfo));
	}
};

class noAnimNodeRandom : public noAnimNodeBlendList
{
public:
	TArrayNoInit<struct FRandomAnimInfo> RandomInfo;
	class noAnimNodeSequence* PlayingSeqNode;
	INT PendingChildIndex;

	virtual void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
	virtual void InitAnim( USkeletalMeshComponent* meshComp, noAnimNodeBlendBase* Parent );
	/** A child has been added, update RandomInfo accordingly */
	virtual void OnAddChild(INT ChildNum);
	/** A child has been removed, update RandomInfo accordingly */
	virtual void OnRemoveChild(INT ChildNum);

	virtual void OnChildAnimEnd(noAnimNodeSequence* Child, FLOAT PlayedTime, FLOAT ExcessTime);

	INT		PickNextAnimIndex();
	void	PlayPendingAnimation(FLOAT BlendTime=0.f, FLOAT StartTime=0.f);
	void OnBecomeRelevant();
};

struct FChildBoneBlendInfo
{
	TArrayNoInit<FLOAT> TargetPerBoneWeight;
	FName InitTargetStartBone;
	FLOAT InitPerBoneIncrease;
	FName OldStartBone;
	FLOAT OldBoneIncrease;
	TArrayNoInit<BYTE> TargetRequiredBones;

	/** Constructors */
	FChildBoneBlendInfo() {}
	FChildBoneBlendInfo(EEventParm)
	{
		appMemzero(this, sizeof(FChildBoneBlendInfo));
	}
};

class noAnimNodeBlendMultiBone : public noAnimNodeBlendBase
{
public:
	TArrayNoInit<struct FChildBoneBlendInfo> BlendTargetList;
	TArrayNoInit<BYTE> SourceRequiredBones;

	virtual void InitAnim( USkeletalMeshComponent* meshComp, noAnimNodeBlendBase* Parent );
	virtual void GetBoneAtoms(TArray<FBoneAtom>& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);

		// AnimNodeBlendPerBone interface
	/** 
	 * Calculates total weight of children. 
	 * Set a full weight on source, because it's potentially always feeding animations into the final blend.
	 */
	virtual void SetChildrenTotalWeightAccumulator(const INT Index);

	/** Utility for creating the TargetPerBoneWeight array. Starting from the named bone, walk down the heirarchy increasing the weight by PerBoneIncrease each step. */
	virtual void SetTargetStartBone( INT TargetIdx, FName StartBoneName, FLOAT PerBoneIncrease = 1.f );
};

class noAnimNodeMirror : public noAnimNodeBlendBase
{
public:
	BITFIELD bEnableMirroring:1 GCC_BITFIELD_MAGIC;

	virtual void GetBoneAtoms(TArray<FBoneAtom>& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);
};

class noAnimNodeScalePlayRate : public noAnimNodeBlendBase
{
public:
	FLOAT ScaleByValue;

	virtual void	TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
	virtual FLOAT	GetScaleValue();
};

class noAnimNodeScaleRateBySpeed : public noAnimNodeScalePlayRate
{
public:
	FLOAT BaseSpeed;

	virtual FLOAT	GetScaleValue();

};

class noAnimNodeSlot : public noAnimNodeBlendBase
{
public:
	BITFIELD bIsPlayingCustomAnim:1 GCC_BITFIELD_MAGIC;
	BITFIELD bEarlyAnimEndNotify:1;
	BITFIELD bSkipBlendWhenNotRendered:1;
	FLOAT PendingBlendOutTime;
	INT CustomChildIndex;
	INT TargetChildIndex;
	TArrayNoInit<FLOAT> TargetWeight;
	FLOAT BlendTimeToGo;
	class noAnimNodeSynch* SynchNode;

	FLOAT PlayCustomAnim(FName AnimName,FLOAT Rate,FLOAT BlendInTime=0,FLOAT BlendOutTime=0,UBOOL bLooping=FALSE,UBOOL bOverride=FALSE);
	void PlayCustomAnimByDuration(FName AnimName,FLOAT Duration,FLOAT BlendInTime=0,FLOAT BlendOutTime=0,UBOOL bLooping=FALSE,UBOOL bOverride=TRUE);
	FName GetPlayedAnimation();
	void StopCustomAnim(FLOAT BlendOutTime);
	void SetCustomAnim(FName AnimName);
	void SetActorAnimEndNotification(UBOOL bNewStatus);
	class noAnimNodeSequence* GetCustomAnimNodeSeq();
	void SetRootBoneAxisOption(BYTE AxisX=0,BYTE AxisY=0,BYTE AxisZ=0);
	void AddToSynchGroup(FName GroupName);

	// AnimNode interface
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, noAnimNodeBlendBase* Parent);

	/** Update position of given channel */
	virtual void MAT_SetAnimPosition(INT ChannelIndex, FName InAnimSeqName, FLOAT InPosition, UBOOL bFireNotifies, UBOOL bLooping);
	/** Update weight of channels */
	virtual void MAT_SetAnimWeights(const FAnimSlotInfo& SlotInfo);
	/** Rename all child nodes upon Add/Remove, so they match their position in the array. */
	//virtual void RenameChildConnectors();

	// AnimNode interface
	virtual	void	TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );

	// AnimNodeBlendBase interface
	virtual void	OnAddChild(INT ChildNum);
	virtual void	OnRemoveChild(INT ChildNum);

	/**
	 * When requested to play a new animation, we need to find a new child.
	 * We'd like to find one that is unused for smooth blending, 
	 * but that may be a luxury that is not available.
	 */
	INT		FindBestChildToPlayAnim(FName AnimToPlay);

	void	SetActiveChild(INT ChildIndex, FLOAT BlendTime);
	void	UpdateWeightsForAdditiveAnimations();
	void RenameChildConnectors();
};

struct FSynchGroup
{
	TArrayNoInit<class noAnimNodeSequence*> SeqNodes;
	class noAnimNodeSequence* MasterNode;
	FName GroupName;
	BITFIELD bFireSlaveNotifies:1;
	FLOAT RateScale;

	/** Constructors */
	FSynchGroup() {}
	FSynchGroup(EEventParm)
	{
		appMemzero(this, sizeof(FSynchGroup));
	}
};

class noAnimNodeSynch : public noAnimNodeBlendBase
{
public:
	TArrayNoInit<struct FSynchGroup> Groups;
	//## END PROPS AnimNodeSynch

	void AddNodeToGroup(class noAnimNodeSequence* SeqNode,FName GroupName);
	void RemoveNodeFromGroup(class noAnimNodeSequence* SeqNode,FName GroupName);
	class noAnimNodeSequence* GetMasterNodeOfGroup(FName GroupName);
	void ForceRelativePosition(FName GroupName,FLOAT RelativePosition);
	FLOAT GetRelativePosition(FName GroupName);
	void SetGroupRateScale(FName GroupName,FLOAT NewRateScale);

	virtual void	PostLoad();
	virtual void	InitAnim(USkeletalMeshComponent* MeshComp, noAnimNodeBlendBase* Parent);
	virtual	void	TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);

	void			UpdateMasterNodeForGroup(FSynchGroup& SynchGroup);
	void			RepopulateGroups();
};

struct FAnimGroup
{
	TArrayNoInit<class noAnimNodeSequence*> SeqNodes;
	class noAnimNodeSequence* SynchMaster;
	class noAnimNodeSequence* NotifyMaster;
	FName GroupName;
	FLOAT RateScale;
	FLOAT SynchPctPosition;

	/** Constructors */
	FAnimGroup() {}
	FAnimGroup(EEventParm)
	{
		appMemzero(this, sizeof(FAnimGroup));
	}
};

struct FSkelControlListHead
{
	FName BoneName;
	class USkelControlBase* ControlHead;
	INT DrawY;

	/** Constructors */
	FSkelControlListHead() {}
	FSkelControlListHead(EEventParm)
	{
		appMemzero(this, sizeof(FSkelControlListHead));
	}
};

class UAnimTree : public noAnimNodeBlendBase
{
public:
	//## BEGIN PROPS AnimTree
	TArrayNoInit<struct FAnimGroup> AnimGroups;
	TArrayNoInit<FName> PrioritizedSkelBranches;
	TArrayNoInit<BYTE> PriorityList;
	TArrayNoInit<class UMorphNodeBase*> RootMorphNodes;
	TArrayNoInit<struct FSkelControlListHead> SkelControlLists;
	INT MorphConnDrawY;
	BITFIELD bBeingEdited:1;
	FLOAT PreviewPlayRate;
	/*class USkeletalMesh* PreviewSkelMesh;
	class USkeletalMesh* SocketSkelMesh;
	class UStaticMesh* SocketStaticMesh;*/
	FName SocketName;
	TArrayNoInit<class UAnimSet*> PreviewAnimSets;
	TArrayNoInit<class UMorphTargetSet*> PreviewMorphSets;
	FVector PreviewCamPos;
	//FRotator PreviewCamRot;
	FVector PreviewFloorPos;
	INT PreviewFloorYaw;

	class USkelControlBase* FindSkelControl(FName InControlName);
	class UMorphNodeBase* FindMorphNode(FName InNodeName);
	UBOOL SetAnimGroupForNode(class noAnimNodeSequence* SeqNode,FName GroupName,UBOOL bCreateIfNotFound=FALSE);
	class noAnimNodeSequence* GetGroupSynchMaster(FName GroupName);
	class noAnimNodeSequence* GetGroupNotifyMaster(FName GroupName);
	void ForceGroupRelativePosition(FName GroupName,FLOAT RelativePosition);
	FLOAT GetGroupRelativePosition(FName GroupName);
	void SetGroupRateScale(FName GroupName,FLOAT NewRateScale);
	FLOAT GetGroupRateScale(FName GroupName);
	INT GetGroupIndex(FName GroupName);

	virtual void	InitAnim(USkeletalMeshComponent* meshComp, noAnimNodeBlendBase* Parent);

	void			UpdateAnimNodeSeqGroups(FLOAT DeltaSeconds);
	void			UpdateMasterNodesForGroup(FAnimGroup& AnimGroup);
	void			RepopulateAnimGroups();

	//void			PostEditChange(UProperty* PropertyThatChanged);

	/** Get all SkelControls within this AnimTree. */
	void			GetSkelControls(TArray<USkelControlBase*>& OutControls);

	/** Get all MorphNodes within this AnimTree. */
	//void			GetMorphNodes(TArray<class UMorphNodeBase*>& OutNodes);

	///** Call InitMorph on all morph nodes attached to the tree. */
	//void			InitTreeMorphNodes(USkeletalMeshComponent* InSkelComp);

	///** Calls GetActiveMorphs on each element of the RootMorphNodes array. */
	//void			GetTreeActiveMorphs(TArray<FActiveMorph>& OutMorphs);

	// noAnimNode interface
	//virtual FIntPoint GetConnectionLocation(INT ConnType, INT ConnIndex);
};

class UAnimNotify 
{
public:
	//## BEGIN PROPS AnimNotify
	//## END PROPS AnimNotify

	//DECLARE_ABSTRACT_CLASS(UAnimNotify,UObject,0,Engine)
	// AnimNotify interface.
	virtual void Notify( class noAnimNodeSequence* NodeSeq ) {};
};

class UAnimNotify_Footstep : public UAnimNotify
{
public:
	//## BEGIN PROPS AnimNotify_Footstep
	INT FootDown;
	//## END PROPS AnimNotify_Footstep

	//DECLARE_CLASS(UAnimNotify_Footstep,UAnimNotify,0,Engine)
	// AnimNotify interface.
	virtual void Notify( class noAnimNodeSequence* NodeSeq );
};

class UAnimNotify_Script : public UAnimNotify
{
public:
	//## BEGIN PROPS AnimNotify_Script
	FName NotifyName;
	//## END PROPS AnimNotify_Script

	//DECLARE_CLASS(UAnimNotify_Script,UAnimNotify,0,Engine)
	// AnimNotify interface.
	virtual void Notify( class noAnimNodeSequence* NodeSeq );
};

struct AnimNotify_Scripted_eventNotify_Parms
{
	class AActor* Owner;
	class noAnimNodeSequence* AnimSeqInstigator;
	AnimNotify_Scripted_eventNotify_Parms(EEventParm)
	{
	}
};
class UAnimNotify_Scripted : public UAnimNotify
{
public:
	//## BEGIN PROPS AnimNotify_Scripted
	//## END PROPS AnimNotify_Scripted

	void eventNotify(class AActor* Owner,class noAnimNodeSequence* AnimSeqInstigator)
	{
		AnimNotify_Scripted_eventNotify_Parms Parms(EC_EventParm);
		Parms.Owner=Owner;
		Parms.AnimSeqInstigator=AnimSeqInstigator;
		//ProcessEvent(FindFunctionChecked(ENGINE_Notify),&Parms);
	}
	//DECLARE_ABSTRACT_CLASS(UAnimNotify_Scripted,UAnimNotify,0,Engine)
	// AnimNotify interface.
	virtual void Notify( class noAnimNodeSequence* NodeSeq );
};

class UAnimNotify_Sound : public UAnimNotify
{
public:
	//## BEGIN PROPS AnimNotify_Sound
	//class USoundCue* SoundCue;
	BITFIELD bFollowActor:1;
	FName BoneName;
	//## END PROPS AnimNotify_Sound

	//DECLARE_CLASS(UAnimNotify_Sound,UAnimNotify,0,Engine)
	// AnimNotify interface.
	virtual void Notify( class noAnimNodeSequence* NodeSeq );
};

struct FAnimNotifyEvent
{
	FLOAT Time;
	class UAnimNotify* Notify;
	FName Comment;

	/** Constructors */
	FAnimNotifyEvent() {}
	FAnimNotifyEvent(EEventParm)
	{
		appMemzero(this, sizeof(FAnimNotifyEvent));
	}
};

struct FRawAnimSequenceTrack
{
	TArrayNoInit<FVector> PosKeys;
	TArrayNoInit<noQuat> RotKeys;
	TArrayNoInit<FLOAT> KeyTimes;

	/** Constructors */
	FRawAnimSequenceTrack() {}
	FRawAnimSequenceTrack(EEventParm)
	{
		appMemzero(this, sizeof(FRawAnimSequenceTrack));
	}
};

struct FTranslationTrack
{
	TArrayNoInit<FVector> PosKeys;
	TArrayNoInit<FLOAT> Times;

	/** Constructors */
	FTranslationTrack() {}
	FTranslationTrack(EEventParm)
	{
		appMemzero(this, sizeof(FTranslationTrack));
	}
};

struct FRotationTrack
{
	TArrayNoInit<noQuat> RotKeys;
	TArrayNoInit<FLOAT> Times;

	/** Constructors */
	FRotationTrack() {}
	FRotationTrack(EEventParm)
	{
		appMemzero(this, sizeof(FRotationTrack));
	}
};

struct FCompressedTrack
{
	TArrayNoInit<BYTE> ByteStream;
	TArrayNoInit<FLOAT> Times;
	FLOAT Mins[3];
	FLOAT Ranges[3];

	/** Constructors */
	FCompressedTrack() {}
	FCompressedTrack(EEventParm)
	{
		appMemzero(this, sizeof(FCompressedTrack));
	}
};

class UAnimSequence 
{
public:
	FName SequenceName;
	TArrayNoInit<struct FAnimNotifyEvent> Notifies;
	FLOAT SequenceLength;
	INT NumFrames;
	FLOAT RateScale;
	BITFIELD bNoLoopingInterpolation:1;
	BITFIELD bIsAdditive:1;
	TArrayNoInit<struct FRawAnimSequenceTrack> RawAnimData;
	TArrayNoInit<struct FTranslationTrack> TranslationData;
	TArrayNoInit<struct FRotationTrack> RotationData;
	class UAnimationCompressionAlgorithm* CompressionScheme;
	BYTE TranslationCompressionFormat;
	BYTE RotationCompressionFormat;
	TArrayNoInit<INT> CompressedTrackOffsets;
	TArrayNoInit<BYTE> CompressedByteStream;
	TArrayNoInit<FBoneAtom> AdditiveRefPose;
	FName AdditiveRefName;

		/**
	 * Reconstructs a bone atom from key-reduced tracks.
	 */
	static void ReconstructBoneAtom(FBoneAtom& OutAtom,
									const FTranslationTrack& TranslationTrack,
									const FRotationTrack& RotationTrack,
									FLOAT SequenceLength,
									FLOAT Time,
									UBOOL bLooping);

	/**
	 * Interpolate keyframes in this sequence to find the bone transform (relative to parent).
	 * 
	 * @param	OutAtom			[out] Output bone transform.
	 * @param	TrackIndex		Index of track to interpolate.
	 * @param	Time			Time on track to interpolate to.
	 * @param	bLooping		TRUE if the animation is looping.
	 * @param	bUseRawData		If TRUE, use raw animation data instead of compressed data.
	 */
	void GetBoneAtom(FBoneAtom& OutAtom, INT TrackIndex, FLOAT Time, UBOOL bLooping, UBOOL bUseRawData) const;

	/** Sort the Notifies array by time, earliest first. */
	void SortNotifies();

	/**
	 * @return		A reference to the AnimSet this sequence belongs to.
	 */
	UAnimSet* GetAnimSet() const;


	/** Clears any data in the AnimSequence, so it can be recycled when importing a new animation with same name over it. */
	void RecycleAnimSequence();
	void SeparateRawDataToTracks(const TArray<FRawAnimSequenceTrack>& RawAnimData, FLOAT SequenceLength, TArray<FTranslationTrack>& OutTranslationData, TArray<FRotationTrack>& OutRotationData);
};


struct FAnimSetMeshLinkup
{
	FGuid SkelMeshLinkupGUID;
	TArrayNoInit<INT> BoneToTrackTable;
	TArrayNoInit<BYTE> BoneUseAnimTranslation;

	/** Reset this linkup and re-create between the provided skeletal mesh and anim set. */
	void BuildLinkup(class USkeletalMesh* InSkelMesh, UAnimSet* InAnimSet);

};

class UAnimSet
{
public:
	//## BEGIN PROPS AnimSet
	BITFIELD bAnimRotationOnly:1;
	TArrayNoInit<FName> TrackBoneNames;
	TArrayNoInit<class UAnimSequence*> Sequences;
	TArrayNoInit<struct FAnimSetMeshLinkup> LinkupCache;
	TArrayNoInit<FName> UseTranslationBoneNames;
	FName PreviewSkelMeshName;

	/**
	 * See if we can play sequences from this AnimSet on the provided SkeletalMesh.
	 * Returns true if there is a bone in SkelMesh for every track in the AnimSet,
	 * or there is a track of animation for every bone of the SkelMesh.
	 * 
	 * @param	SkelMesh	SkeletalMesh to compare the AnimSet against.
	 * @return				TRUE if animation set can play on supplied SkeletalMesh, FALSE if not.
	 */
	UBOOL CanPlayOnSkeletalMesh(USkeletalMesh* SkelMesh) const;

	/**
	 * Returns the AnimSequence with the specified name in this set.
	 * 
	 * @param		SequenceName	Name of sequence to find.
	 * @return						Pointer to AnimSequence with desired name, or NULL if sequence was not found.
	 */
	UAnimSequence* FindAnimSequence(FName SequenceName) const;

	/** 
	 * Find a mesh linkup table (mapping of sequence tracks to bone indices) for a particular SkeletalMesh
	 * If one does not already exist, create it now.
	 */
	INT GetMeshLinkupIndex(USkeletalMesh* SkelMesh);

	/**
	 * @return		The track index for the bone with the supplied name, or INDEX_NONE if no track exists for that bone.
	 */
	INT FindTrackWithName(FName BoneName) const
	{
		return TrackBoneNames.FindItemIndex( BoneName );
	}

	/**
	 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
	 *
	 * @return size of resource as to be displayed to artists/ LDs in the Editor.
	 */
	INT GetResourceSize();

	/**
	 * Clears all sequences and resets the TrackBoneNames table.
	 */
	void ResetAnimSet();

	/** Util that find all AnimSets and flushes their LinkupCache, then calls InitAnimTree on all SkeletalMeshComponents. */
	static void ClearAllAnimSetLinkupCaches();
};

class UMorphNodeBase
{
public:
	//## BEGIN PROPS MorphNodeBase
	FName NodeName;
	BITFIELD bDrawSlider:1;
	class USkeletalMeshComponent* SkelComponent;
	INT NodePosX;
	INT NodePosY;
	INT DrawWidth;
	INT DrawHeight;
	INT OutDrawY;
};

class UMorphNodePose : public UMorphNodeBase
{
public:

};

struct FMorphNodeConn
{
	TArrayNoInit<class UMorphNodeBase*> ChildNodes;
	FName ConnName;
	INT DrawY;

	/** Constructors */
	FMorphNodeConn() {}
	FMorphNodeConn(EEventParm)
	{
		appMemzero(this, sizeof(FMorphNodeConn));
	}
};

class UMorphNodeWeightBase : public UMorphNodeBase
{
public:

};

class UMorphNodeWeight : public UMorphNodeWeightBase
{
public:

};

class UMorphTargetSet 
{
public:

};

class USkelControlBase 
{
public:
	//## BEGIN PROPS SkelControlBase
	class USkeletalMeshComponent* SkelComponent;
	FName ControlName;
	FLOAT ControlStrength;
	FLOAT BlendInTime;
	FLOAT BlendOutTime;
	FLOAT StrengthTarget;
	FLOAT BlendTimeToGo;
	BITFIELD bSetStrengthFromAnimNode:1;
	BITFIELD bInitializedCachedNodeList:1;
	BITFIELD bPropagateSetActive:1;
	BITFIELD bIgnoreWhenNotRendered:1;
	BITFIELD bEnableEaseInOut:1;
	TArrayNoInit<FName> StrengthAnimNodeNameList;
	TArrayNoInit<class noAnimNode*> CachedNodeList;
	FLOAT BoneScale;
	INT ControlTickTag;
	INT IgnoreAtOrAboveLOD;
	class USkelControlBase* NextControl;
	INT ControlPosX;
	INT ControlPosY;
	INT DrawWidth;
};

class USkelControlLimb : public USkelControlBase
{
public:

};

class USkelControlFootPlacement : public USkelControlLimb
{
public:

};


class USkelControlLookAt : public USkelControlBase
{
public:

};

class USkelControlSingleBone : public USkelControlBase
{
public:

};

class USkelControlWheel : public USkelControlSingleBone
{
public:

};

class USkelControlSpline : public USkelControlBase
{
public:
	
};

class USkelControlTrail : public USkelControlBase
{
public:

};

#pragma pack (pop)



