/************************************************************************
module	:	U2GaussPointsFit3
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once
#ifndef U2_GAUSSPOINTSFIT3_H
#define U2_GAUSSPOINTSFIT3_H

// Wild Magic Source Code
// David Eberly
// http://www.geometrictools.com
// Copyright (c) 1998-2008
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or (at
// your option) any later version.  The license is available for reading at
// either of the locations:
//     http://www.gnu.org/copyleft/lgpl.html
//     http://www.geometrictools.com/License/WildMagicLicense.pdf
//
// Version: 4.0.0 (2006/06/28)


// Fit points with a Gaussian distribution.  The center is the mean of the
// points, the axes are the eigenvectors of the covariance matrix, and the
// extents are the eigenvalues of the covariance matrix and are returned in
// increasing order.  The quantites are stored in a Box3<Real> just to have a
// single container.
U2_3D U2Box GaussPointsFit3(uint16 usVertCnt, const D3DXVECTOR3* pPoints);


#endif
