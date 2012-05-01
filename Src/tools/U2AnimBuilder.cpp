#include <U2_3D/src/U23DLibPCH.h>
#include "U2AnimBuilder.h"

//#include <U2_3D/src/Object/U2WinFile.h>


U2AnimBuilder::U2AnimBuilder()
{

}


U2AnimBuilder::~U2AnimBuilder()
{

}


void U2AnimBuilder::Clear()
{
	for(int i=0; i < groupArray.Size(); ++i)
	{
		U2_DELETE groupArray[i];
	}

	groupArray.RemoveAll();
}


//------------------------------------------------------------------------------
/**
Return the actual number of keys (not including the 'keys' in collapsed
curves).
*/
int U2AnimBuilder::GetNumKeys()
{
	int numKeys = 0;

	int groupIndex;
	int numGroups = this->GetNumGroups();
	for (groupIndex = 0; groupIndex < numGroups; groupIndex++)
	{
		const Group& group = this->GetGroupAt(groupIndex);
		int curveIndex;
		int numCurves = group.GetNumCurves();
		for (curveIndex = 0; curveIndex < numCurves; curveIndex++)
		{
			const Curve& curve = group.GetCurveAt(curveIndex);
			if (!curve.IsCollapsed())
			{
				numKeys += group.GetNumKeys();
			}
		}
	}
	return numKeys;
}

//-------------------------------------------------------------------------------------------------
bool U2AnimBuilder::Save(const TCHAR* filename)
{
	U2Filename filePath(filename);

	U2StackString ext;
	ext = filePath.GetExt();

	if(ext == (U2StackString)_T(".xanim"))
	{
		return SaveXAnim(filename);
	}
	else 
	{
		U2ASSERT(false);
		 FILE_LOG(logDEBUG) << _T("U2AnimBuilder::Save(): unsupported file extension in ")
			 << filename;

		return false;
	}	

}

//-------------------------------------------------------------------------------------------------
bool U2AnimBuilder::SaveXAnim(const TCHAR* szAnimFilename)
{
	U2ASSERT(szAnimFilename);

	U2WinFile *file = U2_NEW U2WinFile();
	U2ASSERT(file);

#define LINE_SIZE 1024

	if(file->Open(szAnimFilename, _T("w")))
	{
		TCHAR line[LINE_SIZE];
		const int numGroups = GetNumGroups();

		file->PutS(_T("type xanim\n"));
		_stprintf_s(line, LINE_SIZE, _T("numgroups %d\n"), numGroups);
		file->PutS(line);
		_stprintf_s(line, LINE_SIZE, _T("numkeys %d\n"), GetNumKeys());
		file->PutS(line);

		int groupIdx;
		for(groupIdx = 0; groupIdx < numGroups; ++groupIdx)
		{
			const Group& group			= GetGroupAt(groupIdx);
			const int numCurves			= group.GetNumCurves();
			const int statKey			= group.GetStartKey();
			const int numKeys			= group.GetNumKeys();
			const int keyStride			= group.GetKeyStride();
			const float keyTime			= group.GetKeyTime();
			const float fadeInTime		= group.GetFadeInFrames();
			const TCHAR* szLoopType		= Group::LoopType2String(group.GetLoopType());
			_stprintf_s(line,LINE_SIZE, _T("group %d %d %d %d %f %f %s\n"), 
				numCurves, statKey, numKeys, keyStride, keyTime, fadeInTime, 
				szLoopType);
			file->PutS(line);
			
			int curveIdx;
			for(curveIdx = 0; curveIdx < numCurves; ++curveIdx)
			{
				const Curve& curve = group.GetCurveAt(curveIdx);
				const TCHAR* szInterpType = Curve::IpolType2String(curve.GetIpolType());
				const int isAnimated = curve.IsAnimated();
				const int firstKeyIdx = curve.GetFirstKeyIndex();
				const D3DXVECTOR4& collapsedKey = curve.GetCollapsedKey();
				_stprintf_s(line, LINE_SIZE, _T("curve %s %d %d %f %f %f %f\n"), 
					szInterpType, firstKeyIdx, isAnimated, 
					collapsedKey.x, collapsedKey.y, collapsedKey.z, collapsedKey.w);
				file->PutS(line);

			}

			// write interleaved keys of the non-collapsed keys...
			int keyIdx;
			for(keyIdx = 0; keyIdx < numKeys; ++keyIdx)
			{
				for(curveIdx = 0; curveIdx < numCurves; ++curveIdx)
				{
					const Curve& Curve = group.GetCurveAt(curveIdx);
					if(!Curve.IsCollapsed())
					{
						const D3DXVECTOR4& val = 
							group.GetCurveAt(curveIdx).GetKeyAt(keyIdx).Get();
						_stprintf_s(line, LINE_SIZE, _T("key %f %f %f %f\n"), 
							val.x, val.y, val.z, val.w);
						file->PutS(line);
					}
				}
			}		
		}

		file->Close();		
	}

	U2_DELETE file;	
	
	return true;
}

//------------------------------------------------------------------------------
/**
Optimize curves. At the moment this will just collapse curves where
all keys are identical. Returns number of collapsed curves.
*/
int U2AnimBuilder::Optimize()
{
	int numOptimizedCurves = 0;
	int numGroups = GetNumGroups();
	int groupIdx;
	for(groupIdx=0; groupIdx < numGroups; ++groupIdx)
	{
		Group& group = GetGroupAt(groupIdx);
		int numCurves = group.GetNumCurves();
		int curveIdx;
		for(curveIdx=0; curveIdx < numCurves; ++curveIdx)
		{
			Curve& curve = group.GetCurveAt(curveIdx);
			if(curve.Optimize())
			{
				numOptimizedCurves++;
			}
		}
	}
	return numOptimizedCurves;
}


//------------------------------------------------------------------------------
/**
Fixes the first key index and key stride members in the
contained groups and curves.
*/
void U2AnimBuilder::FixKeyOffsets()
{
	int numGroups = GetNumGroups();
	int groupIdx;
	int groupFirstKeyIdx = 0;
	for(groupIdx = 0; groupIdx < numGroups; ++groupIdx)
	{
		Group& group = GetGroupAt(groupIdx);

		// count non-collapsed curves
		int numCurves = group.GetNumCurves();
		int numNoneCollaspedCurves = 0;
		int curveIdx;
		for(curveIdx = 0; curveIdx < numCurves; ++curveIdx)
		{
			Curve& curve = group.GetCurveAt(curveIdx);
			if(!curve.IsCollapsed())
			{
				curve.SetFirstKeyIndex(groupFirstKeyIdx + numNoneCollaspedCurves);
				numNoneCollaspedCurves++;
			}

			U2DynString str;
			str.Format(_T("Collapesed Key : %f, %f, %f, %f\n"), curve.collapsedKey.x, 
				curve.collapsedKey.y, curve.collapsedKey.z, curve.collapsedKey.w);
			FILE_LOG(logDEBUG) << str.Str();
		}

		// the key stride in each group is identical to the number of non-collapsed curves
		group.SetKeyStride(numNoneCollaspedCurves);

		// update the groupFirstKeyIndex for the next group
		groupFirstKeyIdx += (numNoneCollaspedCurves * group.GetNumKeys());
	}

}

//-------------------------------------------------------------------------------------------------
bool U2AnimBuilder::Load(const TCHAR* filename)
{
	U2Filename filePath(filename);

	U2StackString ext;
	ext = filePath.GetExt();

	if(ext == (U2StackString)_T("XAnim"))
	{
		return LoadXAnim(filename);
	}
	else 
	{
		U2ASSERT(false);
		FILE_LOG(logDEBUG) << _T("U2AnimBuilder::Save(): unsupported file extension in ")
			<< filename;

		return false;
	}	

}

//-------------------------------------------------------------------------------------------------
bool U2AnimBuilder::LoadXAnim(const TCHAR* szAnimFilename)
{
	U2ASSERT(szAnimFilename);

	bool retval = false;
	U2WinFile* file = U2_NEW U2WinFile;
	U2ASSERT(file);

	U2PrimitiveVec<Key*> keyArray(4096, 4096);

	int numGroups = 0;
	int numKeys = 0;
	TCHAR* szNextToken;
	if(file->Open(szAnimFilename, _T("r")))
	{
		TCHAR line[1024];
		while(file->GetS(line, sizeof(line)))
		{
			TCHAR* szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);
			if(0 == szKeyword)
			{
				continue;
			}
			else if(0 == _tcscmp(szKeyword, _T("type")))
			{
				const TCHAR* szFileType = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				U2ASSERT(szFileType);
				if(0 == _tcscmp(szFileType, _T("xanim")))
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
				numGroups = atoi(szNumGroups);
			}
			else if(0 == _tcscmp(szKeyword, _T("numkeys")))
			{
				const TCHAR* szNumKeys = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				U2ASSERT(szNumKeys);
				numKeys = atoi(szNumKeys);
			}
			else if(0 == _tcscmp(szKeyword, _T("group")))
			{
				const TCHAR* szNumCurves	= _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szStartKey		= _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szNumKeys		= _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szKeyStride	= _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szKeyTime		= _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szLoopType		= _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				U2ASSERT(szNumCurves);
				U2ASSERT(szStartKey);
				U2ASSERT(szNumKeys);
				U2ASSERT(szKeyStride);
				U2ASSERT(szKeyTime);
				U2ASSERT(szLoopType);

				Group* pGroup = U2_NEW Group;
				U2ASSERT(pGroup);
				pGroup->SetLoopType(Group::String2LoopType(szLoopType));
				pGroup->SetStartKey(atoi(szStartKey));
				pGroup->SetNumKeys(atoi(szNumKeys));
				pGroup->SetKeyStride(atoi(szKeyStride));
				pGroup->SetKeyTime(atof(szKeyTime));
				AddGroup(*pGroup);
			}
			else if(0 == _tcscmp(szKeyword, _T("curve")))
			{
				
				const TCHAR* szInterpType		= _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szFirstKeyIdx		= _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szCollapsedKeyX	= _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szCollapsedKeyY	= _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szCollapsedKeyZ	= _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szCollapsedKeyW	= _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				U2ASSERT(szInterpType);
				U2ASSERT(szFirstKeyIdx);
				U2ASSERT(szCollapsedKeyX);
				U2ASSERT(szCollapsedKeyY);
				U2ASSERT(szCollapsedKeyZ);
				U2ASSERT(szCollapsedKeyW);

				static D3DXVECTOR4 collapsedKey;
				collapsedKey.x = atof(szCollapsedKeyX);
				collapsedKey.y = atof(szCollapsedKeyY);
				collapsedKey.z = atof(szCollapsedKeyZ);
				collapsedKey.w = atof(szCollapsedKeyW);

				Curve* pCurve = U2_NEW Curve;
				U2ASSERT(pCurve);
				pCurve->SetIpolType(Curve::String2IpolType(szInterpType));
				pCurve->SetFirstKeyIndex(atoi(szFirstKeyIdx));
				if(pCurve->GetFirstKeyIndex() == -1)
				{
					pCurve->SetCollapsed(true);
				}
				pCurve->SetCollapsedKey(collapsedKey);
				GetGroupAt(GetNumGroups() - 1).AddCurve(*pCurve);
			}
			else if(0 == _tcscmp(szKeyword, _T("key")))
			{
				const TCHAR* szKeyX = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szKeyY = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szKeyZ = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				const TCHAR* szKeyW = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				U2ASSERT(szKeyX && szKeyY && szKeyZ && szKeyW);

				Key* pKey = U2_NEW Key;
				static uint32 keyIdx = 0;
				pKey->value.x = atof(szKeyX); pKey->value.y = atof(szKeyY);
				pKey->value.z = atof(szKeyZ); pKey->value.w = atof(szKeyW);				
			}
		}
		file->Close();
		retval = true;
	}
	U2_DELETE file;

	if(retval)
	{
		// transfer keys into curves
		int groupIdx;
		int numGroups = GetNumGroups();
		for(groupIdx = 0; groupIdx < numGroups; ++groupIdx)
		{
			Group& group = GetGroupAt(groupIdx);
			int curveIdx;
			const int numCurves = group.GetNumCurves();
			const int numKeysPerGroup = group.GetNumKeys();
			const int keyStride = group.GetKeyStride();

			for(curveIdx = 0; curveIdx < numCurves; ++curveIdx)
			{
				Curve& curve = group.GetCurveAt(curveIdx);
				if(!curve.IsCollapsed())
				{
					int firstKey = curve.GetFirstKeyIndex();
					int keyIdx;
					for(keyIdx = 0; keyIdx < numKeysPerGroup; ++keyIdx)
					{						
						curve.SetKey(keyIdx ,
							*keyArray[firstKey + (keyIdx * keyStride)]);
					}
				}
			}
		}

		keyArray.RemoveAll();
	}

	return retval;
}