//Writen by Hugh Smith summer and fall 2007
// Part of Smoke Framework implementation for procedural Fire system.

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

#include "..\BaseTypes\TbbSpinMutex.h"

class FireSystem;
class FireScene;
class FireTask;

class Tree;
class TreeScene;
class TreeObject;
struct LevelDetail;
#include "Fire.h"
#include "aabb.h"
#include "FireScene.h"

// CAUTION 
// Combination FIREOBJ_PREBUILD_VERTICES==0 && FIREOBJ_PARALLEL_BUILD_VERTICES==1 will deadlock!

// By prebuilding vertex buffers we trade space for performance. Prebuild gives us
// two benefits:
// 1) Shortens serialized part of OGREGraphicsObjectParticles::ChangeOccurred processing
//    (and it is the major contributor to the change distribution stage)
// 2) Brings some work from change distribution stage to the execution one. Since
//    execution stage has a long serial rendering task, this extra work allows
//    to decrease the number of frames where rendering would have otherwise become
//    the limiting (longest) operation.
#define FIREOBJ_PREBUILD_VERTICES 0

// Use the constant BuildVerticesGrainSize defined in FireObject.cpp for fine 
// tuning load balance vs. overhead.
#define FIREOBJ_PARALLEL_BUILD_VERTICES 1

///////////////////////////////////////////////////////////////////////////////
/// <summary>
///  Implementation of the ISystemObject interface for the fire system.
///  See Interfaces\Graphics.h and Interfaces\System.h for a definition of the
///   class and its functions.
/// </summary>
///////////////////////////////////////////////////////////////////////////////

class FireObject : public ISystemObject, public IGraphicsObject, public IGraphicsParticleObject, public IAreaObject
{
    friend FireSystem;
    friend FireScene;
    friend FireTask;

public:
    Bool StepUpdate( f32 fDeltaTime );
    virtual Bool fireIsActive();

    Bool m_bExtinguished;

protected:

#if NO_MUTUAL_FIRE_COLLISION_CHECKS
    FireObject( ISystemScene* pSystemScene, pcstr pszType, pcstr pszName, u32 nIndex );
#else
    FireObject( ISystemScene* pSystemScene, pcstr pszType, pcstr pszName );
#endif
    ~FireObject( void );

    /// <summary cref="FireObject::GetSystemType">
    ///   Implementation of the ISystemObject GetSystemType function.
    /// </summary>
    /// <seealso cref="ISystemObject::GetSystemType"/>
    virtual System::Type GetSystemType( void );

    virtual Error Initialize( std::vector<Properties::Property> Properties );
    virtual void GetProperties( Properties::Array& Properties );
    virtual void SetProperties( Properties::Array Properties );


    virtual System::Types::BitMask GetDesiredSystemChanges( void );

    /// <summary cref="FireObject::ChangeOccurred">
    ///   Implementation of the IObserver ChangeOccurred function.
    /// </summary>
    /// <seealso cref="IObserver::ChangeOccurred"/>
    virtual Error ChangeOccurred( ISubject* pSubject, System::Changes::BitMask ChangeType );

    /// <summary cref="FireObject::GetPotentialSystemChanges">
    ///   Implementation of the ISubject GetPotentialSystemChanges function.
    /// </summary>
    /// <seealso cref="ISubject::GetPotentialSystemChanges"/>
    virtual System::Changes::BitMask GetPotentialSystemChanges( void );

    ///<summary cref="FireObject::FillPostedData">
    /// local function to fill shared posted data
    ///</summary>
    virtual void FillPostedData();

    /// <summary cref="FireObject::GetParticleGroupCount">
    ///   Implementation of the IGraphicsParticleObject GetParticleGroupCount function.
    /// </summary>
    /// <seealso cref="IGraphicsParticleObject::GetParticleGroupCount"/>
    virtual std::string GetParticleGroupTechnique(void);

    /// <summary cref="FireObject::GetSubMeshCount">
    ///   Implementation of the IGraphicsObject GetSubMeshCount function.
    /// </summary>
    /// <seealso cref="IGraphicsObject::GetSubMeshCount"/>
    virtual u32 GetSubMeshCount( void );

    /// <summary cref="FireObject::GetIndexDeclaration">
    ///   Implementation of the IGraphicsObject GetIndexDeclaration function.
    /// </summary>
    /// <seealso cref="IGraphicsObject::GetIndexDeclaration"/>
    virtual u32 GetIndexDeclaration( In  u16 nSubMeshIndex );

    /// <summary cref="FireObject::GetVertexDeclarationCount">
    ///   Implementation of the IGraphicsObject GetVertexDeclarationCount function.
    /// </summary>
    /// <seealso cref="IGraphicsObject::GetVertexDeclarationCount"/>
    virtual u32 GetVertexDeclarationCount( In  u16 nSubMeshIndex );

    /// <summary cref="FireObject::GetVertexDeclaration">
    ///   Implementation of the IGraphicsObject GetVertexDeclaration function.
    /// </summary>
    /// <seealso cref="IGraphicsObject::GetVertexDeclaration"/>
    virtual void GetVertexDeclaration( Out VertexDecl::Element* pVertexDecl,
									   In  u16 nSubMeshIndex );

    /// <summary cref="FireObject::GetIndexCount">
    ///   Implementation of the IGraphicsObject GetIndexCount function.
    /// </summary>
    /// <seealso cref="IGraphicsObject::GetIndexCount"/>
    virtual u32 GetIndexCount( In  u16 nSubMeshIndex );

    /// <summary cref="FireObject::GetVertexCount">
    ///   Implementation of the IGraphicsObject GetVertexCount function.
    /// </summary>
    /// <seealso cref="IGraphicsObject::GetVertexCount"/>
    virtual u32 GetVertexCount( In  u16 nSubMeshIndex );

    /// <summary cref="FireObject::GetIndices">
    ///   Implementation of the IGraphicsObject GetIndices function.
    /// </summary>
    /// <seealso cref="IGraphicsObject::GetIndices"/>
    virtual void GetIndices( Out void* pIndices,
                             In  u16 nSubMeshIndex );

    /// <summary cref="FireObject::GetVertices">
    ///   Implementation of the IGraphicsObject GetVertices function.
    /// </summary>
    /// <seealso cref="IGraphicsObject::GetVertices"/>
    virtual void GetVertices( Out void* pVertices,
                              In  u16 nSubMeshIndex,
                              In  u16 nStreamIndex,
                              In  u32 nVertexDeclCount,
                              In  VertexDecl::Element* pVertexDecl );

    /// <summary cref="FireObject::GetStreamsChanged">
    ///   Implementation of the IGraphicsObject GetStreamsChanged function.
    /// </summary>
    /// <returns>Always returns 1 to indicate change to stream 0 only.</returns>
    virtual u32 GetStreamsChanged( void ) { return 1; }

    /// <summary cref="FireObject::GetAABB">
    ///   Implementation of the IGraphicsObject GetAABB function.
    /// </summary>
    /// <seealso cref="IGraphicsObject::GetAABB"/>
    virtual void GetAABB( Out Math::Vector3& Min, Out Math::Vector3& Max );
   
     /// <summary cref="FireObject::GetAreaBB">
    ///   Implementation of the IAreaObject GetAreaBB function.
    /// </summary>
	virtual void GetAreaBB( Out Math::Vector3& Min, Out Math::Vector3& Max );

    /// <summary cref="FireObject::GetAreaName">
    ///   Implementation of the IAreaObject GetAreaName function.
    /// </summary>
    virtual pcstr GetAreaName( void ) { return GetName(); }

    /// <summary cref="FireObject::GetAreaType">
    ///   Implementation of the IAreaObject GetAreaType function.
    /// </summary>
    virtual AreaType GetAreaType( void ) { return AreaType::e_Fire; }

    /// <summary cref="FireObject::IsAreaActive">
    ///   Implementation of the IAreaObject IsAreaActive function.
    /// </summary>
    virtual Bool IsAreaActive( void ) { return fireIsActive(); };
    
    ///<summary cref="FireObject::update">
    /// local function available for Task update to call
    /// so object can deal with local data and do any posting
    /// of new notifications
    ///</summary> 
    virtual void update(f32 fDeltaTime, const FireObjectList& fireObjectList);

    /// <summary cref="FireObject::createFireSystem">
    ///   Implementation of the createFireSystem function.
    ///   Creates the structures, initializes transforms, and sets up the particles systems for the fire
    ///   object
    /// </summary>
    virtual void createFireSystem();

    /// <summary cref="FireObject::checkCollision">
    ///   Implementation of the checkCollision function.
    ///   fine grained collision check between a particle or ray and a bounding box
    /// </summary>
    Bool checkCollision(V3 pos,V3 prePos,aabb AABB);

    /// <summary cref="FireObject::EmitterCollisionCheck">
    ///   Implementation of the EmmitterCollisionCheck function.
    ///   setup for the particles to get them into the same space and data types
    ///   as the bounding boxes.  this is a prelininary setup for the collision check
    /// </summary>
    void EmitterCollisionCheck( const FireObjectList& fireObjectList, 
                                ParticleEmitter::HeatEmitter * heatEm );
    virtual void Vector4_2_V3(Math::Vector4 &v4, V3 &v3); 
    virtual void V3_2_Vector4(V3 &v3, Math::Vector4 &v4); 

private:
    /// <summary cref="FireObject::SetTransformsAsZeroBasedLS">
    ///   Implementation of the SetTransformsAsZeroBasedLS function.
    ///   creates a transform for a zero based Local Space transform
    /// </summary>
    virtual void SetTransformsAsZeroBasedLS();

    /// <summary cref="FireObject::SetWSBBox">
    ///   Implementation of the SetWSBBox function.
    ///   sets a bounding box in World space rather than local space coordinates
    /// </summary>
    virtual void SetWSBBox();

    virtual ParticleEmitter::HeatEmitter* GetEmitter();


protected:


    //Enumerated Types
    enum PropertyTypesPrimitives
    {
        Primitive_Sphere, Primitive_Box,
        Primitive_Patch, Primitive_Unknown,
        PropertyType_Count
    };
    PropertyTypesPrimitives             m_PrimitiveType;


    enum Types
    {
        Type_TreeFire, Type_CanopyFire, Type_HouseFire, Type_SphereFire, Type_BoxFire, Type_PatchFire, Type_ColdParticle, Type_Unknown
    };
    Types                               m_Type;

    enum PropertyTypes
    {
        Property_Technique, Property_Emitter,
        Property_FPImpulse,         Property_HPImpulse,
        Property_FPShift,           Property_HPShift,
        Property_FPDensity,         Property_HPDensity,
        Property_FPTimePerParticle, Property_HPTimePerParticle,
        Property_FPMaxLifeTime,     Property_HPMaxLifeTime,     
        Property_FPMinLifeTime,     Property_HPMinLifeTime,     
        Property_FPMaxSize,         Property_HPMaxSize,     
        Property_FPMinSize,         Property_HPMinSize,     
        Property_PrimitiveType,     Property_ShowHP,  
        Property_OnFire,            Property_Count
    };

    //static variables
    static pcstr                        sm_kapszTypeNames[];
	static pcstr						sm_kapszEmitterTypes[];
    static pcstr                        sm_kapszPropertyNames[];
    static const Properties::Property   sm_kaDefaultProperties[];

    //Property variable
	u32				m_BurnStatus;
	pcstr			m_Emitter;
	i32			    m_FPDensity;
	i32			    m_HPDensity;
    Math::Vector3   m_FPImpulse;
    Math::Vector3   m_HPImpulse;
    Math::Vector3   m_FPShift;
    Math::Vector3   m_HPShift;
    f32             m_FPTimePerParticle;
    f32             m_HPTimePerParticle;
    f32             m_FPMaxSize;
    f32             m_HPMaxSize;
    f32             m_FPMinSize;
    f32             m_HPMinSize;
    f32             m_FPMaxLifeTime;
    f32             m_HPMaxLifeTime;
    f32             m_FPMinLifeTime;
    f32             m_HPMinLifeTime;


    struct FireSystemData {
        Math::Vector3 position;
        Math::Vector3 scale;
        std::vector<Math::Vector3> * particlePositions;

    };
    //Structures
    // point pair used to establish the extends for a fire particle system
    struct PointPair {
        V3 basePoint;
        V3 extendPoint;
        B3 basis;
        aabb AABB;
        Bool burning;
   };

    // vertex structure for the FireParticle
    // structure allows for conversion to generic particle used for the graphics interface.
	struct VertexFP {
		Math::Vector3	Pos;
		Math::Vector3	Tex0;
        f32	Tex1;
        f32	Tex2;
        f32	Tex3;
        f32	Tex4;
        f32	Tex5;
		f32	Dif;

		VertexFP(ParticleEmitter::Particle *particle){
			ParticleEmitter::Particle ptl = *particle;
			Pos.x = ptl.initialPos.x;
			Pos.y = ptl.initialPos.y;
			Pos.z = ptl.initialPos.z;
			Tex0.x = ptl.initialVelocity.x;
			Tex0.y = ptl.initialVelocity.y;
			Tex0.z = ptl.initialVelocity.z;
			Tex1 = ptl.initialSize;
			Tex2 = ptl.initialTime;
			Tex3 = ptl.lifeTime;
			Tex4 = ptl.mass;
			Tex5 = ptl.age;
			Dif  = static_cast<f32>(ptl.initialColor);
		}

		static void convertFromParticle(VertexFP* dst, ParticleEmitter::Particle* ptl)
		{
            dst->Pos.x = ptl->initialPos.x;
			dst->Pos.y = ptl->initialPos.y;
			dst->Pos.z = ptl->initialPos.z;
			dst->Tex0.x = ptl->initialVelocity.x;
			dst->Tex0.y = ptl->initialVelocity.y;
			dst->Tex0.z = ptl->initialVelocity.z;
			dst->Tex1 = ptl->initialSize;
			dst->Tex2 = ptl->initialTime;
			dst->Tex3 = ptl->lifeTime;
			dst->Tex4 = ptl->mass;
			dst->Tex5 = ptl->age;
			dst->Dif  = static_cast<f32>(ptl->initialColor);
		}
	};

    //Vertex structure for a heat particle
	struct VertexHP {
		Math::Vector3	Pos;

        VertexHP(ParticleEmitter::HeatParticle *particle){
            ParticleEmitter::HeatParticle ptl = *particle;
			Pos.x = ptl.currentPos.x;
			Pos.y = ptl.currentPos.y;
			Pos.z = ptl.currentPos.z;
		}

        static void convertFromHeatParticle(VertexHP* dst, ParticleEmitter::HeatParticle* ptl)
		{
			dst->Pos.x = ptl->currentPos.x;
			dst->Pos.y = ptl->currentPos.y;
			dst->Pos.z = ptl->currentPos.z;
		}
	};


    typedef struct {
       	std::vector<PointPair*>         pointPairs;
    } RetrievedPostedData, *pRetrievedPostedData;

    typedef struct {
        std::vector<ParticleEmitter::ParticleSystem *>  Fires;
       	std::vector<PointPair*>         pointPairs;
    } PostedData, *pPostedData;

    typedef struct {
        Bool position;
        Bool orientation;
        Bool scale;
        Bool aabb;
        Bool created;
    } createParams;

    void *                                  m_KAV;
    std::string                             m_Technique;
    createParams                            m_CreateParams;

    Math::Vector3                           m_CameraPosition;

    Math::Vector3                           m_ObjectPosition;
    Math::Vector3 *                         m_pObjectPosition;
                       
    Math::Vector3                           m_ObjectScale;
    Math::Vector3 *                         m_pObjectScale;

    Math::Quaternion                        m_ObjectOrientation;
    Math::Quaternion *                      m_pObjectOrientation;

    Math::Matrix4x4                         m_Transform2WS;
    Math::Matrix4x4                         m_Transform2LS;

    Math::Matrix4x4                         m_BBTransform2WS;
    Math::Matrix4x4                         m_BBTransform2LS;

    std::vector<Math::Vector3>              m_ParticlePositions;
    std::vector<Math::Vector3> *            m_pParticlePositions;
 
    // Key Objects
    float                                   m_CurrentTime;

public:
    std::vector<ParticleEmitter::ParticleSystem *>     m_Fires;

    typedef ParticleEmitter::HeatEmitter::Emitter_Type Emitter_Type;

    struct CollisionCheckInfo
    {
        CollisionCheckInfo ( FireObject::Emitter_Type et )
            : m_EmitterType(et)
        {}

        FireObject::Emitter_Type    m_EmitterType;
        std::vector<V3>             m_vPositions;
        std::vector<V3>             m_vPrevPositions;
    };

protected:
    Bool                                    m_bRenderHeatParticles;
    Bool                                    m_bOnFire;
    
    // BurnState denotes the current state of a branch
    // Note that in the current code, once a burning item has been marked Extinguished,
    // it cannot be set to Burning again (to allow users to put out the fire undefeatably).
    enum BurnState
    {
        Normal,
        Burning,
        Extinguished,
    };

    std::vector<BurnState>                       m_BurningList;
    std::vector<aabb>                       m_BoundingBoxList;
    
    aabb                                    m_ObjectWSBBox; // world space bb
    aabb                                    m_ObjectOSBBox; // local "object" space bb
    pPostedData                             m_pPostedData;
    pRetrievedPostedData                    m_pRetrievedPostedData;
    ParticleEmitter::ParticleSystem *       m_Fire;
	f32										m_Age;
	u32										m_ParticleVertexCount;
	Math::Vector3							m_minExtends;
	Math::Vector3							m_maxExtends;

    Bool                                    m_fireIsActive;

#if NO_MUTUAL_FIRE_COLLISION_CHECKS
    u32             m_nIndex;
    #if MUTUAL_CHECK_TWEAK
    bool            m_bNextIsReverse;   
    #endif
#endif

    f32             m_fDeltaTime;
    const FireObjectList  *m_pFireObjectList;

    static void FireCollisionCallback( void *param, u32 begin, u32 end );
	void ProcessFireCollisionsRange ( u32 begin, u32 end, CollisionCheckInfo* pcci );

    static void UpdateCallback( void *param, u32 begin, u32 end );
	void UpdateRange ( u32 begin, u32 end );

    Bool            m_bDirty;
	Math::Vector3	m_newMinExtends;
	Math::Vector3	m_newMaxExtends;

    void BuildVertexBuffer( void* pVertices );

#if FIREOBJ_PREBUILD_VERTICES
    void*   m_pVertexBuffer;
    u32     m_nVertexBufferSize;    // bytes
    u32     m_nVertexBufferCapacity;
    u32     m_nVertexSize;
#endif /* FIREOBJ_PREBUILD_VERTICES */

#if FIREOBJ_PARALLEL_BUILD_VERTICES
    void*   m_pVertices;
    DEFINE_SPIN_MUTEX(m_mutex);

    /// <summary cref="FireTask::UpdateCallback">
    ///   Invoked by ParalellFor algorithm to build a range of vertices.
    /// </summary>
	static void BuildVerticesCallback( void *param, u32 begin, u32 end );

    /// <summary cref="FireTask::UpdateCallback">
    ///   Builds a range of vertices.
    /// </summary>
	void BuildVerticesRange( u32 begin, u32 end );
#endif /* FIREOBJ_PARALLEL_BUILD_VERTICES */
};
