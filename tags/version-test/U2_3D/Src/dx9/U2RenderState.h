/************************************************************************
module	:	U2Dx9AlphaState
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once 
#ifndef U2_DX9_RENDERSTATE_H
#define U2_DX9_RENDERSTATE_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2_3D/Src/Main/U2Object.h>
#include <U2_3D/Src/main/U2TPointerList.h>
#include <U2Lib/Src/U2SmartPtr.h>


U2SmartPointer(U2RenderState);

class U2_3D U2RenderState : public U2Object
{
	DECLARE_RTTI;
	

public:
	virtual ~U2RenderState();

	enum 
	{ 
		ALPHA,
		//DITHER,
		//FOG,
		MATERIAL,
		//REND_SPEC,
		//SHADE,
		//SPECULAR,
		STENCIL,
		TEXTURE,
		//VERTEX_COLOR,
		WIREFRAME,
		ZBUFFER,
		MAX_TYPES
	};  

	virtual int Type() const = 0;
	
	static U2RenderState* ms_aspDefault[MAX_TYPES];
protected:
	U2RenderState();
};


typedef U2TPointerList<U2RenderState*> U2RSList;



#endif
