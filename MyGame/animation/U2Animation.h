#ifndef	U2_ANIMATION_H
#define	U2_ANIMATION_H
#pragma once

#include <U2_3D/src/Main/U2Object.h>

class U2_3D U2Animation : public U2Object
{

public:
	class InterpKey : public U2MemObj
	{
	public:
		enum InterpType 
		{
			INTERP_NONE = 0,
			INTERP_STEP, 
			INTERP_QUAT,
			INTERP_LINEAR, 
		};

		InterpKey();
		~InterpKey();

		void SetInterpType(InterpType type);
		InterpType GetInterpType() const;

		void SetConstValue(const D3DXVECTOR4& val);
		
		const D3DXVECTOR4& GetConstValue() const;

		static InterpType StrToInterpType(const TCHAR* str);

		void SetFirstKeyIdx(int idx);

		int GetFirstKeyIdx() const;

		void SetAnimated(int isAnim);
		
		int IsAnimated() const;

		void SetStartValue(const D3DXVECTOR4& val);
		
		D3DXVECTOR4 GetStartValue() const;
		
		void SetCurAnimClipValue(const D3DXVECTOR4& val);

		D3DXVECTOR4 GetCurAnimClipValue() const;

	private:
		InterpType m_eInterpType;
		D3DXVECTOR4 m_constVal;
		int m_iFirstKeyIdx;
		int m_iIsAnimated;
		D3DXVECTOR4 m_startVal;
		D3DXVECTOR4 m_firstClipVal;

	};

	class InterpKeyGroup : public U2MemObj
	{
	public:
		enum LoopTytpe 
		{
			LOOP_CLAMP = 0,
			LOOP_REPEAT, 
			INVALID_LOOP_TYPE,
		};

		InterpKeyGroup();
		~InterpKeyGroup();

		void SetNumInterpKeys(int c);
		uint32 GetNumInterpKeys() const;

		InterpKey& GetInterpKey(int i) const;

		void SetStartKey(int idx);
		int GetStartKey() const;

		void SetNumKeys(int i);
		int GetNumKeys() const;

		void SetKeyStride(int stride);
		int GetKeyStride() const;

		void SetKeyTime(float fTime);
		float GetKeyTime() const;

		void SetFadeInFrames(float fFrames);

		float GetFadeInFrames() const;

		void SetLoopType(LoopTytpe eLoop);

		LoopTytpe GetLoopType() const;

		static LoopTytpe StrToLoopType(const TCHAR* str);
		/// convert a time stamp into 2 global key indexes and an inbetween value
		void TimeToIdx(float fTime, int& iKeyIdx0, int& iKeyIdx1, float& fInBetween) const;
		/// return true if time is between startTime and stopTime (handles looped and clamped case correctly)
		bool IsBetweenTimes(float fTime, float fStartTime, float fStopTime) const;

		double GetDuration() const;

	private:
		int m_iStartKey;
		int m_iNumKeys;
		int m_iKeyStride;
		float m_fKeyTime;
		LoopTytpe m_eLoopType;
		float m_fFadeInFrames;
		U2PrimitiveVec<InterpKey*> m_interpKeyArray;
	};

	U2Animation();

	virtual ~U2Animation();

	virtual void SampeInterpKeys(float fTime, int iInterpGroupIdx, 		
		int firstInterpKeyIdx, int numInterpKeys, D3DXVECTOR4* pKeyArray);

	double GetDuration(int iInterpGroupIdx) const;

	void SetNumInterpKeyGroups(int group);
	int GetNumInterpKeyGroups() const;

	InterpKeyGroup& GetInterpKeyGroup(int idx) const;

	virtual bool LoadResource();
	virtual void UnloadResource();

	protected:
		

		/// fix the firstKeyIndex and keyStride members in the contained animation curve objects
		void FixKeyOffets();

		U2PrimitiveVec<InterpKeyGroup*> m_interpKeyGroupArray;
};

typedef U2SmartPtr<U2Animation> U2AnimationPtr;

#include "U2Animation.inl"

#endif