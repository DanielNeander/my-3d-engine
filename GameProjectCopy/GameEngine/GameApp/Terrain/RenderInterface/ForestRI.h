///////////////////////////////////////////////////////////////////////  
//  ForestRI.h
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
#include "GameApp/Terrain/Forest/Forest.h"
#include "EngineCore/SpeedTree/Set.h"
#include "EngineCore/SpeedTree/List.h"
#include "EngineCore/SpeedTree/Timer.h"


//  When SPEEDTREE_WORLD_TRANSLATE is #defined, the SDK switches into a mode 
//  where the camera stays at the origin and the scene geometry is translated
//  to it, as opposed to translating the camera through the scene. This helps
//  mitigate depth buffer resolution break down with large coordinate values
//  and is recommended.

//#define SPEEDTREE_WORLD_TRANSLATE


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    ///////////////////////////////////////////////////////////////////////  
    //  Class CRenderStateRI

    enum EAlphaFunc
    {
        ALPHAFUNC_LESS,
        ALPHAFUNC_GREATER
    };
    enum ECullType
    {
        CULLTYPE_NONE,
        CULLTYPE_FRONT,
        CULLTYPE_BACK
    };
    enum ERenderStyle
    {
        RENDERSTYLE_SOLID,
        RENDERSTYLE_WIREFRAME
    };
    enum EDepthTestFunc
    {
        DEPTHTEST_EQUAL,
        DEPTHTEST_LESS
    };

    template<class TRenderStatePolicy>
    class ST_STORAGE_CLASS CRenderStateRI
    {
    public:

    static  void       ST_CALL_CONV Initialize(void);
    static  void       ST_CALL_CONV ApplyStates(void);
    static  void       ST_CALL_CONV Destroy(void);

    static  void       ST_CALL_CONV ClearScreen(bool bClearColor = true, bool bClearDepth = true);
    static  void       ST_CALL_CONV SetAlphaFunction(EAlphaFunc eFunc, float fValue);  // fValue range = [0.0,255.0]
    static  void       ST_CALL_CONV SetAlphaTesting(bool bFlag);
    static  void       ST_CALL_CONV SetAlphaToCoverage(bool bFlag);
    static  void       ST_CALL_CONV SetBlending(bool bFlag);
    static  void       ST_CALL_CONV SetColorMask(bool bRed, bool bGreen, bool bBlue, bool bAlpha);
    static  void       ST_CALL_CONV SetDepthMask(bool bFlag);
    static  void       ST_CALL_CONV SetDepthTestFunc(EDepthTestFunc eDepthTestFunc);
    static  void       ST_CALL_CONV SetDepthTesting(bool bFlag);
    static  void       ST_CALL_CONV SetFaceCulling(ECullType eCullType);
    static  void       ST_CALL_CONV SetMultisampling(bool bMultisample);
    static  void       ST_CALL_CONV SetPointSize(float fSize);
    static  void       ST_CALL_CONV SetPolygonOffset(float fFactor = 0.0f, float fUnits = 0.0f); // both zeros deactivates polygon offset
    static  void       ST_CALL_CONV SetRenderStyle(ERenderStyle eStyle);

            TRenderStatePolicy      m_tRenderStatePolicy;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CTextureRI

    template<class TTexturePolicy>
    class ST_STORAGE_CLASS CTextureRI
    {
    public:
            
            // loading
            bool                 Load(const char* pFilename, int nMaxAnisotropy = 0);
            bool                 LoadColor(unsigned int uiColor); // 0xrrggbbaa
            bool                 LoadAlphaNoise(int nWidth, int nHeight);
            bool                 LoadSpecularLookup(void);
            bool                 LoadCubeNormalizer(int nResolution);
            bool                 Unload(void);

            // render
            bool                 FixedFunctionBind(void);
            const TTexturePolicy&   GetPolicy(void) const;

            // other
            const char*             GetFilename(void) const;
            bool                 IsValid(void) const;

            TTexturePolicy          m_tTexturePolicy;

    private:
            SpeedTree::CString      m_strFilename;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class SMaterialRI

    template<class TTexturePolicy>
    struct ST_STORAGE_CLASS SMaterialRI : public SMaterial
    {
            void                    Copy(const SMaterial& sSource);

            CTextureRI<TTexturePolicy>  m_acTextureObjects[TL_NUM_TEX_LAYERS];
    };
    #define SMaterialRI_t SMaterialRI<TTexturePolicy>


    ///////////////////////////////////////////////////////////////////////  
    //  Class CGeometryRI

    enum EVertexAttribSemantic
    {
        VERTEX_ATTRIB_SEMANTIC_POS,             // generic attrib 0
        VERTEX_ATTRIB_SEMANTIC_BLENDWEIGHT,     // generic attrib 1
        VERTEX_ATTRIB_SEMANTIC_NORMAL,          // generic attrib 2
        VERTEX_ATTRIB_SEMANTIC_DIFFUSE,         // generic attrib 3
        VERTEX_ATTRIB_SEMANTIC_SPECULAR,        // generic attrib 4
        VERTEX_ATTRIB_SEMANTIC_TESSFACTOR,      // generic attrib 5
        VERTEX_ATTRIB_SEMANTIC_PSIZE,           // generic attrib 6
        VERTEX_ATTRIB_SEMANTIC_BLENDINDICES,    // generic attrib 7
        VERTEX_ATTRIB_SEMANTIC_TEXCOORD0,       // generic attrib 8
        VERTEX_ATTRIB_SEMANTIC_TEXCOORD1,       // generic attrib 9
        VERTEX_ATTRIB_SEMANTIC_TEXCOORD2,       // generic attrib 10
        VERTEX_ATTRIB_SEMANTIC_TEXCOORD3,       // generic attrib 11
        VERTEX_ATTRIB_SEMANTIC_TEXCOORD4,       // generic attrib 12
        VERTEX_ATTRIB_SEMANTIC_TEXCOORD5,       // generic attrib 13
        VERTEX_ATTRIB_SEMANTIC_TEXCOORD6,       // generic attrib 14
        VERTEX_ATTRIB_SEMANTIC_TEXCOORD7,       // generic attrib 15
        VERTEX_ATTRIB_SEMANTIC_COUNT,
        VERTEX_ATTRIB_SEMANTIC_END
    };

    enum EVertexAttribType
    {
        VERTEX_ATTRIB_TYPE_UNSIGNED_BYTE,
        VERTEX_ATTRIB_TYPE_UNSIGNED_BYTE_NORMALIZED,
        VERTEX_ATTRIB_TYPE_SHORT,
        VERTEX_ATTRIB_TYPE_SHORT_NORMALIZED,
        VERTEX_ATTRIB_TYPE_UNSIGNED_SHORT,
        VERTEX_ATTRIB_TYPE_UNSIGNED_SHORT_NORMALIZED,
        VERTEX_ATTRIB_TYPE_INT,
        VERTEX_ATTRIB_TYPE_INT_NORMALIZED,
        VERTEX_ATTRIB_TYPE_UNSIGNED_INT,
        VERTEX_ATTRIB_TYPE_UNSIGNED_INT_NORMALIZED,
        VERTEX_ATTRIB_TYPE_FLOAT,
        VERTEX_ATTRIB_TYPE_DOUBLE,
        VERTEX_ATTRIB_TYPE_COUNT
    };

    enum EPrimitiveType
    {
        PRIMITIVE_POINTS, 
        PRIMITIVE_LINE_STRIP, 
        PRIMITIVE_LINE_LOOP, 
        PRIMITIVE_LINES, 
        PRIMITIVE_TRIANGLE_STRIP, 
        PRIMITIVE_TRIANGLE_FAN, 
        PRIMITIVE_TRIANGLES, 
        PRIMITIVE_QUAD_STRIP, 
        PRIMITIVE_QUADS
    };

    struct SVertexAttribDesc
    {
            EVertexAttribSemantic   m_eSemantic;
            EVertexAttribType       m_eDataType;
            unsigned short               m_uiNumElements;    // e.g., 3 for (x,y,z)

            unsigned int               SizeOfDataType(void) const;
            unsigned int               SizeOfAttrib(void) const;
            const char*             DataTypeName(void) const;
            const char*             SemanticName(void) const;
    };

    #define VERTEX_ATTRIB_END() { VERTEX_ATTRIB_SEMANTIC_END, VERTEX_ATTRIB_TYPE_FLOAT, 0 }

    enum EIndexFormat
    {
        INDEX_FORMAT_UNSIGNED_16BIT,
        INDEX_FORMAT_UNSIGNED_32BIT
    };

    template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
    class ST_STORAGE_CLASS CGeometryBufferRI
    {
    public:
                                        CGeometryBufferRI( );
            virtual                     ~CGeometryBufferRI( );

            // vertex buffer
            bool                     SetVertexFormat(const SVertexAttribDesc* pAttribDesc, const TShaderTechniqueRI* pTechnique, bool bDynamic = false);
            bool                     AppendVertices(const void* pVertexData, unsigned int uiNumVertices);
            bool                     ReplaceVertices(const void* pVertexData, unsigned int uiNumVertices);
            bool                     EndVertices(void);
            unsigned int                   NumVertices(void) const;
            unsigned int                   VertexSize(void) const;
    static  unsigned int      ST_CALL_CONV SizeOf(EVertexAttribType eDataType);

            bool                     EnableFormat(void) const;
    static  bool        ST_CALL_CONV DisableFormat(void);

            bool                     BindVertexBuffer(void) const;
    static  bool        ST_CALL_CONV UnBindVertexBuffer(void);

            // index buffer
            bool                     SetIndexFormat(EIndexFormat eFormat);
            bool                     AppendIndices(const void* pIndexData, unsigned int uiNumIndices);
            bool                     ReplaceIndices(const void* pIndexData, unsigned int uiNumIndices);
            bool                     EndIndices(void);
            unsigned int                   NumIndices(void) const;

            bool                     BindIndexBuffer(void) const;
            bool                     UnBindIndexBuffer(void) const;

            // render functions
            bool                     RenderIndexed(EPrimitiveType ePrimType, unsigned int uiStartIndex, unsigned int uiNumIndices, unsigned int uiNumVerticesOverride = 0) const;
            bool                     RenderArrays(EPrimitiveType ePrimType, unsigned int uiStartVertex, unsigned int uiNumVertices) const;
    static  bool        ST_CALL_CONV IsPrimitiveTypeSupported(EPrimitiveType ePrimType);

            TGeometryBufferPolicy       m_tGeometryBufferPolicy;

    private:
            // vertex buffer
            CArray<unsigned char>             m_aVertexData;
            unsigned int                   m_uiVertexSize;         // in bytes
            unsigned int                   m_uiNumVertices;
#ifndef NDEBUG
    mutable bool                     m_bFormatEnabled;
    mutable bool                     m_bVertexBufferBound;
    mutable bool                     m_bIndexBufferBound;
#endif

            // index buffer
            CArray<unsigned char>             m_aIndexData;
            unsigned int                   m_uiIndexSize;          // in bytes
            unsigned int                   m_uiNumIndices;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CShaderTechniqueRI

    template<class TShaderTechniquePolicy>
    class ST_STORAGE_CLASS CShaderTechniqueRI
    {
    public:
                                    CShaderTechniqueRI( );
                                    ~CShaderTechniqueRI( );
    
            bool                 Bind(unsigned int uiPass = 0);
            bool                 UnBind(void);
            bool                 CommitConstants(void);
            bool                 CommitTextures(void);

            bool                 IsValid(void) const;
            const char*             GetName(void) const;
            void                    SetName(const char* pName);

            TShaderTechniquePolicy  m_tShaderTechniquePolicy;

    private:
            CString                 m_strName;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CShaderConstantRI

    template<class TShaderConstantPolicy>
    class ST_STORAGE_CLASS CShaderConstantRI
    {
    public:
                                    CShaderConstantRI( );
                                    ~CShaderConstantRI( );
    
            bool                 Set1f(float x) const;
            bool                 Set2f(float x, float y) const;
            bool                 Set2fv(const float afValues[2]) const;
            bool                 Set3f(float x, float y, float z) const;
            bool                 Set3fv(const float afValues[3]) const;
            bool                 Set3fvPlus1f(const float afValues[3], float w) const;
            bool                 Set4f(float x, float y, float z, float w) const;
            bool                 Set4fv(const float afValues[4]) const;
            bool                 SetArray4f(const float* pArray, int nSize, int nOffset = 0) const;
            bool                 SetMatrix(const float afMatrix[16]) const;
            bool                 SetMatrixTranspose(const float afMatrix[16]) const;
            bool                 SetMatrixIndex(const float afMatrix[16], unsigned int uiIndex) const;
            bool                 SetMatrix4x4Array(const float* pMatrixArray, unsigned int uiOffset, unsigned int uiNumMatrices) const;

            bool                 IsValid(void) const;
            const char*             GetName(void) const;
            void                    SetName(const char* pName);

            TShaderConstantPolicy   m_tShaderConstantPolicy;

    private:
            CString                 m_strName;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CShaderLoaderRI

    template<class TShaderLoaderPolicy, class TShaderTechniqueRI, class TShaderConstantRI>
    class ST_STORAGE_CLASS CShaderLoaderRI
    {
    public:
                                    CShaderLoaderRI( );
                                    ~CShaderLoaderRI( );

            bool                 Load(const char* pFilename, const CArray<CString>& aDefines);
            const char*             GetError(void) const;

            bool                 GetTechnique(const char* pName, TShaderTechniqueRI& cTechnique) const;
            bool                 GetConstant(const char* pName, TShaderConstantRI& cConstant) const;

            void                    Release(void);
            void                    OnResetDevice(void);
            void                    OnLostDevice(void);

            TShaderLoaderPolicy     m_tShaderLoaderPolicy;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CDepthTextureRI

    template<class TDepthTexturePolicy>
    class ST_STORAGE_CLASS CDepthTextureRI
    {
    public:
                                    CDepthTextureRI( );
                                    ~CDepthTextureRI( );

            bool                 Create(int nWidth, int nHeight);

            void                    Release(void);
            void                    OnResetDevice(void);
            void                    OnLostDevice(void);

            bool                 SetAsTarget(void);
            bool                 ReleaseAsTarget(void);

            bool                 BindAsTexture(void);
            bool                 UnBindAsTexture(void);
            void                    EnableShadowMapComparison(bool bFlag);

            TDepthTexturePolicy     m_tDepthTexturePolicy;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Enumeration EStatsCategory

    enum EStatsCategory
    {
        // main geometry types
        STATS_CATEGORY_BRANCHES,
        STATS_CATEGORY_FRONDS,
        STATS_CATEGORY_LEAF_CARDS,
        STATS_CATEGORY_LEAF_MESHES,
        STATS_CATEGORY_MESHES,
        STATS_CATEGORY_BILLBOARDS,
        STATS_CATEGORY_TERRAIN,
        STATS_CATEGORY_GRASS,
        STATS_CATEGORY_SKY,

        // utility
        STATS_CATEGORY_COUNT
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Enumeration ERenderPassType

    enum ERenderPassType
    {
        RENDER_PASS_STANDARD,
        RENDER_PASS_DEPTH_ONLY,
        RENDER_PASS_SHADOW,
        RENDER_PASS_NUM_TYPES
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Enumeration ETextureAlphaRenderMode

    enum ETextureAlphaRenderMode
    {
        TRANS_TEXTURE_ALPHA_TESTING,
        TRANS_TEXTURE_ALPHA_TO_COVERAGE,
        TRANS_TEXTURE_BLENDING,
        TRANS_TEXTURE_NOTHING,
        TRANS_TEXTURE_UNASSIGNED
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CLodRenderInfo
    //
    //  Class used internally; not for client-side use.

    class ST_STORAGE_CLASS CLodRenderInfo
    {
    public:
            struct SRef
            {
                int                m_nMaterialId;
                int                m_nOffset;
                int                m_nLength;
            };

                                        CLodRenderInfo( );

            // init functions
            void                        InitIndexed(const SIndexedTriangles* pLods, int nNumLods);
            void                        InitLeafCards(const SLeafCards* pLods, int nNumLods);
            void                        AddRef(int nLod, int nMaterialId, int nOffset, int nLength);

            // calls made during render loop
            const CLodRenderInfo::SRef* IsActive(int nLod, int nMaterialId) const;
            const CSet<int>*       MaterialRefs(void) const;

    private:
            int                    m_nNumLods;
            int                    m_nMaxMaterialId;
            CArray<SRef>                m_aRefs;

            CSet<int>              m_sMaterialsUsed;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CTreeRI

    template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
    class ST_STORAGE_CLASS CTreeRI : public CTree
    {
    public:
                                            CTreeRI( );
            virtual                         ~CTreeRI( );
    
            // graphics
            bool                         InitGraphics(const CArray<CFixedString>& aSearchPaths, bool bQuadsSupported, const TShaderTechniquePolicy** pTechniques, int nMaxAnisotropy = 0);
            bool                         GraphicsAreInitialized(void) const;
            const TGeometryBufferPolicy*    GetGeometryBuffer(EGeometryType eType) const;

            // geometry
            const CArray<SMaterialRI_t>*    GetMaterials(void) const;
            const CLodRenderInfo*           GetLodRenderInfo(EGeometryType eType) const;
            bool                         CardsAreQuads(void) const;

    protected:
            bool                         InitMaterials(const SGeometry* pGeometry, const CArray<CFixedString>& aSearchPaths, int nMaxAnisotropy = 0);
            bool                         InitBranches(const SGeometry* pGeometry, const TShaderTechniquePolicy* pTechnique);
            bool                         InitFronds(const SGeometry* pGeometry, const TShaderTechniquePolicy* pTechnique);
            bool                         InitLeafCards(const SGeometry* pGeometry, bool bQuadsSupported, const TShaderTechniquePolicy* pTechnique);
            bool                         InitLeafMeshes(const SGeometry* pGeometry, const TShaderTechniquePolicy* pTechnique);

            // used by indexed triangle geometry types (branches, fronds, leaf meshes)
            bool                         InitIndexBuffer(int nLod, const SIndexedTriangles* pLod, TGeometryBufferPolicy* pGeometryBuffer, CLodRenderInfo& cLodRenderInfo);

            // used by leaf cards
            bool                         InitLeafCardRenderInfo(int nLod, const SLeafCards* pLod);

            // materials/textures
            CArray<SMaterialRI_t>           m_aMaterials;

            // geometry
            CLodRenderInfo                  m_acRenderInfo[GEOMETRY_TYPE_NUM_3D_TYPES];
            TGeometryBufferPolicy           m_atGeometryBuffers[GEOMETRY_TYPE_NUM_3D_TYPES];
            bool                         m_bCardsAreQuads;

            // textures
            const CArray<CString>*          m_pSearchPaths;

            // misc
            bool                         m_bGraphicsInitialized;

    private:
                                            CTreeRI(const CTreeRI& cRight);     // copying CTreeRI disabled
    };
    #define CTreeRI_t CTreeRI<TTexturePolicy, TGeometryBufferPolicy, TShaderTechniquePolicy>


    ///////////////////////////////////////////////////////////////////////  
    //  Type definitions

    typedef CArray<unsigned int> MaterialIndices;


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SRenderStats

    struct ST_STORAGE_CLASS SRenderStats
    {
    public:
                                    SRenderStats( );

            struct ST_STORAGE_CLASS SGeometryStats
            {
                                    SGeometryStats( );

                void                Reset(void);

                unsigned int           m_uiNumObjects;
                unsigned int           m_uiNumTextureBinds;
                unsigned int           m_uiNumDrawCalls;
                unsigned int           m_uiNumVboBinds;
                unsigned int           m_uiNumTriangles;
            };

            void                    Reset(void);
            void                    GetTotals(SGeometryStats& sTotals) const;

            SGeometryStats          m_aGeometries[STATS_CATEGORY_COUNT][RENDER_PASS_NUM_TYPES];
            float              m_fCullTime;        // microseconds
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SForestRenderInfo

    struct ST_STORAGE_CLASS SForestRenderInfo
    {
                                        SForestRenderInfo( );

            // general rendering
            CFixedString                m_strShaderFilename;            // see note below
            int                    m_nMaxAnisotropy;               // see note below
            bool                     m_bHorizontalBillboards;        // see note below
            float                  m_fAlphaTestScalar;
            bool                     m_bZPrePass;                    // see note below
            int                    m_nMaxBillboardImagesByBase;    // the maximum # of billboard images any one base tree may have

            // lighting
            float                  m_fGlobalLightScalar;
            SMaterial                   m_sLightMaterial;
            bool                     m_bSpecularLighting;            // see note below
            bool                     m_bTransmissionLighting;        // see note below
            bool                     m_bDetailLayer;                 // see note below
            bool                     m_bDetailNormalMapping;         // see note below
            bool                     m_bAmbientContrast;             // see note below

            // fog
            float                  m_fFogStartDistance;
            float                  m_fFogEndDistance;
            noVec3                        m_vFogColor;

            // sky
            noVec3                        m_vSkyColor;
            float                  m_fSkyFogMin;
            float                  m_fSkyFogMax;

            // sun
            noVec3                        m_vSunColor;
            float                  m_fSunSize;
            float                  m_fSunSpreadExponent;
            float                  m_fSunFogBloom;

            // shadows
            int                    m_nNumShadowMaps;
            int                    m_nShadowMapResolution;
            bool                     m_bSmoothShadows;               // see note below
            bool                     m_bShowShadowSplitsOnTerrain;   // see note below

            // wind
            bool                     m_bWindEnabled;                 // set note below
            bool                     m_bFrondRippling;               // see note below

            // *note: these values will be ignored if changed after CForestRI::InitGraphics() has been called
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Billboard vertex description

    struct SBillboardVertex
    {
        noVec3        m_vPosition; float m_fCornerIndex;     // position
        float  m_fWidth, m_fHeight, m_fScale, m_fRotation; // texcoord0
    };

    static const SVertexAttribDesc asBillboardVertexAttribs[ ] =
    {
        { VERTEX_ATTRIB_SEMANTIC_POS, VERTEX_ATTRIB_TYPE_FLOAT, 4 },
        { VERTEX_ATTRIB_SEMANTIC_TEXCOORD0, VERTEX_ATTRIB_TYPE_FLOAT, 4 },
        VERTEX_ATTRIB_END( )
    };



    ///////////////////////////////////////////////////////////////////////  
    //  Class CForestRI

    #define CForestRI_TemplateList template<class TRenderStatePolicy, class TTexturePolicy, class TGeometryBufferPolicy, class TShaderLoaderPolicy, class TShaderTechniquePolicy, class TShaderConstantPolicy, class TDepthTexturePolicy>
    #define CForestRI_Class CForestRI<TRenderStatePolicy, TTexturePolicy, TGeometryBufferPolicy, TShaderLoaderPolicy, TShaderTechniquePolicy, TShaderConstantPolicy, TDepthTexturePolicy>

    CForestRI_TemplateList
    class ST_STORAGE_CLASS CForestRI : public CForest
    {
    public:
            enum EShaderType
            {
                SHADER_TYPE_STANDARD,
                SHADER_TYPE_DEPTH_ONLY,
                SHADER_TYPE_SHADOW,
                SHADER_TYPE_NUMTYPES
            };

                                            CForestRI( );
                                            ~CForestRI( );
    
            // tree management
            bool                         RegisterTree(CTreeRI_t* pTree, const char* pTexturePath = NULL);
            bool                         ReplaceTree(const CTreeRI_t* pOldTree, CTreeRI_t* pNewTree, const char* pTexturePath = NULL);
    virtual CTree*                          AllocateTree(void) const;

            // culling & dynamic VBO updates
    virtual void                            CullAndComputeLOD(const CView& cView, SForestCullResults& sResults, bool bSort = false);

            // general graphics
            void                            SetRenderInfo(const SForestRenderInfo& sInfo);
            const SForestRenderInfo&        GetRenderInfo(void) const;
            bool                         InitGraphics(void);

            // main render functions
            bool                         StartRender(void);
            bool                         EndRender(void);
            bool                         UploadViewShaderParameters(const CView& cView) const;
            bool                         RenderBranches(const SForestCullResults& sVisibleTrees, ERenderPassType ePassType);
            bool                         RenderFronds(const SForestCullResults& sVisibleTrees, ERenderPassType ePassType);
            bool                         RenderLeafMeshes(const SForestCullResults& sVisibleTrees, ERenderPassType ePassType);
            bool                         RenderLeafCards(const SForestCullResults& sVisibleTrees, ERenderPassType ePassType, const CView& cView);
            bool                         RenderBillboards(const SForestCullResults& sVisibleTrees, ERenderPassType ePassType, const CView& cView);

            // shadows
            bool                         BeginShadowMap(int nShadowMap, const CView& cLightView);
            bool                         EndShadowMap(int nShadowMap);
            bool                         ShadowsAreEnabled(void) const;
            TDepthTexturePolicy*            GetShadowDepthTexture(int nShadowMap); 

            // texturing
            void                            EnableTexturing(bool bFlag);
            void                            TransparentTextureModeNotify(ETextureAlphaRenderMode eMode);

            // wind
            bool                         UploadWindParams(const CWind& cWind) const;

            // overlays
            bool                         InitOverlays(void);
            bool                         RenderOverlays(void);

            // needed mostly for DirectX devices
            void                            OnResetDevice(void);
            void                            OnLostDevice(void);

            // statistics
            SRenderStats&                   GetRenderStats(void);

            // shaders
            TShaderLoaderPolicy*            GetShaderLoader(void);
    static  CFixedString                    ShaderNameByPass(const CFixedString& strBaseName, EShaderType eShaderType);
    static  TShaderTechniquePolicy&         PickTechnique(TShaderTechniquePolicy atAvailableTechniques[SHADER_TYPE_NUMTYPES], ERenderPassType ePassType);

    protected:
            // shader setup
            bool                         LoadShaders(void);
            bool                         QueryUniformVariables(void);
            bool                         SetupOverlayShader(void);

            // shader uploads
            int                        UploadMaterial(const SMaterialRI_t* pMaterial, bool bTextureOnly = false);
            bool                         UploadBaseTreeParameters(const CTreeRI_t* pBaseTree) const;

            // wind
            void                            UploadWindParams(const CTreeRI_t* pBaseTree, const CWind* pWind = NULL);

            // billboards
            bool                         ResizeBillboardIndexBuffer(int nMaxNumBillboards);
            bool                         InitBillboardVboPool(void);
            TGeometryBufferPolicy*          NextBillboardVbo(void);
            TGeometryBufferPolicy*          NewBillboardVbo(void);
            void                            PopulateBillboardVbos(TTreeCellArrayNonConst& aCells);
            bool                         UploadBillboardTangents(float fAzimuth) const;

            // render states
            void                            DisableTransparentTextures(void);

            // misc rendering
            void                            InitFallbackTextures(void);

            // data
            CArray<CTreeRI_t*>              m_aBaseTrees;

            // rendering
            SForestRenderInfo               m_sRenderInfo;
            TRenderStatePolicy              m_tRenderStatePolicy;
            TShaderLoaderPolicy             m_tShaderLoader;
            TShaderTechniquePolicy          m_atBranchTechniques[SHADER_TYPE_NUMTYPES];
            TShaderTechniquePolicy          m_atFrondTechniques[SHADER_TYPE_NUMTYPES];
            TShaderTechniquePolicy          m_atLeafCardTechniques[SHADER_TYPE_NUMTYPES];
            TShaderTechniquePolicy          m_atLeafMeshTechniques[SHADER_TYPE_NUMTYPES];
            TShaderTechniquePolicy          m_atBillboardTechniques[SHADER_TYPE_NUMTYPES];

            // texture management
            ETextureAlphaRenderMode         m_eTextureAlphaMethod;
            bool                         m_bTexturingEnabled;
            TTexturePolicy                  m_tAlphaTestingNoiseMap;

            // uniform shader constants (shared by multiple techniques)
            TShaderConstantPolicy           m_tProjModelviewMatrix;
            TShaderConstantPolicy           m_tCameraFacingMatrix;
            TShaderConstantPolicy           m_tTreePosAndScale;
            TShaderConstantPolicy           m_tTreeRotation;
            TShaderConstantPolicy           m_tLodProfile;
            TShaderConstantPolicy           m_tMaterial;
            TShaderConstantPolicy           m_tMaterialExtras;
            TShaderConstantPolicy           m_tMaterialTransmission;
            TShaderConstantPolicy           m_atTextures[TL_NUM_TEX_LAYERS];
            TShaderConstantPolicy           m_tLightDir;
            TShaderConstantPolicy           m_tFogParams;
            TShaderConstantPolicy           m_tFogColor;
            TShaderConstantPolicy           m_tCameraAngles;
            TShaderConstantPolicy           m_tCameraPosition;
            TShaderConstantPolicy           m_tCameraDirection;
            TShaderConstantPolicy           m_tFarClip;
            TShaderConstantPolicy           m_tSkyColor;
            TShaderConstantPolicy           m_tSunColor;
            TShaderConstantPolicy           m_tSunParams;

            // wind shader constants
            TShaderConstantPolicy           m_tWindDir;
            TShaderConstantPolicy           m_tWindTimes;
            TShaderConstantPolicy           m_tWindDistances;
            TShaderConstantPolicy           m_tWindLeaves;
            TShaderConstantPolicy           m_tWindFrondRipple;
            TShaderConstantPolicy           m_tWindGust;
            TShaderConstantPolicy           m_tWindGustHints;

            // billboard shader constants
            TShaderConstantPolicy           m_tNum360Images;
            TShaderConstantPolicy           m_tCameraAzimuthTrig;
            TShaderConstantPolicy           m_t360BillboardTexCoords;
            TShaderConstantPolicy           m_tBillboardTangents;
            TShaderConstantPolicy           m_tHorzBillboardFade;
            TShaderConstantPolicy           m_tHorzBillboardTexCoords;

            // billboards
            CArray<TGeometryBufferPolicy*>  m_aBillboardVboPool;
            TGeometryBufferPolicy           m_tBillboardIndexBuffer;
            CArray<SBillboardVertex>        m_aBillboardPopulationBuffer;

            // shadows
            TDepthTexturePolicy             m_atShadowBuffers[c_nMaxNumShadowMaps];
            TShaderConstantPolicy           m_atShadowMaps[c_nMaxNumShadowMaps];
            TShaderConstantPolicy           m_tLightViewMatricesArray;
            TShaderConstantPolicy           m_tShadowSplitDistances;
            TShaderConstantPolicy           m_tShadowFadeParams;

            // overlays
            TShaderTechniquePolicy          m_tOverlayTechnique;
            TShaderConstantPolicy           m_tOverlayTextureVar;
            TGeometryBufferPolicy           m_tOverlayGeometry;
            
            // render-related
            TTexturePolicy                  m_atFallbackTextures[TL_NUM_TEX_LAYERS];
            SRenderStats                    m_sRenderStats;
            bool                         m_bCanRenderQuads;

    private:
                                            CForestRI(const CForestRI& cRight);     // copying CForestRI disabled
    };

    // include inline functions
    #include "TextureRI.inl"
    #include "RenderStateRI.inl"
    #include "GeometryBufferRI.inl"
    #include "ShaderRI.inl"
    #include "DepthTextureRI.inl"
    #include "TreeRI.inl"
    #include "MiscRI.inl"
    #include "ForestRI.inl"
    #include "BillboardRI.inl"
    #include "OverlayRI.inl"

} // end namespace SpeedTree

#include "EngineCore/SpeedTree/ExportEnd.h"
