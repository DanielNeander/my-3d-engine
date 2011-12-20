/**************************************************************************************************
module	:	U2VertexAttributes
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_ATTRIBUTE_H
#define U2_ATTRIBUTE_H

#include <U2Lib/src/U2RefObject.h>
#include <U2Lib/src/U2SmartPtr.h>
#include <U2Lib/Src/U2TVec.h>

class U2_3D U2VertexAttributes : public U2MemObj 
{
	
public:
	// Set the attributes you want.  The currently supported attributes are
	//    position:               1 to 4 channels
	//    blendweight:            1 to 4 channels
	//    normal vector:          1 to 4 channels
	//    color[i], 0 <= i < M:   1 to 4 channels (M = 2 for now)
	//    fog:                    1 channel
	//    psize:                  1 channel
	//    blendindices:           1 to 4 channels
	//    tcoord[i], 0 <= i < M:  1 to 4 channels (M depends on shader model)
	//    tangent vector:         1 to 4 channels
	//    bitangent vector:       1 to 4 channels
	//
	// The attrbitues are organized internally according to the
	// aforementioned list.


	U2VertexAttributes();
	~U2VertexAttributes();

	int GetChannelCnt();

	void SetPositionChannels (int iPositionChannels);
	void SetBlendWeightChannels (int iBlendWeightChannels);
	void SetNormalChannels (int iNormalChannels);
	void SetColorChannels (int iUnit, int iColorChannels);
	void SetFogChannels (int iFogChannels);
	void SetPSizeChannels (int iPSizeChannels);
	void SetBlendIndicesChannels (int iBlendIndicesChannels);
	void SetTCoordChannels (int iUnit, int iTCoordChannels);
	void SetTangentChannels (int iTangentChannels);
	void SetBitangentChannels (int iBitangentChannels);

	// Access to position information.
	int GetPositionOffset () const;
	int GetPositionChannels () const;
	bool HasPosition () const;

	// Access to blend-weight information.
	int GetBlendWeightOffset () const;
	int GetBlendWeightChannels () const;
	bool HasBlendWeight () const;

	// Access to normal information.
	int GetNormalOffset () const;
	int GetNormalChannels () const;
	bool HasNormal () const;

	// Access to color information.
	int GetMaxColors () const;
	int GetColorOffset (int unit) const;
	int GetColorChannels (int unit) const;
	bool HasColor (int unit) const;

	// Access to fog information.
	int GetFogOffset () const;
	int GetFogChannels () const;
	bool HasFog () const;

	// Access to psize information.
	int GetPSizeOffset () const;
	int GetPSizeChannels () const;
	bool HasPSize () const;

	// Access to blend-indices information.
	int GetBlendIndicesOffset () const;
	int GetBlendIndicesChannels () const;
	bool HasBlendIndices () const;

	// Access to texture coordinate information.
	int GetMaxTCoords () const;
	int GetTCoordOffset (int iUnit) const;
	int GetTCoordChannels (int iUnit) const;
	bool HasTCoord (int iUnit) const;

	// Access to tangent information.
	int GetTangentOffset () const;
	int GetTangentChannels () const;
	bool HasTangent () const;

	// Access to bitangent information.
	int GetBitangentOffset () const;
	int GetBitangentChannels () const;
	bool HasBitangent () const;

	// Support for comparing vertex program outputs with pixel program inputs.
	bool Matches (const U2VertexAttributes& lhs, bool bIncludePosition,
		bool bIncludeBlendWeight, bool bIncludeNormal, bool bIncludeColor,
		bool bIncludeFog, bool bIncludePSize, bool bIncludeBlendIndices,
		bool bIncludeTCoord, bool bIncludeTangent, bool bIncludeBitangent)
		const;


	bool operator== (const U2VertexAttributes& lhs) const;
	bool operator!= (const U2VertexAttributes& lhs) const;

protected:
	void UpdateOffsets();

	int m_iChannelCnt;
	int m_iElemsCnt;

	int m_iPositionOffset;
	int m_iPositionChannels;

	int m_iBlendWeightOffset;
	int m_iBlendWeightChannels;

	int m_iNormalOffset;
	int m_iNormalChannels;

	U2PrimitiveVec<int> m_colorOffsetArray;
	U2PrimitiveVec<int> m_colorChannelArray;

	int m_iFogOffset;
	int m_iFogChannels;

	int m_iPSizeOffset;
	int m_iPSizeChannels;

	int m_iBlendIndicesOffset;
	int m_iBlendIndicesChannels;

	U2PrimitiveVec<int> m_texCoordOffsetArray;
	U2PrimitiveVec<int> m_texCoordChannelArray;

	int m_iTangentOffset;
	int m_iTangentChannels;

	int m_iBitangentOffset;
	int m_iBitangentChannels;	
};

#include "U2VertexAttributes.inl"

#endif 