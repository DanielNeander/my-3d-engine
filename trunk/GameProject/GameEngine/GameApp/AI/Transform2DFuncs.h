#ifndef __MATH__TRANSFORM2D_H__
#define __MATH__TRANSFORM2D_H__

//------------------------------------------------------------------------
//
//  Name:   Transformations.h
//
//  Desc:   Functions for converting 2D vectors between World and Local
//          space.
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include "Matrix2D.h"

//--------------------------- WorldTransform -----------------------------
//
//  given a std::vector of 2D vectors, a position, orientation and scale,
//  this function transforms the 2D vectors into the object's world space
//------------------------------------------------------------------------
inline std::vector<noVec2> WorldTransform(std::vector<noVec2> &points,
	const noVec2   &pos,
	const noVec2   &forward,
	const noVec2   &side,
	const noVec2   &scale)
{
	//copy the original vertices into the buffer about to be transformed
	std::vector<noVec2> TrannoVec2s = points;

	//create a transformation matrix
	Matrix2D matTransform;

	//scale
	if ( (scale.x != 1.0) || (scale.y != 1.0) )
	{
		matTransform.Scale(scale.x, scale.y);
	}

	//rotate
	matTransform.Rotate(forward, side);

	//and translate
	matTransform.Translate(pos.x, pos.y);

	//now transform the object's vertices
	matTransform.TransformVector2Ds(TrannoVec2s);

	return TrannoVec2s;
}

//--------------------------- WorldTransform -----------------------------
//
//  given a std::vector of 2D vectors, a position and  orientation
//  this function transforms the 2D vectors into the object's world space
//------------------------------------------------------------------------
inline std::vector<noVec2> WorldTransform(std::vector<noVec2> &points,
	const noVec2   &pos,
	const noVec2   &forward,
	const noVec2   &side)
{
	//copy the original vertices into the buffer about to be transformed
	std::vector<noVec2> TrannoVec2s = points;

	//create a transformation matrix
	Matrix2D matTransform;

	//rotate
	matTransform.Rotate(forward, side);

	//and translate
	matTransform.Translate(pos.x, pos.y);

	//now transform the object's vertices
	matTransform.TransformVector2Ds(TrannoVec2s);

	return TrannoVec2s;
}

//--------------------- PointToWorldSpace --------------------------------
//
//  Transforms a point from the agent's local space into world space
//------------------------------------------------------------------------
inline noVec3 PointToWorldSpace(const noVec3 &_point,
	const noVec3 &_AgentHeading,
	const noVec3 &_AgentSide,
	const noVec3 &_AgentPosition)
{
	noVec2 point(_point.x, _point.z);
	noVec2 AgentHeading(_AgentHeading.x, _AgentHeading.z);
	noVec2 AgentSide(_AgentSide.x, _AgentSide.z);
	noVec2 AgentPosition(_AgentPosition.x, _AgentPosition.z);

	//make a copy of the point
	noVec2 TransPoint = point;

	//create a transformation matrix
	Matrix2D matTransform;

	//rotate
	matTransform.Rotate(AgentHeading, AgentSide);

	//and translate
	matTransform.Translate(AgentPosition.x, AgentPosition.y);

	//now transform the vertices
	matTransform.TransformVector2Ds(TransPoint);

	return noVec3(TransPoint.x, _point.y, TransPoint.y) ;

}

//--------------------- VectorToWorldSpace --------------------------------
//
//  Transforms a vector from the agent's local space into world space
//------------------------------------------------------------------------
inline noVec3 VectorToWorldSpace(const noVec3 &_vec,
	const noVec3 &_AgentHeading,
	const noVec3 &_AgentSide)
{
	noVec2 vec(_vec.x, _vec.z);
	noVec2 AgentHeading(_AgentHeading.x, _AgentHeading.z);
	noVec2 AgentSide(_AgentSide.x, _AgentSide.z);

	//make a copy of the point
	noVec2 TransVec = vec;

	//create a transformation matrix
	Matrix2D matTransform;

	//rotate
	matTransform.Rotate(AgentHeading, AgentSide);

	//now transform the vertices
	matTransform.TransformVector2Ds(TransVec);

	return noVec3(TransVec.x, _vec.y, TransVec.y);
}


//--------------------- PointToLocalSpace --------------------------------
//
//------------------------------------------------------------------------
inline noVec3 PointToLocalSpace(const noVec3 &_point,
	noVec3 &_AgentHeading,
	noVec3 &_AgentSide,
	noVec3 &_AgentPosition)
{
	noVec2 point(_point.x, _point.z);
	noVec2 AgentHeading(_AgentHeading.x, _AgentHeading.z);
	noVec2 AgentSide(_AgentSide.x, _AgentSide.z);
	noVec2 AgentPosition(_AgentPosition.x, _AgentPosition.z);


	//make a copy of the point
	noVec2 TransPoint = point;

	//create a transformation matrix
	Matrix2D matTransform;

	double Tx = -AgentPosition * AgentHeading;
	double Ty = -AgentPosition * AgentSide;

	//create the transformation matrix
	matTransform._11(AgentHeading.x); matTransform._12(AgentSide.x);
	matTransform._21(AgentHeading.y); matTransform._22(AgentSide.y);
	matTransform._31(Tx);           matTransform._32(Ty);

	//now transform the vertices
	matTransform.TransformVector2Ds(TransPoint);

	return noVec3(TransPoint.x, _point.y, TransPoint.y) ;
}

//--------------------- VectorToLocalSpace --------------------------------
//
//------------------------------------------------------------------------
inline noVec2 VectorToLocalSpace(const noVec2 &vec,
	const noVec2 &AgentHeading,
	const noVec2 &AgentSide)
{ 

	//make a copy of the point
	noVec2 TransPoint = vec;

	//create a transformation matrix
	Matrix2D matTransform;

	//create the transformation matrix
	matTransform._11(AgentHeading.x); matTransform._12(AgentSide.x);
	matTransform._21(AgentHeading.y); matTransform._22(AgentSide.y);

	//now transform the vertices
	matTransform.TransformVector2Ds(TransPoint);

	return TransPoint;
}

//-------------------------- Vec2DRotateAroundOrigin --------------------------
//
//  rotates a vector ang rads around the origin
//-----------------------------------------------------------------------------
inline void Vec2DRotateAroundOrigin(noVec3& _v, double ang)
{
	//create a transformation matrix
	noVec2 v(_v.x, _v.z);
	Matrix2D mat;

	//rotate
	mat.Rotate(ang);

	//now transform the object's vertices
	mat.TransformVector2Ds(v);

	_v.x = v.x;
	_v.z = v.y;
}

//------------------------ CreateWhiskers ------------------------------------
//
//  given an origin, a facing direction, a 'field of view' describing the 
//  limit of the outer whiskers, a whisker length and the number of whiskers
//  this method returns a vector containing the end positions of a series
//  of whiskers radiating away from the origin and with equal distance between
//  them. (like the spokes of a wheel clipped to a specific segment size)
//----------------------------------------------------------------------------
inline std::vector<noVec3> CreateWhiskers(unsigned int  NumWhiskers,
	double        WhiskerLength,
	double        fov,
	noVec3      facing,
	noVec3      origin)
{
	//this is the magnitude of the angle separating each whisker
	double SectorSize = fov/(double)(NumWhiskers-1);

	std::vector<noVec3> whiskers;
	noVec3 temp;
	double angle = -fov*0.5; 

	for (unsigned int w=0; w<NumWhiskers; ++w)
	{
		//create the whisker extending outwards at this angle
		temp = facing;
		Vec2DRotateAroundOrigin(temp, angle);
		whiskers.push_back(origin + WhiskerLength * temp);

		angle+=SectorSize;
	}

	return whiskers;
}



#endif