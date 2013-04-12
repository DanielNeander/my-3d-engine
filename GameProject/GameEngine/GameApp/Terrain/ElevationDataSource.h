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

#include <vector>
#include "HierarchyArray.h"
#include "DynamicQuadTreeNode.h"

// Class that stores height map data for the particular quad tree node
class CPatchElevationData
{
public:
    CPatchElevationData(const class CElevationDataSource *pDataSource,
                        const SQuadTreeNodeLocation &pos, // Location in the quad tree
                        int iLeftBndrExt,  // Height map extensions
                        int iBottomBndrExt,
                        int iRightBndrExt,
                        int iTopBndrExt,
                        int iHighResDataLODBias);
    ~CPatchElevationData(void);

    int GetPatchSize()const;

    // Gets data pointer to the stored height map
    void GetDataPtr( const UINT16* &pDataPtr, 
                     size_t &Pitch, 
                     int LeftBoundaryExtension,
                     int BottomBoundaryExtension,
                     int RightBoundaryExtension,
                     int TopBoundaryExtension )const;

    // Gets data pointer to the stored height map
    void GetHighResDataPtr( const UINT16* &pHighResDataPtr, 
                            size_t &HighResDataPitch, 
                            int LeftBoundaryExtension,
                            int BottomBoundaryExtension,
                            int RightBoundaryExtension,
                            int TopBoundaryExtension )const;

    void GetPos( SQuadTreeNodeLocation &outPos )const;
    
    int GetHighResDataLODBias()const{return m_iHighResDataLODBias;}

    // Returns this patch approximation error bound
    UINT16 GetApproximationErrorBound()const;

	

private:
    friend class CElevationDataSource;
    SQuadTreeNodeLocation m_pos; // Position in the quad tree
    std::vector<UINT16> m_HeightMap; // The height map itself
    std::vector<UINT16> m_HighResHeightMap; // The higher resolution height map used to generate normal map
    size_t m_Pitch;
    size_t m_HighResDataPitch;
    int m_iLeftBndrExt; // Height map boundary extension widths
    int m_iBottomBndrExt;
    int m_iRightBndrExt;
    int m_iTopBndrExt;
    int m_iHighResDataLODBias;
    int m_iPatchSize;
    UINT16 m_ErrorBound;

    
    CPatchElevationData();
};

// Class implementing elevation data source
class CElevationDataSource
{
public:
    // Creates data source from the specified raw data file
    CElevationDataSource(LPCWSTR strSrcDemFile,
                         int iPatchSize);
    virtual ~CElevationDataSource(void);

    // Creates object storing height map for the specified patch
    CPatchElevationData* GetElevData(const struct SQuadTreeNodeLocation &Pos)const;

    // Returns minimal and maximal heights of the patch
    void GetPatchMinMaxElevation(const SQuadTreeNodeLocation &pos,
                                 UINT16 &MinElevation, 
                                 UINT16 &MaxElevation)const;
    
    // Returns minimal height of the whole terrain
    UINT16 GetGlobalMinElevation()const;

    // Returns maximal height of the whole terrain
    UINT16 GetGlobalMaxElevation()const;
    
    // Returns patch height map world space error bound
    UINT16 GetPatchElevDataErrorBound(const SQuadTreeNodeLocation &pos)const;

    int GetNumLevelsInHierarchy()const;

    int GetPatchSize()const;
    
    // Sets required boundary extension widths. If requested extension will be
    // larger than what has been set by this method, then additional memory
    // will be required to store the requested height map
    void SetRequiredElevDataBoundaryExtensions(int iRequiredLeftBoundaryExt,
                                               int iRequiredBottomBoundaryExt,
                                               int iRequiredRightBoundaryExt,
                                               int iRequiredTopBoundaryExt);
    // Sets the LOD bias for the higher resolution height map, which is used to
    // calculate the normal map
    void SetHighResDataLODBias(int iHighResDataLODBias);

    enum
    { 
        LB_DATA_EXTENSION_WIDTH = 1, // Left and bottom height map extensions of the stored data
        RT_DATA_EXTENSION_WIDTH = 2  // Right and top height map extensions of the stored data
    };

	static int m_iWidth, m_iHeight;
	static unsigned int m_iNumCols, m_iNumRows;


private:
    CElevationDataSource();

    // Calculates min/max elevations for all patches in the tree
    void CalculateMinMaxElevations();

    // Calculates world space approximation error bounds for all patches in the tree
    void CalculatePatchErrorBounds();

    // Copies height map to the specified memory location
    // using quad tree node location as input
    void FillPatchHeightMap(const SQuadTreeNodeLocation &pos,
                            UINT16 *pDataPtr,
                            size_t DataPitch,
                            int iLeftExt = 0,
                            int iBottomExt = 0,
                            int iRightExt = 0,
                            int iTopExt = 0,
                            int iLODBias = 0)const;
    
    // Copies height map to the specified memory location
    // using start/end columns and rows as input
    void FillPatchHeightMap(UINT16 *pDataPtr,
                            size_t DataPitch,
                            int iStartCol, int iEndCol, 
                            int iStartRow, int iEndRow, 
                            int iStep)const;

    // Hierarchy array storing minimal and maximal heights for quad tree nodes
    HierarchyArray< std::pair<UINT16, UINT16> > m_MinMaxElevation;
    // Hierarchy array storing world space approximation error bounds for quad tree nodes
    HierarchyArray< UINT16 > m_ErrorBounds;

    int m_iNumLevels;
    int m_iPatchSize;

    // The whole terrain height map
    std::vector<UINT16> m_TheHeightMap;

    int m_iRequiredLeftBoundaryExt;
    int m_iRequiredBottomBoundaryExt;
    int m_iRequiredRightBoundaryExt;
    int m_iRequiredTopBoundaryExt;
    int m_iHighResDataLODBias;
};
