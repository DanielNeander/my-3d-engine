/**************************************************************************************************
module	:	U2MaxAnimExport
Author	:	Yun sangyong
Desc	:	BuildNoteTrack 미구현...
			convertSkinAnim, convertBones 미구현
*************************************************************************************************/
#pragma once
#ifndef U2_MAXANIMEXPORT_H
#define U2_MAXANIMEXPORT_H

#include "U2MaxLib.h"


class U2MaxNoteTrack;
class U2SkinController;

class U2MaxAnimExport : public U2MemObj
{

public:
	enum MaxControlType 
	{
		CTRL_POS,        /// Position Controller
		CTRL_ROT,        /// Rotation Controller
		CTRL_SCALE,      /// Scale Controller
		CTRL_FLOAT,      /// Float Controller
		CTRL_POINT3,     /// Point3 Controller
		CTRLTSTRM,         /// Used for sampling the node transformation matrix.  
		CTRL_EULERX,     /// Euler X controller
		CTRL_EULERY,     /// Euler Y controller
		CTRL_EULERZ,     /// Euler Z controller
		CTRL_POINT4,     /// Point4 based controllers - This is available from 3ds max 6.
	};

	enum MaxKeyType 
	{
		TCB_FLOAT,
		TCB_POS,
		TCB_ROT,
		TCB_SCALE,
		TCB_POINT3,
		HYBRID_FLOAT,
		HYBRID_POS,
		HYBRID_ROT,
		HYBRID_SCALE,
		HYBRID_POINT3,
		LINEAR_FLOAT,
		LINEAR_POS,
		LINEAR_ROT,
		LINEAR_SCALE,
		EULER_ROT,
		UNKNOWN,
	};


	class Bone : public U2MemObj
	{
	public:
		Bone();

		int m_iParentId;
		int m_iThisId;
		U2DynString m_szName;
		INode* m_pMaxNode;

		Matrix3 m_localTM;
		Matrix3 m_worldTM;

		bool m_bDummy;
	};

	//-----------------------------------------------------------------------------
	/**
	@class nMaxSampleKey
	@ingroup NebulaMaxExport2Contrib

	@brief A class for sampled key of a control.

	@note
	pos, rot, scale members are decomposed for nMaxTM for easy to use.
	*/
	struct MaxSampleKey : public U2MemObj
	{
		Matrix3    tm;    // use it for nMaxTM
		Point3     pos;   // use it for nMaxPos and nMaxPoint3
		Quat       rot;   // use if for nMaxRot
		Point3     scale; // use it for nMaxScale
		Point4     pt4;      // use if for nMaxPoint4
		float      fval;  // use it for nMaxFloat

		float      time;  // time in seconds 
	};

	static U2MaxAnimExport* Instance();

	void Init(TimeValue animStart, TimeValue animEnd);

	U2SkinController* ConvertSkinAnim(int skelIdx, const TCHAR* szCtrlName, 
		const TCHAR* szAnimFilename);
	void ConvertBones(U2SkinController* pCtrl, const U2PrimitiveVec<Bone*>& bones);
	void ConvertAnimClips(U2SkinController* pCtrl, U2MaxNoteTrack& noteTrack);

	void ConvertToU2Controller(INode *pMaxNode);

	void ConvertPosition(Control*, U2TransformInterpController);
	


	static void CalculateGlobalTimeRange( TimeValue& animStart, TimeValue& animEnd);
	
	static void CalculateNodeTimeRange(INode* pkAnim, 
		TimeValue& animStart, TimeValue& animEnd);

	static void CalculateAnimatableTimeRange(Animatable* pkAnim, 
		TimeValue& animStart, TimeValue& animEnd);

	static void CalculateControlTimeRange(Control* pControl,
		TimeValue& animStart, 
		TimeValue& animEnd, 
		unsigned int& uiNumFrames);
	
	bool BuildBones(Interface* pIf, INode* pMaxNode);
	bool Export(int skelIndex, const TCHAR* animFileName);
	// @}

	/// Retrieves the number of bones which are actually used in the animation.
	int GetNumBones() const;
	int GetNumBones(int skelIndex) const;
	int GetNumSkeletons() const;

	void BuildNoteTrack(INode* pMaxNode);

	void Clear();
	
	
	int FindBoneIdByNode(INode* pMaxNode);
	int FindBoneIdByName(const U2DynString& szBonename);
	int FindBoneIndexByNodeId(int nodeId);
	INode*	FindBoneNodeByIndex(int idx);

	const Bone& GetBone(int skellIdx, int idx);

	U2MaxNoteTrack& GetNoteTrack(int skelIdx);


	U2PrimitiveVec<U2MaxAnimExport::Bone*>& GetBoneArray(int seklIdx);

	int GetSkeletonForNode(INode* pMaxNode);


	static void GetSampledKey(INode* node, U2PrimitiveVec<MaxSampleKey*>* sampleKeyArray, 
		int sampleRate, MaxControlType type, bool optimize = false);

	static void GetSampledKey(Control* control, U2PrimitiveVec<MaxSampleKey*> & sampleKeyArray, 
		int sampleRate, MaxControlType type, bool optimize = false);

	static MaxKeyType GetKeyType(Control *control);

	static bool AreKeysEqual(const MaxSampleKey& key1, const MaxSampleKey& key2, 
		ControlType type);

	static bool AreKeysLerp(const MaxSampleKey& key, const MaxSampleKey& key1, 
		const MaxSampleKey& key2, MaxControlType type);

	void CreateU2Controller(INode *pMaxNode);

	static const float ms_fKeyTolerance;
	std::map<int, U2SkinController*> m_skinCtrlMap;

protected:	

	static bool AreKeysLerp(const D3DXVECTOR4& key, const D3DXVECTOR4& key1,
		const D3DXVECTOR4& key2, float lerpVal);
	static bool AreKeysLerp(const D3DXVECTOR3& key, const D3DXVECTOR3& key1, 
		const D3DXVECTOR3& key2, float lerpVal);
	U2PrimitiveVec<TimeValue>* SampleTime(TimeValue start, TimeValue end, TimeValue delta);

	void GetNodes(INode* node, U2PrimitiveVec<INode*>& nodeArray);


	void GetBoneByModifier(const U2PrimitiveVec<INode*>& nodeArray, U2PrimitiveVec<INode*> &boneNodeArray);
	void GetBoneByClassID(const U2PrimitiveVec<INode*>& nodeArray, U2PrimitiveVec<INode*> &boneNodeArray);

	void ExtractPhysiqueBones(INode* node, Modifier* phyMod, ObjectState* os, 
		U2PrimitiveVec<INode*> &boneNodeArray);
	void ExtractSkinBones(INode* node, Modifier* skinMod,U2PrimitiveVec<INode*> &boneNodeArray);
	
	int GetRootBones(INode *sceneRoot, U2PrimitiveVec<INode*> &boneNodeArray, U2PrimitiveVec<INode*> &rootBonesNodeArray);
	void ReconstructBoneHierarchy(int parentID, int skeletion, INode* node, U2PrimitiveVec<INode*> &boneNodeArray);
	// @}

	void AddBoneToNode(INode* inode, INode* bone);


	

private:	
	U2MaxAnimExport();
	U2MaxAnimExport(const U2MaxAnimExport& );

	static U2MaxAnimExport* ms_pSingleton;


protected:
	typedef U2PrimitiveVec<Bone*> Skeleton;
	U2PrimitiveVec<Skeleton*> m_skeletons;
	std::map<INode*, INode*>	m_nodeToBoneMap;
	std::map<INode*, int>		m_nodeToSkelMap;
	std::map<INode*, int>		m_boneToSkelMap;

	/// note track object which we retrieves states and clips.
	U2PrimitiveVec<U2MaxNoteTrack*> m_noteTracks;

	TimeValue m_animStart, m_animEnd;	
	unsigned int m_uiNumFrames;

	

};

//---------------------------------------------------------------------------
inline
U2MaxAnimExport* U2MaxAnimExport::Instance()
{
	if (0 == ms_pSingleton)
	{
		return U2_NEW U2MaxAnimExport();
	}
	return ms_pSingleton;
}

//-------------------------------------------------------------------------------------------------
inline int U2MaxAnimExport::GetNumBones() const
{
	int num = 0;
	for(int i=0; i < m_skeletons.FilledSize(); ++i)
	{
		num += m_skeletons[i]->FilledSize();
	}
	return num;
}

//-------------------------------------------------------------------------------------------------
inline int U2MaxAnimExport::GetNumBones(int skel) const
{
	U2ASSERT(0 <= skel && skel < m_skeletons.Size());
	return m_skeletons[skel]->FilledSize();
}

//-------------------------------------------------------------------------------------------------
inline const U2MaxAnimExport::Bone& 
U2MaxAnimExport::GetBone(int skelIdx, int idx)
{
	U2ASSERT(idx >= 0);
	U2ASSERT(skelIdx >= 0);
	return *m_skeletons[skelIdx]->GetElem(idx);
}

//-------------------------------------------------------------------------------------------------
inline U2PrimitiveVec<U2MaxAnimExport::Bone*>& 
U2MaxAnimExport::GetBoneArray(int skelIdx)
{
	U2ASSERT(skelIdx >= 0);
	return *m_skeletons[skelIdx];
}

//-------------------------------------------------------------------------------------------------
inline int U2MaxAnimExport::GetNumSkeletons() const
{
	return m_skeletons.FilledSize();
}

//-------------------------------------------------------------------------------------------------
inline U2MaxNoteTrack& U2MaxAnimExport::GetNoteTrack(int skelIdx)
{
	return *m_noteTracks[skelIdx];
}


#endif