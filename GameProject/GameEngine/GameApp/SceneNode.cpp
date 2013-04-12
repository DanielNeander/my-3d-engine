#include "stdafx.h"
#include "SceneNode.h"

void SceneNode::AddChild( SceneNode* pChild )
{
	pChild->parent_ = this;
	childNodes_.push_back(pChild);	
}

SceneNode* SceneNode::NewNode()
{
	return new SceneNode;
}

void SceneNode::Update( float fDelta )
{	
	PreUpdate();
	// PreUpdate
	UpdateDownward();

	if (parent_)
		parent_->UpdateUpward();

	// PostUpdate
	PostUpdate();
}

SceneNode::SceneNode() 	:parent_(NULL), visible_(true), IsInheritTM(true)
{
	localTM_.MakeIdentity();
	worldTM_.MakeIdentity();
}

void SceneNode::UpdateTM()
{

	if (parent_ && IsInheritTM)
	{			
		worldTM_ = parent_-> worldTM_ * localTM_ ;								
	}
	else 
	{					
		worldTM_ = localTM_;	
	}
	localTM_.isDirty = false;
	
	// Update Collision Object

}


void SceneNode::SetDir(const noVec3& vec, TransformSpace relativeTo, 
	const noVec3& localDirectionVector)
{
	if (vec == vec3_zero) return;

	noVec3 targetDir = vec;
	targetDir.Normalize();

	switch (relativeTo)
	{
	case TS_PARENT:
	/*	if (mInheritOrientation)
		{
			if (mParent)
			{
				targetDir = mParent->_getDerivedOrientation() * targetDir;
			}
		}*/
		break;
	case TS_LOCAL:
		targetDir = worldTM_.rotate_ * targetDir;
		break;
	case TS_WORLD:
		// default orientation
		break;
	}

	noQuat currentOrient = worldTM_.rotate_.ToQuat();
	noQuat targetOrientation;

	// Get current local direction relative to world space
	noVec3 currDir = currentOrient * localDirectionVector;

	if ((currDir + targetDir).LengthSqr() < 0.00005f)
	{
		// Oops, a 180 degree turn (infinite possible rotation axes)
		// Default to yaw i.e. use current UP
		targetOrientation =
			noQuat(-currentOrient.z, currentOrient.w, currentOrient.x, -currentOrient.y);
	}
	else 
	{
		noQuat rotQuat = currDir.getRotationTo(targetDir);
		targetOrientation = rotQuat * currentOrient;
	}
	targetOrientation.Normalize();
	localTM_.rotate_ = targetOrientation.ToMat3();
	UpdateTM();
}

SceneNode::~SceneNode()
{
	for (size_t i=0; i < childNodes_.size(); ++i)
		SAFE_DELETE(childNodes_[i]);

	childNodes_.clear();
}

void SceneNode::setVisible( bool bVisible )
{
	visible_ = bVisible;

	for (size_t i=0; i < childNodes_.size(); ++i)
		childNodes_.at(i)->setVisible(bVisible);
}

void SceneNode::UpdateDownward()
{
	UpdateData();
	UpdateTM();

	for (size_t i=0; i < childNodes_.size(); ++i)
		childNodes_[i]->UpdateDownward();
}

void SceneNode::UpdateUpward()
{
	// UdateWorldBound
	if (parent_)
		parent_->UpdateUpward();
}

SceneNode* SceneNode::GetChildByName( const char* name ) const
{		
	for (size_t i=0; i < childNodes_.size(); ++i)
	{		
		 if (childNodes_.at(i)->name_ == name)
			 return childNodes_.at(i);		 
		 else 
			 if (childNodes_.at(i)->childNodes_.size() > 0)
				 childNodes_.at(i)->GetChildByName(name); 
	}
	return NULL;
}
