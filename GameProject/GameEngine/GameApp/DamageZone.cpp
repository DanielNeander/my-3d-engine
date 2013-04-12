#include "stdafx.h"
#pragma hdrstop

#include "DamageZone.h"
#include "DamageModel.h"

void CArDamageZone::GatherChildJoints( const SMD5Skeleton &Skel )
{
	// This probably shouldn't happen (a hitbox on the root bone)?
	if ( m_iAttachmentJoint == -1 )
	{
		return;
	}

	ChildJointList.push_back( m_iAttachmentJoint );
	AddUniqueJoint( ChildJointList, m_iAttachmentJoint );

	// Get all the child joints.
	for ( int i = m_iAttachmentJoint + 1; i < Skel.m_iNumJoints; ++i )
	{
		// If the joints parent is greater than the hitbox joint, it's a child.
		if ( Skel.m_pJoints[ i ].m_iParentIndex >= m_iAttachmentJoint )
		{
			ChildJointList.push_back( i );
			AddUniqueJoint( ChildJointList, i );
		}
		else
		{
			break;
		}
	}

	m_iJointRange = ChildJointList[ ChildJointList.size() - 1 ];
}

 void AddUniqueJoint( std::vector< int > &JointList, int iJoint )
{
	for ( UINT32 i = 0; i < JointList.size(); ++i )
	{
		if ( iJoint == JointList[ i ] )
		{
			return;
		}
	}

	JointList.push_back( iJoint );
}
