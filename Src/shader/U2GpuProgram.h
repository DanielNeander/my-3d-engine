/**************************************************************************************************
module	:	U2GpuProgram
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_GPUPROGRAM_H
#define U2_GPUPROGRAM_H

#include <U2_3D/Src/U2_3DLibType.h>
#include <U2_3D/Src/Main/U2Object.h>
#include <U2Lib/Src/U2TVec.h>
#include "U2VertexAttributes.h"
#include "U2ShaderConstant.h"
#include "U2ShaderUserConstant.h"
#include "U2NumerIcalConstant.h"
#include "U2ShaderSamplerInfo.h"
#include "U2Shader.h"

class U2_3D U2GpuProgram : public U2Object
{
	DECLARE_RTTI;
public:
	virtual ~U2GpuProgram();

	const U2String& GetProgramText() const;

	const U2VertexAttributes& GetInputVertAtts() const;
	const U2VertexAttributes& GetOutputVertAtts() const;

	uint32 GetShaderConstantCnt() const;
	U2ShaderConstant* GetShaderConstant(int i);
	U2ShaderConstant* GetShaderConstant(
		U2ShaderConstant::ShaderConstantMapping eMapping);

	uint32 GetNumericalConstantCnt() const;
	U2NumericalConstant* GetNumericalConstant(int i);

	uint32 GetUserConstantCnt() const;
	U2ShaderUserConstant* GetUserConstant(int i);
	U2ShaderUserConstant* GetUserConstant(const U2String& name);

	uint32 GetSamplerInfoCnt() const;
	U2ShaderSamplerInfo* GetSamplerInfo(int i);
	U2ShaderSamplerInfo* GetSamplerInfo(const U2String& name);

	static bool LoadAndParse(const U2String& filename, U2GpuProgram* pProgram);

	static U2String ms_strRenderType;
	static TCHAR ms_cCommentChar;

	U2GpuProgram* GetNext();
	U2GpuProgram* GetPrev();

	static U2GpuProgram* GetHead();
	static U2GpuProgram* GetTail();
	

	uint32 m_uiRegister;

protected:
	U2GpuProgram();

	void AddProgramToList();
	void RemoveProgramFromList();

	U2GpuProgram* m_pNext;
	U2GpuProgram* m_pPrev;

	static U2GpuProgram* ms_pHead;
	static U2GpuProgram* ms_pTail; 


	U2String m_strProgramText;

	U2VertexAttributes m_inputVertAtts;
	U2VertexAttributes m_outputvertAtts;

	U2ObjVec<U2ShaderConstantPtr> m_shaderConstants;
	U2ObjVec<U2NumericalConstantPtr> m_numericalConstants;
	U2ObjVec<U2ShaderUserConstantPtr> m_userConstants;
	U2ObjVec<U2ShaderSamplerInfoPtr> m_samplerInfos;

	static const U2String ms_strFloat;
	static const U2String ms_strFloat1;
	static const U2String ms_strFloat1x1;
	static const U2String ms_strFloat1x2;
	static const U2String ms_strFloat1x3;
	static const U2String ms_strFloat1x4;	
	static const U2String ms_strFloat2;
	static const U2String ms_strFloat2x1;
	static const U2String ms_strFloat2x2;
	static const U2String ms_strFloat2x3;
	static const U2String ms_strFloat2x4;
	static const U2String ms_strFloat3;
	static const U2String ms_strFloat3x1;
	static const U2String ms_strFloat3x2;
	static const U2String ms_strFloat3x3;
	static const U2String ms_strFloat3x4;
	static const U2String ms_strFloat4;
	static const U2String ms_strFloat4x1;
	static const U2String ms_strFloat4x2;
	static const U2String ms_strFloat4x3;
	static const U2String ms_strFloat4x4;
	static const U2String ms_strSampler1D;
	static const U2String ms_strSampler2D;
	static const U2String ms_strSampler3D;
	static const U2String ms_strSamplerCube;
	static const U2String ms_strSamplerProj;
	static const U2String ms_strPosition;
	static const U2String ms_strBlendWeight;
	static const U2String ms_strNormal;
	static const U2String ms_strColor;
	static const U2String ms_strColor0;
	static const U2String ms_strColor1;
	static const U2String ms_strFogCoord;
	static const U2String ms_strFogStr;
	static const U2String ms_strPSize;
	static const U2String ms_strBlendindices;
	static const U2String ms_strTexcoord;
	static const U2String ms_strTangent;
	static const U2String ms_strBinormal;
	static const U2String ms_strIn;
	static const U2String ms_strEOL;

};

typedef U2SmartPtr<U2GpuProgram> U2GpuProgramPtr;


#endif