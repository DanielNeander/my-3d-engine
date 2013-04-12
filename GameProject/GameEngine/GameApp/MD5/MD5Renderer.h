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

#ifndef __RENDERWORLD_H__
#define __RENDERWORLD_H__

#include "GameApp/Mesh.h"
#include "GameApp/Mesh_D3D11.h"
/*
===============================================================================

	Render World

===============================================================================
*/

#define PROC_FILE_EXT				"proc"
#define	PROC_FILE_ID				"mapProcFile003"

// shader parms
const int MAX_GLOBAL_SHADER_PARMS	= 12;

const int SHADERPARM_RED			= 0;
const int SHADERPARM_GREEN			= 1;
const int SHADERPARM_BLUE			= 2;
const int SHADERPARM_ALPHA			= 3;
const int SHADERPARM_TIMESCALE		= 3;
const int SHADERPARM_TIMEOFFSET		= 4;
const int SHADERPARM_DIVERSITY		= 5;	// random between 0.0 and 1.0 for some effects (muzzle flashes, etc)
const int SHADERPARM_MODE			= 7;	// for selecting which shader passes to enable
const int SHADERPARM_TIME_OF_DEATH	= 7;	// for the monster skin-burn-away effect enable and time offset

// model parms
const int SHADERPARM_MD5_SKINSCALE	= 8;	// for scaling vertex offsets on md5 models (jack skellington effect)

const int SHADERPARM_MD3_FRAME		= 8;
const int SHADERPARM_MD3_LASTFRAME	= 9;
const int SHADERPARM_MD3_BACKLERP	= 10;

const int SHADERPARM_BEAM_END_X		= 8;	// for _beam models
const int SHADERPARM_BEAM_END_Y		= 9;
const int SHADERPARM_BEAM_END_Z		= 10;
const int SHADERPARM_BEAM_WIDTH		= 11;

const int SHADERPARM_SPRITE_WIDTH		= 8;
const int SHADERPARM_SPRITE_HEIGHT		= 9;

const int SHADERPARM_PARTICLE_STOPTIME = 8;	// don't spawn any more particles after this time

// guis
const int MAX_RENDERENTITY_GUI		= 3;

typedef bool(*deferredEntityCallback_t)( struct renderEntity_s *, const struct renderView_s * );


typedef struct renderEntity_s {
	class idRenderModel *			hModel;				// this can only be null if callback is set

	int						entityNum;
	int						bodyId;

	deferredEntityCallback_t	callback;

	void *					callbackData;			// used for whatever the callback wants

	class idBounds				bounds;					// only needs to be set for deferred models and md5s

	// player bodies and possibly player shadows should be suppressed in views from
	// that player's eyes, but will show up in mirrors and other subviews
	// security cameras could suppress their model in their subviews if we add a way
	// of specifying a view number for a remoteRenderMap view
	int						suppressSurfaceInViewID;
	int						suppressShadowInViewID;

	// world models for the player and weapons will not cast shadows from view weapon
	// muzzle flashes
	int						suppressShadowInLightID;

	// if non-zero, the surface and shadow (if it casts one)
	// will only show up in the specific view, ie: player weapons
	int						allowSurfaceInViewID;

	// positioning
	// axis rotation vectors must be unit length for many
	// R_LocalToGlobal functions to work, so don't scale models!
	// axis vectors are [0] = forward, [1] = left, [2] = up
	noVec3					origin;
	noMat3					axis;

	// texturing
	const idMaterial *		customShader;			// if non-0, all surfaces will use this
	const idMaterial *		referenceShader;		// used so flares can reference the proper light shader
	const idDeclSkin *		customSkin;				// 0 for no remappings
	float					shaderParms[ MAX_ENTITY_SHADER_PARMS ];	// can be used in any way by shader or model generation


	struct renderView_s	*	remoteRenderView;		// any remote camera surfaces will use this

	int						numJoints;
	class idJointMat *			joints;					// array of joints that will modify vertices.
	float					modelDepthHack;			// squash depth range so particle effects don't clip into walls

	// options to override surface shader flags (replace with material parameters?)
	bool					noSelfShadow;			// cast shadows onto other objects,but not self
	bool					noShadow;				// no shadow at all

	bool					noDynamicInteractions;	// don't create any light / shadow interactions after
	// the level load is completed.  This is a performance hack
	// for the gigantic outdoor meshes in the monorail map, so
	// all the lights in the moving monorail don't touch the meshes

	bool					weaponDepthHack;		// squash depth range so view weapons don't poke into walls
	// this automatically implies noShadow
	int						forceUpdate;			// force an update (NOTE: not a bool to keep this struct a multiple of 4 bytes)
	int						timeGroup;
	int						xrayIndex;

} renderEntity_t;

typedef struct renderLight_s {
	noMat3					axis;				// rotation vectors, must be unit length
	noVec3					origin;

	// if non-zero, the light will not show up in the specific view,
	// which may be used if we want to have slightly different muzzle
	// flash lights for the player and other views
	int						suppressLightInViewID;

	// if non-zero, the light will only show up in the specific view
	// which can allow player gun gui lights and such to not effect everyone
	int						allowLightInViewID;

	// I am sticking the four bools together so there are no unused gaps in
	// the padded structure, which could confuse the memcmp that checks for redundant
	// updates
	bool					noShadows;			// (should we replace this with material parameters on the shader?)
	bool					noSpecular;			// (should we replace this with material parameters on the shader?)

	bool					pointLight;			// otherwise a projection light (should probably invert the sense of this, because points are way more common)
	bool					parallel;			// lightCenter gives the direction to the light at infinity
	noVec3					lightRadius;		// xyz radius for point lights
	noVec3					lightCenter;		// offset the lighting direction for shading and
	// shadows, relative to origin

	// frustum definition for projected lights, all reletive to origin
	// FIXME: we should probably have real plane equations here, and offer
	// a helper function for conversion from this format
	noVec3					target;
	noVec3					right;
	noVec3					up;
	noVec3					start;
	noVec3					end;

	// Dmap will generate an optimized shadow volume named _prelight_<lightName>
	// for the light against all the _area* models in the map.  The renderer will
	// ignore this value if the light has been moved after initial creation
	idRenderModel *			prelightModel;

	// muzzle flash lights will not cast shadows from player and weapon world models
	int						lightId;


	const class idMaterial *		shader;				// NULL = either lights/defaultPointLight or lights/defaultProjectedLight
	float					shaderParms[MAX_ENTITY_SHADER_PARMS];		// can be used in any way by shader
	//idSoundEmitter *		referenceSound;		// for shader sound tables, allowing effects to vary with sounds
} renderLight_t;

typedef struct renderView_s {
	// player views will set this to a non-zero integer for model suppress / allow
	// subviews (mirrors, cameras, etc) will always clear it to zero
	int						viewID;

	// sized from 0 to SCREEN_WIDTH / SCREEN_HEIGHT (640/480), not actual resolution
	int						x, y, width, height;

	float					fov_x, fov_y;
	noVec3					vieworg;
	noMat3					viewaxis;			// transformation matrix, view looks down the positive X axis

	bool					cramZNear;			// for cinematics, we want to set ZNear much lower
	bool					forceUpdate;		// for an update 

	// time in milliseconds for shader effects and other time dependent rendering issues
	int						time;
	float					shaderParms[MAX_GLOBAL_SHADER_PARMS];		// can be used in any way by shader
	const idMaterial		*globalMaterial;							// used to override everything draw

} renderView_t;



//==============================================================================
//  Simplified batch, only use floats for data and unsigned int for indices
//==============================================================================



enum Attribute_type {
	ATT_VERTEX    = 0,
	ATT_NORMAL    = 1,
	ATT_TEXCOORD  = 2,
	ATT_COLOR     = 3,
	ATT_TANGENT_S = 4,
	ATT_TANGENT_T = 5
};

struct Format {
	Attribute_type att_type;
	unsigned int size;
	unsigned int offset;
	unsigned int index;
};

//==============================================================================
//  Batch
//==============================================================================
class BatchGeom {
public:
	BatchGeom();
	virtual ~BatchGeom();

	void clean();

	unsigned int get_index(const unsigned int index) const {
		return m_indices[index];
	}

	void set_index(const unsigned int index, const unsigned int value) {
		m_indices[index] = value;
	}

	float *get_vertices() const                             { return m_vertices; }
	unsigned short *get_indices() const                      { return m_indices;  }
	const unsigned int get_vertex_count() const             { return m_num_vertices;  }
	const unsigned int get_index_count()  const             { return m_num_indices;   }
	const unsigned int get_vertex_size()  const             { return m_vertex_size; }

	const unsigned int get_format_count() const             { return m_formats.size(); }
	const Format &get_format(unsigned int index) const      { return m_formats[index]; }

	void set_vertices(void *vertex_array, const unsigned int vertex_count, const unsigned int size) {
		m_vertices = (float *) vertex_array;
		m_num_vertices = vertex_count;
		m_vertex_size = size;
		m_vertex_elements = m_vertex_size / sizeof( float );
	}
	void set_indices(unsigned short *index_array, const unsigned int index_count ) {
		m_indices = index_array;
		m_num_indices = index_count;
	}

	void add_format(const Attribute_type att_type, const unsigned int size, const unsigned int offset, const unsigned int index = 0) {
		Format format;

		format.att_type = att_type;
		format.size = size;
		format.offset = offset;
		format.index = index;

		m_formats.push_back(format);

		if(index == 0) {
			switch( att_type ) {
			case ATT_VERTEX:     m_vertex_offset = offset;
				break;
			case ATT_NORMAL:     m_normal_offset = offset;
				break;
			case ATT_TEXCOORD:   m_texcoord_offset = offset;
				break;					
			case ATT_TANGENT_S:  m_tangent_s_offset = offset;
				break;											                    
			case ATT_TANGENT_T:  m_tangent_t_offset = offset;
				break;											                    						                    
			}	 
		}

		if( size+offset > m_vertex_elements ) {
			m_vertex_elements = size+offset;
			m_vertex_size = m_vertex_elements * sizeof(float);
		}
	}

	bool find_attribute(const Attribute_type att_type, const unsigned int index = 0, unsigned int *where = NULL) const;
	bool insert_attribute(const Attribute_type att_type, const unsigned int size, const unsigned int index = 0);
	bool remove_attribute(const Attribute_type att_type, const unsigned int index = 0);

	virtual void render() {}

	bool add_normals();
	bool add_tangent_space();

	float get_value(const float *src, const unsigned int index) {
		return *(((float *) src) + index);
	}

	inline noVec3& get_vertex_by_index( unsigned int index )      { return get_vertex   ( m_indices[index] ); }
	inline noVec3& get_normal_by_index( unsigned int index )      { return get_normal   ( m_indices[index] ); }
	inline noVec2& get_texcoord_by_index( unsigned int index )    { return get_texcoord ( m_indices[index] ); }
	inline noVec3& get_tangent_s_by_index( unsigned int index )   { return get_tangent_s( m_indices[index] ); }
	inline noVec3& get_tangent_t_by_index( unsigned int index )   { return get_tangent_t( m_indices[index] ); }

	inline noVec3& get_vertex( unsigned int index )      { return *((noVec3 *)(&m_vertices[ index*m_vertex_elements + m_vertex_offset ])); }
	inline noVec3& get_normal( unsigned int index )      { return *((noVec3 *)(&m_vertices[ index*m_vertex_elements + m_normal_offset ])); }
	inline noVec2& get_texcoord( unsigned int index )    { return *((noVec2 *)(&m_vertices[ index*m_vertex_elements + m_texcoord_offset ])); }
	inline noVec3& get_tangent_s( unsigned int index )   { return *((noVec3 *)(&m_vertices[ index*m_vertex_elements + m_tangent_s_offset ])); }
	inline noVec3& get_tangent_t( unsigned int index )   { return *((noVec3 *)(&m_vertices[ index*m_vertex_elements + m_tangent_t_offset ])); }    


	int m_startVert;
	int m_endVert;
protected:
	float *m_vertices;
	unsigned short *m_indices;

	

	unsigned short m_num_vertices, m_num_indices;
	unsigned short m_vertex_size, m_vertex_elements;
	unsigned short m_vertex_offset, m_normal_offset, m_texcoord_offset, m_tangent_s_offset, m_tangent_t_offset;

	std::vector<Format> m_formats;
	Primitives m_primitive_type;    
};

struct VertexPNTBUV {
	noVec3 pos;
	noVec3 norm;
	noVec3 tangent;
	noVec3 binormal;
	noVec2 uv;
	//noVec4 color;
};

struct Vertex_doom3 {
	noVec3 vertex;
	noVec2 texcoord;
	noVec3 normal;
};

struct ShadowVertex_doom3 {
	noVec4	v;
};

class BSPMesh : public Mesh_D3D11
{
public:
	
	int numIndexes;
	int numVerts;
	int							numShadowIndexesNoFrontCaps;	// shadow volumes with front caps omitted
	int							numShadowIndexesNoCaps;			// shadow volumes with the front and rear caps omitted
		
	// projected on it, which means that if the view is on the outside of that
	// plane, we need to draw the rear caps of the shadow volume
	// turboShadows will have SHADOW_CAP_INFINITE	
	int shadowCapPlaneBits;

	BatchGeom* batch;

	const idMaterial* mat;
};

class InterAreaPortal;

class PortalArea {
public:
	PortalArea( const std::string & name, int index) : m_name(name), m_index(index) {
	}
	const std::string & get_name() const { return m_name; }
	void Render(class BaseCamera* c, InterAreaPortal* portal, noVec2& min, noVec2& max);
		
	std::vector<class BSPMesh*>	mBspMeshes;
	std::map<const class idMaterial*, std::vector<BatchGeom*> >	mBatches;

	std::string m_name;
	int m_index;

	VertexFormatID			m_vf;
	VertexBufferID			m_vb;
	IndexBufferID			m_ib;
	ShaderID				m_shader;
	SamplerStateID			m_samplerState;

	void CreateBuffers();
	
	void add_portal( InterAreaPortal *portal) { m_portals.push_back( portal ); }
	std::vector<InterAreaPortal*> m_portals;
	std::map<InterAreaPortal*, int> m_rendered;   

};

class InterAreaPortal {
public:
	InterAreaPortal() : m_frame_rendered(999) {}

	int CheckVisible(BaseCamera* c);

	void Render(BaseCamera* c, int index, noVec2& minExt, noVec2& maxExt);
	void transform_points();

	noVec2 m_transformed_min;
	noVec2 m_transformed_max;

	std::vector<noVec3> m_points;
	std::vector<noVec2> m_transformed_points;
	int m_area_pos;
	int m_area_neg;

	int m_frame_rendered;
	int m_visible;
};

class MD5Renderer 
{
public:

	void ClearWorld();
	void FreeWorld();
	bool InitFromMap( const char *name );
	PortalArea *ParseModel( idLexer *src );
	void ParseInterAreaPortals( idLexer *src );
	void ParseNodes( idLexer *src );


	int numPortalAreas;
	int numInterAreaPortals;
	int numAreaNodes;
	idStr	mapName;

	renderView_t			primaryRenderView;
	viewDef_t *				primaryView;

	noVec4					ambientLightVector;	// used for "ambient bump mapping"
	float					sortOffset;				// for determinist sorting of equal sort materials
	viewDef_t *				viewDef;

	void	Render();

};

extern MD5Renderer	tr;


// this does various checks before calling the idDeclSkin
const idMaterial *R_RemapShaderBySkin( const idMaterial *shader, const idDeclSkin *customSkin, const idMaterial *customShader );

#endif