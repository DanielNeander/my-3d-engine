#include <U2_3D/Src/U23DLibPch.h>
#include "U2TransformInterpController.h"
#include "U2AnimFactory.h"

IMPLEMENT_RTTI(U2TransformInterpController, U2Controller);
//-------------------------------------------------------------------------------------------------
U2TransformInterpController::U2TransformInterpController()
:m_iAnimGroup(0),
m_spAnim(0)
{

}

U2TransformInterpController::~U2TransformInterpController()
{
	UnloadAnim();
}

void U2TransformInterpController::Initialize(U2RenderContext* pCxt)
{
	if(LoadAnim())
	{
		U2Controller::Initialize(pCxt);
	}
}

void U2TransformInterpController::Terminate(U2RenderContext* pCxt)
{
	UnloadAnim();
}


bool U2TransformInterpController::LoadAnim()
{
	if(!m_spAnim && m_szAnimname.Str())
	{
		U2Animation* pAnim = U2AnimFactory::Instance()->CreateMemAnim(m_szAnimname);
		U2ASSERT(pAnim);

		U2FilePath fPath;		
		TCHAR fullPath[MAX_PATH];
		// StackString Memory Leak...
		U2DynString includePath(MODEL_PATH);	

		fPath.ConvertToAbs(fullPath, MAX_PATH, m_szAnimname, includePath);		

		pAnim->SetName(fullPath);

		if(pAnim)
		{
			if(!pAnim->LoadResource())
			{
				FDebug("U2SkinController: Error loading anim file '%s'\n", this->m_szAnimname.Str());
				U2_DELETE pAnim;
				return false;
			}
		}
		m_spAnim = pAnim;		

		return true;
	}
	return false;		
}

void U2TransformInterpController::UnloadAnim()
{
	m_spAnim = 0;
}



bool U2TransformInterpController::Update(float fTime, U2RenderContext* pCxt)
{
	U2ASSERT(m_pTarget);
	U2ASSERT(pCxt);
	U2ASSERT(U2Variable::InvalidHandle != m_channelVarHandle);

	U2Variable* pVar = pCxt->GetVariable(m_channelVarHandle);
	U2ASSERT(pVar);
	float fCurrTime = pVar->GetFloat();

	D3DXVECTOR4 keyArray[3];
	m_spAnim->SampeInterpKeys(fCurrTime, m_iAnimGroup, 0, 3, &keyArray[0]); 
	m_pTarget->SetLocalTrans(keyArray[0].x, keyArray[0].y, keyArray[0].z);
	m_pTarget->SetLocalRot(D3DXQUATERNION(keyArray[0].x, keyArray[0].y, keyArray[0].z, keyArray[0].w));
	m_pTarget->SetLocalScale(keyArray[2].x, keyArray[2].y, keyArray[2].z);

	return true;
}



