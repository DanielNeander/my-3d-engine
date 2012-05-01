/************************************************************************
module	:	U2RenderContext
Author	:	Yun sangyong
Desc	:	Object 마다 사용자 정의 변수를 정의할 수 있는 기능 제공.
************************************************************************/
#pragma once
#ifndef U2_RENDERCONTEXT_H
#define U2_RENDERCONTEXT_H

#include <U2Lib/Src/U2TVec.h>

#include "U2VariableContext.h"
#include <U2_3D/Src/shader/U2ShaderAttribute.h>


U2SmartPointer(U2Node);

class U2_3D U2RenderContext : public U2VariableContext
{
public:
	enum Flag 
	{
		SHADOW_VISIBLE		= ( 1 << 0),
		OCCLUDED			= ( 1 << 1),
		CAST_SHADOWS		= (	1 << 2),
		DO_OCCULSION_QUERY	= ( 1 << 3),
		DO_ALL				= SHADOW_VISIBLE | OCCLUDED | CAST_SHADOWS | DO_OCCULSION_QUERY 
	};

	U2RenderContext();
	~U2RenderContext();

	void SetFrameID(uint32 id);
	uint32 GetFrameID() const;

	void SetRootNode(U2Node* pNode);
	U2NodePtr GetRootNode() const;

	bool IsValid() const;

	/// access to shader parameter overrides
	U2ShaderAttribute& GetShaderOverides();

	uint32 AddLocalVar(U2Variable* val);
	
	U2Variable& GetLocalVar(uint32 idx);

	void ClearLocalVars();

	U2Variable* FindLocalVar(U2Variable::Handle handle);

	void SetShadowIntensity(float i);
	float GetShadowIntensity() const;

	/// set flags
	void SetFlag(Flag f, bool b);
	/// get visibility hint
	bool GetFlag(Flag f) const;

	void* pUserData;

private:
	friend class U2SceneMgr;

	
	uint32 m_uiFrameId;
	uint32 m_uiFlags;
	U2NodePtr m_spRootNode;
	float m_fPriority;
	float m_fShadowIntensity;
	U2ObjVec<U2VariablePtr> m_aspLocalVars;
	U2ShaderAttribute m_shderOverrides;

};

typedef U2SmartPtr<U2RenderContext> U2RenderContextPtr;

#include "U2RenderContext.inl"

#endif 