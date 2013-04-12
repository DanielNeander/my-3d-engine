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

#include "AdaptiveModelDX11Render.h"
#include "ConfigFile.h"

#define MAX_PATH_LENGTH 1024

// Parses the string as bool value
HRESULT ParseParameterBool( LPWSTR Value, bool &BoolValue )
{
    if( wcscmp(L"true", Value) == 0 )
        BoolValue = true;
    else if( wcscmp(L"false", Value) == 0 )
        BoolValue = false;
    else
    {
        CHECK_HR_RET( E_FAIL, L"\"%s\" is not correct boolean value. Only \"true\" and \"false\" are allowed", Value);
    }
    return S_OK;
}

// Parses the string as float value
float ParseParameterFloat( LPWSTR Value )
{
    float FloatVal;
    _stscanf_s( Value, L"%f", &FloatVal );
    return FloatVal;
}

// Parses the string as int value
int ParseParameterInt( LPWSTR Value )
{
    int IntVal;
    _stscanf_s( Value, L"%d", &IntVal);
    return IntVal;
}

// Parses the string as string value
void ParseParameterString( LPWSTR StringValue, int MaxLen, FILE *pFile )
{
    // Read space
    fgetc(pFile);
    fgetws(StringValue, MaxLen, pFile);
    size_t len = wcslen(StringValue);
    // Remove \n from the end of the line 
    if( StringValue[len-1] == L'\n' ) 
        StringValue[len-1] = L'\0';
}

// Parses the configuration file
HRESULT ParseConfigurationFile( LPCWSTR ConfigFilePath )
{
	g_DX11PatchRenderParams.m_bAsyncModeWorkaround = true;

    FILE *pConfigFile = NULL;
    if( _tfopen_s( &pConfigFile, ConfigFilePath, _T("r") ) != 0 )
    {
        CHECK_HR_RET(E_FAIL, L"Failed to open the configuration file (%s)", ConfigFilePath);
    }
    
    while( !feof(pConfigFile) )
    {
        TCHAR Parameter[128];
        TCHAR EqualSign[128];

        _ftscanf_s( pConfigFile, _T("%s"), Parameter, _countof(Parameter));
        _ftscanf_s( pConfigFile, _T("%s"), EqualSign, _countof(EqualSign));
        if( wcscmp(EqualSign, L"=") != 0 )
        {
            LOG_ERROR( L"Equal sign (=) is missing for parameter \"%s\"", Parameter);
            goto ERROR_EXIT;
        }

        // Directories
        if( wcscmp(L"RawDEMDataFile", Parameter) == 0 )
        {
            ParseParameterString(g_strRawDEMDataFile, MAX_PATH_LENGTH, pConfigFile);
        }
        else if( wcscmp(L"EncodedRQTTriangFile", Parameter) == 0 )
        {
            ParseParameterString(g_strEncodedRQTTriangFile, MAX_PATH_LENGTH, pConfigFile);
        }
		else if( wcscmp(L"CameraTrack", Parameter) == 0 )
        {
            ParseParameterString(g_strCameraTrackPath, MAX_PATH_LENGTH, pConfigFile);
        }
        else if( wcscmp(L"TexturingMode", Parameter) == 0 )
        {
            TCHAR Value[128];
            ParseParameterString(Value, _countof(Value), pConfigFile);
            if( wcscmp(L"HeightBased", Value) == 0 )
                g_DX11PatchRenderParams.m_TexturingMode = CAdaptiveModelDX11Render::TM_HEIGHT_BASED;
            else 
                LOG_ERROR( L"Unknown texturing mode (%s)\n"
                           L"Only the following modes are recognized:\n"
                           L"HeightBased\n", Value);
        }
        else
        {
            TCHAR Value[128];
            _ftscanf_s( pConfigFile, _T("%s"), Value, _countof(Value));

            // Parameters
            if( wcscmp(L"ForceRecreateTriang", Parameter) == 0 )
            {
                if( FAILED(ParseParameterBool( Value, g_bForceRecreateTriang ) ) )
                {
                    LOG_ERROR( L"Failed to parse value of the parameter \"%s\"", Parameter);
                    goto ERROR_EXIT;
                }
            }
            else if( wcscmp(L"ElevationSamplingInterval", Parameter) == 0 )
            {
                g_fElevationSamplingInterval = ParseParameterFloat( Value );
            }
            else if( wcscmp(L"NumColumns", Parameter) == 0 )
            {
                g_iNumColumns = ParseParameterInt( Value );
            }
            else if( wcscmp(L"NumRows", Parameter) == 0 )
            {
                g_iNumRows = ParseParameterInt( Value );
            }
            else if( wcscmp(L"PatchSize", Parameter) == 0 )
            {
                g_iPatchSize = ParseParameterInt( Value );
            }
            else if( wcscmp(L"ScreenSpaceThreshold", Parameter) == 0 )
            {
                g_TerrainRenderParams.m_fScrSpaceErrorBound = ParseParameterFloat( Value );
            }
            else if( wcscmp(L"AsyncModeWorkaround", Parameter) == 0 )
            {
                if( FAILED(ParseParameterBool( Value, g_DX11PatchRenderParams.m_bAsyncModeWorkaround) ) )
                {
                    LOG_ERROR( L"Failed to parse value of the parameter \"%s\"", Parameter);
                    goto ERROR_EXIT;
                }
            }
        }
    }

    fclose(pConfigFile);

    return S_OK;

ERROR_EXIT:

    fclose(pConfigFile);

    return E_FAIL;
}
