#include "stdafx.h"
#include "GameApp.h"

#include "Terrain/ElevationDataSource.h"
#include "Terrain/AdaptiveModelDX11Render.h"
#include "Terrain/TriangDataSource.h"
#include "Terrain/TerrainPatch.h"
#include "Terrain/SNBTerrainConfigFile.h"


D3DXVECTOR3 g_vDirectionOnSun(-0.295211f, +0.588244f, +0.539928 );
D3DXVECTOR3 g_CameraPos, g_CameraLookAt;

std::vector< std::wstring > g_ConfigFiles; // Configuration file names
WCHAR g_strCameraTrackPath[MAX_PATH_LENGTH] = L"media\\CameraTrack.raw";

enum DISPLAY_GUI_MODE
{
	DGM_NOTHING = 0,
	DGM_FPS,
	DGM_FULL_INFO
};
DISPLAY_GUI_MODE g_DisplayGUIMode = DGM_FULL_INFO;

std::auto_ptr<CElevationDataSource> g_pElevDataSource;
std::auto_ptr<CTriangDataSource> g_pTriangDataSource;

extern CAdaptiveModelDX11Render g_TerrainDX11Render;


const float CLIP_NEAR = 5, CLIP_FAR = 25000;

#ifdef _DEBUG
bool g_bForceRecreateTriang = true;
#else
bool g_bForceRecreateTriang = false;
#endif

WCHAR g_strRawDEMDataFile[MAX_PATH_LENGTH];
WCHAR g_strEncodedRQTTriangFile[MAX_PATH_LENGTH];

// These variables are initialized by ParseConfigurationFile()
int g_iNumColumns = 1024;
int g_iNumRows    = 1024;
int g_iPatchSize = 64;
float g_fElevationSamplingInterval = 160.f;
float g_fElevationScale = 0.1f;

SRenderingParams g_TerrainRenderParams = 
{
	g_fElevationSamplingInterval,
	g_fElevationScale,
	2.f, //m_fScrSpaceErrorBound;
	SRenderingParams::UP_AXIS_Y,
	g_iPatchSize,
	0,1, // Min/max elev
	0, // Num levels in hierarchy
	true // Async execution
};

CAdaptiveModelDX11Render::SRenderParams g_DX11PatchRenderParams;

HRESULT InitTerrainRender()
{
	HRESULT hr = S_OK;

	float fFinestLevelTriangError = g_fElevationSamplingInterval / 4.f;

	g_pTriangDataSource.reset( new CTriangDataSource );

	CHAR str[MAX_PATH];
		
	WideCharToMultiByte( CP_ACP, 0, g_strEncodedRQTTriangFile, -1, str, MAX_PATH, NULL, FALSE );
	
	//hr = DXUTFindDXSDKMediaFileCch( str, MAX_PATH, g_strEncodedRQTTriangFile );

	bool bCreateAdaptiveTriang = g_bForceRecreateTriang;
	if( !bCreateAdaptiveTriang )
	{
		if( SUCCEEDED(hr) )
		{
			// Try load triangulation data file
			hr = g_pTriangDataSource->LoadFromFile(str);
			if( SUCCEEDED(hr) )
			{
				if( g_pTriangDataSource->GetNumLevelsInHierarchy() != g_pElevDataSource->GetNumLevelsInHierarchy() ||
					g_pTriangDataSource->GetPatchSize() != g_pElevDataSource->GetPatchSize() )
					bCreateAdaptiveTriang =  true; // Incorrect parameters
			}
			else
				bCreateAdaptiveTriang = true; // Loading failed
		}
		else
			bCreateAdaptiveTriang = true; // File not found
	}

	// Init empty adaptive triangulation data source if file was not found or other problem occured
	if( bCreateAdaptiveTriang )
	{
		g_pTriangDataSource->Init( g_pElevDataSource->GetNumLevelsInHierarchy(), g_pElevDataSource->GetPatchSize(), fFinestLevelTriangError );
	}

	g_TerrainDX11Render.Init(g_TerrainRenderParams, g_DX11PatchRenderParams, g_pElevDataSource.get(), g_pTriangDataSource.get() );

	// Create adaptive triangulation if file was not found or other problem occured
	if( bCreateAdaptiveTriang )
	{
		g_TerrainDX11Render.ConstructPatchAdaptiveTriangulations();
		hr = g_pTriangDataSource->SaveToFile(str);
	}

	SPatchBoundingBox TerrainAABB;
	g_TerrainDX11Render.GetTerrainBoundingBox(TerrainAABB);

	return S_OK;
}

// Loads the selected scene
HRESULT LoadScene()
{
	memset( g_strRawDEMDataFile, 0, sizeof(g_strRawDEMDataFile) );
	memset( g_strEncodedRQTTriangFile, 0, sizeof(g_strEncodedRQTTriangFile) );
	// Get selected config file
	int iSelectedConfigFile = 0;
	// Parse the config file
	if( FAILED(ParseConfigurationFile( g_ConfigFiles[iSelectedConfigFile].c_str() )) )
	{
		//LOG_ERROR(_T("Failed to load config file %s"), g_ConfigFiles[iSelectedConfigFile].c_str() );
		return E_FAIL;
	}

	g_TerrainRenderParams.m_fElevationSamplingInterval = g_fElevationSamplingInterval;
		
	// Create data source
	try
	{

		g_pElevDataSource.reset( new CElevationDataSource(g_strRawDEMDataFile, g_iPatchSize) );
	}
	catch(const std::exception &)
	{
		LOG_ERROR(_T("Failed to create elevation data source"));
		return E_FAIL;
	}

	g_TerrainRenderParams.m_iNumLevelsInPatchHierarchy = g_pElevDataSource->GetNumLevelsInHierarchy();
	g_TerrainRenderParams.m_fGlobalMinElevation = g_pElevDataSource->GetGlobalMinElevation() * g_fElevationScale ;
	g_TerrainRenderParams.m_fGlobalMaxElevation = g_pElevDataSource->GetGlobalMaxElevation() * g_fElevationScale ;
	g_TerrainRenderParams.m_iPatchSize = g_pElevDataSource->GetPatchSize();
	g_TerrainRenderParams.m_fElevationScale = g_fElevationScale;

	return S_OK;
}

void App::ReleaseingSNBTerrain()
{
	g_TerrainDX11Render.OnD3D11ReleasingSwapChain( NULL );
}

void App::ResizeSNBTerrain()
{
	if (!terrainload_) return;
	if (!device) return;

	HRESULT hr;
	ID3D11Texture2D* pBackBuffer;
	hr = swapChain->GetBuffer( 0, __uuidof( *pBackBuffer ), ( LPVOID* )&pBackBuffer );
	DXGI_SURFACE_DESC pBBufferSurfaceDesc;
	ZeroMemory( &pBBufferSurfaceDesc, sizeof( DXGI_SURFACE_DESC ) );
	if( SUCCEEDED( hr ) )
	{
		D3D11_TEXTURE2D_DESC TexDesc;
		pBackBuffer->GetDesc( &TexDesc );
		pBBufferSurfaceDesc.Width = ( UINT )TexDesc.Width;
		pBBufferSurfaceDesc.Height = ( UINT )TexDesc.Height;
		pBBufferSurfaceDesc.Format = TexDesc.Format;
		pBBufferSurfaceDesc.SampleDesc = TexDesc.SampleDesc;
		SAFE_RELEASE( pBackBuffer );
	}

	float fAspectRatio = pBBufferSurfaceDesc.Width / ( FLOAT )pBBufferSurfaceDesc.Height;
	/*pcamera_->setFOV(RAD2DEG(noMath::PI / 4));
	pcamera_->setNear(CLIP_NEAR);
	pcamera_->setFar(CLIP_FAR);*/
	ActiveCam_->SetAspect(fAspectRatio);
	ActiveCam_->ComputeProjection();

	//g_Camera.SetProjParams( noMath::PI / 4, fAspectRatio, 50.f, 250000.0f );

	g_TerrainDX11Render.OnD3D11ResizedSwapChain( device, swapChain, &pBBufferSurfaceDesc, NULL );

	BaseCamera* pCam = GetApp()->ActiveCam_;
	D3DXMATRIX mProj;
	//D3DXMATRIX mView;

	const unsigned int size16 = sizeof(float) * 16;		 
	//memcpy(&mView, pCam->getViewMatrix(), size16);
	memcpy(&mProj, pCam->getProjectionMatrix(), size16);

	g_TerrainDX11Render.SetViewFrustumParams( pBBufferSurfaceDesc.Width, pBBufferSurfaceDesc.Height, mProj );

}

void App::LoadSNBTerrain()
{
	terrainload_ = true;

	g_ConfigFiles.push_back( L"Default_Config.txt");

	g_TerrainDX11Render.SetQuadTreePreviewPos(10, 290, 200, 200);

	LoadScene();

	// Setup the camera's view parameters
	D3DXVECTOR3 vecEye( 2643.75f, 2178.89f, 2627.14f );
	D3DXVECTOR3 vecAt ( 2644.52f, 2178.71f, 2627.74f );
	//D3DXVECTOR3 vecEye( 150.75f, 100.89f, 150.14f );
	//D3DXVECTOR3 vecAt ( 151.52f, 100.71f, 150.74f );
	//g_Camera.SetViewParams( &vecEye, &vecAt );
	//g_Camera.SetRotateButtons(true, false, false);
	ActiveCam_->setFrom((float*)vecEye);
	ActiveCam_->setTo((float*)vecAt);
	ActiveCam_->setFOV(RAD2DEG(noMath::PI / 4));
	ActiveCam_->setNear(CLIP_NEAR);
	ActiveCam_->setFar(CLIP_FAR);
	ActiveCam_->SetAspect(GetAspectRatio());
	ActiveCam_->computeModelView();	
	ActiveCam_->ComputeProjection();
	

	static bool bFirstTime = true;
	if( bFirstTime )
	{
		InitTerrainRender();
		bFirstTime = false;
	}
	g_TerrainDX11Render.OnD3D11CreateDevice( device, context );

	//ResizeSNBTerrain();

	D3DXCOLOR SunColor(1,1,1,1), AmbientLight(0,0,0,0);
	g_TerrainDX11Render.SetSunParams(g_vDirectionOnSun, SunColor, AmbientLight);
	g_TerrainDX11Render.EnableAdaptiveTriangulation(true);
}

noVec3& App::Pick()
{
	D3DXVECTOR3 orig, dir;

	GetRay(dir, orig);

	float DistToSurface = 0.f;
	g_TerrainDX11Render.RayCast(orig, dir, &DistToSurface);

	/*float height = DistToSurface;
	
	pickPos_.x = orig.x;
	pickPos_.y = height;
	pickPos_.z = orig.z;*/
	{	
		D3DXVECTOR3 temp  = orig + dir * DistToSurface;
		pickPos_ = noVec3(temp.x, temp.y, temp.z);	
	
		orig = temp;
		orig.y += 500.f;
		dir = D3DXVECTOR3(0.0f, -1.0f, 0.f);
		g_TerrainDX11Render.RayCast(orig, dir, &DistToSurface);

		temp  = orig + dir * DistToSurface;
		pickPos_ = noVec3(temp.x, temp.y, temp.z);		
	}		

	//if (g_TerrainDX11Render.GetCurrPatch())
	//	pickPos_.y = g_TerrainDX11Render.GetCurrPatch()->GetHeight(noVec2(pickPos_.x, pickPos_.z));

	return pickPos_;	
}

bool App::MapWindowPointToBufferPoint(unsigned int uiX,
	unsigned int uiY, float& fX, float& fY, uint32 uiXSize, uint32 uiYSize)
{
	//const NiRenderTargetGroup* pkCurrentRTGroup = pkTarget;
	if (uiXSize && uiYSize)
	{
		fX = (float)(uiX) / (float)uiXSize;
		fY = 1.0f - (float)(uiY) / (float)uiYSize;

		return true;
	}
	else
	{
		return false;
	}
}

void App::GetRay(float* pos, noVec3& vPickRayDir, noVec3& vPickRayOrig) {

	noVec3 result;
	GetCamera()->project(pos[0], pos[1], pos[1], GetApp()->getWidth(), GetApp()->getHeight(), result);

	noVec3 rayOrigin, rayTarget;
	//rayOrigin = m * nearPoint;
	//rayTarget = m * midPoint;	

	ActiveCam_->unProject( result.x, result.y, 0.0f, getWidth(), getHeight(), rayOrigin);
	ActiveCam_->unProject( result.x, result.y, 1.0f, getWidth(), getHeight(), rayTarget);

	noVec3 rayDirection = rayTarget - rayOrigin;
	rayDirection.Normalize();

	rayOrig_ = rayOrigin;
	rayTarget_ = rayTarget;

	vPickRayOrig = rayOrig_;
		
	vPickRayDir = rayDirection;
}

void App::GetRay(D3DXVECTOR3& vPickRayDir, D3DXVECTOR3& vPickRayOrig)
{
	if (!ActiveCam_) return;
	D3DXMATRIX pmatProj(ActiveCam_->getProjectionMatrix());

	POINT pos;
	//GetCursorPos(&pos);
	//ScreenToClient(getWindow(), &pos);
	pos = mMousePos;

	//pos.x = getWidth() / 2;
	//pos.y = getHeight() / 2;
	
	noVec3 orig, dir;
	ActiveCam_->WindowPointToRay(pos.x, pos.y, orig, dir, width, height);
	vPickRayOrig.x =orig.x;
	vPickRayOrig.y =orig.y;
	vPickRayOrig.z =orig.z;
	vPickRayDir.x = dir.x;
	vPickRayDir.y = dir.y;
	vPickRayDir.z = dir.z;

#if 0
	// Compute the vector of the pick ray in screen space	
	D3DXVECTOR3 v;
	v.x = ( ( ( 2.0f * pos.x ) / getWidth() ) - 1 ) / pmatProj._11;
	v.y = -( ( ( 2.0f * pos.y ) / getHeight() ) - 1 ) / pmatProj._22;
	v.z = 1.0f;

	

	// Get the inverse view matrix
	D3DXMATRIX matView(ActiveCam_->getViewMatrix());		
	
			
	D3DXMATRIX mWorldView = matView;
	D3DXMATRIX m;
	D3DXMatrixInverse( &m, NULL, &mWorldView );		

	// Transform the screen space pick ray into 3D space
	vPickRayDir.x = v.x * m._11 + v.y * m._21 + v.z * m._31;
	vPickRayDir.y = v.x * m._12 + v.y * m._22 + v.z * m._32;
	vPickRayDir.z = v.x * m._13 + v.y * m._23 + v.z * m._33;
	vPickRayOrig.x = m._41;
	vPickRayOrig.y = m._42;
	vPickRayOrig.z = m._43;
#endif

#if 1
	//float screenX = (float)pos.x / getWidth();
	//float screenY = (float)pos.y / getHeight();
	//float nx = (2.0f * screenX) - 1.0f;
	//float ny = 1.0f - (2.0f * screenY);
	//noVec3 nearPoint(nx, ny, -1.f);
	//// Use midPoint rather than far point to avoid issues with infinite projection
	//noVec3 midPoint (nx, ny,  0.0f);

	//noMat4  matView(pcamera_->getViewMatrix());		
	//noMat4 matProj(pcamera_->getProjectionMatrix());
	//noMat4 mViewProj = matProj * matView;
	//noMat4 m;	
	//m = mViewProj.Inverse();
	//		
	noVec3 rayOrigin, rayTarget;
	//rayOrigin = m * nearPoint;
	//rayTarget = m * midPoint;	
		
	ActiveCam_->unProject( pos.x, pos.y, 0.0f, getWidth(), getHeight(), rayOrigin);
	ActiveCam_->unProject( pos.x, pos.y, 1.0f, getWidth(), getHeight(), rayTarget);
		
	noVec3 rayDirection = rayTarget - rayOrigin;
	rayDirection.Normalize();

	rayOrig_ = rayOrigin;
	rayTarget_ = rayTarget;

	// Transform the screen space pick ray into 3D space	

	vPickRayDir.x = rayDirection.x;
	vPickRayDir.y = rayDirection.y;
	vPickRayDir.z = rayDirection.z;
	vPickRayOrig.x = rayOrigin.x;
	vPickRayOrig.y = rayOrigin.y;
	vPickRayOrig.z = rayOrigin.z;
#endif
}

float App::GetTerrainHeight(const noVec3& StartPoint, float DistToSurface)
{
	if (!GetApp()->terrainload_) return DistToSurface;	
	g_TerrainDX11Render.RayCast(D3DXVECTOR3(StartPoint.ToFloatPtr()), D3DXVECTOR3(0,-1,0), &DistToSurface);
	float fTerrainHeightUnderCamera = StartPoint.y + DistToSurface;
	return fTerrainHeightUnderCamera;
}

void App::UpdateSNBTerrain()
{	
	if (!terrainload_) return;
	
	ActiveCam_->getFrom((float*)g_CameraPos);
	ActiveCam_->getTo((float*)g_CameraLookAt);

	// Currently intersections with the base mesh are not supported
	D3DXVECTOR3 StartPoint;
	StartPoint.x = g_CameraPos.x;
	StartPoint.y = g_pElevDataSource->GetGlobalMinElevation()*g_fElevationScale - g_fElevationSamplingInterval;
	StartPoint.z = g_CameraPos.z;
	float DistToSurface = 0.f;
	g_TerrainDX11Render.RayCast(StartPoint, D3DXVECTOR3(0,1,0), &DistToSurface);
	float fTerrainHeightUnderCamera = StartPoint.y + DistToSurface;

	float fMinimalDistToSurfae = max(ActiveCam_->GetNear() *2, g_fElevationSamplingInterval * 5.f);
	fTerrainHeightUnderCamera += fMinimalDistToSurfae;
	if( g_CameraPos.y < fTerrainHeightUnderCamera )
	{
		g_CameraPos.y = Min(3000.f, fTerrainHeightUnderCamera);
		D3DXVECTOR3 dir;
		ActiveCam_->getDir((float*)dir);
		g_CameraLookAt = g_CameraPos + dir;
		//g_Camera.SetViewParams( &g_CameraPos, &g_CameraLookAt );	
		
		ActiveCam_->setFrom((float*)g_CameraPos);
		ActiveCam_->setTo((float*)g_CameraLookAt);
		
		ActiveCam_->computeModelView();		
	}
	
	BaseCamera* pCam = GetApp()->ActiveCam_;
	//D3DXMATRIX mProj;
	//D3DXMATRIX mView;

	//const unsigned int size16 = sizeof(float) * 16;		 
	//memcpy(&mView, pCam->getViewMatrix(), size16);
	//memcpy(&mProj, pCam->getProjectionMatrix(), size16);

	g_TerrainDX11Render.UpdateModel( g_CameraPos, pCam->getProjectionMatrix() );
}

void App::RenderSNBTerrain()
{
	if (!terrainload_) return;

	//float ClearColor[4] = { 98.f/255.f, 98.f/255.f, 98.f/255.f, 0.0f };

	/*renderer->changeToMainFramebuffer();	

	ID3D11DeviceContext* pContext =  GetApp()->GetContext();

	ID3D11RenderTargetView* pBackBufferRTV; 
	ID3D11DepthStencilView* pBackBufferDSV;

	pContext->OMGetRenderTargets(1, &pBackBufferRTV, &pBackBufferDSV);*/

	//context->ClearRenderTargetView(pBackBufferRTV , ClearColor );

	// Clear the depth stencil
	//context->ClearDepthStencilView( pBackBufferDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	// Get the projection & view matrix from the camera class
	D3DXMATRIX mProj;
	D3DXMATRIX mView;

	BaseCamera* pCam = GetApp()->ActiveCam_;
	const unsigned int size16 = sizeof(float) * 16;		 
	memcpy(&mView, pCam->getViewMatrix(), size16);
	memcpy(&mProj, pCam->getProjectionMatrix(), size16);
	D3DXMATRIX mCameraViewProjection = mView * mProj;

	//D3DXVECTOR3 SunLightDir = *g_LightCamera.GetWorldAhead();
	//g_vDirectionOnSun = -SunLightDir;
	

	bool bWireframe = false;
	bool bShowBoundBoxes = false;
	// Render terrain
	g_TerrainDX11Render.UpdateTerrain( context, g_CameraPos, mCameraViewProjection );
	g_TerrainDX11Render.Render( context, g_CameraPos, mCameraViewProjection, bShowBoundBoxes, g_DisplayGUIMode == DGM_FULL_INFO, bWireframe, false);

	

	/*static DWORD dwTimefirst = GetTickCount();
	if ( GetTickCount() - dwTimefirst > 5000 )
	{    
		OutputDebugString( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
		OutputDebugString( L"\n" );
		dwTimefirst = GetTickCount();
	}	*/
}