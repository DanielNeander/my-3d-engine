//SpeciesGrammar Class
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

#ifndef _SpeciesGrammar
#define _SpeciesGrammar
#include "Vector3.h"
#include <string>

#pragma warning( push )
#pragma warning( disable : 4635 6011 )
#include "TinyXML/TinyXml.h"
#pragma warning( pop )

#define RadianDegree (float)57.29578

enum treeType {PLAIN, PINE, MAPLE, SPIKE, FAT, SCRATCH};
enum branchType {TRUNK, BRANCH, STEM, LEAF, FLOWER, FRUIT, BUD, NULL_BRANCH};
enum canopyType {PATCH, HEX, PROCEDURAL, NULL_CANOPY};
//Ordinary:  no specified split; random in twist and follows normal drop angle pattern
//Opposed: First split continues on in the heading of the last segment of the previous branch
//basically continues in the general direction(straight) but it progresses to the next grammar level
//Repeat_Opposed:  Same as opposed except it repeats the grammar level
//Spear: no use at this point
//Square: no use at this point
//Repeat_Square: no use at this point
//Uniform: no use at this point
enum splitType {ORDINARY, REPEAT_ORDINARY, SQUARE, OPPOSED, UNIFORM, SPEAR, 
                REPEAT_SQUARE, REPEAT_OPPOSED, CANOPY, ORDINARY_CANOPY, REPEAT_CANOPY};

struct split{
    int splitCount;
    float probability;
    splitType type;
};

struct range{
    float minAngle;
    float maxAngle;
    float biasAngle;
};

struct plumbBias{
    V3 biasDirection;
    range biasRange;
};

struct HeadingBias{
    V3 Heading;
    range biasRange;
};

struct CanopyDefinition{
    canopyType cnpyType;
    int patchWidth;
    int patchHeight;
    float hexRadius;
};

struct LevelDetail {

    int LevelID;
    split *splitList;
    HeadingBias heading;
    range dropAngle;
    range AxialBias;
    float diameter; //radius
    float length; //radius
    int maxSegmentCount;
    float taper; 
    float perturbFactor;
	int tipPointCount;
    branchType brnchType;
    canopyType cnpyType;
};


class Grammar {
public:
    LevelDetail *m_pLevels;
    int m_levelCount;
    std::string m_species;
	std::string m_filename;
    treeType m_type;
    Grammar(){ 
    };
	Grammar(std::string fn){ 
		m_filename = fn;
		loadLevel(m_filename);
	};
	void loadLevel(std::string filename);
    LevelDetail *m_pCurrentLevel;
};
#endif
