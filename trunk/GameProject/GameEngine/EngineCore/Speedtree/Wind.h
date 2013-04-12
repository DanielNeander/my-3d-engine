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
#include "ExportBegin.h"
#include "EngineCore/Types.h"
#include "Random.h"


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

                float              m_fStrengthResponse;
                float              m_fDirectionResponse;

                float              m_fWindHeight;
                float              m_fWindHeightExponent;
                float              m_fWindHeightOffset;

                float              m_afOscillationValues[NUM_COMPONENTS][NUM_OSCILLATION_PARAMS];
                
                float              m_fGustFrequency;
                float              m_fGustPrimaryDistance;
                float              m_fGustScale;
                float              m_fGustStrengthMin;
                float              m_fGustStrengthMax;
                float              m_fGustDurationMin;
                float              m_fGustDurationMax;
                float              m_fGustDirectionAdjustment;
                float              m_fGustUnison;

                float              m_fFrondUTile;
                float              m_fFrondVTile;

                float              m_afExponents[NUM_COMPONENTS];

                float              m_fLeavesLightingChange;
                float              m_fLeavesWindwardScalar;
            };

                                        CWind( );
                                        ~CWind( );

            // settings
            void                        SetParams(const SParams& sParams);  // this should be called infrequently and never when trees that use it are visible
            void                        SetStrength(float fStrength);              // use this function to set a new desired strength (it will reach that strength smoothly)
            void                        SetDirection(const float afDirection[3]);  // use this function to set a new desired direction (it will reach that direction smoothly)

            // animation
			void                        Advance(bool bEnabled, float fTime) {}       // called every frame to 'tick' the wind
            const float*           GetShaderValues(void) const                 { return m_afShaderValues; }

            // leader
            void                        SetWindLeader(const CWind* pLeader);
            float                  GetGustTargetForFollowers(float fTime) const;
            float                  GetStrenghTargetForFollowers(void) const;
            void                        GetDirectionTargetForFollowers(float afDirection[3]) const;

            // visualization
            float                  GetApparentStrength(void) const             { return m_fCombinedStrength; }
            const float*           GetApparentDirection(void) const;
            void                        GetStrengthData(float& fTarget, float& fActual);
            void                        GetGustData(float& fTarget, float& fActual);

            // utility
            const SParams&              GetParams(void) const                       { return m_sParams; }

    protected:
            void                        Gust(float fTime);
            float                  RandomFloat(float fMin, float fMax);
            float                  LinearSigmoid(float fInput, float fLinearness);
            void                        Normalize(float* pVector);

            SParams                     m_sParams;
            float                  m_fStrength;
            float                  m_afDirection[3];
            const CWind*                m_pWindLeader;
            CRandom                     m_cDice;

            float                  m_fLastTime;
            float                  m_fElapsedTime;

            float                  m_fGust;
            float                  m_fGustTarget;
            float                  m_fGustRiseTarget;
            float                  m_fGustFallTarget;
            float                  m_fGustStart;
            float                  m_fGustAtStart;
            float                  m_fGustFallStart;

            float                  m_fStrengthTarget;
            float                  m_fStrengthChangeStartTime;
            float                  m_fStrengthChangeEndTime;
            float                  m_fStrengthAtStart;

            float                  m_afDirectionTarget[3];
            float                  m_afDirectionMidTarget[3];
            float                  m_fDirectionChangeStartTime;
            float                  m_fDirectionChangeEndTime;
            float                  m_afDirectionAtStart[3];

            float                  m_afEffectiveStrengths[NUM_COMPONENTS];
            float                  m_afOscillationTimes[NUM_COMPONENTS];

            float                  m_fCombinedStrength;

            float                  m_afShaderValues[NUM_SHADER_VALUES];
    };

} // end namespace SpeedTree


///////////////////////////////////////////////////////////////////////  
//  Packing

#if defined(_WIN32) || defined(_XBOX)
    #pragma pack(pop)   
    #pragma warning(pop)    
#endif

