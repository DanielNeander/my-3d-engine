#pragma once 


#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <windows.h>
#include <assert.h>
#include <tchar.h>
//#include <commctrl.h> // for InitCommonControls() 

#include <U2Application/U2WinApp.h>
#include <U2_3D/Src/main/U2Scene.h>

//#include <ThirdPersonCamera.h>

class ShadowMapApp : public WinApp
{

public:


	virtual bool		OnInitialize();
	virtual void		OnTerminate();
	virtual void		OnIdle();
	//virtual bool		OnKeyDown(unsigned char ucKey, int x, int y);
	/*virtual bool		OnMouseClick(int button, int state, int x, int y, 
		unsigned int uModifiers);*/

	static ShadowMapApp*		Create();

	static ShadowMapApp* ms_pSingleton;



private:
	TCHAR				m_acPickedName[MAX_PATH];

	U2N2MeshPtr			m_spMesh;
	U2MeshGroupPtr		m_spLitSphere;
	U2NodePtr			m_spTrnNode;

	class Terrain*		m_pTerrain;
	class U2Picker*		m_pPicker;

	float				m_fVertDist;


	int					m_iNumRays;
	float				m_fHalfAngle;
	float*				m_afCos;
	float*				m_afSin;
	float*				m_afTolerance;

	bool				m_bAllowedCameraMove;

	class CGun*			m_pCurGun;

	std::vector<Mtrl>				m_vMtrl;
	std::vector<IDirect3DTexture9*> m_vTex;

private:
	ShadowMapApp();

	virtual void CreateRenderer();
	
	virtual bool ProcessUnbufferedKeyInput();
	void UpdateCamera();


	void		CreateScene ();		
	U2N2Mesh*	CreateTerrain();	


	//virtual void MoveForward();
	//virtual void MoveBackward();
	void BeginGunTrasition(CGun* pNextGun);
		

};
