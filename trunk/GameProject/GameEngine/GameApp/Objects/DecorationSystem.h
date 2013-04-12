#pragma once 

#include "BBGrassPatch.h"

class DecorationSystem {
	
public:
	virtual ~DecorationSystem();

	BBGrassPatch							bbGrassPatch;

	bool Initialize();
	void GenerateGrass(BBGrassPatch* pGrass, const noVec3 terrainCenter, float terrainRadius, uint32 grassBladesPerPatch, float patchRadius);

	BBGrassPatch* CreateNewGrass(idStr texName);

	void Update(float fDeltaTime);
	void Render(float fDeltaTime);

	void DelGrass(const std::string keyname);
protected:
	virtual HRESULT LoadShaders();

	ID3DX11Effect* m_pGrassEffect;

	ID3DX11EffectMatrixVariable*		m_pfxWorld;
	ID3DX11EffectMatrixVariable*		m_pfxView;
	ID3DX11EffectMatrixVariable*		m_pfxProj;
	ID3DX11EffectScalarVariable*		m_fTimeVar;
	ID3DX11EffectShaderResourceVariable* m_pGrassSRV;

	std::map<std::string, BBGrassPatch*>	m_GrassMap;
};

extern DecorationSystem gDecoSys;