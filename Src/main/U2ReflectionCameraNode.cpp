#include <U2_3D/src/U23DLibPCH.h>
#include "U2ReflectionCameraNode.h"
//#include "U2Math.h"

IMPLEMENT_RTTI(U2ReflectionCameraNode, U2CameraNode);

U2ReflectionCameraNode::U2ReflectionCameraNode(U2Camera* pCamera /* = 0 */)
:U2CameraNode(pCamera)
{

}

//-------------------------------------------------------------------------------------------------
void U2ReflectionCameraNode::UpdateWorldData(float fAppTime)
{
	U2Node::UpdateWorldData(fAppTime);


}
