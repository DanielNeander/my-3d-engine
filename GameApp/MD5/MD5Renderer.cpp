#include "stdafx.h"
#include "MD5Renderer.h"
#include "MD5Scene.h"
#include "GameApp/GameApp.h"
#include "GameApp/GameCamera.h"
#include "MD5Triangle.h"


MD5Renderer	tr;
//==============================================================================
//  set_value()
//==============================================================================
void set_value(const char *dest, const unsigned int index, float value) {
	*(((float *) dest) + index) = value;
}

BatchGeom::BatchGeom() {
	m_vertices = NULL;
	m_indices  = NULL;

	clean();
}

BatchGeom::~BatchGeom(){
	clean();
}

void BatchGeom::clean() {
	if (m_vertices != NULL) delete[] m_vertices;
	if (m_indices  != NULL) delete[] m_indices;
	m_vertices = NULL;
	m_indices = NULL;    

	m_num_vertices = 0;
	m_num_indices  = 0;
	m_vertex_size = 0;
	m_vertex_elements = 0;

	m_tangent_s_offset = m_tangent_t_offset = m_vertex_offset = m_normal_offset = m_texcoord_offset = 0;

	m_primitive_type = PRIM_TRIANGLES;
}

bool BatchGeom::find_attribute( const Attribute_type att_type, const unsigned int index /*= 0*/, unsigned int *where /*= NULL*/ ) const{
	for (unsigned int i = 0; i < m_formats.size(); i++) {
		if (m_formats[i].att_type == att_type && m_formats[i].index == index) {
			if (where != NULL) *where = i;
			return true;
		}
	}
	return false;
}

bool BatchGeom::insert_attribute( const Attribute_type att_type, const unsigned int size, const unsigned int index /*= 0*/ ){
	unsigned int i;

	for (i = 0; i < m_formats.size(); i++) {
		if (m_formats[i].att_type == att_type && m_formats[i].index == index) return false;
	}

	add_format(att_type, size, m_vertex_elements, index);

	unsigned int old_vertex_elements = m_vertex_elements - size;
	float *new_vertices = new float[m_num_vertices * m_vertex_elements];

	float *dest = new_vertices;
	float *src = m_vertices;
	for(i=0; i<m_num_vertices; ++i) {
		memcpy(dest, src, old_vertex_elements * sizeof(float) );
		memset(&dest[old_vertex_elements], 0, size * sizeof(float) );
		dest += m_vertex_elements;
		src += old_vertex_elements;
	}

	delete[] m_vertices;    
	m_vertices = new_vertices;

	return true;
}

bool BatchGeom::remove_attribute( const Attribute_type att_type, const unsigned int index /*= 0*/ ){
	unsigned int i, j;

	for (i = 0; i < m_formats.size(); i++) {
		if (m_formats[i].att_type == att_type && m_formats[i].index == index) {
			Format format = m_formats[i];
			std::vector<Format>::iterator it = m_formats.begin() + i;
			m_formats.erase(it);

			for (j = i; j < m_formats.size(); j++) m_formats[i].offset -= format.size;

			unsigned int newm_vertex_size = m_vertex_size - format.size * sizeof(float);
			float *newm_vertices = new float[m_num_vertices * newm_vertex_size / sizeof(float) ];

			float *dest = newm_vertices;
			float *src = m_vertices;
			memcpy(dest, src, format.offset * sizeof( float ) );
			dest += format.offset * sizeof( float );
			src += (format.offset + format.size)  * sizeof( float );
			for (j = 0; j < m_num_vertices - 1; j++) {
				memcpy(dest, src, newm_vertex_size);
				dest += newm_vertex_size;
				src += m_vertex_size;
			}
			memcpy(dest, src, (m_vertex_size - (format.offset + format.size) * sizeof(float)));

			delete[] m_vertices;
			m_vertices = newm_vertices;
			m_vertex_size = newm_vertex_size;

			return true;
		}
	}
	return false;
}

bool BatchGeom::add_normals(){
	if (m_indices == NULL) return false;
	if (m_primitive_type != PRIM_TRIANGLES) return false;    
	if (!find_attribute(ATT_NORMAL) && !insert_attribute(ATT_NORMAL, 3) ) return false;

	for(unsigned int i=0; i<m_num_indices; ++i) {
		get_normal_by_index( i ) = noVec3( 0.0f, 0.0f, 0.0f );
	}
	for(unsigned int i=0; i<m_num_indices; i+=3) {
		noVec3 v1 = get_vertex_by_index( i+0 );
		noVec3 v2 = get_vertex_by_index( i+1 );
		noVec3 v3 = get_vertex_by_index( i+2 );
		noVec3 normal = ( v2-v1).Cross(v3-v1 ).NormalizeCopy();

		get_normal_by_index( i+0 ) += normal;
		get_normal_by_index( i+1 ) += normal;
		get_normal_by_index( i+2 ) += normal;
	}
	for(unsigned int i=0; i<m_num_vertices; ++i) {
		get_normal( i ).Normalize();
	}

	return true;
}

bool BatchGeom::add_tangent_space(){
	if (m_indices == NULL) return false;
	if (!find_attribute(ATT_TEXCOORD) ) return false;    
	if (m_primitive_type != PRIM_TRIANGLES) return false;    
	if (!find_attribute(ATT_TANGENT_S) && !insert_attribute(ATT_TANGENT_S, 3) ) return false;
	if (!find_attribute(ATT_TANGENT_T) && !insert_attribute(ATT_TANGENT_T, 3) ) return false;

	for(unsigned int i=0; i<m_num_indices; ++i) {
		get_tangent_s_by_index( i ) = noVec3( 0.0f, 0.0f, 0.0f );
		get_tangent_t_by_index( i ) = noVec3( 0.0f, 0.0f, 0.0f );		
	}

	for(unsigned int i=0; i<m_num_indices; i+=3) {
		const noVec3 v1 = get_vertex_by_index( i+0 );
		const noVec3 v2 = get_vertex_by_index( i+1 );
		const noVec3 v3 = get_vertex_by_index( i+2 );
		const noVec2 w1 = get_texcoord_by_index( i+0 );
		const noVec2 w2 = get_texcoord_by_index( i+1 );
		const noVec2 w3 = get_texcoord_by_index( i+2 );

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r;

		if(fabs(s1*t2 - s2*t1) <= 0.0001f) {
			r = 1.0f;
		} else {
			r = 1.0f / (s1*t2 - s2*t1);
		}
		noVec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		noVec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		get_tangent_s_by_index( i+0 ) += sdir;
		get_tangent_s_by_index( i+1 ) += sdir;
		get_tangent_s_by_index( i+2 ) += sdir;

		get_tangent_t_by_index( i+0 ) += tdir;
		get_tangent_t_by_index( i+1 ) += tdir;
		get_tangent_t_by_index( i+2 ) += tdir;		
	}

	for(unsigned int i=0; i<m_num_vertices; ++i) {
		const noVec3 n = get_normal(i);

		// Gram-Schmidt orthogonalize
		get_tangent_t(i) = (get_tangent_t(i) - n * n * get_tangent_t(i)).NormalizeCopy();
		get_tangent_s(i) = (get_tangent_s(i) - n * n * get_tangent_s(i) - n * (n * get_tangent_t(i))).NormalizeCopy();
	}

	return true;   
}

void MD5Renderer::FreeWorld() {

}

void MD5Renderer::ClearWorld() {
	numPortalAreas = 1;
}

bool MD5Renderer::InitFromMap( const char *name ) {
	idLexer *		src;
	idToken			token;
	idStr			filename;
	idRenderModel *	lastModel;

	// if this is an empty world, initialize manually
	if ( !name || !name[0] ) {
		FreeWorld();
		mapName.Clear();
		ClearWorld();
		return true;
	}

	// load it
	filename = name;
	filename.SetFileExtension( PROC_FILE_EXT );

	FreeWorld();

	src = new idLexer( filename, LEXFL_NOSTRINGCONCAT | LEXFL_NODOLLARPRECOMPILE );
	if ( !src->IsLoaded() ) {
		//common->Printf( "idRenderWorldLocal::InitFromMap: %s not found\n", filename.c_str() );
		ClearWorld();
		return false;
	}

	mapName = name;
	
	if ( !src->ReadToken( &token ) || token.Icmp( PROC_FILE_ID ) ) {
		//common->Printf( "idRenderWorldLocal::InitFromMap: bad id '%s' instead of '%s'\n", token.c_str(), PROC_FILE_ID );
		delete src;
		return false;
	}

	int num = 0;
	// parse the file
	while ( 1 ) {
		if ( !src->ReadToken( &token ) ) {
			break;
		}

		if ( token == "model" ) {
			//lastModel = ParseModel( src );
			ParseModel( src );

			num++;
			// add it to the model manager list
			//renderModelManager->AddModel( lastModel );

			// save it in the list to free when clearing this map
			//localModels.Append( lastModel );
			continue;
		}

		if ( token == "shadowModel" ) {
			//lastModel = ParseShadowModel( src );

			// add it to the model manager list
			//renderModelManager->AddModel( lastModel );

			// save it in the list to free when clearing this map
			//localModels.Append( lastModel );
			continue;
		}

		if ( token == "interAreaPortals" ) {
			ParseInterAreaPortals( src );
			continue;
		}

		if ( token == "nodes" ) {
			ParseNodes( src );
			continue;
		}

		src->Error( "idRenderWorldLocal::InitFromMap: bad token \"%s\"", token.c_str() );
	}

	delete src;

	// if it was a trivial map without any areas, create a single area
	if ( !numPortalAreas ) {
		ClearWorld();
	}

	for (size_t i = 0; i < gameLocal.mCurrScene->Areas_.size(); ++i) 	{
		gameLocal.mCurrScene->Areas_.at(i)->CreateBuffers();
	}

	// find the points where we can early-our of reference pushing into the BSP tree
	//CommonChildrenArea_r( &areaNodes[0] );

	//AddWorldModelEntities();
	//ClearPortalStates();

	// done!
	return true;

}

PortalArea *MD5Renderer::ParseModel( idLexer *src ) {
	idToken			token;
	int				i, j;

	src->ExpectTokenString( "{" );
	// parse the name
	src->ExpectAnyToken( &token );
	int numSurfaces = src->ParseInt();
	if ( numSurfaces < 0 ) {
		src->Error( "R_ParseModel: bad numSurfaces" );
	}
		
	PortalArea* pNewArea = new PortalArea(token.c_str(), gameLocal.mCurrScene->Areas_.size());

	for(int i=0; i < numSurfaces; ++i) {
		src->ExpectTokenString( "{" );
		src->ExpectAnyToken( &token );

		BSPMesh* prim = new BSPMesh;
		prim->mat = declManager->FindMaterial( token );
		int numVerts = src->ParseInt();
		int numIndices = src->ParseInt();
		

		BatchGeom* batch = new BatchGeom;
		batch->add_format( ATT_VERTEX,   3, 0, 0);
		batch->add_format( ATT_TEXCOORD, 2, 3, 0);
		batch->add_format( ATT_NORMAL,   3, 5, 0);           

		std::vector<Vertex_doom3>*	verticeptr = new std::vector<Vertex_doom3>();                
		std::vector<unsigned short>* indiceptr = new std::vector<unsigned short>();
		verticeptr->resize( numVerts );
		indiceptr->resize( numIndices );

		std::vector<Vertex_doom3>& vertices = *verticeptr;
		std::vector<unsigned short>& indices = *indiceptr;
				
		for ( j = 0 ; j < numVerts ; j++ ) {
			float	vec[8];

			src->Parse1DMatrix( 8, vec );
			Vertex_doom3 tmp;
			tmp.vertex.x = vec[0];
			tmp.vertex.y = vec[1];
			tmp.vertex.z = vec[2];
			tmp.texcoord.x = vec[3];
			tmp.texcoord.y = vec[4];
			tmp.normal.x = vec[5];
			tmp.normal.y = vec[6];
			tmp.normal.z = vec[7];
			vertices.at(j) = tmp;
		}
		
		for( int j=0; j < numIndices; j++) {
			indices[j] = src->ParseInt();
		}
		for( int j=0; j < numIndices; j+=3) {
			noVec3 v1v0 = vertices[indices[j+1]].vertex - vertices[indices[j]].vertex;
			noVec3 v2v0 = vertices[indices[j+2]].vertex - vertices[indices[j]].vertex;
			float fDot = (v1v0.Cross(v2v0)) * vertices[indices[j]].normal;
			if ( fDot < 0) {
				int tmp = indices[j+1];
				indices[j+1] = indices[j+2];
				indices[j+2] = tmp;
			}
		}
		batch->m_startVert = prim->m_rd.endIndex;
		batch->m_endVert = batch->m_startVert + numIndices;

		prim->m_rd.vertexEnd += numVerts;
		prim->m_rd.endIndex += numIndices;


		batch->set_vertices(&vertices[0], vertices.size(), sizeof(Vertex_doom3));
		batch->set_indices(&indices[0], indices.size());

		batch->add_normals();
		batch->add_tangent_space();
		
		prim->batch = batch;

		pNewArea->mBspMeshes.push_back(prim);
		pNewArea->mBatches[prim->mat].push_back(batch);		
		src->ExpectTokenString( "}" );
	}

	gameLocal.mCurrScene->Areas_.push_back(pNewArea);

	src->ExpectTokenString( "}" );
	return pNewArea;
}

void MD5Renderer::ParseInterAreaPortals( idLexer *src ) {
	int i, j;

	src->ExpectTokenString( "{" );

	numPortalAreas = src->ParseInt();
	if ( numPortalAreas < 0 ) {
		src->Error( "R_ParseInterAreaPortals: bad numPortalAreas" );
		return;
	}
	numInterAreaPortals = src->ParseInt();
	if ( numInterAreaPortals < 0 ) {
		src->Error(  "R_ParseInterAreaPortals: bad numInterAreaPortals" );
		return;
	}

	
	for ( i = 0 ; i < numInterAreaPortals ; i++ ) {

		InterAreaPortal* portal = new InterAreaPortal;

		int		numPoints, a1, a2;
		numPoints = src->ParseInt();
		a1 = src->ParseInt();
		a2 = src->ParseInt();

		portal->m_points.resize(numPoints);
		portal->m_transformed_points.resize(numPoints);

		for (int j=0 ;j < numPoints; ++j) {
			noVec3 tmp;
			src->Parse1DMatrix( 3, tmp.ToFloatPtr());
			portal->m_points[j] = tmp;
		}

		std::stringstream name_pos;
		name_pos << "_area" << a1;
		portal->m_area_pos = gameLocal.mCurrScene->get_area_index_by_name(name_pos.str());
		std::stringstream name_neg;    
		name_neg << "_area" << a2;
		portal->m_area_neg = gameLocal.mCurrScene->get_area_index_by_name(name_neg.str());    

		if(portal->m_area_pos>=0) {
			gameLocal.mCurrScene->get_area(portal->m_area_pos)->add_portal(portal);
		}    
		if(portal->m_area_neg>=0) 
			gameLocal.mCurrScene->get_area(portal->m_area_neg)->add_portal(portal);

		gameLocal.mCurrScene->Portals_.push_back(portal);
	}
	src->ExpectTokenString( "}" );

}


void MD5Renderer::ParseNodes( idLexer *src ) {
	int			i;

	src->ExpectTokenString( "{" );

	numAreaNodes = src->ParseInt();
	if ( numAreaNodes < 0 ) {
		src->Error( "R_ParseNodes: bad numAreaNodes" );
	}
	gameLocal.mCurrScene->Nodes_.resize(numAreaNodes);

	for(int i=0; i<numAreaNodes; ++i) {
		Doom3_node node;
		node.pos_child = 

		src->Parse1DMatrix( 4, node.plane.ToFloatPtr() );
		node.pos_child = src->ParseInt();
		node.neg_child = src->ParseInt();

		if(node.pos_child<0) {
			std::stringstream name;
			name << "_area" << (-1-node.pos_child);
			node.pos_child = -1- gameLocal.mCurrScene->get_area_index_by_name(name.str());
		}
		if(node.neg_child<0) {
			std::stringstream name;
			name << "_area" << (-1-node.neg_child);
			node.neg_child = -1 - gameLocal.mCurrScene->get_area_index_by_name(name.str());
		}
		gameLocal.mCurrScene->Nodes_[i] = node;
	}

	src->ExpectTokenString( "}" );

}


void MD5Renderer::Render() {

	int start_area = gameLocal.mCurrScene->get_area( GetCamera()->GetFrom() );

	if (start_area >=0 ) {

	}
	else {
		start_area = -1-start_area;
		gameLocal.mCurrScene->get_area(start_area)->Render(GetCamera(), NULL, vec2_zero, 
			noVec2(GetApp()->getWidth(), GetApp()->getHeight()));
	}

}

void PortalArea::Render( BaseCamera* c, InterAreaPortal* portal, noVec2& min, noVec2& max )
{
	if (m_rendered[portal] == gameLocal.framenum)
		return;
	m_rendered[portal] = gameLocal.framenum;

	for(size_t i=0; i < mBspMeshes.size(); i++) {
		// render È£Ãâ 
	}

	// render portals 
	for (size_t j=0, num_portals = m_portals.size(); j < num_portals; ++j) {
		if( m_portals.at(j) != portal) {
			m_portals.at(j)->Render(c, m_index, min, max);
		}
	}
}



void PortalArea::CreateBuffers()
{
	FormatDesc vbFmt[] = {
		0, TYPE_VERTEX, FORMAT_FLOAT,  3,
		0, TYPE_NORMAL, FORMAT_FLOAT,   3,
		0, TYPE_TANGENT, FORMAT_FLOAT, 3,
		0, TYPE_BINORMAL,FORMAT_FLOAT, 3,
		0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,					
		//0, TYPE_COLOR, FORMAT_FLOAT, 4,				
	};

	m_shader = gColor;
	m_vf = gRenderer->addVertexFormat(vbFmt, elementsOf(vbFmt), m_shader);
	/*rd.vf = gRenderer->addVertexFormat(vbFmt, elementsOf(vbFmt), rd.shader);

	rd.vb = gRenderer->addVertexBuffer(sizeof(GridVertex) * numVertices	, STATIC, pVerts);
	rd.ib = gRenderer->addIndexBuffer(numIndices, sizeof(WORD), STATIC, pIndices);*/	
	std::vector<VertexPNTBUV> vertices;
	std::vector<uint16>			indices;
	VertexPNTBUV temp;
	int index = 0;
	int baseIndex = 0;
	
	std::map<const idMaterial*, std::vector<BatchGeom*> >::iterator itr = mBatches.begin();
	for (; itr != mBatches.end(); itr++) {
		std::vector<BatchGeom*> Batches =  itr->second;
		vertices.clear();
		indices.clear();
		 baseIndex = 0;
		for(size_t i=0; i < Batches.size(); i++) {
			for (size_t j=0; j < Batches.at(i)->get_vertex_count(); j++) {
				 temp.pos = Batches.at(i)->get_vertex(j);
				 temp.norm = Batches.at(i)->get_normal(j);
				 temp.uv = Batches.at(i)->get_texcoord(j);
				 temp.tangent = Batches.at(i)->get_tangent_s(j);
				 temp.binormal = Batches.at(i)->get_tangent_t(j);
				 vertices.push_back(temp);				 
			}
			uint16* index = Batches.at(i)->get_indices();
			for ( int k=0; k < Batches.at(i)->get_index_count(); k++) {
				indices.push_back(baseIndex + index[k]);
			}
			baseIndex += Batches.at(i)->get_index_count();			 
		}
		if (vertices.size() > 0) {
			m_vb = gRenderer->addVertexBuffer(sizeof(VertexPNTBUV) * vertices.size(), STATIC, &vertices[0]);
			m_ib = gRenderer->addIndexBuffer(baseIndex, sizeof(WORD), STATIC, &indices[0]);
		}
	}

}

void InterAreaPortal::Render( BaseCamera* c, int index, noVec2& min, noVec2& max ) {
	if (m_frame_rendered != gameLocal.framenum) {
		m_frame_rendered = gameLocal.framenum;

		if ( !(m_visible = CheckVisible(c)) ) {
			// portal is outside frustrum 
			return;
		}
		transform_points();		
	}

	// check visibility
	if(!m_visible) {
		return;
	} else if(m_visible<0) {
		// intersection of portal and front plane of frustum
		// set min and max to renderport
		m_transformed_min = vec2_zero;
		m_transformed_max = noVec2(GetApp()->getWidth(), GetApp()->getHeight());
	}
	// clip min and max
	if(min.x < m_transformed_min.x) min.x = m_transformed_min.x;
	if(max.x > m_transformed_max.x) max.x = m_transformed_max.x;

	if(min.y < m_transformed_min.y) min.y = m_transformed_min.y;
	if(max.y > m_transformed_max.y) max.y = m_transformed_max.y;

	// render area if visible
	if( (max.x > min.x) && (max.y > min.y) ) {
		if(index == m_area_pos) {
			gameLocal.mCurrScene->get_area( m_area_neg )->Render(c, this, min, max);
		} else {
			gameLocal.mCurrScene->get_area( m_area_pos )->Render(c, this, min, max);		}
	}
}

int InterAreaPortal::CheckVisible( BaseCamera* c )
{
	noFrustum f;

	noMat4 view(c->getViewMatrix());
	noMat4 proj(c->getProjectionMatrix());
	f.loadFrustum(ToMat4(view) * ToMat4(proj));
	
	int num_points = m_points.size();
	int i;
	Plane p;
	// check front plane.
	bool all_front = true;
	bool all_back  = true;

	for(i=0; i<num_points; ++i) { 
		p = f.getPlane(FRUSTUM_NEAR);
		vec3 point(m_points[i].ToFloatPtr());
		if (p.dist(point) > 0) 
			all_front = false;
		 else {
			all_back = false;
		}    
	}
	if(all_back) {
		return 0;
	}

	for (int j = FRUSTUM_LEFT; j < FRUSTUM_FAR; j++) {
		bool all_back = true;
		for(i=0; i < num_points; ++i) {
			p = f.getPlane(j);
			vec3 point(m_points[i].ToFloatPtr());
			if (p.dist(point) > 0)  {
				all_back = false;
				break;
			}
		}
		if(all_back) {
			return 0;
		}
	}

	return (all_front?1:-1);		
}

void InterAreaPortal::transform_points() {
	int num_points = m_points.size();
	
	m_transformed_min.Set( 99999, 99999);
	m_transformed_max.Set(-99999,-99999);

	noVec3 result;
	for(int i=0; i<num_points; ++i) {
		GetCamera()->unProject(m_points[i].x,m_points[i].y,m_points[i].z, GetApp()->getWidth(), GetApp()->getHeight(), result);
		m_transformed_points[i] = result.ToVec2();		
		// find maximum and minimum x, y and z values of transformed points to construct renderports.
		if(m_transformed_points[i].x > m_transformed_max.x) {
			m_transformed_max.x = m_transformed_points[i].x;
		} 
		if(m_transformed_points[i].x < m_transformed_min.x) {
			m_transformed_min.x = m_transformed_points[i].x;
		}

		if(m_transformed_points[i].y > m_transformed_max.y) {
			m_transformed_max.y = m_transformed_points[i].y;
		} 
		if(m_transformed_points[i].y < m_transformed_min.y) {
			m_transformed_min.y = m_transformed_points[i].y;
		}
	}
}


/*
===============
R_RemapShaderBySkin
===============
*/
const idMaterial *R_RemapShaderBySkin( const idMaterial *shader, const idDeclSkin *skin, const idMaterial *customShader ) {

	if ( !shader ) {
		return NULL;
	}

	// never remap surfaces that were originally nodraw, like collision hulls
	if ( !shader->IsDrawn() ) {
		return shader;
	}

	if ( customShader ) {
		// this is sort of a hack, but cause deformed surfaces to map to empty surfaces,
		// so the item highlight overlay doesn't highlight the autosprite surface
		if ( shader->Deform() ) {
			return NULL;
		}
		return const_cast<idMaterial *>(customShader);
	}

	if ( !skin || !shader ) {
		return const_cast<idMaterial *>(shader);
	}

	return skin->RemapShaderBySkin( shader );
}