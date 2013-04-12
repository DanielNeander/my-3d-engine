///////////////////////////////////////////////////////////////////////  
//  TGALoader.cpp
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


///////////////////////////////////////////////////////////////////////  
//  Preprocessor
#include "stdafx.h"
#include "TGALoader.h"
#include "Utility.h"
using namespace SpeedTree;


///////////////////////////////////////////////////////////////////////  
// Uncompressed TGA header key

const st_uchar c_acTGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


///////////////////////////////////////////////////////////////////////  
//  CTGALoader::CTGALoader

CTGALoader::CTGALoader(void) :
    m_nHeight(0),
    m_nWidth(0),
    m_nDepth(0),
    m_pImageData(NULL),
    m_bDeleteData(true)
{
}


///////////////////////////////////////////////////////////////////////  
//  CTGALoader::CTGALoader

CTGALoader::CTGALoader(st_uchar* pImageData, st_int32 nWidth, st_int32 nHeight, st_int32 nDepth, st_bool bCopyData) :
    m_nHeight(nHeight),
    m_nWidth(nWidth),
    m_nDepth(nDepth),
    m_pImageData(NULL),
    m_bDeleteData(bCopyData)
{
    if (pImageData == NULL)
    {
        st_int32 nSize = m_nWidth * m_nHeight * m_nDepth;
        m_pImageData = st_new_array<st_uchar>(nSize, "CTGALoader::unsigned char");
        memset(m_pImageData, 0, nSize);
    }
    else
    {
        if (bCopyData)
        {
            st_int32 nSize = m_nWidth * m_nHeight * m_nDepth;
            m_pImageData = st_new_array<st_uchar>(nSize, "CTGALoader::unsigned char");
            memcpy(m_pImageData, pImageData, nSize);
        }
        else
        {
            m_pImageData = pImageData;
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CTGALoader::~CTGALoader

CTGALoader::~CTGALoader( )
{
    if (m_bDeleteData)
        st_delete_array<st_uchar>(m_pImageData, "CTGALoader::unsigned char");
}


///////////////////////////////////////////////////////////////////////  
//  CTGALoader::Read

st_bool CTGALoader::Read(const char* pFilename)
{
    if (m_bDeleteData && m_pImageData != NULL)
        st_delete_array<st_uchar>(m_pImageData, "CTGALoader::unsigned char");
    m_pImageData = NULL;

    st_bool bSuccess = false;

    FILE* pFile = fopen(pFilename, "rb");
    if (pFile != NULL)
    {
        st_uchar acTGAcompare[12];                              // used to compare tga header
        st_uchar acDimensionBytes[6];                                   // first 6 useful bytes from the header

        // are there 12 bytes to read
        fread(acTGAcompare, sizeof(acTGAcompare), 1, pFile);

        // does the header match the key?
        if (memcmp(c_acTGAheader, acTGAcompare, sizeof(c_acTGAheader)) == 0)
        {
            // if so, read the next 6 header bytes
            fread(acDimensionBytes, sizeof(acDimensionBytes), 1, pFile);

            m_nWidth = (acDimensionBytes[1] << 8) + acDimensionBytes[0];
            m_nHeight = (acDimensionBytes[3] << 8) + acDimensionBytes[2];

            if (m_nWidth > 0 || m_nHeight > 0)
            {
                st_uint32 nBitsPerPixel = acDimensionBytes[4];
                if (nBitsPerPixel == 24 || nBitsPerPixel == 32)
                {
                    m_nDepth = nBitsPerPixel / 8;
                    st_int32 nImageSize = m_nWidth * m_nHeight * m_nDepth;

                    m_pImageData = st_new_array<st_uchar>(nImageSize, "CTGALoader::unsigned char");

                    fread(m_pImageData, nImageSize, 1, pFile);

                    bSuccess = true;
                }
            }
        }

        fclose(pFile);
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTGALoader::GetPixel

unsigned char* CTGALoader::GetPixel(st_int32 nWidth, st_int32 nHeight) const
{
    st_uchar* pPixel = NULL;

    if (m_pImageData != NULL)
        pPixel = m_pImageData + m_nDepth * (m_nWidth * nHeight + nWidth);

    return pPixel;
}


///////////////////////////////////////////////////////////////////////  
//  CTGALoader::SetPixel

void CTGALoader::SetPixel(const st_uchar* pValue, st_int32 nWidth, st_int32 nHeight)
{
    if (m_pImageData != NULL)
        memcpy(m_pImageData + m_nDepth * (m_nWidth * nHeight + nWidth), pValue, m_nDepth);
}
