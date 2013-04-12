#pragma once 

//#include "..\BaseTypes\BaseTypes.h"
//#include "..\Interfaces\Interface.h"
//
//#include "GameApp/ColorValue.h"
//#include "GameApp/Common.h"
//
//#include "GameApp/GraphicSystem.h"
//#include "GameApp/GraphicsScene.h"

//==============================================================================
//  Doom3_node
//==============================================================================
struct Doom3_node {
	noPlane plane;
	int pos_child, neg_child;
};

class MD5Scene 
{
public:
	MD5Scene() : m_use_portals(true) {  }
	std::vector<Doom3_node> Nodes_;    
	std::vector<class PortalArea*>	Areas_;
	std::vector<class InterAreaPortal*>	Portals_;
	int get_area_index_by_name(const std::string & name);

	PortalArea * get_area(int i) { return Areas_[i]; }    
	int get_area(const noVec3 & position);

	bool m_use_portals;
};