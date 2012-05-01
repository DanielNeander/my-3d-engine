/**************************************************************************************************
module	:	U2ShaderSamperInfo
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_SHADERSAMPLERINFO_H
#define U2_SHADERSAMPLERINFO_H

#include <U2Lib/Src/U2RefObject.h>


class U2_3D U2ShaderSamplerInfo : public U2RefObject
{
public:
	enum SamplerType 
	{
		SAMPLER_INVALID = 0,
		SAMPLER_1D,
		SAMPLER_2D,
		SAMPLER_3D,
		SAMPLER_CUBE,
		SAMPLER_PROJ,
		MAX_SAMPLER_COUNT		
	};
	
	U2ShaderSamplerInfo(const U2String& name, SamplerType eSampler, int iTexUnit);
	~U2ShaderSamplerInfo();

	const U2String& GetName() const;
	SamplerType GetSamplerType() const;
	int GetTextureUnit() const;
	int GetDimension() const;

private:
	U2String m_strName;
	SamplerType m_eSampler;
	int m_iTextureUnit;
	unsigned short  m_usSamplerDims;
};

#include "U2ShaderSamplerInfo.inl"

typedef U2SmartPtr<U2ShaderSamplerInfo> U2ShaderSamplerInfoPtr;

#endif 