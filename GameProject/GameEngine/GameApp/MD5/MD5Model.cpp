#include "stdafx.h"
#include "EngineCore/Util/GameLog.h"
#include "Token.h"
#include "Lexer.h"
#include "Parser.h"
#include "MD5Model.h"
#include "MD5Triangle.h"

/***********************************************************************

	idMD5Mesh

***********************************************************************/

static int c_numVerts = 0;
static int c_numWeights = 0;
static int c_numWeightJoints = 0;

typedef struct vertexWeight_s {
	int							vert;
	int							joint;
	noVec3						offset;
	float						jointWeight;
} vertexWeight_t;


idMD5Mesh::idMD5Mesh()
{
	scaledWeights	= NULL;
	weightIndex		= NULL;
	//shader			= NULL;
	numTris			= 0;
	deformInfo		= NULL;
	surfaceNum		= 0;
}

idMD5Mesh::~idMD5Mesh()
{
	Mem_Free16( scaledWeights );
	Mem_Free16( weightIndex );
	/*if ( deformInfo ) {
		R_FreeDeformInfo( deformInfo );
		deformInfo = NULL;
	}*/
}

void idMD5Mesh::ParseMesh( idLexer &parser, int numJoints, const idJointMat *joints, SMD5Mesh* pCurMesh )
{
	idToken		token;
	idToken		name;
	int			num;
	int			count;
	int			jointnum;
	idStr		shaderName;
	int			i, j;
	idList<int>	tris;
	idList<int>	firstWeightForVertex;
	idList<int>	numWeightsForVertex;
	int			maxweight;
	idList<vertexWeight_t> tempWeights;

	parser.ExpectTokenString( "{" );

	//
	// parse name
	//
	if ( parser.CheckTokenString( "name" ) ) {
		parser.ReadToken( &name );
	}

	//
	// parse shader
	//
	parser.ExpectTokenString( "shader" );

	parser.ReadToken( &token );
	shaderName = token;

	shader = declManager->FindMaterial( shaderName );

	//
	// parse texture coordinates
	//
	parser.ExpectTokenString( "numverts" );
	count = parser.ParseInt();
	if ( count < 0 ) {
		parser.Error( "Invalid size: %s", token.c_str() );
	}

	texCoords.SetNum( count );
	firstWeightForVertex.SetNum( count );
	numWeightsForVertex.SetNum( count );

	pCurMesh->m_iNumVertices = count;
	pCurMesh->m_pVertices = new SMD5Vertex[ pCurMesh->m_iNumVertices ];	

	numWeights = 0;
	maxweight = 0;
	for( i = 0; i < texCoords.Num(); i++ ) {
		parser.ExpectTokenString( "vert" );
		parser.ParseInt();

		parser.Parse1DMatrix( 2, texCoords[ i ].ToFloatPtr() );

		firstWeightForVertex[ i ]	= parser.ParseInt();
		numWeightsForVertex[ i ]	= parser.ParseInt();

		if ( !numWeightsForVertex[ i ] ) {
			parser.Error( "Vertex without any joint weights." );
		}

		numWeights += numWeightsForVertex[ i ];
		if ( numWeightsForVertex[ i ] + firstWeightForVertex[ i ] > maxweight ) {
			maxweight = numWeightsForVertex[ i ] + firstWeightForVertex[ i ];
		}
		pCurMesh->m_pVertices[ i ].m_ST[ 0 ] = texCoords[i].x;
		pCurMesh->m_pVertices[ i ].m_ST[ 1 ] = texCoords[i].y;
		pCurMesh->m_pVertices[ i ].m_iStartWeight = firstWeightForVertex[ i ];
		pCurMesh->m_pVertices[ i ].m_iWeightCount = numWeightsForVertex[ i ];
	}

	//
	// parse tris
	//
	parser.ExpectTokenString( "numtris" );
	count = parser.ParseInt();
	if ( count < 0 ) {
		parser.Error( "Invalid size: %d", count );
	}
	pCurMesh->m_iNumTriangles = count;
	pCurMesh->m_pTriangles = new SMD5Triangle[ pCurMesh->m_iNumTriangles ];
	
	tris.SetNum( count * 3 );
	numTris = count;
	for( i = 0; i < count; i++ ) {
		parser.ExpectTokenString( "tri" );
		parser.ParseInt();

		tris[ i * 3 + 0 ] = parser.ParseInt();
		tris[ i * 3 + 1 ] = parser.ParseInt();
		tris[ i * 3 + 2 ] = parser.ParseInt();

		pCurMesh->m_pTriangles[ i ].m_Indices[ 0 ] =tris[ i * 3 + 0 ];
		pCurMesh->m_pTriangles[ i ].m_Indices[ 1 ] =tris[ i * 3 + 1 ];
		pCurMesh->m_pTriangles[ i ].m_Indices[ 2 ] =tris[ i * 3 + 2 ];
	}

	//
	// parse weights
	//
	parser.ExpectTokenString( "numweights" );
	count = parser.ParseInt();
	if ( count < 0 ) {
		parser.Error( "Invalid size: %d", count );
	}

	if ( maxweight > count ) {
		parser.Warning( "Vertices reference out of range weights in model (%d of %d weights).", maxweight, count );
	}

	pCurMesh->m_iNumWeights = count;
	pCurMesh->m_pWeights = new SMD5Weight[ pCurMesh->m_iNumWeights ];

	tempWeights.SetNum( count );

	for( i = 0; i < count; i++ ) {
		parser.ExpectTokenString( "weight" );
		parser.ParseInt();

		jointnum = parser.ParseInt();
		if ( ( jointnum < 0 ) || ( jointnum >= numJoints ) ) {
			parser.Error( "Joint Index out of range(%d): %d", numJoints, jointnum );
		}

		tempWeights[ i ].joint			= jointnum;
		tempWeights[ i ].jointWeight	= parser.ParseFloat();		
		// bone translation
		parser.Parse1DMatrix( 3, tempWeights[ i ].offset.ToFloatPtr() );

		pCurMesh->m_pWeights[ i ].m_iJointId = jointnum;
		pCurMesh->m_pWeights[ i ].m_fBias = tempWeights[ i ].jointWeight;		// 
		pCurMesh->m_pWeights[ i ].m_Position[ 0 ] = tempWeights[ i ].offset[0];
		pCurMesh->m_pWeights[ i ].m_Position[ 1 ] = tempWeights[ i ].offset[1];
		pCurMesh->m_pWeights[ i ].m_Position[ 2 ] = tempWeights[ i ].offset[2];
	}	

	// create pre-scaled weights and an index for the vertex/joint lookup
	scaledWeights = (noVec4 *) Mem_Alloc16( numWeights * sizeof( scaledWeights[0] ) );
	weightIndex = (int *) Mem_Alloc16( numWeights * 2 * sizeof( weightIndex[0] ) );
	memset( weightIndex, 0, numWeights * 2 * sizeof( weightIndex[0] ) );

	count = 0;
	for( i = 0; i < texCoords.Num(); i++ ) {
		num = firstWeightForVertex[i];
		for( j = 0; j < numWeightsForVertex[i]; j++, num++, count++ ) {
			scaledWeights[count].ToVec3() = tempWeights[num].offset * tempWeights[num].jointWeight;
			scaledWeights[count].w = tempWeights[num].jointWeight;
			weightIndex[count * 2 + 0] = tempWeights[num].joint * sizeof( idJointMat );
		}
		weightIndex[count * 2 - 1] = 1;
	}

	tempWeights.Clear();
	numWeightsForVertex.Clear();
	firstWeightForVertex.Clear();

	parser.ExpectTokenString( "}" );

	// update counters
	c_numVerts += texCoords.Num();
	c_numWeights += numWeights;
	c_numWeightJoints++;
	for ( i = 0; i < numWeights; i++ ) {
		c_numWeightJoints += weightIndex[i*2+1];
	}

	//
	// build the information that will be common to all animations of this mesh:
	// silhouette edge connectivity and normal / tangent generation information
	//
	idDrawVert *verts = (idDrawVert *) _alloca16( texCoords.Num() * sizeof( idDrawVert ) );
	for ( i = 0; i < texCoords.Num(); i++ ) {
		verts[i].Clear();
		verts[i].st = texCoords[i];
	}
	TransformVerts( verts, joints );			
	//deformInfo = R_BuildDeformInfo( texCoords.Num(), verts, tris.Num(), tris.Ptr(), shader->UseUnsmoothedTangents() );

}

//void idMD5Mesh::UpdateSurface( const struct renderEntity_s *ent, const idJointMat *joints, modelSurface_t *surf )
//{
//
//}

idBounds idMD5Mesh::CalcBounds( const idJointMat *entJoints )
{
	idBounds	bounds;
	idDrawVert *verts = (idDrawVert *) _alloca16( texCoords.Num() * sizeof( idDrawVert ) );

	TransformVerts( verts, entJoints );

	SIMDProcessor->MinMax( bounds[0], bounds[1], verts, texCoords.Num() );

	return bounds;
}

int idMD5Mesh::NearestJoint( int a, int b, int c ) const
{
	int i, bestJoint, vertNum, weightVertNum;
	float bestWeight;

	// duplicated vertices might not have weights
	if ( a >= 0 && a < texCoords.Num() ) {
		vertNum = a;
	} else if ( b >= 0 && b < texCoords.Num() ) {
		vertNum = b;
	} else if ( c >= 0 && c < texCoords.Num() ) {
		vertNum = c;
	} else {
		// all vertices are duplicates which shouldn't happen
		return 0;
	}

	// find the first weight for this vertex
	weightVertNum = 0;
	for( i = 0; weightVertNum < vertNum; i++ ) {
		weightVertNum += weightIndex[i*2+1];
	}

	// get the joint for the largest weight
	bestWeight = scaledWeights[i].w;
	bestJoint = weightIndex[i*2+0] / sizeof( idJointMat );
	for( ; weightIndex[i*2+1] == 0; i++ ) {
		if ( scaledWeights[i].w > bestWeight ) {
			bestWeight = scaledWeights[i].w;
			bestJoint = weightIndex[i*2+0] / sizeof( idJointMat );
		}
	}
	return bestJoint;
}

int idMD5Mesh::NumVerts( void ) const
{
	return texCoords.Num();

}

int idMD5Mesh::NumTris( void ) const
{
	return numTris;

}

int idMD5Mesh::NumWeights( void ) const
{
	return numWeights;

}

void idMD5Mesh::TransformVerts( idDrawVert *verts, const idJointMat *entJoints )
{
	SIMDProcessor->TransformVerts( verts, texCoords.Num(), entJoints, scaledWeights, weightIndex, numWeights );

}

void idMD5Mesh::TransformScaledVerts( idDrawVert *verts, const idJointMat *entJoints, float scale )
{
	noVec4 *scaledWeights = (noVec4 *) _alloca16( numWeights * sizeof( scaledWeights[0] ) );
	SIMDProcessor->Mul( scaledWeights[0].ToFloatPtr(), scale, scaledWeights[0].ToFloatPtr(), numWeights * 4 );
	SIMDProcessor->TransformVerts( verts, texCoords.Num(), entJoints, scaledWeights, weightIndex, numWeights );
}

idRenderModelStatic::idRenderModelStatic()
{
	purged = false;
}

idRenderModelStatic::~idRenderModelStatic()
{

}

/*
================
idRenderModelStatic::Bounds
================
*/
idBounds idRenderModelStatic::Bounds( const struct renderEntity_s *mdef ) const {
	return bounds;
}

void idRenderModelStatic::LoadModel()
{

}

bool idRenderModelStatic::IsLoaded()
{
	return !purged;

}

/*================
	idRenderModelStatic::NumJoints
	================
*/
int idRenderModelStatic::NumJoints( void ) const {
	return 0;
}

/*
================
idRenderModelStatic::GetJoints
================
*/
const idMD5Joint *idRenderModelStatic::GetJoints( void ) const {
	return NULL;
}

/*
================
idRenderModelStatic::GetJointHandle
================
*/
jointHandle_t idRenderModelStatic::GetJointHandle( const char *name ) const {
	return INVALID_JOINT;
}

/*
================
idRenderModelStatic::GetJointName
================
*/
const char * idRenderModelStatic::GetJointName( jointHandle_t handle ) const {
	return "";
}

/*
================
idRenderModelStatic::GetDefaultPose
================
*/
const idJointQuat *idRenderModelStatic::GetDefaultPose( void ) const {
	return NULL;
}

/*
================
idRenderModelStatic::NearestJoint
================
*/
int idRenderModelStatic::NearestJoint( int surfaceNum, int a, int b, int c ) const {
	return INVALID_JOINT;
}

const char * idRenderModelStatic::Name() const
{
	return name;
}

/*
====================
idRenderModelMD5::LoadModel

used for initial loads, reloadModel, and reloading the data of purged models
Upon exit, the model will absolutely be valid, but possibly as a default model
====================
*/
void idRenderModelMD5::LoadModel()
{
	int			version;
	int			i;
	int			num;
	int			parentNum;
	idToken		token;
	idLexer		parser( LEXFL_ALLOWPATHNAMES | LEXFL_NOSTRINGESCAPECHARS );
	idJointQuat	*pose;
	idMD5Joint	*joint;
	idJointMat *poseMat3;

	if ( !purged ) {
		PurgeModel();
	}

	purged = false;

	name.DefaultPath("Doom3/");

	if ( !parser.LoadFile( name ) ) {
		//MakeDefaultModel();
		return;
	}
	parser.ExpectTokenString( MD5_VERSION_STRING );
	version = parser.ParseInt();

	if ( version != MD5_VERSION ) {
		parser.Error( "Invalid version %d.  Should be version %d\n", version, MD5_VERSION );
	}

	//
	// skip commandline
	//
	parser.ExpectTokenString( "commandline" );
	parser.ReadToken( &token );

	// parse num joints
	parser.ExpectTokenString( "numJoints" );
	num  = parser.ParseInt();
	joints.SetGranularity( 1 );
	joints.SetNum( num );
	defaultPose.SetGranularity( 1 );
	defaultPose.SetNum( num );
	poseMat3 = ( idJointMat * )_alloca16( num * sizeof( *poseMat3 ) );

	m_SkelData.m_iNumJoints = num;
	m_SkelData.m_pJoints = new SMD5Joint[ m_SkelData.m_iNumJoints];

	// parse num meshes
	parser.ExpectTokenString( "numMeshes" );
	num = parser.ParseInt();
	if ( num < 0 ) {
		parser.Error( "Invalid size: %d", num );
	}
	meshes.SetGranularity( 1 );
	meshes.SetNum( num );

	m_ModelData.m_iNumMeshes = num;
	m_ModelData.m_pMeshes = new SMD5Mesh[ m_ModelData.m_iNumMeshes ];
	//
	// parse joints
	//
	parser.ExpectTokenString( "joints" );
	parser.ExpectTokenString( "{" );
	pose = defaultPose.Ptr();
	joint = joints.Ptr();
	for( i = 0; i < joints.Num(); i++, joint++, pose++ ) {
		ParseJoint( parser, joint, pose );
		poseMat3[ i ].SetRotation( pose->q.ToMat3() );
		poseMat3[ i ].SetTranslation( pose->t );
		D3DXQUATERNION poseQ(pose->q.ToFloatPtr());
		D3DXVECTOR3 poseT(pose->t.ToFloatPtr());
		
		parentNum = -1;
		if ( joint->parent ) {
			parentNum = joint->parent - joints.Ptr();
			pose->q = ( poseMat3[ i ].ToMat3() * poseMat3[ parentNum ].ToMat3().Transpose() ).ToQuat();
			pose->t = ( poseMat3[ i ].ToVec3() - poseMat3[ parentNum ].ToVec3() ) * poseMat3[ parentNum ].ToMat3().Transpose();
		}
		SMD5Joint &Joint = m_SkelData.m_pJoints[ i ];
		Joint.m_strName = joint->name;
		Joint.m_iParentIndex = parentNum;
		D3DXQUATERNION Q = poseQ;		
		// Calculate implicit w.
		CalcQuaternionW( Q );
		Joint.m_Orientation = Q;
		D3DXMATRIXA16 mOri;
		D3DXMatrixRotationQuaternion( &mOri, &Q );		
		Joint.m_Position = poseT;		
		// Set origin.
		mOri._41 = Joint.m_Position.x;
		mOri._42 = Joint.m_Position.y;
		mOri._43 = Joint.m_Position.z;

		if ( D3DXMatrixInverse( &Joint.m_InverseBindPoseMatrix, NULL, &mOri ) == NULL )
		{
			assert( 0 );
		}

	}

	


	parser.ExpectTokenString( "}" );

	for( i = 0; i < meshes.Num(); i++ ) {
		parser.ExpectTokenString( "mesh" );
		SMD5Mesh *pCurMesh = &m_ModelData.m_pMeshes[i];

		meshes[ i ].ParseMesh( parser, defaultPose.Num(), poseMat3, pCurMesh );
	}
			
	m_pBaseModel = new CArBaseModel();
	GenerateBaseModelData( m_pBaseModel );

	//
	// calculate the bounds of the model
	//
	CalculateBounds( poseMat3 );
}

/*
===================
idRenderModelMD5::PurgeModel

frees all the data, but leaves the class around for dangling references,
which can regenerate the data with LoadModel()
===================
*/
void idRenderModelMD5::PurgeModel() {
	purged = true;
	joints.Clear();
	defaultPose.Clear();
	meshes.Clear();
	SAFE_DELETE( m_pBaseModel );
}


int idRenderModelMD5::NumJoints( void ) const
{
	return joints.Num();

}

const idMD5Joint * idRenderModelMD5::GetJoints( void ) const
{
	return joints.Ptr();

}

jointHandle_t idRenderModelMD5::GetJointHandle( const char *name ) const
{
	const idMD5Joint *joint;
	int	i;

	joint = joints.Ptr();
	for( i = 0; i < joints.Num(); i++, joint++ ) {
		if ( idStr::Icmp( joint->name.c_str(), name ) == 0 ) {
			return ( jointHandle_t )i;
		}
	}

	return INVALID_JOINT;
}

const char * idRenderModelMD5::GetJointName( jointHandle_t handle ) const
{
	if ( ( handle < 0 ) || ( handle >= joints.Num() ) ) {
		return "<invalid joint>";
	}

	return joints[ handle ].name;
}

const idJointQuat * idRenderModelMD5::GetDefaultPose( void ) const
{
	return defaultPose.Ptr();

}

int idRenderModelMD5::NearestJoint( int surfaceNum, int a, int b, int c ) const
{
	int i;
	const idMD5Mesh *mesh;

	if ( surfaceNum > meshes.Num() ) {
		//common->Error( "idRenderModelMD5::NearestJoint: surfaceNum > meshes.Num()" );
	}

	for ( mesh = meshes.Ptr(), i = 0; i < meshes.Num(); i++, mesh++ ) {
		if ( mesh->surfaceNum == surfaceNum ) {
			return mesh->NearestJoint( a, b, c );
		}
	}
	return 0;
}

void idRenderModelMD5::CalculateBounds( const idJointMat *entJoints )
{
	int			i;
	idMD5Mesh	*mesh;

	bounds.Clear();
	for( mesh = meshes.Ptr(), i = 0; i < meshes.Num(); i++, mesh++ ) {
		bounds.AddBounds( mesh->CalcBounds( entJoints ) );
	}
}


/*
====================
idRenderModelMD5::InitFromFile
====================
*/
void idRenderModelMD5::InitFromFile( const char *fileName ) {
	name = fileName;
	LoadModel();
}


void idRenderModelMD5::ParseJoint( idLexer &parser, idMD5Joint *joint, idJointQuat *defaultPose )
{
	idToken	token;
	int		num;

	//
	// parse name
	//
	parser.ReadToken( &token );
	joint->name = token;

	//
	// parse parent
	//
	num = parser.ParseInt();
	if ( num < 0 ) {
		joint->parent = NULL;
	} else {
		if ( num >= joints.Num() - 1 ) {
			parser.Error( "Invalid parent for joint '%s'", joint->name.c_str() );
		}
		joint->parent = &joints[ num ];
	}

	//
	// parse default pose
	//
	parser.Parse1DMatrix( 3, defaultPose->t.ToFloatPtr() );
	parser.Parse1DMatrix( 3, defaultPose->q.ToFloatPtr() );
	defaultPose->q.w = defaultPose->q.CalcW();
}


void idRenderModelMD5::GenerateBaseModelData( CArBaseModel *pBaseModel )
{
	m_pBaseModel->Initialize(&m_SkelData);
	//////////////////////////////////////////////////////////////////////////
	// Generate model data.
	//////////////////////////////////////////////////////////////////////////			
	for ( int i = 0; i < m_ModelData.m_iNumMeshes; ++i)
	{
		CArModelMesh *pMesh = new CArModelMesh();
		pBaseModel->m_Meshes.push_back( pMesh );
		
		SMD5Mesh &Md5Mesh = m_ModelData.m_pMeshes[ i ];

		
		
		pMesh->m_pGeometry = new CArGeometry();

		pMesh->m_pGeometry->m_uiNumVertices = Md5Mesh.m_iNumVertices;
		pMesh->m_pGeometry->m_uiNumTriangles = Md5Mesh.m_iNumTriangles;

		//////////////////////////////////////////////////////////////////////////
		// Generate vertices.
		//////////////////////////////////////////////////////////////////////////

		pMesh->m_pGeometry->m_pVertices = new SVertex[ Md5Mesh.m_iNumVertices ];

		SVertex *pVtx = pMesh->m_pGeometry->m_pVertices;

		int count = 0;		
		for ( int k = 0; k < Md5Mesh.m_iNumVertices; ++k )
		{
			SMD5Vertex &Vert = Md5Mesh.m_pVertices[ k ];

			//////////////////////////////////////////////////////////////////////////
			// We only support 4 weights per-vertex so find the 4 highest influences
			// and renormalize.
			//////////////////////////////////////////////////////////////////////////

			memset( pVtx[ k ].BlendWeights, 0, sizeof( pVtx[ k ].BlendWeights ) );
			memset( pVtx[ k ].BlendIndices, 0, sizeof( pVtx[ k ].BlendIndices ) );

			const int MAX_WEIGHTS = 4;

					
			// Fill in the initial influences.
			int j = 0;
			for ( ; j < MAX_WEIGHTS && j < Vert.m_iWeightCount; ++j, count++)
			{
				const SMD5Weight &Weight = Md5Mesh.m_pWeights[ Vert.m_iStartWeight + j ];				
				
				pVtx[ k ].BlendWeights[ j ] = Weight.m_fBias;
				pVtx[ k ].BlendIndices[ j ] = Weight.m_iJointId;
			}

			// Overwrite the smallest value with one of the remaining weights.
			for ( ; j < Vert.m_iWeightCount; ++j )
			{
				const SMD5Weight &Weight = Md5Mesh.m_pWeights[ Vert.m_iStartWeight + j ];				

				float fSmallest = pVtx[ k ].BlendWeights[ 0 ];
				int iSmallest = 0;
#if 0
				for ( int l = 1; l < MAX_WEIGHTS-1; ++l )
				{
					if ( pVtx[ k ].BlendWeights[ l ] < fSmallest )
					{				
						pVtx[ k ].BlendWeights[ l ] = fSmallest;
					}
				}		
#endif
				for ( int l = 1; l < MAX_WEIGHTS; ++l )
				{
					if ( pVtx[ k ].BlendWeights[ l ] < fSmallest )
					{
						fSmallest = pVtx[ k ].BlendWeights[ l ];
						iSmallest = l;
					}
				}

				if ( Weight.m_fBias > fSmallest )
				{
					pVtx[ k ].BlendWeights[ iSmallest ] = Weight.m_fBias;
					pVtx[ k ].BlendIndices[ iSmallest ] = Weight.m_iJointId;

					break;
				}
			}
			
			float fBlendWeightTotal = pVtx[ k ].BlendWeights.x + pVtx[ k ].BlendWeights.y + pVtx[ k ].BlendWeights.z + pVtx[ k ].BlendWeights.w;
			pVtx[ k ].BlendWeights /= fBlendWeightTotal;
			LOG_INFO << "Blend Weights " <<  pVtx[ k ].BlendWeights.x << " " << pVtx[ k ].BlendWeights.y  << " " << pVtx[ k ].BlendWeights.z  << " " << pVtx[ k ].BlendWeights.w;


			D3DXVECTOR3 vPos( 0.0f, 0.0f, 0.0f );
			D3DXMATRIXA16 mTransform;
			D3DXVECTOR3 vWeightPos;
						
			// Move the weight position to object space.
			for ( j = 0; j < Vert.m_iWeightCount; ++j )
			{
				const SMD5Weight &Weight = Md5Mesh.m_pWeights[ Vert.m_iStartWeight + j ];
				const SMD5Joint &Joint = m_SkelData.m_pJoints[ Weight.m_iJointId ];

				D3DXMatrixRotationQuaternion( &mTransform, &Joint.m_Orientation );
				mTransform._41 = Joint.m_Position.x;
				mTransform._42 = Joint.m_Position.y;
				mTransform._43 = Joint.m_Position.z;
								
				// Calculate the transformed position based on the joint position and orientation and the weight position.				
				D3DXVec3TransformCoord( &vWeightPos, &Weight.m_Position, &mTransform );
				vPos += vWeightPos * Weight.m_fBias;
#if 0
				//// Calculate the transformed position based on the joint position and orientation and the weight position.				
				D3DXVec3TransformCoord( &vWeightPos, &Weight.m_Position, &mTransform );
				D3DXQUATERNION q(Weight.m_Position.x, Weight.m_Position.y, Weight.m_Position.z, 0.0f);
				
				D3DXQUATERNION conjugateQ;
				D3DXQuaternionConjugate(&conjugateQ, &Joint.m_Orientation);
				
				D3DXQUATERNION result;
				D3DXQuaternionMultiply(&result, &conjugateQ, &q);
				D3DXQuaternionMultiply(&result, &result, &Joint.m_Orientation);
				//vPos += vWeightPos * Weight.m_fBias;						
				vPos[0] += (Joint.m_Position[0] + result[0]) * Weight.m_fBias;
				vPos[1] += (Joint.m_Position[1] + result[1]) * Weight.m_fBias;
				vPos[2] += (Joint.m_Position[2] + result[2]) * Weight.m_fBias;
#endif

			}
			LOG_INFO << "Pos " <<  vPos[0] << " " << vPos[1]  << " " << vPos[2];

			pVtx[ k ].Pos = vPos;			
			pVtx[ k ].Texcoord = Vert.m_ST;

						
				
			//pVtx[ k ].Color = 0xFFFFFFFF;
		}

		//////////////////////////////////////////////////////////////////////////
		// Copy triangles.
		//////////////////////////////////////////////////////////////////////////

		pMesh->m_pGeometry->m_pTriangles = new STriangle[ pMesh->m_pGeometry->m_uiNumTriangles ];

		STriangle *pDestTri = pMesh->m_pGeometry->m_pTriangles;

		for ( int j = 0; j < Md5Mesh.m_iNumTriangles; ++j, ++pDestTri )
		{
			SMD5Triangle &Tri = Md5Mesh.m_pTriangles[ j ];

			pDestTri->m_Indices[ 0 ] = (WORD)Tri.m_Indices[ 0 ];
			pDestTri->m_Indices[ 1 ] = (WORD)Tri.m_Indices[ 1 ];
			pDestTri->m_Indices[ 2 ] = (WORD)Tri.m_Indices[ 2 ];
		}
	}

#if 0
	//////////////////////////////////////////////////////////////////////////
	// Modify geometry for the damage zones.
	//////////////////////////////////////////////////////////////////////////

	// FIXME: Obviously this should be defined externally.

	CArDamageZone *pDz;

#if 1
	pDz = new CArDamageZone();
	pDz->m_iAttachmentJoint = m_pBaseModel->FindJointIndex( "Lloarm" );
	pDz->m_Box.m_vMin = D3DXVECTOR3( -2.0f, 0.0f, -2.0f );
	pDz->m_Box.m_vMax = D3DXVECTOR3( 2.0f, 10.0f, 2.0f );
	pBaseModel->m_DamageZones.push_back( pDz );

	pDz = new CArDamageZone();
	pDz->m_iAttachmentJoint = m_pBaseModel->FindJointIndex( "Luparm" );
	pDz->m_Box.m_vMin = D3DXVECTOR3( -2.0f, 0.0f, -2.0f );
	pDz->m_Box.m_vMax = D3DXVECTOR3( 2.0f, 10.0f, 2.0f );
	pBaseModel->m_DamageZones.push_back( pDz );
#endif

#if 1
	pDz = new CArDamageZone();
	pDz->m_iAttachmentJoint = m_pBaseModel->FindJointIndex( "Ruparm" );
	pDz->m_Box.m_vMin = D3DXVECTOR3( -2.0f, -10.0f, -2.0f );
	pDz->m_Box.m_vMax = D3DXVECTOR3( 2.0f, 0.0f, 2.0f );
	pBaseModel->m_DamageZones.push_back( pDz );

	pDz = new CArDamageZone();
	pDz->m_iAttachmentJoint = m_pBaseModel->FindJointIndex( "Rloarm" );
	pDz->m_Box.m_vMin = D3DXVECTOR3( -2.0f, -10.0f, -2.0f );
	pDz->m_Box.m_vMax = D3DXVECTOR3( 2.0f, 0.0f, 2.0f );
	pBaseModel->m_DamageZones.push_back( pDz );
#endif

#if 1
	pDz = new CArDamageZone();
	pDz->m_iAttachmentJoint = m_pBaseModel->FindJointIndex( "Lupleg" );
	pDz->m_Box.m_vMin = D3DXVECTOR3( -3.0f, 0.0f, -3.0f );
	pDz->m_Box.m_vMax = D3DXVECTOR3( 3.0f, 15.0f, 3.0f );
	pBaseModel->m_DamageZones.push_back( pDz );

	pDz = new CArDamageZone();
	pDz->m_iAttachmentJoint = m_pBaseModel->FindJointIndex( "Lloleg" );
	pDz->m_Box.m_vMin = D3DXVECTOR3( -3.0f, 0.0f, -3.0f );
	pDz->m_Box.m_vMax = D3DXVECTOR3( 3.0f, 15.0f, 3.0f );
	pBaseModel->m_DamageZones.push_back( pDz );
#endif

#if 1
	pDz = new CArDamageZone();
	pDz->m_iAttachmentJoint = m_pBaseModel->FindJointIndex( "Rupleg" );
	pDz->m_Box.m_vMin = D3DXVECTOR3( -3.0f, 0.0f, -3.0f );
	pDz->m_Box.m_vMax = D3DXVECTOR3( 3.0f, 15.0f, 3.0f );
	pBaseModel->m_DamageZones.push_back( pDz );

	pDz = new CArDamageZone();
	pDz->m_iAttachmentJoint = m_pBaseModel->FindJointIndex( "Rloleg" );
	pDz->m_Box.m_vMin = D3DXVECTOR3( -3.0f, 0.0f, -3.0f );
	pDz->m_Box.m_vMax = D3DXVECTOR3( 3.0f, 15.0f, 3.0f );
	pBaseModel->m_DamageZones.push_back( pDz );
#endif

#if 1
	pDz = new CArDamageZone();
	pDz->m_iAttachmentJoint = m_pBaseModel->FindJointIndex( "head" );
	pDz->m_Box.m_vMin = D3DXVECTOR3( -5.0f, -5.0f, -5.0f );
	pDz->m_Box.m_vMax = D3DXVECTOR3( 5.0f, 5.0f, 5.0f );
	pBaseModel->m_DamageZones.push_back( pDz );
#endif

	struct SLocalScope {
		static bool SortFunc( const CArHitBox *a, const CArHitBox *b ) {
			if ( a->m_iAttachmentJoint < b->m_iAttachmentJoint )
			{
				return true;
			}

			return false;
		}
	};

	// Sort damage zones by joint index.
	sort( pBaseModel->m_DamageZones.begin(), pBaseModel->m_DamageZones.end(), SLocalScope::SortFunc );

	//////////////////////////////////////////////////////////////////////////
	// Generate the damage surfaces and finally create the GPU buffers.
	//////////////////////////////////////////////////////////////////////////

	m_pBaseModel->GenerateDamageSurfaces( pBaseModel->m_DamageZones );
#endif
	m_pBaseModel->CreateBuffers();
}

idRenderModelMD5::idRenderModelMD5() : m_pBaseModel(NULL)
{

}

idRenderModelMD5::~idRenderModelMD5()
{
	SAFE_DELETE(m_pBaseModel);
}
