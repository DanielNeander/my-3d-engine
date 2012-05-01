#include <U2_3D/Src/U23DLibPCH.h>
#include "U2MemAnimation.h"

U2MemAnimation::U2MemAnimation() 
:m_keyArray(0, 0)
{

}


U2MemAnimation::~U2MemAnimation()
{
	for(unsigned int i = 0; i < m_keyArray.Size(); ++i)
	{
		U2_DELETE m_keyArray[i];
	}
	m_keyArray.RemoveAll();
}


bool U2MemAnimation::LoadResource()
{
	bool bSuccess = false;

	#define BUFSIZE MAX_PATH 
	
	U2FilePath fPath;				
	TCHAR fullPath[MAX_PATH];
	_tcscpy_s(fullPath, MAX_PATH, GetName().Str());
	//fPath.ConvertToAbs(fullPath, MAX_PATH * sizeof(TCHAR), ANIM_DIR);

	U2Filename filename(fullPath);
	
	if(0 == _tcscmp(filename.GetExt(), _T(".xanim")))
	{		
		bSuccess = LoadU2TextAnim(fullPath);
	}
	

	return bSuccess;
}


void U2MemAnimation::UnloadResource()
{

}


bool U2MemAnimation::LoadU2TextAnim(const U2DynString& filename)
{
	U2WinFile* file = U2_NEW U2WinFile;
	U2ASSERT(file);

	if(file->Open(filename, _T("r")))
	{
		const int LINE_SIZE = 1024;
		TCHAR line[LINE_SIZE];
		TCHAR* szNextToken;

		int groupIdx = 0;
		int curveIdx = 0;
		int keyIdx = 0;
		InterpKeyGroup* pCurrGroup = 0;
		InterpKey* pCurrInterpKey = 0;

		static D3DXVECTOR4 sVec4 = VECTOR4_ZERO;

		while(file->GetS(line,sizeof(line)))
		{
			TCHAR* szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);
			if(0 == szKeyword)
				continue;
			else if(0 == _tcscmp(szKeyword, _T("type")))
			{
				const TCHAR* szType = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				U2ASSERT(szType);
				if(0 != _tcscmp(szType,_T("xanim")))
				{
					file->Close();
					U2_DELETE file;
					return false;
				}
			}
			else if(0 == _tcscmp(szKeyword, _T("numgroups")))
			{
				const TCHAR* szNumGroups = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				U2ASSERT(szNumGroups);

				int numGroups = atoi(szNumGroups);

				if(0 == numGroups)
				{
					U2ASSERT(false);
					file->Close();
					U2_DELETE file;
					return false;
				}

				SetNumInterpKeyGroups(numGroups);				
			}
			else if(0 == _tcscmp(szKeyword, _T("numkeys")))
			{
				const TCHAR* szNumKeys = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				U2ASSERT(szNumKeys);
				m_keyArray.Resize(atoi(szNumKeys));
			}
			else if(0 == _tcscmp(szKeyword, _T("group")))
			{
				const TCHAR* szNumInterpKeys = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szStartKey = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szNumKeys = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szKeyStride = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szKeyTime = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szFadeInFrames = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szAnimLoopType = _tcstok_s(0, U2_WHITESPACE, &szNextToken);

				U2ASSERT(szNumInterpKeys && szStartKey && szNumKeys && szKeyStride 
					&& szKeyTime && szFadeInFrames && szAnimLoopType);

				curveIdx = 0;
				pCurrGroup = &GetInterpKeyGroup(groupIdx++);
				pCurrGroup->SetNumInterpKeys(atoi(szNumInterpKeys));
				pCurrGroup->SetStartKey(atoi(szStartKey));
				pCurrGroup->SetNumKeys(atoi(szNumKeys));
				pCurrGroup->SetKeyStride(atoi(szKeyStride));
				pCurrGroup->SetKeyTime(float(atof(szKeyTime)));
				pCurrGroup->SetFadeInFrames(float(atof(szFadeInFrames)));
				pCurrGroup->SetLoopType(U2Animation::InterpKeyGroup::StrToLoopType(szAnimLoopType));		
			}
			else if(0 == _tcscmp(szKeyword, _T("curve")))
			{
				const TCHAR* szInterpType = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szFirstKeyIdx = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szIsAnimated = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szConstX = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szConstY = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szConstZ = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szConstW = _tcstok_s(0, U2_WHITESPACE, &szNextToken);

				// Check
				U2ASSERT(pCurrGroup);
				pCurrInterpKey = &(pCurrGroup->GetInterpKey(curveIdx++));
				pCurrInterpKey->SetInterpType(U2Animation::InterpKey::StrToInterpType(szInterpType));
				pCurrInterpKey->SetFirstKeyIdx(atoi(szFirstKeyIdx));
				pCurrInterpKey->SetAnimated(atoi(szIsAnimated));

				sVec4.x = float(atof(szConstX));
				sVec4.y = float(atof(szConstY));
				sVec4.z = float(atof(szConstZ));
				sVec4.w = float(atof(szConstW));								
				pCurrInterpKey->SetConstValue(sVec4);
			}
			else if(0 == _tcscmp(szKeyword, _T("key")))
			{
				const TCHAR* szKeyX = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szKeyY = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szKeyZ = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szKeyW = _tcstok_s(0, U2_WHITESPACE, &szNextToken);

				sVec4.x = float(atof(szKeyX));
				sVec4.y = float(atof(szKeyY));
				sVec4.z = float(atof(szKeyZ));
				sVec4.w = float(atof(szKeyW));

				m_keyArray[keyIdx++]= U2_NEW U2AnimKey(sVec4);
			}
			else 
			{
				U2ASSERT(false);
				file->Close();
				U2_DELETE file;
				return false;
			}
		}

		file->Close();
		U2_DELETE file;
		return true;
	}

	FILE_LOG(logDEBUG) << _T("Open Failed");
	

	return false;
}


bool U2MemAnimation::LoadU2BinaryAnim(const U2DynString& filename)
{
	return true;
}


//------------------------------------------------------------------------------
/**
Samples the current values for a number of curves in the given
animation group. The sampled values will be written to a client provided
vector4 array.

@param  time                a point in time
@param  groupIndex          index of animation group to sample from
@param  firstCurveIndex     group-relative curve index of first curve to sample
@param  numCurves           number of curves to sample
@param  dstKeyArray         pointer to vector4 array with numCurves element which
will be filled with the results
- 18-Oct-2004   floh    Fixed collapsed curve check (now checks if start key
is -1, instead of the curveIpolType). The curve ipol type
MUST be set to something sane even for collapsed curves,
because it's used at various places for deviding whether
quaternion-interpolation must be used instead of simple
linear interpolation!!
*/
void U2MemAnimation::SampeInterpKeys(float fTime, int iInterpGroupIdx, 		
								int firstInterpKeyIdx, int numInterpKeys, D3DXVECTOR4* pDstKeyArray)
{
	const InterpKeyGroup& group = GetInterpKeyGroup(iInterpGroupIdx);
	int startKey = group.GetStartKey();
	float frameTime = startKey * group.GetKeyTime();
	int keyIdx[2];
	int startKeyIdx[2];
	float fStartInBetween;
	float fInBetween;

	group.TimeToIdx(0.0f, startKeyIdx[0], startKeyIdx[1], fStartInBetween);
	group.TimeToIdx(fTime, keyIdx[0], keyIdx[1], fInBetween);

	int i;
	static D3DXQUATERNION q0;
	static D3DXQUATERNION q1;
	static D3DXQUATERNION q;
	int animCnt = 0;
	for(i=0; i < numInterpKeys; ++i)
	{
		InterpKey& curve = group.GetInterpKey(i + firstInterpKeyIdx);

		if(curve.GetFirstKeyIdx() == -1)
		{
			// a collapsed curve
			pDstKeyArray[i] = curve.GetConstValue();
			curve.SetStartValue(curve.GetConstValue());
		}
		else 
		{
			switch(curve.GetInterpType())
			{
			case InterpKey::INTERP_STEP:
				{
					int idx0 = curve.GetFirstKeyIdx()+ keyIdx[0];
					pDstKeyArray[i] = m_keyArray[idx0]->m_key;

					idx0 = curve.GetFirstKeyIdx();
					curve.SetStartValue(m_keyArray[idx0]->m_key);
				}
				break;			
			case InterpKey::INTERP_QUAT:
				{
					int curveFirstKeyIdx = curve.GetFirstKeyIdx();
					int idx0 = curveFirstKeyIdx + keyIdx[0];
					int idx1 = curveFirstKeyIdx + keyIdx[1];
					D3DXQUATERNION key0 = m_keyArray[idx0]->m_key;
					D3DXQUATERNION key1 = m_keyArray[idx1]->m_key;
					q0 = D3DXQUATERNION(key0.x, key0.y, key0.z, key0.w);					
					q1 = D3DXQUATERNION(key1.x, key1.y, key1.z, key1.w);
					D3DXQuaternionSlerp(&q, &q0, &q1, fInBetween);
					pDstKeyArray[i] = D3DXVECTOR4(q.x, q.y, q.z, q.w);

					idx0 = curveFirstKeyIdx + startKeyIdx[0];
					idx1 = curveFirstKeyIdx + startKeyIdx[1];
					key0 = (float*)&m_keyArray[idx0]->m_key;
					key1 = (float*)&m_keyArray[idx1]->m_key;
					q0 = D3DXQUATERNION(key0.x, key0.y, key0.z, key0.w);					
					q1 = D3DXQUATERNION(key1.x, key1.y, key1.z, key1.w);
					D3DXQuaternionSlerp(&q, &q0, &q1, fInBetween);
					D3DXVECTOR4 val(q.x, q.y, q.z, q.w);
					curve.SetConstValue(val);
				}
				break;
			case InterpKey::INTERP_LINEAR:
				{
					int curveFirstKeyIdx = curve.GetFirstKeyIdx();
					int idx0 = curveFirstKeyIdx + keyIdx[0];
					int idx1 = curveFirstKeyIdx + keyIdx[1];
					const D3DXVECTOR4& v0 = m_keyArray[idx0]->m_key;
					const D3DXVECTOR4& v1 = m_keyArray[idx1]->m_key;
					pDstKeyArray[i] = v0 + ((v1 - v0) * fInBetween);

					idx0 = curveFirstKeyIdx + startKeyIdx[0];
					idx1 = curveFirstKeyIdx + startKeyIdx[1];
					D3DXVECTOR4 v2 = m_keyArray[idx0]->m_key;
					D3DXVECTOR4 v3 = m_keyArray[idx1]->m_key;
					curve.SetStartValue(v2 + ((v3 - v2) * fStartInBetween));
				}
				break;
			default:
				U2ASSERT(FALSE);
				FDebug("U2MemoryAnimation::SampleCurves(): invalid curveIpolType %d!", 
					curve.GetInterpType());
				break;
			}
		}		
	}
}


uint32 U2MemAnimation::GetByteInSize()
{	
	return m_keyArray.FilledSize() * sizeof(D3DXVECTOR4);
}
