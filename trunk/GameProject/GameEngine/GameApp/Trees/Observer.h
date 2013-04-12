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


// When dealing with linked lists or tree structures it is often cumbersome to deal
// with the data used in the recursive functions.  trying to keep track of where you
// are can get confusing and difficult.  The choices for the programmar are to either pass a lot 
// of data in the function creating a lot of stack bloat or create a lot of global variables.
// I'm not personally fond of either solution so the observer class was created as a singleton
// container that will basically manage those variables for you.  A managed global container
// (not to be confused with the Observer Pattern).  It oversees the variables needed and provides
// a way for the application using the library to pass in the data without having extern in or out a bunch
// of variables.  You can call the instance at any point and know you have the variables in there present state.
//
class observer;

#ifndef OBSERVER_H
#define OBSERVER_H

//#include <d3d9.h>
//#include <d3dx9.h>
#include <dxerr.h>
#include "Vertex.h"
#include "Vector3.h"
#include "branch.h"
#include "tree.h"
#include "aabb.h"
#include "stdlib.h"
#include <vector>


struct RenderStructure {
    VertexType type;    
    VertexPos *vpos;  //vertex buffer pointer
    VertexPNT *vpnt;  //vertex buffer pointer
    WORD *ptrIBData;  //index buffer pointer
    DWORD CurrentIndex;
    DWORD CurrentVIndex; 
	WORD BranchCount;
    bool ReverseWindingOrder;
    aabb AABB;
};


class observer {
public:
    static observer * Instance();
    static RenderStructure *DXRS;
    void BoundingBox(V3 vertex);

    int addVertex(VertexType type, V3 vertex, V3 normal, V3 theTexture);
    void addIndexes(WORD a, WORD b, WORD c);
    float randf(float f);//defaults to number between 0(inclusive)-1(exclusive) 
    float randf(float min, float max);
    int randi(unsigned int i);
    int randi(int min, int max);
    void seed(unsigned int theSeed);
    Tree *m_tree;
	BranchBase *m_SoughtAfterBranch;
	bool m_branchFound;
    
protected:
    observer(){};
private:
    static observer* _instance;
    
};
#endif //OBSERVER_H