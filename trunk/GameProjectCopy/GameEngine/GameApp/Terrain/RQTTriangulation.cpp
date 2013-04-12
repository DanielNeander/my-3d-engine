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
#include "RQTTriangulation.h"

void CRQTVertsEnabledFlags::Init(int iPatchSize, char bInitialEnableValue/* = TRUE*/)
{
    m_iPatchSize = iPatchSize;
    m_Flags.resize((m_iPatchSize+1)*(m_iPatchSize+1));
    for(int i=0; i < (int)m_Flags.size(); i++)m_Flags[i] = bInitialEnableValue;
}

const CRQTVertsEnabledFlags& CRQTVertsEnabledFlags::operator = (const CRQTVertsEnabledFlags& RQTFlags)
{
    m_iPatchSize = RQTFlags.m_iPatchSize;
    m_Flags = RQTFlags.m_Flags;
    return *this;
}

void CRQTVertsEnabledFlags::SaveToFile(FILE *pFile)
{
    fwrite(&m_iPatchSize, sizeof(m_iPatchSize), 1, pFile);
    if( m_Flags.size() > 0 )
    {
        for(int i=0; i < (int)m_Flags.size(); i+=32)
        {
            int iCurr32Flags = 0;
            size_t iFlagsLeft = m_Flags.size() - (size_t)i;
            for(int iBit=0; iBit < (int)min(32, iFlagsLeft); iBit++)
                iCurr32Flags |= (m_Flags[i+iBit] ? 1 : 0) << iBit;
            fwrite( &iCurr32Flags, sizeof(iCurr32Flags), 1, pFile);
        }
    }
}

HRESULT CRQTVertsEnabledFlags::LoadFromFile(FILE *pFile)
{
    int iPatchSize;
    size_t ItemsRead = fread(&iPatchSize, sizeof(iPatchSize), 1, pFile);
    if( ItemsRead != 1 )
        CHECK_HR_RET(E_FAIL, _T("Failed to read patch size") );

    Init(iPatchSize);
    if( m_Flags.size() > 0 )
    {
        for(int i=0; i < (int)m_Flags.size(); i+=32)
        {
            int iCurr32Flags = 0;
            ItemsRead = fread( &iCurr32Flags, sizeof(iCurr32Flags), 1, pFile);
            if( ItemsRead != 1 )
                CHECK_HR_RET(E_FAIL, _T("Failed to read next 32 flags for vertex (%d, %d)"), i%iPatchSize, i/iPatchSize );
            size_t iFlagsLeft = m_Flags.size() - (size_t)i;
            for(int iBit=0; iBit < (int)min(32, iFlagsLeft); iBit++)
                m_Flags[i+iBit] = (iCurr32Flags & (1 << iBit)) ? true : false;
        }
    }
    return S_OK;
}

// This constructor initializes the object to encode the triangulation
CRQTTriangulation::CRQTTriangulation(const SQuadTreeNodeLocation &pos,
                                     const CRQTVertsEnabledFlags &EnabledFlags,
                                     int iNumLevelsInHierarchy,
                                     CBitStream* pEncodedRQTBitStream /*= NULL*/) : 
    m_pos(pos),
    m_iNumLevelsInHierarchy(iNumLevelsInHierarchy),
    m_iNumLevelsInLocalPatchQT(0),
    m_EnabledFlags(EnabledFlags),
    m_pEncodedRQTBitStream(pEncodedRQTBitStream),
    m_bIsEncodingMode(true)
{
    m_iNumLevelsInLocalPatchQT = 0;
    while( (1 << m_iNumLevelsInLocalPatchQT) <= EnabledFlags.GetPatchSize())m_iNumLevelsInLocalPatchQT++;
}

// This constructor initializes the object to build the triangulation using encoded bitstream
CRQTTriangulation::CRQTTriangulation(const SQuadTreeNodeLocation &pos,
                                     int iNumLevelsInHierarchy,
                                     CBitStream *pEncodedRQTBitStream,
                                     int iPatchSize) :
    m_pos(pos),
    m_iNumLevelsInHierarchy(iNumLevelsInHierarchy),
    m_iNumLevelsInLocalPatchQT(0),
    
    // This is the first time the indices are generated. We will use
    // encoded bit stream to decode enabled flags and store them in
    // m_EnabledFlags
    m_pEncodedRQTBitStream(pEncodedRQTBitStream),
    m_bIsEncodingMode(false)
{
    m_EnabledFlags.Init(iPatchSize, FALSE);
    m_iNumLevelsInLocalPatchQT = 0;
    while( (1 << m_iNumLevelsInLocalPatchQT) <= iPatchSize)m_iNumLevelsInLocalPatchQT++;
}

CRQTTriangulation::CRQTTriangulation(const CRQTTriangulation &Triang)
{
    m_iNumLevelsInLocalPatchQT = Triang.m_iNumLevelsInLocalPatchQT;
    m_pos = Triang.m_pos;
    m_iNumLevelsInHierarchy = Triang.m_iNumLevelsInHierarchy;
    m_EnabledFlags = Triang.m_EnabledFlags;
    m_pEncodedRQTBitStream = Triang.m_pEncodedRQTBitStream;
}

CRQTTriangulation::~CRQTTriangulation(void)
{
}

void CRQTTriangulation::GetPos( SQuadTreeNodeLocation &outPos )
{
    outPos = m_pos;
}

void CRQTTriangulation::DefineTriangle(UINT* &puiIndices , int iX1, int iY1, int iX2, int iY2, int iX3, int iY3)const
{
    *(puiIndices++) = CalculatePackedIndex(iX1, iY1, m_iNumLevelsInLocalPatchQT-1, m_iNumLevelsInLocalPatchQT, m_iElevDataBoundaryExtension);
    *(puiIndices++) = CalculatePackedIndex(iX2, iY2, m_iNumLevelsInLocalPatchQT-1, m_iNumLevelsInLocalPatchQT, m_iElevDataBoundaryExtension);
    *(puiIndices++) = CalculatePackedIndex(iX3, iY3, m_iNumLevelsInLocalPatchQT-1, m_iNumLevelsInLocalPatchQT, m_iElevDataBoundaryExtension);
}

void CRQTTriangulation::EncodeDecodeEnabledFlag(bool &bBaseVertexEnabled, int iTriangleBaseX, int iTriangleBaseY)
{
    if( m_bIsEncodingMode )
    {
        // Load flag from vertex enabled map
        bBaseVertexEnabled = m_EnabledFlags.IsVertexEnabled(iTriangleBaseX, iTriangleBaseY) ? true : false;
        // Write flag to the bit stream if it is set
        if(m_pEncodedRQTBitStream)
            WriteEnabledFlag(bBaseVertexEnabled);
    }
    else
    {
        if( m_pEncodedRQTBitStream )
        {
            //This is the first time the triangulation is decoded
            //Read flags from the bit stream
            bBaseVertexEnabled = ReadNextFlag() ? true : false;
            //and store them in enabled flags map
            m_EnabledFlags.SetVertexEnabledFlag(iTriangleBaseX, iTriangleBaseY, bBaseVertexEnabled);
        }
        else
            // Enabled flags map is already decoded
            // Read the flag from the map
            bBaseVertexEnabled = m_EnabledFlags.IsVertexEnabled(iTriangleBaseX, iTriangleBaseY) ? true : false;
    }    
}

// This method recursively generates indices for the restricted quad tree triangulation
void CRQTTriangulation::RecursiveGenerateIndices(int iRightAngleX, int iRightAngleY, int iLevel,
                                                 RQT_TRIANG_ORIENTATION Orientation,              
                                                 int iElevDataBoundaryExtension,
                                                 UINT* &puiCurrIndex)
{
    int iLevelStep = 1 << ((m_iNumLevelsInLocalPatchQT-1) - iLevel);
    int iPatchSize = 1 << (m_iNumLevelsInLocalPatchQT-1);
    int iNextFinerLevelStep = ( iLevel < m_iNumLevelsInLocalPatchQT-1 ) ? 1 << ((m_iNumLevelsInLocalPatchQT-1) - (iLevel+1)) : 0;
    m_iElevDataBoundaryExtension = iElevDataBoundaryExtension;

    bool bBaseVertexEnabled = false;

    int iTriangleBaseX = -1;
    int iTriangleBaseY = -1;

    switch(Orientation)
    {
        case ORIENT_LB: 
        {
            /*   |\     */
            /*   | \    */
            /*   |  \   */
            /*   |___\  */
            if( iNextFinerLevelStep > 0 )
            {
                iTriangleBaseX = iRightAngleX + iNextFinerLevelStep;
                iTriangleBaseY = iRightAngleY + iNextFinerLevelStep;



                EncodeDecodeEnabledFlag(bBaseVertexEnabled, iTriangleBaseX, iTriangleBaseY);
            }

            if( bBaseVertexEnabled )
            {
                /*   |\     */
                /*   | \    */
                /*   | /\   */
                /*   |/__\  */
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel+1, ORIENT_T, iElevDataBoundaryExtension, puiCurrIndex);
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel+1, ORIENT_R, iElevDataBoundaryExtension, puiCurrIndex);
            }
            else if(puiCurrIndex)
            {
                DefineTriangle( puiCurrIndex, 
                                iRightAngleX,            iRightAngleY,
                                iRightAngleX+iLevelStep, iRightAngleY,
                                iRightAngleX,            iRightAngleY+iLevelStep);
                // Add two flange triangles on left border
                if(iRightAngleX == 0)
                {
                    DefineTriangle( puiCurrIndex, 
                                   -1, iRightAngleY,
                                    0, iRightAngleY,
                                    0, iRightAngleY+iLevelStep);
                    DefineTriangle( puiCurrIndex, 
                                    -1, iRightAngleY,
                                     0, iRightAngleY+iLevelStep,
                                    -1, iRightAngleY+iLevelStep);
                }
                // Add two flange triangles bottom border
                if(iRightAngleY == 0)
                {
                    DefineTriangle( puiCurrIndex, iRightAngleX, 0,
                                    iRightAngleX,-1,
                                    iRightAngleX+iLevelStep, -1);
                    DefineTriangle( puiCurrIndex, iRightAngleX, 0,
                                    iRightAngleX+iLevelStep, -1,
                                    iRightAngleX+iLevelStep,  0);
                }
            }
            break;
        }

        case ORIENT_RB:
        {
            /*     /|   */
            /*    / |   */
            /*   /  |   */
            /*  /___|   */
            if( iNextFinerLevelStep > 0 )
            {
                iTriangleBaseX = iRightAngleX - iNextFinerLevelStep;
                iTriangleBaseY = iRightAngleY + iNextFinerLevelStep;
                EncodeDecodeEnabledFlag(bBaseVertexEnabled, iTriangleBaseX, iTriangleBaseY);
            }

            if( bBaseVertexEnabled )
            {
                /*     /|   */
                /*    / |   */
                /*   /\ |   */
                /*  /__\|   */
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel+1, ORIENT_T, iElevDataBoundaryExtension, puiCurrIndex);
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel+1, ORIENT_L, iElevDataBoundaryExtension, puiCurrIndex);
            }
            else if(puiCurrIndex)
            {
                DefineTriangle( puiCurrIndex, iRightAngleX,            iRightAngleY,
                                iRightAngleX,            iRightAngleY+iLevelStep,
                                iRightAngleX-iLevelStep, iRightAngleY);
                
                // Add two flange triangles on right border
                if( iRightAngleX == iPatchSize )
                {
                    DefineTriangle( puiCurrIndex, iPatchSize,   iRightAngleY,
                                    iPatchSize+1, iRightAngleY,
                                    iPatchSize+1, iRightAngleY+iLevelStep);
                    DefineTriangle( puiCurrIndex, iPatchSize,   iRightAngleY,
                                    iPatchSize+1, iRightAngleY+iLevelStep,
                                    iPatchSize,   iRightAngleY+iLevelStep);
                }
                
                // Add two flange triangles on bottom border
                if(iRightAngleY == 0)
                {
                    DefineTriangle( puiCurrIndex, iRightAngleX,            0,
                                    iRightAngleX-iLevelStep, 0,
                                    iRightAngleX-iLevelStep,-1);
                    DefineTriangle( puiCurrIndex, iRightAngleX,            0,
                                    iRightAngleX-iLevelStep,-1,
                                    iRightAngleX,           -1);
                }
            }
            break;
        }

        case ORIENT_LT:
        {
            /*    ____   */
            /*   |   /   */
            /*   |  /    */
            /*   | /     */
            /*   |/      */
            if( iNextFinerLevelStep > 0 )
            {
                iTriangleBaseX = iRightAngleX + iNextFinerLevelStep;
                iTriangleBaseY = iRightAngleY - iNextFinerLevelStep;
                EncodeDecodeEnabledFlag(bBaseVertexEnabled, iTriangleBaseX, iTriangleBaseY);
            }

            if( bBaseVertexEnabled )
            {
                /*    ____   */
                /*   |\  /   */
                /*   | \/    */
                /*   | /     */
                /*   |/      */
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel+1, ORIENT_R, iElevDataBoundaryExtension, puiCurrIndex);
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel+1, ORIENT_B, iElevDataBoundaryExtension, puiCurrIndex);
            }
            else if(puiCurrIndex)
            {
                DefineTriangle( puiCurrIndex, iRightAngleX,            iRightAngleY,
                                iRightAngleX,            iRightAngleY-iLevelStep,
                                iRightAngleX+iLevelStep, iRightAngleY);
                
                // Add two flange triangles on left border
                if( iRightAngleX == 0 )
                {
                    DefineTriangle( puiCurrIndex,  0, iRightAngleY,
                                    -1, iRightAngleY,
                                    -1, iRightAngleY-iLevelStep);
                    DefineTriangle( puiCurrIndex,  0, iRightAngleY,
                                    -1, iRightAngleY-iLevelStep,
                                     0, iRightAngleY-iLevelStep);
                }

                // Add two flange triangles on top border
                if( iRightAngleY == iPatchSize )
                {
                    DefineTriangle( puiCurrIndex,  iRightAngleX,            iPatchSize,
                                     iRightAngleX+iLevelStep, iPatchSize,
                                     iRightAngleX+iLevelStep, iPatchSize+1);

                    DefineTriangle( puiCurrIndex,  iRightAngleX,            iPatchSize,
                                     iRightAngleX+iLevelStep, iPatchSize+1,
                                     iRightAngleX,            iPatchSize+1);
                }
            }
            break;
        }

        case ORIENT_RT: 
        { 
            /*  ____  */
            /*  \   | */
            /*   \  | */
            /*    \ | */
            /*     \| */
            if( iNextFinerLevelStep > 0 )
            {
                iTriangleBaseX = iRightAngleX - iNextFinerLevelStep;
                iTriangleBaseY = iRightAngleY - iNextFinerLevelStep;
                EncodeDecodeEnabledFlag(bBaseVertexEnabled, iTriangleBaseX, iTriangleBaseY);
            }

            if( bBaseVertexEnabled )
            {
                /*  ____  */
                /*  \  /| */
                /*   \/ | */
                /*    \ | */
                /*     \| */
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel+1, ORIENT_B, iElevDataBoundaryExtension, puiCurrIndex);
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel+1, ORIENT_L, iElevDataBoundaryExtension, puiCurrIndex);
            }
            else if(puiCurrIndex)
            {
                DefineTriangle( puiCurrIndex, iRightAngleX,            iRightAngleY,
                                iRightAngleX-iLevelStep, iRightAngleY,
                                iRightAngleX,            iRightAngleY-iLevelStep);

                // Add two flange triangles on right border
                if( iRightAngleX == iPatchSize )
                {
                    DefineTriangle( puiCurrIndex, iPatchSize,  iRightAngleY,
                                    iPatchSize,  iRightAngleY-iLevelStep,
                                    iPatchSize+1,iRightAngleY-iLevelStep);
                    DefineTriangle( puiCurrIndex, iPatchSize,  iRightAngleY,
                                    iPatchSize+1,iRightAngleY-iLevelStep,
                                    iPatchSize+1,iRightAngleY);
                }
                
                // Add two flange triangles on top border
                if( iRightAngleY == iPatchSize )
                {
                    DefineTriangle( puiCurrIndex, iRightAngleX,             iPatchSize,
                                    iRightAngleX,             iPatchSize+1,
                                    iRightAngleX-iLevelStep,  iPatchSize);
                    DefineTriangle( puiCurrIndex, iRightAngleX-iLevelStep,  iPatchSize,
                                    iRightAngleX,             iPatchSize+1,
                                    iRightAngleX-iLevelStep,  iPatchSize+1);
                }
            }
            break;
        }

        case ORIENT_L:
        {
            //    /|
            //   / |
            //   \ |
            //    \|
            iTriangleBaseX = iRightAngleX + iLevelStep;
            iTriangleBaseY = iRightAngleY;
            EncodeDecodeEnabledFlag(bBaseVertexEnabled, iTriangleBaseX, iTriangleBaseY);

            if( bBaseVertexEnabled )
            {
                //    /|
                //   /_|
                //   \ |
                //    \|
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel, ORIENT_RB, iElevDataBoundaryExtension, puiCurrIndex);
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel, ORIENT_RT, iElevDataBoundaryExtension, puiCurrIndex);
            }
            else if(puiCurrIndex)
            {
                DefineTriangle( puiCurrIndex, iRightAngleX,            iRightAngleY,
                                iRightAngleX+iLevelStep, iRightAngleY-iLevelStep,
                                iRightAngleX+iLevelStep, iRightAngleY+iLevelStep);
                
                // Add two flange triangles on right border
                if( iRightAngleX+iLevelStep == iPatchSize )
                {
                    DefineTriangle( puiCurrIndex, iPatchSize,   iRightAngleY-iLevelStep,
                                    iPatchSize+1, iRightAngleY-iLevelStep,
                                    iPatchSize+1, iRightAngleY+iLevelStep);
                    DefineTriangle( puiCurrIndex, iPatchSize,   iRightAngleY-iLevelStep,
                                    iPatchSize+1, iRightAngleY+iLevelStep,
                                    iPatchSize,   iRightAngleY+iLevelStep);
                }
            }
            break;
        }

        case ORIENT_R:
        {
            //   |\
            //   | \
            //   | /
            //   |/
            iTriangleBaseX = iRightAngleX - iLevelStep;
            iTriangleBaseY = iRightAngleY;
            EncodeDecodeEnabledFlag(bBaseVertexEnabled, iTriangleBaseX, iTriangleBaseY);

            if( bBaseVertexEnabled )
            {
                //   |\
                //   |_\
                //   | /
                //   |/
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel, ORIENT_LB, iElevDataBoundaryExtension, puiCurrIndex);
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel, ORIENT_LT, iElevDataBoundaryExtension, puiCurrIndex);
            }
            else if(puiCurrIndex)
            {
                DefineTriangle( puiCurrIndex, iRightAngleX,            iRightAngleY,
                                iRightAngleX-iLevelStep, iRightAngleY+iLevelStep,
                                iRightAngleX-iLevelStep, iRightAngleY-iLevelStep);
                // Add two flange triangles on left border
                if( iRightAngleX-iLevelStep == 0 )
                {
                    DefineTriangle( puiCurrIndex, -1, iRightAngleY-iLevelStep,
                                     0, iRightAngleY-iLevelStep,
                                     0, iRightAngleY+iLevelStep);
                    DefineTriangle( puiCurrIndex, -1, iRightAngleY-iLevelStep,
                                     0, iRightAngleY+iLevelStep,
                                    -1, iRightAngleY+iLevelStep);
                }
            }
            break;
        }

        case ORIENT_B:
        {
            //  ________
            //  \       /
            //   \     /
            //    \   /
            //     \ /
            iTriangleBaseX = iRightAngleX;
            iTriangleBaseY = iRightAngleY + iLevelStep;
            EncodeDecodeEnabledFlag(bBaseVertexEnabled, iTriangleBaseX, iTriangleBaseY);

            if( bBaseVertexEnabled )
            {
                //  ________
                //  \   |   /
                //   \  |  /
                //    \ | /
                //     \|/
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel, ORIENT_LT, iElevDataBoundaryExtension, puiCurrIndex);
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel, ORIENT_RT, iElevDataBoundaryExtension, puiCurrIndex);
            }
            else if(puiCurrIndex)
            {
                DefineTriangle( puiCurrIndex, iRightAngleX,            iRightAngleY,
                                iRightAngleX+iLevelStep, iRightAngleY+iLevelStep,
                                iRightAngleX-iLevelStep, iRightAngleY+iLevelStep);
                // Add two flange triangles on top border
                if( iRightAngleY+iLevelStep == iPatchSize )
                {
                    DefineTriangle( puiCurrIndex, iRightAngleX-iLevelStep, iPatchSize,
                                    iRightAngleX+iLevelStep, iPatchSize,
                                    iRightAngleX+iLevelStep, iPatchSize+1);
                    DefineTriangle( puiCurrIndex, iRightAngleX-iLevelStep, iPatchSize,
                                    iRightAngleX+iLevelStep, iPatchSize+1,
                                    iRightAngleX-iLevelStep, iPatchSize+1);
                }
            }

            break;
        }

        case ORIENT_T:
        {
            //     /\
            //    /  \
            //   /    \
            //  /______\ 
            iTriangleBaseX = iRightAngleX;
            iTriangleBaseY = iRightAngleY - iLevelStep;
            EncodeDecodeEnabledFlag(bBaseVertexEnabled, iTriangleBaseX, iTriangleBaseY);

            if( bBaseVertexEnabled )
            {
                //     /|\
                //    / | \
                //   /  |  \
                //  /___|___\ 
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel, ORIENT_LB, iElevDataBoundaryExtension, puiCurrIndex);
                RecursiveGenerateIndices(iTriangleBaseX, iTriangleBaseY, iLevel, ORIENT_RB, iElevDataBoundaryExtension, puiCurrIndex);
            }
            else if(puiCurrIndex)
            {
                DefineTriangle( puiCurrIndex, iRightAngleX,            iRightAngleY,
                                iRightAngleX-iLevelStep, iRightAngleY-iLevelStep,
                                iRightAngleX+iLevelStep, iRightAngleY-iLevelStep);
                // Add two flange triangles on bottom border
                if( iRightAngleY-iLevelStep == 0 )
                {
                    DefineTriangle( puiCurrIndex, iRightAngleX-iLevelStep, -1,
                                    iRightAngleX+iLevelStep, -1,
                                    iRightAngleX+iLevelStep,  0);
                    DefineTriangle( puiCurrIndex, iRightAngleX-iLevelStep, -1,
                                    iRightAngleX+iLevelStep,  0,
                                    iRightAngleX-iLevelStep,  0);
                }
            }
            break;
        }

        default: assert(false);
    }
}


void CRQTTriangulation::GenerateIndices( int iElevDataBoundaryExtension,
                                         UINT *puiIndices,
                                         UINT &uiNumIndicesGenerated)

{
    if( m_bIsEncodingMode )
    {
        // Prepare the bit stream for writing
        if(m_pEncodedRQTBitStream)
            m_pEncodedRQTBitStream->StartWriting();
    }
    else
    {
        if( m_pEncodedRQTBitStream )
        {
            // Prepare the bit stream for reading
            m_pEncodedRQTBitStream->StartReading();
        }
    }

    int iPatchSize = m_EnabledFlags.GetPatchSize();
    UINT *puiCurrIndex = puiIndices;
    // Start recursive process from two coarsest-level triangles
    RecursiveGenerateIndices(0, iPatchSize, 0, ORIENT_LT, iElevDataBoundaryExtension, puiCurrIndex);
    RecursiveGenerateIndices(iPatchSize, 0, 0, ORIENT_RB, iElevDataBoundaryExtension, puiCurrIndex);

    uiNumIndicesGenerated = (UINT)( puiCurrIndex - puiIndices );

    if( m_bIsEncodingMode )
    {
        // Finish writing the bit stream
        if(m_pEncodedRQTBitStream)
            m_pEncodedRQTBitStream->FinishWriting();
    }
    else
    {
        if( m_pEncodedRQTBitStream )
        {
            // Finish reading the bit stream
            m_pEncodedRQTBitStream->FinishReading();
            // m_EnabledFlags are now initialized
        }
    }

    // We do not need m_pEncodedRQTBitStream anymore
    m_pEncodedRQTBitStream = NULL;
}
