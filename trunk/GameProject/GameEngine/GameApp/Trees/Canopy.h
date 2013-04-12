//Canopy Class
//Written by Hugh Smith November 2007
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

#ifndef _Canopy
#define _Canopy

#include "vector3.h"
#include <vector>
#include "aabb.h"
#include "Observer.h"
#include "SpeciesGrammar.h"
struct treeNode;
enum CanopyType {HEX_CANOPY=0x01, PATCH_CANOPY=0x10, PROCEDURAL_CANOPY=0x100, TIP_CANOPY=0x1000};

class Canopy {
public: 
    //canopy(){};
    Canopy(int baseWidth = 9,int baseHeight =9,float WidthStep = 1.0f, float HeightStep = 1.25f)
    {
        m_width      = baseWidth ; 
        m_height     = baseHeight; 
        m_widthStep  = WidthStep ; 
        m_heightStep = HeightStep; 
    };
	void heading(V3 Heading);
    V3 AxisHeading; // unlike the branch heading this is the normal from the plane of the tipPointList
    B3 Basis; 
    CanopyType type; 
    std::vector<V3 *> m_Canopy;
	int   m_width     ;
	int   m_height    ;
	float m_widthStep ;
	float m_heightStep;
    int m_startVertex;
    int m_startIndex; 
    int m_vertexCount;
    int m_indexCount; 
	int m_attribute;
    aabb m_AABB;
    bool m_burning;
    observer *theOverseer;
    int m_nodeLevel; //level of grammar or present depth of tree growth.
    LevelDetail *m_pSpeciesLevelGrammar; //just the facts needed for this branch. no sense dragging the whole grammar along
    void growCanopySegment(treeNode *rootOfTree, BranchBase *pCanopyBranch,aabb TreeBoundingBox, Grammar *grammar);
    void growPatchSegment(treeNode *rootOfTree, BranchBase *pCanopyBranch,aabb TreeBoundingBox, LevelDetail *grammar, V3 startHeading);
    void growHexSegment(treeNode *rootOfTree, BranchBase *pCanopyBranch,aabb TreeBoundingBox, LevelDetail *grammar, V3 startHeading);

private:
    aabb setAABB();
};
#endif
