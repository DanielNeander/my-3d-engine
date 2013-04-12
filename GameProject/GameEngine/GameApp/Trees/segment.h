//Segment Class
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

#ifndef _Segment
#define _Segment
//#define TipPointCount 11
#include "vector3.h"
enum SegmentType {INITIAL_SEGMENT=0x01, BASIC_SEGMENT=0x10, ONLY_SEGMENT=0x100, TIP_SEGMENT=0x1000};

class Segment {
public: 
    Segment(){};
    ~Segment();
    Segment(float baseWidth, float tipWidth);
	void initialize(int tpc);
    V3 *m_tipPointList;//1 center point c0 and 6 rim points p0-p0''''', tps change
    V3 m_AxisHeading; // unlike the branch heading this is the normal from the plane of the tipPointList
    V3 m_pointOHeading; // the Normal vector between the center c0 and the rim point p0.
    SegmentType m_type; 
    float m_base; // radius: information comes from previous segment
    float m_tip;  // radius
	int m_tipPointCount;
private:
};
#endif
