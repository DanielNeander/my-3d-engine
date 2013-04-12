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
#include "BitStream.h"
#include <vector>

// Class storing Restricted Quad Tree triangulation enabling flags
// Vertex is enabled if it is included into the triangulation and disabeld otherwise
class CRQTVertsEnabledFlags
{
public:
    CRQTVertsEnabledFlags() : m_iPatchSize(0){}

    // Initializes the storage
    void Init(int iPatchSize, char bInitialEnableValue = TRUE);

    // Tests if specified vertex is enabled
    char IsVertexEnabled(int iX, int iY)const{return m_Flags[iX + iY*(m_iPatchSize+1)];}
    // Sets the specified vertex enabled flag
    void SetVertexEnabledFlag(int iX, int iY, char bFlag){m_Flags[iX + iY*(m_iPatchSize+1)] = bFlag;}

    const CRQTVertsEnabledFlags& operator = (const CRQTVertsEnabledFlags& RQTFlags);

    // Saves data to file
    void SaveToFile(FILE *pFile);

    // Loads data from the file
    HRESULT LoadFromFile(FILE *pFile);

    int GetPatchSize()const{return m_iPatchSize;}

private:
    // std::vector<bool> is very slow. Use char instead
    std::vector<char> m_Flags;
    int m_iPatchSize;
};

// Class implementing Restricted Quad Tree triangulation
class CRQTTriangulation
{
public:
    // This constructor initializes the object to encode the triangulation
    CRQTTriangulation(const SQuadTreeNodeLocation &pos,
                      const CRQTVertsEnabledFlags &EnabledFlags,
                      int iNumLevelsInHierarchy,
                      CBitStream* pEncodedRQTBitStream = NULL);
    
    // This constructor initializes the object to build the triangulation using encoded bitstream
    CRQTTriangulation(const SQuadTreeNodeLocation &pos,
                      int iNumLevelsInHierarchy,
                      CBitStream *pEncodedRQTBitStream,
                      int iPatchSize);

    ~CRQTTriangulation(void);

    void GetPos( SQuadTreeNodeLocation &outPos );

    // If pEnabledFlags != NULL, the method generates indices and encodes them
    // into the encoded bit stream (m_EncodedRQTBitStream). Otherwise it reads the data 
    // from the bit stream
    void GenerateIndices( int iElevDataBoundaryExtension,
                          UINT *puiIndices,
                          UINT &uiNumIndicesGenerated );
    
    // Enables encoding mode. In this mode labels are output to the specified bit stream
    void SetEncodingMode(CBitStream *pEncodedRQTBitStream){m_pEncodedRQTBitStream = pEncodedRQTBitStream; m_bIsEncodingMode = true;}

private:
    CRQTTriangulation(const CRQTTriangulation &Triang);

    int m_iNumLevelsInLocalPatchQT;
    int m_iElevDataBoundaryExtension;

    enum RQT_TRIANG_ORIENTATION
    {
        ORIENT_L, //    /|
                  //    \|

        ORIENT_R, //   |\
                  //   |/

        ORIENT_B, //   --
                  //   \/

        ORIENT_T, //   /\
                  //   --
        
        ORIENT_LB,/*   |\    */
                  /*   |_\   */

        ORIENT_RB,/*    /|   */
                  /*   /_|   */

                  /*    __   */
        ORIENT_LT,/*   | /   */
                  /*   |/    */
 
        ORIENT_RT /*   __    */
                  /*   \ |   */
                  /*    \|   */
    };

    // Generates indices from the enbaled flags
    void RecursiveGenerateIndices(int iTrianleBaseX, int iTriangleBaseY, int iLevel,
                                  RQT_TRIANG_ORIENTATION Orientation,              
                                  int iElevDataBoundaryExtension,
                                  UINT* &puiCurrIndex);
    // Reads next flag from the bit stream
    char ReadNextFlag(){return m_pEncodedRQTBitStream->ReadBit() ? TRUE : FALSE;}
    
    // Writes flag to the output bit stream
    void WriteEnabledFlag(char bEnabledFlag){ m_pEncodedRQTBitStream->WriteBit(bEnabledFlag ? 1 : 0);}

    // Depending on the current mode encodes or decodes enabled flag for the specified vertex
    void EncodeDecodeEnabledFlag(bool &bBaseVertexEnabled, int iTriangleBaseX, int iTriangleBaseY);

    // Output triangle with the specified indices to the output index buffer
    void DefineTriangle(UINT* &puiIndices, int iX1, int iY1, int iX2, int iY2, int iX3, int iY3)const;

    SQuadTreeNodeLocation m_pos;
    int m_iNumLevelsInHierarchy;
    bool m_bIsEncodingMode;

    CRQTVertsEnabledFlags m_EnabledFlags;
    
    CBitStream *m_pEncodedRQTBitStream;

    friend class CTriangDataSource;
};

// Pack vertex indices into single UINT32 value
inline 
UINT CalculatePackedIndex(int iVertXInd, int iVertYInd, int iLevel, 
                          int iNumLevelsInQuadTree,
                          int iElevDataBoundaryExtension)
{
    iVertXInd <<= (iNumLevelsInQuadTree-1 - iLevel);
    iVertXInd += iElevDataBoundaryExtension;
    iVertXInd = max(iVertXInd, 0);

    iVertYInd <<= (iNumLevelsInQuadTree-1 - iLevel);
    iVertYInd += iElevDataBoundaryExtension;
    iVertYInd = max(iVertYInd, 0);

    return (iVertXInd & 0x0FFFF) | ((iVertYInd & 0x0FFFF) << 16);
}

// Pack vertex indices from the single UINT32 value    
inline 
void UnpackIndices(UINT uiPackedIndex,
                   int &iVertXInd, int &iVertYInd,
                   int iElevDataBoundaryExtension)
{
    iVertXInd = (int)(uiPackedIndex & 0x0FFFF) - iElevDataBoundaryExtension;
    iVertYInd = (int)( (uiPackedIndex >> 16) & 0x0FFFF) - iElevDataBoundaryExtension;
}
