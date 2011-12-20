///////////////////////////////////////////////////////////////////////  
//  Wind.h
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

#pragma once
#include "Core/ExportBegin.h"
#include "Core/Types.h"
#include "Core/Random.h"


///////////////////////////////////////////////////////////////////////  
//  Packing

#if defined(_WIN32) || defined(_XBOX)
    #pragma warning(push)
    #pragma warning(disable : 4103)
    #pragma pack(push, 8)   
#endif


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    ///////////////////////////////////////////////////////////////////////  
    // class CWind

    class ST_STORAGE_CLASS CWind
    {
    public:
            enum EShaderValues
            {
                // float3 g_vWindDir
                SH_WIND_DIR_X,
                SH_WIND_DIR_Y,
                SH_WIND_DIR_Z,
                // float4 g_vWindTimes
                SH_TIME_PRIMARY,
                SH_TIME_SECONDARY,
                SH_TIME_FRONDS,
                SH_TIME_LEAVES,
                // float4 g_vWindDistances
                SH_DIST_PRIMARY,
                SH_DIST_SECONDARY,
                SH_DIST_HEIGHT,
                SH_DIST_HEIGHT_EXPONENT,
                // float g_vWindLeaves
                SH_DIST_LEAVES,
                SH_LEAVES_LIGHTING_CHANGE,
                SH_LEAVES_WINDWARD_SCALAR,
                // float g_vWindFrondRipple
                SH_DIST_FROND_RIPPLE,
                SH_FROND_U_TILE,
                SH_FROND_V_TILE,
                // float3 g_vWindGust
                SH_STRENGTH_COMBINED,
                SH_BEND_DISTANCE,
                SH_DIRECTION_SCALE,
                // float3 g_vWindGustHints
                SH_HEIGHT_OFFSET,
                SH_DIRECTION_ADJUSTMENT,
                SH_GUST_UNISON,
                NUM_SHADER_VALUES
            };

            enum EComponents
            {
                PRIMARY, SECONDARY, FRONDS, LEAVES, NUM_COMPONENTS
            };

            enum EOscillationControl
            {
                DISTANCE_LOW_WIND, DISTANCE_HIGH_WIND, FREQ_LOW_WIND, FREQ_HIGH_WIND, NUM_OSCILLATION_PARAMS
            };

            struct ST_STORAGE_CLASS SParams
            {
                                        SParams( );

                st_float32              m_fStrengthResponse;
                st_float32              m_fDirectionResponse;

                st_float32              m_fWindHeight;
                st_float32              m_fWindHeightExponent;
                st_float32              m_fWindHeightOffset;

                st_float32              m_afOscillationValues[NUM_COMPONENTS][NUM_OSCILLATION_PARAMS];
                
                st_float32              m_fGustFrequency;
                st_float32              m_fGustPrimaryDistance;
                st_float32              m_fGustScale;
                st_float32              m_fGustStrengthMin;
                st_float32              m_fGustStrengthMax;
                st_float32              m_fGustDurationMin;
                st_float32              m_fGustDurationMax;
                st_float32              m_fGustDirectionAdjustment;
                st_float32              m_fGustUnison;

                st_float32              m_fFrondUTile;
                st_float32              m_fFrondVTile;

                st_float32              m_afExponents[NUM_COMPONENTS];

                st_float32              m_fLeavesLightingChange;
                st_float32              m_fLeavesWindwardScalar;
            };

                                        CWind( );
                                        ~CWind( );

            // settings
            void                        SetParams(const SParams& sParams);  // this should be called infrequently and never when trees that use it are visible
            void                        SetStrength(st_float32 fStrength);              // use this function to set a new desired strength (it will reach that strength smoothly)
            void                        SetDirection(const st_float32 afDirection[3]);  // use this function to set a new desired direction (it will reach that direction smoothly)

            // animation
            void                        Advance(bool bEnabled, st_float32 fTime);       // called every frame to 'tick' the wind
            const st_float32*           GetShaderValues(void) const                 { return m_afShaderValues; }

            // leader
            void                        SetWindLeader(const CWind* pLeader);
            st_float32                  GetGustTargetForFollowers(st_float32 fTime) const;
            st_float32                  GetStrenghTargetForFollowers(void) const;
            void                        GetDirectionTargetForFollowers(st_float32 afDirection[3]) const;

            // visualization
            st_float32                  GetApparentStrength(void) const             { return m_fCombinedStrength; }
            const st_float32*           GetApparentDirection(void) const;
            void                        GetStrengthData(st_float32& fTarget, st_float32& fActual);
            void                        GetGustData(st_float32& fTarget, st_float32& fActual);

            // utility
            const SParams&              GetParams(void) const                       { return m_sParams; }

    protected:
            void                        Gust(float fTime);
            st_float32                  RandomFloat(st_float32 fMin, st_float32 fMax);
            st_float32                  LinearSigmoid(st_float32 fInput, st_float32 fLinearness);
            void                        Normalize(st_float32* pVector);

            SParams                     m_sParams;
            st_float32                  m_fStrength;
            st_float32                  m_afDirection[3];
            const CWind*                m_pWindLeader;
            CRandom                     m_cDice;

            st_float32                  m_fLastTime;
            st_float32                  m_fElapsedTime;

            st_float32                  m_fGust;
            st_float32                  m_fGustTarget;
            st_float32                  m_fGustRiseTarget;
            st_float32                  m_fGustFallTarget;
            st_float32                  m_fGustStart;
            st_float32                  m_fGustAtStart;
            st_float32                  m_fGustFallStart;

            st_float32                  m_fStrengthTarget;
            st_float32                  m_fStrengthChangeStartTime;
            st_float32                  m_fStrengthChangeEndTime;
            st_float32                  m_fStrengthAtStart;

            st_float32                  m_afDirectionTarget[3];
            st_float32                  m_afDirectionMidTarget[3];
            st_float32                  m_fDirectionChangeStartTime;
            st_float32                  m_fDirectionChangeEndTime;
            st_float32                  m_afDirectionAtStart[3];

            st_float32                  m_afEffectiveStrengths[NUM_COMPONENTS];
            st_float32                  m_afOscillationTimes[NUM_COMPONENTS];

            st_float32                  m_fCombinedStrength;

            st_float32                  m_afShaderValues[NUM_SHADER_VALUES];
    };

} // end namespace SpeedTree


///////////////////////////////////////////////////////////////////////  
//  Packing

#if defined(_WIN32) || defined(_XBOX)
    #pragma pack(pop)   
    #pragma warning(pop)    
#endif

