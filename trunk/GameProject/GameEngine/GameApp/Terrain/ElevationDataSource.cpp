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

#include "ElevationDataSource.h"
#include "DynamicQuadTreeNode.h"
#include <exception>

#include <wincodec.h>
#include <wincodecsdk.h>
#pragma comment(lib, "WindowsCodecs.lib")

// Elevation data
CPatchElevationData::CPatchElevationData(const class CElevationDataSource *pDataSource, 
                                         const SQuadTreeNodeLocation &pos,
                                         int iLeftBndrExt,
                                         int iBottomBndrExt,
                                         int iRightBndrExt,
                                         int iTopBndrExt,
                                         int iHighResDataLODBias) : 
    m_pos(pos),
    // Extensions of the stored data
    m_iLeftBndrExt   ( iLeftBndrExt ),
    m_iBottomBndrExt ( iBottomBndrExt),
    m_iRightBndrExt  ( iRightBndrExt),
    m_iTopBndrExt    (  iTopBndrExt),
    m_iHighResDataLODBias (iHighResDataLODBias),
    m_HighResDataPitch(0),
    m_iPatchSize(pDataSource->GetPatchSize()),
    m_ErrorBound(pDataSource->GetPatchElevDataErrorBound(pos))

{
    // Height map data returned by the data source will have the following layout:
    //    m_iLeftBndrExt    m_iRightBndrExt
    //    |__|____________|_| _
    //    |   ____________   |_ m_iTopBndrExt
    //    |  |            |  |
    //    |  |            |  |
    //    |  |            |  |
    //    |  |            |  |
    //    |  |____________|  |_
    //    |__________________|_ m_iBottomBndrExt
    //       |<---------->|
    //         iPatchSize
    int iPatchSize = GetPatchSize();
    m_Pitch = (iPatchSize + m_iBottomBndrExt + m_iTopBndrExt);
    m_HeightMap.resize( (iPatchSize + m_iLeftBndrExt + m_iRightBndrExt)*m_Pitch );
    if( m_iHighResDataLODBias )
    {
        m_HighResDataPitch = m_Pitch << m_iHighResDataLODBias;
        m_HighResHeightMap.resize( ((iPatchSize + m_iLeftBndrExt + m_iRightBndrExt)<<m_iHighResDataLODBias)*m_HighResDataPitch );
    }
}

CPatchElevationData::~CPatchElevationData(void)
{
}

int CPatchElevationData::GetPatchSize()const
{
    return m_iPatchSize;
}

void CPatchElevationData::GetPos( SQuadTreeNodeLocation &outPos )const
{
    outPos = m_pos;
}

// Returns pointer to the stored data
void CPatchElevationData::GetDataPtr( const UINT16* &pDataPtr, 
                                      size_t &Pitch, 
                                      int LeftBoundaryExtension,
                                      int BottomBoundaryExtension,
                                      int RightBoundaryExtension,
                                      int TopBoundaryExtension )const
{
    // Check that requested extensions do not exceed extensions of the stored data
    assert( LeftBoundaryExtension <= m_iLeftBndrExt );
    assert( RightBoundaryExtension <= m_iRightBndrExt );
    assert( BottomBoundaryExtension <= m_iBottomBndrExt );
    assert( TopBoundaryExtension <= m_iTopBndrExt );
    
    // Get data pointer
    Pitch = m_Pitch;
    pDataPtr = &m_HeightMap[ (m_iLeftBndrExt - LeftBoundaryExtension) + (m_iBottomBndrExt-BottomBoundaryExtension) *m_Pitch]; 
}

void CPatchElevationData::GetHighResDataPtr( const UINT16* &pHighResDataPtr, 
                                             size_t &HighResDataPitch, 
                                             int LeftBoundaryExtension,
                                             int BottomBoundaryExtension,
                                             int RightBoundaryExtension,
                                             int TopBoundaryExtension )const
{
    // Check that requested extensions do not exceed extensions of the stored data
    assert( LeftBoundaryExtension <= (m_iLeftBndrExt<<m_iHighResDataLODBias) );
    assert( RightBoundaryExtension <= (m_iRightBndrExt<<m_iHighResDataLODBias) );
    assert( BottomBoundaryExtension <= (m_iBottomBndrExt<<m_iHighResDataLODBias) );
    assert( TopBoundaryExtension <= (m_iTopBndrExt<<m_iHighResDataLODBias) );
    if( m_iHighResDataLODBias )
    {
        HighResDataPitch = m_HighResDataPitch;
        pHighResDataPtr = &m_HighResHeightMap[ ((m_iLeftBndrExt<<m_iHighResDataLODBias) - LeftBoundaryExtension) + ((m_iBottomBndrExt<<m_iHighResDataLODBias) - BottomBoundaryExtension) *m_HighResDataPitch]; 
    }
    else
    {
        GetDataPtr( pHighResDataPtr, HighResDataPitch, LeftBoundaryExtension, BottomBoundaryExtension, RightBoundaryExtension, TopBoundaryExtension );
    }
}

UINT16 CPatchElevationData::GetApproximationErrorBound()const
{
    return m_ErrorBound;
}



// Creates data source from the specified raw data file
CElevationDataSource::CElevationDataSource(LPCWSTR strSrcDemFile,
                                           int iPatchSize):
    m_iPatchSize(iPatchSize),
    m_iRequiredLeftBoundaryExt(0),
    m_iRequiredBottomBoundaryExt(0),
    m_iRequiredRightBoundaryExt(0),
    m_iRequiredTopBoundaryExt(0),
    m_iHighResDataLODBias(0)
{
    HRESULT hr;
    if( iPatchSize & (iPatchSize-1) )
    {
        CHECK_HR(E_FAIL, _T("Patch size (%d) must be power of 2"), iPatchSize );
        throw std::exception("Patch size must be power of 2");
    }

    V( CoInitialize(NULL) );

    // Create components to read 16-bit png data
    CComPtr<IWICImagingFactory> pFactory;
    hr = pFactory.CoCreateInstance(CLSID_WICImagingFactory);
    CHECK_HR(hr, _T("Failed to create WICImagingFactory"));
   
    CComPtr<IWICStream> pInputStream;
    hr = pFactory->CreateStream(&pInputStream);
    CHECK_HR(hr, _T("Failed to create WICStream"));

    hr = pInputStream->InitializeFromFilename(strSrcDemFile, GENERIC_READ);
    CHECK_HR(hr, _T("Failed to initialize WICStream from file") );

    CComPtr<IWICBitmapDecoder> pDecoder;
    hr = pFactory->CreateDecoderFromStream(
                  pInputStream,
                  0, // vendor
                  WICDecodeMetadataCacheOnDemand,
                  &pDecoder);
    CHECK_HR(hr, _T("Failed to Create decoder from stream") );

    //GUID ContainerFormat;
    //pDecoder->GetContainerFormat(&ContainerFormat);
    //if( ContainerFormat == GUID_ContainerFormatPng )
    //    printf("Container Format: PNG\n");
    //else if( ContainerFormat == GUID_ContainerFormatTiff )
    //    printf("Container Format: TIFF\n");

    UINT frameCount = 0;
    hr = pDecoder->GetFrameCount(&frameCount);
    //printf("Frame count %d\n", frameCount);
    assert( frameCount == 1 );

    CComPtr<IWICBitmapFrameDecode> pTheFrame;
    pDecoder->GetFrame(0, &pTheFrame);

    UINT width = 0;
    UINT height = 0;
    pTheFrame->GetSize(&width, &height);

    // Calculate minimal number of columns and rows
    // in the form 2^n+1 that encompass the data
    m_iNumCols = 1;
    m_iNumRows = 1;
    while( m_iNumCols+1 < width || m_iNumRows+1 < height)
    {
        m_iNumCols *= 2;
        m_iNumRows *= 2;
    }

    m_iNumLevels = 1;
    while( (m_iPatchSize << (m_iNumLevels-1)) < (int)m_iNumCols ||
           (m_iPatchSize << (m_iNumLevels-1)) < (int)m_iNumRows )
        m_iNumLevels++;

    m_iNumCols++;
    m_iNumRows++;

    GUID pixelFormat = { 0 };
    pTheFrame->GetPixelFormat(&pixelFormat);
    if( pixelFormat != GUID_WICPixelFormat16bppGray )
    {
        assert(false);
        throw std::exception("expected 16 bit format");
    }

	m_iWidth = width;
	m_iHeight = height;

    // Load the data
    m_TheHeightMap.resize( m_iNumCols * m_iNumRows );
    WICRect SrcRect;
    SrcRect.X = 0;
    SrcRect.Y = 0;
    SrcRect.Height = height;
    SrcRect.Width = width;
    pTheFrame->CopyPixels(
      &SrcRect,
      (UINT)m_iNumCols*2, //UINT stride
      (UINT)m_TheHeightMap.size()*2, //UINT bufferSize
      (BYTE*)&m_TheHeightMap[0]);

	    // Duplicate the last row and column
    for(UINT iRow = 0; iRow < height; iRow++)
        for(UINT iCol = width; iCol < m_iNumCols; iCol++)
            m_TheHeightMap[iCol + iRow * m_iNumCols] = m_TheHeightMap[(width-1) + iRow * m_iNumCols];
    for(UINT iCol = 0; iCol < m_iNumCols; iCol++)
        for(UINT iRow = height; iRow < m_iNumRows; iRow++)
            m_TheHeightMap[iCol + iRow * m_iNumCols] = m_TheHeightMap[iCol + (height-1) * m_iNumCols];

    pTheFrame.Release();
    pFactory.Release();
    pDecoder.Release();
    pInputStream.Release();

    CoUninitialize();

    m_MinMaxElevation.Resize(m_iNumLevels);
    m_ErrorBounds.Resize(m_iNumLevels-1);
    
    // Calcualte min/max elevations
    CalculateMinMaxElevations();

    // Calcualte world space error bounds
    CalculatePatchErrorBounds();
}

CElevationDataSource::~CElevationDataSource(void)
{
}

// Copies height map to the specified memory location
// using start/end columns and rows as input
void CElevationDataSource :: FillPatchHeightMap(UINT16 *pDataPtr,
                                                size_t DataPitch,
                                                int iStartCol, int iEndCol, 
                                                int iStartRow, int iEndRow, 
                                                int iStep)const
{
    for(int iRow = iStartRow; iRow < iEndRow; iRow++)
        for(int iCol = iStartCol; iCol < iEndCol; iCol++)
        {
            int iSrcCol = max(0, iCol*iStep); iSrcCol = min(iSrcCol, (int)m_iNumCols-1);
            int iSrcRow = max(0, iRow*iStep); iSrcRow = min(iSrcRow, (int)m_iNumRows-1);
            pDataPtr[(iCol-iStartCol) + (iRow-iStartRow) * DataPitch] = m_TheHeightMap[iSrcCol + iSrcRow * m_iNumCols];
        }
}

// Copies height map to the specified memory location
// using quad tree node location as input
void CElevationDataSource :: FillPatchHeightMap(const SQuadTreeNodeLocation &pos,
                                                UINT16 *pDataPtr,
                                                size_t DataPitch,
                                                int iLeftExt/* = 0 */,
                                                int iBottomExt/* = 0 */,
                                                int iRightExt/* = 0 */,
                                                int iTopExt/* = 0 */,
                                                int iLODBias/* = 0*/)const
{
    int iStep = 1 << (m_iNumLevels-1 - pos.level);
    int iStartCol =  pos.horzOrder    * m_iPatchSize - iLeftExt;
    int iEndCol   = (pos.horzOrder+1) * m_iPatchSize + iRightExt;
    int iStartRow =  pos.vertOrder    * m_iPatchSize - iBottomExt;
    int iEndRow   = (pos.vertOrder+1) * m_iPatchSize + iTopExt;
    FillPatchHeightMap(pDataPtr, DataPitch, iStartCol<<iLODBias, iEndCol<<iLODBias, iStartRow<<iLODBias, iEndRow<<iLODBias, iStep>>iLODBias);
}

void CElevationDataSource :: GetPatchMinMaxElevation(const SQuadTreeNodeLocation &pos,
                                                     UINT16 &MinElevation, 
                                                     UINT16 &MaxElevation)const
{
    MinElevation = m_MinMaxElevation[pos].first;
    MaxElevation = m_MinMaxElevation[pos].second;
}

UINT16 CElevationDataSource :: GetGlobalMinElevation()const
{
    return m_MinMaxElevation[SQuadTreeNodeLocation()].first;
}

UINT16 CElevationDataSource :: GetGlobalMaxElevation()const
{
    return m_MinMaxElevation[SQuadTreeNodeLocation()].second;
}

UINT16 CElevationDataSource :: GetPatchElevDataErrorBound(const SQuadTreeNodeLocation &pos)const
{
    if(pos.level < m_iNumLevels-1)
        return m_ErrorBounds[pos];
    else
        return 0;
}

// Calculates min/max elevations for the hierarchy
void CElevationDataSource :: CalculateMinMaxElevations()
{
    // Calculate min/max elevations for the finest level patches
    int iPatchesAlongFinestLevelSide = 1 << (m_iNumLevels-1);
    for( int vertOrder = 0; vertOrder < iPatchesAlongFinestLevelSide; vertOrder++)
    {
        for( int horzOrder = 0; horzOrder < iPatchesAlongFinestLevelSide; horzOrder++)
        {
            std::pair<UINT16, UINT16> &CurrPatchMinMaxElev = m_MinMaxElevation[SQuadTreeNodeLocation(horzOrder, vertOrder, m_iNumLevels-1)];
            int iStartCol = horzOrder*m_iPatchSize;
            int iStartRow = vertOrder*m_iPatchSize;
            CurrPatchMinMaxElev.first = CurrPatchMinMaxElev.second = m_TheHeightMap[iStartCol + iStartRow*m_iNumCols];
            for(int iRow = iStartRow; iRow <= iStartRow + m_iPatchSize; iRow++)
                for(int iCol = iStartCol; iCol <= iStartCol + m_iPatchSize; iCol++)
                {
                    UINT16 CurrElev = m_TheHeightMap[iCol + iRow*m_iNumCols];
                    CurrPatchMinMaxElev.first = min(CurrPatchMinMaxElev.first, CurrElev);
                    CurrPatchMinMaxElev.second = max(CurrPatchMinMaxElev.second, CurrElev);
                }
        }
    }

    // Recursively calculate min/max elevations for the coarser levels
    for( HierarchyReverseIterator it(m_iNumLevels-1); it.IsValid(); it.Next() )
    {
        std::pair<UINT16, UINT16> &CurrPatchMinMaxElev = m_MinMaxElevation[it];
        std::pair<UINT16, UINT16> &LBChildMinMaxElev = m_MinMaxElevation[GetChildLocation(it, 0)];
        std::pair<UINT16, UINT16> &RBChildMinMaxElev = m_MinMaxElevation[GetChildLocation(it, 1)];
        std::pair<UINT16, UINT16> &LTChildMinMaxElev = m_MinMaxElevation[GetChildLocation(it, 2)];
        std::pair<UINT16, UINT16> &RTChildMinMaxElev = m_MinMaxElevation[GetChildLocation(it, 3)];

        CurrPatchMinMaxElev.first = min( LBChildMinMaxElev.first, RBChildMinMaxElev.first );
        CurrPatchMinMaxElev.first = min( CurrPatchMinMaxElev.first, LTChildMinMaxElev.first );
        CurrPatchMinMaxElev.first = min( CurrPatchMinMaxElev.first, RTChildMinMaxElev.first );

        CurrPatchMinMaxElev.second = max( LBChildMinMaxElev.second, RBChildMinMaxElev.second);
        CurrPatchMinMaxElev.second = max( CurrPatchMinMaxElev.second, LTChildMinMaxElev.second );
        CurrPatchMinMaxElev.second = max( CurrPatchMinMaxElev.second, RTChildMinMaxElev.second );
    }
}

void CElevationDataSource :: CalculatePatchErrorBounds()
{
    std::vector<UINT16> ParentHeightMap, ChildrenHeightMap;
    ParentHeightMap.resize( (m_iPatchSize+1) * (m_iPatchSize+1) );
    ChildrenHeightMap.resize( (m_iPatchSize*2+1) * (m_iPatchSize*2+1) );
    // Start from the coarsest level
    for( HierarchyReverseIterator it(m_iNumLevels-1); it.IsValid(); it.Next() )
    {
        FillPatchHeightMap(it, &ParentHeightMap[0], m_iPatchSize+1,0,0,1,1);

        int iStep = 1 << (m_iNumLevels-1 - (it.Level()+1));
        int iStartCol =  it.Horz()    * m_iPatchSize*2;
        int iEndCol   = (it.Horz()+1) * m_iPatchSize*2+1;
        int iStartRow =  it.Vert()    * m_iPatchSize*2;
        int iEndRow   = (it.Vert()+1) * m_iPatchSize*2+1;
        
        FillPatchHeightMap(&ChildrenHeightMap[0], m_iPatchSize*2+1, iStartCol, iEndCol, iStartRow, iEndRow, iStep);

        int iMaxErrCol = -1;
        int iMaxErrRow = -1;
        float fInterpolationError = 0.f;
        for(int iHighResRow = 0; iHighResRow < m_iPatchSize*2; iHighResRow++)
            for(int iHighResCol = 0; iHighResCol < m_iPatchSize*2; iHighResCol++)
            {
                // Get finer resolution height map sample
                float fHighResElev = ChildrenHeightMap[ iHighResCol + iHighResRow * (m_iPatchSize*2+1) ];
                // Calculate the sample bilinear interpolation using corse height map samples
                int iSrcCol0 = iHighResCol/2;
                int iSrcRow0 = iHighResRow/2;
                // *X    *     *X
                //
                // *     *     *
                //
                // *X    *     *X
                float fHorzWeight = (float)iHighResCol/2.f - (float)iSrcCol0;
                float fVertWeight = (float)iHighResRow/2.f - (float)iSrcRow0;
                float fElev00 = ParentHeightMap[iSrcCol0   + iSrcRow0 * (m_iPatchSize+1) ];
                float fElev10 = ParentHeightMap[iSrcCol0+1 + iSrcRow0 * (m_iPatchSize+1) ];
                float fElev01 = ParentHeightMap[iSrcCol0   + (iSrcRow0+1) * (m_iPatchSize+1) ];
                float fElev11 = ParentHeightMap[iSrcCol0+1 + (iSrcRow0+1) * (m_iPatchSize+1) ];
                float fInterploatedElev = (fElev00 * (1.f-fHorzWeight) + fElev10 * fHorzWeight) * (1 - fVertWeight) + 
                                          (fElev01 * (1.f-fHorzWeight) + fElev11 * fHorzWeight) * fVertWeight;
                // Calculate approximation error
                float fCurrError = fabsf(fHighResElev - fInterploatedElev);
                if( fCurrError > fInterpolationError )
                {
                    iMaxErrCol = iHighResCol;
                    iMaxErrRow = iHighResRow;
                }
                fInterpolationError = max(fInterpolationError, fCurrError);
            }

        float CurrPatchError = 0;
        if(it.Level() < m_iNumLevels-2)
        {
            // Add child interpolation errors
            for(int i=0; i<4; i++)
                CurrPatchError = max(CurrPatchError, m_ErrorBounds[GetChildLocation(it,i)]);
        }
        CurrPatchError += fInterpolationError;
        m_ErrorBounds[it] = (UINT16)min(max(0, (int)CurrPatchError ), UINT16_MAX);
    }
}

int CElevationDataSource :: GetNumLevelsInHierarchy()const
{
    return m_iNumLevels;
}

int CElevationDataSource :: GetPatchSize()const
{
    return m_iPatchSize;
}

// Decompresses child height map taking their parent height map as input
CPatchElevationData* CElevationDataSource :: GetElevData(const struct SQuadTreeNodeLocation &Pos)const
{
    int iHighResDataLODBias = max(0, min(m_iHighResDataLODBias, (m_iNumLevels-1) - Pos.level) );
    CPatchElevationData* pElevData = new CPatchElevationData( this, Pos, 
                                           m_iRequiredLeftBoundaryExt, 
                                           m_iRequiredBottomBoundaryExt, 
                                           m_iRequiredRightBoundaryExt, 
                                           m_iRequiredTopBoundaryExt,
                                           iHighResDataLODBias );
    FillPatchHeightMap( Pos, 
                        &pElevData->m_HeightMap[0],
                        pElevData->m_Pitch,
                        m_iRequiredLeftBoundaryExt, 
                        m_iRequiredBottomBoundaryExt, 
                        m_iRequiredRightBoundaryExt, 
                        m_iRequiredTopBoundaryExt );
    if( iHighResDataLODBias )
    {
        FillPatchHeightMap( Pos, 
                    &pElevData->m_HighResHeightMap[0],
                    pElevData->m_HighResDataPitch,
                    m_iRequiredLeftBoundaryExt, 
                    m_iRequiredBottomBoundaryExt, 
                    m_iRequiredRightBoundaryExt, 
                    m_iRequiredTopBoundaryExt,
                    iHighResDataLODBias);
    }
    return pElevData;
}

void CElevationDataSource::SetRequiredElevDataBoundaryExtensions(int iRequiredLeftBoundaryExt,
                                                                 int iRequiredBottomBoundaryExt,
                                                                 int iRequiredRightBoundaryExt,
                                                                 int iRequiredTopBoundaryExt)
{
    // Elevation data must contain extended data provided by data source
    m_iRequiredLeftBoundaryExt   = max( iRequiredLeftBoundaryExt,  LB_DATA_EXTENSION_WIDTH);
    m_iRequiredBottomBoundaryExt = max( iRequiredBottomBoundaryExt,LB_DATA_EXTENSION_WIDTH);
    m_iRequiredRightBoundaryExt  = max( iRequiredRightBoundaryExt, RT_DATA_EXTENSION_WIDTH);
    m_iRequiredTopBoundaryExt    = max( iRequiredTopBoundaryExt,   RT_DATA_EXTENSION_WIDTH);
}

void CElevationDataSource::SetHighResDataLODBias(int iHighResDataLODBias)
{
    m_iHighResDataLODBias = iHighResDataLODBias;
}

unsigned CElevationDataSource::m_iNumRows;

unsigned CElevationDataSource::m_iNumCols;

int CElevationDataSource::m_iWidth;
int CElevationDataSource::m_iHeight;
