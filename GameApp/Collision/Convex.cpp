#include "stdafx.h"
#include "Convex.h"

bool ConvexFeature::collide( ConvexFeature& cf,CollisionList* cList, float tol /*= 0.1*/ )
{

}

void ConvexFeature::testVertex( const noVec3& v,CollisionList* cList,bool,float tol )
{

}

void ConvexFeature::testEdge( ConvexFeature* cf,const noVec3& s1, const noVec3& e1, CollisionList* cList, float tol )
{

}

bool ConvexFeature::inVolume( const noVec3& v )
{

}

CollisionState::CollisionState()
{

}

CollisionState::~CollisionState()
{

}

void CollisionState::swap()
{

}

void CollisionState::set( Convex* a,Convex* b,const noMat4& a2w, const noMat4& b2w )
{

}

float CollisionState::distance( const noMat4& a2w, const noMat4& b2w, const float dontCareDist, const noMat4* w2a /*= NULL*/, const noMat4* _w2b /*= NULL*/ )
{

}

CollisionStateList::CollisionStateList()
{

}

void CollisionStateList::linkAfter( CollisionStateList* next )
{

}

void CollisionStateList::unlink()
{

}

CollisionStateList* CollisionStateList::alloc()
{

}

void CollisionStateList::free()
{

}

void Convex::linkAfter( Convex* next )
{

}

void Convex::unlink()
{

}

Convex::Convex()
{

}

Convex::~Convex()
{

}

void Convex::registerObject( Convex *convex )
{

}

void Convex::collectGarbage()
{

}

void Convex::nukeList()
{

}

void Convex::addToWorkingList( Convex* ptr )
{

}

CollisionState* Convex::findClosestState( const noMat4& mat, const noVec3& scale, const float dontCareDist /*= 1*/ )
{

}

void Convex::updateStateList( const noMat4& mat, const noVec3& scale, const noVec3* displacement /*= NULL*/ )
{

}

void Convex::updateWorkingList( const BoundingBox& box, const uint32 colMask )
{

}

const noMat4& Convex::getTransform() const
{

}

const noVec3& Convex::getScale() const
{

}

BoundingBox Convex::getBoundingBox() const
{

}

BoundingBox Convex::getBoundingBox( const noMat4& mat, const noVec3& scale ) const
{

}

noVec3 Convex::support( const noVec3& v ) const
{

}

void Convex::getFeatures( const noMat4& mat,const noVec3& n, ConvexFeature* cf )
{

}

void Convex::getPolyList( AbstractPolyList* list )
{

}

bool Convex::getCollisionInfo( const noMat4& mat, const noVec3& scale, CollisionList* cList,float tol )
{

}

__declspec(selectany) uint32 Convex::sTag;
