/************************************************************************
module	:	U2ContainBox3D
Author	:	Yun sangyong
Desc	:	GT, Section 7.7: The Method of Separating Axes			
************************************************************************/
#pragma once
#ifndef U2_CONTAINBOX3D_H
#define U2_CONTAINBOX3D_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2_3D/src/Main/U2Box.h>




// Test for containment.  Let X = C + y0*U0 + y1*U1 + y2*U2 where C is the
// box center and U0, U1, U2 are the orthonormal axes of the box.  X is in
// the box if |y_i| <= E_i for all i where E_i are the extents of the box.

U2_3D bool InU2Box (const D3DXVECTOR3& rkPoint, const U2Box& rkBox);

// Construct an oriented box that contains two other oriented boxes.  The
// result is not guaranteed to be the minimum volume box containing the
// input boxes.

U2_3D U2Box MergeU2Boxes(const U2Box& box0, const U2Box& box1);

#endif