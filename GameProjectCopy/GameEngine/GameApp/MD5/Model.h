// Model.h:
// Created by: Aurelio Reis

#ifndef __AR__MODEL__H__
#define __AR__MODEL__H__


//////////////////////////////////////////////////////////////////////////
// SVertex
//////////////////////////////////////////////////////////////////////////

struct SVertex
{
	D3DXVECTOR3		Pos;
	D3DXVECTOR2		Texcoord;
	D3DCOLOR		Color;
	byte			BlendIndices[ 4 ];
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
	IDirect3DVertexBuffer9	*m_pVB;
	IDirect3DIndexBuffer9	*m_pIB;

	CArGeometry() :
		m_pVertices( NULL ), m_pTriangles( NULL ),
		m_uiNumVertices( 0 ), m_uiNumTriangles( 0 ),
		m_pVB( NULL ), m_pIB( NULL ) {}

	~CArGeometry()
	{
		SAFE_DELETE( m_pVertices );
		SAFE_DELETE( m_pTriangles );

		SAFE_RELEASE( m_pVB );
		SAFE_RELEASE( m_pIB );
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

// Forward decls.
class IArModelFile;
class CArAnimator;
class CArMD5Anim;
struct SMD5Skeleton;
class CArDamageZone;

class CArBaseModel
{
public:
	vector< CArDamageZone * >	m_DamageZones;
	vector< CArModelMesh * >	m_Meshes;
	IArModelFile				*m_pModelFile;

	CArBaseModel();
	virtual ~CArBaseModel();

	virtual void Destroy();
	virtual void CreateBuffers();
	virtual void GenerateDamageSurfaces( vector< CArDamageZone * > &DamageZones );
	virtual void Render( D3DXMATRIX WorldMatrix, CArAnimator &Animator );
	virtual void DrawSkeleton( const CArAnimator &Animator );
	virtual int FindJointIndex( const char *strJointName );
};


//////////////////////////////////////////////////////////////////////////
// CArModelInstance
// Desc: This kind of model is used by entities to contain unique or referenced
// geometry (i.e. from a model file).
//////////////////////////////////////////////////////////////////////////

class CArModelInstance : public CArBaseModel
{
public:
	CArModelInstance() {}
	~CArModelInstance() {}

	virtual void InitializeFromModelFile( IArModelFile *pModelFile );
};


//////////////////////////////////////////////////////////////////////////
// IArModelFile
// Desc: Contains raw model data to be loaded into a usable model container.
//////////////////////////////////////////////////////////////////////////

class IArModelFile
{
public:
	IArModelFile() {}
	virtual ~IArModelFile() = 0 {}

	virtual void CreateFromFile( const char *strFileName ) = 0;
	virtual void Destroy() = 0;
	virtual void GenerateBaseModelData( CArBaseModel *pBaseModel ) = 0;
	virtual int FindJointIndex( const char *strJointName ) = 0;
	virtual SMD5Skeleton &GetSkeleton() = 0;
	virtual CArBaseModel *GetBaseModel() = 0;
};


#endif // __AR__MODEL__H__
