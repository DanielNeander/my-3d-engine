/************************************************************************
module	:	U2Camera
Author	:	Yun sangyong
Desc	:	2010-03-09 확정..
			default Dir		: -Z AXIS = (0, 0, -1)
			default UP		:  Y AXIS = (0, 1, 0) 
			default RIGHT	:  X AXIS = (1, 0, 0)
************************************************************************/
#pragma once
#ifndef U2_CAMERA_H
#define U2_CAMERA_H

#include <U2_3D/src/U2_3DLibType.h>
#include "U2Spatial.h"
#include "U2Frustum.h"
#include "U2Rect.h"

class U2_3D U2Camera : public U2Object
{

public:
	U2Camera();
	virtual ~U2Camera();

	// Direct3D Coordinate System
	// right : +x Axis 
	// up : +y Axis
	// dir : +z Axis

	void SetFrame(const D3DXVECTOR3& loc, const D3DXVECTOR3& dir, const D3DXVECTOR3& up, 
		const D3DXVECTOR3& right);
	void SetEye(const D3DXVECTOR3& loc);
	void SetAxes(const D3DXVECTOR3& dir, const D3DXVECTOR3& up, const D3DXVECTOR3& right);

	void SetFrustum (float fLeft, float fRight, float fBottom, float fTop,
		float fNear, float fFar, bool bOrtho = false);
	// Set a symmetric view frustum (umin = -umax, rmin = -rmax) using a field
	// of view in the "up" direction and an aspect ratio "width/height".  This
	// call is the equivalent of gluPerspective in OpenGL.  As such, the field
	// of view in this function must be specified in degrees and be in the
	// interval (0,180).
	void SetFrustum (float fUpFovDegrees, float fAspectRatio, float fDMin,
		float fDMax);


	// adjust the camera planes to contain the visible scene as tightly as possible
	// PSSMs
	void AdjustPlanes(U2VisibleSet& visibleObjs);

	// Get the parameters for a symmetric view frustum.  The return value is
	// 'true' iff the current frustum is symmetric, in which case the output
	// parameters are valid.
	bool GetFrustum (float& rfUpFovDegrees, float& rfAspectRatio,
		float& rfDMin, float& rfDMax) const;

	U2Frustum& GetFrustum();
	const U2Frustum& GetFrustum() const;


	// PSSMs Related Function Begin 

	// calculates split plane distances in view space
	void CalculateSplitPositions(float *pDistances);

	U2Frustum&  CalculateFrustum(float fNear, float fFar);



	// PSSMs Related Function End 

	void SetViewport (float fLeft, float fRight, float fTop, float fBottom);
	void GetViewport (float& rfLeft, float& rfRight, float& rfTop,
		float& rfBottom);

	// viewport depth range
	void SetDepthRange(float fNear, float fFar);
	void GetDepthRange(float& rfNear, float& rfFar);

	// Mouse picking support.  The (x,y) input point is in left-handed screen
	// coordinates (what you usually read from the windowing system).  The
	// function returns 'true' if and only if the input point is located in
	// the current viewport.  When 'true', the origin and direction values
	// are valid and are in world coordinates.  The direction vector is unit
	// length.
	bool GetPickRay (int iX, int iY, int iWidth, int iHeight,
		D3DXVECTOR3& rkOrigin, D3DXVECTOR3& rkDirection) const;

	// Camera Direction, Up, Right Vector
	inline D3DXVECTOR3					GetEye() const;
	inline D3DXVECTOR3					GetDir() const;
	inline D3DXVECTOR3					GetUp() const;
	inline D3DXVECTOR3					GetRight() const;

	inline const float*					GetWorldToViewMat() const;
	inline const float*					GetViewToWorldMat() const;


	// ViewFrustum Near/Far 경계 지정
	/*inline void							SetNearDist(float near);
	inline float						GetNearDist() const;
	inline void							SetFarDist(float far);
	inline float						GetFarDist() const;*/
		
	void								AdjustAspectRatio(float target, 
										bool bAdjustVerticalDims = false);

	float m_fFarMax;
	float m_fNearMin;

	int					GetNumSplits() const { return m_iNumSplits; }

protected:
	D3DXVECTOR3 m_vEye, m_vDir, m_vUp, m_vRight;
	D3DXQUATERNION m_quatOrient;

	U2Frustum m_frustum;

	// used when adjusting camera planes
	

	U2Rect<float> m_viewPort;

	float m_fPortNear, m_fPortFar;

	// PSSMs variables
	// Added 2010-05-11 
	// split scheme
	int m_iNumSplits;
	float m_fSplitSchemeWeight;	
		
};

typedef U2SmartPtr<U2Camera> U2CameraPtr;

#include "U2Camera.inl"

#endif