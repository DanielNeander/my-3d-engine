#include "noTransform.h"


void noTransform::MakeIdentity()
{
	rotate_.Identity();
	trans_ = vec3_zero;
	scale_ = 1.0f;
	isDirty = false;
}

noTransform noTransform::operator*( const noTransform &xform ) const
{
	noTransform res;

	res.scale_ = xform.scale_ * scale_;		
	res.rotate_ = xform.rotate_ * rotate_ ;	// 순서 조심... 
	res.trans_ = trans_ + scale_ * (rotate_ * xform.trans_);

	return res;
}



void noTransform::Interpolate( float fAmount, const noTransform& kStart, const noTransform& kFinish, noTransform& kOutput )
{

}

bool noTransform::operator !=( const noTransform &xform ) const
{
	return rotate_ != xform.rotate_ ||
		trans_ != xform.trans_ ||
		scale_ != xform.scale_;
}

void noTransform::Invert(noTransform& kDest) const 
{
	kDest.rotate_ = rotate_.Transpose();
	kDest.scale_ = 1.0f / scale_;
	kDest.trans_ = kDest.scale_ * (kDest.rotate_ * -trans_);
}

