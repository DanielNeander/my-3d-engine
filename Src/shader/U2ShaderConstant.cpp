#include <U2_3D/src/U23DLibPCH.h>
#include "U2ShaderConstant.h"


IMPLEMENT_RTTI(U2ShaderConstant, U2Object);
IMPLEMENT_INITIALIZE(U2ShaderConstant);
IMPLEMENT_TERMINATE(U2ShaderConstant);


 const TCHAR* U2ShaderConstant::ms_strSCMs[U2ShaderConstant::MAX_SCM_COUNT + 1] =
{
	_T("INVALID"),
	_T("WMatrix"),
	_T("VMatrix"),
	_T("PMatrix"),
	_T("WVMatrix"),
	_T("VPMatrix"),
	_T("WVPMatrix"),
	_T("WMatrixT"),
	_T("VMatrixT"),
	_T("PMatrixT"),
	_T("WVMatrixT"),
	_T("VPMatrixT"),
	_T("WVPMatrixT"),
	_T("WMatrixI"),
	_T("VMatrixI"),
	_T("PMatrixI"),
	_T("WVMatrixI"),
	_T("VPMatrixI"),
	_T("WVPMatrixI"),
	_T("WMatrixIT"),
	_T("VMatrixIT"),
	_T("PMatrixIT"),
	_T("WVMatrixIT"),
	_T("VPMatrixIT"),
	_T("WVPMatrixIT"),	//25

	_T("MaterialEmissive"),
	_T("MaterialAmbient"),
	_T("MaterialDiffuse"),
	_T("MaterialSpecular"),

	_T("CameraModelPosition"),
	_T("CameraModelDirection"),
	_T("CameraModelUp"),
	_T("CameraModelRight"),
	_T("CameraWorldPosition"),
	_T("CameraWorldDirection"),
	_T("CameraWorldUp"),
	_T("CameraWorldRight"),

	_T("ProjectorModelPosition"),
	_T("ProjectorModelDirection"),
	_T("ProjectorModelUp"),
	_T("ProjectorModelRight"),
	_T("ProjectorWorldPosition"),
	_T("ProjectorWorldDirection"),
	_T("ProjectorWorldUp"),
	_T("ProjectorWorldRight"),
	_T("ProjectorMatrix"),
								// 50
	_T("Light0ModelPosition"),
	_T("Light0ModelDirection"),
	_T("Light0WorldPosition"),
	_T("Light0WorldDirection"),
	_T("Light0Ambient"),
	_T("Light0Diffuse"),
	_T("Light0Specular"),
	_T("Light0SpotCutoff"),
	_T("Light0Attenuation"),
	
};




// Start array at zero so that it won't be initialized until after Main::Initialize().
U2TStringHashTable< U2ShaderConstant::ShaderConstantMapping>* 
U2ShaderConstant::ms_SCMTbl = NULL;


void U2ShaderConstant::Initialize()
{
	ms_SCMTbl = U2_NEW U2TStringHashTable< ShaderConstantMapping>(MAX_SCM_COUNT, 8);	
	for(uint32 i=0; i < MAX_SCM_COUNT; ++i)
	{
		if(ms_strSCMs[i])
			ms_SCMTbl->Insert(ms_strSCMs[i], (ShaderConstantMapping)i);			
	}

	
	
	
	
}

void U2ShaderConstant::Terminate()
{
	ms_SCMTbl->RemoveAll();
	U2_DELETE ms_SCMTbl;

}


U2ShaderConstant::U2ShaderConstant(U2ShaderConstant::ShaderConstantMapping eMapping, 
								   int iBaseRegister, uint32 uRegisterCnt)
{
	U2ASSERT(iBaseRegister >= 0);
	U2ASSERT(1 <= uRegisterCnt && uRegisterCnt <= 4);

	m_eSCM = eMapping;
	m_iBaseRegister = iBaseRegister;
	m_uRegisterCnt = uRegisterCnt;

	memset(m_afData, 0, 16 * sizeof(float));

}

U2ShaderConstant::~U2ShaderConstant()
{

}

const TCHAR* U2ShaderConstant::GetName(ShaderConstantMapping eMapping)
{
	return ms_strSCMs[eMapping];
}

U2ShaderConstant::ShaderConstantMapping U2ShaderConstant::GetSCM(const U2DynString& name)
{

	ShaderConstantMapping* pSCMIter = ms_SCMTbl->Find(name);
	if(pSCMIter)
		return *pSCMIter;
	
	return SCM_INVALID;
}