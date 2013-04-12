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

#pragma once

//#include "..\..\extern\trees\Vector3.h"
//#include "..\..\extern\trees\aabb.h"
#include "..\trees\Vector3.h"
#include "..\trees\aabb.h"
#include "TreeScene.h"
class TreeSystem;
class TreeScene;
class TreeTask;
class Tree;
class Canopy;
struct LevelDetail;
class BranchBase;


/*******************************************************************************
* CLASS: TreeObject
*
* DESCRIPTION:
* Implementation of the ISystemObject interface.
* See Interfaces\Graphics.h and Interfaces\System.h for a definition of the
*  class and its functions.
*******************************************************************************/

class TreeObject : public ISystemObject, public IGraphicsObject
{
    friend TreeSystem;
    friend TreeScene;
    friend TreeTask;

public:
    /// <summary cref="TreeObject::GetObjectPostedVariables">
    ///   allows a mechanism to get fullfill requests for data
    ///   this object has posted
    /// </summary>
	virtual void* GetObjectPostedVariables(void *Params);

	 Error Test( std::vector<Properties::Property> Properties );

protected:

    TreeObject( ISystemScene* pSystemScene, pcstr pszType );
    ~TreeObject( void );

    /// <summary cref="ISystemObject::GetSystemType">
    ///   Implementation of the ISystemObject GetSystemType function.
    /// </summary>
    virtual System::Type GetSystemType( void );

    virtual Error Initialize( std::vector<Properties::Property> Properties );
    virtual void GetProperties( Properties::Array& Properties );
    virtual void SetProperties( Properties::Array Properties );

    virtual System::Types::BitMask GetDesiredSystemChanges( void );

    /// <summary cref="IObserver::ChangeOccurred">
    ///   Implementation of the IObserver ChangeOccurred function.
    /// </summary>
    virtual Error ChangeOccurred( ISubject* pSubject, System::Changes::BitMask ChangeType );

    /// <summary cref="ISubject::GetPotentialSystemChanges">
    ///   Implementation of the ISubject GetPotentialSystemChanges function.
    /// </summary>
    virtual System::Changes::BitMask GetPotentialSystemChanges( void );

    /// <summary cref="IGraphicsObject::GetSubMeshCount">
    ///   Implementation of the IGraphicsObject GetSubMeshCount function.
    /// </summary>
    virtual u32 GetSubMeshCount( void );

    /// <summary cref="IGraphicsObject::GetIndexDeclaration">
    ///   Implementation of the IGraphicsObject GetIndexDeclaration function.
    /// </summary>
    virtual u32 GetIndexDeclaration( In  u16 nSubMeshIndex );

    /// <summary cref="IGraphicsObject::GetVertexDeclarationCount">
    ///   Implementation of the IGraphicsObject GetVertexDeclarationCount function.
    /// </summary>
    virtual u32 GetVertexDeclarationCount( In  u16 nSubMeshIndex );

    /// <summary cref="IGraphicsObject::GetVertexDeclaration">
    ///   Implementation of the IGraphicsObject GetVertexDeclaration function.
    /// </summary>
    virtual void GetVertexDeclaration( Out VertexDecl::Element* pVertexDecl,
									   In  u16 nSubMeshIndex );

    /// <summary cref="IGraphicsObject::GetIndexCount">
    ///   Implementation of the IGraphicsObject GetIndexCount function.
    /// </summary>
    virtual u32 GetIndexCount( In  u16 nSubMeshIndex );

    /// <summary cref="IGraphicsObject::GetVertexCount">
    ///   Implementation of the IGraphicsObject GetVertexCount function.
    /// </summary>
    virtual u32 GetVertexCount( In  u16 nSubMeshIndex );

    /// <summary cref="IGraphicsObject::GetIndices">
    ///   Implementation of the IGraphicsObject GetIndices function.
    /// </summary>
    virtual void GetIndices( Out void* pIndices,
                             In  u16 nSubMeshIndex );

    /// <summary cref="IGraphicsObject::GetVertices">
    ///   Implementation of the IGraphicsObject GetVertices function.
    /// </summary>
    virtual void GetVertices( Out void* pVertices,
                              In  u16 nSubMeshIndex,
                              In  u16 nStreamIndex,
                              In  u32 nVertexDeclCount,
                              In  VertexDecl::Element* pVertexDecl );

    /// <summary>
    ///   Implementation of the IGraphicsObject GetStreamsChanged function.
    /// </summary>
    /// <returns>Always returns 1 to indicate change to stream 0 only.</returns>
    virtual u32 GetStreamsChanged( void ) { return 1; }

    /// <summary cref="IGraphicsObject::GetAABB">
    ///   Implementation of the IGraphicsObject GetAABB function.
    /// </summary>
    virtual void GetAABB( Out Math::Vector3& Min, Out Math::Vector3& Max );

    ///<summary cref="TreeObject::FillPostedData">
    /// local function to fill shared posted data
    ///</summary> 
    virtual void FillPostedData();
    ///<summary cref="TreeObject::update">
    /// local function available for Task update to call
    /// so object can deal with local data and do any posting
    /// of new notifications
    ///</summary> 
    virtual void update(f32 DeltaTime);

protected:

    //Enumerated Types
    enum PropertyTypesPrimitives
    {
        Primitive_Branches, Primitive_Canopy,
        PropertyType_Count
    };
    PropertyTypesPrimitives             m_PrimitiveType;

    enum PropertyTypes
    {
        Property_Position, Property_Seed, Property_LevelCount,
        Property_PrimitiveType, Property_Count
    };

    //static variables
    static pcstr                        sm_kapszPropertyNames[];
    static const Properties::Property   sm_kaDefaultProperties[];

    //Property varialble
    Math::Vector3   m_Position;
    i32             m_Seed ;
    i32             m_LevelCount;
    Bool            m_Initialized;
    std::string     m_GrammarType;

    //Structures
    struct PointPair {
        V3 basePoint;
        V3 extendPoint;
        B3  basis;
        aabb AABB;
        Bool burning;
    };

    typedef struct {
       	std::vector<PointPair*>         pointPairs;
    } *pPostedData, PostedData;
    
    // Key Objects
    float                           m_CurrentTime;
    Tree *                          m_Tree;
	LevelDetail *                   m_GrammarDetails;
    std::vector<BranchBase*>        m_TreeNodeList;
    std::vector<Canopy*>            m_TreeNodeCanopyList;
    aabb                            m_ObjectBoundingBox;
    pPostedData                     m_pPostedData;


};
