#pragma once 


#define INVALID_JOINT	-1

class CArHitBox
{
public:
	int m_iAttachmentJoint;
	BoundingBox	m_Box;
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
	std::vector< int >	ChildJointList;

	// The largest joint index in the limb hierarchy.
	int				m_iJointRange;

	CArDamageZone() : m_Next( NULL ), m_iJointRange( INVALID_JOINT ) {}
	~CArDamageZone() {}

	void GatherChildJoints( const SMD5Skeleton &Skel );
};



extern void DrawAxis( float fAxisLen, const noMat4 &Transform, bool bDrawAlways = false );
extern void AddUniqueJoint( std::vector< int > &JointList, int iJoint );

//extern void DrawAabbWireframe( const BoundingBox &Box, const noMat4 *pTransform, UINT Color );
//extern void DrawAabb( const BoundingBox &Box, const noMat4 *pTransform, UINT Color );
