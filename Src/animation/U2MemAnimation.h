/**************************************************************************************************
module	:	U2MemAnimation
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_MEMANIMATION_H
#define U2_MEMANIMATION_H

#include "U2Animation.h"


class U2AnimKey : public U2MemObj
{
public:
	U2AnimKey(const D3DXVECTOR4& key) 
		:m_key(key)
	{

	}
	

	D3DXVECTOR4 m_key;
};


class U2MemAnimation : public U2Animation
{
public:
	U2MemAnimation();
	virtual ~U2MemAnimation();

	virtual void SampeInterpKeys(float fTime, int iInterpGroupIdx, 		
		int firstInterpKeyIdx, int numInterpKeys, D3DXVECTOR4* pKeyArray);

	virtual uint32 GetByteInSize();

	U2PrimitiveVec<U2AnimKey*>& GetKeyArray() { return m_keyArray; }	

	virtual bool LoadResource();
	virtual void UnloadResource();

protected:
	

	bool LoadU2TextAnim(const U2DynString& filename);

	bool LoadU2BinaryAnim(const U2DynString& filename);
		
	U2PrimitiveVec<U2AnimKey*> m_keyArray;
};


#endif