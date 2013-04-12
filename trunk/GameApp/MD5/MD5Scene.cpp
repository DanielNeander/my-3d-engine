#include "stdafx.h"
#include "MD5Scene.h"

//==============================================================================
//  Scene_portal::get_area_index_by_name()
//==============================================================================
int MD5Scene::get_area_index_by_name(const std::string & name) {
	int num_areas = Areas_.size();

	for(int i=0; i<num_areas; ++i) {
		if(Areas_[i]->get_name() == name) {
			return i;
		}          
	}
	return -1;
}

int MD5Scene::get_area( const noVec3 & position )
{
	if(!Nodes_.size()) {
		return 0;
	}

	// walk through nodes...
	Doom3_node *node = &Nodes_[0];
	while(true) {	
		if (node->plane.Side(position) == PLANESIDE_FRONT) {
			if( node->pos_child > 0 ) {
				node = &Nodes_[ node->pos_child ];
			} else {
				return node->pos_child;
			}        
		} else { // backside
			if( node->neg_child > 0 ) {
				node = &Nodes_[ node->neg_child ];                
			} else {
				return node->neg_child;
			}        
		}
	}
}
