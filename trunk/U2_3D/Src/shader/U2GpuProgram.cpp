#include <U2_3D/src/U23DLibPCH.h>
#include <U2Lib/src/U2Util.h>

#include "U2GpuProgram.h"
#include <fstream>

U2String U2GpuProgram::ms_strRenderType(_T("dx9"));
TCHAR U2GpuProgram::ms_cCommentChar= 47;

const U2String U2GpuProgram::ms_strFloat(_T("float"));
const U2String U2GpuProgram::ms_strFloat1(_T("float1"));
const U2String U2GpuProgram::ms_strFloat1x1(_T("float1x1"));
const U2String U2GpuProgram::ms_strFloat1x2(_T("float1x2"));
const U2String U2GpuProgram::ms_strFloat1x3(_T("float1x3"));
const U2String U2GpuProgram::ms_strFloat1x4(_T("float1x4"));	
const U2String U2GpuProgram::ms_strFloat2(_T("float2"));
const U2String U2GpuProgram::ms_strFloat2x1(_T("float2x1"));
const U2String U2GpuProgram::ms_strFloat2x2(_T("float2x2"));
const U2String U2GpuProgram::ms_strFloat2x3(_T("float2x3"));
const U2String U2GpuProgram::ms_strFloat2x4(_T("float2x4"));
const U2String U2GpuProgram::ms_strFloat3(_T("float3"));
const U2String U2GpuProgram::ms_strFloat3x1(_T("float3x1"));
const U2String U2GpuProgram::ms_strFloat3x2(_T("float3x2"));
const U2String U2GpuProgram::ms_strFloat3x3(_T("float3x3"));
const U2String U2GpuProgram::ms_strFloat3x4(_T("float3x4"));
const U2String U2GpuProgram::ms_strFloat4(_T("float4")); 
const U2String U2GpuProgram::ms_strFloat4x1(_T("float4x1"));
const U2String U2GpuProgram::ms_strFloat4x2(_T("float4x2"));
const U2String U2GpuProgram::ms_strFloat4x3(_T("float4x3"));
const U2String U2GpuProgram::ms_strFloat4x4(_T("float4x4"));
const U2String U2GpuProgram::ms_strSampler1D(_T("sampler1D"));
const U2String U2GpuProgram::ms_strSampler2D(_T("sampler2D"));
const U2String U2GpuProgram::ms_strSampler3D(_T("sampler3D"));
const U2String U2GpuProgram::ms_strSamplerCube(_T("samplerCUBE"));
const U2String U2GpuProgram::ms_strSamplerProj(_T("sampler2DSHADOW"));
const U2String U2GpuProgram::ms_strPosition(_T("POSITION"));
const U2String U2GpuProgram::ms_strBlendWeight(_T("BLENDWEIGHT"));
const U2String U2GpuProgram::ms_strNormal(_T("NORMAL"));
const U2String U2GpuProgram::ms_strColor(_T("COLOR"));
const U2String U2GpuProgram::ms_strColor0(_T("COLOR0"));
const U2String U2GpuProgram::ms_strColor1(_T("COLOR1"));
const U2String U2GpuProgram::ms_strFogCoord(_T("FOGCOORD"));
const U2String U2GpuProgram::ms_strFogStr(_T("FOG"));
const U2String U2GpuProgram::ms_strPSize(_T("PSIZE"));
const U2String U2GpuProgram::ms_strBlendindices(_T("BLENDINDICES"));
const U2String U2GpuProgram::ms_strTexcoord(_T("TEXCOORD"));
const U2String U2GpuProgram::ms_strTangent(_T("TANGENT"));
const U2String U2GpuProgram::ms_strBinormal(_T("BINORMAL"));
const U2String U2GpuProgram::ms_strIn(_T("in"));
const U2String U2GpuProgram::ms_strEOL(_T("\n"));

#define PARSE_ERROR U2ASSERT(false); return false 



IMPLEMENT_RTTI(U2GpuProgram, U2Object);

U2GpuProgram* U2GpuProgram::ms_pHead;
U2GpuProgram* U2GpuProgram::ms_pTail; 

//-------------------------------------------------------------------------------------------------
U2GpuProgram* U2GpuProgram::GetHead()
{
	return ms_pHead;
}

//-------------------------------------------------------------------------------------------------
U2GpuProgram* U2GpuProgram::GetTail()
{
	return ms_pTail;
}

//-------------------------------------------------------------------------------------------------
U2GpuProgram* U2GpuProgram::GetNext()
{
	return m_pNext;
}

//-------------------------------------------------------------------------------------------------
U2GpuProgram* U2GpuProgram::GetPrev()
{
	return m_pPrev;
}

//-------------------------------------------------------------------------------------------------
void  U2GpuProgram::AddProgramToList()
{
	if(!ms_pHead)
		ms_pHead = this;
	if(ms_pTail)
	{
		ms_pTail->m_pNext = this;
		m_pPrev = ms_pTail;
	}
	else 
	{
		m_pPrev = 0;
	}
	ms_pTail = this;
	m_pNext = 0;
}

//-------------------------------------------------------------------------------------------------
void  U2GpuProgram::RemoveProgramFromList()
{
	if(ms_pHead == this)
		ms_pHead = m_pNext;
	if(ms_pTail == this)
		ms_pTail = m_pPrev;

	if(m_pPrev)
		m_pPrev->m_pNext = m_pNext;
	if(m_pNext)
		m_pNext->m_pPrev = m_pPrev;
}

//-------------------------------------------------------------------------------------------------
bool U2GpuProgram::LoadAndParse(const U2String& szFilename, U2GpuProgram* pProgram)
{
	if(!pProgram)
		return false;
#ifdef UNICODE 
	std::wifstream ifs(szFilename.c_str());
#else 
	std::ifstream ifs(szFilename.c_str());
#endif
	if(!ifs)
	{
		return false;
	}
	
	FILE_LOG(logDEBUG) << _T("File ") << szFilename.c_str() << _T(" Opened");
	U2String& programText = pProgram->m_strProgramText;
	U2VertexAttributes& IAttr = pProgram->m_inputVertAtts;
	U2VertexAttributes& OAttr = pProgram->m_outputvertAtts;


	U2ObjVec<U2ShaderConstantPtr>* pRCs = &pProgram->m_shaderConstants;
	U2ObjVec<U2NumericalConstantPtr>* pNCs = &pProgram->m_numericalConstants;
	U2ObjVec<U2ShaderUserConstantPtr>* pUCs = &pProgram->m_userConstants;
	U2ObjVec<U2ShaderSamplerInfoPtr>* pSIs = &pProgram->m_samplerInfos;

	U2String line, token, varType, varName, varIO, varSemantic;
	U2String kRegister, kData, kUnit;
	U2String::size_type begin, end, save;
	uint32 numFloats, uUnit, baseRegister, registerCnt;
	U2ShaderSamplerInfo::SamplerType eSType;
	U2ShaderConstant::ShaderConstantMapping eSCM;
	
	while(!ifs.eof())
	{
		getline(ifs, line);

		// Depending on the platform that *.wmsp files are compiled on, the
		// newline may consist of a line feed '\n' (10), a carriage return
		// '\r' (13), or a carriage return followed by a line feed.

		if(line.length() > 0)
		{
			if(10 == line[line.length()-1])
			{
				if(line.length() > 1 && 13 == (line[line.length()-2]))
					line.resize(line.length()-2);
				else
					line.resize(line.length()-1);
			}		
			else if(13 == (int)line[line.length()-1])
				line.resize(line.length()-1);
		}

		// The information needed by Program is contained in the shader
		// program comment lines.  All other lines are assumed to be needed
		// by the graphics API.

		if(line[0] != ms_cCommentChar)
		{
			programText.append(line.c_str());						
			programText.append(ms_strEOL.c_str());
			
			FILE_LOG(logDEBUG) << _T("Line Added: ") << line.c_str() << ms_strEOL.c_str();
			continue;
		}

		begin = line.find(_T("var"), 1);
		if(begin != U2String::npos)
		{
			// Skip over "var"
			begin += 3;

			// Skip over white space
			begin = line.find_first_not_of(_T(" "), begin);
			if(begin == U2String::npos)
			{
				U2ASSERT(false);
				return false;
			}

			// Get the variable's data type.
			numFloats = 0;
			eSType = U2ShaderSamplerInfo::MAX_SAMPLER_COUNT;
			end = line.find(_T(" "), begin);
			varType = line.substr(begin, end-begin);			
			if(varType == ms_strFloat)
				numFloats = 1;
			else if(varType == ms_strFloat2)
				numFloats = 2;
			else if(varType == ms_strFloat3)
				numFloats = 3;
			else if(varType == ms_strFloat4)
				numFloats = 4;
			else if(varType == ms_strFloat4x4)
				numFloats = 16;
			else if(varType == ms_strSampler1D)
				eSType = U2ShaderSamplerInfo::SAMPLER_1D;
			else if(varType == ms_strSampler2D)
				eSType = U2ShaderSamplerInfo::SAMPLER_2D;
			else if(varType == ms_strSampler3D)
				eSType = U2ShaderSamplerInfo::SAMPLER_3D;
			else if(varType == ms_strSamplerCube)
				eSType = U2ShaderSamplerInfo::SAMPLER_CUBE;
			else if(varType == ms_strSamplerProj)
				eSType = U2ShaderSamplerInfo::SAMPLER_PROJ;
			else 
			{
				U2ASSERT(false);
				return false;
			}

			FILE_LOG(logDEBUG) << _T("var Type: ") << varType.c_str() << 
				_T("numFloat : ") << numFloats;

			// Skip over white space.
			begin = line.find_first_not_of(_T(" "), end);
			if(begin == U2String::npos)
			{
				U2ASSERT(false);
				return false;
			}

			// Get the variable's name.
			end = line.find(_T(" "), begin);
			varName = line.substr(begin, end-begin);

			// Skip over white space.
			begin = line.find_first_not_of(_T(" "), end);
			if(begin == U2String::npos)
			{
				U2ASSERT(false);
				return false;
			}

			FILE_LOG(logDEBUG) << _T("var Name: ") << varName.c_str();				

			 // Get sampler information (if relevant).
			if(eSType != U2ShaderSamplerInfo::MAX_SAMPLER_COUNT)
			{
				begin = line.find(_T("texunit"), begin);
				if(begin == U2String::npos)
				{
					U2ASSERT(false);
					return false;
				}
				begin += 7;
				if(line[begin] != _T(' '))
				{
					PARSE_ERROR;
				}

				// Get the texture unit.
				begin = line.find_first_of(_T("0123456789"), begin);
				end = line.find_first_not_of(_T("0123456789"), begin);
				kUnit = line.substr(begin, end -begin);
				uUnit = _tstoi(kUnit.c_str());
				if(uUnit < 0 || uUnit >= 16)
				{
					PARSE_ERROR;
				}

				FILE_LOG(logDEBUG) << _T("Texture Unit: ") << kUnit.c_str();					
				
				pSIs->AddElem(U2_NEW U2ShaderSamplerInfo(varName, eSType, uUnit));
				continue;
			}

			// Get the variable's I/O status.
			save = begin;
			begin = line.find(_T("$"), save);
			if(begin != U2String::npos)
			{
				// The variable is either an input or output variable.

				begin += 2;
				end = line.find_first_of(_T("."), begin);
				varIO = line.substr(begin, end-begin);

				// Get the variable's semantic.

				begin = end+1;
				end = line.find_first_of(_T(" "), begin);
				varSemantic = line.substr(begin, end-begin);

				if(varIO == ms_strIn)
				{
					if(varSemantic == ms_strPosition)
					{
						IAttr.SetPositionChannels(numFloats);
					}
					else if(varSemantic == ms_strBlendWeight)
					{
						IAttr.SetBlendWeightChannels(numFloats);
					}
					else if(varSemantic == ms_strNormal)
						IAttr.SetNormalChannels(numFloats);
					else if(varSemantic == ms_strColor
						|| varSemantic == ms_strColor0)
					{
						IAttr.SetColorChannels(0, numFloats);
					}
					else if(varSemantic == ms_strColor1) 
					{
						IAttr.SetColorChannels(1, numFloats);
					}
					else if(varSemantic == ms_strFogCoord)
						IAttr.SetFogChannels(1);
					else if(varSemantic == ms_strPSize)
						IAttr.SetPSizeChannels(1);
					else if(varSemantic == ms_strBlendindices)
						IAttr.SetBlendIndicesChannels(numFloats);
					else if(varSemantic == ms_strTangent)
						IAttr.SetTangentChannels(numFloats);
					else if(varSemantic == ms_strBinormal)
						IAttr.SetBitangentChannels(numFloats);
					else 
					{
						if(varSemantic.substr(0, 8) != ms_strTexcoord)
						{
							PARSE_ERROR;
						}
						uUnit = (int)varSemantic[8] - _T('0');
						IAttr.SetTCoordChannels(uUnit, numFloats);
					}

					FILE_LOG(logDEBUG) << _T("IN var semantic: ") << 
						varSemantic  << _T("float num : ") << numFloats;
				}
				else 
				{
					if(varSemantic == ms_strPosition)
					{
						OAttr.SetPositionChannels(numFloats);
					}				
					else if(varSemantic == ms_strNormal)
					{
						OAttr.SetNormalChannels(numFloats);
					}
					else if(varSemantic == ms_strColor
						|| varSemantic == ms_strColor0)
					{
						OAttr.SetColorChannels(0, numFloats);
					}
					else if(varSemantic == ms_strColor1) 
					{
						OAttr.SetColorChannels(1, numFloats);
					}
					else if(varSemantic == ms_strFogCoord)
					{
						OAttr.SetFogChannels(1);
					}
					else if(varSemantic == ms_strPSize)
					{
						OAttr.SetPSizeChannels(1);				
					}
					else if(varSemantic == ms_strTangent)
					{
						OAttr.SetTangentChannels(numFloats);
					}
					else if(varSemantic == ms_strBinormal)
					{
						OAttr.SetBitangentChannels(numFloats);
					}
					else 
					{
						if(varSemantic.substr(0, 8) != ms_strTexcoord)
						{
							PARSE_ERROR;
						}
						uUnit = (int)varSemantic[8] - _T('0');
						OAttr.SetTCoordChannels(uUnit, numFloats);
					}

					FILE_LOG(logDEBUG) << _T("var semantic: ") << 
						varSemantic  << _T("float num : ") << numFloats;
				}
			}					
			else 
			{
				 // The variable is stored in a constant register.
				begin = line.find(_T("c["), save);
				if(begin == U2String::npos)
				{
					PARSE_ERROR;
				}
				end = line.find_first_of(_T("]"), begin);
				begin += 2;
				kRegister = line.substr(begin, end-begin);
				baseRegister = _tstoi(kRegister.c_str());

				registerCnt = uint32(numFloats * 0.25f);
				if(registerCnt == 0)
				{
					registerCnt = 1;
				}

				 // The variable is either a render state or user-defined.
				if(varName == _T("INVALID"))
				{
					int a= 1;
				}


				eSCM = U2ShaderConstant::GetSCM(varName.c_str());
				if(eSCM != U2ShaderConstant::MAX_SCM_COUNT)
				{
					
					
					pRCs->AddElem(U2_NEW U2ShaderConstant(eSCM, baseRegister, registerCnt));
				}
				else 
				{
					pUCs->AddElem(U2_NEW U2ShaderUserConstant(varName, baseRegister, registerCnt));
				}

				FILE_LOG(logDEBUG) << _T("Register Name : ") << varName << 
					_T("Base Register : ") << 
					kRegister  << _T("Register Count : ") << registerCnt
					<< _T(" SCM : ") << eSCM;

				continue;
			}
		}

		
		begin = line.find(_T("const"), 1);
		if(begin != U2String::npos)
		{
				// A numerical constant register has been found.

				begin = line.find(_T("c["));
				if(begin == U2String::npos)
				{
					PARSE_ERROR;
				}
				end = line.find_first_of(_T("]"), begin);
				begin += 2;
				kRegister = line.substr(begin, end -begin);
				baseRegister = _tstoi(kRegister.c_str());

				// Get the constant's data, which occurs after the equality.
				float afData[4] = {0.0f};
				begin =line.find(_T("="));
				if(begin == U2String::npos)
				{
					PARSE_ERROR;
				}
				begin++;
				if(begin == U2String::npos)
				{
					PARSE_ERROR;
				}
			begin = line.find_first_not_of(_T(" "), begin);
			if(begin == U2String::npos)
			{
				PARSE_ERROR;
			}

			for(int i = 0; i < 4; ++i)
			{
				end = line.find_first_of(_T(" "), begin);
				if(end == U2String::npos)
				{
						kData = line.substr(begin);
						afData[i] = (float)_tstof(kData.c_str());
						break;
				}
				kData= line.substr(begin, end -begin);
				afData[i] = (float)_tstof(kData.c_str());
				begin = line.find_first_not_of(_T(" "), end);
				if(begin == U2String::npos)
				{
					PARSE_ERROR;
				}

			}
			pNCs->AddElem(U2_NEW U2NumericalConstant(baseRegister, afData));

			FILE_LOG(logDEBUG) << _T("Numerical Base Register  : ") << baseRegister  				
				<< _T("Register Data : ") << afData;

		}	
	}

	ifs.close();	

	FILE_LOG(logDEBUG) << _T("File ") << szFilename.c_str() << _T(" Closed");

	return true;	
}

//-------------------------------------------------------------------------------------------------
U2GpuProgram::U2GpuProgram()
	:m_uiRegister(0)
{
	AddProgramToList();
}

//-------------------------------------------------------------------------------------------------
U2GpuProgram::~U2GpuProgram()
{
	m_shaderConstants.RemoveAll();
	m_numericalConstants.RemoveAll();
	m_userConstants.RemoveAll();
	m_samplerInfos.RemoveAll();

	RemoveProgramFromList();	
}

//-------------------------------------------------------------------------------------------------
const U2String& U2GpuProgram::GetProgramText() const
{
	return m_strProgramText;
}

//-------------------------------------------------------------------------------------------------
uint32 U2GpuProgram::GetShaderConstantCnt() const
{
	return m_shaderConstants.Size();
}


//----------------------------------------------------------------------------
U2ShaderConstant* U2GpuProgram::GetShaderConstant (int i)
{
	if (0 <= i && i < (int)m_shaderConstants.Size())
	{
		return m_shaderConstants.GetElem(i);
	}

	U2ASSERT(false);
	return 0;
}
//----------------------------------------------------------------------------
U2ShaderConstant* U2GpuProgram::GetShaderConstant (
	U2ShaderConstant::ShaderConstantMapping eType)
{
	for (int i = 0; i < (int)m_shaderConstants.Size(); i++)
	{
		if (eType == m_shaderConstants.GetElem(i)->GetSCM())
		{
			return m_shaderConstants.GetElem(i);
		}
	}

	U2ASSERT(false);
	return 0;
}

//-------------------------------------------------------------------------------------------------
uint32 U2GpuProgram::GetNumericalConstantCnt() const
{
	return m_numericalConstants.Size();
}


//----------------------------------------------------------------------------
U2NumericalConstant* U2GpuProgram::GetNumericalConstant (int i)
{
	if (0 <= i && i < (int)m_numericalConstants.Size())
	{
		return m_numericalConstants.GetElem(i);
	}

	U2ASSERT(false);
	return 0;
}

uint32 U2GpuProgram::GetUserConstantCnt() const
{
	return m_userConstants.Size();
}

//----------------------------------------------------------------------------
U2ShaderUserConstant* U2GpuProgram::GetUserConstant (int i)
{
	if (0 <= i && i < (int)m_userConstants.Size())
	{
		return m_userConstants.GetElem(i);
	}

	U2ASSERT(false);
	return 0;
}
//----------------------------------------------------------------------------
U2ShaderUserConstant* U2GpuProgram::GetUserConstant (const U2String& rkName)
{
	for (int i = 0; i < (int)m_userConstants.Size(); i++)
	{
		if (rkName == m_userConstants.GetElem(i)->GetName())
		{
			return m_userConstants.GetElem(i);
		}
	}

	U2ASSERT(false);
	return 0;
}

uint32 U2GpuProgram::GetSamplerInfoCnt() const
{
	return m_samplerInfos.Size();
}

//----------------------------------------------------------------------------
U2ShaderSamplerInfo* U2GpuProgram::GetSamplerInfo (int i)
{
	if (0 <= i && i < (int)m_samplerInfos.Size())
	{
		return m_samplerInfos.GetElem(i);
	}

	U2ASSERT(false);
	return 0;
}
//----------------------------------------------------------------------------
U2ShaderSamplerInfo* U2GpuProgram::GetSamplerInfo (const U2String& rkName)
{
	for (int i = 0; i < (int)m_samplerInfos.Size(); i++)
	{
		if (rkName == m_samplerInfos.GetElem(i)->GetName())
		{
			return m_samplerInfos.GetElem(i);
		}
	}

	U2ASSERT(false);
	return 0;
}



const U2VertexAttributes& U2GpuProgram::GetInputVertAtts() const
{
	return m_inputVertAtts;
}


const U2VertexAttributes& U2GpuProgram::GetOutputVertAtts() const
{
	return m_outputvertAtts;
}






