#include <U2_3D/src/U23DLibPCH.h>
#include "U2AnimData.h"
#include "U2SkinController.h"


U2ObjVec<U2AnimKeyTrackPtr> U2AnimData::ms_resultAnimKeyTacks;
D3DXVECTOR4 U2AnimData::ms_scratchKeys[MAX_INTERPKEYS];
D3DXVECTOR4 U2AnimData::ms_keys[MAX_INTERPKEYS];
D3DXVECTOR4 U2AnimData::ms_trasnsitionKeys[MAX_INTERPKEYS];

U2AnimData::U2AnimData() 
	:m_bAnimEnabled(true),
	m_uiLastEvalFrameId(0),
	m_pCurStateInfo(NULL),
	m_pPrevStateInfo(NULL),
	m_spSkinCtrl(NULL)
{
	m_pSkel = U2_NEW U2Skeleton();

}


U2AnimData::U2AnimData(const U2AnimData& src) 
	:m_bAnimEnabled(true)	
{
	m_pSkel = U2_NEW U2Skeleton();

	*this = src;	
}

U2AnimData::~U2AnimData()
{	

	U2_DELETE m_pSkel;
	m_pSkel = 0;
	SetSkinController(0);
}



void U2AnimData::SetSkinController(U2SkinController* pCtrl)
{
	if(m_spSkinCtrl)
	{
		m_spSkinCtrl = 0;
	}
	if(pCtrl)
	{
		m_spSkinCtrl = pCtrl;		
	}

}


void U2AnimData::SetActivateState(const U2AnimStateInfo& newState)
{
	if(m_pPrevStateInfo)
		U2_DELETE m_pPrevStateInfo;
	
	m_pPrevStateInfo = m_pCurStateInfo;

	m_pCurStateInfo = (U2AnimStateInfo*)&newState;

}


void U2AnimData::EvaluateSkeleton(float fTime)
{
	if( 0 == m_spAnim->GetNumInterpKeyGroups())
	{
		U2ASSERT(false);
		FILE_LOG(logDEBUG) << _T("No KeyGroups");
		return;
	}	

	if(IsAnimEnabled() && m_pCurStateInfo->IsValid())
	{
		U2ASSERT(m_spAnim);

		// check if a state transition is necessary
		float fCurRelTime = fTime - m_pCurStateInfo->GetStateStarted();

		// handle time exception (this happens when time is reset to a smaller value
		// since the last animation state switch)
		if(fCurRelTime < 0.f)
		{
			fCurRelTime = 0.f;
			m_pCurStateInfo->SetStateStarted(fTime);
		}

		int numClips = m_pCurStateInfo->GetNumClips();
		int clipIdx;
		for(clipIdx = 0; clipIdx < numClips; ++clipIdx)
		{
			if(m_pCurStateInfo->GetClip(clipIdx).GetClipname() != U2DynString(_T("baseClip")))
			{			

				int idx = m_pCurStateInfo->GetClip(clipIdx).GetAnimGroupIdx();
				const U2Animation::InterpKeyGroup& group = m_spAnim->GetInterpKeyGroup(0);

				float fFadeInTime = m_pCurStateInfo->GetFadeInTime();
				float fLerp = 1.f;
				bool bTransition = false;
				if(fFadeInTime > 0.f && fCurRelTime < fFadeInTime && 
					m_pPrevStateInfo->IsValid())
				{
					// state transition is necessary, compute a lerp value
					// and sample the previous animation state
					float fPrevRelTime = fTime - m_pPrevStateInfo->GetStateStarted();
					float fSampleTime = fPrevRelTime + m_pPrevStateInfo->GetStateOffset();
					if(Sample(*m_pPrevStateInfo, fSampleTime, 
						ms_trasnsitionKeys, ms_scratchKeys, MAX_INTERPKEYS))
					{
						bTransition = true;
						fLerp = fCurRelTime / fFadeInTime;
					}
				}

				// get samples from current animation state
				float fSampleTime = fCurRelTime + m_pCurStateInfo->GetStateOffset();
				if(Sample(*m_pCurStateInfo, fSampleTime, ms_keys, ms_scratchKeys, MAX_INTERPKEYS))
				{					

					// transfer the sampled animation values into the character skeleton
					int numJoints = m_pSkel->GetNumJoints();
					int iJointIdx;
					const D3DXVECTOR4* pKeyBase = ms_keys;
					const D3DXVECTOR4* pPrevKeyBase = ms_trasnsitionKeys;

					D3DXVECTOR3 trans, prevTrans;
					D3DXQUATERNION rot, prevRot;
					D3DXVECTOR3 scale, prevScale;
					for(iJointIdx = 0; iJointIdx < numJoints; ++iJointIdx)
					{
						// read sampled translation, rotation and scale
						trans.x = pKeyBase->x; trans.y = pKeyBase->y; trans.z = pKeyBase->z; 
						pKeyBase++;
						rot = D3DXQUATERNION(pKeyBase->x, pKeyBase->y, pKeyBase->z, pKeyBase->w);
						pKeyBase++;
						scale.x = pKeyBase->x; scale.y = pKeyBase->y; scale.z =pKeyBase->z; 
						pKeyBase++;

						if(bTransition)
						{
							prevTrans = D3DXVECTOR3(pPrevKeyBase->x, pPrevKeyBase->y, pPrevKeyBase->z); 
							pPrevKeyBase++;
							prevRot = D3DXQUATERNION(pPrevKeyBase->x, pPrevKeyBase->y, 
								pPrevKeyBase->z, pPrevKeyBase->w);
							pPrevKeyBase++;
							prevScale = D3DXVECTOR3(prevTrans.x, prevTrans.y, prevTrans.z);
							pPrevKeyBase++;
							D3DXVECTOR3 tempTrans = trans;
							D3DXVec3Lerp(&trans, &prevTrans, &tempTrans, fLerp);

							D3DXQUATERNION normPrevRot, normRot;
							D3DXQuaternionNormalize(&normRot, &rot);
							D3DXQuaternionNormalize(&normPrevRot, &prevRot);

							D3DXQuaternionSlerp(&rot, &normPrevRot, &normRot, fLerp);

							D3DXVECTOR3 tempScale = scale;
							D3DXVec3Lerp(&scale, &prevScale, &tempScale, fLerp);
						}

						U2Bone& joint = m_pSkel->GetJoint(iJointIdx);						
						joint.SetTrans(trans);
						joint.SetRot(rot);
						joint.SetScale(scale);
					}
				}
			}
		}
	}
	m_pSkel->Evaluate();
}

/**
Emit animation event for the current animation states.
*/
void U2AnimData::EmitAnimEvents(float fStartTime, float fStopTime)
{
	
}


//------------------------------------------------------------------------------
/**
Sample the complete animation state at a given time into the
provided float4 array. The size of the provided key array must be
equal to the number of curves in any animation clip in the state (all clips
must have the same number of curves).

@param  time            the time at which to sample
@param  keyArray        pointer to a float4 array which will be filled with
the sampled values (one per curve)
@param  keyArraySize    number of elements in the key array, must be identical
to the number of curves in any animation clip
@return                 true, if the returned keys are valid (false if all
clip weights are zero)
*/
bool U2AnimData::Sample(const U2AnimStateInfo& stateInfo, float fTime, 
			D3DXVECTOR4* pKeyArray, D3DXVECTOR4* pScratchKeyArray, int keyArraySize)
{
	U2ASSERT(pKeyArray);
	U2ASSERT(keyArraySize >= stateInfo.GetClip(0).GetNumInterpKeys());
	U2ASSERT(m_spAnim);

	static D3DXQUATERNION currQuat;
	static D3DXQUATERNION accumQuat;
	static D3DXQUATERNION slerpQuat;

	float fAccumWeight = 0.f;
	int iClipIdx;
	const int iNumClips = stateInfo.GetNumClips();
	for(iClipIdx=0; iClipIdx < iNumClips; ++iClipIdx)
	{
		const U2AnimClip& clip = stateInfo.GetClip(iClipIdx);
		const float fClipWeight = stateInfo.GetClipWeight(iClipIdx);
		const float fScaledWeight = fClipWeight / (fAccumWeight + fClipWeight); 
		const int iAnimGroupIdx = clip.GetAnimGroupIdx();
		const U2Animation::InterpKeyGroup& group = m_spAnim->GetInterpKeyGroup(iAnimGroupIdx);
		const int iNumIterpKeys = group.GetNumInterpKeys();

		// obtain sampled curve value for the clip's animation curve range
		m_spAnim->SampeInterpKeys(fTime, iAnimGroupIdx, 0, iNumIterpKeys, pScratchKeyArray);

		int iInterpKeyIdx;
		for(iInterpKeyIdx=0; iInterpKeyIdx < iNumIterpKeys; ++iInterpKeyIdx)
		{
			D3DXVECTOR4& curMixedKey = pKeyArray[iInterpKeyIdx];
			const D3DXVECTOR4& curSampleKey = pScratchKeyArray[iInterpKeyIdx];

			// perform weighted blending
			U2Animation::InterpKey& animCurve = m_spAnim->GetInterpKeyGroup(iAnimGroupIdx)
				.GetInterpKey(iInterpKeyIdx);
			if(animCurve.IsAnimated() && fClipWeight > 0.f)
			{
				// FIXME: (for cases with more than two clips) maybe all weights of animated curves
				// have to be summed up (for every frame)

				// check all curves from previous clips at current curve index if they are animated
				bool bAnim = false;
				int i;
				for(i=0; i < iClipIdx; ++i)
				{
					U2AnimClip& clip = stateInfo.GetClip(i);
					int iAnimGroupIdx = clip.GetAnimGroupIdx();
					U2Animation::InterpKey& prevClipCurve = m_spAnim->GetInterpKeyGroup(iAnimGroupIdx)
						.GetInterpKey(iInterpKeyIdx);
					if(prevClipCurve.IsAnimated())
					{
						bAnim = true;
						break;
					}
				}

				if(!bAnim)
				{
					// no previous curves are animated; take my current sample key
					if(animCurve.GetInterpType() == U2Animation::InterpKey::INTERP_QUAT)
					{
						curMixedKey = curSampleKey;
					}
					else 
						curMixedKey = curSampleKey * fClipWeight;
				}
				else 
				{
					// lerp my current sample key with previous curves
					if(animCurve.GetInterpType() == U2Animation::InterpKey::INTERP_QUAT)
					{
						currQuat = D3DXQUATERNION((float*)&curSampleKey);
						accumQuat = D3DXQUATERNION((float*)&curMixedKey);
						D3DXQuaternionSlerp(&slerpQuat, &accumQuat, &currQuat, fScaledWeight);
						curMixedKey = D3DXVECTOR4((float*)&slerpQuat);
					}
					else 
						curMixedKey += curSampleKey * fClipWeight;
				}

				animCurve.SetCurAnimClipValue(curSampleKey);
			}
			else // curve is not animated
			{
				// FIXME: (for cases with more than two clips) maybe all weights of animated curves
				// have to be summed up (for every frame)

				// check all curves from previous clips at current curve index if they are animated
				D3DXVECTOR4 startVal;
				bool bAnim = false;
				int i;
				for(i=0; i < iClipIdx; ++i)
				{
					U2AnimClip& clip = stateInfo.GetClip(i);
					int iAnimGroupIdx = clip.GetAnimGroupIdx();
					U2Animation::InterpKey& prevClipCurve = m_spAnim->
						GetInterpKeyGroup(iAnimGroupIdx).GetInterpKey(iInterpKeyIdx);
					if(prevClipCurve.IsAnimated())
					{
						// start value from prev animated curve taken
						bAnim = true;
						startVal = prevClipCurve.GetCurAnimClipValue();
					}
				}

				if(!bAnim)
				{
					// no other curves are animated, take the start value of the current curve
					if(animCurve.GetInterpType() == U2Animation::InterpKey::INTERP_QUAT)
					{
						curMixedKey = animCurve.GetStartValue();
					}
					else 
					{
						curMixedKey = animCurve.GetStartValue() * fClipWeight;
					}
				}
				else 
				{
					// lerp my start value key with previous curves
					if(animCurve.GetInterpType() == U2Animation::InterpKey::INTERP_QUAT)
					{
						currQuat = D3DXQUATERNION((float*)&startVal);
						accumQuat = D3DXQUATERNION((float*)&curMixedKey);
						D3DXQuaternionSlerp(&slerpQuat, &accumQuat, &currQuat, fScaledWeight);
						curMixedKey = D3DXVECTOR4((float*)&slerpQuat);
					}
					else 
						curMixedKey += startVal * fClipWeight;
				}
			}
		}
		fAccumWeight += fClipWeight;
	}

	return true;
}


//------------------------------------------------------------------------------
/**
This emits animation events through the given animation event handler
from the provided start time to the provided end time (should be
the last time that the animation has been sampled, and the current time.

Animation event transformations will be weight-mixed.
*/
void U2AnimData::EmitAnimEvents(const U2AnimStateInfo& stateInfo, float fStartTime, 
								float fEndTime)
{
	float fTimeDiff = fEndTime - fStartTime;
	if(fTimeDiff <= 0.f || fTimeDiff > 0.25f)
	{
		return;
	}

	BeginEmitEvents();

	int iClipIdx;
	int numClips = stateInfo.GetNumClips();
	for(iClipIdx=0; iClipIdx < numClips; ++iClipIdx)
	{
		const U2AnimClip& clip = stateInfo.GetClip(iClipIdx);
		const float fClipWeight = stateInfo.GetClipWeight(iClipIdx);
		const U2Animation::InterpKeyGroup& animGroup = 
			m_spAnim->GetInterpKeyGroup(stateInfo.GetClip(iClipIdx).GetAnimGroupIdx());

		if(fClipWeight > 0.0f)
		{
			int numTracks = clip.GetNumAimKeyTracks();
			int trackIdx;
			for(trackIdx =0; trackIdx < numTracks; ++trackIdx)
			{
				const U2AnimKeyTrack& track = clip.GetAnimKeyTrack(trackIdx);
				int numEventKeys = track.GetNumKeyDatas();
				int eventIdx;
				for(eventIdx=0; eventIdx < numEventKeys; ++eventIdx)
				{
					const U2AnimKeyData& eventKey = track.GetKeyData(eventIdx);
					if(animGroup.IsBetweenTimes(eventKey.GetTime(), fStartTime, fEndTime))
					{
						AddEmitEvent(track, track.GetKeyData(eventIdx), fClipWeight);
					}
				}
			}
		}
	}

	EndEmitEvents();
}


//------------------------------------------------------------------------------
/**
Begin blended event emission. Transformations of animation events may
be blended just as normal animation curves. The rule is that identically
named event tracks between several clips will be blended together if
their events have the same timestamp.
*/
void U2AnimData::BeginEmitEvents()
{
	ms_resultAnimKeyTacks.RemoveAll();
}


//------------------------------------------------------------------------------
/**
This adds a new, possibly blended animation event.
*/
void U2AnimData::AddEmitEvent(const U2AnimKeyTrack& track, const U2AnimKeyData& eventKey, 
							  float fWeight)
{
	
	int iTrackIdx;
	int iNumTracks = ms_resultAnimKeyTacks.Size();
	for(iTrackIdx=0; iTrackIdx < iNumTracks; ++iTrackIdx)
	{
		if(ms_resultAnimKeyTacks[iTrackIdx]->GetName() == track.GetName())
		{
			break;
		}
	}
	if(iTrackIdx == iNumTracks)
	{
		U2AnimKeyTrack *pNewTrack = U2_NEW U2AnimKeyTrack;
		pNewTrack->SetName(track.GetName());
		ms_resultAnimKeyTacks.AddElem(pNewTrack);
	}

	// check if event must be blended, this is the case if an event with an
	// identical time stamp already exists in this track
	int iEventIdx;
	int iNumEventKeys = ms_resultAnimKeyTacks[iTrackIdx]->GetNumKeyDatas();
	for(iEventIdx=0; iEventIdx < iNumEventKeys; ++iEventIdx)
	{
		float fTime0 = ms_resultAnimKeyTacks[iTrackIdx]->GetKeyData(iEventIdx).GetTime();
		float fTime1 = eventKey.GetTime();
		if(U2Math::FEqual(fTime0, fTime1, 0.01f))
		{
			break;
		}
	}
	if(iEventIdx == iNumEventKeys)
	{
		U2AnimKeyData* pNewKeyData = U2_NEW U2AnimKeyData;
		pNewKeyData->SetTime(eventKey.GetTime());
		pNewKeyData->SetTrans(eventKey.GetTrans() * fWeight);
		pNewKeyData->SetScale(eventKey.GetScale() * fWeight);
		pNewKeyData->SetQuatRot(eventKey.GetQuatRot());
		
		ms_resultAnimKeyTacks[iTrackIdx]->AddKeyData(*pNewKeyData);
	}
	else 
	{
		U2AnimKeyData* pBlendEventKey = ms_resultAnimKeyTacks[iTrackIdx]->
			KeyDataArray()[iEventIdx];
		pBlendEventKey->SetTrans(pBlendEventKey->GetTrans() + eventKey.GetTrans() * fWeight);
		pBlendEventKey->SetScale(pBlendEventKey->GetScale() + eventKey.GetScale() * fWeight);
	}
}


//------------------------------------------------------------------------------
/**
Finish defining animation event emission. This is where the events
will actually be transferred to the animation event handler.
*/
void U2AnimData::EndEmitEvents()
{
	int iNumTracks = ms_resultAnimKeyTacks.Size();
	int iTrackIdx;
	for(iTrackIdx=0; iTrackIdx < iNumTracks; ++iTrackIdx)
	{
		int numEventKeys = ms_resultAnimKeyTacks[iTrackIdx]->GetNumKeyDatas();
		int iEventIdx;
		for(iEventIdx=0; iEventIdx < numEventKeys; ++iEventIdx)
		{
			// Handle Event
		}		
	}
}


