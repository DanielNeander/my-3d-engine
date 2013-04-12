//Branch Class
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


class Branch;
class BranchBase;

#ifndef _Branch
#define _Branch
#include "SpeciesGrammar.h"
#include "segment.h"
#include "observer.h"
#include "canopy.h"
#include "aabb.h"

struct treeNode;
class Canopy;
class BranchBase 
{
public:
    BranchBase();
    ~BranchBase();
    
    Segment *segments; //create a list of segments for this branch
    Canopy  *canopies;
    bool isCanopy;
    int segmentCount;
	int tipPointCount;
    V3 heading; // This is overall branch direction.  different from initial segement heading.
    V3 position; // This is the root segment center point. This is the Parent branch Tip Point.
    V3 tipPoint; // This is the last segment center point.  This is the child branch position point.
    branchType type; // might be useful for extending the Grammar
    int startVertex;
    int startIndex; 
    int vertexCount;
    int indexCount; 
	int attribute;
    aabb AABB;


    long life;
    bool burning;

};


class Branch {
public:
    Branch(int level, V3 basePosition);
    void growBranch(Branch *pBranch, treeNode *ctreeNode, Grammar *grammar,V3 startHeading);
    virtual Branch *CreateNextBranch(int level, V3 basePosition);
    void calcSegmentDepth(LevelDetail * levelGrammar);
    aabb setAABB();

    V3 m_position; // This is the root segment center point. This is the Parent branch Tip Point.
    Segment *m_pSegments; //create a list of segments for this branch
    int m_segmentCount;
    splitType m_splitType;
    observer *theOverseer;
    int m_splitCount;
    int m_nodeLevel; //level of grammar or present depth of tree growth.
//    branchNode *nodeBranch;
    float m_dropAngle; //angle from previous branch. Could actually be upward so name could be missleading
    LevelDetail *m_pSpeciesLevelGrammar; //just the facts needed for this branch. no sense dragging the whole grammar along
    aabb m_AABB;
protected:
    Branch(){};
};
#endif

    