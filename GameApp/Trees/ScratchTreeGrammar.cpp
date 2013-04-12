//ScratchTreeGrammar Class
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
#include "ScratchTreeGrammar.h"
#include "math.h"
/*
Probability	Split 	Split Type	DropAngle	Axial Bias	Segment Length	Segment Diameter	Wobble	        Taper
-----------------	----------  ---------   ----------  --------------  ----------------    -------------   -------
Level        0								   
5	         3        general	45-80	    30-330	        10	            2	            0-20 	        0.25
5	         2        Square		  			            	                            (bias up)
25	         3        Opposed					            	
0	         1        						
0	         1        						
10	         3        Opposed						
55	         2        					            	
Probability	Split 		        DropAngle	Axial Bias	Segment Length	Segment Diameter	Trajectory	    Taper

*/

ScratchTree::ScratchTree()
{
    m_type = SCRATCH;
    m_pLevels = new LevelDetail[3];
    m_levelCount = 3;
    //level 0
	m_pLevels[0].tipPointCount = 13;
    m_pLevels[0].AxialBias.maxAngle = 330.0f / RadianDegree; //convert to radians
    m_pLevels[0].AxialBias.minAngle = 30.0f / RadianDegree;
    m_pLevels[0].length = 10.0f;
    m_pLevels[0].diameter = 4.0f;//3.0;
    m_pLevels[0].taper = 0.1f;//0.25;
    m_pLevels[0].LevelID = 0;
    m_pLevels[0].dropAngle.minAngle = 0.0f / RadianDegree;
    m_pLevels[0].dropAngle.maxAngle = 5.0f / RadianDegree;
    //levels[0].dropAngle.minAngle = 10.0f / RadianDegree;
    //levels[0].dropAngle.maxAngle = 25.0f / RadianDegree;
    m_pLevels[0].dropAngle.biasAngle = 50.0f / RadianDegree;
    m_pLevels[0].heading.Heading = V3(0.0,1.0,0.0);
    m_pLevels[0].heading.biasRange.minAngle = -5.0f/ RadianDegree;
    m_pLevels[0].heading.biasRange.maxAngle = 5.0f / RadianDegree;
    m_pLevels[0].splitList = new split[2];
    m_pLevels[0].splitList[0].probability = 0.5f;
    m_pLevels[0].splitList[0].splitCount = 1;
    m_pLevels[0].splitList[0].type = REPEAT_OPPOSED;
    m_pLevels[0].splitList[1].probability = 0.5f;
    m_pLevels[0].splitList[1].splitCount = 0;
    m_pLevels[0].splitList[1].type = ORDINARY;
    m_pLevels[0].brnchType = branchType::BRANCH;
    m_pLevels[0].cnpyType = canopyType::NULL_CANOPY;
    m_pLevels[0].brnchType = branchType::TRUNK;
    m_pLevels[0].cnpyType = canopyType::NULL_CANOPY;
//    levels[0].splitList[4].splitCount = ;
    
    //level 1
	m_pLevels[1].tipPointCount = 11;
    m_pLevels[1].AxialBias.maxAngle = 330.0f / RadianDegree; //convert to radians
    m_pLevels[1].AxialBias.minAngle = 30.0f / RadianDegree;
    m_pLevels[1].length = 7.0f;
    m_pLevels[1].diameter = 2.0f;
    m_pLevels[1].taper = 0.20f;
    m_pLevels[1].LevelID = 1;
    m_pLevels[1].dropAngle.minAngle = 35.0f / RadianDegree;
    m_pLevels[1].dropAngle.maxAngle = 50.0f / RadianDegree;
    m_pLevels[1].dropAngle.biasAngle = 40.0f / RadianDegree;
    //levels[1].heading.biasRange.minAngle = -8.0f/ RadianDegree;
    //levels[1].heading.biasRange.maxAngle = 8.0f / RadianDegree;
    m_pLevels[1].heading.biasRange.minAngle = -20.0f/ RadianDegree;
    m_pLevels[1].heading.biasRange.maxAngle = 20.0f / RadianDegree;
    m_pLevels[1].splitList = new split[1];
    m_pLevels[1].splitList[0].probability = 1.0f;
    m_pLevels[1].splitList[0].splitCount = 0;
    m_pLevels[1].splitList[0].type = ORDINARY;
    m_pLevels[1].brnchType = branchType::BRANCH;
    m_pLevels[1].cnpyType = canopyType::NULL_CANOPY;
    
    //level 2 leaf (later implement)
    m_pLevels[2].cnpyType = canopyType::PATCH;
    m_pLevels[2].brnchType = branchType::NULL_BRANCH;
    m_pLevels[2].perturbFactor = 2.0f;
    m_pLevels[2].splitList = new split[1];
    m_pLevels[2].splitList[0].probability = 1.0f;
    m_pLevels[2].splitList[0].splitCount = 0;
    m_pLevels[2].splitList[0].type=REPEAT_ORDINARY;

};         

ScratchTree::~ScratchTree()
{
    for(int i=0;i<m_levelCount;i++)
    {
        delete [] (m_pLevels[i].splitList);
    }
    delete [] m_pLevels;
}