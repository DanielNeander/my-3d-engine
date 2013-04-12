#ifndef __MATH__TRANSFORM__H__
#define	__MATH__TRANSFORM__H__

#include "Matrix.h"
#include "Vector.h"

class noTransform
{
public:
	noMat3 rotate_;
	noVec3 trans_;
	float scale_;

	bool isDirty;

	void MakeIdentity();
	bool operator!=(const noTransform &xform) const;
	noTransform operator*(const noTransform &xform) const;
	void Invert(noTransform& kDest) const;
	inline noVec3 operator*(const noVec3 &kPoint) const;
	static void Interpolate(
		float fAmount, 
		const noTransform& kStart,
		const noTransform& kFinish,
		noTransform& kOutput);
};

inline noVec3 noTransform::operator*( const noVec3 &kPoint ) const
{
	return (((rotate_ * kPoint) * scale_) + trans_);
}

#endif