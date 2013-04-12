///////////////////////////////////////////////////////////////////////  
//  Core.h
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
//
//  The SpeedTree SDK generally depends on the following external macros to 
//  distinguish platforms:
//
//    Windows 32/64:
//      - _WIN32: defined in both 32- and 64-bit environments
//
//    XBOX 360/Xenon:
//      - _XBOX: Xenon only; no SpeedTree version currently exists for the 
//               original XBOX (SPEEDTREE_BIG_ENDIAN is #defined if detected)
//      - _WIN32: also defined for Xbox 360
//
//    PLAYSTATION(r)3:
//      - __CELLOS_LV2__ (SPEEDTREE_BIG_ENDIAN is #defined if detected)
//
//    Mac OSX:
//      - __APPLE__

#pragma once
//#include "Core/ExportBegin.h"
#include "EngineCore/Types.h"
#include "CoordSys.h"
#include "EngineCore/Math/Matrix.h"
#include "Extents.h"
#include "FixedString.h"
#include "Wind.h"
#include <cstdlib>


///////////////////////////////////////////////////////////////////////  
//  Versioning

#define SPEEDTREE_VERSION           5.0
#define SPEEDTREE_VERSION_STRING    "5.0"


///////////////////////////////////////////////////////////////////////  
//  Platform specifics

#if defined(_XBOX) || defined(__CELLOS_LV2__)
    #define SPEEDTREE_BIG_ENDIAN
#endif


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
    //  Forward references

    class CAllocator;
    class CGeometry;


    ///////////////////////////////////////////////////////////////////////  
    //  Enumeration ETextureLayers

    enum ETextureLayers
    {
        TL_DIFFUSE,        
        TL_NORMAL,         
        TL_SPECULAR,  
        TL_DETAIL_DIFFUSE,        
        TL_DETAIL_NORMAL,

        TL_NUM_TEX_LAYERS
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SMaterial

    struct ST_STORAGE_CLASS SMaterial
    {
                        SMaterial( ) {}
                        ~SMaterial( ) {}

        noVec4            m_vAmbient;                     // alpha value is reserved
        noVec4            m_vDiffuse;                     // alpha value is reserved
        noVec4            m_vSpecular;                    // alpha value is reserved
        noVec4            m_vEmissive;                    // emissive is used for transmission effects; alpha value is reserved
        float      m_fShininess;                   // value ranges [0-128]

        // special material values
        float      m_fLightScalar;                 // multiplied against the diffuse color during lighting
        float      m_fAlphaScalar;                 // for textures with alpha values; blending, a2c, and alpha testing all require different scalars
        float      m_fAmbientContrast;             // contributes to a non-uniform ambient lighting layer
        float      m_fTransmissionShadow;          // how light/dark shadows are on transmission surfaces when looking through back side
        float      m_fTransmissionViewDependence;  // amount of transmission effect to use, based on look vector

        CFixedString    m_astrTextureFilenames[TL_NUM_TEX_LAYERS];
        noVec4            m_vTexCoords;                   // (left, bottom, right, top) used to reference special textures, forced into an atlas from the Modeler 

        CFixedString    m_strUserData;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SDrawCallInfo

    struct ST_STORAGE_CLASS SDrawCallInfo
    {
                        SDrawCallInfo( );

        int        m_nMaterialIndex;

        // when SDrawCallInfo is used to describe leaf card geometry, m_nOffset and m_nLength both represent
        // the number of cards (not vertices); when SDrawCallInfo is used to describe branch, frond, and leaf
        // mesh geometry, m_nOffset and m_nLength represent vertices
        int        m_nOffset;
        int        m_nLength;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SIndexedTriangles

    struct ST_STORAGE_CLASS SIndexedTriangles
    {
                                SIndexedTriangles( );
                                ~SIndexedTriangles( );

        bool                 HasGeometry(void) const;

        int                m_nNumMaterialGroups;
        const SDrawCallInfo*    m_pDrawCallInfo;            // m_nNumMaterialGroups number of elements
        const unsigned short*        m_pTriangleIndices16;       // 16-bit, default indices (default)
        const unsigned int*        m_pTriangleIndices32;       // if a tree is large enough, this structure will be used instead


        // vertex attribute tables
        int                m_nNumVertices;
        const float*       m_pCoords;                  // sets of 3 (m_nNumVertices * 3 total floats)
        const float*       m_pLodCoords;               // sets of 3 (m_nNumVertices * 3 total floats)
        const unsigned char*         m_pNormals;                 // sets of 3 (m_nNumVertices * 3 total uint8s)
        const unsigned char*         m_pBinormals;               // sets of 3 (m_nNumVertices * 3 total uint8s); compressed into [0,255] range
        const unsigned char*         m_pTangents;                // sets of 3 (m_nNumVertices * 3 total uint8s); compressed into [0,255] range
        const float*       m_pTexCoordsDiffuse;        // sets of 2 (m_nNumVertices * 2 total floats); compressed into [0,255] range
        const float*       m_pTexCoordsDetail;         // sets of 2 (m_nNumVertices * 2 total floats)
        const unsigned char*         m_pAmbientOcclusionValues;  // sets of 1 (m_nNumVertices * 1 total uint8s); compressed into [0,255] range
        float              m_fWindDataMagnitude;       // used to decompress m_pWindData.xyz values
        const unsigned char*         m_pWindData;                // sets of 5 (m_nNumVertices * 5 total floats)
        const float*       m_pFrondRipple;             // sets of 2 (m_nNumVertices * 2 total floats)
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SLeafCards

    struct ST_STORAGE_CLASS SLeafCards
    {
                                SLeafCards( );
                                ~SLeafCards( );

        bool                 HasGeometry(void) const;

        int                m_nNumMaterialGroups;
        const SDrawCallInfo*    m_pDrawCallInfo;            // m_nNumMaterialGroups number of elements
        int                m_nTotalNumCards;           // # of cards contained in all material groups in this struct

        const float*       m_pPositions;               // one (x, y, z) coords for each card, m_nNumCards * 3 total floats
        const float*       m_pDimensions;              // one (width, height) pair per card, m_nNumCards * 2 total floats
        const float*       m_pLodScales;               // one (start_scale, end_scale) pair per card, m_nNumCards * 2 total floats
        const float*       m_pPivotPoints;             // one (x, y) pair per card, m_nNumCards * 2 total floats
        const unsigned char*         m_pNormals;                 // four (x, y, z) coords for each card, m_nNumCards * 3 * 4 total uint8s; compressed into [0,255] range
        const unsigned char*         m_pBinormals;               // four (x, y, z) coords for each card, m_nNumCards * 3 * 4 total uint8s; compressed into [0,255] range
        const unsigned char*         m_pTangents;                // four (x, y, z) coords for each card, m_nNumCards * 3 * 4 total uint8s; compressed into [0,255] range
        const float*       m_pTexCoordsDiffuse;        // four (u, v) texcoords for each card, m_nNumCards * 2 * 4 total floats
        const unsigned char*         m_pAmbientOcclusionValues;  // one value for each card, m_nNumCards total uint8s; compressed into [0,255] range
        float              m_fWindDataMagnitude;       // used to decompress m_pWindData.xyz values
        const unsigned char*         m_pWindData;                // five values for each card, m_nNumCards * 5 total floats
        const float*       m_pLeafCardOffsets;         // four values for each card, m_nNumCards * 4 total floats
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SVerticalBillboards

    struct ST_STORAGE_CLASS SVerticalBillboards
    {
                            SVerticalBillboards( );
                            ~SVerticalBillboards( );

        int            m_nMaterialIndex;           // indexes SGeometry material table
        int            m_nNumBillboards;           // number of 360-degree billboards generated by Compiler app
        float          m_fWidth;                   // width of the billboard, governed by tree extents     
        float          m_fTopCoord;                // top-most point of the billboard, governed by tree height
        float          m_fBottomCoord;             // bottom-most point, can be below zero for trees with roots, etc.
        const float*   m_pTexCoords;               // four values per billboard, 4 * m_nNumBillboards total; each set of
                                                        // four values are (max_s, max_t, s_width, t_height)
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SHorizontalBillboard

    struct ST_STORAGE_CLASS SHorizontalBillboard
    {
                            SHorizontalBillboard( );
                            ~SHorizontalBillboard( );

        bool             m_bPresent;                 // true if an overhead billboard was exported using Compiler
        int            m_nMaterialIndex;           // indexes SGeometry material table
        noVec3                m_avCoords[4];              // four sets of (xyz) to render the overhead square
        float          m_afTexCoords[8];           // diffuse/normal texcoords for the billboard
        noVec4                m_vTexCoordsShader;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SCollisionObject

    struct ST_STORAGE_CLASS SCollisionObject
    {
                            SCollisionObject( );
                            ~SCollisionObject( );
                            
        enum 
        {
            STRING_SIZE = 256
        };

        char             m_szUserData[STRING_SIZE];
        noVec3                m_vCenter1;
        noVec3                m_vCenter2;
        float          m_fRadius;
    };

    
    ///////////////////////////////////////////////////////////////////////  
    //  Enumeration EGeometryType

    enum EGeometryType
    {
        GEOMETRY_TYPE_BRANCHES,
        GEOMETRY_TYPE_FRONDS,
        GEOMETRY_TYPE_LEAF_CARDS,
        GEOMETRY_TYPE_LEAF_MESHES,
        GEOMETRY_TYPE_NUM_3D_TYPES,
        GEOMETRY_TYPE_VERTICAL_BILLBOARDS = GEOMETRY_TYPE_NUM_3D_TYPES,
        GEOMETRY_TYPE_HORIZONTAL_BILLBOARDS,

        GEOMETRY_TYPE_COUNT
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SGeometry

    struct ST_STORAGE_CLASS SGeometry
    {
                                    SGeometry( );
                                    ~SGeometry( );

        void                        Clear(void);        // internal use

        // materials
        int                    m_nNumMaterials;
        SMaterial*                  m_pMaterials;       // shared by all of the geometry types

        // branches
        int                    m_nNumBranchLods;
        SIndexedTriangles*          m_pBranchLods;

        // fronds
        int                    m_nNumFrondLods;
        SIndexedTriangles*          m_pFrondLods;

        // leaf meshes
        int                    m_nNumLeafMeshLods;
        SIndexedTriangles*          m_pLeafMeshLods;

        // leaf cards
        int                    m_nNumLeafCardLods;
        SLeafCards*                 m_pLeafCardLods;

        // billboards
        SVerticalBillboards         m_sVertBBs;
        SHorizontalBillboard        m_sHorzBB;

        // internal
        SIndexedTriangles*          m_pCompositeIndexedLods;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SLodRange

    struct ST_STORAGE_CLASS SLodProfile
    {
                        SLodProfile( );

        bool         IsValid(void) const;
        void            ComputeDerived(void);
        void            Scale(float fScale);
        void            Square(SLodProfile& sSquaredProfile) const;

        float      m_fHighDetail3dDistance;        // distance at which LOD transition from highest 3D level begins
        float      m_fLowDetail3dDistance;         // distance at which the lowest 3D level is sustained
        float      m_fBillboardStartDistance;      // distance at which the billboard begins to fade in and 3D fade out
        float      m_fBillboardFinalDistance;      // distance at which the billboard is fully visible and 3D is completely gone

        // derived values
        float      m_f3dRange;
        float      m_fBillboardRange;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SLodSnapshot

    struct ST_STORAGE_CLASS SLodSnapshot
    {
                        SLodSnapshot( );

        // characters are used here as small signed integer values since the SLodSnapshot
        // structure is a member of every CInstance class in the Forest library; it's
        // important to keep them small

        // for each, 0 is highest LOD, (num_lods - 1) is lowest, and -1 is inactive
        char         m_nBranchLodIndex;
        char         m_nFrondLodIndex;
        char         m_nLeafCardLodIndex;
        char         m_nLeafMeshLodIndex;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CAllocatorInterface

    class ST_STORAGE_CLASS CAllocatorInterface
    {
    public:
                    CAllocatorInterface(CAllocator* pAllocator);
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CCore
        
    class ST_STORAGE_CLASS CCore
    {
    public:
    friend class CParser;

                                            CCore( ) {}
            virtual                         ~CCore( ) {}

            // loading
            bool                         LoadTree(const char* pFilename, float fScale = 1.0f);
            bool                         LoadTree(const unsigned char* pMemBlock, 
                                                     unsigned int uiNumBytes, 
                                                     bool bCopyBuffer, 
                                                     float fScale = 1.0f);
            const char*                     GetFilename(void) const;
    static  unsigned char*           ST_CALL_CONV LoadFileIntoBuffer(const char* pFilename, unsigned int& uiBufferSize);
    static  void               ST_CALL_CONV SetTextureFlip(bool bFlip);
    static  bool            ST_CALL_CONV GetTextureFlip(void);

            // geometry
            const SGeometry*                GetGeometry(void) const;
            bool                         HasGeometryType(EGeometryType eType) const;
    static  const char*                     GetGeometryTypeName(EGeometryType eType);
            const CExtents&                 GetExtents(void) const;

            // LOD
            const SLodProfile&              GetLodProfile(void) const;
            const SLodProfile&              GetLodProfileSquared(void) const;
            bool                         SetLodProfile(const SLodProfile& sLodProfile);
            void                            ComputeLodSnapshot(float fLod, SLodSnapshot& sProfile) const;
            float                      ComputeLodByDistance(float fDistance) const;
            float                      ComputeLodByDistanceSquared(float fDistance) const;
    static  float                      ComputeShaderLerp(float fLod, int nNumDiscreteLevels);

            // clip-space settings (part of the culling system)
    static  void               ST_CALL_CONV SetClipSpaceDepthRange(float fNear, float fFar); // opengl is (-1,1), directx is (0,1)
    static  void               ST_CALL_CONV GetClipSpaceDepthRange(float& fNear, float& fFar);

            // wind
            CWind&                          GetWind(void) { return m_cWind; }
            const CWind&                    GetWind(void) const { return m_cWind; }

            // collision
            const SCollisionObject*         GetCollisionObjects(int& nNumObjects) const;

            // memory management
            void                            DeleteGeometry(bool bKeepBillboardTexCoords);
    static  size_t             ST_CALL_CONV GetTotalHeapUsage(void);    // in bytes
    static  void               ST_CALL_CONV ShutDown(void);

            // error system
    static  void               ST_CALL_CONV SetError(const char* pError, ...);
    static  const char*        ST_CALL_CONV GetError(void);
    static  bool               ST_CALL_CONV IsRunTimeBigEndian(void);

            // licensing system
    static  bool            ST_CALL_CONV Authorize(const char* pKey);
    static  bool            ST_CALL_CONV IsAuthorized(void);
    static  const char*        ST_CALL_CONV Version(bool bShort = false);

            // utility
    static  noVec3               ST_CALL_CONV UncompressVec3(const unsigned char* pCompressedVector);
    static  float         ST_CALL_CONV UncompressScalar(unsigned char uiCompressedScalar);

    private:
                                            CCore(const CCore& cRight); // copying CCore disabled

            void                            InventoryGeometry(void);
            void                            ApplyScale(float fScale);

            CFixedString                    m_strFilename;
            unsigned char*                        m_pSrtBuffer;
            SGeometry                       m_sGeometry;
            SLodProfile                     m_sLodProfile;
            SLodProfile                     m_sLodProfileSquared;
            bool                         m_abGeometryTypesPresent[GEOMETRY_TYPE_COUNT];
            CWind                           m_cWind;
            CExtents                        m_cExtents;
            int                        m_nNumCollisionObjects;
            SCollisionObject*               m_pCollisionObjects;
            bool                         m_bOwnsSrtBuffer;
            bool                         m_bBillboardTexCoordsCopied;

            // internal use
    public:
            struct SSupportingData
            {
                                            SSupportingData( );

                // lod
                SLodProfile                 m_sLodProfile;
                int                    m_nNumBranchLods;
                int                    m_nNumFrondLods;
                int                    m_nNumLeafMeshLods;
                int                    m_nNumLeafCardLods;

                // collision objects
                enum
                {
                    MAX_NUM_COLLISION_OBJECTS = 20
                };
                int                    m_nNumCollisionObjects;
                SCollisionObject            m_asCollisionObjects[MAX_NUM_COLLISION_OBJECTS];

                // wind
                CWind::SParams              m_sWindParams;
            };

            bool                         PopulateSupportingDataBlock(SSupportingData& sData) const;
            bool                         ApplySupportingDataBlock(const SSupportingData& sData);
    };
    typedef CCore CTree;

    // include inline functions
    #include "Core.inl"

} // end namespace SpeedTree


///////////////////////////////////////////////////////////////////////  
//  Packing

#if defined(_WIN32) || defined(_XBOX)
    #pragma pack(pop)   
    #pragma warning(pop)    
#endif


#include "ExportEnd.h"
