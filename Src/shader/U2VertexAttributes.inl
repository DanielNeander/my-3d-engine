inline 
int U2VertexAttributes::GetChannelCnt()
{
	return m_iChannelCnt;
}


inline 
int U2VertexAttributes::GetPositionOffset () const
{
	return m_iPositionOffset;
}


inline 
int U2VertexAttributes::GetPositionChannels () const
{
	return m_iPositionChannels;
}


inline 
bool U2VertexAttributes::HasPosition () const
{
	return m_iPositionChannels > 0;
}


inline 
int U2VertexAttributes::GetBlendWeightOffset () const
{
	return m_iBlendWeightOffset;
}


inline 
int U2VertexAttributes::GetBlendWeightChannels () const
{
	return m_iBlendWeightChannels;
}


inline 
bool U2VertexAttributes::HasBlendWeight () const
{
	return m_iBlendWeightChannels > 0;
}


inline 
int U2VertexAttributes::GetNormalOffset () const
{
	return m_iNormalOffset;
}


inline 
int U2VertexAttributes::GetNormalChannels () const
{
	return m_iNormalChannels;
}


inline 
bool U2VertexAttributes::HasNormal () const
{
	return m_iNormalChannels > 0;
}


inline 
int U2VertexAttributes::GetMaxColors () const
{
	return m_colorChannelArray.FilledSize();
}


inline 
int U2VertexAttributes::GetColorOffset (int iUnit) const
{
	if (0 <= iUnit && iUnit < (int)m_colorOffsetArray.FilledSize())
	{
		return m_colorOffsetArray.GetElem(iUnit);
	}
	return -1;	
}


inline 
int U2VertexAttributes::GetColorChannels (int iUnit) const
{
	if (0 <= iUnit && iUnit < (int)m_colorChannelArray.FilledSize())
	{
		return m_colorChannelArray.GetElem(iUnit);
	}
	return 0;

}


inline 
bool U2VertexAttributes::HasColor (int iUnit) const
{
	if (0 <= iUnit && iUnit < (int)m_colorChannelArray.FilledSize())
	{
		return m_colorChannelArray.GetElem(iUnit) > 0;
	}
	return false;
}


inline 
int U2VertexAttributes::GetFogOffset () const
{
	return m_iFogOffset;
}


inline 
int U2VertexAttributes::GetFogChannels () const
{
	return m_iFogChannels;
}


inline 
bool U2VertexAttributes::HasFog () const
{
	return m_iFogChannels > 0;	
}


inline 
int U2VertexAttributes::GetPSizeOffset () const
{
	return m_iPSizeOffset;
}


inline 
int U2VertexAttributes::GetPSizeChannels () const
{
	return m_iPSizeChannels;
}


inline 
bool U2VertexAttributes::HasPSize () const
{
	return m_iPSizeChannels > 0;
}


inline 
int U2VertexAttributes::GetBlendIndicesOffset () const
{
	return m_iBlendIndicesOffset;
}


inline 
int U2VertexAttributes::GetBlendIndicesChannels () const
{
	return m_iBlendIndicesChannels;
}


inline 
bool U2VertexAttributes::HasBlendIndices () const
{
	return m_iBlendIndicesChannels > 0;
}


inline 
int U2VertexAttributes::GetMaxTCoords () const
{
	return m_texCoordChannelArray.FilledSize();
}


inline 
int U2VertexAttributes::GetTCoordOffset (int iUnit) const
{
	if (0 <= iUnit && iUnit < (int)m_texCoordOffsetArray.FilledSize())
	{
		return m_texCoordOffsetArray.GetElem(iUnit);
	}
	return -1;
}


inline
int U2VertexAttributes::GetTCoordChannels (int iUnit) const
{
	if (0 <= iUnit && iUnit < (int)m_texCoordChannelArray.FilledSize())
	{
		return m_texCoordChannelArray.GetElem(iUnit);
	}
	return 0;
}


inline 
bool U2VertexAttributes::HasTCoord (int iUnit) const
{
	if (0 <= iUnit && iUnit < (int)m_texCoordChannelArray.FilledSize())
	{
		return m_texCoordChannelArray.GetElem(iUnit) > 0;
	}
	return false;
}


inline 
int U2VertexAttributes::GetTangentOffset () const
{
	return m_iTangentOffset;
}


inline 
int U2VertexAttributes::GetTangentChannels () const
{
	return m_iTangentChannels;
}


inline 
bool U2VertexAttributes::HasTangent () const
{
	return m_iTangentChannels > 0;
}


inline 
int U2VertexAttributes::GetBitangentOffset () const
{
	return m_iBitangentOffset;
}


inline 
int U2VertexAttributes::GetBitangentChannels () const
{
	return m_iBitangentChannels;
}


inline 
bool U2VertexAttributes::HasBitangent () const
{
	return m_iBitangentChannels > 0;
}


inline 
void U2VertexAttributes::SetPositionChannels (int uPositionChannels)
{
	U2ASSERT(0 <= uPositionChannels && uPositionChannels <= 4);

	m_iPositionChannels = uPositionChannels;
	UpdateOffsets();
}


inline 
void U2VertexAttributes::SetBlendWeightChannels (int uBlendWeightChannels)
{
	U2ASSERT(0 <= uBlendWeightChannels && uBlendWeightChannels <= 4);
	
	m_iBlendWeightChannels = uBlendWeightChannels;
	UpdateOffsets();
}


inline 
void U2VertexAttributes::SetNormalChannels (int iNormalChannels)
{
	U2ASSERT(0 <= iNormalChannels && iNormalChannels <= 4);

	m_iNormalChannels = iNormalChannels;
	UpdateOffsets();
}


inline 
void U2VertexAttributes::SetColorChannels (int iUnit, int iColorChannels)
{
	U2ASSERT(0 <= iUnit && 0 <= iColorChannels && iColorChannels <= 4);

	int iMaxUnits = (int)m_colorChannelArray.FilledSize();
	if (iUnit >= iMaxUnits)
	{
		m_colorChannelArray.Resize(iUnit+1);
	}

	m_colorChannelArray.SetElem(iUnit,iColorChannels);
	UpdateOffsets();

}


inline 
void U2VertexAttributes::SetFogChannels (int iFogChannels)
{
	U2ASSERT(0 <= iFogChannels && iFogChannels <= 1);

	m_iFogChannels = iFogChannels;
	UpdateOffsets();
}


inline 
void U2VertexAttributes::SetPSizeChannels (int iPSizeChannels)
{
	U2ASSERT(0 <= iPSizeChannels && iPSizeChannels <= 1);

	m_iPSizeChannels = iPSizeChannels;
	UpdateOffsets();
}


inline 
void U2VertexAttributes::SetBlendIndicesChannels (int iBlendIndicesChannels)
{

	U2ASSERT(0 <= iBlendIndicesChannels && iBlendIndicesChannels <= 4);

	m_iBlendIndicesChannels = iBlendIndicesChannels;
	UpdateOffsets();
}


inline 
void U2VertexAttributes::SetTCoordChannels (int iUnit, int iTCoordChannels)
{
	U2ASSERT(0 <= iUnit && 0 <= iTCoordChannels && iTCoordChannels <= 4);

	int iMaxUnits = (int)m_texCoordChannelArray.FilledSize();
	if (iUnit >= iMaxUnits)
	{
		m_texCoordChannelArray.Resize(iUnit+1);
	}

	m_texCoordChannelArray.SetElem(iUnit, iTCoordChannels);
	UpdateOffsets();
}


inline 
void U2VertexAttributes::SetTangentChannels (int iTangentChannels)
{
	U2ASSERT(0 <= iTangentChannels && iTangentChannels <= 4);

	m_iTangentChannels = iTangentChannels;
	UpdateOffsets();
}


inline 
void U2VertexAttributes::SetBitangentChannels (int iBitangentChannels)
{
	U2ASSERT(0 <= iBitangentChannels && iBitangentChannels <= 4);

	m_iBitangentChannels = iBitangentChannels;
	UpdateOffsets();
}
