#include "stdafx.h"
#include "OgreTree.h"
#include "OgreBranch.h"


OgreTree::OgreTree(Grammar *theGrammar, V3 thePosition, long theSeed):Tree(theGrammar, thePosition, theSeed){

 
}

OgreTree::~OgreTree(){
}

branch* OgreTree::createTrunk(int level, V3 basePosition){
    return new OgreBranch(level,basePosition);
}


