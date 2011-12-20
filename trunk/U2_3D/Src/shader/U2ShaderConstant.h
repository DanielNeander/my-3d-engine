/**************************************************************************************************
module	:	U2ShaderConstant
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_SHADERCONSTANT_H
#define U2_SHADERCONSTANT_H


#include <U2_3D/src/Main/U2Object.h>
#include <U2Lib/src/U2TStringHashTable.h>
#include <U2Lib/src/U2Util.h>


class U2_3D U2ShaderConstant : public U2Object
{
	DECLARE_RTTI;
	DECLARE_INITIALIZE;
	DECLARE_TERMINATE;
public:

	enum ShaderConstantMapping
	{
		SCM_INVALID = 0,
		SCM_WORLD,							// 4x4 model-to-world 
		SCM_VIEW,							// 4x4 world-to-view 
		SCM_PROJ,							// 4x4 world to clip 
		SCM_WORLDVIEW,						// 4x4 model to view 
		SCM_VIEWPORJ,
		SCM_WORLDVIEWPROJ,					// 4x4 model to clip 
		
		SCM_WORLD_TRANSPOSE,				// 4x4 transpose of world
		SCM_VIEW_TRANSPOSE,					// 4x4 transpose world-to-view 
		SCM_PROJ_TRANSPOSE,					// 4x4 transpose view to clip 
		SCM_WORLDVIEW_TRANSPOSE,			// 4x4 transpose of model-to-view 
		SCM_VIEWPROJ_TRANSPOSE,				// 4X4 transpose view-to-clip
		SCM_WORLDVIEWPROJ_TRANSPOSE,		// 4X4,transpose model-to-clip(world x view x proj)
		SCM_INVWORLD,						// 4x4 inverse model-to-world		
		SCM_INVERSE_VIEW,						// 4x4 inverse world-to-view		
		SCM_INVERSE_PROJ,						// 4x4 inverse world-to-clip		
		SCM_INVERSE_WORLDVIEW,					// 4x4 inverse model to view  
		SCM_INVERSE_VIEWPORJ,
		SCM_INVERSE_WORLDVIEWPROJ,				// 4x4 inverse world-to-proj		
		SCM_INVERSE_WORLD_TRANSPOSE,				// 4x4 inverse transpose of world		
		SCM_INVERSE_VIEW_TRANSPOSE,				// 4x4 inverse transpose world-to-view  		
		SCM_INVERSE_PORJ_TRANSPOSE,				// 4x4 inverse transpose view to clip		
		SCM_INVERSE_WORLDVIEW_TRANSPOSE,			// 4x4 inverse transpose of model-to-view
		SCM_INVERSE_VIEWPROJ_TRANSPOSE,			// 4x4 inverse transpose view-to-clip		
		SCM_INVERSE_WORLDVIEWPROJ_TRANSPOSE,		// 4x4 inverse transpose model-to-clip
		SCM_MATRIX_COUNT,

		SCM_MATERIAL_EMISSIVE = SCM_MATRIX_COUNT,			// Material Emissive color
		SCM_MATERIAL_AMBIENT,								// Material Ambient color 
		SCM_MATERIAL_DIFFUSE,					 			// Material Diffuse Color 
		SCM_MATERIAL_SPECULAR,								// Material Specular color
		
		SCM_CAM_MODEL_POS,
		SCM_CAM_MODEL_DIR,
		SCM_CAM_MODEL_UP,
		SCM_CAM_MODEL_RIGHT,

		SCM_CAM_WORLD_POS,					// camera position in world space 
		SCM_CAM_WORLD_DIR,					// camera direction in world space 
		SCM_CAM_WORLD_UP,
		SCM_CAM_WORLD_RIGHT,

		SCM_PROJCAM_MODEL_POS,
		SCM_PROJCAM_MODEL_DIR,
		SCM_PROJCAM_MODEL_UP,
		SCM_PROJCAM_MODEL_RIGHT,

		SCM_PROJCAM_WORLD_POS,
		SCM_PROJCAM_WORLD_DIR,
		SCM_PROJCAM_WORLD_UP,
		SCM_PROJCAM_WORLD_RIGHT,

		SCM_PROJECTOR_MATRIX,		// 4x4 world to clip Matrix
		

		SCM_LIGHT0_MODEL_POS,		         // (x,y,z,1)
		SCM_LIGHT0_MODEL_DIR,			     // (x,y,z,0)
		SCM_LIGHT0_WORLD_POS,	            // (x,y,z,1)
		SCM_LIGHT0_WORLD_DIR,				 // (x,y,z,0)
		SCM_LIGHT0_AMBIENT,                    // (r,g,b,a)
		SCM_LIGHT0_DIFFUSE,                    // (r,g,b,a)
		SCM_LIGHT0_SPECULAR,                   // (r,g,b,a)
		SCM_LIGHT0_SPOTCUTOFF,                 // (angle,cos,sin,exponent)
		SCM_LIGHT0_ATTENUATION,                // (const,lin,quad,intensity)

		SCM_TEXTURE_MATRIX0,		
		SCM_TEXTURE_MATRIX1,		
		SCM_TEXTURE_MATRIX2,		
		SCM_TEXTURE_MATRIX3,		
		

		MAX_SCM_COUNT
	};

	U2ShaderConstant(ShaderConstantMapping  eMapping, int iBaseRegister, uint32 uRegisterCnt);
	~U2ShaderConstant();

	ShaderConstantMapping GetSCM() const;
	int GetBaseRegister() const;
	uint32 GetRegisterCnt() const;
	float* GetData() const;

	static const TCHAR* GetName(ShaderConstantMapping eMapping) ;
	static ShaderConstantMapping GetSCM(const U2DynString& name);
	
	
private:
	ShaderConstantMapping m_eSCM;
	int	m_iBaseRegister;
	uint32 m_uRegisterCnt;		// √÷¥Î 4∞≥
	float m_afData[16];

	static const TCHAR* ms_strSCMs[MAX_SCM_COUNT + 1];


	static U2TStringHashTable< ShaderConstantMapping>* ms_SCMTbl;
};

typedef U2SmartPtr<U2ShaderConstant> U2ShaderConstantPtr;

#include "U2ShaderConstant.inl"

#endif 
