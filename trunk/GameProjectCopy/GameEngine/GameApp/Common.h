#pragma once

//#include <windows.h>
//#include <stdio.h>
//#include <vector>
//#include <set>
//
//#include <math.h>
#include <float.h>// swap variables of any type

#include "EngineCore/Math/GameMath.h"
#include "GameApp/Util/DrawUtil.h"

// a simple function to convert degrees to radians
#define DegreeToRadian(fDegrees) ((3.14159265f/180.0f)*fDegrees)


/** Sort mode for billboard-set and particle-system */
enum SortMode
{
	/** Sort by direction of the camera */
	SM_DIRECTION,
	/** Sort by distance from the camera */
	SM_DISTANCE
};

/** Fog modes. */
enum FogMode
{
	/// No fog. Duh.
	FOG_NONE,
	/// Fog density increases  exponentially from the camera (fog = 1/e^(distance * density))
	FOG_EXP,
	/// Fog density increases at the square of FOG_EXP, i.e. even quicker (fog = 1/e^(distance * density)^2)
	FOG_EXP2,
	/// Fog density increases linearly between the start and end distances
	FOG_LINEAR
};


// return smaller of the given variables
//template<typename Type>
//inline Type Min(const Type &A, const Type &B)
//{
//  if (A < B) return A;
//  return B;
//}
//
//// return larger of the given variables
//template<typename Type>
//inline Type Max(const Type &A, const Type &B)
//{
//  if (A > B) return A;
//  return B;
//}

// time in seconds
inline double GetAccurateTime(void)
{
  __int64 iCurrentTime = 0;
  __int64 iFrequency = 1;
  QueryPerformanceFrequency((LARGE_INTEGER*)&iFrequency);
  QueryPerformanceCounter((LARGE_INTEGER*)&iCurrentTime);
  return (double)iCurrentTime / (double)iFrequency;
}

inline float DeltaTimeUpdate(double &fLastUpdate)
{
  double fTimeNow = GetAccurateTime();
  float fDeltaTime = Clamp((float)(fTimeNow - fLastUpdate), 0.0f, 1.0f);
  fLastUpdate = fTimeNow;
  return fDeltaTime;
}

struct PickData {
	unsigned int dwFace;
	float fBary1, fBary2;
	float fDist;
	noVec3 vert;
	void* pUserData;
	
};

struct PickSort
{
	bool operator()(PickData data1, PickData data2)
	{
		return data1.fDist < data2.fDist;
	}
};

class Mesh;	
class CMeshLoader10;

class PickUtil {

public:
	static void Clear();
	static void SavePickData() { pickDatas.push_back(ms_pickData); }

	static bool Picking(class SceneNode* RootNode);

	static bool Intersect( Mesh* pMesh, const noVec3& vPickRayOrig, const noVec3& vPickRayDir );
	static bool Intersect( CMeshLoader10* pMesh, const noVec3& vPickRayOrig, const noVec3& vPickRayDir );
	static bool Intersect( class CDXUTSDKMesh* pMesh, const noVec3& vPickRayOrig, const noVec3& vPickRayDir );
	static DWORD GetFace() { return ms_pickData.dwFace; }
	static float GetPickDist() { return ms_pickData.fDist; }
	static noVec3&	GetPickVert() { return ms_pickData.vert; }
	static void GetBarys(float& out1, float& out2) { out1 = ms_pickData.fBary1; out2 = ms_pickData.fBary2; }
	
	static std::vector<PickData>	pickDatas;
	static PickData	ms_pickData;

	static void ConvertRayFromWorldToLocal( const noTransform& kWord, const noVec3& kWorldOrigin, 
		const noVec3& kWordDir, noVec3& localOrig, noVec3& klocalDir);
	static bool FindBoundIntersect( const BoundingBox& kWorldBB, const noVec3& kOrig, const noVec3& kDir);
};

bool IntersectTriangle( const noVec3& orig, const noVec3& dir, noVec3& v0, noVec3& v1, noVec3& v2, FLOAT* t, FLOAT* u, FLOAT* v );

class Mesh* makeMesh(RenderData& rd, noVec3* vert, uint32 numVerts, uint16* indices, uint32 numIndices, noVec4 col = noVec4(1.f, 0.f ,0.5f, 1.f));
class Mesh* makeMesh( RenderData& rd, const char* file );
class Mesh* makeEditGrid(RenderData& rd, const noVec3& corner, const noVec3& sideA, const noVec3& sideB, int resA /*= 1*/, int resB /*= 1*/, int numTexCoords /*= 1*/, bool vcolor /*= false*/, bool tangents /*= false*/ );
class Mesh* makeGrid(RenderData& rd, const noVec3& corner, const noVec3& sideA, const noVec3& sideB, int resA /*= 1*/, int resB /*= 1*/, int numTexCoords /*= 1*/, bool vcolor /*= false*/, bool tangents /*= false*/ );

   template< typename T > struct TRect
        {
          T left, top, right, bottom;
          TRect() : left(0), top(0), right(0), bottom(0) {}
          TRect( T const & l, T const & t, T const & r, T const & b )
            : left( l ), top( t ), right( r ), bottom( b )
          {
          }
          TRect( TRect const & o )
            : left( o.left ), top( o.top ), right( o.right ), bottom( o.bottom )
          {
          }
          TRect & operator=( TRect const & o )
          {
            left = o.left;
            top = o.top;
            right = o.right;
            bottom = o.bottom;
            return *this;
          }
          T width() const
          {
            return right - left;
          }
          T height() const
          {
            return bottom - top;
          }
		  bool isNull() const
		  {
			  return width() == 0 || height() == 0;
		  }
		  void setNull()
		  {
			  left = right = top = bottom = 0;
		  }
		  TRect & merge(const TRect& rhs)
		  {
			  if (isNull())
			  {
				  *this = rhs;
			  }
			  else if (!rhs.isNull())
			  {
				  left = std::min(left, rhs.left);
				  right = std::max(right, rhs.right);
				  top = std::min(top, rhs.top);
				  bottom = std::max(bottom, rhs.bottom);
			  }

			  return *this;

		  }
		  TRect intersect(const TRect& rhs) const
		  {
			  TRect ret;
			  if (isNull() || rhs.isNull())
			  {
				  // empty
				  return ret;
			  }
			  else
			  {
				  ret.left = std::max(left, rhs.left);
				  ret.right = std::min(right, rhs.right);
				  ret.top = std::max(top, rhs.top);
				  ret.bottom = std::min(bottom, rhs.bottom);
			  }

			  if (ret.left > ret.right || ret.top > ret.bottom)
			  {
				  // no intersection, return empty
				  ret.left = ret.top = ret.right = ret.bottom = 0;
			  }

			  return ret;

		  }

        };
		template<typename T>
		std::ostream& operator<<(std::ostream& o, const TRect<T>& r)
		{
			o << "TRect<>(l:" << r.left << ", t:" << r.top << ", r:" << r.right << ", b:" << r.bottom << ")";
			return o;
		}

        /** Structure used to define a rectangle in a 2-D floating point space.
        */
        typedef TRect<float> FloatRect;

		/** Structure used to define a rectangle in a 2-D floating point space, 
			subject to double / single floating point settings.
		*/
		typedef TRect<Real> RealRect;

        /** Structure used to define a rectangle in a 2-D integer space.
        */
        typedef TRect< long > Rect;

        /** Structure used to define a box in a 3-D integer space.
         	Note that the left, top, and front edges are included but the right, 
         	bottom and back ones are not.
         */
        struct Box
        {
            size_t left, top, right, bottom, front, back;
			/// Parameterless constructor for setting the members manually
            Box()
				: left(0), top(0), right(1), bottom(1), front(0), back(1)
            {
            }
            /** Define a box from left, top, right and bottom coordinates
            	This box will have depth one (front=0 and back=1).
            	@param	l	x value of left edge
            	@param	t	y value of top edge
            	@param	r	x value of right edge
            	@param	b	y value of bottom edge
            	@note Note that the left, top, and front edges are included 
 		           	but the right, bottom and back ones are not.
            */
            Box( size_t l, size_t t, size_t r, size_t b ):
                left(l),
                top(t),   
                right(r),
                bottom(b),
                front(0),
                back(1)
            {
          		assert(right >= left && bottom >= top && back >= front);
            }
            /** Define a box from left, top, front, right, bottom and back
            	coordinates.
            	@param	l	x value of left edge
            	@param	t	y value of top edge
            	@param  ff  z value of front edge
            	@param	r	x value of right edge
            	@param	b	y value of bottom edge
            	@param  bb  z value of back edge
            	@note Note that the left, top, and front edges are included 
 		           	but the right, bottom and back ones are not.
            */
            Box( size_t l, size_t t, size_t ff, size_t r, size_t b, size_t bb ):
                left(l),
                top(t),   
                right(r),
                bottom(b),
                front(ff),
                back(bb)
            {
          		assert(right >= left && bottom >= top && back >= front);
            }
            
            /// Return true if the other box is a part of this one
            bool contains(const Box &def) const
            {
            	return (def.left >= left && def.top >= top && def.front >= front &&
					def.right <= right && def.bottom <= bottom && def.back <= back);
            }
            
            /// Get the width of this box
            size_t getWidth() const { return right-left; }
            /// Get the height of this box
            size_t getHeight() const { return bottom-top; }
            /// Get the depth of this box
            size_t getDepth() const { return back-front; }
        };


		typedef TRect<float> FloatRect;

		 struct noBox
        {
            size_t left, top, right, bottom, front, back;
			/// Parameterless constructor for setting the members manually
            noBox()
				: left(0), top(0), right(1), bottom(1), front(0), back(1)
            {
            }
            /** Define a box from left, top, right and bottom coordinates
            	This box will have depth one (front=0 and back=1).
            	@param	l	x value of left edge
            	@param	t	y value of top edge
            	@param	r	x value of right edge
            	@param	b	y value of bottom edge
            	@note Note that the left, top, and front edges are included 
 		           	but the right, bottom and back ones are not.
            */
            noBox( size_t l, size_t t, size_t r, size_t b ):
                left(l),
                top(t),   
                right(r),
                bottom(b),
                front(0),
                back(1)
            {
          		assert(right >= left && bottom >= top && back >= front);
            }
            /** Define a box from left, top, front, right, bottom and back
            	coordinates.
            	@param	l	x value of left edge
            	@param	t	y value of top edge
            	@param  ff  z value of front edge
            	@param	r	x value of right edge
            	@param	b	y value of bottom edge
            	@param  bb  z value of back edge
            	@note Note that the left, top, and front edges are included 
 		           	but the right, bottom and back ones are not.
            */
            noBox( size_t l, size_t t, size_t ff, size_t r, size_t b, size_t bb ):
                left(l),
                top(t),   
                right(r),
                bottom(b),
                front(ff),
                back(bb)
            {
          		assert(right >= left && bottom >= top && back >= front);
            }
            
            /// Return true if the other box is a part of this one
            bool contains(const Box &def) const
            {
            	return (def.left >= left && def.top >= top && def.front >= front &&
					def.right <= right && def.bottom <= bottom && def.back <= back);
            }
            
            /// Get the width of this box
            size_t getWidth() const { return right-left; }
            /// Get the height of this box
            size_t getHeight() const { return bottom-top; }
            /// Get the depth of this box
            size_t getDepth() const { return back-front; }
        };

		
	

