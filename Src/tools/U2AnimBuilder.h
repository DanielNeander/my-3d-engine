/**************************************************************************************************
module	:	U2AnimBuilder
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_ANIMBUILDER_H
#define U2_ANIMBUILDER_H

#include <U2Lib/Src/U2RefObject.h>
#include <U2Lib/Src/U2SmartPtr.h>

//------------------------------------------------------------------------------
class U2_3D U2AnimBuilder : public U2RefObject
{
public:
	/// an animation key
	class Key : public U2MemObj
	{
	public:
		/// constructor
		Key();
		/// constructor with value
		Key(const D3DXVECTOR4& val);
		/// set value
		void Set(const D3DXVECTOR4& val);
		/// get value
		const D3DXVECTOR4& Get() const;
		/// set x component
		void SetX(float x);
		/// set y component
		void SetY(float y);
		/// set z component
		void SetZ(float z);
		/// set w component
		void SetW(float w);

		D3DXVECTOR4 value;
	};

	/// an animation curve
	class Curve : public U2MemObj
	{
	public:
		/// interpolation types
		enum IpolType
		{
			NONE,           // not interpolated (invalid)
			STEP,           // step through keys
			LINEAR,         // linear interpolation
			QUAT,           // quaternion interpolation
		};

		/// default constructor
		Curve();
		/// fixed size constructor
		Curve(int numKeys, const Key& fillKey);

		~Curve();

		/// add a key to the end of the array
		void SetKey(int index, const Key& key);
		/// get number of keys
		int GetNumKeys() const;
		/// get key at index
		Key& GetKeyAt(int index);
		/// set the interpolation type of the curve
		void SetIpolType(IpolType t);
		/// get the interpolation type of the curve
		IpolType GetIpolType() const;
		/// set the first key index
		void SetFirstKeyIndex(int i);
		/// get the first key index
		int GetFirstKeyIndex() const;
		/// optimize the curve
		bool Optimize();
		/// set collapsed flag
		void SetCollapsed(bool b);
		/// check whether the curve has been collapsed by Optimize()
		bool IsCollapsed() const;
		/// set the collapsed key
		void SetCollapsedKey(const D3DXVECTOR4& key);
		/// get the collapsed key
		const D3DXVECTOR4& GetCollapsedKey() const;
		/// convert ipol type to string
		static const TCHAR* IpolType2String(IpolType t);
		/// convert string to ipol type
		static IpolType String2IpolType(const TCHAR* str);
		///flag  = true, if curve belongs to an animated joint
		void SetIsAnimated(int isAnim);
		///returns isAnimated flag
		int IsAnimated() const;

		U2PrimitiveVec<Key*> keyArray;
		IpolType ipolType;
		bool isCollapsed;
		int firstKeyIndex;
		D3DXVECTOR4 collapsedKey;
		int isAnimated;
	};

	/// an animation curve group
	class Group : public U2MemObj
	{
	public:
		/// loop types
		enum LoopType
		{
			CLAMP,
			REPEAT,
		};

		/// constructor
		Group();

		~Group();

		/// add an animation curve object
		void AddCurve(const Curve& curve);
		/// get number of animation curves
		int GetNumCurves() const;
		/// get animation curve at index
		Curve& GetCurveAt(int index) const;
		/// set loop type
		void SetLoopType(LoopType t);
		/// get loop type
		LoopType GetLoopType() const;
		/// set fade in frames
		void SetFadeInFrames(float frames);
		/// get fade in frames
		float GetFadeInFrames() const;
		/// set the start key
		void SetStartKey(int start);
		/// get the start key
		int GetStartKey() const;
		/// set number of keys
		void SetNumKeys(int num);
		/// get number of keys
		int GetNumKeys() const;
		/// set the key duration
		void SetKeyTime(float t);
		/// get the key duration
		float GetKeyTime() const;
		/// set the key stride
		void SetKeyStride(int stride);
		/// get the key stride
		int GetKeyStride() const;
		/// check if the curves are valid
		bool Validate() const;
		/// convert loop type to string
		static const TCHAR* LoopType2String(LoopType t);
		/// convert string to loop type
		static LoopType String2LoopType(const TCHAR* str);
		/// set array of animated joints
		void SetAnimJoints(U2PrimitiveVec<U2DynString>* joints);
		/// get array og animated joints
		const U2PrimitiveVec<U2DynString>& GetAnimJoints() const;

		U2PrimitiveVec<Curve*> curveArray;
		LoopType loopType;
		U2DynString groupName;
		int startKey;
		int numKeys;
		int keyStride;
		float keyTime;
		U2PrimitiveVec<U2DynString>* animJoints;
		float fadeInFrames;
	};

	/// constructor
	U2AnimBuilder();
	/// destructor
	~U2AnimBuilder();

	/// updates the first key index and key stride members (call before saving the file)
	void FixKeyOffsets();
	/// save animation file (filename extension decides format)
	bool Save(const TCHAR* filename);
	///// save nanim2 file
	bool SaveXAnim(const TCHAR* filename);
	///// save binary data to existing file
	//bool SaveNax2(nFile* file);
	///// save nax2 file
	//bool SaveNax2(nFileServer2* fileServer, const char* filename);
	///// load animation file (filename extension decides format)
	bool Load(const TCHAR* filename);
	///// load nanim2 file
	bool LoadXAnim(const TCHAR* filename);
	///// load nax2 file
	//bool LoadNax2(nFileServer2* fileServer, const char* filename);

	/// clear all
	void Clear();
	/// add an animation group
	void AddGroup(Group& group);
	/// get number of groups
	int GetNumGroups() const;
	/// get group at index
	Group& GetGroupAt(int i);

	/// optimize the animation data
	int Optimize();

private:
	/// return total number of keys
	int GetNumKeys();

	U2PrimitiveVec<Group*> groupArray;
};

typedef U2SmartPtr<U2AnimBuilder> U2AnimBuilderPtr;

#include "U2AnimBuilder.inl"

#endif
