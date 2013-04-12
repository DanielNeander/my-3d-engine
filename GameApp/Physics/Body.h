#ifndef _BODY_H_
#define _BODY_H_

#include "coordinateframe.h"

class RigidBodyState : public CoordFrame
{
public:
	noSymmetricMat3	tensor_;			//inertia tensor with respect to parent frame, kg*m^2
	noSymmetricMat3	inv_tensor_;		//inverse inertia tensor
	noVec3		linearVelocity_;		//linear velocity, meters/sec
	noVec3		angularVelocity_;		//angular velocity, radians/sec

public:
	RigidBodyState() {}

	RigidBodyState( const noVec3& v, const noVec3& w )
		:linearVelocity_(v),
		angularVelocity_(w)
	{

	}

	//calculate the inertia tensor and its
	//inverse from the current orientation
	//and the principal moments of inertia
	void calculateInertiaTensor( const noVec3& ip );
};

class RigidBody : public RigidBodyState
{
public:
	RigidBodyState prevState_;

	float mass_;		// kg
	noVec3	inertia_;	//principal moments of inertia, kg * m^2
	noVec3	dim_;	// bounding bos dimmensions

public:

	RigidBody()
		:	mass_(0)
	{}

	RigidBody(	const float m, const noVec3& d, const noVec3 ip = noVec3(0,0,0) )
		: mass_	(m),
		dim_	(d),
		inertia_	(ip)
	{
		//if no moments were passed in,
		//give the moments for a box
		if( ip.x==0 && ip.y==0 && ip.z==0 )
		{
			this->inertia_ = (m/12) * noVec3(d.y*d.y + d.z*d.z,
				d.x*d.x + d.z*d.z,
				d.x*d.x + d.y*d.y);
		}

		//before any physics are done
		this->calculateInertiaTensor( this->inertia_ );
	}

	//physics
	void mass( const float m )							{ mass_ = m; }
	const float mass() const							{ return mass_; }

	void principalInertia( const noVec3& ip )			{ inertia_ = ip; }
	const noVec3& principalInertia() const				{ return inertia_; }

	const RigidBodyState currentState() const			{ return *this; }
	const RigidBodyState previousState() const		{ return prevState_; }

	//time derivative of angular velocity
	const noVec3 dwdt( const noVec3& T ) const
	{
		return this->inv_tensor_ * ( T - this->angularVelocity_.Cross(this->tensor_*this->angularVelocity_) );
	}

	void rotate( const noVec3& v )
	{
		this->BASIS::rotate( v );
		this->calculateInertiaTensor( this->inertia_ );
	}
};

#endif
