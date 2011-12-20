/**************************************************************************************************
module	:	U2Shader
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_SHADER_H
#define U2_SHADER_H

#include <U2Lib/Src/U2TVec.h>
#include <U2Lib/Src/U2SmartPtr.h>
#include "U2GpuProgram.h"

class U2_3D U2Shader : public U2Object
{

public:
	virtual ~U2Shader();

	
	const U2DynString& GetShaderName() const;

	
	uint32 GetTextureCnt() const;
	bool SetTexture(int idx, U2Dx9BaseTexturePtr pTex);
	bool SetTexture(int, const U2DynString& name);

	U2Dx9BaseTexturePtr GetTexture(uint32 i);
	const U2Dx9BaseTexturePtr GetTexture(uint32 i) const;

	U2Dx9BaseTexturePtr GetTexture(const U2DynString& name);
	const U2Dx9BaseTexturePtr GetTexture(const U2DynString& name) const;
	
	virtual void OnLostDevice() {}
	virtual void OnResetDevice() {}
	
protected:
	U2Shader();

	U2Shader(const U2DynString& szName);

	void OnLoadProgram();

	U2DynString m_szShaderName;

	U2GpuProgramPtr m_spProgram;

	U2PrimitiveVec<float> m_userDataArray;

	U2ObjVec<U2Dx9BaseTexturePtr> m_textureArray;


};

typedef U2SmartPtr<U2Shader> U2ShaderPtr;



#endif