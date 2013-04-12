//OgreTree Class
//Writen by Hugh Smith April 2007
// Part of Tree Grammar Structure for creating procedural trees 
// Created for Nehalem Demo to be compatable with OGRE Rendering methodologies


#include "Tree.h"

class OgreTree : public Tree {
public: 
    OgreTree(Grammar *theGrammar, V3 position, long seed);
    ~OgreTree();
    virtual branch* createTrunk(int level, V3 basePosition);
private:
    OgreTree(){};
};