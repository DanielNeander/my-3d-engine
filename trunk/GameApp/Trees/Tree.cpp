//Tree Class
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
#include "tree.h"
#include "canopy.h"

bool Tree::counting = false;

Tree::Tree(std::string grammarName, V3 thePosition, long theSeed)
{

    m_grammar = *(createGrammar(grammarName));
    m_position = thePosition;
    m_seed     = theSeed;
    theOverseer = observer::Instance();
    m_boundingBox.xMin = FLT_MAX;
    m_boundingBox.yMin = FLT_MAX;
    m_boundingBox.zMin = FLT_MAX;
    m_boundingBox.xMax = -FLT_MAX;
    m_boundingBox.yMax = -FLT_MAX;
    m_boundingBox.zMax = -FLT_MAX;
    m_boundingBox.max = V3(m_boundingBox.xMax,
                         m_boundingBox.yMax,
                         m_boundingBox.zMax);
    m_boundingBox.min = V3(m_boundingBox.xMin,
                         m_boundingBox.yMin,
                         m_boundingBox.zMin);
	theOverseer->DXRS->AABB = m_boundingBox;
	m_StartBranchIndexBuffer = theOverseer->DXRS->CurrentIndex;
	m_StartBranchVertexBuffer = theOverseer->DXRS->CurrentVIndex;
	m_StartCanopyIndexBuffer = theOverseer->DXRS->CurrentIndex;
	m_StartCanopyVertexBuffer = theOverseer->DXRS->CurrentVIndex;
	m_nodeTree = new treeNode();
	m_nodeTree->pPrevNode = m_nodeTree;
	m_nodeTree->nextNodeSize;
    m_BranchCount=0;
    m_CanopyCount=0;
    m_CanopyCountTest=0;

}
Tree::~Tree(){
	deleteNode (m_nodeTree);
}

Branch* Tree::createTrunk(int level, V3 basePosition){
    return new Branch(level,basePosition);
}

void Tree::deleteNode(treeNode *ctn){
	if(ctn->nextNodeSize == 0){
		delete ctn->pbranch;
	}else{
		for (int i = 0; i<ctn->nextNodeSize;i++){
			deleteNode(&(ctn->pNextNodes[i]));
		}
	}
	delete ctn;
}
//linked list traversal function used to count the number of vertices used
void Tree::countVertices(treeNode *ctn)
{	
	if(!counting){
		counting = true;
		m_FaceCount = 0;
	}
	if(ctn->nextNodeSize == 0){
    // brackets for if statement left in for easier debugging		
	}else{
		for (int i = 0; i<ctn->nextNodeSize;i++){
			countVertices(&(ctn->pNextNodes[i]));
		}
	}
	m_FaceCount += (6* (ctn->pbranch->segmentCount))+ 2;

}
//linked list traversal function
void Tree::countBranches(treeNode *ctn)
{	
	if(!counting){
		counting = true;
		m_BranchCount = 0;
	}
	m_BranchCount ++;
	if(ctn->nextNodeSize == 0){
    // brackets for if statement left in for easier debugging		
	}else{
		for (int i = 0; i<ctn->nextNodeSize;i++){
			countBranches(&(ctn->pNextNodes[i]));
		}
	}
}
//linked list traversal function
void Tree::fillCanopies(treeNode *ctn)
{
    if(ctn->pbranch->isCanopy){
    fillBuffers(ctn->pbranch->canopies);
    }
	if(ctn->nextNodeSize == 0){
    // brackets for if statement left in for easier debugging		
	}else{
		for (int i = 0; i<ctn->nextNodeSize;i++){
            fillCanopies(&(ctn->pNextNodes[i]));
		}
	}

}
//linked list traversal function
void Tree::fillBranches(treeNode *ctn)
{	
    fillBuffers((ctn->pbranch));
	if(ctn->nextNodeSize == 0){
    // brackets for if statement left in for easier debugging			
	}else{
		for (int i = 0; i<ctn->nextNodeSize;i++){
			fillBranches(&(ctn->pNextNodes[i]));
		}
	}
}
//linked list traversal function
void Tree::BranchToList(treeNode *ctn, std::vector<BranchBase*> *treeNodeList)
{
    treeNodeList->push_back(ctn->pbranch);
	if(ctn->nextNodeSize == 0){
    // brackets for if statement left in for easier debugging				
	}else{
		for (int i = 0; i<ctn->nextNodeSize;i++){
            BranchToList(&(ctn->pNextNodes[i]),treeNodeList);
		}
	}
}
//linked list traversal function
void Tree::CanopyToList(treeNode *ctn, std::vector<Canopy*> *treeNodeCanopyList)
{
    if(ctn->pbranch->isCanopy){
        treeNodeCanopyList->push_back(ctn->pbranch->canopies);
    }
	if(ctn->nextNodeSize == 0){
   // brackets for if statement left in for easier debugging			
	}else{
		for (int i = 0; i<ctn->nextNodeSize;i++){
            CanopyToList(&(ctn->pNextNodes[i]),treeNodeCanopyList);
		}
	}
}
//linked list traversal function
// Extends the Bounding box for the tree to include all points
// contained in the canopy segments
void Tree::BoundingBox(treeNode *ctn, bool Canopy)
{	
    bounds((ctn->pbranch));
    if(Canopy && ctn->pbranch->isCanopy)
    {
        bounds((ctn->pbranch->canopies));
    }
	if(ctn->nextNodeSize == 0){
		
	}else{
		for (int i = 0; i<ctn->nextNodeSize;i++){
			BoundingBox(&(ctn->pNextNodes[i]));
		}
	}
}
// Extends the Bounding box for the tree to include all points
// contained in the canopy segments
void Tree::bounds(Canopy *Canopy)
{
    theOverseer = observer::Instance();
    std::vector<V3*>::iterator it,itend;
    it = Canopy->m_Canopy.begin();
    itend = Canopy->m_Canopy.end();
    for(it; it!= itend;it++){
       theOverseer->BoundingBox(*(*it));
       m_boundingBox = theOverseer->DXRS->AABB;
    }
}
void Tree::bounds(BranchBase *branch)
{
    theOverseer = observer::Instance();
	int tps = branch->segments[0].m_tipPointCount;;  
    for(int i = 0;i<branch->segmentCount;i++){
        for(int k=0; k<tps;k++){
           theOverseer->BoundingBox(branch->segments[i].m_tipPointList[k]);
           m_boundingBox = theOverseer->DXRS->AABB;
        }
    }
}

//linked list traversal function
void Tree::getBranch(treeNode *ctn, int branch)
{
	theOverseer = observer::Instance();
	
	//what is my current Branch
	// except for very first branch (trunk) branchcount should be correct coming into function.
	if(!counting){
		counting = true;
		theOverseer->m_branchFound = false;
		m_BranchCount = 0;
		
	}

	if(m_BranchCount == branch){
		theOverseer->m_SoughtAfterBranch = ctn->pbranch;
		theOverseer->m_branchFound = true;
		return ;
	}
	if(ctn->nextNodeSize == 0){
		
	}else{
		for (int i = 0; i<ctn->nextNodeSize;i++){
			m_BranchCount++;
			if(!theOverseer->m_branchFound){
				getBranch(&(ctn->pNextNodes[i]), branch);
			}
		}
	}

}
Grammar * Tree::createGrammar(std::string grammarName)
{
    const char * pstr; 
    std::string defaultFileName("tg1.tdf");
    if ( strcmp( grammarName.c_str(), "" ) == 0 )
    {
        pstr = defaultFileName.c_str();
    }else
    {
        pstr = grammarName.c_str();
    }
    if ( strcmp( pstr, "PlainTree" ) == 0 )
    {
		PlainTree *newGrammar = new PlainTree();
        return reinterpret_cast<Grammar*>(newGrammar);
    }
    else if ( strcmp( pstr, "SpikeTree" ) == 0 )
    {
		SpikeTree *	newGrammar = new SpikeTree();
        return reinterpret_cast<Grammar*>(newGrammar);        
    }
    else if ( strcmp( pstr, "TG1Tree" ) == 0 )
    {
		TG1Tree *newGrammar = new TG1Tree();
		return reinterpret_cast<Grammar*>(newGrammar);
    }
    else 
	{
		Grammar *newGrammar = new Grammar(grammarName);
        return reinterpret_cast<Grammar*>(newGrammar);
    }
    return reinterpret_cast<Grammar*>(0);

}

//linked list traversal function
void Tree::growCanopy(treeNode *ctn)
{
    // check if canopy node ;
    if(ctn->pbranch->isCanopy)
    {
	    int cwidth =9;
	    int cheight =9;
	    float m_widthStep =2.0f;
	    float m_heightStep =2.5f;
        ctn->pbranch->canopies = new Canopy(cwidth,cheight,m_widthStep,m_heightStep);
        
        ctn->pbranch->canopies->growCanopySegment(m_nodeTree, ctn->pbranch, m_boundingBox, &this->m_grammar);
        //growCanopySegment(nodeTree, ctn->pbranch); 
    }
	if(ctn->nextNodeSize == 0){
		
	}else{
		for (int i = 0; i<ctn->nextNodeSize;i++){
			growCanopy(&(ctn->pNextNodes[i]));
		}
	}

}
// creates the Tree nodes linked list of branches and canopies
// you grow both the canopy and branches in the same call to growTree
// you need to traverse the grammar once for the branches and once for the canopies
// therefore you need to reset the observer back to the initialization between the 
// two calls so the observer is set to the right state for the canopy creation.
void Tree::growTree(bool justcount)
{
    sprout();
    Branch *Trunk = createTrunk(0, m_position);
    m_nodeTree = new treeNode();
    m_nodeTree->pPrevNode =0;
    m_nodeTree->tree = this;
 
    V3 startHeading = m_grammar.m_pLevels[0].heading.Heading.Normalize();
    Trunk->growBranch(Trunk, m_nodeTree, &m_grammar, startHeading);
	theOverseer = observer::Instance();
    
	m_EndBranchIndexBuffer = theOverseer->DXRS->CurrentIndex;
	m_EndBranchVertexBuffer = theOverseer->DXRS->CurrentVIndex;
    BoundingBox(m_nodeTree);
    theOverseer->DXRS->CurrentIndex=0;
    theOverseer->DXRS->CurrentVIndex=0;
    
	m_StartCanopyIndexBuffer=0;
	m_EndCanopyIndexBuffer=0;
	m_StartCanopyVertexBuffer=0;
	m_EndCanopyVertexBuffer=0;

    growCanopy(m_nodeTree);
	m_EndCanopyIndexBuffer = theOverseer->DXRS->CurrentIndex;
	m_EndCanopyVertexBuffer = theOverseer->DXRS->CurrentVIndex;
    delete Trunk;
}
void Tree::sprout()
{
    m_levelCount = m_grammar.m_levelCount;
    observer *ob = observer::Instance();
    ob->seed(m_seed);
}
void Tree::pruneTree(V3 *pVertexBuffer)
{
    //stub for future work
}

// iterate through the canopy segments of the tree and 
// fill the index and vertex buffers associated with them.
// 
void Tree::fillBuffers(Canopy *cnpy)
{
    m_CanopyCountTest++;
    theOverseer = observer::Instance();
    WORD idx = cnpy->m_startVertex;
    V3 normal;
    V3 testNormal;
    V3 TexCoord;
    float texUCoord;
    float texVCoord;
    //VertexType Vtype = VertexType::VP;
    VertexType Vtype = VertexType::VPNT;
    V3 *canopyPoints = new V3[cnpy->m_Canopy.size()];
    std::vector<V3 *>::iterator it,itend;
    it = cnpy->m_Canopy.begin();
    itend = cnpy->m_Canopy.end();
    int i = 0;
    for (it;it<itend;it++)
    {
        canopyPoints[i]=*(*it);
        i++;
    }
    // this is a basically a patch of points that is width by height
    // we need to convert it from a set of points to a set of triangles with indexes
    for(int s=0;s<2;s++){
        if(s==0){
	        for(int j=0;j<cnpy->m_height;j++){
	            for(int k=0;k<cnpy->m_width;k++){
                    V3 segment1,segment2;
                    if(k!=cnpy->m_width-1){
                        segment1 = canopyPoints[(j*cnpy->m_width)+k+1]-canopyPoints[(j*cnpy->m_width)+k];
                    }else{
                        segment1 = (canopyPoints[(j*cnpy->m_width)+k-1]-canopyPoints[(j*cnpy->m_width)+k]);
                        //segment1 = V3(0.0f,0.0f,0.0f);V3(0.0f,0.0f,0.0f)-
                    }
                    if(j!=cnpy->m_height-1){
                        segment2 = canopyPoints[((j+1)*cnpy->m_width)+k]-canopyPoints[(j*cnpy->m_width)+k];
                    }else{
                        segment2 = (canopyPoints[(j*cnpy->m_width)+k]-canopyPoints[((j-1)*cnpy->m_width)+k]);
                        //segment2 = V3(0.0f,0.0f,0.0f);V3(0.0f,0.0f,0.0f)-
                    }
                    normal = CrossProduct(segment1,segment2); 
                    normal.Normalize();
                    texUCoord = (1.0f/(cnpy->m_width-1))*k;
                    texVCoord = (1.0f/(cnpy->m_height-1))*j;
                    TexCoord.x = texUCoord;
                    TexCoord.y = texVCoord;
                    theOverseer->addVertex(Vtype, canopyPoints[(j*cnpy->m_width)+k],normal, TexCoord);

               }
            }

        }else{
	        for(int j=0;j<cnpy->m_height;j++){
	            for(int k=0;k<cnpy->m_width;k++){
                    V3 segment1,segment2;
                    if(k!=cnpy->m_width-1){
                        segment1 = canopyPoints[(j*cnpy->m_width)+k+1]-canopyPoints[(j*cnpy->m_width)+k];
                    }else{
                        segment1 = (canopyPoints[(j*cnpy->m_width)+k-1]-canopyPoints[(j*cnpy->m_width)+k]);//V3(0.0f,0.0f,0.0f)-
                    }
                    if(j!=cnpy->m_height-1){
                        segment2 = canopyPoints[((j+1)*cnpy->m_width)+k]-canopyPoints[(j*cnpy->m_width)+k];
                    }else{
                        segment2 = (canopyPoints[(j*cnpy->m_width)+k]-canopyPoints[((j-1)*cnpy->m_width)+k]);//V3(0.0f,0.0f,0.0f)-
                    }
                    normal = CrossProduct(segment2,segment1); 
                    normal.Normalize();
                    texUCoord = (1.0f/(cnpy->m_width-1))*k;
                    texVCoord = (1.0f/(cnpy->m_height-1))*j;
                    TexCoord.x = texUCoord;
                    TexCoord.y = texVCoord;
                    theOverseer->addVertex(Vtype, canopyPoints[(j*cnpy->m_width)+k],normal, TexCoord);

               }
            }
        }
        if(s==0){
	        for(int j=0;j<cnpy->m_height-1;j++){
	            for(int k=0;k<cnpy->m_width-1;k++){
                    theOverseer->addIndexes(idx+1            ,idx  ,idx +cnpy->m_width);
                    theOverseer->addIndexes(idx+1+cnpy->m_width,idx+1,idx+cnpy->m_width );
                    //theOverseer->addIndexes(idx  ,idx+1            ,idx +cnpy->m_width);
                    //theOverseer->addIndexes(idx+1,idx+1+cnpy->m_width,idx+cnpy->m_width );
                    idx++;
                }
            }
        }else{
	        for(int j=0;j<cnpy->m_height-1;j++){
	            for(int k=0;k<cnpy->m_width-1;k++){
                    theOverseer->addIndexes(idx  ,idx+1            ,idx +cnpy->m_width);
                    theOverseer->addIndexes(idx+1,idx+1+cnpy->m_width,idx+cnpy->m_width );
                    //theOverseer->addIndexes(idx+1            ,idx  ,idx +cnpy->m_width);
                    //theOverseer->addIndexes(idx+1+cnpy->m_width,idx+1,idx+cnpy->m_width );
                    idx++;
                }
            }
        }


    }
   
    delete [] canopyPoints;

}
// iterate through the branch segments of the tree and 
// fill the index and vertex buffers associated with them.
// 
void Tree::fillBuffers(BranchBase *branch)
{
    theOverseer = observer::Instance();
    WORD idx = branch->startVertex;
    V3 normal;
    V3 TexCoord;
    float texUCoord;
    float texVCoord;
    //VertexType Vtype = VertexType::VP;
    VertexType Vtype = VertexType::VPNT;
	int tps = branch->segments[0].m_tipPointCount;
    //initial segment cap  7 vertices 
    for(int j=0;j<tps;j++){
       if(theOverseer->DXRS->ReverseWindingOrder){
           normal = CrossProduct((branch->segments[0].m_tipPointList[2]-branch->segments[0].m_tipPointList[0]),(branch->segments[0].m_tipPointList[1]-branch->segments[0].m_tipPointList[0])); 
       }else{
           normal = CrossProduct((branch->segments[0].m_tipPointList[1]-branch->segments[0].m_tipPointList[0]),(branch->segments[0].m_tipPointList[2]-branch->segments[0].m_tipPointList[0])); 
        }
        texUCoord = 0.0f;
        //texVCoord = (float)(j-1)* 1.0f/6.0f;
        if(j==0){
            texVCoord = (float)0.0f;
		}else if(j<(static_cast<float>((tps-1)/2)+1)){
			texVCoord = (float)(j-1) * 1.0f/(float)((tps-1)/2);
		}else{
			texVCoord= 1.0f - ((float)((j-1)%(int)((tps-1)/2)) * 1.0f/(float)((tps-1)/2));
		}
        //}else if(j<4){
        //    texVCoord = (float)(j-1)* 1.0f/3.0f;
        //}else {
        //    texVCoord = 1.0f - (float)((j-1)%3)* 1.0f/3.0f;
        //}
        TexCoord.x = texUCoord;
        TexCoord.y = texVCoord;
        theOverseer->addVertex(Vtype, branch->segments[0].m_tipPointList[j],normal, TexCoord);
    }
    //initial segment cap 18 indices
    if(theOverseer->DXRS->ReverseWindingOrder){
        //fill base cone; 0-6,  021,032,043,054,065,016;
		for(int h=0; h<tps-1; h++){
			theOverseer->addIndexes(idx,idx+1 +(h+1)%(tps-1),idx+h+1);
		}
        //theOverseer->addIndexes(idx,idx+2,idx+1);
        //theOverseer->addIndexes(idx,idx+3,idx+2);
        //theOverseer->addIndexes(idx,idx+4,idx+3);
        //theOverseer->addIndexes(idx,idx+5,idx+4);
        //theOverseer->addIndexes(idx,idx+6,idx+5);
        //theOverseer->addIndexes(idx,idx+1,idx+6);
    }else{

		for(int h=0; h<tps-1; h++){
			theOverseer->addIndexes(idx,idx+h+1,idx+1 +(h+1)%(tps-1));
		}
        //fill base cone; 0-6,  012,023,034,045,056,061;
        theOverseer->addIndexes(idx,idx+1,idx+2);
        theOverseer->addIndexes(idx,idx+2,idx+3);
        theOverseer->addIndexes(idx,idx+3,idx+4);
        theOverseer->addIndexes(idx,idx+4,idx+5);
        theOverseer->addIndexes(idx,idx+5,idx+6);
        theOverseer->addIndexes(idx,idx+6,idx+1);
    }
    WORD p = idx+1;
    idx = idx+tps;
    //for each additional segment 
    //add 6 vertexes except for last segment which has 7

    for(int i=1;i<branch->segmentCount;i++){
        for(int k=0;k<tps;k++){
            if(k==0){
                if(i == branch->segmentCount-1){
                   if(theOverseer->DXRS->ReverseWindingOrder){
                       normal = CrossProduct((branch->segments[i].m_tipPointList[2]-branch->segments[i].m_tipPointList[0]),(branch->segments[i].m_tipPointList[1]-branch->segments[i].m_tipPointList[0])); 
                   }else{
                       normal = CrossProduct((branch->segments[i].m_tipPointList[1]-branch->segments[i].m_tipPointList[0]),(branch->segments[i].m_tipPointList[2]-branch->segments[i].m_tipPointList[0])); 
                   }
                    texUCoord = (float)0.0f;
                    texVCoord = (float)0.0f;
					TexCoord.x = texUCoord;
					TexCoord.y = texVCoord;
                    theOverseer->addVertex(Vtype, branch->segments[i].m_tipPointList[k], normal.Normalize(), TexCoord);
                    idx++;
                }
            }else{
                normal = branch->segments[i].m_tipPointList[k]-branch->segments[i].m_tipPointList[0];
                texUCoord = (float)i * 1.0f/(float)branch->segmentCount;
                //texVCoord = (float)(k-1)* 1.0f/6.0f;
				if(k<(static_cast<float>((tps-1)/2)+1)){
					texVCoord = (float)(k-1) * 1.0f/(float)((tps-1)/2);
				}else{
					texVCoord= 1.0f - ((float)((k-1)%(int)((tps-1)/2)) * 1.0f/(float)((tps-1)/2));
				}                
				//if(k<4){
    //                texVCoord = (float)(k-1)* 1.0f/3.0f;
    //            }else {
    //                texVCoord = 1.0f - (float)((k-1)%3)* 1.0f/3.0f;
    //            }
                
                TexCoord.x = texUCoord;
                TexCoord.y = texVCoord;
                
                theOverseer->addVertex(Vtype, branch->segments[i].m_tipPointList[k], normal.Normalize(), TexCoord);
            }
        }
        //Add 36 indices for each segment pair.
        if(theOverseer->DXRS->ReverseWindingOrder){
       //fill segment[i] & [i-1]== [p]     p0.i1.i0, p0.p1.i1, p1.i2.i1, p1.p2.i2, p2.i3.i2, p2.p3.i3,
        //                                 p3.i4.i3, p3.p4.i4, p4.i5.i4, p4.p5.i5,
        //                                 p5.i0.i5, p5.p0.i0, p6.i0.i6, p6.p0.i0
		for(int h=0; h<tps-1; h++){
			theOverseer->addIndexes(p+h, idx+(h+1)%(tps-1), idx +  h   %(tps-1)    );//(p  ,idx+1,idx   );
			theOverseer->addIndexes(p+h, p  +(h+1)%(tps-1), idx + (h+1)%(tps-1));//(p  ,p+1  ,idx+1 ); 
		}
        //theOverseer->addIndexes(p  ,idx+1,idx   );
        //theOverseer->addIndexes(p  ,p+1  ,idx+1 );
        //theOverseer->addIndexes(p+1,idx+2,idx+1 );
        //theOverseer->addIndexes(p+1,p+2  ,idx+2 );
        //theOverseer->addIndexes(p+2,idx+3,idx+2 );
        //theOverseer->addIndexes(p+2,p+3  ,idx+3 );
        //theOverseer->addIndexes(p+3,idx+4,idx+3 );
        //theOverseer->addIndexes(p+3,p+4  ,idx+4 );
        //theOverseer->addIndexes(p+4,idx+5,idx+4 );
        //theOverseer->addIndexes(p+4,p+5  ,idx+5 );
        //theOverseer->addIndexes(p+5,idx  ,idx+5 );
        //theOverseer->addIndexes(p+5,p    ,idx   ); 
        }else{
       //fill segment[i] & [i-1]== [p]     p0.i0.i1,   p0.i1.p1,  p1.i1.i2, p1.i2.p2, p2.i2.i3, p2.i3.p3,
        //                                  p3.i3.i4,  p3.i4.p4, p4.i4.i5, p4.i5.p5,
        //                                  p5.i5.i0,  p5.i0.p0, p6.i6.i0, p6.i0.p0
		for(int h=0; h<tps-1; h++){
			theOverseer->addIndexes(p+h, idx +  h   %(tps-1), idx+(h+1)%(tps-1) );//(p  ,idx  ,idx+1);
			theOverseer->addIndexes(p+h, idx + (h+1)%(tps-1), p  +(h+1)%(tps-1) );    //(p  ,idx+1,p+1  );
		}
        //theOverseer->addIndexes(p  ,idx  ,idx+1);
        //theOverseer->addIndexes(p  ,idx+1,p+1  );
        //theOverseer->addIndexes(p+1,idx+1,idx+2);
        //theOverseer->addIndexes(p+1,idx+2,p+2  );
        //theOverseer->addIndexes(p+2,idx+2,idx+3);
        //theOverseer->addIndexes(p+2,idx+3,p+3  );
        //theOverseer->addIndexes(p+3,idx+3,idx+4);
        //theOverseer->addIndexes(p+3,idx+4,p+4  );
        //theOverseer->addIndexes(p+4,idx+4,idx+5);
        //theOverseer->addIndexes(p+4,idx+5,p+5  );
        //theOverseer->addIndexes(p+5,idx+5,idx  );
        //theOverseer->addIndexes(p+5,idx  ,p    ); 
        }
        p=p+tps-1;

        if(i == branch->segmentCount-1){
            idx--;
            if(theOverseer->DXRS->ReverseWindingOrder){
            //fill tip cone; 0-6,  021,032,043,054,065,016;
			for(int h=0; h<tps-1; h++){
				theOverseer->addIndexes(idx,idx+h+1,idx+1 +(h+1)%(tps-1));
			}
            //theOverseer->addIndexes(idx,idx+1,idx+2);
            //theOverseer->addIndexes(idx,idx+2,idx+3);
            //theOverseer->addIndexes(idx,idx+3,idx+4);
            //theOverseer->addIndexes(idx,idx+4,idx+5);
            //theOverseer->addIndexes(idx,idx+5,idx+6);
            //theOverseer->addIndexes(idx,idx+6,idx+1);
            }else{
            //fill tip cone; 0-6,  012,023,034,045,056,061;
			for(int h=0; h<tps-1; h++){
				theOverseer->addIndexes(idx,idx+1 +(h+1)%(tps-1),idx+h+1);
			}
            //theOverseer->addIndexes(idx,idx+2,idx+1);
            //theOverseer->addIndexes(idx,idx+3,idx+2);
            //theOverseer->addIndexes(idx,idx+4,idx+3);
            //theOverseer->addIndexes(idx,idx+5,idx+4);
            //theOverseer->addIndexes(idx,idx+6,idx+5);
            //theOverseer->addIndexes(idx,idx+1,idx+6);
            }
            idx=idx+tps-1;

        }
        idx=idx+tps-1;

    }
    
}
void Tree::KeepVariableAlive(void * a)
{
    m_KAV = a;
}