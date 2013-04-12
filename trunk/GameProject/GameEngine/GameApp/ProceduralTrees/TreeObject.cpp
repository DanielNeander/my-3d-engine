//Writen by Hugh Smith summer and fall 2007
// Part of Smoke Framework implementation for procedural Tree system.

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

//
// core includes
//
#include "stdafx.h"
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"
#include "..\Trees\Vertex.h"
#include "..\Trees\Vector3.h"
#include "..\Trees\Tree.h"

#include "..\Trees\Observer.h"
#include  "..\ProceduralFire\FireObject.h"

//
// ODE system includes
//
#include "TreeObject.h"
std::vector<D3DXATTRIBUTERANGE> mAttribBuffer;
std::vector<D3DXATTRIBUTERANGE> *ptrAttrBuff;

#pragma warning( push )
#pragma warning( disable : 6386 )


#define NUM_VERTEXDECL_ELEMENTS         3




pcstr TreeObject::sm_kapszPropertyNames[] =
{
    "Position", "Seed", "LevelCount","PrimitiveType" 
};

const Properties::Property TreeObject::sm_kaDefaultProperties[] =
{
    Properties::Property( sm_kapszPropertyNames[ Property_Position ],
                          Properties::Values::Vector3,
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          Math::Vector3::Zero ),
    Properties::Property( sm_kapszPropertyNames[ Property_Seed ],
                          VALUE1( Properties::Values::Int32 ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          0 ),
    Properties::Property( sm_kapszPropertyNames[ Property_LevelCount ],
                          VALUE1( Properties::Values::Int32 ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          0 ),
    Properties::Property( sm_kapszPropertyNames[ Property_PrimitiveType ],
                          VALUE1( Properties::Values::String ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          "" ),
};



TreeObject::TreeObject(
    ISystemScene* pSystemScene,
    pcstr pszType
    )
    : ISystemObject( pSystemScene, NULL )
    ,m_Position(Math::Vector3::Zero)
    ,m_Seed(0) 
    ,m_LevelCount(0)
    ,m_Initialized(False)
    ,m_pPostedData(0)
{    
    ASSERT( Property_Count == sizeof sm_kapszPropertyNames /
                                sizeof sm_kapszPropertyNames[ 0 ] );
    ASSERT( Property_Count == sizeof sm_kapszPropertyNames /
                                sizeof sm_kapszPropertyNames[ 0 ] );

    m_GrammarType = pszType;

}


TreeObject::~TreeObject(
    void
    )
{
	// Clean up static elements in Overseer class, and the object itself.
	observer* Overseer = observer::Instance();
	//SAFE_DELETE( Overseer->m_tree );
	SAFE_DELETE( Overseer->DXRS );
	SAFE_DELETE( Overseer );

	SAFE_DELETE( m_pPostedData );

    reinterpret_cast<TreeScene*>(GetSystemScene())->GetForest().remove(this);

    std::vector<BranchBase *>::iterator i, iend;
    iend = m_TreeNodeList.end();
    for (i = m_TreeNodeList.begin(); i != iend; ++i)
    {
        delete *i;
    }
    m_TreeNodeList.empty();
 }


System::Type
TreeObject::GetSystemType(
    void
    )
{
    return System::Types::MakeCustom( 0 );
}


Error 
TreeObject::Initialize( std::vector<Properties::Property> Properties )
{    
    for ( Properties::Iterator it=Properties.begin(); it != Properties.end(); it++ )
    {
            std::string sName = it->GetName();

            if ( sName == sm_kapszPropertyNames[ Property_LevelCount ] )
            {
                    m_LevelCount = it->GetInt32( 0 );
            } else if (sName == sm_kapszPropertyNames[ Property_Position ] )
            {
                m_Position.x = it->GetFloat32( 0 );
                m_Position.y = it->GetFloat32( 1 );
                m_Position.z = it->GetFloat32( 2 );
            } else if (sName == sm_kapszPropertyNames[ Property_Seed ] )
            {
                m_Seed = it->GetInt32( 0 );
            } else if (sName == sm_kapszPropertyNames[ Property_PrimitiveType ] )
            {
                pcstr pt = it->GetStringPtr(0);
                if(!strcmp(pt,"Branches"))
                {
                    m_PrimitiveType = Primitive_Branches;
                }else if(!strcmp(pt,"Canopy")){
                    m_PrimitiveType = Primitive_Canopy;
                }                
            }
    }

    ptrAttrBuff = &mAttribBuffer;
    observer *Overseer = observer::Instance();
    ASSERT( Overseer != NULL );
	DBG_UNREFERENCED_LOCAL_VAR( Overseer );
    if (Overseer->DXRS) {delete Overseer->DXRS;}
    Overseer->DXRS = new RenderStructure();
    Overseer->DXRS->CurrentIndex	= 0;
    Overseer->DXRS->CurrentVIndex	= 0;
    Overseer->DXRS->ptrIBData;
    Overseer->DXRS->BranchCount	= 0;
    Overseer->DXRS->ReverseWindingOrder = true;



    m_Tree = new Tree(m_GrammarType.c_str(),V3(m_Position.x,m_Position.y,m_Position.z),m_Seed);

    m_Tree->growTree();
    m_Tree->BoundingBox(m_Tree->m_nodeTree);
    m_ObjectBoundingBox = Overseer->DXRS->AABB;
    if(m_PrimitiveType == Primitive_Branches)
    {
        m_Tree->BranchToList(m_Tree->m_nodeTree, &m_TreeNodeList);
    }else if(m_PrimitiveType == Primitive_Canopy)
    {
        m_Tree->CanopyToList(m_Tree->m_nodeTree, &m_TreeNodeCanopyList);
    }
    FillPostedData();
    
    reinterpret_cast<TreeScene*>(GetSystemScene())->GetForest().push_back(this);
    Overseer->DXRS->CurrentIndex	= 0;
    Overseer->DXRS->CurrentVIndex	= 0;
    Overseer->DXRS->ptrIBData;
    Overseer->DXRS->BranchCount	= 0;
    Overseer->DXRS->ReverseWindingOrder = true;

    PostChanges( System::Changes::Graphics::AllMesh | System::Changes::Custom );

    return Errors::Success;
}
void TreeObject::FillPostedData(){
    if(m_PrimitiveType == Primitive_Branches)
    {
        std::vector<BranchBase *>::iterator i, iend;
        iend = m_TreeNodeList.end();
        if(m_pPostedData){
        }else{
            m_pPostedData = new PostedData();
        }
        for (i = m_TreeNodeList.begin(); i != iend; ++i)
        {
           //branchBase *pbb = *i;
           PointPair * pp = new PointPair();
           pp->basePoint = (*i)->position;
           pp->extendPoint = (*i)->segments[(*i)->segmentCount-1].m_tipPointList[0];
           pp->AABB = (*i)->AABB;
           pp->burning = (*i)->burning;
           m_pPostedData->pointPairs.push_back(pp);           
        }
    }else if(m_PrimitiveType == Primitive_Canopy)
    {
        std::vector<Canopy *>::iterator i, iend;
        iend = m_TreeNodeCanopyList.end();
        if(m_pPostedData){
        }else{
            m_pPostedData = new PostedData();
        }
        for (i = m_TreeNodeCanopyList.begin(); i != iend; ++i)
        {
           Canopy *cnpy = *i;
           PointPair * pp = new PointPair();
           pp->basePoint = *(cnpy->m_Canopy[((cnpy->m_width)/2) + ((cnpy->m_height)/2)*cnpy->m_width ]);
           pp->basis = cnpy->Basis;
           
           pp->AABB = cnpy->m_AABB;
           pp->burning = cnpy->m_burning;
           m_pPostedData->pointPairs.push_back(pp);           

        }
    }


}
void TreeObject::update(f32 DeltaTime)
{
	UNREFERENCED_PARAM( DeltaTime );
    //PostChanges( System::Changes::Custom);

}


void* TreeObject::GetObjectPostedVariables(void *Params)
{
	UNREFERENCED_PARAM( Params );
    //PostChanges( System::Changes::Custom |
    //    System::Changes::Graphics::AllPointList);

    return m_pPostedData;

    
}

void
TreeObject::GetProperties(
    Properties::Array& Properties
    )
{
    //
    // Get the index of our first item.
    //
    i32 iProperty = static_cast<i32>(Properties.size());

    //
    // Add the common properties.
    //
    Properties.reserve( Properties.size() + Property_Count );

    for ( i32 i=0; i < Property_Count; i++ )
    {
        Properties.push_back( sm_kaDefaultProperties[ i ] );
    }

    //
    // Modify the default values.
    //
    Properties[ iProperty+Property_Position ].SetValue( m_Position );
    Properties[ iProperty+Property_Seed ].SetValue( 0, m_Seed );
    Properties[ iProperty+Property_LevelCount ].SetValue( 0, m_LevelCount );

}


void
TreeObject::SetProperties(
    Properties::Array Properties
    )
{
    //
    // Read in the properties.
    //
    UNREFERENCED_PARAM(Properties);
}


System::Types::BitMask
TreeObject::GetDesiredSystemChanges(
    void
    )
{
    return System::Changes::Graphics::AllMesh | System::Changes::Custom;
}


Error
TreeObject::ChangeOccurred(
	ISubject* pSubject,
	System::Changes::BitMask ChangeType
	)
{
   if ( ChangeType & (System::Changes::Custom) )
    {
                //
        ISystemObject* pSysObj = dynamic_cast<ISystemObject*>(pSubject);
        pcstr name = pSysObj->GetSystemScene()->GetSystem()->GetName();
        //IGraphicsObject* pGfxObj = dynamic_cast<IGraphicsObject*>(pSubject);
        
        if (!strcmp(name,"ProceduralFire")) {
            FireObject* pFireObj = dynamic_cast<FireObject*>(pSubject);
            UNREFERENCED_PARAM(pFireObj);
            //pFireObj->somefunction();
        }
        if (!strcmp(name,"ProceduralTrees")) {
            TreeObject* pTreeObj = dynamic_cast<TreeObject*>(pSubject);
            //pTreeObj->somefuction();
            UNREFERENCED_PARAM(pTreeObj);
        }

    }

    return Errors::Success;
}


System::Changes::BitMask
TreeObject::GetPotentialSystemChanges(
    void
    )
{
    return System::Changes::Graphics::AllMesh | System::Changes::Custom;
}


u32 
TreeObject::GetSubMeshCount(
    void
    )
{
    // We use only one SubMesh
    return 1;
}


u32
TreeObject::GetIndexDeclaration(
    In  u16 nSubMeshIndex
    )
{
	// Ignored as we use only one SubMesh
	UNREFERENCED_PARAM( nSubMeshIndex );

    return IndexDecl::Type::Index16;
}


u32
TreeObject::GetVertexDeclarationCount(
   In  u16 nSubMeshIndex
    )
{
	// Ignored as we use only one SubMesh
	UNREFERENCED_PARAM( nSubMeshIndex );

    return NUM_VERTEXDECL_ELEMENTS;
}


void
TreeObject::GetVertexDeclaration(
    Out VertexDecl::Element* pVertexDecl,
	In  u16 nSubMeshIndex
    )
{
	// Ignored as we use only one SubMesh
	UNREFERENCED_PARAM( nSubMeshIndex );

    static const u32 Size = sizeof (VertexDecl::Element) * NUM_VERTEXDECL_ELEMENTS;
    static const VertexDecl::Element Decl[ NUM_VERTEXDECL_ELEMENTS ] =
    {
        { VertexDecl::Type ::Float3, VertexDecl::Usage::Position, 0 },
        { VertexDecl::Type ::Float3, VertexDecl::Usage::Normal, 0 },
        { VertexDecl::Type ::Float2, VertexDecl::Usage::Texture, 0 },
    };

    ::memcpy_s( pVertexDecl, Size, Decl, Size );
}


u32
TreeObject::GetIndexCount(
    In  u16 nSubMeshIndex
    )
{
	// Ignored as we use only one SubMesh
	UNREFERENCED_PARAM( nSubMeshIndex );

    if(m_PrimitiveType == Primitive_Branches)
    {
        return m_Tree->m_EndBranchIndexBuffer - m_Tree->m_StartBranchIndexBuffer;
    }else if(m_PrimitiveType == Primitive_Canopy)
    {
        return m_Tree->m_EndCanopyIndexBuffer - m_Tree->m_StartCanopyIndexBuffer;
    }

	// NOTE: what is appropriate default value?
	ASSERT( False );
	return 0;
}


u32
TreeObject::GetVertexCount(
    In  u16 nSubMeshIndex
    )
{
 	// Ignored as we use only one SubMesh
	UNREFERENCED_PARAM( nSubMeshIndex );

    if(m_PrimitiveType == Primitive_Branches)
    {
        return m_Tree->m_EndBranchVertexBuffer - m_Tree->m_StartBranchVertexBuffer;
    }else if(m_PrimitiveType == Primitive_Canopy)
    {
        return m_Tree->m_EndCanopyVertexBuffer - m_Tree->m_StartCanopyVertexBuffer;
    }

	// NOTE: what is appropriate default value?
	ASSERT( False );
	return 0;
}


void
TreeObject::GetIndices(
    Out void* pIndices,
    In  u16 nSubMeshIndex
    )
{
	// Ignored as we use only one SubMesh
	UNREFERENCED_PARAM( nSubMeshIndex );

    observer *Overseer = observer::Instance();
    ASSERT( Overseer != NULL );
	DBG_UNREFERENCED_LOCAL_VAR( Overseer );
    //added code
    Overseer->DXRS->CurrentIndex	= 0;
    Overseer->DXRS->CurrentVIndex	= 0;
    Overseer->DXRS->ptrIBData;
    Overseer->DXRS->BranchCount	= 0;
    Overseer->DXRS->ReverseWindingOrder = true;
    //end added Code
    Overseer->DXRS->ptrIBData = reinterpret_cast<WORD*>(pIndices);
    Overseer->DXRS->vpnt = NULL;
    if(m_PrimitiveType == Primitive_Branches)
    {
        m_Tree->fillBranches( m_Tree->m_nodeTree );
    }else if(m_PrimitiveType == Primitive_Canopy)
    {
        m_Tree->fillCanopies( m_Tree->m_nodeTree );
    }

}


void
TreeObject::GetVertices(
    Out void* pVertices,
    In  u16 nSubMeshIndex,
    In  u16 nStreamIndex,
    In  u32 nVertexDeclCount,
    In  VertexDecl::Element* pVertexDecl
    )
{
	// Ignored as we use only one SubMesh
	UNREFERENCED_PARAM( nSubMeshIndex );
	// Ignored as we use only one stream
	UNREFERENCED_PARAM( nStreamIndex );

    // nVertexDeclCount should be 0 to indicate the full vertex list
    ASSERT( 0 == nVertexDeclCount );
    UNREFERENCED_PARAM( pVertexDecl );
	UNREFERENCED_PARAM( nVertexDeclCount );

    observer *Overseer = observer::Instance();
    ASSERT( Overseer != NULL );
	DBG_UNREFERENCED_LOCAL_VAR( Overseer );
    //added code
    Overseer->DXRS->CurrentIndex	= 0;
    Overseer->DXRS->CurrentVIndex	= 0;
    Overseer->DXRS->ptrIBData;
    Overseer->DXRS->BranchCount	= 0;
    Overseer->DXRS->ReverseWindingOrder = true;
    //end added Code

    Overseer->DXRS->ptrIBData = NULL;
    Overseer->DXRS->vpnt = reinterpret_cast<VertexPNT*>(pVertices);
    if(m_PrimitiveType == Primitive_Branches)
    {
        m_Tree->fillBranches( m_Tree->m_nodeTree );
    }else if(m_PrimitiveType == Primitive_Canopy)
    {
        m_Tree->fillCanopies( m_Tree->m_nodeTree );
    }

}


void
TreeObject::GetAABB(
    Out Math::Vector3& Min,
    Out Math::Vector3& Max
    )
{
    //observer *Overseer = observer::Instance();
    //ASSERT( Overseer != NULL );

    //Min.x = Overseer->DXRS->AABB.xMin;
    //Min.y = Overseer->DXRS->AABB.yMin;
    //Min.z = Overseer->DXRS->AABB.zMin;

    //Max.x = Overseer->DXRS->AABB.xMax;
    //Max.y = Overseer->DXRS->AABB.yMax;
    //Max.z = Overseer->DXRS->AABB.zMax;
    Min.x = m_ObjectBoundingBox.xMin;
    Min.y = m_ObjectBoundingBox.yMin;
    Min.z = m_ObjectBoundingBox.zMin;

    Max.x = m_ObjectBoundingBox.xMax;
    Max.y = m_ObjectBoundingBox.yMax;
    Max.z = m_ObjectBoundingBox.zMax;
}

#pragma warning( pop )
