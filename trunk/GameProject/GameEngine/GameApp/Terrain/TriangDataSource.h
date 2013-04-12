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

#include "DynamicQuadTreeNode.h"
#include "RQTTriangulation.h"
#include "HierarchyArray.h"

// Class implementing triangulation data source
class CTriangDataSource
{
public:
    CTriangDataSource(void);
    ~CTriangDataSource(void);

    // Init empty triangulation data source object
    void Init(int iNumLevelsInHierarchy, int iPatchSize, float fFinestLevelTriangErrorThreshold);

    // Decodes child patch triangulations taking parent patch triangulation as input
    CRQTTriangulation* DecodeTriangulation(const SQuadTreeNodeLocation &pos);

    // Encodes the triangulation
    void EncodeTriangulation(const SQuadTreeNodeLocation &pos,
                             CRQTTriangulation &Triangulation,
                             float fTriangulationErrorBound);

    // Returns size of the encoded trinagulation for the specified quad tree node
    size_t GetEncodedTriangulationsSize(const struct SQuadTreeNodeLocation &pos);

    int GetNumLevelsInHierarchy();

    // Gets number of levels in single patch quad tree
    // If patch size is 128x128, then its quad tree consists of 8 level
    int GetNumLevelsInLocalPatchQT(){return m_iNumLevelsInPatchQuadTree;}

    int GetPatchSize();

    // Get world space triangulation error bound for a specified quad tree node
    float GetTriangulationErrorBound(const SQuadTreeNodeLocation &pos);

    // Returns world space error bound of a finest level trinagulations
    float GetFinestLevelTriangErrorThreshold();

    // Saves the data to file
    HRESULT SaveToFile(LPCTSTR FilePath);
    // Loads the data from file
    HRESULT LoadFromFile(LPCTSTR FilePath);

    // Builds adaptive triangulation for the specified patch
    CRQTTriangulation* CreateAdaptiveTriangulation(class CPatchElevationData *pElevData,
                                     class CRQTTriangulation *pLBChildTriangulation,
                                     class CRQTTriangulation *pRBChildTriangulation,
                                     class CRQTTriangulation *pLTChildTriangulation,
                                     class CRQTTriangulation *pRTChildTriangulation,
                                     float fTriangulationErrorThreshold,
                                     float &fTriangulationError,
                                     UINT &uiNumTriangles);

private:
    int m_iNumLevelsInHierarchy, m_iNumLevelsInPatchQuadTree;
    float m_fFinestLevelTriangErrorThreshold;

    struct SRQTTriangInfo
    {
        CBitStream m_EncodedRQTEnabledFlags;
        float fTriangulationErrorBound;
        SRQTTriangInfo() : fTriangulationErrorBound(0){}
        size_t GetDataSize(){return (m_EncodedRQTEnabledFlags.GetBitStreamSizeInBits() + 7)/8;}
    };

    HierarchyArray<SRQTTriangInfo> m_AdaptiveTriangInfo; 
};
