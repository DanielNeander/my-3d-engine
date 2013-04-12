// BoundingVolume.h:
// Created by: Aurelio Reis

#ifndef __AR__BOUNDING__VOLUME__H__
#define __AR__BOUNDING__VOLUME__H__


//////////////////////////////////////////////////////////////////////////
// CArRay
//////////////////////////////////////////////////////////////////////////

class CArRay
{
public:
	D3DXVECTOR3 m_vOrigin;
	D3DXVECTOR3 m_vDirection;

	CArRay() {}
	~CArRay() {}
};


//////////////////////////////////////////////////////////////////////////
// CArIntersectionInfo
//////////////////////////////////////////////////////////////////////////

class CArIntersectionInfo
{
public:
	float m_fDistance;
	D3DXVECTOR3 m_vIntersectionPt;
};


//////////////////////////////////////////////////////////////////////////
// CArSphere
//////////////////////////////////////////////////////////////////////////

class CArSphere
{
public:
	D3DXVECTOR3 m_vOrigin;
	float		m_fRadius;
	float		m_fRadiusSquared;

	CArSphere( D3DXVECTOR3 &vOrigin, float fRadius ) : m_vOrigin( vOrigin ), m_fRadius( fRadius )
	{
		m_fRadiusSquared = m_fRadius * m_fRadius;
	}

	~CArSphere() {}

	bool IntersectRay( const CArRay &Ray, CArIntersectionInfo *pIntersection = NULL )
	{
		const D3DXVECTOR3 &c = m_vOrigin;
		float r2 = m_fRadiusSquared;

		const D3DXVECTOR3 &o = Ray.m_vOrigin;
		const D3DXVECTOR3 &d = Ray.m_vDirection;

		D3DXVECTOR3 l = c - o;
		float s = D3DXVec3Dot( &l, &d );	// angle between vector to sphere and ray direction
		float l2 = D3DXVec3Dot( &l, &l );	// distance between center of sphere and ray origin

		if ( s < 0.0f && l2 > r2 ) 
		{
			return false;
		}

		float s2 = s * s;
		float m2 = l2 - s2;

		if ( m2 > r2 )
		{
			return false;
		}

		if ( pIntersection )
		{
			float q = sqrtf( r2 - m2 );

			float t;
			if ( l2 > r2 )
			{
				t = s - q;
			}
			else
			{
				t = s + q;
			}

			pIntersection->m_fDistance = t;
			pIntersection->m_vIntersectionPt = o + t * d;
		}

		return true;
	}
};


//////////////////////////////////////////////////////////////////////////
// CArAabb
//////////////////////////////////////////////////////////////////////////

class CArAabb
{
public:
	D3DXVECTOR3 m_vMin, m_vMax;

	CArAabb() {}
	~CArAabb() {}

	float CalculateRadius() const
	{
		float fRad = fabs( m_vMin.x );

		if ( fabs( m_vMin.y ) > fRad ) { fRad = fabs( m_vMin.y ); }
		if ( fabs( m_vMin.z ) > fRad ) { fRad = fabs( m_vMin.z ); }

		if ( fabs( m_vMax.x ) > fRad ) { fRad = fabs( m_vMax.x ); }
		if ( fabs( m_vMax.y ) > fRad ) { fRad = fabs( m_vMax.y ); }
		if ( fabs( m_vMax.z ) > fRad ) { fRad = fabs( m_vMax.z ); }

		return fRad;
	}
};


//////////////////////////////////////////////////////////////////////////
// CArHitBox
//////////////////////////////////////////////////////////////////////////

#define INVALID_JOINT	-1

class CArHitBox
{
public:
	int m_iAttachmentJoint;
	CArAabb m_Box;

	CArHitBox() : m_iAttachmentJoint( INVALID_JOINT ) {}
	~CArHitBox() {}
};


//////////////////////////////////////////////////////////////////////////
// CArDamageSurface
//////////////////////////////////////////////////////////////////////////

class CArDamageSurface
{
public:
	UINT32 m_uiStartVert;
	UINT32 m_uiStartIndex;
	UINT32 m_uiNumVerts;
	UINT32 m_uiNumTris;
};


//////////////////////////////////////////////////////////////////////////
// CArDamageZone
//////////////////////////////////////////////////////////////////////////

// Forward decls.
struct SMD5Skeleton;

class CArDamageZone : public CArHitBox
{
public:
	CArDamageSurface	m_Surface;

	// The next damage zone below this one in the skeletal hierarchy.
	CArDamageZone	*m_Next;

	// The joints below this damage zone's joint
	vector< int >	ChildJointList;

	// The largest joint index in the limb hierarchy.
	int				m_iJointRange;

	CArDamageZone() : m_Next( NULL ), m_iJointRange( INVALID_JOINT ) {}
	~CArDamageZone() {}

	void GatherChildJoints( const SMD5Skeleton &Skel );
};


extern void DrawAabbWireframe( const CArAabb &Box, const D3DXMATRIXA16 *pTransform, UINT Color );
extern void DrawAabb( const CArAabb &Box, const D3DXMATRIXA16 *pTransform, UINT Color );

#endif // __AR__BOUNDING__VOLUME__H__
