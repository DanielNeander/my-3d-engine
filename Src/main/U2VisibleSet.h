/**************************************************************************************************
module	:	U2VisibleSet
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_VISIBLESET_H
#define U2_VISIBLESET_H


#include <U2_3D/src/U2_3DLibType.h>
#include <u2lib/src/U2RefObject.h>
class U2Effect;
class U2Spatial;

class U2_3D U2VisibleObject : public U2MemObj
{
public:
	U2Spatial* m_pObj;
	U2Effect* m_pGlobalEffect; 		

	bool IsDrawable() const { return m_pObj && !m_pGlobalEffect; }
};



class U2_3D U2VisibleSet : public U2MemObj
{

public:
	U2VisibleSet();
	U2VisibleSet(unsigned int uMaxSize, unsigned int uGrowBy);
	~U2VisibleSet();

	void Insert(U2Spatial* mesh, U2Effect* globalEffect);

	void RemoveAll();

	void SetAt(uint32 uIdx, U2Mesh& mesh);

	U2VisibleObject* GetVisible();
	U2VisibleObject& GetVisible(uint32 uIdx);

	uint32 GetCount() const;
	void Resize(int maxSize, int growBy);
			

private:
	enum 
	{
		DEFAULT_MAX_COUNT = 512,
		DEFAULT_GROWBY = 512,
	};
	
	U2VisibleObject* m_pArray;
	uint32 m_uCurrSize;
	uint32 m_uAllocatedSize;
	
	uint32 m_uGrowBy;
};


#endif