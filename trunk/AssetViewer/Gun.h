#ifndef GUN_H
#define GUN_H

class CGun
{
public:
	CGun();
	virtual ~CGun();

	virtual HRESULT RestoreDeviceObjects();
	virtual void InvalidateDeviceObjects();

	virtual void Render() = 0;
	virtual void Update(float fElapsedTime) = 0;

	virtual bool CanFire() = 0;
	virtual void Fire() = 0;

protected:
	
	ID3DXMesh* m_pMesh;
	D3DXVECTOR3 m_vPos;
};

#endif