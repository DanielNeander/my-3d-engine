#include <U2_3D/Src/U23DLibPCH.h>
#include "U2SkinController.h"
#include "U2AnimFactory.h"
#include "U2AnimClip.h"


IMPLEMENT_RTTI(U2SkinController, U2Controller);
//-------------------------------------------------------------------------------------------------
U2SkinController::U2SkinController() 
:m_bAnimEnabled(true),
m_pAnim(0),
m_iAnimVarIdx(-1),
m_iAnimSetIdx(-1)
{
	m_pAnimData = U2_NEW U2AnimData();

}

//-------------------------------------------------------------------------------------------------
U2SkinController::~U2SkinController()
{
	UnloadAnim();
	U2_DELETE m_pAnimData;
	m_pAnimData = 0;
}

//-------------------------------------------------------------------------------------------------
void U2SkinController::Terminate(U2RenderContext* pCxt)
{
	U2Variable var = pCxt->GetLocalVar(m_iAnimVarIdx);
	U2AnimData* pCurAnimData = (U2AnimData*)var.GetObj();
	U2ASSERT(pCurAnimData);
	U2_DELETE pCurAnimData;
	
	var = pCxt->GetLocalVar(m_iAnimSetIdx);
	U2SkinCharSet* pCharSet = (U2SkinCharSet*)var.GetObj();
	U2ASSERT(pCharSet);
	U2_DELETE pCharSet;
}

//-------------------------------------------------------------------------------------------------
void U2SkinController::Initialize(U2RenderContext* pCxt)
{
	if(LoadAnim())
	{
		U2AnimData* pCurAnimData = U2_NEW U2AnimData(*m_pAnimData);
		U2ASSERT(pCurAnimData);
		pCurAnimData->SetSkinController(this);

		if(0 != m_clips.FilledSize())
		{
			U2AnimStateInfo* pAnimStateInfo = U2_NEW U2AnimStateInfo;
			pAnimStateInfo->SetStateStarted(0.0f);
			pAnimStateInfo->BeginClips(1);
			pAnimStateInfo->SetClip(0, GetClip(0), 1.0f);
			pAnimStateInfo->EndClips();
			pCurAnimData->SetActivateState(*pAnimStateInfo);
		}

		U2SkinCharSet* pCharSet = U2_NEW U2SkinCharSet;
		U2ASSERT(pCharSet);

		U2Variable::Handle charSetHandle = U2VariableMgr::Instance()->
			GetVariableHandleByName(_T("charSetPointer"));
		m_iAnimSetIdx = pCxt->AddLocalVar(U2_NEW U2Variable(charSetHandle, pCharSet));
		U2Variable::Handle charHandle = U2VariableMgr::Instance()->
			GetVariableHandleByName(_T("charPointer"));
		m_iAnimVarIdx =pCxt->AddLocalVar(U2_NEW U2Variable(charHandle, pCurAnimData));		

		U2Controller::Initialize(pCxt);
	}	
}

void U2SkinController::UnloadAnim()
{
	U2_DELETE m_pAnim;
	m_pAnim = 0;
}

bool U2SkinController::LoadAnim()
{
	if(!m_pAnim && m_szAnimname.Str())
	{
		U2Animation* pAnim = U2AnimFactory::Instance()->CreateMemAnim(m_szAnimname);
		U2ASSERT(pAnim);

		U2FilePath fPath;		
		TCHAR fullPath[MAX_PATH];
		// StackString Memory Leak...
		U2DynString includePath(MODEL_PATH);	

		fPath.ConvertToAbs(fullPath, MAX_PATH, m_szAnimname, includePath);		

		pAnim->SetName(fullPath);
		
		if(pAnim)
		{
			if(!pAnim->LoadResource())
			{
				FDebug("U2SkinController: Error loading anim file '%s'\n", this->m_szAnimname.Str());
				U2_DELETE pAnim;
				return false;
			}			
		}
		m_pAnim = pAnim;
		m_pAnimData->SetAnim(pAnim);

		FILE_LOG(logDEBUG) << _T("AnimLoad : Success...");

		return true;
	}
	return false;		
}


//bool LoadResources()
//virtual void UnloadResources();

bool U2SkinController::Update(float fTime, U2RenderContext* pCxt)
{	
	if( m_iAnimVarIdx == -1 )
		return false;
		
	U2SkinModifier* pSkinMod = (U2SkinModifier*)pCxt->pUserData;
	U2ASSERT(pSkinMod);
		
	const U2Variable& charVar = pCxt->GetLocalVar(m_iAnimVarIdx);
	U2AnimData* pCurAnimData = (U2AnimData*)charVar.GetObj();
	U2ASSERT(pCurAnimData);

	// update the animation enabled Flag
	pCurAnimData->SetAnimEnabled(m_bAnimEnabled);

	uint32 curFrameId = pCxt->GetFrameID();

	if(pCurAnimData->GetLastEvalFrameId() != curFrameId)
	{
		pCurAnimData->SetLastEvalFrameId(curFrameId);

		  // get the sample time from the render context
		U2Variable* pVar = pCxt->GetVariable(m_channelVarHandle);		
		assert(pVar);
		
		float curTime = pVar->GetFloat();
		
		// get the time offset from the render context
		pVar = pCxt->GetVariable(m_channelOffsetVarHandle);						
		float curOffset = pVar != 0 ? pVar->GetFloat() : 0.0f;

		

		
		const U2Variable& charSetVar = pCxt->GetLocalVar(m_iAnimSetIdx);
		U2SkinCharSet* pCharSet = (U2SkinCharSet*)charSetVar.GetObj();
		assert(pCharSet);		
		
		
		// get character 2 set from render context and check if animation state needs to be updated
		if(pCharSet->IsDirty())		
		{
			U2AnimStateInfo* pNewState = U2_NEW U2AnimStateInfo;
			int numClips = pCharSet->GetNumClips();

			float fWeightSum = 0.0f;
			for(int i=0; i < numClips; ++i)
			{
				fWeightSum += pCharSet->GetClipWeight(i);
			}

			// Add Clips
			if(fWeightSum > 0.0f)
			{
				pNewState->SetStateStarted(curTime);
				pNewState->SetFadeInTime(pCharSet->GetFadeInTime());
				pNewState->BeginClips(numClips);
				for(int i=0; i < numClips; ++i)
				{
					int idx = GetClipIdx(pCharSet->GetClipname(i));
					if(-1 == idx)
					{						
						FDebug("nSkinAnimator::Animate(): Requested clip \"%s\" does not exist.\n", 
							pCharSet->GetClipname(i).Str());						

						FILE_LOG(logDEBUG) << _T("Faield GetClipIdx");
					}
					pNewState->SetClip(i, GetClip(idx), pCharSet->GetClipWeight(i) / fWeightSum);
				}
				pNewState->EndClips();
			}

			pCurAnimData->SetActivateState(*pNewState);
			pCharSet->SetDirty(false);
		}

		// evaluate the current state of the character skeleton		
		// 이 함수를 주석 처리하면 업데이트 되지 않음.
		pCurAnimData->EvaluateSkeleton(curTime);				
	}	

	// Shadow?
	pSkinMod->SetSkeleton(&pCurAnimData->GetSkeleton());	
	
	// Update Target Node 
	return true;

}


void U2SkinController::BeginClips(int numClips)
{
	m_clips.Resize(numClips);
}

void U2SkinController::SetClip(int clipIdx, int animGroupIdx, const U2DynString& szClipname)
{
	const uint32 numCurves = GetNumJoints() * 3;
	
	U2AnimClip* pNewClip = U2_NEW U2AnimClip(szClipname, animGroupIdx, numCurves);
	m_clips.SetElem((uint32)clipIdx, pNewClip);
}

void U2SkinController::EndClips()
{

}

int U2SkinController::GetNumClips() const
{
	return m_clips.Size();
}

const U2AnimClip& U2SkinController::GetClip(int clipIdx) const
{
	return *SmartPtrCast(U2AnimClip, m_clips[clipIdx]);
}

int U2SkinController::GetClipIdx(const U2DynString& name) const
{
	for(uint32 i=0; i < GetNumClips(); ++i)
	{
		if(m_clips[i]->GetClipname() == name)
		{
			return i;
		}
	}

	return -1;
}

double U2SkinController::GetClipDuration(int idx) const
{
	const U2AnimClip& clip = GetClip(idx);
	return m_pAnim->GetDuration(clip.GetAnimGroupIdx());
}


void U2SkinController::BeginAnimKeyTracks(int clipIdx, int numTracks)
{
	m_clips.GetElem(clipIdx)->SetNumAnimKeyTracks(numTracks);
}

void U2SkinController::BeginAnimKeyTracks(int clipIdx, int trackIdx, const U2DynString& name, int numKeys)
{
	U2FixedObjArray<U2AnimKeyTrack>& animKeyTracks = m_clips[clipIdx]->GetAnimKeyTracks();
	U2AnimKeyTrack& track = animKeyTracks[trackIdx];
	track.SetName(name);
	track.SetNumKeyDatas(numKeys);	
}

void U2SkinController::SetAnimKey(int clipIdx, int trackIdx, int keyIDx, float time, 
				const D3DXVECTOR3& trans, const D3DXQUATERNION& rot, const D3DXVECTOR3& scale)
{	
	U2FixedObjArray<U2AnimKeyTrack>& animKeyTracks = m_clips[clipIdx]->GetAnimKeyTracks();
	U2AnimKeyTrack& track = animKeyTracks[trackIdx];
	U2AnimKeyData *pNewKeyData = U2_NEW U2AnimKeyData;

	if((U2AnimKeyData*)&track.GetKeyData(keyIDx))
	{
		U2_DELETE (U2AnimKeyData*)&track.GetKeyData(keyIDx);
	}

	pNewKeyData->SetTime(time);
	pNewKeyData->SetTrans(trans);
	pNewKeyData->SetQuatRot(rot);
	pNewKeyData->SetScale(scale);
	track.SetKeyData(keyIDx, *pNewKeyData);
}

void U2SkinController::EndAnimKeyTrack(int clipIdx, int trackIdx)
{

}

void U2SkinController::EndAnimKeyTracks(int clipIdx)
{

}


int U2SkinController::GetJoint(const TCHAR* szJointname)
{
	return m_pAnimData->GetSkeleton().GetJointIdx(szJointname);
}