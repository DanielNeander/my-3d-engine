#include "stdafx.h"
#include "SkeletalMesh.h"
#include "AnimClasses.h"

/**
 * Match up startbone by name. Also allows the attachment tag aliases.
 * Pretty much the same as USkeletalMeshComponent::MatchRefBone
 */
INT USkeletalMesh::MatchRefBone(FName StartBoneName) const
{
	INT BoneIndex = INDEX_NONE;
	if( StartBoneName != NAME_None )
	{
		FString out;
		StartBoneName.ToString(out);
		const INT Index = NameIndexMap.find(out)->second;
		
		BoneIndex = Index;		
	}
	return BoneIndex;
}

INT USkeletalMeshComponent::MatchRefBone( FName BoneName) const
{
	INT BoneIndex = INDEX_NONE;
	if ( BoneName != NAME_None && SkeletalMesh )
	{
		BoneIndex = SkeletalMesh->MatchRefBone( BoneName );
	}

	return BoneIndex;
}

/** 
 * Build a priority list of branches that should be evaluated first.
 * This is to solve Controllers relying on bones to be updated before them. 
 */
void USkeletalMeshComponent::BuildComposePriorityList(TArray<BYTE>& PriorityList)
{
	if( !SkeletalMesh || !Animations )
	{
		return;
	}

	UAnimTree*	Tree		= (UAnimTree*)Animations;
	const	INT			NumBones	= SkeletalMesh->RefSkeleton.Num();

	// If the first node of the Animation Tree if not a UAnimTree, then skip.
	// This can happen in the AnimTree editor when previewing a node different than the root.
	if( !Tree )
	{
		return;
	}

	// reinitialize list
	PriorityList.Empty();
	PriorityList.AddZeroed(NumBones);

	const BYTE Flag = 1;

	for(INT i=0; i<Tree->PrioritizedSkelBranches.Num(); i++)
	{
		const FName BoneName = Tree->PrioritizedSkelBranches(i);

		if( BoneName != NAME_None )
		{
			const INT BoneIndex = SkeletalMesh->MatchRefBone(BoneName);

			if( BoneIndex != INDEX_NONE )
			{
				// flag selected bone.
				PriorityList(BoneIndex) = Flag;

				// flag all parents up until root node to be evaluated.
				if( BoneIndex > 0 )
				{
					INT TestBoneIndex = SkeletalMesh->RefSkeleton(BoneIndex).ParentIndex;
					PriorityList(TestBoneIndex) = Flag;
					while( TestBoneIndex > 0 )
					{
						TestBoneIndex = SkeletalMesh->RefSkeleton(TestBoneIndex).ParentIndex;
						PriorityList(TestBoneIndex) = Flag;
					}
				}

				// Flag all child bones. We rely on the fact that they are in strictly increasing order
				// so we start at the bone after BoneIndex, up until we reach the end of the list
				// or we find another bone that has a parent before BoneIndex.
				INT	Index = BoneIndex + 1;
				if(Index < NumBones)
				{
					INT ParentIndex	= SkeletalMesh->RefSkeleton(Index).ParentIndex;

					while( Index < NumBones && ParentIndex >= BoneIndex )
					{
						PriorityList(Index)	= Flag;
						ParentIndex			= SkeletalMesh->RefSkeleton(Index).ParentIndex;

						Index++;
					}
				}
			}
		}
	}

#if 0
	debugf(TEXT("USkeletalMeshComponent::BuildComposePriorityList"));
	for(INT i=0; i<PriorityList.Num(); i++)
	{
		debugf(TEXT(" Bone: %3d, Flag: %3d"), i, PriorityList(i));
	}
#endif

}


UAnimSequence* USkeletalMeshComponent::FindAnimSequence(FName AnimSeqName)
{
	if( AnimSeqName == NAME_None )
	{
		return NULL;
	}

	// Work from last element in list backwards, so you can replace a specific sequence by adding a set later in the array.
	//for(INT i=AnimSets.Num()-1; i>=0; i--)
	{
		//if( AnimSets(i) )
		{
			//UAnimSequence* FoundSeq = AnimSets(i)->FindAnimSequence(AnimSeqName);
			//if( FoundSeq )
			{
				//return FoundSeq;
			}
		}
	}

	return NULL;
}