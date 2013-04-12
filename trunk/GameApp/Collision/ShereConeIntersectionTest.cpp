// Copyright ?2008 Intel Corporation
// All Rights Reserved
// 
// The sample source code contained or described herein and all documents
// related to the sample source code ("Material") are owned by Intel Corporation
// or its suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its suppliers
// and licensors, and is protected by worldwide copyright and trade secret laws and
// treaty provisions. The sample source code is provided AS IS, with no warranties
// of any kind, express or implied. Except as expressly permitted by the Software
// license, neither Intel Corporation nor its suppliers assumes any responsibility
// or liability for any errors or inaccuracies that may appear herein.

// the intersection function is based on Sphere Cone intersection test described in Geometric tools 
// by Philip J. Schneider and David H. Eberly, 
// The Morgan Kaufmann Series in Computer Graphics and Geometric Modeling
// http://www.geometrictools.com/Documentation/IntersectionSphereCone.pdf


#include "stdafx.h"
BOOL SphereConeTest ( D3DXVECTOR3 sphereCenter, float radius, float fov, D3DXVECTOR3 eyePt, D3DXVECTOR3 lookAt)
{
	float fSin = sinf(fov);
	float fCos = cosf(fov);
	float fInvSin = 1.0f/fSin;
	float fCosSqr = fCos*fCos;
	float fSinSqr = fSin*fSin;
	D3DXVECTOR3 vAxis = lookAt - eyePt;

	D3DXVECTOR3 vCmV =  sphereCenter - eyePt;
	D3DXVECTOR3 vD = vCmV + (radius*fInvSin)*vAxis;
	float fDSqrLen = D3DXVec3Dot(&vD, &vD);
	float fE = D3DXVec3Dot(&vD,&vAxis);
	if (fE > 0.0f && fE*fE >= fDSqrLen*fCosSqr)
	{
		fDSqrLen = D3DXVec3Dot(&vCmV, &vCmV);
		fE = -D3DXVec3Dot(&vCmV, &vAxis);
		if (fE > 0.0f && fE*fE >= fDSqrLen*fSinSqr)
		{
			float fRSqr = radius*radius;
			return fDSqrLen <= fRSqr;
		}
		return TRUE;
	}
	return FALSE;
}

