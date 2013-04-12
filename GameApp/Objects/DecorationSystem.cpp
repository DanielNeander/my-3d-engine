#include "stdafx.h"
#include "GameApp/Utility.h"
#include "DecorationSystem.h"
#include "GameApp/GameApp.h"
#include "GameApp/Util/EffectUtil.h"

DecorationSystem gDecoSys;

HRESULT DecorationSystem::LoadShaders()
{

	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;

#if defined( DEBUG ) || defined( _DEBUG )
	dwShaderFlags |= D3D10_SHADER_DEBUG;
	//	dwShaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	CComPtr<ID3DBlob> pShader;
	CompileShaderFromFile("Data/Shaders/Grass.fx", NULL, NULL,"fx_5_0", &pShader);
	D3DX11CreateEffectFromMemory(pShader->GetBufferPointer(), pShader->GetBufferSize(), dwShaderFlags, D3D11Dev(), &m_pGrassEffect);
	pShader.Detach();

	GET_EFFECT_VAR( m_pGrassEffect, "World",			MATRIX,			m_pfxWorld);
	GET_EFFECT_VAR( m_pGrassEffect, "View",			MATRIX,			m_pfxView);
	GET_EFFECT_VAR( m_pGrassEffect, "Proj",			MATRIX,			m_pfxProj);
	GET_EFFECT_VAR( m_pGrassEffect,	"g_fTime",		SCALAR,			m_fTimeVar);
	GET_EFFECT_VAR( m_pGrassEffect, "txDiffuse",	   SHADER_RESOURCE, m_pGrassSRV);

	return S_OK;
}

bool DecorationSystem::Initialize()
{
	LoadShaders();	
	//bbGrassPatch.SetDiffuseTextureName("texture//grassY.dds");
	//bbGrassPatch.CreateDevice(D3D11Dev(), D3D11Context(), m_pGrassEffect);
	return true;
}

void DecorationSystem::DelGrass(const std::string keyname) {

	std::map<std::string, BBGrassPatch*>::iterator it = m_GrassMap.find(keyname);	
	if (it != m_GrassMap.end()) {
		m_GrassMap.erase(it);
	}
}

BBGrassPatch* DecorationSystem::CreateNewGrass(idStr texName) {

	DelGrass(texName.c_str());
	BBGrassPatch* pNewGrass = new BBGrassPatch;
	pNewGrass->SetDiffuseTextureName("textures//grassY.dds");
	
	m_GrassMap.insert(std::make_pair(std::string(texName.c_str()), pNewGrass ));
	return pNewGrass;
}


void DecorationSystem::Update(float fDeltaTime) { 
	
	noVec3 eyePt = GetCamera()->GetFrom();
	noVec3 atPt	= GetCamera()->GetTo();
	//bbGrassPatch.Cull(D3D11Context(),noMath::PI/8 /* frustum cone angle = camera FOV/2*/ , eyePt.ToFloatPtr(), atPt.ToFloatPtr() );

	std::map<std::string, BBGrassPatch*>::iterator it = m_GrassMap.begin();
	for ( ; it != m_GrassMap.end(); ++it) {
		it->second->Cull(D3D11Context(),noMath::PI/8 /* frustum cone angle = camera FOV/2*/ , eyePt.ToFloatPtr(), atPt.ToFloatPtr() );
	}	

}

void DecorationSystem::Render(float fDeltaTime) {

	//bbGrassPatch.Render(D3D11Context(), m_pGrassSRV);
	noMat4 Identity(mat4_identity);	
	m_pGrassEffect->GetVariableByName("World")->AsMatrix()->SetMatrix(Identity.ToFloatPtr());
	m_pGrassEffect->GetVariableByName("View")->AsMatrix()->SetMatrix((float*)GetCamera()->getViewMatrix());
	m_pGrassEffect->GetVariableByName("Projection")->AsMatrix()->SetMatrix((float*)GetCamera()->getProjectionMatrix());
	m_pGrassEffect->GetVariableByName( "g_fTime" )->AsScalar()->SetFloat(GetApp()->GetSecTime());

	std::map<std::string, BBGrassPatch*>::iterator it = m_GrassMap.begin();
	for ( ; it != m_GrassMap.end(); ++it) {
		it->second->Render(D3D11Context(), m_pGrassEffect->GetVariableByName( "txDiffuse" )->AsShaderResource());
	}
}

DecorationSystem::~DecorationSystem()
{
	SAFE_RELEASE(m_pGrassEffect);
	

	std::map<std::string, BBGrassPatch*>::iterator it = m_GrassMap.begin();
	for ( ; it != m_GrassMap.end(); ++it) {
		SAFE_DELETE(it->second);
	}
}

void DecorationSystem::GenerateGrass( BBGrassPatch* pGrass, const noVec3 terrainCenter, float terrainRadius, uint32 grassBladesPerPatch, float patchRadius )
{
	pGrass->Generate( terrainCenter.ToFloatPtr(), terrainRadius, grassBladesPerPatch, patchRadius);
	pGrass->CreateDevice(D3D11Dev(), D3D11Context(), m_pGrassEffect);
}
