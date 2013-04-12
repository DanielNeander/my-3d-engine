/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/

#ifndef __BBOX_H__
#define __BBOX_H__

#include "ray.h"		//Ray


#define XAXIS 0
#define YAXIS 1
#define ZAXIS 2

class Bbox
{
public:
	Bbox() {
		xmin = ymin = zmin = FLT_MAX;
		xmax = ymax = zmax = -FLT_MAX;
	}

    float xmin, xmax;
	float ymin, ymax;
	float zmin, zmax;

	bool intersectBB(const Ray& ray, float* hitmin, float* hitmax, 
                     float tmin, float tmax) {
		float origpart;
		float dirpart;
		float xtmin, xtmax;
		float ytmin, ytmax;
		float ztmin, ztmax;
		float temp; //for float switching

		const Ray& normray = ray;
		origpart = normray.o.x;
		dirpart = 1/normray.d.x;
		xtmin = (xmin - origpart) * dirpart;
		xtmax = (xmax - origpart) * dirpart;
		if (xtmax < xtmin) {
			temp = xtmin;
			xtmin = xtmax;
			xtmax = temp;
		}
		*hitmin = xtmin;
		*hitmax = xtmax;
		
		origpart = normray.o.y;
		dirpart = 1/normray.d.y;
		ytmin = (ymin - origpart) * dirpart;
		ytmax = (ymax - origpart) * dirpart;
		if (ytmax < ytmin) {
			temp = ytmin;
			ytmin = ytmax;
			ytmax = temp;
		}
		if ((xtmin > ytmax) || (ytmin > xtmax)) { //save some calcs
			return false;
		}
		if (ytmin > *hitmin) {
			*hitmin = ytmin;
		}
		if (ytmax < *hitmax) {
			*hitmax = ytmax;
		}

		origpart = normray.o.z;
		dirpart = 1/normray.d.z;
		ztmin = (zmin - origpart) * dirpart;
		ztmax = (zmax - origpart) * dirpart;
		if (ztmax < ztmin) {
			temp = ztmin;
			ztmin = ztmax;
			ztmax = temp;
		}
		if ( (xtmin > ztmax) || (ztmin > xtmax) ) {
			return false;
		}
		if (ztmin > *hitmin) {
			*hitmin = ztmin;
			//*hitmin *= .9999f;
		}
		if (ztmax < *hitmax) {
			*hitmax = ztmax;
			//*hitmax *= 1.0001f;
		}
		return ( (*hitmin < tmax) && (*hitmax > tmin) );
	}

	void update(float minval, float maxval, int axis) {
		if (axis == XAXIS) {
			if (minval < xmin) { //found lesser xmin
				xmin = minval;
			}
			if (maxval > xmax) { //found greater xmax
				xmax = maxval;
			}
		}

		if (axis == YAXIS) {
			if (minval < ymin) { //found lesser ymin
				ymin = minval;
			}
			if (maxval > ymax) { //found greater ymax
				ymax = maxval;
			}
		}

		if (axis == ZAXIS) {
			if (minval < zmin) { //found lesser zmin
				zmin = minval;
			}
			if (maxval > zmax) { //found greater zmax
				zmax = maxval;
			}
		}
	}

	bool intersectBB(const Bbox& box) {
		if (xmax < xmin) {//trianglemesh bbox, ignore it
			return false;
		}
		if (!((xmin <= box.xmax) && (xmax >= box.xmin))) {
			return false; 
		}
		//x-coords do overlap
		if (!((ymin <= box.ymax) && (ymax >= box.ymin))) {
			return false;
		}
		//y-coords do overlap
		if (!((zmin <= box.zmax) && (zmax >= box.zmin))) {
			return false;
		}
		//all coords overlap, have intersection
		return true;
	}//box-box intersection
};

#endif
