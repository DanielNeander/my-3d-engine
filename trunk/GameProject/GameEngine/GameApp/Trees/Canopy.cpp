//canopy Class
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

#include "stdafx.h"
#include "canopy.h"

void Canopy::heading(V3 Heading)
{
    AxisHeading = Heading;
}

aabb Canopy::setAABB(){
    m_AABB.xMin = FLT_MAX;
    m_AABB.yMin = FLT_MAX;
    m_AABB.zMin = FLT_MAX;
    m_AABB.xMax = -FLT_MAX;
    m_AABB.yMax = -FLT_MAX;
    m_AABB.zMax = -FLT_MAX;
    m_AABB.max = V3(m_AABB.xMax,
                  m_AABB.yMax,
                  m_AABB.zMax);
    m_AABB.min = V3(m_AABB.xMin,
                  m_AABB.yMin,
                  m_AABB.zMin);
    std::vector<V3*>::iterator it,itend;
    it = m_Canopy.begin();
    itend = m_Canopy.end();
    for(it;it!=itend;it++){
            V3 vertex = *(*it);
            if (m_AABB.xMin > vertex.x) m_AABB.xMin = vertex.x;
            if (m_AABB.xMax < vertex.x) m_AABB.xMax = vertex.x;
            if (m_AABB.yMin > vertex.y) m_AABB.yMin = vertex.y;
            if (m_AABB.yMax < vertex.y) m_AABB.yMax = vertex.y;
            if (m_AABB.zMin > vertex.z) m_AABB.zMin = vertex.z;
            if (m_AABB.zMax < vertex.z) m_AABB.zMax = vertex.z;
            m_AABB.min = V3(m_AABB.xMin,m_AABB.yMin,m_AABB.zMin);
            m_AABB.max = V3(m_AABB.xMax,m_AABB.yMax,m_AABB.zMax);
    }

    return m_AABB;

}

// The patch segment is an array of points that will be used as a canopy patch at the end of a branch or where
// a canopy is defined.  To create the patch we need to know what the drop angle will be. This is calculated based on the established 
// grammar drop angle along with the fulcrum calculation for the tree.  The patch will not be flat but will be perturbed from the plane in
// a random way based on a given weight factor for perturbation.  
void Canopy::growPatchSegment(treeNode *rootOfTree, BranchBase *pCanopyBranch,aabb TreeBoundingBox, LevelDetail *grammar, V3 startHeading)
{
    
    theOverseer = observer::Instance();
	//int cwidth =9;
	//int cheight =9;
	//float m_widthStep =1;
	//float m_heightStep =1.25;
    rootOfTree->tree->m_CanopyCount++;
    V3 root(rootOfTree->pbranch->segments[0].m_tipPointList[0]);
	V3 CanopySegmentRoot(pCanopyBranch->tipPoint);// canopySegmentRoot
	V3 CanopyTop(root);
    CanopyTop.y = root.y + TreeBoundingBox.yMax;
	V3 CanopyFulcrum(CanopyTop);
	CanopyFulcrum.y = CanopyFulcrum.y * 0.57f; // 4/7
    if(pCanopyBranch->tipPoint.y < CanopyFulcrum.y){
        CanopyFulcrum.y = pCanopyBranch->tipPoint.y * 0.714f;// 5/7
    }
	V3 CanopyHeading = CanopySegmentRoot-CanopyFulcrum;
	CanopyHeading.Normalize();
	V3 CanopyArbitrary(CanopySegmentRoot-root);
	CanopyArbitrary.Normalize();
	V3 Left = CrossProduct(CanopyArbitrary,CanopyHeading);
	V3 Right = CrossProduct(CanopyHeading,CanopyArbitrary);
	Left.Normalize();
	Right.Normalize();
	V3 Down = CrossProduct(Left,CanopyHeading);
	V3 Up = CrossProduct(CanopyHeading,Left);
    float perturbFactor = grammar->perturbFactor;
    V3 perturb = CanopyHeading;
	Down.Normalize();
	Up.Normalize();
    Basis.iAxis = Right * (((float)m_width)/2) * m_widthStep;
    Basis.jAxis = CanopyHeading;
    Basis.kAxis = Down * (((float)m_height)/2) * m_heightStep;
    int shift = -((int)floor(m_width/2.0f));
    float nudge =0.0f;
	for(int j=0;j<m_height;j++){
	    for(int i=0;i<m_width;i++){
            nudge = theOverseer->randf(-(perturbFactor),(perturbFactor));
            perturb = perturb * nudge;
            V3 tcb = pCanopyBranch->tipPoint + ((i+shift)*m_widthStep)* Left + (j*m_heightStep)*Down;
            V3 *cb;
            if(!(j==0&&i==(m_width/2))){
                tcb = tcb + perturb;
 			    cb = new V3(tcb);
            }else{
 			    cb = new V3(tcb);
            }
		    m_Canopy.push_back(cb);
            perturb = CanopyHeading;
        }
	}
    theOverseer = observer::Instance();
    m_startIndex = theOverseer->DXRS->CurrentIndex;
    m_startVertex = theOverseer->DXRS->CurrentVIndex;
    m_AABB = setAABB();
    theOverseer->DXRS->CurrentIndex = theOverseer->DXRS->CurrentIndex + ((m_height-1)*(m_width-1)*12); // 6 per quad per side = 12
    theOverseer->DXRS->CurrentVIndex = theOverseer->DXRS->CurrentVIndex + ( m_height*m_width*2); // two sides = 2
    m_burning = false;
   
}
void Canopy::growHexSegment(treeNode *rootOfTree, BranchBase *pCanopyBranch,aabb TreeBoundingBox, LevelDetail *grammar, V3 startHeading)
{
    theOverseer = observer::Instance();
    rootOfTree->tree->m_CanopyCount++;
    V3 root(rootOfTree->pbranch->segments[0].m_tipPointList[0]);
	V3 CanopySegmentRoot(pCanopyBranch->tipPoint);// canopySegmentRoot
	V3 CanopyTop(root);
    CanopyTop.y = root.y + TreeBoundingBox.yMax;
	V3 CanopyFulcrum(CanopyTop);
	CanopyFulcrum.y = CanopyFulcrum.y * 0.57f; // 4/7
    if(pCanopyBranch->tipPoint.y < CanopyFulcrum.y){
        CanopyFulcrum.y = pCanopyBranch->tipPoint.y * 0.714f;// 5/7
    }
	V3 CanopyHeading = CanopySegmentRoot-CanopyFulcrum;
	CanopyHeading.Normalize();
	V3 CanopyArbitrary(CanopySegmentRoot-root);
	CanopyArbitrary.Normalize();
	V3 Left = CrossProduct(CanopyArbitrary,CanopyHeading);
	V3 Right = CrossProduct(CanopyHeading,CanopyArbitrary);
	Left.Normalize();
	Right.Normalize();
	V3 Down = CrossProduct(Left,CanopyHeading);
	V3 Up = CrossProduct(CanopyHeading,Left);
    V3 perturb = CanopyHeading;
	Down.Normalize();
	Up.Normalize();
    int shift = -((int)floor(m_width/2.0f));
    float nudge =0.0f;

    growPatchSegment(rootOfTree, pCanopyBranch,TreeBoundingBox, grammar,startHeading);
}

void Canopy::growCanopySegment(treeNode *rootOfTree, BranchBase *pCanopyBranch,aabb TreeBoundingBox, Grammar *grammar)
{

    bool grammarFound = false;
    int level =0;
    while(!grammarFound){
        if(grammar->m_pLevels[level].cnpyType != canopyType::NULL_CANOPY){
            grammarFound = true;
        }else{
            level++;
        }
    }
    canopyType ct = grammar->m_pLevels[level].cnpyType;
    switch(ct){
        case canopyType::HEX:
            {
                pCanopyBranch->canopies->type = CanopyType::HEX_CANOPY;
                m_nodeLevel = level;
                m_pSpeciesLevelGrammar = &grammar->m_pLevels[m_nodeLevel];
                growHexSegment(rootOfTree, pCanopyBranch,TreeBoundingBox, m_pSpeciesLevelGrammar,pCanopyBranch->heading);           
                break;
            }
        case canopyType::PATCH:
            {
                pCanopyBranch->canopies->type = CanopyType::HEX_CANOPY;
                m_nodeLevel = level;
                m_pSpeciesLevelGrammar = &grammar->m_pLevels[m_nodeLevel];
                growPatchSegment(rootOfTree, pCanopyBranch,TreeBoundingBox, m_pSpeciesLevelGrammar,pCanopyBranch->heading);
                break;
            }
    }
}