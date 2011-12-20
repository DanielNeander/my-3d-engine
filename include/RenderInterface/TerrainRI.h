///////////////////////////////////////////////////////////////////////  
//  TerrainRI.h
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
#include "Core/ExportBegin.h"
#include "Forest/Terrain.h"
#include "RenderInterface/ForestRI.h"


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    const st_int32 c_nNumTerrainSplatLayers = 3;

    ///////////////////////////////////////////////////////////////////////  
    //  Structure STerrainRenderInfo

    struct ST_STORAGE_CLASS STerrainRenderInfo
    {
                                STerrainRenderInfo( );

            // splat parameters
            CFixedString        m_strNormalMap;
            CFixedString        m_strSplatMap;
            CFixedString        m_astrSplatLayers[c_nNumTerrainSplatLayers];
            st_float32          m_afSplatTileValues[c_nNumTerrainSplatLayers];
            st_float32          m_fNormalMapBlueScale;

            // lighting
            SMaterial           m_sMaterial;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CTerrainRI

    #define CTerrainRI_TemplateList template<class TRenderStatePolicy, class TTexturePolicy, class TGeometryBufferPolicy, class TShaderLoaderPolicy, class TShaderTechniquePolicy, class TShaderConstantPolicy, class TDepthTexturePolicy>
    #define CTerrainRI_Class CTerrainRI<TRenderStatePolicy, TTexturePolicy, TGeometryBufferPolicy, TShaderLoaderPolicy, TShaderTechniquePolicy, TShaderConstantPolicy, TDepthTexturePolicy>

    CTerrainRI_TemplateList
    class ST_STORAGE_CLASS CTerrainRI : public CTerrain
    {
    public:
                                            CTerrainRI( );
                                            ~CTerrainRI( );

            // graphics initialization
            st_bool                         Init(st_int32 nNumLods, st_int32 nMaxTileRes, st_float32 fCellSize, const SVertexAttribDesc* pVertexFormat);
            void                            SetShaderLoader(TShaderLoaderPolicy* pShaderLoader);
            void                            SetRenderInfo(const STerrainRenderInfo& sInfo);
            const STerrainRenderInfo&       GetRenderInfo(void) const;
            void                            SetMaxAnisotropy(st_int32 nMaxAnisotropy);

            // rendering functions
            st_bool                         Render(const STerrainCullResults& sCullResults, ERenderPassType ePassType, const SMaterial& sLighting, SRenderStats* pStats = NULL);

            // culling & LOD
            void                            CullAndComputeLOD(const CView& cView, STerrainCullResults& sCullResults);

            // render states
            void                            EnableTexturing(st_bool bFlag);

    private:
            // render support
            st_bool                         BeginRender(TShaderTechniquePolicy& tTechnique, const SMaterial& sLighting);
            st_bool                         EndRender(TShaderTechniquePolicy& tTechnique);
            st_bool                         SetTexture(const TShaderConstantPolicy& tTextureVar, const TTexturePolicy& tTextureMap) const;

            // VBO management
            st_bool                         InitVboPool(void);
            TGeometryBufferPolicy*          NextVbo(void);
            TGeometryBufferPolicy*          NewVbo(void) const;

            st_bool                         InitIndexBuffer(void);
            st_bool                         InitShader(void);
            st_bool                         ShaderIsLoaded(void) const;

            // texture
            TTexturePolicy                  m_tNormalMapTexture;
            TTexturePolicy                  m_tSplatMapTexture;
            TTexturePolicy                  m_atSplatTextureLayers[c_nNumTerrainSplatLayers];
            TTexturePolicy                  m_tWhiteTexture;
            st_int32                        m_nMaxAnisotropy;

            // terrain splat data
            STerrainRenderInfo              m_sRenderInfo;

            // shaders
            TShaderLoaderPolicy*            m_pShaderLoader;
            TShaderTechniquePolicy          m_atTechniques[CForestRI_Class::SHADER_TYPE_NUMTYPES];

            // shader constants
            TShaderConstantPolicy           m_tNormalMapTextureVar;
            TShaderConstantPolicy           m_tSplatMapTextureVar;
            TShaderConstantPolicy           m_atSplatTextureLayersVars[c_nNumTerrainSplatLayers];
            TShaderConstantPolicy           m_tSplatMapTiles;
            TShaderConstantPolicy           m_tLightAndMaterial;

            // render mechanics
            TRenderStatePolicy              m_tRenderStatePolicy;
            CArray<TGeometryBufferPolicy*>  m_aVboPool;
            TGeometryBufferPolicy           m_tIndexBuffer;
            const SVertexAttribDesc*        m_pVertexFormat;

            // render states
            st_bool                         m_bTexturingEnabled;
    };

    // include inline functions
    #include "TerrainRI.inl"

} // end namespace SpeedTree

#include "Core/ExportEnd.h"

