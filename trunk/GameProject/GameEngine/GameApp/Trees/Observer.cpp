//Observer Class
//Written by Hugh Smith April 2007
// Part of Tree Grammar Structure for creating procedural trees

// Copyright ?2008-2009 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
#include "stdafx.h"
#include "observer.h"
observer* observer::_instance =0;
RenderStructure *observer::DXRS =0; 
observer* observer::Instance() {
    if(_instance == 0) {
        _instance = new observer;
    }
    return _instance;
}
void observer::BoundingBox(V3 vertex){
    if (DXRS->AABB.xMin > vertex.x) DXRS->AABB.xMin = vertex.x;
    if (DXRS->AABB.xMax < vertex.x) DXRS->AABB.xMax = vertex.x;
    if (DXRS->AABB.yMin > vertex.y) DXRS->AABB.yMin = vertex.y;
    if (DXRS->AABB.yMax < vertex.y) DXRS->AABB.yMax = vertex.y;
    if (DXRS->AABB.zMin > vertex.z) DXRS->AABB.zMin = vertex.z;
    if (DXRS->AABB.zMax < vertex.z) DXRS->AABB.zMax = vertex.z;
    DXRS->AABB.min = V3(DXRS->AABB.xMin,DXRS->AABB.yMin,DXRS->AABB.zMin);
    DXRS->AABB.max = V3(DXRS->AABB.xMax,DXRS->AABB.yMax,DXRS->AABB.zMax);
}
int observer::addVertex(VertexType type, V3 vertex, V3 normal = V3(0.0,1.0,0.0), V3 theTexture = V3(0.5, 0.5, 0.0))
{
    BoundingBox(vertex);
    if (type == VertexType::VPNT && DXRS->vpnt) {
        DXRS->vpnt[DXRS->CurrentVIndex++] = VertexPNT(vertex.x, vertex.y, vertex.z,normal.x, normal.y, normal.z, theTexture.x, theTexture.y);
    }
    if (type == VertexType::VP && DXRS->vpos) {
        DXRS->vpos[DXRS->CurrentVIndex++] = VertexPos(vertex.x, vertex.y, vertex.z); 
    }
   return 1;
}
void observer::addIndexes(WORD a, WORD b, WORD c)
{
    if (DXRS->ptrIBData) {
        DXRS->ptrIBData[DXRS->CurrentIndex++]=a;
        DXRS->ptrIBData[DXRS->CurrentIndex++]=b;
        DXRS->ptrIBData[DXRS->CurrentIndex++]=c;
    }
}



float observer::randf(float f = 1.0f){
    return f*(float)rand()/429496796.0f;
}
float observer::randf(float min, float max){
    return min + (max-min)*(float)rand()/(RAND_MAX + 1) ;

}
int observer::randi(unsigned int max=101){
    return rand() % max;
}
int observer::randi(int min, int max){
    return (int) randf((float)min, float(max));
}
void observer::seed(unsigned int theSeed){
    srand(theSeed);   
}
