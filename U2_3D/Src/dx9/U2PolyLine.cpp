#include <U2_3D/src/U23DLibPCH.h>
#include "U2PolyLine.h"


IMPLEMENT_RTTI(U2PolyLine, U2Mesh);
U2PolyLine::U2PolyLine(U2PolyLineData* pModelData) 
	:U2Mesh(pModelData)
{


}



void U2PolyLine::Render(U2Dx9Renderer* pRenderer)
{
	U2Mesh::Render(pRenderer);
	pRenderer->RenderLines(this);
}



void U2PolyLine::SetConnFlags(unsigned char* pConn)
{
	U2PolyLineData* pData = SmartPtrCast(U2PolyLineData, m_spModelData);
	pData->SetConnFlags(pConn);
}


uint8* U2PolyLine::GetConnFlags()
{
	U2PolyLineData* pData = SmartPtrCast(U2PolyLineData, m_spModelData);
	return pData->GetConnFlags();
}

const uint8* U2PolyLine::GetConnFlags() const
{
	U2PolyLineData* pData = SmartPtrCast(U2PolyLineData, m_spModelData);
	return pData->GetConnFlags();
}



