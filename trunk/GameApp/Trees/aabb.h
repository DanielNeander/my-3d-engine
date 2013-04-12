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



#ifndef AABB_H
#define AABB_H

#include "vertex.h"

class aabb {
public:
    aabb(){};
    ~aabb(){};

    float   xMin;
    float   yMin;
    float   zMin;
    float   xMax;
    float   yMax;
    float   zMax;
    V3      max;
    V3      min;
    //aabb& operator = (const aabb& bb) {
    //    
    //    xMin = bb.xMin;
    //    yMin = bb.yMin;
    //    zMin = bb.zMin;
    //    xMax = bb.xMax;
    //    yMax = bb.yMax;
    //    zMax = bb.zMax;
    //    max  = bb.max ;
    //    min  = bb.min ;

    //    return *this;
    //};
    
};


#endif