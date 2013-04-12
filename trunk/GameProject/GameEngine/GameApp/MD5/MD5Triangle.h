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


const float	DEFAULT_FOG_DISTANCE = 500.0f;

const int FOG_ENTER_SIZE = 64;
const float FOG_ENTER = (FOG_ENTER_SIZE+1.0f)/(FOG_ENTER_SIZE*2);

// idScreenRect gets carried around with each drawSurf, so it makes sense
// to keep it compact, instead of just using the idBounds class
class idScreenRect {
public:
	short		x1, y1, x2, y2;							// inclusive pixel bounds inside viewport
	float       zmin, zmax;								// for depth bounds test

	void		Clear();								// clear to backwards values
	void		AddPoint( float x, float y );			// adds a point
	void		Expand();								// expand by one pixel each way to fix roundoffs
	void		Intersect( const idScreenRect &rect );
	void		Union( const idScreenRect &rect );
	bool		Equals( const idScreenRect &rect ) const;
	bool		IsEmpty() const;
};

idScreenRect R_ScreenRectFromViewFrustumBounds( const idBounds &bounds );
void R_ShowColoredScreenRect( const idScreenRect &rect, int colorIndex );

typedef struct viewEntity_s {
	struct viewEntity_s	*next;

	float				modelMatrix[16];		// local coords to global coords
	float				modelViewMatrix[16];	// local coords to eye coords
} viewEntity_t;

const int	MAX_CLIP_PLANES	= 1;				// we may expand this to six for some subview issues



// viewDefs are allocated on the frame temporary stack memory
typedef struct viewDef_s {
	// specified in the call to DrawScene()
	//renderView_t		renderView;

	float				projectionMatrix[16];
	viewEntity_t		worldSpace;

	float				floatTime;

	noVec3				initialViewAreaOrigin;
	// Used to find the portalArea that view flooding will take place from.
	// for a normal view, the initialViewOrigin will be renderView.viewOrg,
	// but a mirror may put the projection origin outside
	// of any valid area, or in an unconnected area of the map, so the view
	// area must be based on a point just off the surface of the mirror / subview.
	// It may be possible to get a failed portal pass if the plane of the
	// mirror intersects a portal, and the initialViewAreaOrigin is on
	// a different side than the renderView.viewOrg is.

	bool				isSubview;				// true if this view is not the main view
	bool				isMirror;				// the portal is a mirror, invert the face culling
	bool				isXraySubview;

	bool				isEditor;

	int					numClipPlanes;			// mirrors will often use a single clip plane
	noPlane				clipPlanes[MAX_CLIP_PLANES];		// in world space, the positive side

	int					areaNum;				// -1 = not in a valid area

	bool *				connectedAreas;
	// An array in frame temporary memory that lists if an area can be reached without
	// crossing a closed door.  This is used to avoid drawing interactions
	// when the light is behind a closed door.

} viewDef_t;


// viewLights are allocated on the frame temporary stack memory
// a viewLight contains everything that the back end needs out of an idRenderLightLocal,
// which the front end may be modifying simultaniously if running in SMP mode.
// a viewLight may exist even without any surfaces, and may be relevent for fogging,
// but should never exist if its volume does not intersect the view frustum
typedef struct viewLight_s {
	struct viewLight_s *	next;

	// for scissor clipping, local inside renderView viewport
	// scissorRect.Empty() is true if the viewEntity_t was never actually
	// seen through any portals
	idScreenRect			scissorRect;

	// if the view isn't inside the light, we can use the non-reversed
	// shadow drawing, avoiding the draws of the front and rear caps
	bool					viewInsideLight;

	// true if globalLightOrigin is inside the view frustum, even if it may
	// be obscured by geometry.  This allows us to skip shadows from non-visible objects
	bool					viewSeesGlobalLightOrigin;	

	// if !viewInsideLight, the corresponding bit for each of the shadowFrustum
	// projection planes that the view is on the negative side of will be set,
	// allowing us to skip drawing the projected caps of shadows if we can't see the face
	int						viewSeesShadowPlaneBits;

	noVec3					globalLightOrigin;			// global light origin used by backend
	noPlane					lightProject[4];			// light project used by backend
	noPlane					fogPlane;					// fog plane for backend fog volume rendering
	//const srfTriangles_t *	frustumTris;				// light frustum for backend fog volume rendering
	const idMaterial *		lightShader;				// light shader used by backend
	const float	*			shaderRegisters;			// shader registers used by backend
	TextureID				falloffImage;				// falloff image used by backend
} viewLight_t;


typedef struct {
	TextureID lightImg;
	TextureID	lightFallofImg;
	TextureID	bumpImg;
	TextureID	diffuseImg;
	TextureID	specularImg;

	noVec4				diffuseColor;	// may have a light color baked into it, will be < tr.backEndRendererMaxLight
	noVec4				specularColor;	// may have a light color baked into it, will be < tr.backEndRendererMaxLight
	stageVertexColor_t	vertexColor;	// applies to both diffuse and specular

	int					ambientLight;	// use tr.ambientNormalMap instead of normalization cube map 
	// (not a bool just to avoid an uninitialized memory check of the pad region by valgrind)

	// these are loaded into the vertex program
	noVec4				localLightOrigin;
	noVec4				localViewOrigin;
	noVec4				lightProjection[4];	// in local coordinates, possibly with a texture matrix baked in
	noVec4				bumpMatrix[2];
	noVec4				diffuseMatrix[2];
	noVec4				specularMatrix[2];
} drawInteraction_t;
	




typedef struct {
	int		current2DMap;
	int		current3DMap;
	int		currentCubeMap;
	int		texEnv;
	textureType_t	textureType;
} tmu_t;


const int MAX_MULTITEXTURE_UNITS =	8;
typedef struct {
	tmu_t		tmu[MAX_MULTITEXTURE_UNITS];
	int			currenttmu;

	int			faceCulling;
	int			glStateBits;
	bool		forceGlState;		// the next GL_State will ignore glStateBits and set everything
} glstate_t;




// performs radius cull first, then corner cull
bool R_CullLocalBox( const idBounds &bounds, const float modelMatrix[16], int numPlanes, const noPlane *planes );
bool R_RadiusCullLocalBox( const idBounds &bounds, const float modelMatrix[16], int numPlanes, const noPlane *planes );
bool R_CornerCullLocalBox( const idBounds &bounds, const float modelMatrix[16], int numPlanes, const noPlane *planes );

void R_AxisToModelMatrix( const noMat3 &axis, const noVec3 &origin, float modelMatrix[16] );

// note that many of these assume a normalized matrix, and will not work with scaled axis
void R_GlobalPointToLocal( const float modelMatrix[16], const noVec3 &in, noVec3 &out );
void R_GlobalVectorToLocal( const float modelMatrix[16], const noVec3 &in, noVec3 &out );
void R_GlobalPlaneToLocal( const float modelMatrix[16], const noPlane &in, noPlane &out );
void R_PointTimesMatrix( const float modelMatrix[16], const noVec4 &in, noVec4 &out );
void R_LocalPointToGlobal( const float modelMatrix[16], const noVec3 &in, noVec3 &out );
void R_LocalVectorToGlobal( const float modelMatrix[16], const noVec3 &in, noVec3 &out );
void R_LocalPlaneToGlobal( const float modelMatrix[16], const noPlane &in, noPlane &out );
void R_TransformEyeZToWin( float src_z, const float *projectionMatrix, float &dst_z );


void R_TransformModelToClip( const noVec3 &src, const float *modelMatrix, const float *projectionMatrix, noPlane &eye, noPlane &dst );

void R_TransformClipToDevice( const noPlane &clip, const viewDef_t *view, noVec3 &normalized );

void R_TransposeGLMatrix( const float in[16], float out[16] );

void R_SetViewMatrix( viewDef_t *viewDef );

void myGlMultMatrix( const float *a, const float *b, float *out );


// deformable meshes precalculate as much as possible from a base frame, then generate
// complete srfTriangles_t from just a new set of vertexes
typedef struct deformInfo_s {
	int				numSourceVerts;

	// numOutputVerts may be smaller if the input had duplicated or degenerate triangles
	// it will often be larger if the input had mirrored texture seams that needed
	// to be busted for proper tangent spaces
	int				numOutputVerts;

	int				numMirroredVerts;
	int *			mirroredVerts;

	int				numIndexes;
	glIndex_t *		indexes;

	glIndex_t *		silIndexes;

	int				numDupVerts;
	int *			dupVerts;

	int				numSilEdges;
	silEdge_t *		silEdges;

	dominantTri_t *	dominantTris;
} deformInfo_t;

/*
============================================================

SCENE GENERATION

============================================================
*/
void R_InitFrameData( void );
void R_ShutdownFrameData( void );
int R_CountFrameData( void );
void R_ToggleSmpFrame( void );
void *R_FrameAlloc( int bytes );
void *R_ClearedFrameAlloc( int bytes );
void R_FrameFree( void *data );

void *R_StaticAlloc( int bytes );
void *R_ClearedStaticAlloc( int bytes );
void R_StaticFree( void *data );

/*
=============================================================

TR_ORDERINDEXES

=============================================================
*/

void R_OrderIndexes( int numIndexes, glIndex_t *indexes );


// these masks are the inverse, meaning when set the glColorMask value will be 0,
// preventing that channel from being written
const int GLS_DEPTHMASK							= 0x00000100;
const int GLS_REDMASK							= 0x00000200;
const int GLS_GREENMASK							= 0x00000400;
const int GLS_BLUEMASK							= 0x00000800;
const int GLS_ALPHAMASK							= 0x00001000;
const int GLS_COLORMASK							= (GLS_REDMASK|GLS_GREENMASK|GLS_BLUEMASK);