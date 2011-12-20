/************************************************************************
module	:	U2ContainSphere3D
Author	:	Yun sangyong
Desc	:	GT, Section 7.7: The Method of Separating Axes			
************************************************************************/
#pragma once
#ifndef U2_CONTAINSPHERE3D_H
#define U2_CONTAINSPHERE3D_H

#include <U2_3D/src/U2_3DLibType.h>
#include "U2Sphere.h"

U2_3D bool InU2Sphere(const D3DXVECTOR3& rkPoint, const U2Sphere& rkSphere);

U2_3D U2Sphere MergeU2Spheres(const U2Sphere& sphere0, const U2Sphere& sphere1);

#endif