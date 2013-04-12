// Writen by Hugh Smith summer and fall 2007
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


// core includes
//
#include "stdafx.h"
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

// core includes continued...
//
#include "..\Trees\Vertex.h"
#include "..\Trees\Vector3.h"
#include "Fire.h"
#include "..\Trees\Tree.h"
#include "aabb.h"
#include "Fire.h"
#include "FireBall.h"
#include "FirePatch.h"
#include "ColdParticle.h"

//
// System includes
//
#include "..\Input\InputObject.h"

#include  "..\ProceduralTrees\TreeObject.h"
#define NUM_VERTEX_FP_DECL_ELEMENTS         8
#define NUM_VERTEX_HP_DECL_ELEMENTS         1

#include "FireObject.h"

#pragma warning( push )
#pragma warning( disable : 6386 )

extern ManagerInterfaces   g_FireManagers;

#if FIREOBJ_PREBUILD_VERTICES
    #include <tbb/scalable_allocator.h>
#endif


// Grain sizes of the parallel jobs. The less is the grainsize the better is 
// the load balance, and the higher is the parallel overhead. 
static const u32 FireObjectUpdateGrainSize = 10;  // 10 - 20
static const u32 CollisionCheckGrainSize = 20;  // 10 - 20
static const u32 BuildVerticesGrainSize = 10;


pcstr FireObject::sm_kapszTypeNames[] =
{
    "TreeFire", "CanopyFire", "HouseFire", "SphereFire", "BoxFire", "PatchFire", "ColdParticle", "Unknown",
    NULL
};


pcstr FireObject::sm_kapszPropertyNames[] =
{
    "Technique",         "Emitter", 
    "FPImpulse",         "HPImpulse",
    "FPShift",           "HPShift",  
    "FPDensity",         "HPDensity",
    "FPTimePerParticle", "HPTimePerParticle",
    "FPMaxLifeTime",     "HPMaxLifeTime",     
    "FPMinLifeTime",     "HPMinLifeTime",   
    "FPMaxSize",         "HPMaxSize",     
    "FPMinSize",         "HPMinSize",
    "PrimitiveType",     "ShowHP",
    "OnFire",
};

pcstr FireObject::sm_kapszEmitterTypes[]=
{
    "Line", "Circle", NULL
};

const Properties::Property FireObject::sm_kaDefaultProperties[] =
{
    Properties::Property( sm_kapszPropertyNames[ Property_Technique ],
						  VALUE1(Properties::Values::String),
                          Properties::Flags::Valid | Properties::Flags::WriteOnly,
                          "TechniqueName", NULL, NULL, NULL,
						  ""
                          ),
    Properties::Property( sm_kapszPropertyNames[ Property_Emitter ],
						  VALUE1( Properties::Values::Enum ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          "EType", NULL, NULL, NULL,
						  0
                          ),
     Properties::Property( sm_kapszPropertyNames[ Property_FPImpulse ],
                          Properties::Values::Vector3,
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          Math::Vector3::Zero ),
    Properties::Property( sm_kapszPropertyNames[ Property_HPImpulse ],
                          Properties::Values::Vector3,
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          Math::Vector3::Zero ),
    Properties::Property( sm_kapszPropertyNames[ Property_FPShift ],
                          Properties::Values::Vector3,
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          Math::Vector3::Zero ),
    Properties::Property( sm_kapszPropertyNames[ Property_HPShift ],
                          Properties::Values::Vector3,
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          Math::Vector3::Zero ),
   Properties::Property( sm_kapszPropertyNames[ Property_FPDensity ],
                          VALUE1( Properties::Values::Int32 ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          0 ),
    Properties::Property( sm_kapszPropertyNames[ Property_HPDensity ],
                          VALUE1( Properties::Values::Int32 ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          0 ),
    Properties::Property( sm_kapszPropertyNames[ Property_FPTimePerParticle ],
                          VALUE1( Properties::Values::Float32 ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          0.0f ),
    Properties::Property( sm_kapszPropertyNames[ Property_HPTimePerParticle ],
                          VALUE1( Properties::Values::Float32 ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          0.0f ),
    Properties::Property( sm_kapszPropertyNames[ Property_FPMaxLifeTime ],
                          VALUE1( Properties::Values::Float32 ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          0.0f ),
    Properties::Property( sm_kapszPropertyNames[ Property_HPMaxLifeTime ],
                          VALUE1( Properties::Values::Float32 ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          0.0f ),
    Properties::Property( sm_kapszPropertyNames[ Property_FPMinLifeTime ],
                          VALUE1( Properties::Values::Float32 ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          0.0f ),
    Properties::Property( sm_kapszPropertyNames[ Property_HPMinLifeTime ],
                          VALUE1( Properties::Values::Float32 ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          0.0f ),
    Properties::Property( sm_kapszPropertyNames[ Property_FPMaxSize ],
                          VALUE1( Properties::Values::Float32 ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          0.0f ),
    Properties::Property( sm_kapszPropertyNames[ Property_HPMaxSize ],
                          VALUE1( Properties::Values::Float32 ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          0.0f ),
    Properties::Property( sm_kapszPropertyNames[ Property_FPMinSize ],
                          VALUE1( Properties::Values::Float32 ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          0.0f ),
    Properties::Property( sm_kapszPropertyNames[ Property_HPMinSize ],
                          VALUE1( Properties::Values::Float32 ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          0.0f ),
    Properties::Property( sm_kapszPropertyNames[ Property_PrimitiveType ],
                          VALUE1( Properties::Values::String ),
                          Properties::Flags::Valid | Properties::Flags::InitOnly,
                          NULL, NULL, NULL, NULL,
                          "" ),
    Properties::Property( sm_kapszPropertyNames[ Property_ShowHP ],
                          VALUE1( Properties::Values::Boolean ),
                          Properties::Flags::Valid,
                          NULL, NULL, NULL, NULL,
                          False ),
    Properties::Property( sm_kapszPropertyNames[ Property_OnFire ],
                          VALUE1( Properties::Values::Boolean ),
                          Properties::Flags::Valid,
                          NULL, NULL, NULL, NULL,
                          False ),
};

FireObject::FireObject(
    ISystemScene* pSystemScene
    , pcstr pszType
    , pcstr pszName
#if NO_MUTUAL_FIRE_COLLISION_CHECKS
    , u32 nIndex
#endif
    )
    : ISystemObject( pSystemScene, NULL)
    , m_PrimitiveType(Primitive_Unknown)
    , m_fireIsActive(false)
    , m_bRenderHeatParticles(false)
#if NO_MUTUAL_FIRE_COLLISION_CHECKS
    , m_nIndex(nIndex)
    #if MUTUAL_CHECK_TWEAK
    , m_bNextIsReverse(false)
    #endif
#endif
    , m_bDirty(True)
#if FIREOBJ_PREBUILD_VERTICES
    , m_pVertexBuffer(NULL)
    , m_nVertexBufferSize(0)
    , m_nVertexBufferCapacity(0)
    , m_nVertexSize(sizeof(VertexFP))
#endif /* FIREOBJ_PREBUILD_VERTICES */
{    
    ASSERT( Property_Count == sizeof sm_kapszPropertyNames /
        sizeof sm_kapszPropertyNames[ 0 ] );
    ASSERT( Property_Count == sizeof sm_kapszPropertyNames /
        sizeof sm_kapszPropertyNames[ 0 ] );

    m_bExtinguished = false;

    m_pRetrievedPostedData = NULL;

    if ( strcmp( pszType, sm_kapszTypeNames[ Type_TreeFire ] ) == 0 )
    {
        m_Type = Type_TreeFire;
    }
    else if( strcmp( pszType, sm_kapszTypeNames[ Type_CanopyFire ] ) == 0 )
    {
        m_Type = Type_CanopyFire;
    }
    else if ( strcmp( pszType, sm_kapszTypeNames[ Type_HouseFire ] ) == 0 )
    {
        m_Type = Type_HouseFire;
    }
    else if ( strcmp( pszType, sm_kapszTypeNames[ Type_SphereFire ] ) == 0 )
    {
        m_Type = Type_SphereFire;
    }
    else if ( strcmp( pszType, sm_kapszTypeNames[ Type_BoxFire ] ) == 0 )
    {
        m_Type = Type_BoxFire;
    }
    else if ( strcmp( pszType, sm_kapszTypeNames[ Type_PatchFire ] ) == 0 )
    {
        m_Type = Type_PatchFire;
    }
    else if ( strcmp( pszType, sm_kapszTypeNames[ Type_ColdParticle ] ) == 0 )
    {
        m_Type = Type_ColdParticle;
    }
    else if ( strcmp( pszType, sm_kapszTypeNames[ Type_Unknown ] ) == 0 )
    {
        m_Type = Type_Unknown;
    }
    else
    {
        ASSERT( False );
    }
    m_pPostedData = new PostedData();
    m_ObjectPosition.x = 0.0f;
    m_ObjectPosition.y = 0.0f;
    m_ObjectPosition.z = 0.0f;
    m_pObjectPosition = &m_ObjectPosition;
    m_ObjectScale = m_ObjectScale.One;
    Math::Vector3 axis;
    m_ObjectOrientation = m_ObjectOrientation.Set(axis.UnitY, 0.0f);
    m_ParticlePositions.reserve(300);
    m_ParticlePositions.resize(0);
    m_pParticlePositions = &m_ParticlePositions;
    m_CreateParams.aabb = false;
    m_CreateParams.orientation = false;
    m_CreateParams.position = false;
    m_CreateParams.scale = false;
    m_CreateParams.created = false;
    m_FPDensity = 30;
    m_HPDensity = 30;
    //m_FPImpulse;
    //m_HPImpulse;
    //m_FPShift;
    //m_HPShift;
    m_FPTimePerParticle = 0.025f;
    m_HPTimePerParticle = 0.025f;
    m_FPMaxLifeTime = 2.0f;
    m_HPMaxLifeTime = 2.0f;
    m_FPMinLifeTime = 1.0f;
    m_HPMinLifeTime = 1.0f;

    m_KAV = 0;
	m_minExtends = Math::Vector3::One;
    m_maxExtends = Math::Vector3::One;

    SetName( pszName );
}


FireObject::~FireObject(
    void
    )
{
	// TODO: this has to work properly when called during an update(?)
    //reinterpret_cast<FireScene*>(GetSystemScene())->GetFires().remove(this);

    m_ParticlePositions.clear();
    m_pParticlePositions=0;
 
    // Key Objects
    
    std::vector<ParticleEmitter::ParticleSystem *>::iterator  fI,fIend;
    fI = m_Fires.begin();
    fIend = m_Fires.end();
    for(fI;fI!=fIend;fI++){
        delete (*fI);
    }
    m_Fires.clear();
    m_BurningList.clear();
    m_BoundingBoxList.clear();
    delete m_pPostedData;

#if FIREOBJ_PREBUILD_VERTICES
//    delete m_pVertexBuffer;
    scalable_free(m_pVertexBuffer);
#endif /* FIREOBJ_PREBUILD_VERTICES */
}

System::Type
FireObject::GetSystemType(
    void
    )
{
    return System::Types::MakeCustom( 2 );
}


Error 
FireObject::Initialize( std::vector<Properties::Property> Properties )
{    
    for ( Properties::Iterator it=Properties.begin(); it != Properties.end(); it++ )
    {
            std::string sName = it->GetName();

            if ( sName == sm_kapszPropertyNames[ Property_Technique ] )
            {
                    m_Technique = it->GetString( 0 );  
            } else if (sName == sm_kapszPropertyNames[ Property_Emitter ] )
            {
	                m_Emitter = it->GetEnumOption( 0 );
            } else if ( sName == sm_kapszPropertyNames[ Property_PrimitiveType ] )
            {
                pcstr pt = it->GetStringPtr(0);
                if(!strcmp(pt,"Sphere")){
                    m_PrimitiveType = Primitive_Sphere;
                }else if(!strcmp(pt,"Box")){
                    m_PrimitiveType = Primitive_Box;
                }else if(!strcmp(pt,"Patch")){
                    m_PrimitiveType = Primitive_Patch;
                }else {
                    m_PrimitiveType = Primitive_Unknown;
                }
                //
                // Set this property to invalid since it's already been read.
                //
                it->ClearFlag( Properties::Flags::Valid );
            }else if (sName == sm_kapszPropertyNames[ Property_FPDensity ] )
            {
	                m_FPDensity = it->GetInt32( 0 );
            }else if (sName == sm_kapszPropertyNames[ Property_HPDensity ] )
            {
	                m_HPDensity = it->GetInt32( 0 );
            }else if (sName == sm_kapszPropertyNames[ Property_FPImpulse ] )
            {
                m_FPImpulse.x = it->GetFloat32( 0 );
                m_FPImpulse.y = it->GetFloat32( 1 );
                m_FPImpulse.z = it->GetFloat32( 2 );
            }else if (sName == sm_kapszPropertyNames[ Property_HPImpulse ] )
            {
                m_HPImpulse.x = it->GetFloat32( 0 );
                m_HPImpulse.y = it->GetFloat32( 1 );
                m_HPImpulse.z = it->GetFloat32( 2 );
            }else if (sName == sm_kapszPropertyNames[ Property_FPShift ] )
            {
                m_FPShift.x = it->GetFloat32( 0 );
                m_FPShift.y = it->GetFloat32( 1 );
                m_FPShift.z = it->GetFloat32( 2 );
            }else if (sName == sm_kapszPropertyNames[ Property_HPShift ] )
            {
                m_HPShift.x = it->GetFloat32( 0 );
                m_HPShift.y = it->GetFloat32( 1 );
                m_HPShift.z = it->GetFloat32( 2 );
            }else if (sName == sm_kapszPropertyNames[ Property_FPTimePerParticle ] )
            {
	                m_FPTimePerParticle = it->GetFloat32( 0 );
            }else if (sName == sm_kapszPropertyNames[ Property_HPTimePerParticle ] )
            {
	                m_HPTimePerParticle = it->GetFloat32( 0 );
            }else if (sName == sm_kapszPropertyNames[ Property_FPMaxLifeTime ] )
            {
	                m_FPMaxLifeTime = it->GetFloat32( 0 );
            }else if (sName == sm_kapszPropertyNames[ Property_HPMaxLifeTime ] )
            {
	                m_HPMaxLifeTime = it->GetFloat32( 0 );
            }else if (sName == sm_kapszPropertyNames[ Property_FPMinLifeTime ] )
            {
	                m_FPMinLifeTime = it->GetFloat32( 0 );
            }else if (sName == sm_kapszPropertyNames[ Property_HPMinLifeTime ] )
            {
	                m_HPMinLifeTime = it->GetFloat32( 0 );
            }else if (sName == sm_kapszPropertyNames[ Property_FPMaxSize ] )
            {
	                m_FPMaxSize = it->GetFloat32( 0 );
            }else if (sName == sm_kapszPropertyNames[ Property_HPMaxSize ] )
            {
	                m_HPMaxSize = it->GetFloat32( 0 );
            }else if (sName == sm_kapszPropertyNames[ Property_FPMinSize ] )
            {
	                m_FPMinSize = it->GetFloat32( 0 );
            }else if (sName == sm_kapszPropertyNames[ Property_HPMinSize ] )
            {
	                m_HPMinSize = it->GetFloat32( 0 );
            }else if (sName == sm_kapszPropertyNames[ Property_ShowHP ] )
            {
	                m_bRenderHeatParticles = it->GetBool( 0 );
            }else if (sName == sm_kapszPropertyNames[ Property_OnFire ] )
            {
	                m_bOnFire = it->GetBool( 0 );
            }

    }
    // The following operator is commented out because the fire scene adds each 
    // new fire object to its list itself.
    //reinterpret_cast<FireScene*>(GetSystemScene())->GetFires().push_back(this);
#if FIREOBJ_PREBUILD_VERTICES
    m_nVertexSize = m_bRenderHeatParticles ? sizeof(VertexHP) : sizeof(VertexFP);
#endif

//Fire initialize code goes here
    return Errors::Success;
}

void FireObject::FillPostedData(){
    m_pPostedData->Fires = m_Fires;
}

/// <summary>
///   Implementation of the checkCollision function.
///   fine grained collision check between a particle or ray and a bounding box
///   This function takes two position points and first checks the containment
///   of the points inside the bounding box. 
///   A ray is created with the two points and then the intersection of the ray
///   with the bounding box is tested.  this is done by checking the intersection of
///   each side using the ray plane intersection algorithm.  It can be easily
///   proved that if you do this for 5 sides the sixth has been tested by this time
///   that is why there is only 5 side tests rather than six
/// </summary>

Bool 
FireObject::checkCollision(V3 pos,V3 prevPos, aabb AABB)
{
    if(pos.x <= AABB.xMax && pos.x >= AABB.xMin && 
        pos.y <= AABB.yMax && pos.y >= AABB.yMin && 
        pos.z <= AABB.zMax && pos.z >= AABB.zMin )
    {
        return true;
    }
    V3 RaySegment = pos-prevPos;

    V3 normalP0, normalP1, normalP2, normalP3, normalP4, normalP5;
    V3 PointP0, PointP1, PointP2, PointP3, PointP4, PointP5;

    // Equivalent to: V3(AABB.xMin,AABB.yMin,AABB.zMax) - V3(AABB.xMin,AABB.yMin,AABB.zMin);
	normalP0 = V3(0.0f, 0.0f, AABB.zMax - AABB.zMin);
    //normalP0.Normalize();
    PointP0 = AABB.min; 

    float DotPosNormal;
    float DotPrevNormal;
    V3 intersectedPoint = V3(0.0f,0.0f,0.0f);

    // Equivalent to: Normal = DotProduct(position - Point<n>, normal<n>);
	DotPosNormal = (pos.z - PointP0.z) * normalP0.z;
	DotPrevNormal = (prevPos.z - PointP0.z) * normalP0.z;

    if(DotPosNormal >= 0 && DotPrevNormal <=0 ||
        DotPosNormal <= 0 && DotPrevNormal >=0)
    {
        // Equivalent to: intersectedPoint = prevPos + (((DotProduct(normalP0,PointP0))-(DotProduct(normalP0,prevPos)))*RaySegment)/DotProduct(normalP0,RaySegment);
        intersectedPoint = prevPos + ((PointP0.z - prevPos.z) / RaySegment.z) * RaySegment;
        if (intersectedPoint.x >= AABB.xMin && intersectedPoint.x <= AABB.xMax  && intersectedPoint.y >= AABB.yMin && intersectedPoint.y <= AABB.yMax)
        {
            return true;
        }
    }

    // Equivalent to: normalP1 = V3(AABB.xMax,AABB.yMax,AABB.zMin)-V3(AABB.xMax,AABB.yMax,AABB.zMax);
	normalP1 = V3(0.0f, 0.0f, AABB.zMin - AABB.zMax);
    //normalP1.Normalize();
    PointP1 = AABB.max; 
    
	// Equivalent to: Normal = DotProduct(position - Point<n>, normal<n>);
	DotPosNormal = (pos.z - PointP1.z) * normalP1.z;
	DotPrevNormal = (prevPos.z - PointP1.z) * normalP1.z;

    if(DotPosNormal >= 0 && DotPrevNormal <=0 ||
        DotPosNormal <= 0 && DotPrevNormal >=0)
    {
        // Equivalent to: intersectedPoint = prevPos + (((DotProduct(normalP1,PointP1))-(DotProduct(normalP1,prevPos)))*RaySegment)/DotProduct(normalP1,RaySegment);
        intersectedPoint = prevPos + ((PointP1.z-prevPos.z)/RaySegment.z)*RaySegment;
        if (intersectedPoint.x >= AABB.xMin && intersectedPoint.x <= AABB.xMax  && intersectedPoint.y >= AABB.yMin && intersectedPoint.y <= AABB.yMax)
        {
            return true;
        }
    }

    // Equivalent to: normalP2 = V3(AABB.xMax,AABB.yMax,AABB.zMin) - V3(AABB.xMax,AABB.yMin,AABB.zMin);
	normalP2 = V3(0.0f, AABB.yMax - AABB.yMin, 0.0f);
    //normalP2.Normalize();
    PointP2 = V3(AABB.xMax,AABB.yMin,AABB.zMin);
    
	// Equivalent to: Normal = DotProduct(position - Point<n>, normal<n>);
	DotPosNormal = (pos.y - PointP2.y) * normalP2.y;
	DotPrevNormal = (prevPos.y - PointP2.y) * normalP2.y;

    if(DotPosNormal >= 0 && DotPrevNormal <=0 ||
        DotPosNormal <= 0 && DotPrevNormal >=0)
    {
        // Equivalent to: intersectedPoint = prevPos + (((DotProduct(normalP2,PointP2))-(DotProduct(normalP2,prevPos)))*RaySegment)/DotProduct(normalP2,RaySegment);
        intersectedPoint = prevPos + ((PointP2.y-prevPos.y)/RaySegment.y)*RaySegment;
        if (intersectedPoint.x >= AABB.xMin && intersectedPoint.x <= AABB.xMax  && intersectedPoint.z >= AABB.zMin && intersectedPoint.z <= AABB.zMax)
        {
            return true;
        }
    }

	// Equivalent to: normalP3 = V3(AABB.xMin,AABB.yMin,AABB.zMax) - V3(AABB.xMin, AABB.yMax, AABB.zMax); 
	normalP3 = V3(0.0f,AABB.yMin - AABB.yMax, 0.0f); 
    //normalP3.Normalize();
    PointP3 = V3(AABB.xMin, AABB.yMax, AABB.zMax); 

	// Equivalent to: Normal = DotProduct(position - Point<n>, normal<n>);
	DotPosNormal = (pos.y - PointP3.y) * normalP3.y;
	DotPrevNormal = (prevPos.y - PointP3.y) * normalP3.y;

    if(DotPosNormal >= 0 && DotPrevNormal <=0 ||
        DotPosNormal <= 0 && DotPrevNormal >=0)
    {
        // Equivalent to: intersectedPoint = prevPos + (((DotProduct(normalP3,PointP3))-(DotProduct(normalP3,prevPos)))*RaySegment)/DotProduct(normalP3,RaySegment);
        intersectedPoint = prevPos + ((PointP3.y-prevPos.y)/RaySegment.y)*RaySegment;
        if (intersectedPoint.x >= AABB.xMin && intersectedPoint.x <= AABB.xMax  && intersectedPoint.z >= AABB.zMin && intersectedPoint.z <= AABB.zMax)
        {
            return true;
        }
    }

    // Equivalent to: normalP4 = V3(AABB.xMin,AABB.yMin,AABB.zMax)-V3(AABB.xMax,AABB.yMin,AABB.zMax); 
	normalP4 = V3(AABB.xMin - AABB.xMax, 0.0f, 0.0f);
    //normalP4.Normalize();
    PointP4 = V3(AABB.xMax,AABB.yMin,AABB.zMax); 
    
	// Equivalent to: Normal = DotProduct(position - Point<n>, normal<n>);
	DotPosNormal = (pos.x - PointP4.x) * normalP4.x;
	DotPrevNormal = (prevPos.x - PointP4.x) * normalP4.x;

    if(DotPosNormal >= 0 && DotPrevNormal <=0 ||
        DotPosNormal <= 0 && DotPrevNormal >=0)
    {
        // Equivalent to: intersectedPoint = prevPos + (((DotProduct(normalP4,PointP4))-(DotProduct(normalP4,prevPos)))*RaySegment)/DotProduct(normalP4,RaySegment);
        intersectedPoint = prevPos + ((PointP4.x-prevPos.x)/RaySegment.x)*RaySegment;
        if (intersectedPoint.y >= AABB.yMin && intersectedPoint.y <= AABB.yMax  && intersectedPoint.z >= AABB.zMin && intersectedPoint.z <= AABB.zMax)
        {
            return true;
        }
    }

    //This 6th check is not needed since after you have checked any five the sixth is by default included.  I've left
    //the note here just so people won't think I left out a check.
    //normalP5 = V3(AABB.xMax,AABB.yMax,AABB.zMin) - V3(AABB.xMin,AABB.yMax,AABB.zMin);
    ////normalP5.Normalize();
    //PointP5 = V3(AABB.xMin,AABB.yMax,AABB.zMin);
 
    return false;
}


void
FireObject::GetProperties(
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
    Properties[ iProperty+Property_Technique ].SetValue(0, m_Technique );
	Properties[ iProperty+Property_Emitter ].SetEnumOptions(sm_kapszEmitterTypes);
    Properties[ iProperty+Property_FPDensity ].SetValue( 0, m_FPDensity );

}


void
FireObject::SetProperties(
    Properties::Array Properties
    )
{
    //
    // Read in the properties.
    //
    for ( Properties::Iterator it=Properties.begin(); it != Properties.end(); it++ )
    {
        if ( it->GetFlags() & Properties::Flags::Valid )
        {
            std::string sName = it->GetName();

            if ( sName == sm_kapszPropertyNames[ Property_Technique ] )
            {
                m_Technique= it->GetString(0);
            }
            else if ( sName == sm_kapszPropertyNames[ Property_Emitter ] )
            {
                m_Emitter = it->GetEnumOption( 0 );
            }
            else if ( sName == sm_kapszPropertyNames[ Property_FPDensity ] )
            {
                m_FPDensity = it->GetInt32( 0 );
            }
            else
            {
                ASSERT( False );
            }

            //
            // Set this property to invalid since it's already been read.
            //
            it->ClearFlag( Properties::Flags::Valid );
        }
    }
}

System::Types::BitMask
FireObject::GetDesiredSystemChanges(
    void
    )
{
    return System::Changes::Custom |
        System::Changes::POI::Contact | // new
        System::Changes::Geometry::Position | // new
        System::Changes::Geometry::Orientation | // new
        System::Changes::Geometry::All |
        System::Changes::Graphics::AllMesh;
}

void
FireObject::SetWSBBox()
{
    Math::Vector3 AABBMin;
    Math::Vector3 AABBMax;
        
    AABBMax.x = m_ObjectOSBBox.xMax;
    AABBMax.y = m_ObjectOSBBox.yMax;
    AABBMax.z = m_ObjectOSBBox.zMax;
    AABBMin.x = m_ObjectOSBBox.xMin;
    AABBMin.y = m_ObjectOSBBox.yMin;
    AABBMin.z = m_ObjectOSBBox.zMin;

    AABBMax = m_BBTransform2WS * AABBMax;
    AABBMin = m_BBTransform2WS * AABBMin;


    m_ObjectWSBBox.xMax = AABBMax.x;
    m_ObjectWSBBox.yMax = AABBMax.y;
    m_ObjectWSBBox.zMax = AABBMax.z;
    m_ObjectWSBBox.xMin = AABBMin.x;
    m_ObjectWSBBox.yMin = AABBMin.y;
    m_ObjectWSBBox.zMin = AABBMin.z;
    m_ObjectWSBBox.max = V3(AABBMax.x, AABBMax.y, AABBMax.z);
    m_ObjectWSBBox.min = V3(AABBMin.x, AABBMin.y, AABBMin.z);

    PostChanges( System::Changes::POI::Area );
}
void
FireObject::SetTransformsAsZeroBasedLS()
{
    Math::Matrix4x4 sc;
    m_Transform2WS.Transformation(m_ObjectPosition,m_ObjectOrientation);

    sc = sc.Identity;
    sc.m[0] = m_ObjectScale.x;
    sc.m[5] = m_ObjectScale.y;
    sc.m[10] = m_ObjectScale.z;
    sc.m[15] = 1.0f;

    m_Transform2WS = m_Transform2WS * sc;
    m_BBTransform2WS = m_Transform2WS;

    m_Transform2LS = m_Transform2WS;
    m_Transform2LS = m_Transform2LS.Inverse(m_Transform2LS);
    m_BBTransform2LS = m_Transform2LS;

}
Error
FireObject::ChangeOccurred(
	ISubject* pSubject,
	System::Changes::BitMask ChangeType
	)
{
	if ( ChangeType & (System::Changes::Custom) )
    {
        ISystemObject* pSysObj = dynamic_cast<ISystemObject*>(pSubject);

        if ( pSysObj->GetSystemType() == System::Types::MakeCustom( 0 ) )
        {
            TreeObject* pTreeObj = dynamic_cast<TreeObject*>(pSubject);

            //
            // NOTE: Assuming tree object is only sending custom for initialization.
            //

            void *Params = NULL;
            m_pRetrievedPostedData = 
                reinterpret_cast<pRetrievedPostedData>(pTreeObj->GetObjectPostedVariables(Params));
            createFireSystem();
            m_pPostedData->Fires=m_Fires;

			//PostChanges( System::Changes::Graphics::VertexDecl );
        }
    }
	if ( ChangeType & System::Changes::Geometry::Position )
	{
        {
            IGeometryObject* pGeometryObj = dynamic_cast<IGeometryObject*>(pSubject);
            if( pGeometryObj )
            {
                const Math::Vector3& Position = *pGeometryObj->GetPosition();
                m_ObjectPosition = Position;

                if(m_Type == Type_TreeFire || m_Type == Type_CanopyFire){
                    SetTransformsAsZeroBasedLS();
                    SetWSBBox();
                } else {
                    SetTransformsAsZeroBasedLS();
                    m_Transform2WS = m_Transform2WS.Identity;
                    m_Transform2LS = m_Transform2LS.Identity;
                    SetWSBBox();
                }
                m_CreateParams.position = true;
                if(m_CreateParams.aabb && m_CreateParams.scale && m_CreateParams.orientation && m_CreateParams.position && !m_CreateParams.created) 
                { 
                    createFireSystem();
                }
            }
        }
	}

    if ( ChangeType & System::Changes::Geometry::Orientation )
    {
        {
            const Math::Quaternion& Orientation = *dynamic_cast<IGeometryObject*>(pSubject)->GetOrientation();
            m_ObjectOrientation = Orientation;

            if(m_Type == Type_TreeFire || m_Type == Type_CanopyFire){
                SetTransformsAsZeroBasedLS();
                SetWSBBox();
            } else {
                SetTransformsAsZeroBasedLS();
                m_Transform2WS = m_Transform2WS.Identity;
                m_Transform2LS = m_Transform2LS.Identity;
                SetWSBBox();
            }

            m_CreateParams.orientation = true;
            if(m_CreateParams.aabb && m_CreateParams.scale && m_CreateParams.orientation && m_CreateParams.position && !m_CreateParams.created) 
            { 
                createFireSystem();
            }
        }
    }

	if ( ChangeType & System::Changes::Geometry::Scale )
	{
        {
            const Math::Vector3& Scale = *dynamic_cast<IGeometryObject*>(pSubject)->GetScale();
            m_ObjectScale = Scale;

            if(m_Type == Type_TreeFire || m_Type == Type_CanopyFire){
                SetTransformsAsZeroBasedLS();
                SetWSBBox();
            } else {
                SetTransformsAsZeroBasedLS();
                m_Transform2WS = m_Transform2WS.Identity;
                m_Transform2LS = m_Transform2LS.Identity;
                SetWSBBox();
            }

            m_CreateParams.scale = true;
            if(m_CreateParams.aabb && m_CreateParams.scale && m_CreateParams.orientation && m_CreateParams.position && !m_CreateParams.created) 
            { 
                createFireSystem();
            }
        }
	}

    if ( ChangeType & System::Changes::Graphics::AABB )
    {
        {
            IGraphicsObject* pGfxObj = dynamic_cast<IGraphicsObject*>(pSubject);

            if(pGfxObj){
            Math::Vector3 AABBMin;
            Math::Vector3 AABBMax;
            pGfxObj->GetAABB( AABBMin, AABBMax );
            m_ObjectOSBBox.xMax = AABBMax.x;
            m_ObjectOSBBox.yMax = AABBMax.y;
            m_ObjectOSBBox.zMax = AABBMax.z;
            m_ObjectOSBBox.xMin = AABBMin.x;
            m_ObjectOSBBox.yMin = AABBMin.y;
            m_ObjectOSBBox.zMin = AABBMin.z;
            m_ObjectOSBBox.max = V3(AABBMax.x, AABBMax.y, AABBMax.z);
            m_ObjectOSBBox.min = V3(AABBMin.x, AABBMin.y, AABBMin.z);



            m_CreateParams.aabb = true;
            if(m_CreateParams.aabb && m_CreateParams.scale&& m_CreateParams.orientation && m_CreateParams.position && !m_CreateParams.created) 
            { 
                createFireSystem();
            }
            }
        }
    }
    return Errors::Success;
}

void
FireObject::createFireSystem()
{
    if(!m_CreateParams.created)
    {
        if (m_Type == Type_CanopyFire){
            B3 Basis;
            aabb psysBox;
            std::vector<FireObject::PointPair *>::iterator i, iend;
            i = m_pRetrievedPostedData->pointPairs.begin();

            iend = m_pRetrievedPostedData->pointPairs.end();
            // restrict to only one pair for debugging
            //iend = m_pRetrievedPostedData->pointPairs.begin();
            //iend ++;
            //end restrict to only one pair for debugging
            V3 impulse(m_FPImpulse.x, m_FPImpulse.y,m_FPImpulse.z);
            V3 shift(m_FPShift.x, m_FPShift.y,m_FPShift.z);
            for ( i; i != iend; i++){
                psysBox = (*i)->AABB;
                Basis = (*i)->basis;
                //Basis.iAxis = Basis.iAxis ;//* m_ObjectScale.x
                //Basis.jAxis = Basis.jAxis ;//* m_ObjectScale.y
                //Basis.kAxis = Basis.kAxis ;//* m_ObjectScale.z

                ParticleEmitter::FirePatch * fr = new ParticleEmitter::FirePatch(
                    impulse, 
                    shift, 
                    Basis,
                    (*i)->basePoint,
                    psysBox,
                    m_FPDensity,
                    m_FPTimePerParticle,
                    m_FPMinLifeTime,
                    m_FPMaxLifeTime,
                    m_FPMinSize,
                    m_FPMaxSize);            
                m_Fire = dynamic_cast<ParticleEmitter::ParticleSystem*>(fr);
                m_Fires.push_back(m_Fire);
                m_BurningList.push_back((*i)->burning ? Burning : Normal );
                //m_BurningList.push_back(true);
                m_BoundingBoxList.push_back((*i)->AABB);
            }
            m_pPostedData->Fires=m_Fires;
            m_CreateParams.created = true;
        }
        if (m_Type == Type_TreeFire){

            aabb psysBox;
            std::vector<FireObject::PointPair *>::iterator i, iend;
            i = m_pRetrievedPostedData->pointPairs.begin();

            iend = m_pRetrievedPostedData->pointPairs.end();
            // restrict to only one pair for debugging
            //iend = m_pRetrievedPostedData->pointPairs.begin();
            //iend ++;
            //end restrict to only one pair for debugging
            int Trunk = 0;
            V3 impulse(m_FPImpulse.x, m_FPImpulse.y,m_FPImpulse.z);
            V3 shift(m_FPShift.x, m_FPShift.y,m_FPShift.z);
            for ( i; i != iend; i++){
                psysBox = (*i)->AABB;
                ParticleEmitter::Fire * fr = new ParticleEmitter::Fire(
                    impulse,
                    shift, 
                    (*i)->extendPoint, 
                    (*i)->basePoint,
                    psysBox, 
                    m_FPDensity, 
                    m_FPTimePerParticle,
                    m_FPMinLifeTime,
                    m_FPMaxLifeTime,
                    m_FPMinSize,
                    m_FPMaxSize);
                m_Fire = dynamic_cast<ParticleEmitter::ParticleSystem*>(fr);
                m_Fires.push_back(m_Fire);
                if(!Trunk && m_bOnFire){
                    m_BurningList.push_back(Burning);
                }else{
                    m_BurningList.push_back((*i)->burning ? Burning : Normal );
                }
                Trunk++;
                m_BoundingBoxList.push_back((*i)->AABB);
            }
            m_CreateParams.created = true;
        }
        if (m_Type == Type_SphereFire){
            aabb psysBox;
            V3 Radius;
            Math::Vector3 radius;
            f32 average = 0.5f * (m_ObjectOSBBox.xMax - m_ObjectOSBBox.xMin);
            radius.x = 0.0f;
            radius.y = average * m_ObjectScale.x;
            radius.z = 0.0f;
            radius += m_ObjectPosition;
            Radius.x = radius.x;
            Radius.y = radius.y;
            Radius.z = radius.z;
            V3 Position;
            Position.x = m_ObjectPosition.x;
            Position.y = m_ObjectPosition.y;
            Position.z = m_ObjectPosition.z;

            V3 impulse(m_FPImpulse.x, m_FPImpulse.y,m_FPImpulse.z);
            V3 shift(m_FPShift.x, m_FPShift.y,m_FPShift.z);
            ParticleEmitter::FireBall * fr = new ParticleEmitter::FireBall(
                impulse, 
                shift, 
                Radius,
                Position,
                psysBox,
                m_FPDensity,
                m_FPTimePerParticle,
                m_FPMinLifeTime,
                m_FPMaxLifeTime,
                m_FPMinSize,
                m_FPMaxSize);
            m_Fire = dynamic_cast<ParticleEmitter::ParticleSystem*>(fr);
            m_Fires.push_back(m_Fire);
            if(m_bOnFire){
                m_BurningList.push_back(Burning);
            }else{
                m_BurningList.push_back(Normal);
            }
            m_BoundingBoxList.push_back(m_ObjectWSBBox);
            m_CreateParams.created = true;
            m_pPostedData->Fires=m_Fires;

            m_CreateParams.created = true;
        }
        if (m_Type == Type_PatchFire){
            aabb psysBox;
            B3 Basis;
            Math::Vector3 radius;
            f32 average = 0.5f * (m_ObjectOSBBox.xMax - m_ObjectOSBBox.xMin);
            radius.x = 0.0f;
            radius.y = average * m_ObjectScale.x;
            radius.z = 0.0f;
            radius += m_ObjectPosition;
            Basis.jAxis.x = radius.x;
            Basis.jAxis.y = radius.y;
            Basis.jAxis.z = radius.z;
            Basis.iAxis.x = radius.y;
            Basis.iAxis.y = radius.x;
            Basis.iAxis.z = radius.z;
            Basis.kAxis.x = radius.x;
            Basis.kAxis.y = radius.z;
            Basis.kAxis.z = radius.y;
            V3 Position;
            Position.x = m_ObjectPosition.x;
            Position.y = m_ObjectPosition.y;
            Position.z = m_ObjectPosition.z;

            V3 impulse(m_FPImpulse.x, m_FPImpulse.y,m_FPImpulse.z);
            V3 shift(m_FPShift.x, m_FPShift.y,m_FPShift.z);
            ParticleEmitter::FirePatch * fr = new ParticleEmitter::FirePatch(
                impulse, 
                shift, 
                Basis,
                Position,
                psysBox,
                m_FPDensity,
                m_FPTimePerParticle,
                m_FPMinLifeTime,
                m_FPMaxLifeTime,
                m_FPMinSize,
                m_FPMaxSize);
            m_Fire = dynamic_cast<ParticleEmitter::ParticleSystem*>(fr);
            m_Fires.push_back(m_Fire);
            m_BurningList.push_back(Burning);
            m_BoundingBoxList.push_back(m_ObjectWSBBox);
            m_CreateParams.created = true;
            m_pPostedData->Fires=m_Fires;
            m_CreateParams.created = true;
        }
        if (m_Type == Type_ColdParticle)
        {
            aabb psysBox;
            V3 Radius;
            Math::Vector3 radius;
            f32 average = 0.5f * (m_ObjectOSBBox.xMax - m_ObjectOSBBox.xMin);
            radius.x = 0.0f;
            radius.y = average * m_ObjectScale.x;
            radius.z = 0.0f;
            radius += m_ObjectPosition;
            Radius.x = radius.x;
            Radius.y = radius.y;
            Radius.z = radius.z;
            V3 Position;
            Position.x = m_ObjectPosition.x;
            Position.y = m_ObjectPosition.y;
            Position.z = m_ObjectPosition.z;

            V3 impulse(m_FPImpulse.x, m_FPImpulse.y,m_FPImpulse.z);
            V3 shift(m_FPShift.x, m_FPShift.y,m_FPShift.z);
            ParticleEmitter::ColdParticle * fr = new ParticleEmitter::ColdParticle(
                impulse, 
                shift, 
                Radius,
                Position,
                psysBox,
                m_FPDensity,
                m_FPTimePerParticle,
                m_FPMinLifeTime,
                m_FPMaxLifeTime,
                m_FPMinSize,
                m_FPMaxSize);
            m_Fire = dynamic_cast<ParticleEmitter::ParticleSystem*>(fr);
            m_Fires.push_back(m_Fire);
            if(m_bOnFire){
                m_BurningList.push_back(Burning);
            }else{
                m_BurningList.push_back(Normal);
            }
            m_BoundingBoxList.push_back(m_ObjectWSBBox);
            m_CreateParams.created = true;
            m_pPostedData->Fires=m_Fires;

            m_CreateParams.created = true;
        }

        if(m_CreateParams.created)
        {
            if (m_Type == Type_ColdParticle )
            {
                ; // Intentionally empty - water particles are drawn with Ogre's particle system
            }
            else
            {
                //Debug::Print("FireObject: PostChanges: Graphics::VertexDecl\n");
                PostChanges( System::Changes::Graphics::VertexDecl );
            }
        }
    }
}

///<summary cref="FireObject::update">
/// local function available for Task update to call
/// so object can deal with local data and do any posting
/// of new notifications
/// With this update function the goal is to iterate though all the associated FireObjects that
/// are associated with the fire system.  we need to discover which fires
/// contained in each fireObject are burning or not.  We need to establish
/// if the heat particles associated with this Fire Object are intersecting with
/// any non burning fires contained within this fire object and any other fire objects
/// we then do a collision detection between those heat particles and the non burning fires.
/// The fires that there are collisions detected for will be activated and become burning fires
/// rather than inactive fires
///</summary> 
void FireObject::update(f32 fDeltaTime, const FireObjectList& fireObjectList)
{
    m_fDeltaTime = fDeltaTime;
    m_pFireObjectList = &fireObjectList;

    u32 size = (u32)m_Fires.size();
    if (
		( g_FireManagers.pTask != NULL ) &&
		( FireObjectUpdateGrainSize < size )
	)
    {
        // This is nested parallel job, so we do not identify it (so far)
        g_FireManagers.pTask->ParallelFor( /*GetSystemScene()->GetSystemTask()*/
                                       NULL, UpdateCallback, this, 0, size, FireObjectUpdateGrainSize );
    }
    else
    {
        UpdateRange( 0, size );
    }
    m_bDirty = True;
    m_CurrentTime = fDeltaTime;

    // Posted data are never used so do not vaste time to update them
    //FillPostedData();  

#if FIREOBJ_PREBUILD_VERTICES
    u32 vertexCount = GetVertexCount(0);
    u32 newCapacity = vertexCount * m_nVertexSize;
    m_nVertexBufferSize = vertexCount * m_nVertexSize;

    if  ( newCapacity > m_nVertexBufferCapacity )
    {
//        delete m_pVertices;
//        m_pVertices = new char[newCapacity];
        m_pVertexBuffer = scalable_realloc( m_pVertexBuffer, newCapacity );
        m_nVertexBufferCapacity = newCapacity;
    }
    BuildVertexBuffer( m_pVertexBuffer );
#endif /* FIREOBJ_PREBUILD_VERTICES */

    if (m_Type == Type_ColdParticle )
    {
        ; // Intentionally empty - water particles are drawn with Ogre's particle system
    }
    else
    {
        PostChanges( System::Changes::Graphics::VertexBuffer | System::Changes::Graphics::AABB);
    }
}

void FireObject::UpdateCallback( void *param, u32 begin, u32 end )
{
    FireObject *pThis = static_cast<FireObject*>(param);

    pThis->UpdateRange( begin, end );
}

void FireObject::UpdateRange ( u32 begin, u32 end )
{
    if( m_Type == Type_TreeFire )
    {
	    for ( u32 i = begin; i < end; ++i )
        {
            ASSERT ( dynamic_cast<ParticleEmitter::Fire*>(m_Fires[i]) );
            ParticleEmitter::Fire *fr = static_cast<ParticleEmitter::Fire*>(m_Fires[i]);
                
	        fr->update(m_fDeltaTime);
            //if(fr->isActive())
            if(m_BurningList[i] != Burning)
            {
                fr->setInactive();
                fr->reinitialize();
            }else{
               fr->setActive();
               if ( !fr->m_pHeatParticles->isActive())
               {
                    V3 impulse(m_HPImpulse.x, m_HPImpulse.y,m_HPImpulse.z);
                    V3 shift(m_HPShift.x, m_HPShift.y,m_HPShift.z);
                    fr->m_pHeatParticles->Activate(
                        fr->mInitPos + ((fr->mParticleLine - fr->mInitPos) * 0.5f),
                        impulse,
                        shift,
                        m_HPDensity,
                        m_HPTimePerParticle,
                        m_HPMinLifeTime,
                        m_HPMaxLifeTime);
                    //fr->m_pHeatParticles->setActive();  //needs to be updated once before set to active.
               }else{ // Could reset the position of the heat particle if it is active;
                      // and position of the particle engine shifted.
                      // possibly set other parameters such as impulse and shift
                      // It would also be OK to set Activate again with all the parameters
               }
               fr->m_pHeatParticles->update(m_fDeltaTime);
               fr->m_pHeatParticles->setActive();
            }

            EmitterCollisionCheck(*m_pFireObjectList, fr->m_pHeatParticles);
	    }
    }
    if( m_Type == Type_SphereFire )
    {
	    for ( u32 i = begin; i < end; ++i )
        {
            ASSERT ( dynamic_cast<ParticleEmitter::FireBall*>(m_Fires[i]) );
            ParticleEmitter::FireBall *fr = static_cast<ParticleEmitter::FireBall*>(m_Fires[i]);

            fr->update(m_fDeltaTime);
            if(m_BurningList[i] != Burning)
            {
                fr->setInactive();
                fr->reinitialize();
            }else{
               fr->setActive();
               if ( !fr->m_pHeatParticles->isActive())
               {
                    // activate with parameter 3 changed from 1 to 2 will half the Frame rate on some scenes.  
                    V3 impulse(m_HPImpulse.x, m_HPImpulse.y,m_HPImpulse.z);
                    V3 shift(m_HPShift.x, m_HPShift.y,m_HPShift.z);
                    fr->m_pHeatParticles->Activate(
                        fr->mInitPos ,
                        impulse,
                        shift,
                        m_HPDensity,
                        m_HPTimePerParticle,
                        m_HPMinLifeTime,
                        m_HPMaxLifeTime);
                    //fr->m_pHeatParticles->setActive();//testing 1..2
                    
               }else{ // just set the position of the heat particle if it is active;
                      // possibly set other parameters such as impulse and shift
                      // It would also be OK to set Activate again with all the parameters

                   fr->m_pHeatParticles->setPos(fr->mInitPos);
               }
               V3 Position;
               Position.x = m_ObjectPosition.x;
               Position.y = m_ObjectPosition.y;
               Position.z = m_ObjectPosition.z;
               fr->setInitPos(Position);
                V3 Radius;
                Math::Vector3 radius;
                f32 average = 0.5f * (m_ObjectOSBBox.xMax - m_ObjectOSBBox.xMin);
                radius.x = 0.0f;
                radius.y = average * m_ObjectScale.x;
                radius.z = 0.0f;
                radius += m_ObjectPosition;
                Radius.x = radius.x;
                Radius.y = radius.y;
                Radius.z = radius.z;
                fr->setRadius(Radius);

               fr->m_pHeatParticles->update(m_fDeltaTime);
               fr->m_pHeatParticles->setActive();
            }
            EmitterCollisionCheck(*m_pFireObjectList, fr->m_pHeatParticles);
	    }
     }
    if (m_Type == Type_ColdParticle )
    {
	    for ( u32 i = begin; i < end; ++i )
        {
            ASSERT ( dynamic_cast<ParticleEmitter::ColdParticle*>(m_Fires[i]) );
            ParticleEmitter::ColdParticle *fr = static_cast<ParticleEmitter::ColdParticle*>(m_Fires[i]);

            fr->update(m_fDeltaTime);
            if(m_BurningList[i] != Burning)
            {
                fr->setInactive();
                fr->reinitialize();
            }else{
                fr->setActive();
                if ( !fr->m_pColdParticles->isActive())
                {
                    // activate with parameter 3 changed from 1 to 2 will half the Frame rate on some scenes.  
                    V3 impulse(m_HPImpulse.x, m_HPImpulse.y,m_HPImpulse.z);
                    V3 shift(m_HPShift.x, m_HPShift.y,m_HPShift.z);
                    fr->m_pColdParticles->Activate(
                        fr->mInitPos ,
                        impulse,
                        shift,
                        m_HPDensity,
                        m_HPTimePerParticle,
                        m_HPMinLifeTime,
                        m_HPMaxLifeTime);
                    //fr->m_pHeatParticles->setActive();//testing 1..2

                }else{ // just set the position of the heat particle if it is active;
                    // possibly set other parameters such as impulse and shift
                    // It would also be OK to set Activate again with all the parameters

                    fr->m_pColdParticles->setPos(fr->mInitPos);
                }

                V3 Position;
                Position.x = m_ObjectPosition.x;
                Position.y = m_ObjectPosition.y;
                Position.z = m_ObjectPosition.z;


                fr->setInitPos(Position);
                V3 Radius;
                Math::Vector3 radius;
                f32 average = 0.5f * (m_ObjectOSBBox.xMax - m_ObjectOSBBox.xMin);
                radius.x = 0.0f;
                radius.y = average * m_ObjectScale.x;
                radius.z = 0.0f;
                radius += m_ObjectPosition;
                Radius.x = radius.x;
                Radius.y = radius.y;
                Radius.z = radius.z;
                fr->setRadius(Radius);

                fr->m_pColdParticles->update(m_fDeltaTime);
                fr->m_pColdParticles->setActive();
            }
            EmitterCollisionCheck(*m_pFireObjectList, fr->m_pColdParticles);
        }
    }
    if( m_Type == Type_PatchFire || m_Type == Type_CanopyFire )
    {
	    for ( u32 i = begin; i < end; ++i )
        {
            ASSERT ( dynamic_cast<ParticleEmitter::FirePatch*>(m_Fires[i]) );
            ParticleEmitter::FirePatch *fr = static_cast<ParticleEmitter::FirePatch*>(m_Fires[i]);

	        fr->update(m_fDeltaTime);
            if(m_BurningList[i] != Burning)
            {
                fr->setInactive();
                fr->reinitialize();
            }else{
               fr->setActive();
               if ( !fr->m_pHeatParticles->isActive())
               {
                    // activate with parameter 3 changed from 1 to 2 will half the Frame rate on some scenes.  
                    V3 impulse(m_HPImpulse.x, m_HPImpulse.y,m_HPImpulse.z);
                    V3 shift(m_HPShift.x, m_HPShift.y,m_HPShift.z);
                    fr->m_pHeatParticles->Activate(
                        fr->mInitPos ,
                        impulse,
                        shift,
                        m_HPDensity,
                        m_HPTimePerParticle,
                        m_HPMinLifeTime,
                        m_HPMaxLifeTime);
                    //fr->m_pHeatParticles->setActive();//testing 1..2
                    
               }else{ // just set the position of the heat particle if it is active;
                      // possibly set other parameters such as impulse and shift
                      // It would also be OK to set Activate again with all the parameters

                   fr->m_pHeatParticles->setPos(fr->mInitPos);
               }
               if(m_Type == Type_PatchFire)
               {
                   V3 Position;
                   Position.x = m_ObjectPosition.x;
                   Position.y = m_ObjectPosition.y;
                   Position.z = m_ObjectPosition.z;
                   fr->setInitPos(Position);
               }
                //V3 Radius;
                //Math::Vector3 radius;
                //u32 average = 0.5f * (m_ObjectOSBBox.xMax - m_ObjectOSBBox.xMin);
                //radius.x = 0.0f;
                //radius.y = average * m_ObjectScale.x;
                //radius.z = 0.0f;
                //radius += m_ObjectPosition;
                //Radius.x = radius.x;
                //Radius.y = radius.y;
                //Radius.z = radius.z;
                //fr->setRadius(Radius);

               fr->m_pHeatParticles->update(m_fDeltaTime);
               fr->m_pHeatParticles->setActive();
            }
            EmitterCollisionCheck(*m_pFireObjectList, fr->m_pHeatParticles);
	    }
     }
} // FireObject::updateRange


Bool
FireObject::StepUpdate(
    f32 fDeltaTime
    )
{
    UNREFERENCED_PARAM(fDeltaTime);
    return false;
}
void 
FireObject::Vector4_2_V3(Math::Vector4 &v4, V3 &v3)
{
    v3.x = v4.x;
    v3.y = v4.y;
    v3.z = v4.z;
}
void 
FireObject::V3_2_Vector4(V3 &v3, Math::Vector4 &v4)
{
    v4.x = v3.x;
    v4.y = v3.y;
    v4.z = v3.z;
    v4.w = 1.0f;
    
}

ParticleEmitter::HeatEmitter* FireObject::GetEmitter()
{
    return NULL;
}


struct CollisionCheckCallbackData : public FireObject::CollisionCheckInfo
{
    CollisionCheckCallbackData ( FireObject *pThis, FireObject::Emitter_Type et )
        : FireObject::CollisionCheckInfo(et)
        , m_pThis(pThis)
    {}

    FireObject             *m_pThis;
};


void FireObject::EmitterCollisionCheck( const FireObjectList& fireObjectList, ParticleEmitter::HeatEmitter *hm )
{
    ASSERT( m_pFireObjectList == &fireObjectList );
    UNREFERENCED_PARAM( fireObjectList );
    // Check is the heat particle active or does the heat emitter exist, 
    // and is the heat emitter active (active only if associated mesh is on fire)
    if( hm && hm->isActive() )
    {
        // Cannot store emitter data in this FireObject object, since EmitterCollisionCheck
        // can be called concurrently for the same FireObject instance for different fires
        CollisionCheckCallbackData  cccd( this, hm->m_EmitterType );

        // Set up the two point rays for all of the heat emitters associated with this meshes fire
        Bool ValidHM = hm->getPositions( cccd.m_vPositions, cccd.m_vPrevPositions );
        UNREFERENCED_PARAM(ValidHM);

        u32 size = (u32)cccd.m_vPositions.size();

        // Iterate through all of the heat particle (rays) for "this" fire object.
        if (
			( g_FireManagers.pTask != NULL ) &&
			( CollisionCheckGrainSize < size )
		)
        {
            // This is nested parallel job, so we do not identify it (so far)
            g_FireManagers.pTask->ParallelFor( NULL, FireCollisionCallback, &cccd, 0, size, CollisionCheckGrainSize );
        }
        else
        {
            ProcessFireCollisionsRange( 0, size, &cccd );
        }
    }
}

void FireObject::FireCollisionCallback( void *param, u32 begin, u32 end )
{
    CollisionCheckCallbackData  *pcccd = static_cast<CollisionCheckCallbackData*>(param);

    pcccd->m_pThis->ProcessFireCollisionsRange( begin, end, pcccd );
}


void FireObject::ProcessFireCollisionsRange ( u32 begin, u32 end, CollisionCheckInfo* pcci )
{
    for( u32 i = begin; i < end; ++i )
    {
#if NO_MUTUAL_FIRE_COLLISION_CHECKS
        u32 rangeEnd;
        u32 rangeBegin;
    #if MUTUAL_CHECK_TWEAK
        if ( m_bNextIsReverse )
        {
            rangeBegin = 0;
            rangeEnd = m_nIndex;
            m_bNextIsReverse = false;
        }
        else
    #endif /* MUTUAL_CHECK_TWEAK */
        {
            rangeEnd = (u32)m_pFireObjectList->size();
            rangeBegin = m_nIndex;
            m_bNextIsReverse = true;
        }
#else /* !NO_MUTUAL_FIRE_COLLISION_CHECKS */
        u32 rangeEnd = (u32)m_pFireObjectList->size();
        u32 rangeBegin = 0;
#endif /* !NO_MUTUAL_FIRE_COLLISION_CHECKS */

        //for each Fire object in the List of all FireObjects test for collisions with "this" Fire Objects heat particle (rays);
        Emitter_Type    emitterType = pcci->m_EmitterType;
        std::vector<V3> &positions = pcci->m_vPositions;
        std::vector<V3> &prevPositions = pcci->m_vPrevPositions;

	    // Transform the particle into local space of current Fire object in the Fire Object List (FOL)
        // This is done by first transforming the particle to world space using the "this" instance of the
        // WS transform.  Then we transform the particle to the local space of the Fire Object int the (FOL)
        // using the FOLi instance of the Local Space transform.  This puts the heat particle (ray) of "this" fire object 
        // in the Local space of the Fire Object (FOLi).  The particle or Ray can then be tested for collision
        // First we need to convert both the begin and end point of the heat ray between the 
        // V3 class and Math::Vector class... we need to transform to V3 and then back to Math::Vector
        V3 testParticle = positions[i];
        Math::Vector4 tempTestParticle;
        tempTestParticle.x = testParticle.x;
        tempTestParticle.y = testParticle.y;
        tempTestParticle.z = testParticle.z;
        tempTestParticle.w = 1.0f;

		// transform end point particle or (ray) to World Space
		tempTestParticle = m_Transform2WS * tempTestParticle;

        V3 testPrevParticle = prevPositions[i];
        Math::Vector4 tempTestPrevParticle;
        tempTestPrevParticle.x = testPrevParticle.x;
        tempTestPrevParticle.y = testPrevParticle.y;
        tempTestPrevParticle.z = testPrevParticle.z;
        tempTestPrevParticle.w = 1.0f;
        
        // transform begin point of particle ray to World Space
        tempTestPrevParticle = m_Transform2WS * tempTestPrevParticle;

		// transform bounding box of Fire Object from List (FOL) to
		// world space. This will set up a collision detection with the 
		// bounding box to reduce tests for fire object
		// need to convert from aabb class to Math::Vector do the transform and then 
		// convert back to aabb class.
		V3 ttp,ttpp;
		ttp.x = tempTestParticle.x;
		ttp.y = tempTestParticle.y;
		ttp.z = tempTestParticle.z;

		ttpp.x = tempTestPrevParticle.x;
		ttpp.y = tempTestPrevParticle.y;
		ttpp.z = tempTestPrevParticle.z;

        for( u32 j = rangeBegin; j < rangeEnd; ++j )
        {
            FireObject  *pfo = m_pFireObjectList->at(j);

            // While the Particle and the bounding box are in WS do a quick check of a collision 
            // between the bounding box and the heat particle or ray
            if( checkCollision(ttp, ttpp, pfo->m_ObjectWSBBox) )
            {
                // if it passes this test then we want to finish setting the heat particle (ray) of
                // "this" fire object to the Local Space of the FOLi Fire Object instance
                Math::Vector4 tmpTestParticle = pfo->m_Transform2LS * tempTestParticle;
				V3 testPart = V3( tmpTestParticle.x, tmpTestParticle.y, tmpTestParticle.z );

				Math::Vector4 tmpTestPrevParticle = pfo->m_Transform2LS * tempTestPrevParticle;
                V3 testPrevPart = V3( tmpTestPrevParticle.x, tmpTestPrevParticle.y, tmpTestPrevParticle.z );

                u32 size = (u32)pfo->m_BurningList.size();
                // need to change name from branches to some local burning list
                // for a sphere or plank on a house there would be just one item in the burning list
                // conceivably the list could be bigger for aggregated objects other than trees
                if( emitterType == ParticleEmitter::HeatEmitter::Type_Water )
                {
                    for( u32 branches = 0; branches < size; ++branches )
                    {
                        if (pfo->m_BurningList[branches] == Burning)
                        {
                            if (pfo->m_pRetrievedPostedData)
                            {
                                if (checkCollision(testPart, testPrevPart, pfo->m_pRetrievedPostedData->pointPairs[branches]->AABB))
                                {
                                    pfo->m_BurningList[branches] = Extinguished;
                                }
                            }
                        }
                    }
                }
                else
                {
                    for( u32 branches = 0; branches < size; ++branches )
                    {
                        if (pfo->m_BurningList[branches] == Normal)
                        {
                            if (checkCollision(testPart, testPrevPart, pfo->m_pRetrievedPostedData->pointPairs[branches]->AABB))
                            {
                                // Post a change to tell other system that the fire is active
                                if( !pfo->m_fireIsActive)
                                {
                                    pfo->m_fireIsActive = true;
                                    pfo->PostChanges( System::Changes::POI::Area );
                                }

                                // Mark this as burning
                                pfo->m_BurningList[branches] = Burning;
                            }
                        }
                    }
                }
            } // end of "collision between the bounding box and the heat particle or ray detected"
        }
    }
}



System::Changes::BitMask
FireObject::GetPotentialSystemChanges(
    void
    )
{
    return System::Changes::Custom |System::Changes::Graphics::AllMesh | System::Changes::POI::Area;
}


void
FireObject::GetVertexDeclaration(
    Out VertexDecl::Element* pVertexDecl,
	In  u16 nSubMeshIndex
    )
{
    UNREFERENCED_PARAM(nSubMeshIndex);
    ASSERT( 0 == nSubMeshIndex );

    if(m_bRenderHeatParticles){

        static const u32 Size = sizeof (VertexDecl::Element) * NUM_VERTEX_HP_DECL_ELEMENTS;
        static const VertexDecl::Element Decl[ NUM_VERTEX_HP_DECL_ELEMENTS ] =
        {
            { VertexDecl::Type ::Float3, VertexDecl::Usage::Position, 0 },
        };

        ::memcpy_s( pVertexDecl, Size, Decl, Size );
    }else{

        static const u32 Size = sizeof (VertexDecl::Element) * NUM_VERTEX_FP_DECL_ELEMENTS;
        static const VertexDecl::Element Decl[ NUM_VERTEX_FP_DECL_ELEMENTS ] =
        {
            { VertexDecl::Type ::Float3, VertexDecl::Usage::Position, 0 },
		    { VertexDecl::Type ::Float3, VertexDecl::Usage::Texture, 0 },
            { VertexDecl::Type ::Float1, VertexDecl::Usage::Texture, 0 },
            { VertexDecl::Type ::Float1, VertexDecl::Usage::Texture, 0 },
            { VertexDecl::Type ::Float1, VertexDecl::Usage::Texture, 0 },
            { VertexDecl::Type ::Float1, VertexDecl::Usage::Texture, 0 },
            { VertexDecl::Type ::Float1, VertexDecl::Usage::Texture, 0 },
		    { VertexDecl::Type ::Float1, VertexDecl::Usage::Diffuse, 0 },
        };

        ::memcpy_s( pVertexDecl, Size, Decl, Size );
        
    }
}


u32
FireObject::GetSubMeshCount(
    void
    )
{
    // We use only one SubMesh
    return 1;
}


u32
FireObject::GetIndexDeclaration(
    In  u16 nSubMeshIndex
    )
{
	UNREFERENCED_PARAM( nSubMeshIndex );
    ASSERTMSG( False, "Not implemented here." );
    return 0;
}


u32
FireObject::GetIndexCount(
    In  u16 nSubMeshIndex
    )
{
	UNREFERENCED_PARAM( nSubMeshIndex );
    ASSERTMSG( False, "Not implemented here." );
    return 0;
}


void
FireObject::GetIndices(
    Out void* pIndices,
    In  u16 nSubMeshIndex
    )
{
    UNREFERENCED_PARAM( pIndices );
	UNREFERENCED_PARAM( nSubMeshIndex );
    ASSERTMSG( False, "Not implemented here." );
}


u32
FireObject::GetVertexCount(
    In  u16 nSubMeshIndex
    )
{
    UNREFERENCED_PARAM(nSubMeshIndex);
    ASSERT( 0 == nSubMeshIndex );

    if ( !m_bDirty )
        return m_ParticleVertexCount;

    m_bDirty = False;
	m_ParticleVertexCount = 0;

    ASSERT( !m_Fires.empty() );

    if(m_bRenderHeatParticles)
    {
        size_t size = m_Fires.size();
        for( size_t i = 0; i < size; ++i )
        {
            if( m_Fires[i]->isActive() && m_BurningList[i] == Burning )
            {
                ParticleEmitter::ParticleSystemWithEmitter *pswe = 
                        static_cast<ParticleEmitter::ParticleSystemWithEmitter*>(m_Fires[i]);
                m_ParticleVertexCount += pswe->GetEmitter()->getAliveParticleCount();
            }

        }
    }
    else
    {
        std::vector<ParticleEmitter::Particle> particles;
        size_t size = m_Fires.size();
        for( size_t i = 0; i < size; ++i )
        {
            if( m_Fires[i]->isActive() && m_BurningList[i] == Burning )
            {
                m_ParticleVertexCount += m_Fires[i]->getAliveParticleCount();
            }
        }
    }

    return m_ParticleVertexCount;
}

u32
FireObject::GetVertexDeclarationCount(
    In  u16 nSubMeshIndex
    )
{
    UNREFERENCED_PARAM(nSubMeshIndex);
    ASSERT( 0 == nSubMeshIndex );

    if(m_bRenderHeatParticles){
        return NUM_VERTEX_HP_DECL_ELEMENTS;    
    }else{
        return NUM_VERTEX_FP_DECL_ELEMENTS;
    }

}


#if FIREOBJ_PARALLEL_BUILD_VERTICES

void
FireObject::BuildVertexBuffer( void* pVertices )
{
    if ( m_bDirty )
        GetVertexCount(0);

    m_pVertices = pVertices;

    u32 size = (u32)m_Fires.size();
  
    m_newMinExtends = Math::Vector3(FLT_MAX);
    m_newMaxExtends = Math::Vector3(-FLT_MAX);

    if (
		( g_FireManagers.pTask != NULL ) &&
		( BuildVerticesGrainSize < size )
	)
    {
        // This is nested parallel job, so we do not identify it (so far)
        g_FireManagers.pTask->ParallelFor( NULL, BuildVerticesCallback, this, 0, size, BuildVerticesGrainSize );
    }
    else
    {
        BuildVerticesRange( 0, size );
    }

    if( (m_newMinExtends.x != FLT_MAX) &&
        (m_newMinExtends.y != FLT_MAX) &&
        (m_newMinExtends.z != FLT_MAX) &&
        (m_newMaxExtends.x != FLT_MIN) &&
        (m_newMaxExtends.y != FLT_MIN) &&
        (m_newMaxExtends.z != FLT_MIN) )
    {
        m_minExtends = m_newMinExtends; 
        m_maxExtends = m_newMaxExtends;
    }
} // parallel FireObject::BuildVertexBuffer

void FireObject::BuildVerticesCallback( void *param, u32 begin, u32 end )
{
    FireObject* pThis = static_cast<FireObject*>(param);

    pThis->BuildVerticesRange( begin, end );
}

void FireObject::BuildVerticesRange( u32 begin, u32 end )
{
    Math::Vector3 minExtends(FLT_MAX);
    Math::Vector3 maxExtends(-FLT_MAX);
    
    if(m_bRenderHeatParticles)
    {
        for( size_t i = begin; i < end; ++i )
        {
            if( m_Fires[i]->isActive() )
            {
                if( m_BurningList[i] != Burning )
                    continue;
            }
            else
                continue;

            ParticleEmitter::ParticleSystemWithEmitter *pswe = 
                        static_cast<ParticleEmitter::ParticleSystemWithEmitter*>(m_Fires[i]);
            const std::vector<ParticleEmitter::HeatParticle*> &particles = pswe->getAliveParticles();

            size_t      particlesSize = particles.size();
            VertexHP   *vhp = NULL;

            SCOPED_SPIN_LOCK_BEGIN(m_mutex);
            vhp = reinterpret_cast<VertexHP*>(m_pVertices);
            m_pVertices = vhp + particlesSize;
            SCOPED_SPIN_LOCK_END();

            for( size_t j = 0; j < particlesSize; ++j )
            {
                ParticleEmitter::HeatParticle   &particle = *particles[j];
                VertexHP::convertFromHeatParticle( vhp, &particle );
                ++vhp;
                if (minExtends.x > particle.initialPos.x) minExtends.x = particle.initialPos.x;
                if (minExtends.y > particle.initialPos.y) minExtends.y = particle.initialPos.y;
                if (minExtends.z > particle.initialPos.z) minExtends.z = particle.initialPos.z;
                if (maxExtends.x < particle.initialPos.x) maxExtends.x = particle.initialPos.x;
                if (maxExtends.y < particle.initialPos.y) maxExtends.y = particle.initialPos.y;
                if (maxExtends.z < particle.initialPos.z) maxExtends.z = particle.initialPos.z;
            }
        }
    }
    else
    {

        for( size_t i = begin; i < end; ++i )
        {
            if( m_Fires[i]->isActive() )
            {
                if( m_BurningList[i] != Burning)
                    continue;

                const std::vector<ParticleEmitter::Particle*>& particles = m_Fires[i]->getAliveParticles();

                size_t      particlesSize = particles.size();
                VertexFP   *vfp = NULL;

                SCOPED_SPIN_LOCK_BEGIN(m_mutex);
                vfp = reinterpret_cast<VertexFP*>(m_pVertices);
                m_pVertices = vfp + particlesSize;
                SCOPED_SPIN_LOCK_END();

                for( size_t j = 0; j < particlesSize; ++j )
                {
                    ParticleEmitter::Particle   &particle = *particles[j];
                    VertexFP::convertFromParticle( vfp, &particle );
                    ++vfp;
                    if (minExtends.x > particle.initialPos.x) minExtends.x = particle.initialPos.x;
                    if (minExtends.y > particle.initialPos.y) minExtends.y = particle.initialPos.y;
                    if (minExtends.z > particle.initialPos.z) minExtends.z = particle.initialPos.z;
                    if (maxExtends.x < particle.initialPos.x) maxExtends.x = particle.initialPos.x;
                    if (maxExtends.y < particle.initialPos.y) maxExtends.y = particle.initialPos.y;
                    if (maxExtends.z < particle.initialPos.z) maxExtends.z = particle.initialPos.z;
                }
            }
        }
    }
    SCOPED_SPIN_LOCK(m_mutex);
    if (m_newMinExtends.x > minExtends.x) m_newMinExtends.x = minExtends.x;
    if (m_newMinExtends.y > minExtends.y) m_newMinExtends.y = minExtends.y;
    if (m_newMinExtends.z > minExtends.z) m_newMinExtends.z = minExtends.z;
    if (m_newMaxExtends.x < maxExtends.x) m_newMaxExtends.x = maxExtends.x;
    if (m_newMaxExtends.y < maxExtends.y) m_newMaxExtends.y = maxExtends.y;
    if (m_newMaxExtends.z < maxExtends.z) m_newMaxExtends.z = maxExtends.z;
} // FireObject::BuildVerticesRange

#else /* !FIREOBJ_PARALLEL_BUILD_VERTICES */

void
FireObject::BuildVertexBuffer( void* pVertices )
{
    if ( m_bDirty )
        GetVertexCount(0);

    Math::Vector3 minExtends(FLT_MAX);
    Math::Vector3 maxExtends(-FLT_MAX);

    if(m_bRenderHeatParticles)
    {
        VertexHP *vhp = reinterpret_cast<VertexHP*>(m_pVertices);
        std::vector<ParticleEmitter::HeatParticle> particles;

        size_t size = m_Fires.size();
        for( size_t i = 0; i < size; ++i )
        {
            if( m_Fires[i]->isActive() )
            {
                particles.resize(0);
                if( m_BurningList[i] != Burning )
                    continue;
            }
            else
                continue;

            ParticleEmitter::ParticleSystemWithEmitter *pswe = 
                        static_cast<ParticleEmitter::ParticleSystemWithEmitter*>(m_Fires[i]);
            pswe->m_pHeatParticles->getAliveParticles(particles);
            size_t particlesSize = particles.size();
            for( size_t j = 0; j < particlesSize; ++j )
            {
                ParticleEmitter::HeatParticle   &particle = particles[j];
                VertexHP::convertFromHeatParticle( vhp, &particle );
                ++vhp;
                if (minExtends.x > particle.initialPos.x) minExtends.x = particle.initialPos.x;
                if (minExtends.y > particle.initialPos.y) minExtends.y = particle.initialPos.y;
                if (minExtends.z > particle.initialPos.z) minExtends.z = particle.initialPos.z;
                if (maxExtends.x < particle.initialPos.x) maxExtends.x = particle.initialPos.x;
                if (maxExtends.y < particle.initialPos.y) maxExtends.y = particle.initialPos.y;
                if (maxExtends.z < particle.initialPos.z) maxExtends.z = particle.initialPos.z;
            }
        }
    }
    else
    {
        VertexFP *vfp = reinterpret_cast<VertexFP*>(m_pVertices);
        std::vector<ParticleEmitter::Particle> particles;
        size_t size = m_Fires.size();
        for( size_t i = 0; i < size; ++i )
        {
            if( m_Fires[i]->isActive() )
            {
                particles.resize(0);
                if( m_BurningList[i] != Burning)
                    continue;

                m_Fires[i]->getAliveParticles(particles);
                size_t particlesSize = particles.size();
                for( size_t j = 0; j < particlesSize; ++j )
                {
                    ParticleEmitter::Particle   &particle = particles[j];
                    VertexFP::convertFromParticle( vfp, &particle );
                    ++vfp;
                    if (minExtends.x > particle.initialPos.x) minExtends.x = particle.initialPos.x;
                    if (minExtends.y > particle.initialPos.y) minExtends.y = particle.initialPos.y;
                    if (minExtends.z > particle.initialPos.z) minExtends.z = particle.initialPos.z;
                    if (maxExtends.x < particle.initialPos.x) maxExtends.x = particle.initialPos.x;
                    if (maxExtends.y < particle.initialPos.y) maxExtends.y = particle.initialPos.y;
                    if (maxExtends.z < particle.initialPos.z) maxExtends.z = particle.initialPos.z;
                }
            }
        }
    }
    if((minExtends.x == FLT_MAX) ||
        (minExtends.y == FLT_MAX) ||
        (minExtends.z == FLT_MAX) ||
        (maxExtends.x == FLT_MIN) ||
        (maxExtends.y == FLT_MIN) ||
        (maxExtends.z == FLT_MIN)){
    }else {
        m_minExtends = minExtends; 
        m_maxExtends = maxExtends;
    }
} // serial FireObject::BuildVertexBuffer

#endif /* !FIREOBJ_PARALLEL_BUILD_VERTICES */

void
FireObject::GetVertices(
    Out void* pVertices,
    In  u16 nSubMeshIndex,
    In  u16 nStreamIndex,
    In  u32 nVertexDeclCount,
    In  VertexDecl::Element* pVertexDecl
    )
{
	
	UNREFERENCED_PARAM( nVertexDeclCount );
	// Ignored as we use only one SubMesh
	UNREFERENCED_PARAM( nSubMeshIndex );
	// Ignored as we use only one stream
	UNREFERENCED_PARAM( nStreamIndex );

    // nVertexDeclCount should be 0 to indicate the full vertex list
    ASSERT( 0 == nVertexDeclCount );
    UNREFERENCED_PARAM( pVertexDecl );

#if FIREOBJ_PREBUILD_VERTICES
    if ( m_nVertexBufferSize )
        memcpy( pVertices, m_pVertexBuffer, m_nVertexBufferSize );
#else
    BuildVertexBuffer( pVertices );
#endif /* !FIREOBJ_PREBUILD_VERTICES */
}


std::string
FireObject::GetParticleGroupTechnique(
    void
    )
{       
    if(m_bRenderHeatParticles){
        return "HeatEmitter";
    }else{
        return m_Technique;
    }

}

void
FireObject::GetAABB(
    Out Math::Vector3& Min,
    Out Math::Vector3& Max
    )
{
    Min = m_minExtends;
    Max = m_maxExtends;

}

void 
FireObject::GetAreaBB( 
    Out Math::Vector3& Min, 
    Out Math::Vector3& Max )
{
    Min.x = m_ObjectWSBBox.xMin;
    Min.y = m_ObjectWSBBox.yMin;
    Min.z = m_ObjectWSBBox.zMin;
    Max.x = m_ObjectWSBBox.xMax;
    Max.y = m_ObjectWSBBox.yMax;
    Max.z = m_ObjectWSBBox.zMax;
}


Bool 
FireObject::fireIsActive()
{
 return m_fireIsActive;
}

#pragma warning( pop )
