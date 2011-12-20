#include <U2_3D/src/U23DLibPCH.h>
#include "U2ContainBox3D.h"






bool InU2Box (const D3DXVECTOR3& rkPoint, const U2Box& rkBox)
{
	D3DXVECTOR3 kDiff = rkPoint - rkBox.m_vCenter;
	for (int i = 0; i < 3; i++)
	{
		float fCoeff = D3DXVec3Dot(&kDiff,&rkBox.m_avAxis[i]);
		if (U2Math::FAbs(fCoeff) > rkBox.m_afExtent[i])
		{
			return false;
		}
	}
	return true;
}



U2Box MergeU2Boxes(const U2Box& rkBox0, const U2Box& rkBox1)
{
	// construct a box that contains the input boxes
	U2Box kBox;

	// The first guess at the box m_vCenter.  This value will be updated later
	// after the input box vertices are projected onto axes determined by an
	// average of box axes.
	kBox.m_vCenter = 0.5f * (rkBox0.m_vCenter + rkBox1.m_vCenter);

	// A box's axes, when viewed as the columns of a matrix, form a rotation
	// matrix.  The input box axes are converted to quaternions.  The average
	// quaternion is computed, then normalized to unit length.  The result is
	// the slerp of the two input quaternions with t-value of 1/2.  The result
	// is converted back to a rotation matrix and its columns are selected as
	// the merged box axes.
	D3DXQUATERNION quat0, quat1;
	quat0 = U2Math::FromRotationMatrix(rkBox0.m_avAxis);
	quat1 = U2Math::FromRotationMatrix(rkBox1.m_avAxis);
	if (D3DXQuaternionDot(&quat0, &quat1) < 0.f)
	{
		quat1 = -quat1;
	}

	D3DXQUATERNION kQ = quat0 + quat1;
	float fInvLength = U2Math::InvSqrt(D3DXQuaternionDot(&kQ,&kQ));
	kQ = fInvLength*kQ;
	U2Math::ToRotationMatrix(kQ, kBox.m_avAxis);

	// Project the input box vertices onto the merged-box axes.  Each axis
	// D[i] containing the current m_vCenter C has a minimum projected value
	// pmin[i] and a maximum projected value pmax[i].  The corresponding end
	// points on the axes are C+pmin[i]*D[i] and C+pmax[i]*D[i].  The point C
	// is not necessarily the midpoint for any of the intervals.  The actual
	// box m_vCenter will be adjusted from C to a point C' that is the midpoint
	// of each interval,
	//   C' = C + sum_{i=0}^2 0.5*(pmin[i]+pmax[i])*D[i]
	// The box extents are
	//   e[i] = 0.5*(pmax[i]-pmin[i])

	int i, j;
	float fDot;
	D3DXVECTOR3 akVertex[8], kDiff;
	D3DXVECTOR3 kMin = VECTOR3_ZERO;
	D3DXVECTOR3 kMax = VECTOR3_ZERO;

	rkBox0.ComputeVertices(akVertex);
	for (i = 0; i < 8; i++)
	{
		kDiff = akVertex[i] - kBox.m_vCenter;
		for (j = 0; j < 3; j++)
		{
			fDot = D3DXVec3Dot(&kDiff,&kBox.m_avAxis[j]);
			if (fDot > kMax[j])
			{
				kMax[j] = fDot;
			}
			else if (fDot < kMin[j])
			{
				kMin[j] = fDot;
			}
		}
	}

	rkBox1.ComputeVertices(akVertex);
	for (i = 0; i < 8; i++)
	{
		kDiff = akVertex[i] - kBox.m_vCenter;
		for (j = 0; j < 3; j++)
		{
			fDot = D3DXVec3Dot(&kDiff, &kBox.m_avAxis[j]);
			if (fDot > kMax[j])
			{
				kMax[j] = fDot;
			}
			else if (fDot < kMin[j])
			{
				kMin[j] = fDot;
			}
		}
	}

	// [kMin,kMax] is the axis-aligned box in the coordinate system of the
	// merged box axes.  Update the current box m_vCenter to be the m_vCenter of
	// the new box.  Compute the extents based on the new m_vCenter.
	for (j = 0; j < 3; j++)
	{
		kBox.m_vCenter += 0.5f*(kMax[j]+kMin[j])*kBox.m_avAxis[j];
		kBox.m_afExtent[j] = 0.5f*(kMax[j]-kMin[j]);
	}

	return kBox;
}