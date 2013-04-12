#include "stdafx.h"
#include "Body.h"


void RigidBodyState::calculateInertiaTensor( const noVec3& p )
{
	const noVec3 ip( 1/p.x, 1/p.y, 1/p.z );

	//since it's a symmetric matrix, there are only 6 moments and products
	this->tensor_.xx = p.x*R[0][0]*R[0][0] + p.y*R[1][0]*R[1][0] + p.z*R[2][0]*R[2][0];
	this->tensor_.yy = p.x*R[0][1]*R[0][1] + p.y*R[1][1]*R[1][1] + p.z*R[2][1]*R[2][1];
	this->tensor_.zz = p.x*R[0][2]*R[0][2] + p.y*R[1][2]*R[1][2] + p.z*R[2][2]*R[2][2];
	this->tensor_.xy = p.x*R[0][0]*R[0][1] + p.y*R[1][0]*R[1][1] + p.z*R[2][0]*R[2][1];
	this->tensor_.xz = p.x*R[0][0]*R[0][2] + p.y*R[1][0]*R[1][2] + p.z*R[2][0]*R[2][2];
	this->tensor_.yz = p.x*R[0][1]*R[0][2] + p.y*R[1][1]*R[1][2] + p.z*R[2][1]*R[2][2];

	this->inv_tensor_.xx = ip.x*R[0][0]*R[0][0] + ip.y*R[1][0]*R[1][0] + ip.z*R[2][0]*R[2][0];
	this->inv_tensor_.yy = ip.x*R[0][1]*R[0][1] + ip.y*R[1][1]*R[1][1] + ip.z*R[2][1]*R[2][1];
	this->inv_tensor_.zz = ip.x*R[0][2]*R[0][2] + ip.y*R[1][2]*R[1][2] + ip.z*R[2][2]*R[2][2];
	this->inv_tensor_.xy = ip.x*R[0][0]*R[0][1] + ip.y*R[1][0]*R[1][1] + ip.z*R[2][0]*R[2][1];
	this->inv_tensor_.xz = ip.x*R[0][0]*R[0][2] + ip.y*R[1][0]*R[1][2] + ip.z*R[2][0]*R[2][2];
	this->inv_tensor_.yz = ip.x*R[0][1]*R[0][2] + ip.y*R[1][1]*R[1][2] + ip.z*R[2][1]*R[2][2];
}