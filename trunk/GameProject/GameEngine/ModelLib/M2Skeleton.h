#pragma once
#ifndef __M2SKELETON_H__
#define __M2SKELETON_H__

#include "Model_M2Format.h"
#include "Model_M2Interpolator.h"

class M2Loader;

class Bone {
public:
	/*Animated<noVec3> trans;	
	Animated<noQuat, PACK_QUATERNION, Quat16ToQuat32> rot;
	Animated<noVec3> scale;*/
	Animated<noVec3> trans;	
	Animated<noQuat, PACK_QUATERNION, Quat16ToQuat32> rot;
	Animated<noVec3> scale;

	noVec3 pivot, transPivot;
	int16 parent;

	bool billboard;
	noMat4 mat;
	noMat4 mrot;

	mat4 rdmat;
	
	ModelBoneDef boneDef;

	bool calc;
	M2Loader *model;
	void calcMatrix(Bone* allbones, int anim, int time, bool rotate=true);
	void initV3(MPQFile &f, ModelBoneDef &b, uint32 *global, MPQFile *animfiles);

	void initV2(MPQFile &f, ModelBoneDef &b, uint32 *global);

};

#endif