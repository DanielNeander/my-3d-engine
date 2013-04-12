#pragma once


struct Collision;
class CollisionList;
struct CollisionStateList;
class AbstractPolyList;
class SceneNode;
//class BaseMatInstance;
class Convex;

class ConvexFeature
{
public:
	struct Edge {
		int vertex[2];
	};
	struct Face {
		noVec3 normal;
		int vertex[3];
	};

	Array<noVec3> mVertexList;
	Array<Edge> mEdgeList;
	Array<Face> mFaceList;
	//BaseMatInstance* material;
	SceneNode* object;

	ConvexFeature()
		: mVertexList(64), mEdgeList(128), mFaceList(64)
	{
		//VECTOR_SET_ASSOCIATION(mVertexList);
		//VECTOR_SET_ASSOCIATION(mEdgeList);
		//VECTOR_SET_ASSOCIATION(mFaceList);
	}

	bool collide(ConvexFeature& cf,CollisionList* cList, float tol = 0.1);
	void testVertex(const noVec3& v,CollisionList* cList,bool,float tol);
	void testEdge(ConvexFeature* cf,const noVec3& s1, const noVec3& e1, CollisionList* cList, float tol);
	bool inVolume(const noVec3& v);
};



enum ConvexType {
	TSConvexType,
	BoxConvexType,
	TerrainConvexType,
	InteriorConvexType,
	ShapeBaseConvexType,
	TSStaticConvexType,
	AtlasChunkConvexType, ///< @deprecated
	AtlasConvexType,   
	TSPolysoupConvexType,
};

struct CollisionState
{
	CollisionStateList* mLista;
	CollisionStateList* mListb;
	Convex* a;
	Convex* b;

	float dist;            // Current estimated distance
	noVec3 v;           // Vector between closest points

	//
	CollisionState();
	virtual ~CollisionState();
	virtual void swap();
	virtual void set(Convex* a,Convex* b,const noMat4& a2w, const noMat4& b2w);
	virtual float distance(const noMat4& a2w, const noMat4& b2w, const float dontCareDist,
		const noMat4* w2a = NULL, const noMat4* _w2b = NULL);
};


struct CollisionStateList
{
	static CollisionStateList sFreeList;
	CollisionStateList* mNext;
	CollisionStateList* mPrev;
	CollisionState* mState;

	CollisionStateList();

	void linkAfter(CollisionStateList* next);
	void unlink();
	bool isEmpty() { return mNext == this; }

	static CollisionStateList* alloc();
	void free();
};



__declspec(selectany) CollisionStateList CollisionStateList::sFreeList;

struct CollisionWorkingList
{
	static CollisionWorkingList sFreeList;
	struct WLink {
		CollisionWorkingList* mNext;
		CollisionWorkingList* mPrev;
	} wLink;
	struct RLink {
		CollisionWorkingList* mNext;
		CollisionWorkingList* mPrev;
	} rLink;
	Convex* mConvex;

	void wLinkAfter(CollisionWorkingList* next);
	void rLinkAfter(CollisionWorkingList* next);
	void unlink();
	CollisionWorkingList();

	static CollisionWorkingList* alloc();
	void free();
};

class Convex {

	/// @name Linked list management
	/// @{

	/// Next item in linked list of Convexes.
	Convex* mNext;

	/// Previous item in linked list of Convexes.
	Convex* mPrev; 

	/// Insert this Convex after the provided convex
	/// @param   next
	void linkAfter(Convex* next);

	/// Remove this Convex from the linked list
	void unlink();
	/// @}

	uint32 mTag;
	static uint32 sTag;

protected:
	CollisionStateList   mList;            ///< Objects we're testing against
	CollisionWorkingList mWorking;         ///< Objects within our bounds
	CollisionWorkingList mReference;       ///< Other convex testing against us
	SceneNode* mObject;                  ///< Object this Convex is built around
	ConvexType mType;                      ///< Type of Convex this is @see ConvexType

public:

	/// Constructor
	Convex();

	/// Destructor
	virtual ~Convex();

	/// Registers another Convex by linking it after this one
	void registerObject(Convex *convex);

	/// Runs through the linked list of Convex objects and removes the ones
	/// with no references
	void collectGarbage();

	/// Deletes all convex objects in the list
	void nukeList();

	/// Returns the type of this Convex
	ConvexType getType()      { return mType;   }

	/// Returns the object this Convex is built from
	SceneNode* getObject()  { return mObject; }

	/// Adds the provided Convex to the list of objects within the bounds of this Convex
	/// @param   ptr    Convex to add to the working list of this object
	void                  addToWorkingList(Convex* ptr);

	/// Returns the list of objects currently inside the bounds of this Convex
	CollisionWorkingList& getWorkingList() { return mWorking; }

	/// Finds the closest
	CollisionState* findClosestState(const noMat4& mat, const noVec3& scale, const float dontCareDist = 1);

	/// Returns the list of objects this object is testing against
	CollisionStateList*   getStateList()   { return mList.mNext; }

	/// Updates the CollisionStateList (mList) with new collision states and removing
	/// ones no longer under consideration
	/// @param   mat   Used as the matrix to create a bounding box for updating the list
	/// @param   scale   Used to scale the bounding box
	/// @param   displacement   Bounding box displacement (optional)
	void updateStateList(const noMat4& mat, const noVec3& scale, const noVec3* displacement = NULL);

	/// Updates the working collision list of objects which are currently colliding with
	/// (inside the bounds of) this Convex.
	///
	/// @param  box      Used as the bounding box.
	/// @param  colMask  Mask of objects to check against.
	void updateWorkingList(const BoundingBox& box, const uint32 colMask);

	/// Returns the transform of the object this is built around
	virtual const noMat4& getTransform() const;

	/// Returns the scale of the object this is built around
	virtual const noVec3& getScale() const;

	/// Returns the bounding box for the object this is built around in world space
	virtual BoundingBox getBoundingBox() const;

	/// Returns the object space bounding box for the object this is built around
	/// transformed and scaled
	/// @param   mat   Matrix to transform the object-space box by
	/// @param   scale   Scaling factor to scale the bounding box by
	virtual BoundingBox getBoundingBox(const noMat4& mat, const noVec3& scale) const;

	/// Returns the farthest point, along a vector, still bound by the convex
	/// @param   v   Vector
	virtual noVec3 support(const noVec3& v) const;

	///
	virtual void getFeatures(const noMat4& mat,const noVec3& n, ConvexFeature* cf);

	/// Builds a collision poly list out of this convex
	/// @param   list   (Out) Poly list built
	virtual void getPolyList(AbstractPolyList* list);

	///
	bool getCollisionInfo(const noMat4& mat, const noVec3& scale, CollisionList* cList,float tol);
};

