/************************************************************************
module	:	U2Dx9MaterialMgr
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_DX9_MATERIALMGR_H
#define U2_DX9_MATERIALMGR_H

class U2_3D U2Dx9MaterialMgr : public U2MemObj
{

public:
	U2Dx9MaterialMgr();
	~U2Dx9MaterialMgr();

	D3DMATERIAL9* GetCurrMaterial();
	void SetCurrMaterial(U2MaterialState& pMat);

	void UpdateToDevice(LPDIRECT3DDEVICE9 pD3DDev);
	void DestoryMaterialData(U2MaterialState* pMat);

	void Init();

protected:
	class MaterialEntry : public U2MemObj
	{
	public:
		MaterialEntry(const U2MaterialState& mat);

		bool Update(const U2MaterialState& mat);

		D3DMATERIAL9* GetD3DMat() { return &m_d3dMat; }

	private:
		D3DMATERIAL9 m_d3dMat;
		uint32 m_uRevID;
	};

	MaterialEntry* m_pCurrMat;
	bool m_bChanged;
};


#endif