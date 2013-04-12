#pragma once 
/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "GameApp/DamageModel.h"

/*
===============================================================================

	Render Model

===============================================================================
*/

// shared between the renderer, game, and Maya export DLL
#define MD5_VERSION_STRING		"MD5Version"
#define MD5_MESH_EXT			"md5mesh"
#define MD5_ANIM_EXT			"md5anim"
#define MD5_CAMERA_EXT			"md5camera"
#define MD5_VERSION				10


typedef struct lightingCache_s {
	noVec3						localLightVector;		// this is the statically computed vector to the light
	// in texture space for cards without vertex programs
} lightingCache_t;

typedef struct shadowCache_s {
	noVec4						xyz;					// we use homogenous coordinate tricks
} shadowCache_t;

const int SHADOW_CAP_INFINITE	= 64;


// our only drawing geometry type
typedef struct srfTriangles_s {
	idBounds					bounds;					// for culling

	int							ambientViewCount;		// if == tr.viewCount, it is visible this view

	bool						generateNormals;		// create normals from geometry, instead of using explicit ones
	bool						tangentsCalculated;		// set when the vertex tangents have been calculated
	bool						facePlanesCalculated;	// set when the face planes have been calculated
	bool						perfectHull;			// true if there aren't any dangling edges
	bool						deformedSurface;		// if true, indexes, silIndexes, mirrorVerts, and silEdges are
	// pointers into the original surface, and should not be freed

	int							numVerts;				// number of vertices
	idDrawVert *				verts;					// vertices, allocated with special allocator

	int							numIndexes;				// for shadows, this has both front and rear end caps and silhouette planes
	glIndex_t *					indexes;				// indexes, allocated with special allocator

	glIndex_t *					silIndexes;				// indexes changed to be the first vertex with same XYZ, ignoring normal and texcoords

	int							numMirroredVerts;		// this many verts at the end of the vert list are tangent mirrors
	int *						mirroredVerts;			// tri->mirroredVerts[0] is the mirror of tri->numVerts - tri->numMirroredVerts + 0

	int							numDupVerts;			// number of duplicate vertexes
	int *						dupVerts;				// pairs of the number of the first vertex and the number of the duplicate vertex

	int							numSilEdges;			// number of silhouette edges
	silEdge_t *					silEdges;				// silhouette edges

	noPlane *					facePlanes;				// [numIndexes/3] plane equations

	dominantTri_t *				dominantTris;			// [numVerts] for deformed surface fast tangent calculation

	int							numShadowIndexesNoFrontCaps;	// shadow volumes with front caps omitted
	int							numShadowIndexesNoCaps;			// shadow volumes with the front and rear caps omitted

	int							shadowCapPlaneBits;		// bits 0-5 are set when that plane of the interacting light has triangles
	// projected on it, which means that if the view is on the outside of that
	// plane, we need to draw the rear caps of the shadow volume
	// turboShadows will have SHADOW_CAP_INFINITE

	shadowCache_t *				shadowVertexes;			// these will be copied to shadowCache when it is going to be drawn.
	// these are NULL when vertex programs are available

	struct srfTriangles_s *		ambientSurface;			// for light interactions, point back at the original surface that generated
	// the interaction, which we will get the ambientCache from

	struct srfTriangles_s *		nextDeferredFree;		// chain of tris to free next frame

	// data in vertex object space, not directly readable by the CPU
	struct vertCache_s *		indexCache;				// int
	struct vertCache_s *		ambientCache;			// idDrawVert
	struct vertCache_s *		lightingCache;			// lightingCache_t
	struct vertCache_s *		shadowCache;			// shadowCache_t
} srfTriangles_t;

typedef idList<srfTriangles_t *> idTriList;

typedef struct modelSurface_s {
	int							id;
	//const idMaterial *		shader;
	srfTriangles_t *			geometry;
} modelSurface_t;

typedef enum {
	DM_STATIC,		// never creates a dynamic model
	DM_CACHED,		// once created, stays constant until the entity is updated (animating characters)
	DM_CONTINUOUS	// must be recreated for every single view (time dependent things like particles)
} dynamicModel_t;

typedef enum {
	INVALID_JOINT				= -1
} jointHandle_t;

class idMD5Joint {
public:
	idMD5Joint() { parent = NULL; }
	idStr						name;
	const idMD5Joint *			parent;
};


class idRenderModel {
public:
	virtual						~idRenderModel() {};

	virtual void				InitFromFile( const char *fileName ) = 0;

	// used for initial loads, reloadModel, and reloading the data of purged models
	// Upon exit, the model will absolutely be valid, but possibly as a default model
	virtual void				LoadModel() = 0;

	// internal use
	virtual bool				IsLoaded() = 0;

	// Returns the number of joints or 0 if the model is not an MD5
	virtual int					NumJoints( void ) const = 0;

	// Returns the MD5 joints or NULL if the model is not an MD5
	virtual const idMD5Joint *	GetJoints( void ) const = 0;

	// Returns the handle for the joint with the given name.
	virtual jointHandle_t		GetJointHandle( const char *name ) const = 0;

	// Returns the name for the joint with the given handle.
	virtual const char *		GetJointName( jointHandle_t handle ) const = 0;

	// Returns the default animation pose or NULL if the model is not an MD5.
	virtual const idJointQuat *	GetDefaultPose( void ) const = 0;

	// Returns number of the joint nearest to the given triangle.
	virtual int					NearestJoint( int surfaceNum, int a, int c, int b ) const = 0;

	// returns the name of the model
	virtual const char	*		Name() const = 0;

	// dynamic models should return a fast, conservative approximation
	// static models should usually return the exact value
	virtual idBounds			Bounds( const struct renderEntity_s *ent = NULL ) const = 0;
};

class idRenderModelStatic : public idRenderModel {
public:
	

	idRenderModelStatic();
	virtual						~idRenderModelStatic();
	virtual void				InitFromFile( const char *fileName ) {}
	
	virtual void				LoadModel();
	virtual bool				IsLoaded();

	virtual const char *		Name() const;

	virtual int					NumJoints( void ) const;
	virtual const idMD5Joint *	GetJoints( void ) const;
	virtual jointHandle_t		GetJointHandle( const char *name ) const;
	virtual const char *		GetJointName( jointHandle_t handle ) const;
	virtual const idJointQuat *	GetDefaultPose( void ) const;
	virtual int					NearestJoint( int surfaceNum, int a, int b, int c ) const;
	idBounds Bounds( const struct renderEntity_s *mdef ) const;

public:
	bool						purged;					// eventually we will have dynamic reloading
	idBounds					bounds;

protected:
	idStr						name;

};

class idMD5Mesh {
	friend class				idRenderModelMD5;
	friend class				SMD5Mesh;
public:
	idMD5Mesh();
	~idMD5Mesh();

	void ParseMesh( idLexer &parser, int numJoints, const idJointMat *joints, SMD5Mesh* pCurMesh );
	//void						UpdateSurface( const struct renderEntity_s *ent, const idJointMat *joints, modelSurface_t *surf );
	idBounds					CalcBounds( const idJointMat *joints );
	int							NearestJoint( int a, int b, int c ) const;
	int							NumVerts( void ) const;
	int							NumTris( void ) const;
	int							NumWeights( void ) const;

	const idMaterial *			shader;				// material applied to mesh
private:
	idList<noVec2>				texCoords;			// texture coordinates
	int							numWeights;			// number of weights
	noVec4 *					scaledWeights;		// joint weights
	int *						weightIndex;		// pairs of: joint offset + bool true if next weight is for next vertex
	int							numTris;			// number of triangles
	struct deformInfo_s *		deformInfo;			// used to create srfTriangles_t from base frames and new vertexes
	int							surfaceNum;			// number of the static surface created for this mesh

	void						TransformVerts( idDrawVert *verts, const idJointMat *joints );
	void						TransformScaledVerts( idDrawVert *verts, const idJointMat *joints, float scale );
};



//////////////////////////////////////////////////////////////////////////
// CArModel_MD5
//////////////////////////////////////////////////////////////////////////


struct SMD5Vertex
{	
	// Calculated from weights.
	D3DXVECTOR2		m_ST;
	int				m_iStartWeight;
	int				m_iWeightCount;
};

struct SMD5Triangle
{
	int m_Indices[ 3 ];
};

struct SMD5Weight
{
	int				m_iJointId;
	float			m_fBias;
	D3DXVECTOR3		m_Position;
};

struct SMD5Mesh
{
	SMD5Mesh() : m_pVertices( NULL ), m_pTriangles( NULL ), m_pWeights( NULL ) {}

	~SMD5Mesh()
	{
		SAFE_DELETE_ARRAY( m_pVertices );
		SAFE_DELETE_ARRAY( m_pTriangles );
		SAFE_DELETE_ARRAY( m_pWeights );
	}

	SMD5Vertex		*m_pVertices;
	SMD5Triangle	*m_pTriangles;
	SMD5Weight		*m_pWeights;

	int				m_iNumVertices;
	int				m_iNumTriangles;
	int				m_iNumWeights;
};

struct SMD5Joint
{
	idStr			m_strName;
	int				m_iIndex;
	int				m_iParentIndex;
	D3DXVECTOR3		m_Position;
	D3DXQUATERNION	m_Orientation;
	D3DXMATRIXA16	m_InverseBindPoseMatrix;	
	float			m_fScale;

	SMD5Joint() : m_fScale( 1.0f ) {}
};


struct SMD5Skeleton
{
	SMD5Skeleton() : m_pJoints( NULL ) {}
	~SMD5Skeleton() { SAFE_DELETE_ARRAY( m_pJoints ); }

	SMD5Joint			*m_pJoints;
	int					m_iNumJoints;
};

struct SMD5Model
{
	SMD5Model() : m_pMeshes( NULL ) {}
	~SMD5Model() { SAFE_DELETE_ARRAY( m_pMeshes ); }

	SMD5Mesh		*m_pMeshes;
	int				m_iNumMeshes;
};

class idRenderModelMD5 : public idRenderModelStatic {
public:
	idRenderModelMD5();
	virtual ~idRenderModelMD5();

	virtual void				InitFromFile( const char *fileName );

	virtual void				LoadModel();
	virtual int					NumJoints( void ) const;
	virtual const idMD5Joint *	GetJoints( void ) const;
	virtual jointHandle_t		GetJointHandle( const char *name ) const;
	virtual const char *		GetJointName( jointHandle_t handle ) const;
	virtual const idJointQuat *	GetDefaultPose( void ) const;
	virtual int					NearestJoint( int surfaceNum, int a, int b, int c ) const;

public:
	SMD5Skeleton	m_SkelData;
	CArBaseModel				*m_pBaseModel;
	idList<idMD5Mesh>			meshes;

private:
	idList<idMD5Joint>			joints;
	idList<idJointQuat>			defaultPose;
	
	SMD5Model					m_ModelData;


	void						CalculateBounds( const idJointMat *joints );	
	void						ParseJoint( idLexer &parser, idMD5Joint *joint, idJointQuat *defaultPose );
	void PurgeModel();
	void GenerateBaseModelData( CArBaseModel *pBaseModel );

};


