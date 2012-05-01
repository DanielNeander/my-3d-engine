/**************************************************************************************************
module	:	U2TransformInterpController
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_TRANSFORMIMTERPCONTROLLER_H
#define U2_TRANSFORMINTERPCONTROLLER_H

#include "U2Controller.h"
#include "U2Animation.h"

class U2_3D U2TransformInterpController : public U2Controller 
{
	DECLARE_RTTI;
public:
	U2TransformInterpController();
	virtual ~U2TransformInterpController();

	virtual void Initialize(U2RenderContext* pCxt);
	virtual void Terminate(U2RenderContext* pCxt);

	void SetAnim(const U2DynString& szFilename);

	const U2DynString& GetAnim() const;

	void SetAnimGroup(int group);
	int GetAnimGroup();

	virtual Type GetControllerType() const;

	virtual bool Update(float fTime, U2RenderContext* pCxt);

private:

	bool LoadAnim();

	void UnloadAnim();

	int m_iAnimGroup;
	U2DynString m_szAnimname;
	
	U2AnimationPtr m_spAnim;	



};


inline U2Controller::Type U2TransformInterpController::GetControllerType() const
{
	return TRANSFORM;
}

//-------------------------------------------------------------------------------------------------
inline void U2TransformInterpController::SetAnim(const U2DynString& szFilename)
{
	UnloadAnim();
	m_szAnimname = szFilename;

}

//-------------------------------------------------------------------------------------------------
inline const U2DynString& U2TransformInterpController::GetAnim() const
{
	return m_szAnimname;
}

//-------------------------------------------------------------------------------------------------
inline void U2TransformInterpController::SetAnimGroup(int group)
{
	m_iAnimGroup = group;
}

//-------------------------------------------------------------------------------------------------
inline int U2TransformInterpController::GetAnimGroup()
{
	return m_iAnimGroup;
}

#endif