///////////////////////////////////////////////////////////////////////  
//  UserConfig.cpp
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) CONFIDENTIAL AND PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization, Inc. and
//  may not be copied, disclosed, or exploited except in accordance with 
//  the terms of that agreement.
//
//      Copyright (c) 2003-2009 IDV, Inc.
//      All rights reserved in all media.
//
//      IDV, Inc.
//      http://www.idvinc.com


///////////////////////////////////////////////////////////////////////  
//  Preprocessor

#include "stdafx.h"
#include "UserConfig.h"
#include "EngineCore/Speedtree/String.h"
#include "Utility.h"

#ifdef __CELLOS_LV2__
#include <sys/paths.h>
#endif

using namespace std;
using namespace SpeedTree;


///////////////////////////////////////////////////////////////////////  
//  Constants

const float c_fFeetPerKilometer = 3280.839895013f;


///////////////////////////////////////////////////////////////////////  
//  PrintUsage

void PrintUsage(const char* pExeName)
{
    Report("\tSpeedTree 5.0 Application (c) 2002-2009");
    Report("\nUsage:\n\n%s {<options>}\n", CFixedString(pExeName).NoPath( ).c_str( ));
    Report("where options can be (all units are same as the SRT & terrain files):\n");

    Report("\nFile options");
    Report("\t-cmdline_file <text file>     contains all command-line options in a single file");

    Report("\nTree options");
    Report("\t-tree <filename> <count> <min elev> <max elev> <min slope> <max slope>");
    Report("\t                              specify a random species; multiple -tree options allowed");
    Report("\t-stf <stf file>               specify a SpeedTree Forest file; multiple -stf options allowed");
    Report("\t-tree_area <square km>        area populated by trees in km, centered at (0, 0)");
    Report("\t-random_scale <min> <max>     range of random scale values for trees");
    Report("\t-lod_scale <value>            scale applied to lod distances");

    Report("\nLight options");
    Report("\t-lightdir <x> <y> <z>         direction of light (values will be normalized)");
    Report("\t-lightscale <scale>           scale the lighting range to help normalize the scene");
    Report("\t-light_ambient <r> <g> <b>    ambient light color in RGB");
    Report("\t-light_diffuse <r> <g> <b>    diffuse light color in RGB");
    Report("\t-light_specular <r> <g> <b>   specular light color in RGB");
    Report("\t-specular                     enable specular lighting effect");
    Report("\t-transmission                 enable transmission lighting effect");
    Report("\t-detail                       render detail texture layer");
    Report("\t-detail_normal                render detail normal map layer");
    Report("\t-ambient_contrast             use contrast ambient lighting effect");

    Report("\nEnvironment options");
    Report("\t-visibility <distance>        distance from camera to horizon");
    Report("\t-fog <start> <end>            linear fog start and end values");
    Report("\t-bgcolor <r> <g> <b>          color of the background/fog in RGB");
    Report("\t-sky_color <r> <g> <b>        RGB color of the sky");
    Report("\t-sky_fog <min> <max>          range of fog in the sky. -1 is down, 1 is up");
    Report("\t-stars <small> <large>        number of stars in the sky");
    Report("\t-sun_color <r> <g> <b>        RGB color of the sun");
    Report("\t-sun_parameters <size> <spread> <fog bloom>");
    Report("\t                              the size, spread, and fog bloom effect of the sun");

    Report("\nWindow options");
    Report("\t-res <width> <height>         window resolution");
    Report("\t-windowed                     makes the application run in a window");
    Report("\t-fullscreen                   makes the application run in fullscreen mode");
    Report("\t-samples <count>              multisampling configuration");
    Report("\t-anisotropy <value>           set the max texture anisotropy");
    Report("\t-gamma <value>                sets the screen gamma");
    Report("\t-near <distance>              distance of near clipping plane");
    Report("\t-fov <radians>                field of view (defaults to 40 degrees)");

    Report("\nTerrain options");
    Report("\t-forest_cell_size <value>     size of cells/regions for culling");
    Report("\t-terrain <filename>           specify terrain setup file");
    Report("\t-terrain_res <value>          resolution of terrain (must be (power of 2) + 1)");
    Report("\t-terrain_light_scale <value>  light scalar when rendering the terrain ");
    Report("\t-follow_height <value>        height of the observer when walking on the terrain ");
    Report("\t-terrain_ao <amt> <dist>      control the amount (0-1) of AO on the terrain from the trees");
    Report("\t                              and the distance it spreads");

    Report("\nWind options");
    Report("\t-frond_ripple                 enable frond rippling");
    Report("\t-nowind                       disable wind effects");

    Report("\nShadow options");
    Report("\t-num_shadow_maps <value>      sets the # of cascaded shadow maps");
    Report("\t-shadowres <value>            set the shadow map resolution (power of 2)");
    Report("\t-smooth_shadows               enable shadow blurring");
    Report("\t-shadow_scalar <value>        split scalar for the shadow mapping algorithm");
    Report("\t-shadow_splits <0>,...N       split locations for the shadow (in world distance units)");
    Report("\t-shadow_fade <value>          where along the shadow to fade out (0.9 = 90%)");
    Report("\t-show_splits                  enable shadow split drawing");

    Report("\nGrass options");
    Report("\t-grass_texture <filename> <col> <row>");
    Report("\t                              grass composite texture and the setup of textures in it");
    Report("\t-grass_density <value>        number of grass blades in a cell");
    Report("\t-grass_lighting <scale> <variance>");
    Report("\t                              grass lighting parameters");
    Report("\t-grass_lod <near> <far>       grass lod range");
    Report("\t-grass_alpha_test             enable grass alpha testing");
    Report("\t-grass_terrain_match <top> <bot>  how much the grass blades mirror the terrain color (0-1)");
    Report("\t-grass_placement_default <freq> <min elev> <max elev> <min slope> <max slope>");
    Report("\t                              <min width> <max width> <min height> <max height>");
    Report("\t-grass_placement <freq> <min elev> <max elev> <min slope> <max slope>");
    Report("\t                              <min width> <max width> <min height> <max height>");
    Report("\t-grass_wind <str resp> <dir resp> <height offset> <prim low dist> <prim high dist>");
    Report("\t                              <prim low freq> <prim high freq> <sec low dist>");
    Report("\t                              <sec high dist> <sec low freq> <sec high freq>");
    Report("\t                              <gust freq> <gust dist> <gust str min> <gust str max>");
    Report("\t                              <gust dur min> <gust dur max> <prim exp> <sec exp>");

    Report("\nRendering options");
    Report("\t-alpha_test_scalar <value>    adjusts alpha testing to look more like a2c (default: 0.57)");
    Report("\t-z_prepass                    renders a depth-only prepass; helps under certain settings and hardware");

    Report("\nCell size options (advanced)");
    Report("\t-grass_cell_size <value>      size of grass cell (default: 50.0)");
    Report("\t-terrain_cell_size <value>    size of terrain cell (default: 800.0)");
    Report("\t-forest_cell_size <value>     size of tree cell (default: 1200.0)");

    Report("\nExtra options");
    Report("\t-verbose                      print detailed information load forest load");
    Report("\t-shader <fx file>             location of SpeedTree.fx file");
    Report("\t-camera_file <filename>       filename of the camera save file");
}


///////////////////////////////////////////////////////////////////////  
//  SUserSettings::SUserSettings

SUserSettings::SUserSettings( ) :
    // paths
#ifdef __CELLOS_LV2__
    m_strShaderLocation(SYS_APP_HOME"/Shaders/SpeedTree.fx"),
    m_strTerrainPath(SYS_APP_HOME"/Terrain"),
#elif _XBOX
    m_strShaderLocation("game:\\Shaders\\SpeedTree.fx"),
#else // Windows
    m_strShaderLocation("Data/Shaders/SpeedTree.fx"),
    m_strTerrainPath("Data/Terrain"),
    m_strCameraSavePath("SavedCameras.txt"),
#endif
    m_vLightDir(CCoordSys::ConvertFromStd(0.0f, -0.707f, -0.707f)),
    // lighting
    m_fGlobalLightScalar(1.0f),
    m_bSpecularLighting(false),
    m_bTransmissionLighting(false),
    m_bDetailLayer(false),
    m_bDetailNormalMapping(false),
    m_bAmbientContrast(false),
    // sun/sky
    m_vSkyColor(noVec3(0.2f, 0.3f, 0.5f)),
    m_vSunColor(noVec3(1.0f, 1.0f, 0.85f)),
    m_vSunParameters(noVec3(0.001f, 200.0f, 0.0f)),
    // fog
    m_fFogStart(1500.0f),
    m_fFogEnd(5000.0f),
    m_vFogColor(209.0f / 255.0f, 235.0f / 255.0f, 255.0f / 255.0f),
    // size
    m_fTreeAreaWidthInFeet(40000.0f),
    m_fVisibility(5000.0f),
    m_fLodScale(1.0f),
    // display
    m_nWindowWidth(1280),
    m_nWindowHeight(720),
    m_bFullscreen(false),
    m_bFullscreenResOverride(false),
    m_fFieldOfView(40.0f),
    m_fNearClip(0.5f),
    m_nSampleCount(0),
    m_nMaxAnisotropy(0),
    m_nShadowRes(0),
    m_fGamma(1.0f),
    // wind
    m_bWindOff(false),
    m_bFrondRipple(false),
    // terrain
    m_fTerrainLightScalar(1.2f),
    m_nTerrainRes(33),
    m_fTerrainFollowHeight(6.0f),
    m_fTerrainAmbientOcclusion(0.0f),
    m_fTerrainTreeAODistance(25.0f),
    // shadows
    m_nNumShadowMaps(3),
    m_fShadowSplitScalar(1.0f),
    m_fShadowFadePercent(0.85f),
    m_bShowSplitsOnTerrain(false),
    m_bSmoothShadows(false),
    m_nShadowUpdateFreq(3),
    // grass
    m_strGrassTexture("../../Terrain/grass_blades.dds"),
    m_nNumGrassTextureCols(2),
    m_nNumGrassTextureRows(2),
    m_nGrassDensity(100),
    m_fGrassLightScalar(1.0f),
    m_fGrassNormalVariance(0.4f),
    m_fGrassStartFade(250.0f),
    m_fGrassEndFade(400.0f),
    m_bGrassAlphaTesting(false),
    // clouds
#ifdef SPEEDTREE_SIMUL_CLOUDS
    m_bUseClouds(false),
    m_strCloudPath(""),
#endif
    // rendering
    m_fAlphaTestScalar(0.57f),
    m_bZPrePass(false),
    // cell sizes
    m_fGrassCellSize(50.0f),
    m_fTerrainCellSize(800.0f),
    m_fForestCellSize(1200.0f),
    // other
    m_bHorzBillboards(false),
    m_bVerbosePrint(false),
    m_bDiagnostic(false),
    m_fTreeCountScalar(1.0f)
{
    m_sLightMaterial.m_vAmbient.Set(1.0f, 1.0f, 1.0f, 1.0f);
    m_sLightMaterial.m_vDiffuse.Set(1.0f, 1.0f, 1.0f, 1.0f);
    m_sLightMaterial.m_vSpecular.Set(1.0f, 1.0f, 1.0f, 1.0f);
    m_sLightMaterial.m_vEmissive.Set(0.0f, 0.0f, 0.0f, 1.0f);

    m_afRandomScaleRange[0] = 0.7f;
    m_afRandomScaleRange[1] = 1.5f;
    m_anNumStars[0] = 0;
    m_anNumStars[1] = 0;
    m_afSkyFogMinMax[0] = -0.5f;
    m_afSkyFogMinMax[1] = 1.0f;

    m_afShadowSplits[3] = 0.25f * m_fVisibility;
    m_afShadowSplits[2] = 0.5f * m_afShadowSplits[3];
    m_afShadowSplits[1] = 0.5f * m_afShadowSplits[2];
    m_afShadowSplits[0] = 0.5f * m_afShadowSplits[1];

    // setup default grass wind parameters
    // -grass_wind 6 1 0.05      0.5 1.0 1 8     0.2 0.1 5 14     0.4 2   0.95 1 0.1 1.5     2 1 
    m_sGrassWind.m_fStrengthResponse = 6.0f;
    m_sGrassWind.m_fDirectionResponse = 1.0f;
    m_sGrassWind.m_fWindHeightOffset = 0.05f;
    m_sGrassWind.m_afOscillationValues[CWind::PRIMARY][CWind::DISTANCE_LOW_WIND] = 0.5f;
    m_sGrassWind.m_afOscillationValues[CWind::PRIMARY][CWind::DISTANCE_HIGH_WIND] = 1.0f;
    m_sGrassWind.m_afOscillationValues[CWind::PRIMARY][CWind::FREQ_LOW_WIND] = 1.0f;
    m_sGrassWind.m_afOscillationValues[CWind::PRIMARY][CWind::FREQ_HIGH_WIND] = 8.0;
    m_sGrassWind.m_afOscillationValues[CWind::SECONDARY][CWind::DISTANCE_LOW_WIND] = 0.2f;
    m_sGrassWind.m_afOscillationValues[CWind::SECONDARY][CWind::DISTANCE_HIGH_WIND] = 0.1f;
    m_sGrassWind.m_afOscillationValues[CWind::SECONDARY][CWind::FREQ_LOW_WIND] = 5.0f;
    m_sGrassWind.m_afOscillationValues[CWind::SECONDARY][CWind::FREQ_HIGH_WIND] = 14.0f;
    m_sGrassWind.m_fGustFrequency = 0.4f;
    m_sGrassWind.m_fGustPrimaryDistance = 2.0f;
    m_sGrassWind.m_fGustStrengthMin = 0.95f;
    m_sGrassWind.m_fGustStrengthMax = 1.0f;
    m_sGrassWind.m_fGustDurationMin = 0.1f;
    m_sGrassWind.m_fGustDurationMax = 1.5f;
    m_sGrassWind.m_afExponents[CWind::PRIMARY] = 2.0f;
    m_sGrassWind.m_afExponents[CWind::SECONDARY] = 1.0f;

    m_afGrassTerrainMatchFactors[0] = 0.25f;
    m_afGrassTerrainMatchFactors[1] = 1.0f;
}


///////////////////////////////////////////////////////////////////////  
//  CCommandLineParser::Parse

st_bool CCommandLineParser::Parse(st_int32 argc, char* argv[], SUserSettings& sConfig)
{
    st_bool bSuccess = false;

    // extract exe path
    sConfig.m_strExePath = CFixedString(argv[0]).Path( );

    // scan the options
    SGrassPlacement sDefaultPlacement;
    st_bool bNeedsPrintUsage = false;
    for (st_int32 i = 1; i < argc; ++i)
    {
        CFixedString strCommand = argv[i];
        if (strCommand == "-cmdline_file")
        {
            if (i + 1 < argc)
            {
                const char* pCmdLineFile = argv[++i];
                if (ParseCommandLineFile(pCmdLineFile, argv[0], sConfig))
                    break;
                else
                    Warning("Failed to scan command-line file [%s]\n", pCmdLineFile);
            }
        }
        else if (strCommand == "-tree")
        {
            if (i + 8 < argc)
            {
                SSpeciesPlacement sSpecies;
                sSpecies.m_strFilename = CFixedString(argv[++i]);
                sSpecies.m_nQuantity = atoi(argv[++i]);
                sSpecies.m_afElevationRange[0] = st_float32(atof(argv[++i]));
                sSpecies.m_afElevationRange[1] = st_float32(atof(argv[++i]));
                sSpecies.m_afSlopeRange[0] = st_float32(atof(argv[++i]));
                sSpecies.m_afSlopeRange[1] = st_float32(atof(argv[++i]));
                if (sSpecies.m_afSlopeRange[0] < 0.0f || sSpecies.m_afSlopeRange[0] > 1.0f)
                    Warning("min slope value for [%s] out of [0,1] range; clamping", sSpecies.m_strFilename.c_str( ));
                if (sSpecies.m_afSlopeRange[1] < 0.0f || sSpecies.m_afSlopeRange[1] > 1.0f)
                    Warning("max slope value for [%s] out of [0,1] range; clamping", sSpecies.m_strFilename.c_str( ));
                sSpecies.m_afScaleRange[0] = st_float32(atof(argv[++i]));
                sSpecies.m_afScaleRange[1] = st_float32(atof(argv[++i]));
                sConfig.m_aSpecies.push_back(sSpecies);
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-stf")
        {
            if (i + 1 < argc)
            {
                sConfig.m_aStfFiles.push_back(argv[++i]);
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-tree_area")
        {
            if (i + 1 < argc)
            {
                float fArea = st_float32(atof(argv[++i]));
                sConfig.m_fTreeAreaWidthInFeet = sqrt(fArea) * c_fFeetPerKilometer;
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-random_scale")
        {
            if (i + 2 < argc)
            {
                sConfig.m_afRandomScaleRange[0] = st_float32(atof(argv[++i]));
                sConfig.m_afRandomScaleRange[1] = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-fog")
        {
            if (i + 2 < argc)
            {
                sConfig.m_fFogStart = st_float32(atof(argv[++i]));
                sConfig.m_fFogEnd = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-bgcolor")
        {
            if (i + 3 < argc)
            {
                sConfig.m_vFogColor.x = st_float32(atof(argv[++i]));
                sConfig.m_vFogColor.y = st_float32(atof(argv[++i]));
                sConfig.m_vFogColor.z = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-light_ambient")
        {
            if (i + 3 < argc)
            {
                sConfig.m_sLightMaterial.m_vAmbient.x = st_float32(atof(argv[++i]));
                sConfig.m_sLightMaterial.m_vAmbient.y = st_float32(atof(argv[++i]));
                sConfig.m_sLightMaterial.m_vAmbient.z = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-light_diffuse")
        {
            if (i + 3 < argc)
            {
                sConfig.m_sLightMaterial.m_vDiffuse.x = st_float32(atof(argv[++i]));
                sConfig.m_sLightMaterial.m_vDiffuse.y = st_float32(atof(argv[++i]));
                sConfig.m_sLightMaterial.m_vDiffuse.z = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-light_specular")
        {
            if (i + 3 < argc)
            {
                sConfig.m_sLightMaterial.m_vSpecular.x = st_float32(atof(argv[++i]));
                sConfig.m_sLightMaterial.m_vSpecular.y = st_float32(atof(argv[++i]));
                sConfig.m_sLightMaterial.m_vSpecular.z = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-lightdir")
        {
            if (i + 3 < argc)
            {
                sConfig.m_vLightDir.x = st_float32(atof(argv[++i]));
                sConfig.m_vLightDir.y = st_float32(atof(argv[++i]));
                sConfig.m_vLightDir.z = st_float32(atof(argv[++i]));
                sConfig.m_vLightDir.Normalize( );
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-lightscale")
        {
            if (i + 1 < argc)
            {
                sConfig.m_fGlobalLightScalar = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
#ifdef SPEEDTREE_SIMUL_CLOUDS
        else if (strCommand == "-clouds")
        {
            sConfig.m_bUseClouds = true;
        }
        else if (strCommand == "-cloud_file")
        {
            if (i + 1 < argc)
                sConfig.m_strCloudPath = CString(argv[++i]);
            else
                bNeedsPrintUsage = true;
        }
#endif
        else if (strCommand == "-near")
        {
            if (i + 1 < argc)
            {
                sConfig.m_fNearClip = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-fov")
        {
            if (i + 1 < argc)
            {
                sConfig.m_fFieldOfView = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-lod_scale")
        {
            if (i + 1 < argc)
            {
                sConfig.m_fLodScale = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-frond_ripple")
        {
            sConfig.m_bFrondRipple = true;
        }
        else if (strCommand == "-specular")
        {
            sConfig.m_bSpecularLighting = true;
        }
        else if (strCommand == "-transmission")
        {
            sConfig.m_bTransmissionLighting = true;
        }
        else if (strCommand == "-detail")
        {
            sConfig.m_bDetailLayer = true;
        }
        else if (strCommand == "-detail_normal")
        {
            sConfig.m_bDetailNormalMapping = true;
        }
        else if (strCommand == "-ambient_contrast")
        {
            sConfig.m_bAmbientContrast = true;
        }
        else if (strCommand == "-res")
        {
            if (i + 2 < argc)
            {
                sConfig.m_nWindowWidth = atoi(argv[++i]);
                sConfig.m_nWindowHeight = atoi(argv[++i]);
                sConfig.m_bFullscreenResOverride = true;
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-visibility")
        {
            if (i + 1 < argc)
                sConfig.m_fVisibility = st_float32(atof(argv[++i]));
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-nowind")
        {
            sConfig.m_bWindOff = true;
        }
        else if (strCommand == "-samples")
        {
            if (i + 1 < argc)
                sConfig.m_nSampleCount = atoi(argv[++i]);
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-windowed" ||
                 strCommand == "-window")
        {
            sConfig.m_bFullscreen = false;
        }
        else if (strCommand == "-fullscreen")
        {
            sConfig.m_bFullscreen = true;
        }
        else if (strCommand == "-diagnostic")
        {
            sConfig.m_bDiagnostic = true;
        }
        else if (strCommand == "-count_scalar")
        {
            if (i + 1 < argc)
                sConfig.m_fTreeCountScalar = st_float32(atof(argv[++i]));
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-verbose")
        {
            sConfig.m_bVerbosePrint = true;
        }
        else if (strCommand == "-smooth_shadows")
        {
            sConfig.m_bSmoothShadows = true;
        }
        else if (strCommand == "-horzbbs" ||
                 strCommand == "-horz_bbs")
        {
            sConfig.m_bHorzBillboards = true;
        }
        else if (strCommand == "-shader" ||
                 strCommand == "-shaders")
        {
            if (i + 1 < argc)
                sConfig.m_strShaderLocation = CFixedString(argv[++i]);
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-terrain")
        {
            if (i + 1 < argc)
                sConfig.m_strTerrainPath = CFixedString(argv[++i]);
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-terrain_light_scale")
        {
            if (i + 1 < argc)
                sConfig.m_fTerrainLightScalar = st_float32(atof(argv[++i]));
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-follow_height")
        {
            if (i + 1 < argc)
                sConfig.m_fTerrainFollowHeight = st_float32(atof(argv[++i]));
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-grass_texture")
        {
            if (i + 3 < argc)
            {
                sConfig.m_strGrassTexture = CFixedString(argv[++i]);
                sConfig.m_nNumGrassTextureCols = atoi(argv[++i]);
                sConfig.m_nNumGrassTextureRows = atoi(argv[++i]);
                sConfig.m_nNumGrassTextureCols = st_max(sConfig.m_nNumGrassTextureCols, 1);
                sConfig.m_nNumGrassTextureRows = st_max(sConfig.m_nNumGrassTextureRows, 1);
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-grass_density")
        {
            if (i + 1 < argc)
                sConfig.m_nGrassDensity = atoi(argv[++i]);
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-grass_lighting")
        {
            if (i + 2 < argc)
            {
                sConfig.m_fGrassLightScalar = st_float32(atof(argv[++i]));
                sConfig.m_fGrassNormalVariance = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-grass_placement_default")
        {
            if (i + 9 < argc)
            {
                sDefaultPlacement.m_fFrequency = st_float32(atof(argv[++i]));
                sDefaultPlacement.m_afElevationRange[0] = st_float32(atof(argv[++i]));
                sDefaultPlacement.m_afElevationRange[1] = st_float32(atof(argv[++i]));
                sDefaultPlacement.m_afSlopeRange[0] = st_float32(atof(argv[++i]));
                sDefaultPlacement.m_afSlopeRange[1] = st_float32(atof(argv[++i]));
                sDefaultPlacement.m_afWidthRange[0] = st_float32(atof(argv[++i]));
                sDefaultPlacement.m_afWidthRange[1] = st_float32(atof(argv[++i]));
                sDefaultPlacement.m_afHeightRange[0] = st_float32(atof(argv[++i]));
                sDefaultPlacement.m_afHeightRange[1] = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-grass_placement")
        {
            if (i + 10 < argc)
            {
                SGrassPlacement sPlacement;
                sPlacement.m_nIndex = atoi(argv[++i]);
                sPlacement.m_nIndex = st_max(sPlacement.m_nIndex, 0);
                sPlacement.m_fFrequency = st_float32(atof(argv[++i]));
                sPlacement.m_afElevationRange[0] = st_float32(atof(argv[++i]));
                sPlacement.m_afElevationRange[1] = st_float32(atof(argv[++i]));
                sPlacement.m_afSlopeRange[0] = st_float32(atof(argv[++i]));
                sPlacement.m_afSlopeRange[1] = st_float32(atof(argv[++i]));
                sPlacement.m_afWidthRange[0] = st_float32(atof(argv[++i]));
                sPlacement.m_afWidthRange[1] = st_float32(atof(argv[++i]));
                sPlacement.m_afHeightRange[0] = st_float32(atof(argv[++i]));
                sPlacement.m_afHeightRange[1] = st_float32(atof(argv[++i]));

                sConfig.m_aGrassPlacements.push_back(sPlacement);
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-grass_lod")
        {
            if (i + 2 < argc)
            {
                sConfig.m_fGrassStartFade = st_float32(atof(argv[++i]));
                sConfig.m_fGrassEndFade = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-alpha_test_scalar")
        {
            if (i + 1 < argc)
                sConfig.m_fAlphaTestScalar = st_float32(atof(argv[++i]));
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-z_prepass")
        {
            sConfig.m_bZPrePass = true;
        }
        else if (strCommand == "-grass_cell_size")
        {
            if (i + 1 < argc)
                sConfig.m_fGrassCellSize = st_float32(atof(argv[++i]));
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-terrain_cell_size")
        {
            if (i + 1 < argc)
                sConfig.m_fTerrainCellSize = st_float32(atof(argv[++i]));
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-forest_cell_size")
        {
            if (i + 1 < argc)
                sConfig.m_fForestCellSize = st_float32(atof(argv[++i]));
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-terrain_res")
        {
            if (i + 1 < argc)
            {
                sConfig.m_nTerrainRes = atoi(argv[++i]);
                if (sConfig.m_nTerrainRes != 3 &&
                    sConfig.m_nTerrainRes != 5 &&
                    sConfig.m_nTerrainRes != 9 &&
                    sConfig.m_nTerrainRes != 17 &&
                    sConfig.m_nTerrainRes != 33 &&
                    sConfig.m_nTerrainRes != 65 &&
                    sConfig.m_nTerrainRes != 129 &&
                    sConfig.m_nTerrainRes != 257)
                {
                    printf("-terrain_res must be (power of 2) + 1 [e.g. 17, 33, 65, etc.]\n");
                    bNeedsPrintUsage = true;
                }
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-grass_alpha_test" ||
                 strCommand == "-grass_alpha_testing")
        {
            sConfig.m_bGrassAlphaTesting = true;
        }
        else if (strCommand == "-grass_terrain_match")
        {
            if (i + 2 < argc)
            {
                sConfig.m_afGrassTerrainMatchFactors[0] = 1.0f - Clamp(st_float32(atof(argv[++i])), 0.0f, 1.0f);
                sConfig.m_afGrassTerrainMatchFactors[1] = 1.0f - Clamp(st_float32(atof(argv[++i])), 0.0f, 1.0f);
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-terrain_ao")
        {
            if (i + 2 < argc)
            {
                sConfig.m_fTerrainAmbientOcclusion = st_float32(atof(argv[++i]));
                sConfig.m_fTerrainTreeAODistance = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-num_shadow_maps")
        {
            if (i + 1 < argc)
            {
                sConfig.m_nNumShadowMaps = atoi(argv[++i]);
                if (sConfig.m_nNumShadowMaps < 1)
                {
                    Warning("%d shadow maps specified, but must be at least 1; clamping", sConfig.m_nNumShadowMaps);
                    sConfig.m_nNumShadowMaps = 1;
                }
                else if (sConfig.m_nNumShadowMaps > c_nMaxNumShadowMaps)
                {
                    Warning("%d shadow maps specified, but cannot exceed %d; clamping", sConfig.m_nNumShadowMaps, c_nMaxNumShadowMaps);
                    sConfig.m_nNumShadowMaps = c_nMaxNumShadowMaps;
                }
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-anisotropy")
        {
            if (i + 1 < argc)
                sConfig.m_nMaxAnisotropy = atoi(argv[++i]);
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-shadowres")
        {
            if (i + 1 < argc)
                sConfig.m_nShadowRes = atoi(argv[++i]);
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-shadow_freq")
        {
            if (i + 1 < argc)
            {
                sConfig.m_nShadowUpdateFreq = atoi(argv[++i]);
                if (sConfig.m_nShadowUpdateFreq < 1 || sConfig.m_nShadowUpdateFreq > 10)
                {
                    Warning("%d shadow map frequency specified, but must in range of [1,10]; clamping", sConfig.m_nShadowUpdateFreq);
                    sConfig.m_nShadowUpdateFreq = Clamp<st_int32>(sConfig.m_nShadowUpdateFreq, 1, 10);
                }
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-sky_color")
        {
            if (i + 3 < argc)
            {
                sConfig.m_vSkyColor.x = st_float32(atof(argv[++i]));
                sConfig.m_vSkyColor.y = st_float32(atof(argv[++i]));
                sConfig.m_vSkyColor.z = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-sky_fog")
        {
            if (i + 2 < argc)
            {
                sConfig.m_afSkyFogMinMax[0] = st_float32(atof(argv[++i]));
                sConfig.m_afSkyFogMinMax[1] = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-stars")
        {
            if (i + 2 < argc)
            {
                sConfig.m_anNumStars[0] = atoi(argv[++i]);
                sConfig.m_anNumStars[1] = atoi(argv[++i]);
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-sun_color")
        {
            if (i + 3 < argc)
            {
                sConfig.m_vSunColor.x = st_float32(atof(argv[++i]));
                sConfig.m_vSunColor.y = st_float32(atof(argv[++i]));
                sConfig.m_vSunColor.z = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-sun_parameters")
        {
            if (i + 3 < argc)
            {
                sConfig.m_vSunParameters.x = st_float32(atof(argv[++i]));
                sConfig.m_vSunParameters.y = st_float32(atof(argv[++i]));
                sConfig.m_vSunParameters.z = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-camera_file")
        {
            if (i + 1 < argc)
                sConfig.m_strCameraSavePath = CFixedString(argv[++i]);
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-shadow_scalar")
        {
            if (i + 1 < argc)
                sConfig.m_fShadowSplitScalar = Clamp(st_float32(atof(argv[++i])), 0.0f, 1.0f);
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-shadow_splits")
        {
            if (i + sConfig.m_nNumShadowMaps < argc)
            {
                memset(sConfig.m_afShadowSplits, 0, sizeof(sConfig.m_afShadowSplits));
                for (int j = 0; j < sConfig.m_nNumShadowMaps; ++j)
                    sConfig.m_afShadowSplits[j] = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-shadow_fade")
        {
            if (i + 1 < argc)
                sConfig.m_fShadowFadePercent = st_float32(atof(argv[++i]));
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-show_splits")
        {
            sConfig.m_bShowSplitsOnTerrain = true;
        }
        else if (strCommand == "-gamma")
        {
            if (i + 1 < argc)
                sConfig.m_fGamma = st_float32(atof(argv[++i]));
            else
                bNeedsPrintUsage = true;
        }
        else if (strCommand == "-grass_wind")
        {   
            if (i + 19 < argc)
            {
                sConfig.m_sGrassWind.m_fStrengthResponse = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_fDirectionResponse = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_fWindHeightOffset = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_afOscillationValues[CWind::PRIMARY][CWind::DISTANCE_LOW_WIND] = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_afOscillationValues[CWind::PRIMARY][CWind::DISTANCE_HIGH_WIND] = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_afOscillationValues[CWind::PRIMARY][CWind::FREQ_LOW_WIND] = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_afOscillationValues[CWind::PRIMARY][CWind::FREQ_HIGH_WIND] = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_afOscillationValues[CWind::SECONDARY][CWind::DISTANCE_LOW_WIND] = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_afOscillationValues[CWind::SECONDARY][CWind::DISTANCE_HIGH_WIND] = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_afOscillationValues[CWind::SECONDARY][CWind::FREQ_LOW_WIND] = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_afOscillationValues[CWind::SECONDARY][CWind::FREQ_HIGH_WIND] = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_fGustFrequency = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_fGustPrimaryDistance = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_fGustStrengthMin = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_fGustStrengthMax = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_fGustDurationMin = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_fGustDurationMax = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_afExponents[CWind::PRIMARY] = st_float32(atof(argv[++i]));
                sConfig.m_sGrassWind.m_afExponents[CWind::SECONDARY] = st_float32(atof(argv[++i]));
            }
            else
                bNeedsPrintUsage = true;
        }

        else
        {
            Warning("Unknown command [%s] (cmd-line argument #%d)\n", strCommand.c_str( ), i);
            bNeedsPrintUsage = true;
        }
    }

    if (bNeedsPrintUsage)
        PrintUsage(argv[0]);
    else
        bSuccess = true;

    // reconcile grass placements versus defaults
    sConfig.m_fTotalPlacementWeight = 0.0f;
    int nTotalGrassPanes = sConfig.m_nNumGrassTextureCols * sConfig.m_nNumGrassTextureRows;
    CArray<SGrassPlacement> aPlacements(nTotalGrassPanes, sDefaultPlacement);
    for (st_int32 i = 0; i < nTotalGrassPanes; ++i)
    {
        // search through the user-specified replacements to override a default
        for (st_int32 j = 0; j < int(sConfig.m_aGrassPlacements.size( )); ++j)
        {
            if (sConfig.m_aGrassPlacements[j].m_nIndex == i)
            {
                aPlacements[i] = sConfig.m_aGrassPlacements[j];
                break;
            }
        }

        sConfig.m_fTotalPlacementWeight += aPlacements[i].m_fFrequency;
    }
    sConfig.m_aGrassPlacements = aPlacements;

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CCommandLineParser::ParseCommandLineFile

st_bool CCommandLineParser::ParseCommandLineFile(const char* pFilename, const char* pExeName, SUserSettings& sConfig)
{
    st_bool bSuccess = false;

    FILE* pFile = fopen(pFilename, "r");
    if (pFile)
    {
        // parse the cmd-line options in the file and store in an array of arguments
        CArray<CFixedString> aArguments;
        CFixedString strTemp;
        st_bool bInQuotes = false;

        while (!feof(pFile))
        {
            st_char chTemp = (st_char)fgetc(pFile);

            st_bool bSaveString = false;
            if (chTemp == '#')
            {
                // skip rest of comment line
                while (!feof(pFile) && (chTemp != '\r' && chTemp != '\n'))
                    chTemp = (st_char)fgetc(pFile);
                bSaveString = true;
            }
            else if (chTemp == '\"')
            {
                // quote delimited string
                if (bInQuotes)
                    bSaveString = true;
                bInQuotes = !bInQuotes;
            }
            else if (!bInQuotes && (chTemp == ' ' || chTemp == '\r' || chTemp == '\n'))
            {
                // other whitespace
                bSaveString = true;             
            }
            else
            {
                strTemp += chTemp;
            }

            if (bSaveString && !strTemp.empty( ))
            {
                // save this string as an argument
                aArguments.push_back(strTemp);
                strTemp.resize(0);
            }
        }

        fclose(pFile);

        // convert the array to a standard argv-type data structure
        const st_int32 c_nNumArgs = st_int32(aArguments.size( ) + 1);
        char** argv = new char*[c_nNumArgs];
        for (st_int32 i = 1; i < c_nNumArgs; ++i)
            argv[i] = (char*) aArguments[i - 1].c_str( );
        argv[0] = (char*) pExeName; // normally contains the exe name

        // feed back into the parse routine 
        bSuccess = Parse(c_nNumArgs, argv, sConfig);

        delete [] argv;
    }

    return bSuccess;
}



