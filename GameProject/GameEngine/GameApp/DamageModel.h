// Model.h:
// Created by: Aurelio Reis

#ifndef __AR__MODEL__H__
#define __AR__MODEL__H__

struct SVertex 
{
	D3DXVECTOR3		Pos;
	D3DXVECTOR3		Normal;
	D3DXVECTOR3		Tangent;
	D3DXVECTOR3		Binormal;
	D3DXVECTOR2		Texcoord;
	//D3DCOLOR		Color;
	float			BlendIndices[ 4 ];
	D3DXVECTOR4		BlendWeights;	
};

//////////////////////////////////////////////////////////////////////////
// STriangle
//////////////////////////////////////////////////////////////////////////

struct STriangle
{
	WORD m_Indices[ 3 ];
};


//////////////////////////////////////////////////////////////////////////
// CArGeometry
//////////////////////////////////////////////////////////////////////////

class CArGeometry
{
public:
	// CPU Data.
	SVertex					*m_pVertices;
	STriangle				*m_pTriangles;

	UINT32					m_uiNumVertices;
	UINT32					m_uiNumTriangles;

	// GPU Data.
	VertexFormatID			m_vf;
	VertexBufferID			m_vb;
	IndexBufferID			m_ib;
	ShaderID				m_shader;
	SamplerStateID			m_samplerState;
	

	CArGeometry() :
	m_pVertices( NULL ), m_pTriangles( NULL ),
		m_uiNumVertices( 0 ), m_uiNumTriangles( 0 ),
		m_vb( -1 ), m_ib( -1 ) {}

	~CArGeometry()
	{
		SAFE_DELETE( m_pVertices );
		SAFE_DELETE( m_pTriangles );
		
	}
};

//////////////////////////////////////////////////////////////////////////
// CArModelMesh
// Desc: Used for grouping model geometry in sub-groups.
//////////////////////////////////////////////////////////////////////////

class CArModelMesh
{
public:
	CArGeometry *m_pGeometry;

	// Whether the geometry is referenced and so should not be deleted.
	bool					m_bIsGeometryReferenced;

	CArModelMesh() : m_pGeometry( NULL ), m_bIsGeometryReferenced( false )
	{
	}

	~CArModelMesh()
	{
		if ( !m_bIsGeometryReferenced )
		{
			SAFE_DELETE( m_pGeometry );
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// CArBaseModel
// Desc: The lowest level model container.
//////////////////////////////////////////////////////////////////////////

struct SMD5Skeleton;
class CArDamageZone;
class CArAnimator;
struct SMD5Skeleton;

class CArBaseModel
{
public:
	std::vector< CArDamageZone * >	m_DamageZones;
	std::vector< CArModelMesh * >	m_Meshes;
	//IArModelFile				*m_pModelFile;
	CArAnimator					*m_pAnimator;
	const SMD5Skeleton*			m_pSkel;

	CArBaseModel();
	virtual ~CArBaseModel();

	virtual void Destroy();
	virtual void CreateBuffers();
	virtual void GenerateDamageSurfaces(std::vector< CArDamageZone * > &DamageZones );
	void		Render(class idRenderModelMD5* pEntity, class idAnimator* pAnimator);
	void Initialize(const SMD5Skeleton* skel);
	//virtual void Render( D3DXMATRIX WorldMatrix, CArAnimator &Animator );
	//virtual void DrawSkeleton( const CArAnimator &Animator );
	//virtual int FindJointIndex( const char *strJointName );
};

extern void CalcQuaternionW( D3DXQUATERNION &Q );


#endif