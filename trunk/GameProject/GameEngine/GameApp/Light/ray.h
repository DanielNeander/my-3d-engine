/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/

#ifndef __RAY_H__
#define __RAY_H__

//#include "Vector.h"
#include "prtfw.h"

static int countP = 0;

class Ray
{
public:
    noVec3 o,      //!< Origin of ray
					// Target;
            d;      //!< Direction of ray
	unsigned id;
	noMat4 worldtocam; //world to camera transformation matrix
	float scalex, scaley; //x,y scaling coefficients

    Ray() : o(), d(noVec3(0.0f,0.0f,1.0f))
    {
        id = countP++;
    }

    Ray(const noVec3& o, const noVec3& d) : o(o), d(d)
    {
		id = countP++;
    }
	// refresh our mailbox id
	void nextID() { id = countP++; }

	float Normalize() {
		float len = d.LengthSqr();
		d /= len;
		return len;
	}
};

class Mailbox { //for mailboxing, a object wrapper class
public:
	//Object* obj; 
	unsigned lastrayid;
};

//! Contains information about a ray hit with a surface.
/*!
    HitInfos are used by object intersection routines. They are useful in
    order to return more than just the hit distance.
*/
class HitInfo
{
public:
    noVec3 color;                      //!< The color at the hit point
    float t;                            //!< The hit distance
    noVec3 P;                          //!< The hit point
    noVec3 N;                          //!< Shading normal vector
    const class Material* material;           //!< Material of the intersected object
	float dudx, dudy, dvdx, dvdy;
	noVec3 dpdu, dpdv;
	float hitu, hitv;

    //! Default constructor.
    explicit HitInfo(const noVec3& color = noVec3(),
                     float t = 0.0f,
                     const noVec3& P = noVec3(),
                     const noVec3& N = noVec3(0.0f, 1.0f, 0.0f)) :
        color(color), t(t), P(P), N(N), material (0)
    {
        // empty
    }
};

#endif
