// Copyright 2011 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
#include "stdafx.h"

#include "ElevationDataSource.h"
#include "AdaptiveModelDX11Render.h"
#include "TriangDataSource.h"
#include "TerrainPatch.h"
#include "ConfigFile.h"
#include <io.h>
#include "Oscilloscope.h"
#include "TaskMgrTBB.h"

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

CFirstPersonCamera          g_Camera;               // A model viewing camera
D3DXMATRIX                  g_CameraViewMatrix;
D3DXVECTOR3                 g_CameraPos;
D3DXVECTOR3                 g_CameraLookAt;
CFirstPersonCamera          g_LightCamera;      // Camera for controlling light direction
bool g_bRightMouseDown = false; 
D3DXVECTOR3 g_vDirectionOnSun(-0.295211f, +0.588244f, +0.539928 );

CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg             g_SettingsDlg;          // Device settings dialog
CDXUTTextHelper*            g_pTxtHelper = NULL;
CDXUTDialog                 g_HUD;                  // dialog for standard controls
CDXUTDialog                 g_SampleUI;             // dialog for sample specific controls


std::auto_ptr<CElevationDataSource> g_pElevDataSource;
std::auto_ptr<CTriangDataSource> g_pTriangDataSource;

CAdaptiveModelDX11Render g_TerrainDX11Render;

COscilloscope g_Oscilloscope;

std::vector< std::wstring > g_ConfigFiles; // Configuration file names

enum DISPLAY_GUI_MODE
{
    DGM_NOTHING = 0,
    DGM_FPS,
    DGM_FULL_INFO
};
DISPLAY_GUI_MODE g_DisplayGUIMode = DGM_FULL_INFO;
bool g_bShowHelp = true;

double g_dPerfFrequency;
const double g_dMeasurementInterval = 0.5;
LONGLONG g_llPrevTick = 0;
LONGLONG g_llTrianglesRendered = 0;
double g_dCurrMTrPS = 0;

bool g_bActivateAutopilotAtStartup = false;
WCHAR g_strCameraTrackPath[MAX_PATH_LENGTH] = L"media\\CameraTrack.raw";
FILE *g_pCameraTrackFile = NULL;
bool g_bReproducingCameraTrack = false;
double g_dCamTrackCaptureStartTime;
double g_dPrevCamTrackCapturingTime;
struct SCameraTrackElem
{
    D3DXMATRIX m_CamWorldMatrix;
    double m_dTime;
};
std::vector<SCameraTrackElem> g_CameraTrack;
double g_dCamTrackReproductionStartTime = -1;
UINT g_iCurrTrackPos = 0;

WCHAR g_strPerfDataPath[MAX_PATH_LENGTH] = L"media\\PerfData.txt";
bool g_bReportPerformance = true;
FILE *g_pPerfDataFile = NULL;
const double g_dPerformanceReportInterval = 0.5;
double g_dNextPerfReportTime = -1;
double g_dPrevPerfReportTime = -1;
int g_iFramesRendered = -1;


//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
enum CONTROL_IDS
{
    IDC_TOGGLEFULLSCREEN   = 1,
    IDC_TOGGLEREF,
    IDC_CHANGEDEVICE,

    IDC_SHOW_WIREFRAME_CHK,
    IDC_SHOW_BOUND_BOXES_CHK,
    IDC_ENABLE_ADAPTIVE_TRIANGULATION_CHK,
    IDC_UPDATE_MODEL_CHK,
    IDC_ASYNC_CHK,
    IDC_SORT_PATCHES_BY_DIST_CHK,
    IDC_SCR_SPACE_THRESHOLD_STATIC,
    IDC_SCR_SPACE_THRESHOLD_SLIDER,
    IDC_HEIGHT_MAP_MORPH_CHK,
    IDC_NORMAL_MAP_MORPH_CHK,
    IDC_CONFIG_STATIC,
    IDC_CONFIG_COMBO,
    IDC_COMPRESS_NORMAL_MAP_CHK,
    IDC_NORMAL_MAP_LOD_BIAS_COMBO,
    IDC_START_STOP_AUTOPILOT_BTN,
    IDC_STATIC = 1000
};

// UI definition
#define NUM_POSITIONS_IN_SLIDER 1000

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK MouseProc( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, bool bSideButton1Down,
                         bool bSideButton2Down, int nMouseWheelDelta, int xPos, int yPos, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext );
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                 float fElapsedTime, void* pUserContext );

void InitApp();
void RenderText();


#ifdef _DEBUG
    bool g_bForceRecreateTriang = false;//true;
#else
    bool g_bForceRecreateTriang = false;
#endif

TCHAR g_strRawDEMDataFile[MAX_PATH_LENGTH];
TCHAR g_strEncodedRQTTriangFile[MAX_PATH_LENGTH];

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

// Loads the selected scene
HRESULT LoadScene()
{
    memset( g_strRawDEMDataFile, 0, sizeof(g_strRawDEMDataFile) );
    memset( g_strEncodedRQTTriangFile, 0, sizeof(g_strEncodedRQTTriangFile) );
    // Get selected config file
    int iSelectedConfigFile = (int)g_SampleUI.GetComboBox( IDC_CONFIG_COMBO )->GetSelectedData();
    // Parse the config file
    if( FAILED(ParseConfigurationFile( g_ConfigFiles[iSelectedConfigFile].c_str() )) )
    {
        LOG_ERROR(_T("Failed to load config file %s"), g_ConfigFiles[iSelectedConfigFile].c_str() );
        return E_FAIL;
    }

    g_TerrainRenderParams.m_fElevationSamplingInterval = g_fElevationSamplingInterval;

    g_SampleUI.GetSlider( IDC_SCR_SPACE_THRESHOLD_SLIDER )->SetValue( (int)g_TerrainRenderParams.m_fScrSpaceErrorBound );
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
    g_TerrainRenderParams.m_fGlobalMinElevation = g_pElevDataSource->GetGlobalMinElevation() * g_fElevationScale;
    g_TerrainRenderParams.m_fGlobalMaxElevation = g_pElevDataSource->GetGlobalMaxElevation() * g_fElevationScale;
    g_TerrainRenderParams.m_iPatchSize = g_pElevDataSource->GetPatchSize();

    return S_OK;
}

HRESULT InitTerrainRender()
{
    HRESULT hr;

    float fFinestLevelTriangError = g_fElevationSamplingInterval / 4.f;
    
    g_pTriangDataSource.reset( new CTriangDataSource );
    
    WCHAR str[MAX_PATH];
    hr = DXUTFindDXSDKMediaFileCch( str, MAX_PATH, g_strEncodedRQTTriangFile );

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

    V( g_TerrainDX11Render.Init(g_TerrainRenderParams, g_DX11PatchRenderParams, g_pElevDataSource.get(), g_pTriangDataSource.get() ) );

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

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    int argc;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    // Insert default config
    g_ConfigFiles.push_back( L"Default_Config.txt");
    // Read additional config files from command line
    for(int iArg=1; iArg < argc; iArg++)
        g_ConfigFiles.push_back( argv[iArg] );

    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D11) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set DXUT callbacks
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackMouse( MouseProc );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );

    InitApp();
    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true );
    DXUTCreateWindow( L"TerrainRender" );

    // Only require 10-level hardware, change to D3D_FEATURE_LEVEL_11_0 to require 11-class hardware
    DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 1280, 1024 );

    DXUTMainLoop(); // Enter into the DXUT render loop

	if( g_bActivateAutopilotAtStartup )
	{
		OnGUIEvent( EVENT_BUTTON_CLICKED, IDC_START_STOP_AUTOPILOT_BTN, g_SampleUI.GetButton(IDC_START_STOP_AUTOPILOT_BTN), NULL );
	}
    
    return DXUTGetExitCode();
}

//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    HRESULT hr;

    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

    // Init GUI

    g_HUD.SetCallback( OnGUIEvent );
    int iY = 30;
    int iYo = 26;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += iYo, 170, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += iYo, 170, 22, VK_F2 );

    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;
    g_SampleUI.AddCheckBox( IDC_SHOW_WIREFRAME_CHK, L"Wireframe", 0, iY += 24, 180, 22 );
    g_SampleUI.AddButton( IDC_START_STOP_AUTOPILOT_BTN, L"Start Autopilot (F6)", 0, iY += 24, 160, 22, VK_F6 );

    g_SampleUI.AddStatic( IDC_CONFIG_STATIC, L"Config:", 0, iY += 24, 180, 22);
    g_SampleUI.AddComboBox( IDC_CONFIG_COMBO, 0, iY += 24, 180, 22);
    for(int iConfig = 0; iConfig < (int)g_ConfigFiles.size(); iConfig++)
        g_SampleUI.GetComboBox( IDC_CONFIG_COMBO )->AddItem(g_ConfigFiles[iConfig].c_str(), (void*)iConfig);
    g_SampleUI.GetComboBox( IDC_CONFIG_COMBO )->SetSelectedByData( 0 );

	g_SampleUI.AddCheckBox( IDC_UPDATE_MODEL_CHK, L"Update model", 0, iY += 24, 180, 22, true );
    g_SampleUI.AddCheckBox( IDC_ASYNC_CHK, L"Async execution", 0, iY += 24, 180, 22, g_TerrainRenderParams.m_bAsyncExecution );
    g_SampleUI.AddCheckBox( IDC_SHOW_BOUND_BOXES_CHK, L"Show bound boxes", 0, iY += 24, 180, 22, false );

    g_SampleUI.AddCheckBox( IDC_ENABLE_ADAPTIVE_TRIANGULATION_CHK, L"Adaptive triang", 0, iY += 24, 180, 22, true );
    
    //g_SampleUI.AddCheckBox( IDC_SORT_PATCHES_BY_DIST_CHK, L"Sort patches by dist", 0, iY += 24, 180, 22, true );
        
    iY += 12;
    g_SampleUI.AddStatic( IDC_SCR_SPACE_THRESHOLD_STATIC, L"Screen space threshold", 0, iY += 24, 180, 22);
    g_SampleUI.AddSlider( IDC_SCR_SPACE_THRESHOLD_SLIDER, 20, iY += 24, 140, 22, 1, 30, (int)g_TerrainRenderParams.m_fScrSpaceErrorBound );

    iY += 24;
    //g_SampleUI.AddCheckBox( IDC_HEIGHT_MAP_MORPH_CHK, L"HM", 0, iY, 60, 22, g_DX11PatchRenderParams.m_bEnableHeightMapMorph );
    g_SampleUI.AddCheckBox( IDC_NORMAL_MAP_MORPH_CHK, L"Normal Map morph", 0, iY+=24, 180, 22, g_DX11PatchRenderParams.m_bEnableNormalMapMorph );

    iY += 12;
    g_SampleUI.AddCheckBox( IDC_COMPRESS_NORMAL_MAP_CHK, L"Compress normal map", 0, iY += 24, 180, 22, g_DX11PatchRenderParams.m_bCompressNormalMap );

    g_SampleUI.AddStatic( IDC_NORMAL_MAP_LOD_BIAS_COMBO + IDC_STATIC, L"Normal map LOD bias:", 0, iY += 24, 180, 22);
    g_SampleUI.AddComboBox( IDC_NORMAL_MAP_LOD_BIAS_COMBO, 0, iY += 24, 180, 22);
    g_SampleUI.GetComboBox( IDC_NORMAL_MAP_LOD_BIAS_COMBO )->AddItem( L"0 (1x)", (void*)0);
    g_SampleUI.GetComboBox( IDC_NORMAL_MAP_LOD_BIAS_COMBO )->AddItem( L"1 (2x)", (void*)1);
    g_SampleUI.GetComboBox( IDC_NORMAL_MAP_LOD_BIAS_COMBO )->AddItem( L"2 (4x)", (void*)2);
    g_SampleUI.GetComboBox( IDC_NORMAL_MAP_LOD_BIAS_COMBO )->AddItem( L"3 (8x)", (void*)3);
    g_SampleUI.GetComboBox( IDC_NORMAL_MAP_LOD_BIAS_COMBO )->AddItem( L"4 (16x)", (void*)4);
    g_SampleUI.GetComboBox( IDC_NORMAL_MAP_LOD_BIAS_COMBO )->SetSelectedByData( (void*)g_DX11PatchRenderParams.m_iNormalMapLODBias );
    
	
	g_TerrainDX11Render.SetQuadTreePreviewPos(10, 290, 200, 200);

    V( LoadScene() );
    
    // Setup the camera's view parameters
    D3DXVECTOR3 vecEye( 2643.75f, 2178.89f, 2627.14f );
    D3DXVECTOR3 vecAt ( 2644.52f, 2178.71f, 2627.74f );
    g_Camera.SetViewParams( &vecEye, &vecAt );
    g_Camera.SetRotateButtons(true, false, false);

    D3DXVECTOR3 vLightFrom = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vSunLightDir = -g_vDirectionOnSun;
    g_LightCamera.SetViewParams( &vLightFrom, &vSunLightDir );
    g_LightCamera.SetScalers( 0.001f, 1.f );
    g_LightCamera.SetInvertPitch( true );
    g_LightCamera.SetRotateButtons( false, false, true );
    
    LARGE_INTEGER PerfFreq, CurrTick;
    QueryPerformanceFrequency( &PerfFreq );
    g_dPerfFrequency = (double)PerfFreq.QuadPart;
    QueryPerformanceCounter( &CurrTick );
    g_llPrevTick = CurrTick.QuadPart;
    g_llTrianglesRendered = 0;
    g_dCurrMTrPS = 0;

    gTaskMgr.Init();
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    TCHAR Str[512];
    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos( 5, 5 );
    g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    if( g_DisplayGUIMode == DGM_FPS )
    {
        _stprintf_s(Str, sizeof(Str)/sizeof(Str[0]), L"%.1lf fps", DXUTGetFPS());
        g_pTxtHelper->DrawTextLine( Str );
    }
    if( g_DisplayGUIMode == DGM_FULL_INFO )
    {
        g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
        g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );
        if( g_pCameraTrackFile != NULL )
        {
            g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f ) );
            g_pTxtHelper->DrawTextLine( L"Capturing camera track!" );
        }
        // Render last frame complexity
        g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        int iOptimalPatchesCount, iVisiblePatchesCount, iTotalTrianglesRendered;
        g_TerrainDX11Render.GetLastFrameComplexity( iOptimalPatchesCount, iVisiblePatchesCount, iTotalTrianglesRendered );
        g_llTrianglesRendered += iTotalTrianglesRendered;

	    LARGE_INTEGER llCurrTick;
        QueryPerformanceCounter( &llCurrTick );
        double dElapsedTime = ((double)(llCurrTick.QuadPart - g_llPrevTick)) / g_dPerfFrequency;
        if( dElapsedTime > g_dMeasurementInterval )
        {
            g_dCurrMTrPS = (double)g_llTrianglesRendered / dElapsedTime / 1000000.0;
            g_llTrianglesRendered = 0;
            g_llPrevTick = llCurrTick.QuadPart;
        }

        _stprintf_s(Str, sizeof(Str)/sizeof(Str[0]),
	                L"Active patches: %4d  Visible Patches: %4d  Total triangles: %6d  MTrPS: %4.1lf", 
                    iOptimalPatchesCount, iVisiblePatchesCount, iTotalTrianglesRendered,
                    g_dCurrMTrPS);
        g_pTxtHelper->DrawTextLine( Str );

        if( g_bShowHelp )
	    {
		    UINT BackBufferHeight = DXUTGetDXGIBackBufferSurfaceDesc()->Height;
		    g_pTxtHelper->SetForegroundColor( D3DXCOLOR(1.0f, 0.75f, 0.0f, 1.0f) );
		    g_pTxtHelper->SetInsertionPos( 10, BackBufferHeight - 22*8 );
		    g_pTxtHelper->DrawTextLine( L"Controls (F1 to hide):\n"
								        L"Move camera: w,s,a,d,q,e; accelerate: shift\n"
									    L"Rotate camera: left mouse button\n"
									    L"Rotate light: right mouse button\n"
									    L"GUI display mode: F5\n"
									    L"Start/Stop autopilot: F6\n");
	    }
    }
    g_pTxtHelper->End();
}


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependent on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
    HRESULT hr;

    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    V_RETURN( g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
    V_RETURN( g_SettingsDlg.OnD3D11CreateDevice( pd3dDevice ) );
    g_pTxtHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15 );

    static bool bFirstTime = true;
    if( bFirstTime )
    {
        V( InitTerrainRender() );
        bFirstTime = false;
    }
    V( g_TerrainDX11Render.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
 
    V( g_Oscilloscope.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext, g_pTxtHelper ) );
    
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_SettingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 50.f, 250000.0f );
    //g_Camera.SetScalers(0.005f, 3000.f); Scalers are set in the OnFrameMove()
    g_Camera.SetRotateButtons(true, false, false);

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_HUD.SetSize( 170, 170 );
    int SampleUIHeight = 400;
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, 150 /*pBackBufferSurfaceDesc->Height - SampleUIHeight*/ );
    g_SampleUI.SetSize( 170, SampleUIHeight );

    g_TerrainDX11Render.OnD3D11ResizedSwapChain( pd3dDevice, pSwapChain, pBackBufferSurfaceDesc, pUserContext );
    g_TerrainDX11Render.SetViewFrustumParams( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, *g_Camera.GetProjMatrix() );
    V( g_Oscilloscope.OnD3D11ResizedSwapChain( pd3dDevice, pSwapChain, pBackBufferSurfaceDesc) );

    RECT OscilloscopePos;
    OscilloscopePos.left = 5;
    OscilloscopePos.top = 70;
    OscilloscopePos.right = 220;
    OscilloscopePos.bottom = 150;
    g_Oscilloscope.SetScreenPosition(OscilloscopePos);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                 float fElapsedTime, void* pUserContext )
{
    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

    float ClearColor[4] = { 98.f/255.f, 98.f/255.f, 98.f/255.f, 0.0f };
	
	pd3dImmediateContext->ClearRenderTargetView( DXUTGetD3D11RenderTargetView(), ClearColor );

    // Clear the depth stencil
    pd3dImmediateContext->ClearDepthStencilView( DXUTGetD3D11DepthStencilView(), D3D11_CLEAR_DEPTH, 1.0, 0 );

    // Get the projection & view matrix from the camera class
    D3DXMATRIX mView = g_CameraViewMatrix;
    D3DXMATRIX mProj = *g_Camera.GetProjMatrix();
    D3DXMATRIX mCameraViewProjection = mView * mProj;

    D3DXVECTOR3 SunLightDir = *g_LightCamera.GetWorldAhead();
    g_vDirectionOnSun = -SunLightDir;
    D3DXCOLOR SunColor(1,1,1,1), AmbientLight(0,0,0,0);
    g_TerrainDX11Render.SetSunParams(g_vDirectionOnSun, SunColor, AmbientLight);
    g_TerrainDX11Render.EnableAdaptiveTriangulation(g_SampleUI.GetCheckBox( IDC_ENABLE_ADAPTIVE_TRIANGULATION_CHK )->GetChecked());
    
    bool bWireframe = g_SampleUI.GetCheckBox(IDC_SHOW_WIREFRAME_CHK)->GetChecked();
    bool bShowBoundBoxes = g_SampleUI.GetCheckBox(IDC_SHOW_BOUND_BOXES_CHK)->GetChecked();
    // Render terrain
    g_TerrainDX11Render.Render( pd3dImmediateContext, g_CameraPos, mCameraViewProjection, bShowBoundBoxes, g_DisplayGUIMode == DGM_FULL_INFO, bWireframe, false);

    // Render oscilloscope
    if( g_DisplayGUIMode == DGM_FULL_INFO )
        g_Oscilloscope.Render( pd3dDevice, pd3dImmediateContext );

    DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
    if( g_DisplayGUIMode == DGM_FULL_INFO )
    {
        g_HUD.OnRender( fElapsedTime );
        g_SampleUI.OnRender( fElapsedTime );
    }
    if( g_DisplayGUIMode > DGM_NOTHING )
        RenderText();
    DXUT_EndPerfEvent();

    static DWORD dwTimefirst = GetTickCount();
    if ( GetTickCount() - dwTimefirst > 5000 )
    {    
        OutputDebugString( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
        OutputDebugString( L"\n" );
        dwTimefirst = GetTickCount();
    }

    g_iFramesRendered++;
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
    g_TerrainDX11Render.OnD3D11ReleasingSwapChain( pUserContext );
    g_Oscilloscope.OnD3D11ReleasingSwapChain();    
    g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11DestroyDevice();
    g_SettingsDlg.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_DELETE( g_pTxtHelper );

    g_Oscilloscope.OnD3D11DestroyDevice();

    g_TerrainDX11Render.OnD3D11DestroyDevice();
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    assert(DXUT_D3D11_DEVICE == pDeviceSettings->ver);
    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE )
        {
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
        }
    }
    return true;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    g_Oscilloscope.SetLastFrameTime(fElapsedTime);

    if( !g_bReproducingCameraTrack )
    {
		g_Camera.SetScalers(0.005f, GetAsyncKeyState(VK_SHIFT) ? 10000.f : 2000.f);
        g_Camera.FrameMove( fElapsedTime );
        
        g_CameraPos = *g_Camera.GetEyePt();
        g_CameraLookAt = *g_Camera.GetLookAtPt();
		
		// Currently intersections with the base mesh are not supported
		D3DXVECTOR3 StartPoint;
		StartPoint.x = g_CameraPos.x;
		StartPoint.y = g_pElevDataSource->GetGlobalMinElevation()*g_fElevationScale - g_fElevationSamplingInterval * 10.f;
		StartPoint.z = g_CameraPos.z;
		float DistToSurface = 0.f;
		g_TerrainDX11Render.RayCast(StartPoint, D3DXVECTOR3(0,1,0), &DistToSurface);
		float fTerrainHeightUnderCamera = StartPoint.y + DistToSurface;

		float fMinimalDistToSurfae = max(g_Camera.GetNearClip()*2, g_fElevationSamplingInterval * 5.f);
		fTerrainHeightUnderCamera += fMinimalDistToSurfae;
		if( g_CameraPos.y < fTerrainHeightUnderCamera )
		{
			g_CameraPos.y = fTerrainHeightUnderCamera;
			g_CameraLookAt = g_CameraPos + *g_Camera.GetWorldAhead();
			g_Camera.SetViewParams( &g_CameraPos, &g_CameraLookAt );
			g_Camera.FrameMove(0);
		}

		g_CameraViewMatrix = *g_Camera.GetViewMatrix();
		
        if( g_pCameraTrackFile )
        {
            if( g_dCamTrackCaptureStartTime < 0 )
            {
                g_dCamTrackCaptureStartTime = fTime;
                g_dPrevCamTrackCapturingTime = -1000.0;
            }

            double dTrackTime = fTime - g_dCamTrackCaptureStartTime;
            double dTimeElapsedSinceLastDataSave = dTrackTime - g_dPrevCamTrackCapturingTime;
            const double TrackDataSavingInterval = 1.0;
            if( dTimeElapsedSinceLastDataSave > TrackDataSavingInterval )
            {
                SCameraTrackElem NewTrackElem;
                D3DXMatrixInverse( &NewTrackElem.m_CamWorldMatrix, NULL, &g_CameraViewMatrix );
                NewTrackElem.m_dTime = dTrackTime;
                fwrite(&NewTrackElem, sizeof(NewTrackElem), 1, g_pCameraTrackFile);
                g_dPrevCamTrackCapturingTime = dTrackTime;
            }
        }
    }
    else
    {
        if( g_dCamTrackReproductionStartTime < 0 )
        {
            g_iCurrTrackPos = 0;
            g_dCamTrackReproductionStartTime = fTime - (g_CameraTrack[1].m_dTime - g_CameraTrack[0].m_dTime) * 1E-5;
            g_dPrevPerfReportTime = 0;
            g_dNextPerfReportTime = g_dPerformanceReportInterval;
            g_iFramesRendered = 0;
        }

        double dTrackTime = fTime - g_dCamTrackReproductionStartTime;
        while ( g_iCurrTrackPos < g_CameraTrack.size()-1 &&
                !(dTrackTime >= g_CameraTrack[g_iCurrTrackPos].m_dTime && 
                  dTrackTime <= g_CameraTrack[g_iCurrTrackPos+1].m_dTime) )
        {
            g_iCurrTrackPos++;
        }

        D3DXMATRIX InterpolatedCamWorldMatrix;
        if( g_iCurrTrackPos < g_CameraTrack.size()-1 )
        {
            float fInterpolationWeight = (float)((dTrackTime - g_CameraTrack[g_iCurrTrackPos].m_dTime) / 
                                                 ( g_CameraTrack[g_iCurrTrackPos+1].m_dTime - g_CameraTrack[g_iCurrTrackPos].m_dTime ) );
            // Interpolate camera world matrix rows
            // Note that it is not accurate to interpolate camera view matrix
            if( 1 < g_iCurrTrackPos && g_iCurrTrackPos < g_CameraTrack.size()-2 )
            {
                for(int iRow = 0; iRow < 4; iRow++)
                {
                    D3DXVECTOR3* pDstCurrRow = (D3DXVECTOR3*)&((float*)InterpolatedCamWorldMatrix)[4*iRow];
                    D3DXVECTOR3* pSrcRows[4];
                    for(int iSrcRow = -1; iSrcRow<=2; iSrcRow++)
                        pSrcRows[iSrcRow+1] = (D3DXVECTOR3*)&((float*)&g_CameraTrack[g_iCurrTrackPos+iSrcRow].m_CamWorldMatrix)[4*iRow];
                    D3DXVec3CatmullRom( pDstCurrRow, pSrcRows[0], pSrcRows[1], pSrcRows[2], pSrcRows[3], fInterpolationWeight );
                }
            }
            else
            {
                for(int iRow = 0; iRow < 4; iRow++)
                {
                    D3DXVECTOR3* pDstCurrRow = (D3DXVECTOR3*)&((float*)InterpolatedCamWorldMatrix)[4*iRow];
                    D3DXVECTOR3* pSrcRows[2];
                    for(int iSrcRow = 0; iSrcRow<=1; iSrcRow++)
                        pSrcRows[iSrcRow] = (D3DXVECTOR3*)&((float*)&g_CameraTrack[g_iCurrTrackPos+iSrcRow].m_CamWorldMatrix)[4*iRow];
                    *pDstCurrRow = *pSrcRows[0] * (1 - fInterpolationWeight) + *pSrcRows[1] * fInterpolationWeight;
                }
            }
        }
        else
        {
            InterpolatedCamWorldMatrix = g_CameraTrack[g_iCurrTrackPos].m_CamWorldMatrix;
    
            g_iCurrTrackPos = 0;
            g_dCamTrackReproductionStartTime = -1.0;

            if( g_pPerfDataFile )
            {
                fclose(g_pPerfDataFile);
                g_pPerfDataFile = NULL;
            }
        }

        InterpolatedCamWorldMatrix._14 = 0.f;
        InterpolatedCamWorldMatrix._24 = 0.f;
        InterpolatedCamWorldMatrix._34 = 0.f;
        InterpolatedCamWorldMatrix._44 = 1.f;

        // Orthogonalize camera world matrix
        // NOTE! It is not accurate to get camera view matrix first and then orthogonalize it!
        D3DXVECTOR3 &CamWorldXAxis = *(D3DXVECTOR3*)&InterpolatedCamWorldMatrix._11;
        D3DXVECTOR3 &CamWorldYAxis = *(D3DXVECTOR3*)&InterpolatedCamWorldMatrix._21;
        D3DXVECTOR3 &CamWorldZAxis = *(D3DXVECTOR3*)&InterpolatedCamWorldMatrix._31;
        
        D3DXVECTOR3 temp;
        D3DXVec3Normalize(&CamWorldZAxis, &CamWorldZAxis);
	    D3DXVec3Cross(&temp, &CamWorldYAxis, &CamWorldZAxis);
	    D3DXVec3Normalize(&CamWorldXAxis, &temp);
	    D3DXVec3Cross(&temp, &CamWorldZAxis, &CamWorldXAxis);
        D3DXVec3Normalize(&CamWorldYAxis, &temp);

        // Get camera view matrix
        D3DXMatrixInverse( &g_CameraViewMatrix, NULL, &InterpolatedCamWorldMatrix);

        g_CameraPos = *( D3DXVECTOR3* )&InterpolatedCamWorldMatrix._41;
        g_CameraLookAt = g_CameraPos + *( D3DXVECTOR3* )&InterpolatedCamWorldMatrix._31;
        g_Camera.SetViewParams( &g_CameraPos, &g_CameraLookAt);

        if( g_pPerfDataFile )
        {
            if( dTrackTime >= g_dNextPerfReportTime )
            {
                g_dNextPerfReportTime += g_dPerformanceReportInterval;
                double dElapsedTime = dTrackTime - g_dPrevPerfReportTime;
                double dCurrFPS = (double)g_iFramesRendered/dElapsedTime;
                _ftprintf(g_pPerfDataFile, _T("%4.0lf "), dCurrFPS);
                g_dPrevPerfReportTime = dTrackTime;
                g_iFramesRendered = 0;
            }
        }

    }
    g_LightCamera.FrameMove( fElapsedTime );
    
    if( g_SampleUI.GetCheckBox(IDC_UPDATE_MODEL_CHK)->GetChecked() )
    {
        g_TerrainDX11Render.UpdateModel( g_CameraPos, g_CameraViewMatrix );
    }
}

void CALLBACK MouseProc( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, bool bSideButton1Down,
                         bool bSideButton2Down, int nMouseWheelDelta, int xPos, int yPos, void* pUserContext )
{
    g_bRightMouseDown = bRightButtonDown;
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    if( g_DisplayGUIMode == DGM_FULL_INFO )
    {
        *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
        if( *pbNoFurtherProcessing )
            return 0;

        *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
        if( *pbNoFurtherProcessing )
            return 0;
    }


    if( WM_KEYDOWN != uMsg || g_bRightMouseDown )
        g_LightCamera.HandleMessages( hWnd, uMsg, wParam, lParam );

    if( *pbNoFurtherProcessing )
        return 0;
	
    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
			case VK_F1: g_bShowHelp = !g_bShowHelp; break;
            case VK_F5: 
                g_DisplayGUIMode = static_cast<DISPLAY_GUI_MODE>( static_cast<int>(g_DisplayGUIMode)+1 ); 
                if(g_DisplayGUIMode > DGM_FULL_INFO)
                    g_DisplayGUIMode = DGM_NOTHING; 
                break;
            case VK_RETURN:
            {
                if(bAltDown)
                    return;//Alt+Enter pressed

                // Start track capture
                if(g_strCameraTrackPath[0] != 0)
                {
                    if( !g_bReproducingCameraTrack )
                    {
                        if( g_pCameraTrackFile == NULL )
                        {
                            if( _wfopen_s(&g_pCameraTrackFile, g_strCameraTrackPath, L"wb") == 0 )
                            {
                                g_dCamTrackCaptureStartTime = -1.0;
                                g_dPrevCamTrackCapturingTime = -1000.0;
                                g_SampleUI.GetButton(IDC_START_STOP_AUTOPILOT_BTN)->SetEnabled( false );
                            }
                            else
                            {
                                LOG_ERROR(L"Failed to open the \"%s\" track file for writing!\n"
                                          L"Ensure that file is not marked as read only!\n", g_strCameraTrackPath);
                            }
                        }
                        else
                        {
                            fclose(g_pCameraTrackFile);
                            g_pCameraTrackFile = NULL;
                            g_SampleUI.GetButton(IDC_START_STOP_AUTOPILOT_BTN)->SetEnabled( true );
                        }
                    }                       
                }
            }
        }
    }
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    HRESULT hr;
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen();
            break;
        case IDC_TOGGLEREF:
            DXUTToggleREF();
            break;
        case IDC_CHANGEDEVICE:
            g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() );
            break;

        case IDC_SORT_PATCHES_BY_DIST_CHK:
            g_TerrainDX11Render.EnablePatchSorting(g_SampleUI.GetCheckBox(IDC_SORT_PATCHES_BY_DIST_CHK)->GetChecked());
            break;

        case IDC_SCR_SPACE_THRESHOLD_SLIDER:
            g_TerrainRenderParams.m_fScrSpaceErrorBound = (float)g_SampleUI.GetSlider(IDC_SCR_SPACE_THRESHOLD_SLIDER)->GetValue();
            g_TerrainDX11Render.SetScreenSpaceErrorBound( g_TerrainRenderParams.m_fScrSpaceErrorBound );
            break;

        case IDC_HEIGHT_MAP_MORPH_CHK:
        {
            g_DX11PatchRenderParams.m_bEnableHeightMapMorph = g_SampleUI.GetCheckBox( IDC_HEIGHT_MAP_MORPH_CHK )->GetChecked();
            g_TerrainDX11Render.EnableHeightMapMorph( g_DX11PatchRenderParams.m_bEnableHeightMapMorph );
            break;
        }

        case IDC_NORMAL_MAP_MORPH_CHK:
        {
            g_DX11PatchRenderParams.m_bEnableNormalMapMorph = g_SampleUI.GetCheckBox( IDC_NORMAL_MAP_MORPH_CHK )->GetChecked();
            g_TerrainDX11Render.EnableNormalMapMorph( g_DX11PatchRenderParams.m_bEnableNormalMapMorph );
            break;
        }

        case IDC_CONFIG_COMBO:
        {
            g_TerrainDX11Render.OnD3D11DestroyDevice();

            LoadScene();
            
            ID3D11Device *pDevice = DXUTGetD3D11Device();
            ID3D11DeviceContext *pDeviceContext = DXUTGetD3D11DeviceContext();
			
            InitTerrainRender();

            g_TerrainDX11Render.OnD3D11CreateDevice(pDevice, pDeviceContext );
            g_TerrainDX11Render.OnD3D11ResizedSwapChain(pDevice, DXUTGetDXGISwapChain(), DXUTGetDXGIBackBufferSurfaceDesc(), NULL);
            g_TerrainDX11Render.SetViewFrustumParams( DXUTGetDXGIBackBufferSurfaceDesc()->Width, DXUTGetDXGIBackBufferSurfaceDesc()->Height, *g_Camera.GetProjMatrix() );

            break;
        }

        case IDC_COMPRESS_NORMAL_MAP_CHK:
        {
            // It is important to finish all tasks before creating new factory
            g_TerrainDX11Render.WaitForAsyncTasks();
            g_TerrainDX11Render.RestartAdaptiveModel();

            g_TerrainDX11Render.OnD3D11DestroyDevice();
            g_DX11PatchRenderParams.m_bCompressNormalMap = g_SampleUI.GetCheckBox( IDC_COMPRESS_NORMAL_MAP_CHK )->GetChecked();

            V( g_TerrainDX11Render.Init(g_TerrainRenderParams, 
                                        g_DX11PatchRenderParams,
                                        g_pElevDataSource.get(), 
                                        g_pTriangDataSource.get()) );
            g_TerrainDX11Render.OnD3D11CreateDevice( DXUTGetD3D11Device(), DXUTGetD3D11DeviceContext() );

            break;
        }

        case IDC_NORMAL_MAP_LOD_BIAS_COMBO:
        {
            // It is important to finish all tasks before creating new factory
            g_TerrainDX11Render.WaitForAsyncTasks();
            g_TerrainDX11Render.RestartAdaptiveModel();

            g_TerrainDX11Render.OnD3D11DestroyDevice();
            g_DX11PatchRenderParams.m_iNormalMapLODBias = (int)g_SampleUI.GetComboBox( IDC_NORMAL_MAP_LOD_BIAS_COMBO )->GetSelectedData();

            V( g_TerrainDX11Render.Init(g_TerrainRenderParams, 
                                        g_DX11PatchRenderParams,
                                        g_pElevDataSource.get(), 
                                        g_pTriangDataSource.get()) );
            g_TerrainDX11Render.OnD3D11CreateDevice( DXUTGetD3D11Device(), DXUTGetD3D11DeviceContext() );

            break;
        }
        case IDC_START_STOP_AUTOPILOT_BTN:
        {
            if( !g_bReproducingCameraTrack )
            {
                // Read the track from the file
                FILE *TrajectoryFile = NULL;
                if( _wfopen_s(&TrajectoryFile, g_strCameraTrackPath, L"rb") == 0)
                {
                    int FileSize = _filelength(_fileno(TrajectoryFile));
                    int iNumPositionsInTrack = FileSize / ( sizeof(SCameraTrackElem) );
                    g_CameraTrack.resize( iNumPositionsInTrack );
                    if( iNumPositionsInTrack )
                    {
                        fread(&g_CameraTrack[0], FileSize, 1, TrajectoryFile);
                    }
                    fclose( TrajectoryFile );
                    g_dCamTrackReproductionStartTime = -1;
                    g_iCurrTrackPos = 0;
                    g_bReproducingCameraTrack = true;
                    if( g_bReportPerformance )
                    {
                        _wfopen_s(&g_pPerfDataFile, g_strPerfDataPath, L"at");
                        SYSTEMTIME LocalTime;
                        GetLocalTime(&LocalTime);
                        
                        _ftprintf(g_pPerfDataFile, _T("\n\n%u.%u.%u %u:%u:%u\n"), LocalTime.wDay, LocalTime.wMonth, LocalTime.wYear, LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond);
                        _ftprintf(g_pPerfDataFile, _T("Screen resloution: %dx%d\n"), DXUTGetDXGIBackBufferSurfaceDesc()->Width, DXUTGetDXGIBackBufferSurfaceDesc()->Height );
                        _ftprintf(g_pPerfDataFile, _T("Screen space threshold: %.2lf\n"), g_TerrainRenderParams.m_fScrSpaceErrorBound);
                    }
                }
            }
            else
            {
                g_CameraTrack.clear();
                g_bReproducingCameraTrack = false;
                if( g_pPerfDataFile )
                {
                    fclose(g_pPerfDataFile);
                    g_pPerfDataFile = NULL;
                }
            }
            g_SampleUI.GetButton( IDC_START_STOP_AUTOPILOT_BTN )->SetText( g_bReproducingCameraTrack ? L"Stop Autopilot (F6)"  : L"Start Autopilot (F6)" );
            break;
        }

        case IDC_ASYNC_CHK:
        {
            g_TerrainRenderParams.m_bAsyncExecution = g_SampleUI.GetCheckBox( IDC_ASYNC_CHK )->GetChecked();
            g_TerrainDX11Render.EnableAsyncExecution( g_TerrainRenderParams.m_bAsyncExecution );
            break;
        }
    }
}
