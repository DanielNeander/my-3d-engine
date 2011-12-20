#include <U2_3D/src/U23DLibPCH.h>
#include "U2VertexAttributes.h"


U2VertexAttributes::U2VertexAttributes()
{
	m_iChannelCnt = 0;
	m_iElemsCnt = 0;
	m_iPositionOffset = -1;
	m_iPositionChannels = 0;
	m_iNormalOffset = -1;
	m_iNormalChannels = 0;
	m_iBlendWeightOffset = -1;
	m_iBlendWeightChannels = 0;
	m_iFogOffset = -1;
	m_iFogChannels = 0;
	m_iPSizeOffset = -1;
	m_iPSizeChannels = 0;
	m_iBlendIndicesOffset = -1;
	m_iBlendIndicesChannels = 0;
	m_iTangentOffset = -1;
	m_iTangentChannels = 0;
	m_iBitangentOffset = -1;
	m_iBitangentChannels = 0;	
}


U2VertexAttributes::~U2VertexAttributes()
{

}


bool U2VertexAttributes::Matches (const U2VertexAttributes& rkAttr, bool bIncludePosition,
						  bool bIncludeBlendWeight, bool bIncludeNormal, bool bIncludeColor,
						  bool bIncludeFog, bool bIncludePSize, bool bIncludeBlendIndices,
						  bool bIncludeTCoord, bool bIncludeTangent, bool bIncludeBitangent) const
{
	int i;

	if (bIncludePosition)
	{
		if (m_iPositionChannels != rkAttr.m_iPositionChannels)
		{
			return false;
		}
	}

	if (bIncludeBlendWeight)
	{
		if (m_iBlendWeightChannels != rkAttr.m_iBlendWeightChannels)
		{
			return false;
		}
	}

	if (bIncludeNormal)
	{
		if (m_iNormalChannels != rkAttr.m_iNormalChannels)
		{
			return false;
		}
	}

	if (bIncludeColor)
	{
		if (m_colorChannelArray.FilledSize() != rkAttr.m_colorChannelArray.FilledSize())
		{
			return false;
		}
		for (i = 0; i < (int)m_colorChannelArray.FilledSize(); i++)
		{
			if (m_colorChannelArray.GetElem(i) != rkAttr.m_colorChannelArray.GetElem(i))
			{
				return false;
			}
		}
	}

	if (bIncludeFog)
	{
		if (m_iFogChannels != rkAttr.m_iFogChannels)
		{
			return false;
		}
	}

	if (bIncludePSize)
	{
		if (m_iPSizeChannels != rkAttr.m_iPSizeChannels)
		{
			return false;
		}
	}

	if (bIncludeBlendIndices)
	{
		if (m_iBlendIndicesChannels != rkAttr.m_iBlendIndicesChannels)
		{
			return false;
		}
	}

	if (bIncludeTCoord)
	{
		if (m_texCoordChannelArray.FilledSize() != rkAttr.m_texCoordChannelArray.FilledSize())
		{
			return false;
		}
		for (i = 0; i < (int)m_texCoordChannelArray.FilledSize(); i++)
		{
			if (m_texCoordChannelArray.GetElem(i) != rkAttr.m_texCoordChannelArray.GetElem(i))
			{
				return false;
			}
		}
	}

	if (bIncludeTangent)
	{
		if (m_iTangentChannels != rkAttr.m_iTangentChannels)
		{
			return false;
		}
	}

	if (bIncludeBitangent)
	{
		if (m_iBitangentChannels != rkAttr.m_iBitangentChannels)
		{
			return false;
		}
	}

	return true;
}


void U2VertexAttributes::UpdateOffsets()
{
	uint32 i;

	m_iChannelCnt = 0;
	m_iPositionOffset = -1;
	m_iBlendWeightOffset = -1;
	m_iNormalOffset = -1;
	m_colorOffsetArray.Resize(m_colorChannelArray.FilledSize());
	for(i = 0; i < m_colorChannelArray.Size(); ++i)
	{
		m_colorOffsetArray.SetElem(i, -1);
	}
	m_iFogOffset = -1;
	m_iPSizeOffset = -1;
	m_iBlendIndicesOffset = -1;
	m_texCoordOffsetArray.Resize(m_texCoordChannelArray.FilledSize());
	for(i = 0; i < m_texCoordChannelArray.FilledSize(); ++i)
	{
		m_texCoordOffsetArray.SetElem(i, -1);
	}

	m_iTangentOffset = -1;
	m_iBitangentOffset = -1;

	if(m_iPositionChannels > 0)
	{
		m_iPositionOffset = m_iChannelCnt;
		m_iChannelCnt += m_iPositionChannels;
		++m_iElemsCnt;
	}

	if(m_iBitangentChannels > 0)
	{
		m_iBitangentOffset = m_iChannelCnt;
		m_iChannelCnt += m_iBitangentChannels;
		++m_iElemsCnt;
	}

	if(m_iNormalChannels > 0)
	{
		m_iNormalOffset = m_iChannelCnt;
		m_iChannelCnt += m_iNormalChannels;
		++m_iElemsCnt;
	}

	for(i = 0; i < m_colorChannelArray.FilledSize(); ++i)
	{
		if(m_colorChannelArray.GetElem(i) > 0)
		{
			m_colorOffsetArray.SetElem(i, m_iChannelCnt);
			//m_iChannelCnt += m_colorChannelArray.GetElem(i);
			m_iChannelCnt += 1; // sizeof(DWORD)
		}
	}

	if(m_iFogChannels > 0)
	{
		m_iFogChannels = m_iChannelCnt;
		m_iChannelCnt += m_iFogChannels;
	}

	if(m_iPSizeChannels > 0)
	{
		m_iPSizeOffset = m_iChannelCnt;
		m_iChannelCnt += m_iPSizeChannels;
	}

	if(m_iBlendIndicesChannels > 0)
	{
		m_iBlendIndicesOffset = m_iChannelCnt;
		m_iChannelCnt += m_iBlendIndicesChannels;
	}

	for(i=0; i < m_texCoordChannelArray.FilledSize(); ++i)
	{
		if(m_texCoordChannelArray.GetElem(i) > 0)
		{
			m_texCoordOffsetArray.SetElem(i, m_iChannelCnt);
			m_iChannelCnt += m_texCoordChannelArray.GetElem(i);
			
		}
	}

	if(m_iTangentChannels > 0)
	{
		m_iTangentOffset = m_iChannelCnt;
		m_iChannelCnt += m_iTangentChannels;
	}

	if(m_iBitangentChannels > 0)
	{
		m_iBitangentOffset = m_iChannelCnt;
		m_iChannelCnt += m_iBitangentChannels;
	}
}


bool U2VertexAttributes::operator== (const U2VertexAttributes& lhs) const
{
	return true;
}


bool U2VertexAttributes::operator!= (const U2VertexAttributes& lhs) const
{
	return !operator==(lhs);
}