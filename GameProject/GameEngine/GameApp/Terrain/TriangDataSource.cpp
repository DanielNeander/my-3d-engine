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
#include "TriangDataSource.h"
#include "RQTTriangulation.h"
#include "ElevationDataSource.h"

CTriangDataSource::CTriangDataSource(void) :
    m_iNumLevelsInHierarchy(0), 
    m_iNumLevelsInPatchQuadTree(0)
{
}

CTriangDataSource::~CTriangDataSource(void)
{
}

// Init empty triangulation data source object
void CTriangDataSource::Init(int iNumLevelsInHierarchy, int iPatchSize, float fFinestLevelTriangErrorThreshold)
{
    if( iPatchSize & (iPatchSize-1) )
    {
        CHECK_HR(E_FAIL, _T("Patch size (%d) must be power of 2"), iPatchSize );
        return;
    }

    m_iNumLevelsInHierarchy = iNumLevelsInHierarchy;
    m_fFinestLevelTriangErrorThreshold = fFinestLevelTriangErrorThreshold;

    m_iNumLevelsInPatchQuadTree = 0;
    while( (1 << (m_iNumLevelsInPatchQuadTree-1)) < iPatchSize)
        m_iNumLevelsInPatchQuadTree++;

    m_AdaptiveTriangInfo.Resize( m_iNumLevelsInHierarchy );

	// todo: check if this loop is redundant
    for( HierarchyIterator it(m_iNumLevelsInHierarchy); it.IsValid(); it.Next() )
    {
        m_AdaptiveTriangInfo[it].fTriangulationErrorBound = 0;
    }
}


int CTriangDataSource::GetNumLevelsInHierarchy()
{
    return m_iNumLevelsInHierarchy;
}

int CTriangDataSource::GetPatchSize()
{
    return 1 << (m_iNumLevelsInPatchQuadTree-1);
}

float CTriangDataSource::GetTriangulationErrorBound(const SQuadTreeNodeLocation &pos)
{
    return  m_AdaptiveTriangInfo[pos].fTriangulationErrorBound;
}

float CTriangDataSource::GetFinestLevelTriangErrorThreshold()
{
    return m_fFinestLevelTriangErrorThreshold;
}

// Decodes child patch triangulations taking parent patch triangulation as input
CRQTTriangulation* CTriangDataSource::DecodeTriangulation(const SQuadTreeNodeLocation &pos)
{
    // This method does not build adaptive triangulation list
    // It is done by CRQTTriangulation::RecursiveGenerateIndices()
    SQuadTreeNodeLocation parentPos(0,0,0);
    int iNumLevelsInHierarchy = GetNumLevelsInHierarchy();
    int iPatchSize = GetPatchSize();
    CBitStream *pEncodedEnabledFlags = &m_AdaptiveTriangInfo[pos].m_EncodedRQTEnabledFlags;
    CRQTTriangulation* pTriangulation = new CRQTTriangulation( pos, iNumLevelsInHierarchy, pEncodedEnabledFlags, iPatchSize);
    return pTriangulation;
}

// Encodes the triangulation
void CTriangDataSource::EncodeTriangulation(const SQuadTreeNodeLocation &pos,
                                            CRQTTriangulation &Triangulation,
                                            float fTriangulationErrorBound)
{
    SRQTTriangInfo &TriangInfo = m_AdaptiveTriangInfo[pos];

    Triangulation.SetEncodingMode( &TriangInfo.m_EncodedRQTEnabledFlags );

    UINT uiNumIndices;
    // Invoke GenerateIndices() to encode the bit stream
    Triangulation.GenerateIndices(0, NULL, uiNumIndices);
    
    TriangInfo.fTriangulationErrorBound = fTriangulationErrorBound;
}

// Saves the data to file
HRESULT CTriangDataSource::SaveToFile(LPCTSTR FilePath)
{
    FILE *pFile = NULL;
    if( _tfopen_s( &pFile, FilePath, _T("wb") ) != 0 )
    {
        //LOG_ERROR( L"Failed to open triangulation file for writing (%s)", FilePath );
    }

    fwrite(&m_iNumLevelsInHierarchy, sizeof(m_iNumLevelsInHierarchy), 1, pFile);
    fwrite(&m_iNumLevelsInPatchQuadTree, sizeof(m_iNumLevelsInPatchQuadTree), 1, pFile);
    fwrite(&m_fFinestLevelTriangErrorThreshold, sizeof(m_fFinestLevelTriangErrorThreshold), 1, pFile);

    for( HierarchyIterator it(m_iNumLevelsInHierarchy); it.IsValid(); it.Next() )
    {
        fwrite(&m_AdaptiveTriangInfo[it].fTriangulationErrorBound, sizeof(float), 1, pFile);
        if( it.Level() > 0 )
            m_AdaptiveTriangInfo[it].m_EncodedRQTEnabledFlags.SaveToFile(pFile);
    }

    fclose(pFile);

    return S_OK;
}

// Loads the data from file
HRESULT CTriangDataSource::LoadFromFile(LPCTSTR FilePath)
{
    FILE *pFile = NULL;
    if( _tfopen_s( &pFile, FilePath, _T("rb") ) != 0 )
    {
        fclose(pFile);
        CHECK_HR_RET(E_FAIL, "Failed to open triangulation file (%s)", FilePath );
    }

    HRESULT hr;
    int iNumLevelsInHierarchy, iNumLevelsInPatchQuadTree;
    size_t ItemsRead = fread(&iNumLevelsInHierarchy, sizeof(iNumLevelsInHierarchy), 1, pFile);
    if( ItemsRead != 1 )
    {
        fclose(pFile);
        CHECK_HR_RET(E_FAIL, _T("Failed to read num levels in hierarchy") );
    }

    ItemsRead = fread(&iNumLevelsInPatchQuadTree, sizeof(iNumLevelsInPatchQuadTree), 1, pFile);
    if( ItemsRead != 1 )
    {
        fclose(pFile);
        CHECK_HR_RET(E_FAIL, _T("Failed to read num levels in local patch QT") );
    }

    float fFinestLevelTriangErrorThreshold;
    ItemsRead = fread(&fFinestLevelTriangErrorThreshold, sizeof(fFinestLevelTriangErrorThreshold), 1, pFile);
    if( ItemsRead != 1 )
    {
        fclose(pFile);
        CHECK_HR_RET(E_FAIL, _T("Failed to read num Finest Level Triang Error Threshold") );
    }

    Init(iNumLevelsInHierarchy, 1 << (iNumLevelsInPatchQuadTree-1), fFinestLevelTriangErrorThreshold);

    for( HierarchyIterator it(m_iNumLevelsInHierarchy); it.IsValid(); it.Next() )
    {
        ItemsRead = fread(&m_AdaptiveTriangInfo[it].fTriangulationErrorBound, sizeof(float), 1, pFile);
        if( ItemsRead != 1 )
        {
            fclose(pFile);
            CHECK_HR_RET(E_FAIL, _T("Failed to read triangulation error bound") );
        }

        if( it.Level() > 0 )
        {
            try{m_AdaptiveTriangInfo[it].m_EncodedRQTEnabledFlags.LoadFromFile(pFile); hr = S_OK;}
            catch(const std::exception&){hr = E_FAIL;fclose(pFile);}
            CHECK_HR_RET(hr, _T("Failed to read enabled flags for patch (%d, %d) at level %d"), it.Horz(), it.Vert(), it.Level());
        }
    }

    fclose(pFile);

    return S_OK;
}

size_t CTriangDataSource::GetEncodedTriangulationsSize(const struct SQuadTreeNodeLocation &pos)
{
    return m_AdaptiveTriangInfo[pos].GetDataSize();
}




// This function calculates maximum world space error of the triangle specified by
// puiTriangleVertPackedIndices[]
// It goes through all height map samples covered by the triangle and
// calculates maximum vertical distance from the sample to the triangle surface
static
float GetTriangleWorldSpaceError(UINT puiTriangleVertPackedIndices[3],
                                 const UINT16 *pElevData,  size_t ElevDataPitch,
                                 int iPackedIndicesBoundaryExtension,
                                 float fErrorThreshold = +FLT_MAX)
{
    struct SUnpackedVertex
    {
        int iXInd, iYInd;
    }UnpackedVertices[3] = { {INT_MIN, INT_MIN}, {INT_MIN, INT_MIN}, {INT_MIN, INT_MIN} };

    // Triangle vertices
    D3DXVECTOR3 TriangleVertices[3];
    for(int iVert=0; iVert < 3; iVert++)
    {
        UINT uiPackedVertInd = puiTriangleVertPackedIndices[iVert];
        int iXInd, iYInd;
        UnpackIndices(uiPackedVertInd, iXInd, iYInd, iPackedIndicesBoundaryExtension);
        UnpackedVertices[iVert].iXInd = iXInd;
        UnpackedVertices[iVert].iYInd = iYInd;
        TriangleVertices[iVert] = D3DXVECTOR3( (float)iXInd, (float)iYInd, (float)pElevData[iXInd + iYInd*ElevDataPitch] );
    }

    // If some two vertices of the triangle are the same, it is degenerate so do nothing
    if( UnpackedVertices[0].iXInd == UnpackedVertices[1].iXInd && UnpackedVertices[0].iYInd == UnpackedVertices[1].iYInd ||  
        UnpackedVertices[0].iXInd == UnpackedVertices[2].iXInd && UnpackedVertices[0].iYInd == UnpackedVertices[2].iYInd ||
        UnpackedVertices[1].iXInd == UnpackedVertices[2].iXInd && UnpackedVertices[1].iYInd == UnpackedVertices[2].iYInd )
       return 0.f;

    // Calculate triangle area. It will be required to calculate barycentric coordinates
    D3DXVECTOR2 Rib0( TriangleVertices[1].x-TriangleVertices[0].x, TriangleVertices[1].y-TriangleVertices[0].y );
    D3DXVECTOR2 Rib1( TriangleVertices[2].x-TriangleVertices[0].x, TriangleVertices[2].y-TriangleVertices[0].y );
    float fTriangleDoubledArea = fabsf( Rib0.x * Rib1.y - Rib0.y * Rib1.x );
    // Zero-area triangle can't cover any vertices, so return 0
    if( fTriangleDoubledArea < 1e-5f )
        return 0.f;

#ifdef _DEBUG
    // The normal will be required to verify that point lies in the triangle plane
    D3DXVECTOR3 TriangleNormal;
    D3DXVECTOR3 Rib0_3d(Rib0.x, Rib0.y, TriangleVertices[1].z-TriangleVertices[0].z);
    D3DXVECTOR3 Rib1_3d(Rib1.x, Rib1.y, TriangleVertices[2].z-TriangleVertices[0].z);
    D3DXVec3Cross(&TriangleNormal, &Rib0_3d, &Rib1_3d ); 
    D3DXVec3Normalize( &TriangleNormal, &TriangleNormal );
#endif
    
    // Order vetices in the following way:
    // pOrderedVertices[0]->iYInd <= pOrderedVertices[1]->iYInd <= pOrderedVertices[2]->iYInd
    SUnpackedVertex *pOrderedVertices[3] = { &UnpackedVertices[0], &UnpackedVertices[1], &UnpackedVertices[2] };

    if( pOrderedVertices[0]->iYInd > pOrderedVertices[1]->iYInd )
        std::swap( pOrderedVertices[0], pOrderedVertices[1] );
        
    if( pOrderedVertices[1]->iYInd > pOrderedVertices[2]->iYInd )
    {
        std::swap( pOrderedVertices[1], pOrderedVertices[2] );
        
        if( pOrderedVertices[0]->iYInd > pOrderedVertices[1]->iYInd )
            std::swap( pOrderedVertices[0], pOrderedVertices[1] );
    }
    
    // Get column and row for ordered vertices
    float fCol0 = (float)pOrderedVertices[0]->iXInd;
    float fRow0 = (float)pOrderedVertices[0]->iYInd;
    float fCol1 = (float)pOrderedVertices[1]->iXInd;
    float fRow1 = (float)pOrderedVertices[1]->iYInd;
    float fCol2 = (float)pOrderedVertices[2]->iXInd;
    float fRow2 = (float)pOrderedVertices[2]->iYInd;

    // Degenerate triangles can not be met
    assert(fRow0 <= fRow1 && fRow1 <= fRow2 && fRow0 < fRow2);

    int iStartRow        = pOrderedVertices[0]->iYInd;
    int iIntermediateRow = pOrderedVertices[1]->iYInd;
    int iEndRow          = pOrderedVertices[2]->iYInd;
    
    if(iStartRow >= iEndRow) 
        return 0;

    // No go through all rows covered by the triangle:
    float fMaxError = 0;
    for(int iRow = iStartRow; iRow <= iEndRow; iRow++)
    {
        // Calculate start/end column on the ribs
        //
        //  iStartCol  iEndCol    
        //         . V2.        iEndRow 
        //         .  /|
        //         . / |
        //          /  |
        //         * * * <--iRow 
        //        /    |
        //     V1/     |        iIntermediateRow
        //       \     |
        //        \    |
        //         \   |
        //          \  |
        //           \ |
        //            \|        iStartRow
        //            V0
        // Note that fRow0 <= fRow1 <= fRow2, thus
        // start column always lies on rib [V0,V2]
        float fStartCol = fCol0 + (fCol2 - fCol0) * ( ((float)iRow) - fRow0) / (fRow2 - fRow0);
        float fEndCol;
        
        // End column
        if(iRow > iIntermediateRow)
        {
            // Current end vertex lies on rib [V1,V2]
            assert( fRow2 > fRow1 );
            fEndCol = fCol1 + (fCol2 - fCol1) * ( ((float)iRow) - fRow1) / (fRow2 - fRow1);
        }
        else
        {
            // Current end vertex lies on rib [V0,V1]
            if( fRow1 > fRow0 )
                fEndCol = fCol0 + (fCol1 - fCol0) * ( ((float)iRow) - fRow0) / (fRow1 - fRow0);
            else
                fEndCol = fCol1;
        }
 
        // Assure fStartCol <= fEndCol
        if(fStartCol > fEndCol)
            std::swap( fStartCol, fEndCol );
        
        // Get integer column numbers
        int iStartCol, iEndCol;
        iStartCol = (int)floorf( fStartCol);
        if(fStartCol > (float)iStartCol)
            iStartCol++;
        iEndCol = (int)floorf(fEndCol);

        // Go through all samples in the row covered by the triangle:
        for(int iCol = iStartCol; iCol <= iEndCol; iCol++)
        {
            // Coordinates of the current covered sample:
            D3DXVECTOR3 CoveredVert( (float)iCol, (float)iRow, (float)pElevData[iCol + iRow*ElevDataPitch] );
#ifdef _DEBUG
            // Verify vertex is in triangle coverage
            D3DXVECTOR3 Cross[3];
            for(int iTriangleVert = 0; iTriangleVert < 3; iTriangleVert++)
            {
                D3DXVECTOR3 Rib = TriangleVertices[ (iTriangleVert<2) ? (iTriangleVert+1) : 0 ] - TriangleVertices[ iTriangleVert ];
                Rib.z = 0;
                D3DXVECTOR3 Dir = CoveredVert - TriangleVertices[ iTriangleVert ];
                Dir.z = 0;
                D3DXVec3Cross( &Cross[iTriangleVert], &Rib, &Dir );
            }
            assert( Cross[0].z >= 0.f && Cross[1].z >= 0.f && Cross[2].z >= 0.f ||
                    Cross[0].z <= 0.f && Cross[1].z <= 0.f && Cross[2].z <= 0.f );
#endif
            float fCurrError = 0.f;
            // Compute directions from each triangle vertex to the current sample in XY plane:
            D3DXVECTOR2 Dir0( CoveredVert.x-TriangleVertices[0].x, CoveredVert.y-TriangleVertices[0].y );
            D3DXVECTOR2 Dir1( CoveredVert.x-TriangleVertices[1].x, CoveredVert.y-TriangleVertices[1].y );
            D3DXVECTOR2 Dir2( CoveredVert.x-TriangleVertices[2].x, CoveredVert.y-TriangleVertices[2].y );

            //v.z = pV1->x * pV2->y - pV1->y * pV2->x;
            
            // Compute baricentric coordinates of the current vertex in triangle XY projection
            //
            //      W
            //      /\ 
            //     /  \  
            //    / *  \ 
            //   /______\ 
            //  U        V
            //  For instance, U coordinate is the fraction of the triangle area bounded by Dir1 and Dir2:
            float fBaricentricU = fabsf( Dir1.x * Dir2.y - Dir1.y * Dir2.x ) / fTriangleDoubledArea;
            float fBaricentricV = fabsf( Dir0.x * Dir2.y - Dir0.y * Dir2.x ) / fTriangleDoubledArea;
            float fBaricentricW = fabsf( Dir0.x * Dir1.y - Dir0.y * Dir1.x ) / fTriangleDoubledArea;
            assert( (fBaricentricU + fBaricentricV + fBaricentricW) >= 0.99999f &&
                    (fBaricentricU + fBaricentricV + fBaricentricW) <= 1.00001f );

            // Get triangle z value corresponding to the sample location
            float fTriangleZ = fBaricentricU * TriangleVertices[0].z + fBaricentricV * TriangleVertices[1].z + fBaricentricW * TriangleVertices[2].z;

#ifdef _DEBUG
            // Verify that point lies in the triangle plane
            D3DXVECTOR3 PointInPlane(CoveredVert.x, CoveredVert.y, fTriangleZ);
            D3DXVECTOR3 DirOnPoint = PointInPlane - TriangleVertices[0];
            float DotProduct = fabsf( D3DXVec3Dot(&TriangleNormal, &DirOnPoint) );
            assert( DotProduct < 1e-3f );
#endif
            // Calculate vertical distance from the sample to the trinagle plane:
            fCurrError = fabsf( CoveredVert.z - fTriangleZ );

            // If current error is greater than the threshold, we do not need
            // to continue since it is alredy clear that this triangle must be split:
            if( fCurrError >= fErrorThreshold )
                return fCurrError;

            fMaxError = max(fMaxError, fCurrError);
        }
    }

    return fMaxError;
}

// This function calculates maximum world space error of the triangulation specified by
// puiIndices[]. It goes through all triangles and determines maximum world space error
static
float CalculateTriangulationError(UINT *puiIndices,
                                  UINT uiNumTriangles,
                                  int iPatchSize,
                                  const UINT16 *pElevData, size_t ElevDataPitch,
                                  int iPackedIndicesBoundaryExtension)
{
    float fTriangulationError = 0;
    for(UINT uiTriangleNum = 0; uiTriangleNum < uiNumTriangles; uiTriangleNum++)
    {
        UINT *puiTriangleVertPackedIndices = puiIndices + uiTriangleNum*3;
        
        bool bIsFlangeTriangle = false;
        for(int iVert = 0; iVert < 3; iVert++)
        {
            int iVertXInd, iVertYInd;
            UnpackIndices( puiTriangleVertPackedIndices[iVert], iVertXInd, iVertYInd, 1);
            if( iVertXInd < 0 || iVertYInd < 0 || iVertXInd > iPatchSize || iVertYInd > iPatchSize )
            {
                bIsFlangeTriangle = true;
                break;
            }
        }
        // Ommit flange vertices
        if( bIsFlangeTriangle )
            continue;
        float fCurrTriangleWorldSpaceError =
            GetTriangleWorldSpaceError(puiTriangleVertPackedIndices,
                                       pElevData, ElevDataPitch,
                                       iPackedIndicesBoundaryExtension);

        fTriangulationError = max(fTriangulationError, fCurrTriangleWorldSpaceError);
    }

    return  fTriangulationError;
}

// Builds adaptive triangulation for the specified patch
CRQTTriangulation* CTriangDataSource :: CreateAdaptiveTriangulation(class CPatchElevationData *pElevData,
                                                      class CRQTTriangulation* /* pLBChildTriangulation */,
                                                      class CRQTTriangulation* /* pRBChildTriangulation */,
                                                      class CRQTTriangulation* /* pLTChildTriangulation */,
                                                      class CRQTTriangulation* /* pRTChildTriangulation */,
                                                      float fTriangulationErrorThreshold,
                                                      float &fTriangulationError,
                                                      UINT &uiNumTriangles)
{
    CRQTVertsEnabledFlags EnabledFlags;

    int iPatchSize = pElevData->GetPatchSize();
    int iNumLevelsInLocalPatchQT = 0;
    while( (1 << iNumLevelsInLocalPatchQT) <= iPatchSize )iNumLevelsInLocalPatchQT++;

    const int iPackedIndicesBoundaryExtension = 1;

    EnabledFlags.Init(iPatchSize);

    const UINT16 *ElevData;
    size_t ElevDataPitch;
    pElevData->GetDataPtr( ElevData, ElevDataPitch, 0, 0, 1, 1);
    
    for(int iLevel = iNumLevelsInLocalPatchQT-1; iLevel > 0; iLevel--)
    {
        int iLevelStep = 1 << ((iNumLevelsInLocalPatchQT-1) - iLevel);
        int iNextFinerLevelStep = iLevelStep/2;

        //process non-center vertices on even rows
        for(int iY = 0; iY <= iPatchSize; iY += iLevelStep*2 )
            for(int iX = iLevelStep; iX <= iPatchSize; iX += iLevelStep*2 )
            {
                char bEnableVertex = FALSE;
                // Check dependency
                if( iNextFinerLevelStep > 0 )
                {
                    if( iY > 0 )
                        bEnableVertex |= EnabledFlags.IsVertexEnabled(iX - iNextFinerLevelStep, iY - iNextFinerLevelStep) ||
                                         EnabledFlags.IsVertexEnabled(iX + iNextFinerLevelStep, iY - iNextFinerLevelStep);

                    if( iY < iPatchSize )
                        bEnableVertex |= EnabledFlags.IsVertexEnabled(iX - iNextFinerLevelStep, iY + iNextFinerLevelStep) ||
                                         EnabledFlags.IsVertexEnabled(iX + iNextFinerLevelStep, iY + iNextFinerLevelStep);
                }

                if( !bEnableVertex && iY < iPatchSize )
                {
                    UINT puiTriangleVertPackedIndices[3] = 
                    {
                        CalculatePackedIndex(iX-iLevelStep, iY, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX+iLevelStep, iY, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX, iY+iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension)
                    };
                    float fCurrTriangleWorldSpaceError =
                        GetTriangleWorldSpaceError(puiTriangleVertPackedIndices,
                                                   &ElevData[0], ElevDataPitch,
                                                   iPackedIndicesBoundaryExtension,
                                                   fTriangulationErrorThreshold);
                    // If the threshold is exceeded, enable vertex
                    if( fCurrTriangleWorldSpaceError >= fTriangulationErrorThreshold )
                        bEnableVertex = true;
                }   
                
                if( !bEnableVertex && iY > 0 )
                {
                    UINT puiTriangleVertPackedIndices[3] = 
                    {
                        CalculatePackedIndex(iX-iLevelStep, iY, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX, iY-iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX+iLevelStep, iY, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension)
                    };
                    float fCurrTriangleWorldSpaceError =
                        GetTriangleWorldSpaceError(puiTriangleVertPackedIndices,
                                                   &ElevData[0], ElevDataPitch,
                                                   iPackedIndicesBoundaryExtension,
                                                   fTriangulationErrorThreshold);
                    // If the threshold is exceeded, enable vertex
                    if( fCurrTriangleWorldSpaceError >= fTriangulationErrorThreshold )
                        bEnableVertex = true;
                }   

                EnabledFlags.SetVertexEnabledFlag(iX, iY, bEnableVertex);
            }
        
        //process non-center vertices on odd rows
        for(int iY = iLevelStep; iY <= iPatchSize; iY += iLevelStep*2 )
            for(int iX = 0; iX <= iPatchSize; iX += iLevelStep*2 )
            {
                char bEnableVertex = FALSE;
                // Check dependency
                if( iNextFinerLevelStep > 0 )
                {
                    if( iX > 0 )
                        bEnableVertex |= EnabledFlags.IsVertexEnabled(iX - iNextFinerLevelStep, iY - iNextFinerLevelStep) ||
                                         EnabledFlags.IsVertexEnabled(iX - iNextFinerLevelStep, iY + iNextFinerLevelStep);

                    if( iX < iPatchSize )
                        bEnableVertex |= EnabledFlags.IsVertexEnabled(iX + iNextFinerLevelStep, iY - iNextFinerLevelStep) ||
                                         EnabledFlags.IsVertexEnabled(iX + iNextFinerLevelStep, iY + iNextFinerLevelStep);
                }

                if( !bEnableVertex && iX < iPatchSize )
                {
                    UINT puiTriangleVertPackedIndices[3] = 
                    {
                        CalculatePackedIndex(iX, iY-iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX, iY+iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX+iLevelStep, iY, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension)
                    };
                    float fCurrTriangleWorldSpaceError =
                        GetTriangleWorldSpaceError(puiTriangleVertPackedIndices,
                                                   &ElevData[0], ElevDataPitch,
                                                   iPackedIndicesBoundaryExtension,
                                                   fTriangulationErrorThreshold);
                    // If the threshold is exceeded, enable vertex
                    if( fCurrTriangleWorldSpaceError >= fTriangulationErrorThreshold )
                        bEnableVertex = true;
                }   
                
                if( !bEnableVertex && iX > 0 )
                {
                    UINT puiTriangleVertPackedIndices[3] = 
                    {
                        CalculatePackedIndex(iX, iY-iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX, iY+iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX-iLevelStep, iY, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension)
                    };
                    float fCurrTriangleWorldSpaceError =
                        GetTriangleWorldSpaceError(puiTriangleVertPackedIndices,
                                                   &ElevData[0], ElevDataPitch,
                                                   iPackedIndicesBoundaryExtension,
                                                   fTriangulationErrorThreshold);
                    // If the threshold is exceeded, enable vertex
                    if( fCurrTriangleWorldSpaceError >= fTriangulationErrorThreshold )
                        bEnableVertex = true;
                }   

                EnabledFlags.SetVertexEnabledFlag(iX, iY, bEnableVertex);
            }


        //process center vertices of the level
        for(int iY = iLevelStep; iY <= iPatchSize; iY += iLevelStep*2 )
            for(int iX = iLevelStep; iX <= iPatchSize; iX += iLevelStep*2 )
            {
                char bEnableVertex = FALSE;
                // Check dependency
                bEnableVertex |= EnabledFlags.IsVertexEnabled(iX - iLevelStep, iY) ||
                                 EnabledFlags.IsVertexEnabled(iX + iLevelStep, iY) ||
                                 EnabledFlags.IsVertexEnabled(iX, iY - iLevelStep) ||
                                 EnabledFlags.IsVertexEnabled(iX, iY + iLevelStep);
                
                bool bLTtoRBOrientation = ( (((iX-iLevelStep) / (2*iLevelStep)) & 0x01) +
                                            (((iY-iLevelStep) / (2*iLevelStep)) & 0x01) ) & 0x01 ? true : false;

                if( !bEnableVertex )
                {
                    UINT puiTriangleVertPackedIndices[6] = 
                    {
                        CalculatePackedIndex(iX-iLevelStep, iY-iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX+iLevelStep, iY+iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX-iLevelStep, iY+iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),

                        CalculatePackedIndex(iX-iLevelStep, iY+iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX+iLevelStep, iY-iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX-iLevelStep, iY-iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                    };
                    float fCurrTriangleWorldSpaceError =
                        GetTriangleWorldSpaceError(puiTriangleVertPackedIndices + (bLTtoRBOrientation ? 3 : 0),
                                                   &ElevData[0], ElevDataPitch,
                                                   iPackedIndicesBoundaryExtension,
                                                   fTriangulationErrorThreshold);
                    // If the threshold is exceeded, enable vertex
                    if( fCurrTriangleWorldSpaceError >= fTriangulationErrorThreshold )
                        bEnableVertex = true;
                }   
                
                if( !bEnableVertex )
                {
                    UINT puiTriangleVertPackedIndices[6] = 
                    {
                        CalculatePackedIndex(iX-iLevelStep, iY-iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX+iLevelStep, iY+iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX+iLevelStep, iY-iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),

                        CalculatePackedIndex(iX-iLevelStep, iY+iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX+iLevelStep, iY-iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                        CalculatePackedIndex(iX+iLevelStep, iY+iLevelStep, iNumLevelsInLocalPatchQT-1, iNumLevelsInLocalPatchQT, iPackedIndicesBoundaryExtension),
                    };
                    float fCurrTriangleWorldSpaceError =
                        GetTriangleWorldSpaceError(puiTriangleVertPackedIndices + (bLTtoRBOrientation ? 3 : 0),
                                                   &ElevData[0], ElevDataPitch,
                                                   iPackedIndicesBoundaryExtension,
                                                   fTriangulationErrorThreshold);
                    // If the threshold is exceeded, enable vertex
                    if( fCurrTriangleWorldSpaceError >= fTriangulationErrorThreshold )
                        bEnableVertex = true;
                }   

                EnabledFlags.SetVertexEnabledFlag(iX, iY, bEnableVertex);
            }
    }


    SQuadTreeNodeLocation pos;
    pElevData->GetPos(pos);

    CRQTTriangulation *pRQTAdaptiveTriang = new CRQTTriangulation(pos, EnabledFlags, m_iNumLevelsInHierarchy );

    // Calculate the whole triangulation error
    std::vector<UINT> WorkIndexBuffer;
    size_t MaxIndices = (iPatchSize+4 - 1) * (iPatchSize+4 - 1) * 2 * 3;
    WorkIndexBuffer.resize( MaxIndices );

    pRQTAdaptiveTriang->GenerateIndices(iPackedIndicesBoundaryExtension, &WorkIndexBuffer[0], uiNumTriangles);
    uiNumTriangles /=3;

    fTriangulationError = CalculateTriangulationError(&WorkIndexBuffer[0], uiNumTriangles, iPatchSize,
                                                      &ElevData[0],ElevDataPitch,
                                                      iPackedIndicesBoundaryExtension);

    assert( fTriangulationError <= fTriangulationErrorThreshold );    

    return pRQTAdaptiveTriang;
}
