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
#pragma once

#define MAX_PATH_LENGTH 1024

extern WCHAR g_strRawDEMDataFile[];
extern WCHAR g_strEncodedRQTTriangFile[];

extern WCHAR g_strCameraTrackPath[];
extern int g_iNumColumns;
extern int g_iNumRows;
extern int g_iPatchSize;
extern float g_fElevationSamplingInterval;
extern float g_fElevationScale;

extern bool g_bForceRecreateTriang;
extern struct SRenderingParams g_TerrainRenderParams;
extern struct CAdaptiveModelDX11Render::SRenderParams g_DX11PatchRenderParams;

// Parses the configuration file
HRESULT ParseConfigurationFile( LPCWSTR ConfigFilePath );