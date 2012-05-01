/**************************************************************************************************
module	:	U2Controller
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_CONTROLLER_H
#define U2_CONTROLLER_H

#include "U2AnimUtil.h"
#include <U2_3D/Src/main/U2Variable.h>
#include <U2_3D/Src/main/U2Object.h>

class U2RenderContext;

class U2_3D U2Controller : public U2Object
{
	DECLARE_RTTI;
public:
	enum Type
	{
		INVALID, 
		TRANSFORM, 
		SHADER,
		BLEND_SHAPE
	};

	U2Controller();
	virtual ~U2Controller();

	virtual void Initialize(U2RenderContext* pCxt) { m_bInit = true; }
	virtual void Terminate(U2RenderContext* pCxt) = 0;

	bool IsInitialize() const { return m_bInit; }

	virtual Type GetControllerType() const;

	virtual bool Update(float fTime, U2RenderContext* pCxt);

	void SetChannel(const TCHAR* name);

	const TCHAR* GetChannel();

	void SetLoopType(U2AnimLoopType::Type eType) { m_eLoopType = eType; }
	U2AnimLoopType::Type GetLoopType() const { return m_eLoopType; }

	void SetTarget(U2Spatial* pTarget);
	U2Spatial* GetTarget() const;

protected:
	U2AnimLoopType::Type m_eLoopType;
	U2Variable::Handle m_channelVarHandle;
	U2Variable::Handle m_channelOffsetVarHandle;
	bool m_bInit;	// Initialize를 할 곳이 확실하지 않아 추가한 변수.

	U2Spatial* m_pTarget;

};

U2SmartPointer(U2Controller);

#endif 
