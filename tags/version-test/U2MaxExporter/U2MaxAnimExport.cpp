#include "U2MaxAnimExport.h"
#include "U2MaxUtil.h"
#include "U2MaxNoteTrack.h"
#include "U2MaxSceneExport.h"
#include "U2MaxMeshExport.h"
#include "U2MaxOptions.h"

#include <U2_3D/src/Animation/U2SkinController.h>
#include <U2_3D/src/Tools/U2AnimBuilder.h>


const float U2MaxAnimExport::ms_fKeyTolerance = 0.0000001f * 5;
U2MaxAnimExport* U2MaxAnimExport::ms_pSingleton = 0;


//-------------------------------------------------------------------------------------------------
U2MaxAnimExport::Bone::Bone()
:m_iParentId(-1),
m_iThisId(-1),
m_bDummy(false)
{

}

//-------------------------------------------------------------------------------------------------
U2MaxAnimExport::U2MaxAnimExport()
{
	U2ASSERT(ms_pSingleton == 0);
	ms_pSingleton = this;
}

//-------------------------------------------------------------------------------------------------
void U2MaxAnimExport::Init(TimeValue animStart, TimeValue animEnd)
{
	m_animStart = animStart; 
	m_animEnd = animEnd; 
	m_uiNumFrames = ((m_animEnd - m_animStart) / GetTicksPerFrame()) + 1;
}


void U2MaxAnimExport::GetNodes(INode* node, U2PrimitiveVec<INode*>& nodeArray)
{
	if (0 == node)
		return;

	nodeArray.AddElem(node);

	// recursively get nodes.
	const int numChildNodes = node->NumberOfChildren();
	for (int i=0; i<numChildNodes; i++)
	{
		GetNodes(node->GetChildNode(i), nodeArray);
	}
}

//---------------------------------------------------------------------------
/**
@param nodeArray scene nodes array
@param boneNodeArray array which retrieved bone is contained.
*/
void U2MaxAnimExport::GetBoneByModifier(const U2PrimitiveVec<INode*>& nodeArray, 
										U2PrimitiveVec<INode*> &boneNodeArray)
{
	for (int i=0; i<nodeArray.Size(); i++)
	{
		INode* node = nodeArray[i];

		if (!U2MaxUtil::IsGeomObject(node))
			continue;

		Modifier* physique = U2MaxUtil::FindPhysique(node);
		Modifier* skin     = U2MaxUtil::FindSkin(node);

		if (physique || skin)
		{
			ObjectState os = node->EvalWorldState(0);

			if (os.obj)
			{
				if (physique)
				{
					ExtractPhysiqueBones(node, physique, &os, boneNodeArray);
				}
				else
					if (skin)
					{
						ExtractSkinBones(node, skin, boneNodeArray);
					}
			}
		}
	}
}


//-----------------------------------------------------------------------------
/**
*/
void U2MaxAnimExport::GetBoneByClassID(const U2PrimitiveVec<INode*>& nodeArray, 
									   U2PrimitiveVec<INode*> &boneNodeArray)
{
	for (int i=0; i<nodeArray.Size(); i++)
	{
		INode* node = nodeArray[i];

		// check the given node is bone node.
		bool isBone = U2MaxUtil::IsBone(node);

		//HACK: if the dummy node is just like a group node or something 
		//      which is not actually bone?
		//      dummy node can be used in various way in 3dsmax.
		//      so it should be considered when it actually used for bone.
		//      A dummy node which used for bone should be added when it has modifier 
		//      in GetBoneByModifier() func

		//bool isDummy = this->IsDummy(node);

		//if (isBone || isDummy)
		if (isBone)
		{
			if (boneNodeArray.Find(node) == (unsigned int)-1)
			{
				boneNodeArray.AddElem(node);
			}
		}
	}
}

//-----------------------------------------------------------------------------
int U2MaxAnimExport::GetSkeletonForNode(INode* inode) 
{
	std::map<INode*, int>::iterator fj;
	U2ASSERT(inode);
	fj = m_nodeToSkelMap.find(inode);
	if (fj != m_nodeToSkelMap.end()) 
	{
		return fj->second;
	}
	return -1;
}

//-----------------------------------------------------------------------------
/**
@struct BoneLevel
@ingroup U2MaxAnimExport

@brief It is used for a temporary to find root bones.
*/
struct BoneLevel : public U2MemObj
{
	INode *node;
	int   depth;
};

//-----------------------------------------------------------------------------
/**
*/
int U2MaxAnimExport::GetRootBones(INode *sceneRoot, U2PrimitiveVec<INode*> &boneNodeArray, 
								  U2PrimitiveVec<INode*> &rootBonesNodeArray) 
{
	uint32 i, j, k;

	for(i=0; i < boneNodeArray.Size(); ++i)
	{
		FILE_LOG(logDEBUG) << TSTR("Bone Node Name : ") << boneNodeArray[i]->GetName();
	}
	
	U2PrimitiveVec<BoneLevel*> boneLevels;

	INode* bone;
	int depth;

	for(i=0; i < boneNodeArray.Size(); ++i)
	{
		bone = boneNodeArray[i];
		depth = 0;

		if(i == 26)
		{
			int x = 5;
		}

		while(bone != sceneRoot)
		{
			bone = bone->GetParentNode();
			depth++;
		}

		BoneLevel* bl = U2_NEW BoneLevel;
		bl->depth = depth;
		bl->node = boneNodeArray[i];
		boneLevels.AddElem(bl);

		FILE_LOG(logDEBUG) << TSTR("Before Sort : ") << i << 
			TSTR("Name : ") << bl->node->GetName() << 
			TSTR(" depth : ") << depth;
	}

	U2ASSERT(boneNodeArray.Size() == boneLevels.Size());

	// do selection sort.
	BoneLevel* tmp;
	for(i=0; i < boneLevels.Size() - 1; ++i)
	{
		k= i;
		for(j=i+1; j < boneLevels.Size() ;++j)
		{
			// find the bone which has lowest depth value
			if(boneLevels[k]->depth > boneLevels[j]->depth)
			{
				k = j;
			}
		}

		tmp = boneLevels[i];
		boneLevels.SetElem(i, boneLevels[k]);
		boneLevels.SetElem(k, tmp);
		
		FILE_LOG(logDEBUG) << TSTR("After Sort : ") << i <<
			TSTR(" depth : ") << boneLevels[i]->depth;
	}

	// the first node in boneLevelArray is the root bone node.
	depth = boneLevels[0]->depth;
	for(int i=0; i < boneLevels.Size() ; ++i)
	{
		INode* pMaxNode = boneLevels[i]->node;
		Object* obj = U2MaxUtil::GetBaseObject(pMaxNode, 0);

		// GetCustomAttb(obj)
		// GetCustomAttb(pMaxNode)
		
		// this is needes for an object which is any modifier is applied.
		if(pMaxNode->GetObjectRef() != obj)
		{
			// GetCustomAttb(pMaxNode->GetObjectRef)
		}
		
		if(boneLevels[i]->depth == depth)
		{
			// add the root bone to the array.
			rootBonesNodeArray.AddElem(pMaxNode);
		}
	}

	for(int i=0; i < boneLevels.Size() ; ++i)
	{
		BoneLevel* bl = boneLevels[i];
		U2_DELETE bl;
	}

	boneLevels.RemoveAll();

	return rootBonesNodeArray.Size();
}


//-----------------------------------------------------------------------------
/**
*/
void U2MaxAnimExport::ReconstructBoneHierarchy(int parentID, int skeleton, INode* node, 
											   U2PrimitiveVec<INode*> &boneNodeArray) 
{
	Bone* bone = U2_NEW Bone;
	bone->m_localTM		= U2MaxSceneExport::GetLocalTM(node, m_animStart);
	bone->m_iParentId   = parentID;
	bone->m_iThisId      = this->m_skeletons[skeleton]->FilledSize();
	bone->m_szName		= node->GetName();
	bone->m_pMaxNode     = node;

	FILE_LOG(logDEBUG) << TSTR("Bone Name : ") << bone->m_szName.Str();
	FILE_LOG(logDEBUG) << TSTR(" Bone ID : ") << bone->m_iThisId;

	// add only known bone.
	if (boneNodeArray.Find(node) != (unsigned int)-1) 
	{
		this->m_skeletons[skeleton]->AddElem(bone);
		if (m_boneToSkelMap.find(bone->m_pMaxNode) == m_boneToSkelMap.end()) 
		{
			m_boneToSkelMap[bone->m_pMaxNode] = skeleton;
		}
	}

	for (int i=0; i < node->NumberOfChildren(); i++)
	{
		this->ReconstructBoneHierarchy(bone->m_iThisId, 
			skeleton, node->GetChildNode(i), boneNodeArray);
	}
}

//-----------------------------------------------------------------------------
/**
*/
void U2MaxAnimExport::AddBoneToNode(INode* inode, INode* bone) 
{
	U2ASSERT(inode);
	U2ASSERT(bone);
	if (this->m_nodeToBoneMap.find(inode) == this->m_nodeToBoneMap.end()) 
	{
		this->m_nodeToBoneMap[inode] = bone;
	}
}

//-----------------------------------------------------------------------------
/**
-03-Jun-06  kims  Fixed a bug that it cannot find correct nskinanimator 
when there are more than one root bones. (The case that
two or more nskinanimator is created)
When multiple nskinanimator are created, already added 
bone has -1 so it always fails to find correct skinanimator 
because nskinshapenode always find the first skinanimaotr0 
when it try to find nskinanimator.
Thanks Kim, Seung Hoon for this fix.
*/
void U2MaxAnimExport::ExtractPhysiqueBones(INode* node, Modifier* phyMod, ObjectState* os, 
						  U2PrimitiveVec<INode*> &boneNodeArray)
{
	if(!phyMod)
		return;

	// create a Physique Export Interface for the given Physique Modifier       
	IPhysiqueExport* phyExp = (IPhysiqueExport*)phyMod->GetInterface(I_PHYINTERFACE);

	if(phyExp)
	{
		// create a ModContext Export Interface for the specific node 
		// of the Physique Modifier
		IPhyContextExport *mcExport = (IPhyContextExport *)
			phyExp->GetContextInterface(node);

		if(mcExport)
		{
			// we convert all vertices to Rigid in this example
			mcExport->ConvertToRigid(TRUE);
			mcExport->AllowBlending(TRUE);

			// compute the transformed Point3 at time t
			for (int i=0; i<os->obj->NumPoints(); i++)
			{
				IPhyVertexExport *vtxExport = mcExport->GetVertexInterface(i);
				if (vtxExport)
				{
					//need to check if vertex has blending
					if (vtxExport->GetVertexType() & BLENDED_TYPE)
					{
						IPhyBlendedRigidVertex *vtxBlend = (IPhyBlendedRigidVertex *)vtxExport;

						for (int n=0; n<vtxBlend->GetNumberNodes(); n++)
						{
							INode* bone	= vtxBlend->GetNode(n);

							// add found bone to the bones array.
							if (boneNodeArray.Find(bone) == -1)
							{
								U2DynString nodeName = node->GetName();
								U2DynString boneName = bone->GetName();
								FILE_LOG(logDEBUG) << 
									TSTR("node : ") << nodeName.Str() <<
									TSTR(" -> bone : ") << boneName.Str();
							}

							this->AddBoneToNode(node, bone);
						}

						mcExport->ReleaseVertexInterface(vtxExport);
						vtxExport = NULL;
					}
					else 
					{
						IPhyRigidVertex* rigidVertex = (IPhyRigidVertex*)vtxExport;
						INode* bone = rigidVertex->GetNode();

						if(boneNodeArray.Find(bone) == -1)
						{
							U2DynString nodeName = node->GetName();
							U2DynString boneName = bone->GetName();
							FILE_LOG(logDEBUG) << 
								TSTR("node : ") << nodeName.Str() <<
								TSTR(" -> bone : ") << boneName.Str();
							boneNodeArray.AddElem(bone);
						}

						this->AddBoneToNode(node, bone);

						mcExport->ReleaseVertexInterface(vtxExport);
						vtxExport = NULL;
					}
				}
			}
			phyExp->ReleaseContextInterface(mcExport);
		}

		phyMod->ReleaseInterface(I_PHYINTERFACE, phyExp);		
	}
}

//-----------------------------------------------------------------------------
/**
-03-Jun-06  kims  Fixed a bug that it cannot find correct nskinanimator 
when there are more than one root bones. (The case that
two or more nskinanimator is created)
When multiple nskinanimator are created, already added 
bone has -1 so it always fails to find correct skinanimator 
because nskinshapenode always find the first skinanimaotr0 
when it try to find nskinanimator.
Thanks Kim, Seung Hoon for this fix.

*/
void U2MaxAnimExport::ExtractSkinBones(INode* node, Modifier* skinMod,
					  U2PrimitiveVec<INode*> &boneNodeArray)
{
	// get the skin interface
	ISkin* skin = (ISkin*)skinMod->GetInterface(I_SKIN);
	if (skin == NULL)
		return;

	// force to add every bones to the array.
	// This is useful when you've got various dummy bones(no vertices affected by it)
	// that are just there for the purpose of using them as hard points.
	int numBones = skin->GetNumBones();

	for (int i=0; i<numBones; i++)
	{
		INode* bone = skin->GetBone(i);

		// add found bone to the bones array
		if (boneNodeArray.Find(bone) == (uint32)-1)
		{
			U2DynString nodeName = node->GetName();
			U2DynString boneName = bone->GetName();
			FILE_LOG(logDEBUG) << 
				TSTR("node : ") << nodeName.Str() <<
				TSTR(" -> bone : ") << boneName.Str();
			boneNodeArray.AddElem(bone);
		}
		this->AddBoneToNode(node, bone);
	}
}

void U2MaxAnimExport::Clear()
{
	std::map<int, U2SkinController*>::iterator iter = m_skinCtrlMap.begin();
	for(; iter != m_skinCtrlMap.end(); ++iter)
	{
		U2SkinController* pCtrl = iter->second;
		U2_DELETE pCtrl;
		pCtrl = NULL;
	}

	m_skinCtrlMap.clear();	

	for(int skelIdx=0; skelIdx < m_skeletons.Size(); ++skelIdx)
	{
		for(int i=0; i < m_skeletons[skelIdx]->Size(); ++i)
		{
			Bone* pBone = m_skeletons[skelIdx]->GetElem(i);
			U2_DELETE pBone;
		}	
		m_skeletons[skelIdx]->RemoveAll();
		U2_DELETE m_skeletons[skelIdx];
	}

	m_skeletons.RemoveAll();

	m_nodeToBoneMap.clear();
	m_nodeToSkelMap.clear();
	m_boneToSkelMap.clear();	
}



//---------------------------------------------------------------------------
/**
The way collecting nodes of bone type is a bit tricky.

3dsmax support any type of object to be bone, so it is not enough
to check an object which type is bone or biped.
So, we check an object which has any physique or skin modifier and 
retrieve bones from it via physique(or skin) interface.

It is accomplished by two ways to collect bones from the given scene.
First, we check modifier of the geometry node's and collect bones from the
modifier. Second, we collect bones by its class ID if the class ID is one of
the bone's or biped's one.
Functions for each of the ways are GetBoneByModifier() and GetBoneByClassID().

Next thing to do is find the root bone from collected bones. The closet bone
node from scene root is selected for root bone. (Note that the root bone is
not always a child of scene root node)

Example:
@verbatim
scene root
bone A
bone B
bone C
constraint
bone D
@endverbatim

In the above example, if the bone A is not used for the animation, the root
bone is bone B.

And last we build bone array and indices of its elements which will be used 
for joint indices of vertices. 
At this time, only collected bones are added to the bone array.
An artist can add constraint or something other which is not a bone to a bone 
hierarchy and it should be filtered out when the bone array is built.

*/
bool U2MaxAnimExport::BuildBones(Interface* pIf, INode* pMaxNode)
{
	this->Clear();

	INode* root = pIf->GetRootNode();
	U2ASSERT(root == pMaxNode);

	INode* pMaxRoot = pMaxNode;

	U2PrimitiveVec<INode*> nodes;

	GetNodes(pMaxNode, nodes);

	for(int i=0; i < nodes.Size(); ++i)
	{
		FILE_LOG(logDEBUG) << TSTR("Nodes in Array : ") << nodes[i]->GetName();
	}

	U2PrimitiveVec<INode*> boneNodes;

	this->GetBoneByModifier(nodes, boneNodes);

	FILE_LOG(logDEBUG) << TSTR("Actally used number of bones : ") << boneNodes.Size();

	this->GetBoneByClassID(nodes, boneNodes);

	// if there is no bones in the scene, just return.
	if(boneNodes.Size() <= 0)
	{
		FILE_LOG(logDEBUG) << TSTR("The scene has no bones.");
		return true;
	}

	// get top most level of the bone in the bone array.
	// this bone will be used as the root bone.
	U2PrimitiveVec<INode*> rootBonesNodes;
	this->GetRootBones(pMaxRoot, boneNodes, rootBonesNodes);

	for(int rootBoneIdx=0; rootBoneIdx < rootBonesNodes.Size(); ++rootBoneIdx)
	{
		Skeleton* pSkel = U2_NEW Skeleton;
		m_skeletons.AddElem(pSkel);

		U2MaxNoteTrack* pNoteTrack = U2_NEW U2MaxNoteTrack(m_uiNumFrames);
		m_noteTracks.AddElem(pNoteTrack);

		ReconstructBoneHierarchy(-1, rootBoneIdx, 
			rootBonesNodes[rootBoneIdx], boneNodes);

		// extract animation state from note track.
		for(int ai=0; ai < this->GetNumBones(rootBoneIdx); ++ai)
		{
			Bone* pBone = (Bone*)&GetBone(rootBoneIdx, ai);
			FILE_LOG(logDEBUG) << TSTR("Bone name: ") << pBone->m_szName.Str() 
				<< TSTR("ID : ") << pBone->m_iThisId;

			INode* pMaxBone = pBone->m_pMaxNode;
			
			// BuildNoteTrack			
			m_noteTracks[rootBoneIdx]->GetAnimState(pMaxBone);
		}		

		// if there are no animation states, we add a default one.
		// (skin animator needs to have at least one)
		if(m_noteTracks[rootBoneIdx]->GetNumStates() <= 0)
		{
			int firstFrame = m_animStart / GetTicksPerFrame();
			int duration = (int)m_uiNumFrames;
			float fadeIntTime = 0.0f;
			m_noteTracks[rootBoneIdx]->AddAnimState(firstFrame, duration, fadeIntTime);
		}

		FILE_LOG(logDEBUG) << TSTR("Founded Bone Num :") <<  this->GetNumBones();		
	}


	std::map<INode*, INode*>::iterator iter = m_nodeToBoneMap.begin();
	std::map<INode*, int>::iterator fj;
	for (; iter != m_nodeToBoneMap.end(); ++iter) 
	{
		fj = m_boneToSkelMap.find(iter->second);
		if (fj != m_boneToSkelMap.end()) 
		{
			m_nodeToSkelMap[iter->first] = fj->second;
		}
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
int U2MaxAnimExport::FindBoneIdByNode(INode* pMaxNode)
{
	for(int skelIdx=0; skelIdx < m_skeletons.Size(); ++skelIdx)
	{
		for(int i=0; i < this->m_skeletons[skelIdx]->Size(); ++i)
		{
			if(m_skeletons[skelIdx]->GetElem(i)->m_pMaxNode == pMaxNode)
				return m_skeletons[skelIdx]->GetElem(i)->m_iThisId;
		}
	}

	return -1;
}

//-------------------------------------------------------------------------------------------------
int U2MaxAnimExport::FindBoneIdByName(const U2DynString& szBonename)
{
	for(int skelIdx=0; skelIdx < m_skeletons.Size(); ++skelIdx)
	{
		for(int i=0; i < this->m_skeletons[skelIdx]->Size(); ++i)
		{
			if(m_skeletons[skelIdx]->GetElem(i)->m_szName == szBonename)
				return m_skeletons[skelIdx]->GetElem(i)->m_iThisId;
		}
	}

	return -1;
}

//-------------------------------------------------------------------------------------------------
int U2MaxAnimExport::FindBoneIndexByNodeId(int nodeId)
{
	for(int i=0; i < m_skeletons[0]->Size(); ++i)
	{
		if(m_skeletons[0]->GetElem(i)->m_iThisId == nodeId)
		{
			return i;
		}
	}

	return -1;
}

//-------------------------------------------------------------------------------------------------
INode*	U2MaxAnimExport::FindBoneNodeByIndex(int idx)
{
	for(int i=0; i < m_skeletons[0]->Size(); ++i)
	{
		if(m_skeletons[0]->GetElem(i)->m_iThisId == idx)
		{
			return m_skeletons[0]->GetElem(i)->m_pMaxNode;
		}
	}

	return NULL;
}


//-------------------------------------------------------------------------------------------------
void U2MaxAnimExport::CalculateGlobalTimeRange( TimeValue& animStart, TimeValue& animEnd)
{
	Interface* pInterface = GetCOREInterface();
	INode* pNode = pInterface->GetRootNode();

	CalculateNodeTimeRange(pNode, animStart, animEnd);
	CalculateAnimatableTimeRange(pNode, animStart, animEnd);
}

//-------------------------------------------------------------------------------------------------
void U2MaxAnimExport::CalculateAnimatableTimeRange(Animatable* pkAnim, 
												   TimeValue& animStart, TimeValue& animEnd)
{
	if(pkAnim == NULL)
		return; 

	Interval interval = pkAnim->GetTimeRange(TIMERANGE_ALL | 
		TIMERANGE_CHILDNODES | TIMERANGE_CHILDANIMS);

	bool bFoundStartRange = false;
	bool bFoundEndRange = false;
	TimeValue newStart = interval.Start();
	TimeValue newEnd = interval.End();
	if(newStart == newEnd)
		return;

	if(newStart != TIME_NegInfinity && newStart != TIME_PosInfinity)
	{
		bFoundStartRange = true;
		if(animStart > newStart)
			animStart = newStart;
	}

	if(newEnd != TIME_PosInfinity && newEnd != TIME_NegInfinity)
	{
		bFoundEndRange = true;
		if(animEnd < newEnd)
			animEnd = newEnd;
	}


}

//-------------------------------------------------------------------------------------------------
void U2MaxAnimExport::CalculateNodeTimeRange(INode* pkAnim, 
								   TimeValue& animStart, TimeValue& animEnd)
{
	if(!pkAnim)
		return;

	if(pkAnim->IsAnimated())
	{

	}

	if(pkAnim->HasNoteTracks())
	{
		for(uint32 ui=0; ui < (uint32)pkAnim->NumNoteTracks(); ++ui)
		{
			DefNoteTrack* notetrack = 
				(DefNoteTrack*)pkAnim->GetNoteTrack(ui);
			if(notetrack)
			{
				for(int k=0; k < notetrack->keys.Count(); ++k)
				{
					NoteKey* notekey = *(notetrack->keys.Addr(k));
					if(notekey)
					{
						if(notekey->time < animStart)
							animStart = notekey->time;
						if(notekey->time > animEnd)
							animEnd = notekey->time;
					}
				}
			}
		}
	}

	for(uint32 ui=0; ui < (uint32)pkAnim->NumberOfChildren(); ++ui)
	{
		INode* pChild = pkAnim->GetChildNode(ui);
		CalculateNodeTimeRange(pChild, animStart, animEnd);
	}
}

//---------------------------------------------------------------------------
void U2MaxAnimExport::CalculateControlTimeRange(Control* pControl,
														TimeValue& animStart, 
														TimeValue& animEnd, 
														unsigned int& uiNumFrames)
{
	TimeValue tempStart = TIME_PosInfinity;
	TimeValue tempEnd = TIME_NegInfinity;
	CalculateAnimatableTimeRange(pControl, tempStart, tempEnd);

	if (tempStart != TIME_NegInfinity && tempStart != TIME_PosInfinity)
		animStart = tempStart;
	else
		animStart = ms_pSingleton->m_animStart;

	if (tempEnd != TIME_NegInfinity && tempEnd != TIME_PosInfinity)
		animEnd = tempEnd;
	else
		animEnd = ms_pSingleton->m_animEnd;

	uiNumFrames = ((animEnd - animStart) / GetTicksPerFrame()) + 1;
}

//-------------------------------------------------------------------------------------------------
void U2MaxAnimExport::BuildNoteTrack(INode* pMaxNode)
{
	if(pMaxNode->HasNoteTracks())
	{
		int iNumNoteTracks = pMaxNode->NumNoteTracks();
		int i = 0;
		unsigned int uiTotalNoteKeys = 0;
		// Calculate the total # of keys we need
		for (i = 0; i < iNumNoteTracks; i++)
		{
			Class_ID cID;

			NoteTrack* pNoteTrack = pMaxNode->GetNoteTrack(i);
			cID = pNoteTrack->ClassID();

			if (cID == Class_ID(NOTETRACK_CLASS_ID, 0))
			{
				DefNoteTrack *pNotes;

				pNotes = (DefNoteTrack *) pNoteTrack;
				uiTotalNoteKeys +=pNotes->NumKeys();
			}
		}
	}


	// Generate NoteTrack Data 
}

//-------------------------------------------------------------------------------------------------
bool U2MaxAnimExport::Export(int skelIndex, const TCHAR* animFileName)
{
	U2ASSERT(animFileName);
	U2ASSERT(skelIndex >= 0);

	U2AnimBuilder animBuilder;

	// default Setting 
	int iSampleRate = 2;
	float fKeyDuration = (float)iSampleRate / GetFrameRate();
	int iSceneFirstKey = 0;

	int numBones = GetNumBones(skelIndex);

	typedef U2PrimitiveVec<MaxSampleKey*> Keys;
	Keys keys;
	keys.Resize(numBones);

	U2PrimitiveVec<Keys*> keyArray;
	keyArray.Resize(numBones + 1);
	
	for(uint32 keyIdx=0; keyIdx < keyArray.Size(); ++keyIdx)
	{
		keyArray.SetElem(keyIdx, U2_NEW Keys);
	}

	for(int boneIdx=0; boneIdx < numBones; ++boneIdx)
	{
		const Bone& bone = GetBone(skelIndex, boneIdx);
		INode* pBoneNode = bone.m_pMaxNode;

		GetSampledKey(pBoneNode, keyArray[boneIdx], iSampleRate, CTRLTSTRM);
	}	

	// builds seletonal animations 
	U2MaxNoteTrack& noteTrack = GetNoteTrack(skelIndex);
	int numAnimStates = noteTrack.GetNumStates();

	for(int state=0; state < numAnimStates; ++state)
	{
		const U2MaxAnimState& animState = noteTrack.GetState(state);

		TimeValue stateStart = animState.m_iFirstFrame * GetTicksPerFrame();
		TimeValue stateEnd = animState.m_iDuration * GetTicksPerFrame();

		stateStart -= m_animStart;

		int numClips = animState.m_clips.Size();

		int firstKey = animState.m_iFirstFrame / iSampleRate;
		int numStateKeys = animState.m_iDuration / iSampleRate;
		int numClipKeys = numStateKeys / numClips;

		int startKey = (m_animStart / GetTicksPerFrame()) / iSampleRate;
		firstKey -= startKey;

		// do not add anim group, if the number of the state key or the clip keys are 0.
		if(numStateKeys <= 0 || numClipKeys <= 0)
		{		
			continue;
		}

		// determine animations loop type. 
		// the value is specified inside of nMaxNoteTrack::GetAnimState(). 
		// default is 'REPEAT'.

		U2AnimBuilder::Group* pAnimGroup = U2_NEW U2AnimBuilder::Group;

		switch(animState.m_iLoopType)
		{
		case U2AnimBuilder::Group::REPEAT:
			pAnimGroup->SetLoopType(U2AnimBuilder::Group::REPEAT);
			break;
		case U2AnimBuilder::Group::CLAMP:
			pAnimGroup->SetLoopType(U2AnimBuilder::Group::CLAMP);
			break;
		default:
			break;
		}

		pAnimGroup->SetKeyTime(fKeyDuration);
		pAnimGroup->SetNumKeys(numClipKeys);

		for(int clip=0; clip < numClips; ++clip)
		{
			int numBones = GetNumBones(skelIndex);

			for(int boneIdx=0; boneIdx < numBones; ++boneIdx)
			{
				U2PrimitiveVec<MaxSampleKey*>& tmpSamples  
					= *keyArray[boneIdx];

				U2AnimBuilder::Curve* pAnimCurveTrans	= U2_NEW U2AnimBuilder::Curve;
				U2AnimBuilder::Curve* pAnimCurveRot		= U2_NEW U2AnimBuilder::Curve; 
				U2AnimBuilder::Curve* pAnimCurveScale   = U2_NEW U2AnimBuilder::Curve; 

				pAnimCurveTrans->SetIpolType(U2AnimBuilder::Curve::LINEAR);
				pAnimCurveRot->SetIpolType(U2AnimBuilder::Curve::QUAT);
				pAnimCurveScale->SetIpolType(U2AnimBuilder::Curve::LINEAR);

				for(int clipKey=0; clipKey < numClipKeys; clipKey++)
				{
					U2AnimBuilder::Key* keyTrans = U2_NEW U2AnimBuilder::Key;
					U2AnimBuilder::Key* keyRot	 = U2_NEW U2AnimBuilder::Key;
					U2AnimBuilder::Key* keyScale	 = U2_NEW U2AnimBuilder::Key;

					int keyIdx = firstKey - iSceneFirstKey + clip * numClipKeys + clipKey;
					keyIdx = U2Math::Clamp<int>(keyIdx, 0, tmpSamples.Size() - 1);

					MaxSampleKey& skey = *tmpSamples[keyIdx];

					skey.tm.NoScale();

					AffineParts ap;
					decomp_affine(skey.tm, &ap);

					// Nebula Original 
					keyTrans->Set(D3DXVECTOR4(-ap.t.x, ap.t.z, ap.t.y, 0.f));
					keyRot->Set(D3DXVECTOR4(-ap.q.x, ap.q.z, ap.q.y, -ap.q.w));
					//keyTrans->Set(D3DXVECTOR4(ap.t.x, ap.t.z, ap.t.y, 0.f));
					//keyRot->Set(D3DXVECTOR4(ap.q.x, ap.q.z, ap.q.y, ap.q.w));
					keyScale->Set(D3DXVECTOR4(ap.k.x, ap.k.z, ap.k.y, 0.0f));
					
					pAnimCurveTrans->SetKey(clipKey, *keyTrans);					
					pAnimCurveRot->SetKey(clipKey, *keyRot);					
					pAnimCurveScale->SetKey(clipKey, *keyScale);


					//U2DynString str;
					//str.Format(TSTR("key trans : %f, %f, %f key rot : %f, %f, %f "
					//	"key Scale : %f, %f, %f \n"), keyTrans->value.x, 
					//	keyTrans->value.y, keyTrans->value.z, keyRot->value.x,
					//	keyRot->value.y, keyRot->value.z, keyRot->value.w, 
					//	keyScale->value.x, keyScale->value.y, keyScale->value.z);

					//FILE_LOG(logDEBUG) << str.Str();

				}
				

				pAnimGroup->AddCurve(*pAnimCurveTrans);
				pAnimGroup->AddCurve(*pAnimCurveRot);
				pAnimGroup->AddCurve(*pAnimCurveScale);

			}
			
		}

		animBuilder.AddGroup(*pAnimGroup);
	}

	FILE_LOG(logDEBUG) <<TSTR("Optimizing animation curves...");
	int numOptimizeCurves = animBuilder.Optimize();
	FILE_LOG(logDEBUG) << TSTR("Num of optimized curves : ") << numOptimizeCurves;

	animBuilder.FixKeyOffsets();

	if(animBuilder.Save(animFileName))
	{
		FILE_LOG(logDEBUG) << animFileName << TSTR(" animation file was saved.");
	}
	else 
	{
		FILE_LOG(logDEBUG) << TSTR("Faied to save ") << animFileName << 
			TSTR(" animation file");
		return false;
	}

	for(int i=0; i < keyArray.Size(); ++i)
	{
		Keys* pKeys = keyArray[i];

		for(int j=0; j < pKeys->Size(); ++j)
		{
			U2_DELETE pKeys->GetElem(j);
		}
		pKeys->RemoveAll();

		U2_DELETE pKeys;
	}

	keyArray.RemoveAll();

	return true;

}

//-----------------------------------------------------------------------------
/**
Samples the node TM controllers and specifies retrieved keys to the given 
sampleKeyArray.

- 21-Feb-05 kims Fixed transform scale.
*/

void U2MaxAnimExport::GetSampledKey(INode* pMaxNode, 
									U2PrimitiveVec<MaxSampleKey*> * sampleKeyArray, 
						  int sampleRate, MaxControlType type, bool optimize)
{
	if(type != CTRLTSTRM && 
		type != CTRL_POS && 
		type != CTRL_ROT && 
		type != CTRL_SCALE)
	{
		return;
	}

	TimeValue t;
	int iDelta = GetTicksPerFrame() * sampleRate;
	
	

	int numKeys  = (ms_pSingleton->m_animEnd - ms_pSingleton->m_animStart) / iDelta;

	sampleKeyArray->Resize(numKeys+1);

	for(t = ms_pSingleton->m_animStart; t < ms_pSingleton->m_animEnd; t+=iDelta)
	{
		MaxSampleKey* pSampleKey = U2_NEW MaxSampleKey;

		pSampleKey->tm = U2MaxSceneExport::GetLocalTM(pMaxNode, t);

		//transform scale
		float scale = U2MaxOptions::ms_fGeomScale; 
		if(scale != 0.0f)
		{
			Point3 scaleVal(scale, scale, scale);

			Matrix3 scaleTM;
			scaleTM.IdentityMatrix();
			scaleTM.Scale(scaleVal);

			pSampleKey->tm = pSampleKey->tm * scaleTM;
		}

		AffineParts ap;

		decomp_affine(pSampleKey->tm, &ap);

		if(type == CTRL_POS || type == CTRLTSTRM)
			pSampleKey->pos = ap.t;

		if(type == CTRL_ROT || type == CTRLTSTRM)
			pSampleKey->rot = ap.q;

		if(type == CTRL_SCALE || type == CTRLTSTRM)
			pSampleKey->scale = ap.k;

		pSampleKey->time = (t - ms_pSingleton->m_animStart) * SECONDSPERTICK;

	/*	U2DynString str;
		str.Format(TSTR("Pos : %f, %f, %f, Rot : %f, %f, %f %f Scale : %f, %f, %f"),
			pSampleKey->pos.x, pSampleKey->pos.y, pSampleKey->pos.z, 
			pSampleKey->rot.x, pSampleKey->rot.y, pSampleKey->rot.z, pSampleKey->rot.w,
			pSampleKey->scale.x, pSampleKey->scale.y, pSampleKey->scale.z);

		FILE_LOG(logDEBUG) << str.Str();*/

		sampleKeyArray->AddElem(pSampleKey);
	}

	// sample last key for exact looping.
	// end key should be inserted to get correct animation.
	{
		t = ms_pSingleton->m_animEnd;

		MaxSampleKey* pSampleKey = U2_NEW MaxSampleKey;

		pSampleKey->tm = U2MaxSceneExport::GetLocalTM(pMaxNode, t);

		//transform scale
		float scale = 1.0f; 
		if(scale != 0.0f)
		{
			Point3 scaleVal(scale, scale, scale);

			Matrix3 scaleTM;
			scaleTM.IdentityMatrix();
			scaleTM.Scale(scaleVal);

			pSampleKey->tm = pSampleKey->tm * scaleTM;
		}

		AffineParts ap;

		decomp_affine(pSampleKey->tm, &ap);

		if(type == CTRL_POS || type == CTRLTSTRM)
			pSampleKey->pos = ap.t;

		if(type == CTRL_ROT || type == CTRLTSTRM)
			pSampleKey->rot = ap.q;

		if(type == CTRL_SCALE || type == CTRLTSTRM)
			pSampleKey->scale = ap.k;

		pSampleKey->time = (t - ms_pSingleton->m_animStart) * SECONDSPERTICK;

		/*U2DynString str;
		str.Format(TSTR("Pos : %f, %f, %f, Rot : %f, %f, %f %f Scale : %f, %f, %f"),
			pSampleKey->pos.x, pSampleKey->pos.y, pSampleKey->pos.z, 
			pSampleKey->rot.x, pSampleKey->rot.y, pSampleKey->rot.z, pSampleKey->rot.w,
			pSampleKey->scale.x, pSampleKey->scale.y, pSampleKey->scale.z);

		FILE_LOG(logDEBUG) << str.Str();*/

		sampleKeyArray->AddElem(pSampleKey);		
	}

	if(optimize)
	{
		// Remove redundant jeys
		U2PrimitiveVec<MaxSampleKey*> tmpKeys;

		for(int i=0; i < sampleKeyArray->FilledSize(); ++i)
		{
			if(i==0 ||(i == sampleKeyArray->FilledSize() - 1))
			{
				// first and last keys are important
				tmpKeys.AddElem(sampleKeyArray->GetElem(i));
			}
			else 
			{
				// current key is important if keys on either side are different to it.
				MaxSampleKey* pPrevKey = sampleKeyArray->GetElem(i - 1);
				MaxSampleKey* pCurrKey = sampleKeyArray->GetElem(i);
				MaxSampleKey* pNextKey = sampleKeyArray->GetElem(i + 1);
				if(!AreKeysLerp(*pCurrKey, *pPrevKey, *pNextKey, type))
				{					
					tmpKeys.AddElem(pCurrKey);
				}

			}
		}
		
		sampleKeyArray->RemoveAll();

		sampleKeyArray->Resize(tmpKeys.FilledSize());

		for(int i=0; i < tmpKeys.FilledSize(); ++i)
		{
			sampleKeyArray->SetElem(i, tmpKeys[i]);
		}

		tmpKeys.RemoveAll();
	}


}

//-----------------------------------------------------------------------------
/**
Samples the float or point3 controllers and specifies retrieved keys to 
the given sampleKeyArray.

- 21-Feb-05 kims added optimize routine which remove a key if the key has
same value except time value to previous key.
*/
void U2MaxAnimExport::GetSampledKey(Control* control, 
									U2PrimitiveVec<MaxSampleKey*> & sampleKeyArray, 
						  int sampleRate, MaxControlType type, bool optimize)
{
	U2ASSERT(control);
	U2PrimitiveVec<TimeValue> sampleTimes;
	TimeValue t;
	TimeValue delta = GetTicksPerFrame() * sampleRate;
	int i;

	if(control->IsKeyable())
	{
		IKeyControl *keyControl = GetKeyControlInterface(control);
		U2ASSERT(keyControl);
		IKey *key = U2_ALLOC(IKey, keyControl->GetKeySize());
		int numKeys = keyControl->GetNumKeys();
		size_t lastOutTanType = size_t(-1);
		TimeValue lastKeyTime = ms_pSingleton->m_animStart;
		for (i = 0; i < numKeys; i++)
		{
			Interval interv;
			keyControl->GetKey(i, key);

			if( key->time < ms_pSingleton->m_animStart)
				continue;

			size_t inTanType = GetInTanType(key->flags);
			switch(lastOutTanType)
			{
			case BEZKEY_SMOOTH:
				{
					U2PrimitiveVec<TimeValue>& tmpSampleTimes 
						= *ms_pSingleton->SampleTime(lastKeyTime, key->time, delta);

					sampleTimes.Resize(tmpSampleTimes.Size());
					for(uint32 i=0; i < tmpSampleTimes.Size(); ++i)
					{
						sampleTimes.SetElem(i, tmpSampleTimes[i]);
					}

					U2_DELETE &tmpSampleTimes;
				}
				break;
			case BEZKEY_LINEAR:
				{
					// both linear tangent type, could ignore them
					if(inTanType == BEZKEY_LINEAR )
						break;
					else 
					{
						U2PrimitiveVec<TimeValue>& tmpSampleTimes 
							= *ms_pSingleton->SampleTime(lastKeyTime, key->time, delta);

						sampleTimes.Resize(tmpSampleTimes.Size());
						for(uint32 i=0; i < tmpSampleTimes.Size(); ++i)
						{
							sampleTimes.SetElem(i, tmpSampleTimes[i]);
						}

						U2_DELETE &tmpSampleTimes;

					}
				}
				break;
			case BEZKEY_STEP:
				sampleTimes.AddElem(U2Math::Max<TimeValue>(lastKeyTime, key->time -1));
				break;									
			case BEZKEY_FAST:
			case BEZKEY_SLOW:
			case BEZKEY_USER:
			case BEZKEY_FLAT:
				{
					U2PrimitiveVec<TimeValue>& tmpSampleTimes 
						= *ms_pSingleton->SampleTime(lastKeyTime, key->time, delta);

					sampleTimes.Resize(tmpSampleTimes.Size());
					for(uint32 i=0; i < tmpSampleTimes.Size(); ++i)
					{
						sampleTimes.SetElem(i, tmpSampleTimes[i]);
					}

					U2_DELETE &tmpSampleTimes;

				}
				break;
			case size_t(-1):
				break;
			default:
				U2ASSERT(TSTR("Invalid tangent type"));
				break;
			}

			sampleTimes.AddElem(key->time);

			lastOutTanType = GetOutTanType(key->flags);
			lastKeyTime = key->time;
		}

		U2_FREE(key);
		key = NULL;
	}
	else 
	{
		U2PrimitiveVec<TimeValue>* tmpSampleTimes 
			= ms_pSingleton->SampleTime(ms_pSingleton->m_animStart, ms_pSingleton->m_animEnd, delta);

		sampleTimes.Resize(tmpSampleTimes->FilledSize());
		for(uint32 i=0; i < tmpSampleTimes->FilledSize(); ++i)
		{
			sampleTimes.SetElem(i, tmpSampleTimes->GetElem(i));
		}

		U2_DELETE tmpSampleTimes;
		tmpSampleTimes = NULL;
		
	}

	// inset start frame
	if(sampleTimes[0] > ms_pSingleton->m_animStart)
	{
		TimeValue t = ms_pSingleton->m_animStart;
		
		U2PrimitiveVec<TimeValue> tmpSampleTimes;
		tmpSampleTimes.Resize(sampleTimes.Size());
		
		for(int i=0; i < sampleTimes.FilledSize(); ++i)
		{
			tmpSampleTimes.SetElem(i, sampleTimes[i]);
		}
		sampleTimes.RemoveAll();
		sampleTimes.Resize(tmpSampleTimes.FilledSize() + 1);
		sampleTimes.SetElem(0, t);
		for(int i=1; i < sampleTimes.FilledSize(); ++i)
		{
			sampleTimes.SetElem(i, tmpSampleTimes[i-1]);
		}
	}

	// sample last key for exact looping.
	sampleTimes.AddElem(ms_pSingleton->m_animEnd);

	// get samples
	int numKeys = sampleTimes.Size();
	TimeValue lastKeyTime = ms_pSingleton->m_animStart - 1;
	for (i = 0; i < numKeys; i++)
	{
		t = sampleTimes[i];
		if (t == lastKeyTime) // ignore same time
			continue;

		lastKeyTime = t;

		MaxSampleKey* sampleKey = U2_NEW MaxSampleKey;
		Interval interv;
		switch(type)
		{
		case CTRL_FLOAT:
			{
				float value;
				control->GetValue(t, &value, interv, CTRL_ABSOLUTE);

				sampleKey->fval = value;
			}
			break;
		case CTRL_POINT3:
			{
				Point3 value;
				control->GetValue(t, &value, interv);

				sampleKey->pos = value;
			}
			break;
		case CTRL_POINT4:
			{
				Point4 value;
				control->GetValue(t, &value, interv);

				sampleKey->pt4 = value;
			}
			break;
		}

		sampleKey->time = (t - ms_pSingleton->m_animStart) * SECONDSPERTICK;
		sampleKeyArray.AddElem(sampleKey);
	}

	if(optimize && sampleKeyArray.Size() > 2)
	{
		U2PrimitiveVec<MaxSampleKey*> tmpKeys;

		// first and last keys are important		
		tmpKeys.AddElem(sampleKeyArray[0]);

		for(i=1; i < sampleKeyArray.Size() - 1; ++i)
		{
			// current key is important if keys on either side are different to it.
			const MaxSampleKey &previousKey = *sampleKeyArray[i - 1];
			const MaxSampleKey &currentKey = *sampleKeyArray[i];
			const MaxSampleKey &nextKey = *sampleKeyArray[i + 1];

			if(!AreKeysLerp(currentKey, previousKey, nextKey, type))
			{
				tmpKeys.AddElem((MaxSampleKey*)&currentKey);
			}

		}
		// first and last keys are important
		tmpKeys.AddElem(sampleKeyArray[sampleKeyArray.Size()-1]);

		sampleKeyArray.RemoveAll();
		sampleKeyArray.Resize(tmpKeys.Size());

		for(i=0; i < sampleKeyArray.Size(); ++i)
		{
			sampleKeyArray.SetElem(i, tmpKeys[i]);
		}

		tmpKeys.RemoveAll();
	}	
}

//-------------------------------------------------------------------------------------------------
U2MaxAnimExport::MaxKeyType U2MaxAnimExport::GetKeyType(Control *control)
{
	U2ASSERT(control);

	ulong partA = control->ClassID().PartA();

	switch(partA)
	{
	case TCBINTERP_FLOAT_CLASS_ID:       return TCB_FLOAT;
	case TCBINTERP_POSITION_CLASS_ID:    return TCB_POS;
	case TCBINTERP_ROTATION_CLASS_ID:    return TCB_ROT;
	case TCBINTERP_SCALE_CLASS_ID:       return TCB_SCALE;
	case TCBINTERP_POINT3_CLASS_ID:      return TCB_POINT3;
	case HYBRIDINTERP_FLOAT_CLASS_ID:    return HYBRID_FLOAT;
	case HYBRIDINTERP_POSITION_CLASS_ID: return HYBRID_POS;
	case HYBRIDINTERP_ROTATION_CLASS_ID: return HYBRID_ROT;
	case HYBRIDINTERP_SCALE_CLASS_ID:    return HYBRID_SCALE;
	case HYBRIDINTERP_POINT3_CLASS_ID:   return HYBRID_POINT3;
	case LININTERP_FLOAT_CLASS_ID:       return LINEAR_FLOAT;
	case LININTERP_POSITION_CLASS_ID:    return LINEAR_POS;
	case LININTERP_ROTATION_CLASS_ID:    return LINEAR_ROT;
	case LININTERP_SCALE_CLASS_ID:       return LINEAR_SCALE;
	case EULER_CONTROL_CLASS_ID:
	case LOCAL_EULER_CONTROL_CLASS_ID:   return EULER_ROT;
	default:
		return UNKNOWN;
	}

}

//-------------------------------------------------------------------------------------------------
bool U2MaxAnimExport::AreKeysEqual(const MaxSampleKey& key1, const MaxSampleKey& key2, 
						 ControlType type)
{
	switch (type)
	{
	case CTRL_POINT4:
		// 3dsmax sdk, Point4 has no const Equals method
		return const_cast<Point4&>(key1.pt4).Equals(key2.pt4) != 0;
	case CTRL_POINT3:
		return key1.pos.Equals(key2.pos) != 0;
	case CTRL_FLOAT:
		return key1.fval == key2.fval;
	case CTRL_POS:
		return key1.pos.Equals(key2.pos) != 0;
	case CTRL_SCALE:
		return key1.scale.Equals(key2.scale) != 0;
	case CTRL_ROT:
		return key1.rot.Equals(key2.rot) != 0;
	case CTRLTSTRM:
		return key1.pos.Equals(key2.pos) != 0 && 
			key1.scale.Equals(key2.scale) != 0 && key1.rot.Equals(key2.rot) != 0;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
bool U2MaxAnimExport::AreKeysLerp(const MaxSampleKey& key, const MaxSampleKey& key1, 
						const MaxSampleKey& key2, MaxControlType type)
{
	float lerpVal = (key.time - key1.time) / (key2.time - key1.time);
	switch (type)
	{
	case CTRL_POINT4:
		{
			const D3DXVECTOR4 p(key.pt4.x, key.pt4.y, key.pt4.z, key.pt4.w);
			const D3DXVECTOR4 p1(key1.pt4.x, key1.pt4.y, key1.pt4.z, key1.pt4.w);
			const D3DXVECTOR4 p2(key2.pt4.x, key2.pt4.y, key2.pt4.z, key2.pt4.w);
			return AreKeysLerp(p, p1, p2, lerpVal);
		}
		break;
	case CTRL_ROT:
		{
			const D3DXVECTOR4 p(key.rot.x, key.rot.y, key.rot.z, key.rot.w);
			const D3DXVECTOR4 p1(key1.rot.x, key1.rot.y, key1.rot.z, key1.rot.w);
			const D3DXVECTOR4 p2(key2.rot.x, key2.rot.y, key2.rot.z, key2.rot.w);
			return AreKeysLerp(p, p1, p2, lerpVal);
		}
		break;
	case CTRL_SCALE:
		{
			const D3DXVECTOR3 p(key.scale.x, key.scale.y, key.scale.z);
			const D3DXVECTOR3 p1(key1.scale.x, key1.scale.y, key1.scale.z);
			const D3DXVECTOR3 p2(key2.scale.x, key2.scale.y, key2.scale.z);
			return AreKeysLerp(p, p1, p2, lerpVal);
		}
		break;
	case CTRL_POS:
		// nMaxPos is acctually the same with nMaxPoint3
	case CTRL_POINT3:
		{
			const D3DXVECTOR3 p(key.pos.x, key.pos.y, key.pos.z);
			const D3DXVECTOR3 p1(key1.pos.x, key1.pos.y, key1.pos.z);
			const D3DXVECTOR3 p2(key2.pos.x, key2.pos.y, key2.pos.z);
			return AreKeysLerp(p, p1, p2, lerpVal);
		}
		break;
	case CTRL_FLOAT:
		{
			float p = key.fval;
			float p1 = key1.fval;
			float p2 = key2.fval;
			float ptmp = U2Math::U2Lerp(p1, p2, lerpVal);
			// use proportion tolerance instead of neubla2 vector's difference tolerance
			return fabs(p - ptmp) <= ms_fKeyTolerance;
		}
		break;
	case CTRLTSTRM:
		{
			return AreKeysLerp(key, key1, key2, CTRL_POS) &&
				AreKeysLerp(key, key1, key2, CTRL_ROT) &&
				AreKeysLerp(key, key1, key2, CTRL_SCALE);
		}
		break;
	}
	return false;

}

//-------------------------------------------------------------------------------------------------
bool U2MaxAnimExport::AreKeysLerp(const D3DXVECTOR4& key, const D3DXVECTOR4& key1,
						const D3DXVECTOR4& key2, float lerpVal)
{
	D3DXVECTOR4 tmp;
	D3DXVec4Lerp(&tmp, &key1, &key2, lerpVal);

	// use proportion tolerance instead of neubla2 vector's difference tolerance
	D3DXVECTOR4 tolerance(key * ms_fKeyTolerance);
	
	return U2Math::FAbs(key.x - tmp.x) <= tolerance.x && U2Math::FAbs(key.y - tmp.y) <= tolerance.y &&
		 U2Math::FAbs(key.z - tmp.z) <= tolerance.z && U2Math::FAbs(key.w - tmp.w) <= tolerance.w;

}

//-------------------------------------------------------------------------------------------------
bool U2MaxAnimExport::AreKeysLerp(const D3DXVECTOR3& key, const D3DXVECTOR3& key1, 
						const D3DXVECTOR3& key2, float lerpVal)
{
	D3DXVECTOR3 tmp;
	D3DXVec3Lerp(&tmp, &key1, &key2, lerpVal);

	// use proportion tolerance instead of neubla2 vector's difference tolerance
	D3DXVECTOR3 tolerance(key * ms_fKeyTolerance);

	return U2Math::FAbs(key.x - tmp.x) <= tolerance.x && U2Math::FAbs(key.y - tmp.y) <= tolerance.y &&
		U2Math::FAbs(key.z - tmp.z) <= tolerance.z ;
}

//-------------------------------------------------------------------------------------------------
U2PrimitiveVec<TimeValue>* U2MaxAnimExport::SampleTime(TimeValue start, 
													   TimeValue end, TimeValue delta)
{
	U2ASSERT(start < end && delta > 0);
	U2PrimitiveVec<TimeValue>* pArray = U2_NEW U2PrimitiveVec<TimeValue>();

	for(TimeValue t = start + delta * 0.5f; t < end; t += delta)
		pArray->AddElem(t);

	if(pArray->FilledSize() == 0)
	{
		pArray->AddElem((start + end) * 0.5f);
	}
	return pArray;	
}

//-------------------------------------------------------------------------------------------------
U2SkinController* U2MaxAnimExport::ConvertSkinAnim(int skelIdx, const TCHAR* szCtrlName, 
												   const TCHAR* szAnimFilename)
{
	U2ASSERT(skelIdx >= 0);
	U2ASSERT(szCtrlName);
	U2ASSERT(szAnimFilename);

	U2SkinController* pNewSkinCtrl = U2_NEW U2SkinController;
	U2ASSERT(pNewSkinCtrl);
	

	U2PrimitiveVec<Bone*>& bones = GetBoneArray(skelIdx);

	ConvertBones(pNewSkinCtrl, bones);
	pNewSkinCtrl->SetName(szCtrlName);
	pNewSkinCtrl->SetLoopType(U2AnimLoopType::LOOP);
	

	pNewSkinCtrl->SetAnim(szAnimFilename);

	// Build anmation state
	U2MaxNoteTrack& noteTrack = GetNoteTrack(skelIdx);
	ConvertAnimClips(pNewSkinCtrl, noteTrack);	

	return pNewSkinCtrl;
}

//-------------------------------------------------------------------------------------------------
void U2MaxAnimExport::ConvertBones(U2SkinController* pCtrl, const U2PrimitiveVec<Bone*>& bones)
{
	Matrix3 localTM;
	AffineParts ap;

	int numBones = bones.Size();

	pCtrl->BeginJoints(numBones);

	float fScale = U2MaxOptions::ms_fGeomScale;
	for(int i = 0; i < numBones; ++i)
	{
		const Bone& bone = *bones[i];

		localTM = bone.m_localTM;

		if(fScale != 0.f)
		{
			Point3 scaleValue(fScale, fScale, fScale);

			Matrix3 scaleTM;
			scaleTM.IdentityMatrix();
			scaleTM.Scale(scaleValue);

			localTM = localTM * scaleTM;
		}

		localTM.NoScale();

		decomp_affine(localTM, &ap);

		D3DXVECTOR3 poseTrans (-ap.t.x, ap.t.z, ap.t.y);
		D3DXQUATERNION poseRot(-ap.q.x, ap.q.z, ap.q.y, -ap.q.w);
		D3DXVECTOR3 poseScale(ap.k.x, ap.k.z, ap.k.y);

		pCtrl->SetJoint(bone.m_iThisId, bone.m_iParentId,poseTrans, poseRot, 
			poseScale, bone.m_szName);
	}

	pCtrl->EndJoints();


}

void U2MaxAnimExport::ConvertAnimClips(U2SkinController* pCtrl, U2MaxNoteTrack& noteTrack)
{
	int numClips = noteTrack.GetNumStates();
	pCtrl->BeginClips(numClips);

	U2DynString clipName;

	for(int i=0; i < numClips; ++i)
	{
		const U2MaxAnimState& clip = noteTrack.GetState(i);

		// force to get clip name if it does not have any.
		if(clipName.IsEmpty())
		{
			clipName = U2DynString(TSTR("default"));
			clipName.AppendInt(i);			
		}
		else 
		{
			clipName = clip.m_szName;
		}

		pCtrl->SetClip(i, i, clip.m_szName);
	}
}

//-------------------------------------------------------------------------------------------------
void U2MaxAnimExport::ConvertToU2Controller(INode *pMaxNode)
{
	U2ASSERT(pMaxNode);

	// A node has pysique or skin modifier is not affected by key frame animation 
	// in 3dsmax. So we just Ignore animations if the node has physique or skin modifier. 
	// An exported node which has both physique and key frame animation rendered 
	// in wrong position in Nebula2.
	// TODO: Is there any other modifier except physique or skin?
	if(U2MaxUtil::FindPhysique(pMaxNode) || U2MaxUtil::FindSkin(pMaxNode))
	{
		return;
	}

	Control* control = pMaxNode->GetTMController();

	if(!control)
	{
		return;
	}

	Control *posControl, *rotControl, *scaleControl;

	posControl = control->GetPositionController();
	rotControl = control->GetRotationController();
	scaleControl = control->GetScaleController();

	if(posControl || rotControl || scaleControl)
	{
		// we have one of the animation controller at least.
		Class_ID classId = control->ClassID();

		if(classId == BIPBODY_CONTROL_CLASS_ID || 
			classId == BIPSLAVE_CONTROL_CLASS_ID ||
			classId == FOOTPRINT_CLASS_ID )
		{
			return;
		}

		if(classId == IKCONTROL_CLASS_ID || classId == IKCHAINCONTROL_CLASS_ID)
		{

		}

		if(classId == IKSLAVE_CLASSID || 
			(posControl && posControl->ClassID() == IKSLAVE_CLASSID) || 
			(rotControl && rotControl->ClassID() == IKSLAVE_CLASSID))
		{
			// the control is IK control.

		}

		if(posControl && posControl->ClassID() == Class_ID(PATH_CONTROL_CLASS_ID, 0))
		{

		}
		else if(posControl || rotControl || scaleControl)		
		{


		}

	}
	else 
	{
		FILE_LOG(logDEBUG) << pMaxNode->GetName() << 
			TSTR(" node has Control but it does not contain have any PRS controller."); 

		return;	

	}

	return;
}
