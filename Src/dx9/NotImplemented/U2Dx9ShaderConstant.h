/************************************************************************
module	:	U2Dx9ShaderConstant
Author	:	Yun	sangyong
Desc	:
************************************************************************/
#pragma once 
#ifndef U2_DX9_SHADERCONSTANT_H
#define U2_DX9_SHADERCONSTANT_H

#include <U2Lib/Src/U2RefObject.h>

class U2_3D U2Dx9ShaderConstant : public U2RefObject
{

public:
	U2Dx9ShaderConstant();
	virtual ~U2Dx9ShaderConstant();

	// 장치의 레지스터에 매핑할 변수의 타입목록
	enum MappingType
	{
		
	};

};




#endif