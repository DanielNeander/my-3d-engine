/************************************************************************
module	:	U2Object
Author	:	Yun sangyong
Desc	:	Support RTTI, ID, Name, Controller 
************************************************************************/
#pragma once
#ifndef U2_OBJECT_H
#define U2_OBJECT_H

#include <U2Lib/Src/U2RefObject.h>
#include <U2Lib/Src/U2SmartPtr.h>
#include <U2Lib/Src/U2Util.h>

#include <U2_3D/src/U2_3DLibType.h>
#include <U2_3D/src/U2MainMacro.h>
//#include <U2_3D/Src/animation/U2Controller.h>
#include "U2Rtti.h"
#include "U2RttiMacro.h"


U2SmartPointer(U2Controller);
class U2RenderContext;

class U2_3D	U2Object: public U2RefObject 
{
public:
	static const U2Rtti ms_rtti;
	virtual const U2Rtti& GetType() const;
	bool IsExactly(const U2Rtti& type) const;
	bool IsDerived(const U2Rtti& type) const;
	bool IsExactlyTypeOf(const U2Object* pObj) const;
	bool IsDerivedTypeOf(const U2Object* pObj) const;	

	virtual ~U2Object();

	void SetName(const U2DynString& szName);
	const U2DynString& GetName() const;
	unsigned int GetUniqueID () const;
	static unsigned int GetNextID ();

	// Controller System
	// static lib ºôµå½Ã operator = ¿¡·¯.
	uint32 GetNumControllers() const;
	U2Controller* GetController(uint32 idx) const;
	void AttachController(U2Controller* pCtrl);
	void DetachController(U2Controller* pCtrl);
	void DetachAllCotroller();
	bool UpdateControllers(float fAppTime, U2RenderContext* pCxt);

protected:
	U2Object();
	
	unsigned int m_uiId;
	static unsigned int ms_uiNextId;	
	U2DynString m_szName;

private:
	U2PrimitiveVec<U2Controller*> m_controllers;
};

typedef U2SmartPtr<U2Object> U2ObjectPtr;

// static and dynamic type casting
template <class T> T* StaticCast (U2Object* pkObj);
template <class T> const T* StaticCast (const U2Object* pkObj);
template <class T> T* DynamicCast (U2Object* pkObj);
template <class T> const T* DynamicCast (const U2Object* pkObj);

#include "U2Object.inl"

#endif