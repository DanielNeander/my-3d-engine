///////////////////////////////////////////////////////////////////////  
//  GrassRI.h
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
#include "GameApp/Terrain/Forest/Grass.h"
#include "GameApp/Terrain/RenderInterface/ForestRI.h"
#include "GameApp/Terrain/RenderInterface/TerrainRI.h"


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    ///////////////////////////////////////////////////////////////////////  
    //  Structure SGrassRenderInfo

    struct ST_STORAGE_CLASS SGrassRenderInfo
    {
                            SGrassRenderInfo( );

            // terrain matching 
            noVec4            m_avTerrainSplatColors[c_nNumTerrainSplatLayers];
            float      m_fTerrainMatchFactorTop;
            float      m_fTerrainMatchFactorBottom;

            // lighting
            SMaterial       m_sMaterial;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CGrassRI

    #define CGrassRI_TemplateList template<class TRenderStatePolicy, class TTexturePolicy, class TGeometryBufferPolicy, class TShaderLoaderPolicy, class TShaderTechniquePolicy, class TShaderConstantPolicy, class TDepthTexturePolicy>
    #define CGrassRI_Class CGrassRI<TRenderStatePolicy, TTexturePolicy, TGeometryBufferPolicy, TShaderLoaderPolicy, TShaderTechniquePolicy, TShaderConstantPolicy, TDepthTexturePolicy>

    CGrassRI_TemplateList
    class ST_STORAGE_CLASS CGrassRI : public CGrass
    {
    public:
                                            CGrassRI( );
                                            ~CGrassRI( );

            // graphics initialization
            void                            SetShaderLoader(TShaderLoaderPolicy* pShaderLoader);
            void                            SetRenderInfo(const SGrassRenderInfo& sInfo);
            const SGrassRenderInfo&         GetRenderInfo(void) const;
            void                            SetTerrainSplatColors(const noVec4 avSplatColors[c_nNumTerrainSplatLayers]);
            bool                         Init(const char* pTextureFilename, int nImageRows, int nImageCols, float fCellSize, const SVertexAttribDesc* pVertexFormat);

            // rendering functions
            bool                         Render(const SGrassCullResults& sCullResults, ERenderPassType ePassType, const SMaterial& sLighting, SRenderStats* pStats = NULL);

            // culling & LOD
            void                            Cull(const CView& cView, SGrassCullResults& sResults);

            // render states
            void                            EnableTexturing(bool bFlag);

    private:
            // render support
            bool                         BeginRender(TShaderTechniquePolicy& tTechnique, const SMaterial& sLighting);
            bool                         EndRender(TShaderTechniquePolicy& tTechnique);
            void                            ComputeUnitBillboard(const CView& cView);

            // VBO management
            bool                         InitVboPool(void);
            TGeometryBufferPolicy*          NextVbo(void);
            TGeometryBufferPolicy*          NewVbo(void);
            bool                         ResizeIndexBuffer(int nNumGrassBlades);

            bool                         InitShader(void);
            bool                         ShaderIsLoaded(void) const;

            // texturing
            TTexturePolicy                  m_tCompositeTexture;
            TTexturePolicy                  m_tWhiteTexture;

            // shaders
            TShaderLoaderPolicy*            m_pShaderLoader;
            TShaderTechniquePolicy          m_tTechnique;

            // shader constants
            TShaderConstantPolicy           m_tCompositeTextureVar;
            TShaderConstantPolicy           m_tUnitBillboard;
            TShaderConstantPolicy           m_tTerrainSplatColors;
            TShaderConstantPolicy           m_tRenderInfo;
            TShaderConstantPolicy           m_tLightAndMaterial;

            // render mechanics
            TRenderStatePolicy              m_tRenderStatePolicy;
            CArray<TGeometryBufferPolicy*>  m_aVboPool;
            TGeometryBufferPolicy           m_tIndexBuffer;
            const SVertexAttribDesc*        m_pVertexFormat;

            // render states
            bool                         m_bTexturingEnabled;
            noMat4                          m_mCameraFacingUnitBillboard;
            SGrassRenderInfo                m_sRenderInfo;
    };

    // include inline functions
    #include "GrassRI.inl"

} // end namespace SpeedTree

#include "EngineCore/SpeedTree/ExportEnd.h"

