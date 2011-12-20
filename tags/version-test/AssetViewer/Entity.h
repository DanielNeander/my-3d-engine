#ifndef ENTITY_H
#define ENTITY_H

#include <U2_3D/Src/dx9/U2N2Mesh.h>

U2SmartPointer(U2Entity);

class U2Entity : public U2RefObject
{
public:
	U2N2MeshPtr m_spMesh;


	virtual void Reset();

	virtual bool Initialize() { return true; }

	virtual void Release();

	virtual void PrepareRender() {}

	virtual void Update() {}

	DEFINE_GETSET_ACCESSOR( U2N2Mesh*, m_spMesh, Mesh )
};

U2SmartPointer(CSkyEntity);

class CSkyEntity : public U2Entity
{
private:
	D3DXVECTOR3 m_vSunDir;
	D3DXCOLOR	m_vSunColor;

	D3DXVECTOR3 m_vMoonDir;
	D3DXCOLOR   m_vMoonColor;

	// Calculate the sun attributes using Preetham's method.
	void CalculateSunAtterPreetham();

public:
	bool		m_bUseVertexSky;

	CSkyEntity();

	~CSkyEntity();

	virtual void Update();

	virtual void PrepareRender();

	// Calculate the Sky Attributes.
	void CalculateSkyAttributes();

	DEFINE_GETSET_ACCESSOR_REFERENCE( D3DXVECTOR3, m_vSunDir, SunDirection )
	DEFINE_GETSET_ACCESSOR_REFERENCE( D3DXCOLOR, m_vSunColor, SunColor )

	DEFINE_GETSET_ACCESSOR_REFERENCE( D3DXVECTOR3, m_vMoonDir, MoonDirection )
	DEFINE_GETSET_ACCESSOR_REFERENCE( D3DXCOLOR, m_vMoonColor, MoonColor )
};

U2SmartPointer(CBoxEntity);
class CBoxEntity : public U2Entity
{
public:
	// Constructor.
	CBoxEntity();

	// Destructor.
	~CBoxEntity();

	// Prepare any special values for rendering this entity.
	virtual void PrepareRender();
};

U2SmartPointer(CStaticEntity);
class CStaticEntity : public U2Entity
{
public:
	// Constructor.
	CStaticEntity();

	// Destructor.
	~CStaticEntity();

	// Prepare any special values for rendering this entity.
	virtual void PrepareRender();
};

extern CSkyEntity* g_pSky;

#endif