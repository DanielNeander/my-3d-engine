///////////////////////////////////////////////////////////////////////  
//  SkyRI.h
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
//
//  *** Release version 5.0 ***


///////////////////////////////////////////////////////////////////////  
//  Preprocessor

#pragma once
#include "EngineCore/SpeedTree/ExportBegin.h"
#include "EngineCore/SpeedTree/Random.h"
#include "GameApp/Terrain/RenderInterface/ForestRI.h"


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    ///////////////////////////////////////////////////////////////////////  
    //  Structure SSkyRenderInfo

    struct ST_STORAGE_CLASS SSkyRenderInfo
    {
                            SSkyRenderInfo( );

            int        m_nNumLargeStars;
            int        m_nNumSmallStars;
            float      m_fFogMin;
            float      m_fFogMax;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CSkyRI

#define CSkyRI_TemplateList template<class TRenderStatePolicy, class TTexturePolicy, class TGeometryBufferPolicy, class TShaderLoaderPolicy, class TShaderTechniquePolicy, class TShaderConstantPolicy, class TDepthTexturePolicy>
#define CSkyRI_Class CSkyRI<TRenderStatePolicy, TTexturePolicy, TGeometryBufferPolicy, TShaderLoaderPolicy, TShaderTechniquePolicy, TShaderConstantPolicy, TDepthTexturePolicy>

    CSkyRI_TemplateList
    class ST_STORAGE_CLASS CSkyRI
    {
    public:
                                            CSkyRI( );
                                            ~CSkyRI( );

            // graphics initialization
            bool                         Init(void);
            void                            SetShaderLoader(TShaderLoaderPolicy* pShaderLoader);
            void                            SetRenderInfo(const SSkyRenderInfo& sInfo);
            const SSkyRenderInfo&           GetRenderInfo(void) const;

            // rendering functions
            bool                         Render(SRenderStats* pStats = NULL);

            // render states
            void                            EnableTexturing(bool bFlag);

    private:
            // init support
            bool                         InitGeometryBuffer(void);
            bool                         InitShaders(void);

            SSkyRenderInfo                  m_sRenderInfo;

            // rendering
            TRenderStatePolicy              m_tRenderStatePolicy;

            // shaders
            TShaderLoaderPolicy*            m_pShaderLoader;
            TShaderTechniquePolicy          m_tSkyTechnique;
            TShaderTechniquePolicy          m_tStarsTechnique;

            // geometry
            TGeometryBufferPolicy           m_tGeometryBuffer;
    };

    // include inline functions
    #include "SkyRI.inl"

} // end namespace SpeedTree

#include "EngineCore/SpeedTree/ExportEnd.h"

