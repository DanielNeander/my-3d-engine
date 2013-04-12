#pragma once

#include "resource.h"

/***********      .---.         .-"-.      *******************\
* -------- *     /   ._.       / ?` \     * ---------------- *
* Author's *     \_  (__\      \_°v?/     * humus@rogers.com *
*   note   *     //   \\       //   \\     * ICQ #47010716    *
* -------- *    ((     ))     ((     ))    * ---------------- *
*          ****--""---""-------""---""--****                  ********\
* This file is a part of the work done by Humus. You are free to use  *
* the code in any way you like, modified, unmodified or copy'n'pasted *
* into your own work. However, I expect you to respect these points:  *
*  @ If you use this file and its contents unmodified, or use a major *
*    part of this file, please credit the author and leave this note. *
*  @ For use in anything commercial, please request my approval.      *
*  @ Share your work and ideas too as much as you can.                *
\*********************************************************************/

#include <EngineCore/Direct3D11/D3D11App.h>
#include <EngineCore/Math/Noise.h>
#include <EngineCore/Math/Frustum.h>
#include <TerrainLib/TerrainLib.h>

#include <ModelLib/database.h>
#include <ModelLib/M2ModelEdit.h>
#include "M2Edit.h"
#include "InterfacePointers.h"
#include "AppSettings.h"
#include "GraphicsTypes.h"

#include "ShadowCamera.h"

#include "ParticleSystem.h"

#define TERRAIN_SIZE 512
#define INDEX_COUNT ((TERRAIN_SIZE - 1) * (2 * TERRAIN_SIZE + 1) - 1)

#define NOISE_SIZE 128



typedef unsigned int CAMERA_NAVIGATION_MODE;

#define	CAMERA_NAV_DISABLED 0
/// Virtual trackball around the point of interest of the camera
#define	CAMERA_NAV_TRACKBALL 1
/// Fly mode, quake style
#define CAMERA_NAV_FLY 2


class M2Loader;
struct RenderData;
class Dx11TextHelper;

namespace VCloud {
	class VCloudSkySystem;
}


class App : public D3D11App {
public:
	App();

	char *getTitle() const { return "Terrain"; }

	void moveCamera(const vec3 &dir);
	void resetCamera();

	bool onKey(const uint key, const bool pressed);
	void onSize(const int w, const int h);

	void exitAPI();

	virtual bool onMouseMove(const int x, const int y, const int deltaX, const int deltaY);
	virtual bool onMouseButton(const int x, const int y, const MouseButton button, const bool pressed);

	Renderer* getRenderer() { return renderer; }



	bool init();		
	bool load();
	
	bool FrustumTestLoad();
	
	void ReleaseRes();
	void ReleaseTerrain2();
	
	void InitPRT();

	void drawFrame();
	void drawModel(const RenderData& rd, const mat4& world, const mat4& view, const mat4& proj);
	void drawShadowedModels();

	void LoadTerrain();
	void LoadModel(const noVec3 pos);	
	void CreateRenderModel(M2Loader* m2);
	void UpdateModelInfo(M2Loader* m2);

	inline ID3D11Device *GetDevice(void) { return device; }
	inline ID3D11DeviceContext* GetContext(void) { return context; }
	float GetAspectRatio(void);

	void SetDefaultRenderTarget(void);
	
	// SSAO
	void DestorySSAO();
	void RenderSSAO(Renderer* pRenderer);
	void LoadSSAO();
	void SSAO_ShadowPass(float fDeltaTime);


	virtual void controls();
	void LoadParticles();	
	void UpdateParticles(float fTime, float fElapsedTime);
	void RenderParticles(float fTime);
	void UpdateCamera(float dt);
	void UpdateM2Particles(float fElapsedTime);
	void RenderM2Particles(float fTime, const mat4& world, const mat4& view, const mat4& proj);
	void LoadM2Particles();
	void DestoryTerrainRes();
	void RenderTerrain2(bool bReset, float fTime, float fElapsedTime );
	void LoadTessellatedTerrain();

	void OnSizeTerrain();
	bool initAPI();

	void LoadSNBTerrain();
	void ResizeSNBTerrain();
	void RenderSNBTerrain();
	void ReleaseingSNBTerrain();
	void UpdateSNBTerrain();
	void CreateNavScene();
	void RenderNavScene();
	bool BuildNavMesh();
	void buildAllTiles();
	unsigned char* buildTileMesh(const int tx, const int ty, const float* bmin, const float* bmax, int& dataSize);
	void cleanupNavDatas();
	void LoadEntity();

	const char* GetStats();
	void getCurrentHeight();	
	void GetRay(D3DXVECTOR3& vPickRayDir, D3DXVECTOR3& vPickRayOrig);
	void GetRay(float* pos, noVec3& vPickRayDir, noVec3& vPickRayOrig);
	noVec3& Pick();
	bool Pick(int x, int y, SceneNode* pRoot);
	bool Pick(SceneNode* pRoot, const noVec3& rayOrig, const noVec3& rayDir);
	void SetupLight();
	void RenderTiles();
	void interpretCurrentTrackballAction (bool& doRotate, bool& doPan, bool& doZoom);		
	void RenderPSSMScenes(float fDeltaTime);

	bool onKeyAI(const uint key, const bool pressed);
	void NewEntity(int nSlot);

	DWORD GetMiliTime() const {return globalTime; }
	float GetSecTime() const { return globalTime * 0.001; }

	class dtNavMesh* getNavMesh() { return m_navMesh; }
	void AddPath();
	void ExecuteAI();
	
	void LoadPhysics();	
	void RenderPhysics();
	bool CreateBox();

	SceneNode* GetWorldRoot() const { return m_WorldRoot; }
	SceneNode* GetActorRoot() const { return m_ActorRoot; }
	SceneNode* GetSceneRoot() const { return m_SceneRoot; }

	Dx11TextHelper* TextRender() const { return text_; }
	BaseCamera* GetCamera() const { return ActiveCam_; }
	void UpdateRibbonEmitters();
	void RenderRibbions(float fTime, const mat4& world, const mat4& view, const mat4& proj);
	void LoadM2Ribbons();

	int		GetBlendMode(int index) const { return blendModes_[index]; }
	int		GetBlendMode() const { return m_currBlendState; }

	ShadowMapCamera* GetShadowCam() const { return pShadowCam_; }
	void drawShadowed(const RenderData& rd, Mesh* pMesh, ShaderID id);
	void drawShadow( const RenderData& rd, Mesh* mesh, ShaderID shaderid /*= rd.shader*/ );
	void drawSSAO( const RenderData& rd, Mesh* mesh, ShaderID shaderid /*= rd.shader*/ );


	// Indexed Light Deferred
	void LoadeIndexedDeferred();
	void CreateRenderTargets();
	void CreateLightBuffer();
	void ComputeLightTiles();
	void RenderDeferredScene(float fDeltaTime);
	void RenderLID(float fDeltaTime);
	void OnSizeLID();

	static float GetTerrainHeight(const noVec3& StartPoint, float DistToSurface = 0.f);

	void RecreateBuffers();
	void RenderPass_Main();
	void DrawFullScreenQuad();
protected:
	void InitRenderer();
	int FindPath(float* pStartPos, float* pEndPos, int nPathSlot, int nTarget);
	void LoadShaders();
	// Shadow
	void LoadCascadedShadows();
	void RenderCascasedShadowedScene();
	void UpdateViewerCameraNearFar ();
	// Deferred Shading
	bool InitDeferredShading();
	void DrwaDeferredPass();
	
	// Sky
	void UpdateSKy();
	void RenderSky(float fDeltaTime);
	void LoadSky();
	
	// Water
	void LoadWater();
	void ResetWater();
	void RenderWater(float fElapsedTime);	
	void RenderWaterCaustics();
	void RenderPass_Reflection(float fDeltaTime);
	void LoadOcean();
	void UpdateOcean();
	void RenderOcean();
	
	// Lights
	void LoadLights();
	void RenderLights(float fDeltaTime);
	void LoadClipmap();
	void PSSM_ShadowPass(float fDeltaTime);
	void DrawTextShadowDebug();
	bool CreateLightMesh();
	void drawZOnly( const RenderData& rd, Mesh* mesh);
	bool CreateCamera();
	void CasCaded_ShadowPass(float fDeltaTime);

	// Physics
	void InitPhysics();
		

public:
	
	M2Loader* loader;
	M2Edit	m2edit;
	ShadowMapCamera*	DefCam_;
	ShadowMapCamera*	ActiveCam_;
	class FirstPersonCamera*	FpsCam_;
	class FollowCamera*	FollowCam_;
	ShadowMapCamera*	TopDownCam_;
	ShadowMapCamera*	IsometricCam_;
	ShadowMapCamera*	SideScrollCam_;


	Dx11TextHelper* text_;
	SceneNode*		m_SceneRoot;
	SceneNode*		m_ActorRoot;
	SceneNode*		m_WorldRoot;


	mat4                g_World;
	mat4                g_View;
	mat4                g_Projection;

	int ddt;
	int globalTime;

	ID3D11Query*        PipelineQuery_;
	D3D11_QUERY_DATA_PIPELINE_STATISTICS PipelineQueryData_;

	
	class Mesh* gridMesh;
	Mesh* testMesh[10];

	void GetMouseDelta(int& xdelta, int& ydelta) { xdelta = mMouseDelta.x; ydelta = mMouseDelta.y; }
	void CreateCameras();
	bool MapWindowPointToBufferPoint(unsigned int uiX, unsigned int uiY, float& fX, float& fY, uint32 uiXSize, uint32 uiYSize);
	void TestMath();
	void LoadMD5();
	void LoadEditObjs();
	void RenderEdit(float fDeltaTime, float fTime);
	void ExitPhysics();
	void RenderMeshes(bool zOnly);

	noVec3 rayOrig_, rayTarget_, pickPos_;	
	DepthStateID depthWrite_;
	DepthStateID depthTest_;


	static uint LightTileSize;


	class SSAORenderer*	SSAORender_;
protected:

	Terrain* pTerrain;

	ShadowMapCamera* pShadowCam_;
	class CascadedShadowsManager*	pCascaded_;

	CParticleSystem**                   ppParticleSystem_;
	struct NavDebugDraw*	ddray;	
	
	int blendModes_[10];

	ShaderID	shader;
	TextureID	rt, depthRT;
	VertexFormatID vf;
	VertexBufferID vb;
	IndexBufferID ib;
	BlendStateID m_currBlendState;
		
	
	DWORD	lastTime;
	
	int frameNum;

	int m_speedMousePan;
	int m_speedMouseWheelZoom;
	int m_speedMouseButtonZoom;
	int m_speedPadPan;
	int m_speedPadZoom;
	int m_speedFlyStrafe;
	int m_speedFlyMove;
	bool m_mouseAbsoluteSpeed;

	CHAR stats[4096];
	
	CAMERA_NAVIGATION_MODE m_navMode;
	int m_winLL[2]; // from, absolute
	int m_winUR[2]; // to, absolute

	POINT mMousePos;
	POINT mMouseDelta;

	float m_winLLRelative[2]; // from, relative to win size
	float m_winURRelative[2]; // to, relative to win size

	bool m_absolute; //do we use the absolute as the reference or the relative
	
	bool m_lbPressed;
	bool m_mbPressed;
	bool m_rbPressed;
	

	class EffectSystem* EffectSys_;

	class MsgRoute* msgRoute_;
	class DebugLog* dLog_;
	class Time*		timer_;
	class CombatSystem* combat_;
	dtNavMesh* m_navMesh;

	bool	terrainload_;

	class LIDRenderer*	LIDRender_;
	class LightningEffect*	Lightning_;



	Mesh* currMesh_;
public:

	VCloud::VCloudSkySystem* Sky_;

	ID3D11Texture2D			 *main_color_resource;
	ID3D11ShaderResourceView *main_color_resourceSRV;
	ID3D11RenderTargetView   *main_color_resourceRTV;
	ID3D11Texture2D			 *main_depth_resource;
	ID3D11DepthStencilView   *main_depth_resourceDSV;
	ID3D11ShaderResourceView *main_depth_resourceSRV;
	ID3D11Texture2D			 *main_color_resource_resolved;
	ID3D11ShaderResourceView *main_color_resource_resolvedSRV;

	CComPtr<ID3D11VertexShader> QuadVS_;
	CComPtr<ID3D11PixelShader> QuadPS_;
	CComPtr<ID3D11Buffer>		QuadVB_;
	CComPtr<ID3D11Buffer> QuadPSConstants_;
	CComPtr<ID3D11InputLayout> QuadInputLayout_;

	
	class RasterizerStates* rasterizerStates_;
	class BlendStates* blendStates_;	
	class DepthStencilStates* depthStencilStates_;
	class SamplerStates*	samplerStates_;

	// Light Indexed Deferred 
	class PostProcessor* postProcessor;
	struct DepthStencilBuffer* depthBuffer;
	struct RenderTarget2D* colorTargetMSAA;
	RenderTarget2D* colorTarget;
	RenderTarget2D* deferredOutputTarget;

	RenderTarget2D* normalsTarget;
	RenderTarget2D* diffuseAlbedoTarget;
	RenderTarget2D* specularAlbedoTarget;
	RenderTarget2D* lightingTarget;
		
	struct StructuredBuffer* lightsBuffer;
	struct RWBuffer* lightIndicesBuffer;

	ID3D11ComputeShaderPtr lightTilesCS[MSAAModeGUI::NumValues][RenderingModeGUI::NumValues][NumLightsGUI::NumValues];

	struct CSConstantsLID
	{
		XMMATRIX View;
		XMMATRIX Projection;
		XMMATRIX InvViewProjection;
		Float4Align XMFLOAT3 CameraPosWS;
		Float4Align XMFLOAT2 CameraClipPlanes;
		UINT32 NumTilesX;
		UINT32 NumTilesY;
		UINT32 DisplaySizeX;
		UINT32 DisplaySizeY;
	};
		
	ConstantBuffer<CSConstantsLID>* csConstants;

	// Bullet Physics
	///this is the most important class
	class btDynamicsWorld*		m_dynamicsWorld;
	
	class btAlignedObjectArray<btCollisionShape*>	m_collisionShapes;

	class btBroadphaseInterface*	m_broadphase;

	class btCollisionDispatcher*	m_dispatcher;

	class btConstraintSolver*	m_solver;

	class btDefaultCollisionConfiguration* m_collisionConfiguration;
};

extern BaseApp *g_pApplication;
extern Renderer* gRenderer;
extern class GameWorld*	gGameWorld;
inline BaseApp *GetAppBase(void) { return g_pApplication; }
inline App *GetApp(void) { return (App *)g_pApplication; }

inline bool GetKeyDown(int iVirtualKey)
{
	if(GetFocus()!=GetAppBase()->getWindow()) return false;
	return (GetKeyState(iVirtualKey) & 0xfe) ? true : false;
}

// returns true if mouse button VK_LBUTTON / VK_RBUTTON / VK_MBUTTON is down
inline bool GetMouseDown(int iVirtualKey)
{
	if(GetFocus()!=GetAppBase()->getWindow()) return false;
	return (GetAsyncKeyState(iVirtualKey) & 0x8000) ? true : false;
}

inline const mat4& ViewMat() { return GetApp()->g_View; }
inline const mat4& ProjMat() { return GetApp()->g_Projection; }
inline const mat4& WorldMat() { return GetApp()->g_World;  }
inline M2ModelEdit& ModelEdit() { return (GetApp()->m2edit); }
inline ID3D11Device* D3D11Dev() { return GetApp()->GetDevice(); }
inline ID3D11DeviceContext* D3D11Context() { return GetApp()->GetContext(); }
inline float		GetTime() { return GetApp()->GetSecTime(); }
inline BaseCamera* GetCamera() { return GetApp()->GetCamera(); }
inline Renderer* GetRenderer() { return GetApp()->getRenderer(); }

extern ShaderID gSkinned;
extern ShaderID gPssmSkinned;
extern ShaderID gPssm;
extern ShaderID gColorSsao;
extern ShaderID gColorPssm;
extern ShaderID gColorPssm2;
extern ShaderID gSkinnedColorPssm;
extern ShaderID gSkinnedColorSsao;
extern ShaderID gColor;
extern ShaderID	gFireShader;
extern ShaderID	gTreeShader;
// array of crop matrices
extern std::vector<mat4> mCropMatrix;
extern std::vector<mat4> mTextureMatrix;
extern SamplerStateID gPassmSampler;

