#pragma once 


//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

//#include <windows.h>
//#include <assert.h>
//#include <tchar.h>
////#include <commctrl.h> // for InitCommonControls() 
//
//#include <U2Application/U2WinApp.h>
//#include <U2_3D/Src/main/U2Scene.h>

//#include <ThirdPersonCamera.h>

class AssetViewApp : public WinApp
{

public:
	

	virtual bool		OnInitialize();
	virtual void		OnTerminate();
	virtual void		OnIdle();
	virtual bool		OnKeyDown(unsigned char ucKey, int x, int y);
	virtual bool		OnMouseClick(int button, int state, int x, int y, 
		unsigned int uModifiers);

	static AssetViewApp*		Create();
	
	static AssetViewApp* ms_pSingleton;


protected:

	
	U2NodePtr			m_spRootNode;
	U2Culler			m_culler;

	U2XMeshLoaderPtr	m_spMeshLoader;
	U2SceneMgrPtr		m_spSceneMgr;
	U2CameraNodePtr		m_spCamNode;
	

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
	

private:
	AssetViewApp();

	virtual void CreateRenderer();

	void		AdjuestVerticalDistance();
	bool		AllowMotion (float fSign);

	virtual bool ProcessUnbufferedKeyInput();
	void UpdateCamera();


	void		CreateScene ();	
	U2N2Mesh*	CreateTower();
	U2N2Mesh*	CreateSkinned();
	U2N2Mesh*	CreateBouncingBall();
	U2N2Mesh*	CreateTerrain();	
	

	virtual void MoveForward();
	virtual void MoveBackward();

	
};
