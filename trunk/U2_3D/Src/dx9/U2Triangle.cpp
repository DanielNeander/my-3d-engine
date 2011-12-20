#include <u2_3d/src/U23DLibPCH.h>
#include "U2Triangle.h"

IMPLEMENT_RTTI(U2Triangle, U2Mesh);

//-------------------------------------------------------------------------------------------------
U2Triangle::~U2Triangle()
{

}

//-------------------------------------------------------------------------------------------------
void U2Triangle::SetModelData(U2TriangleData* pMeshData)
{
	U2Mesh::SetModelData(pMeshData);
}

//-------------------------------------------------------------------------------------------------
U2Triangle::U2Triangle(U2TriangleData* pModelData)
	:U2Mesh(pModelData)
{

}

//-------------------------------------------------------------------------------------------------
U2Triangle::U2Triangle()
{

}

//-------------------------------------------------------------------------------------------------
void U2Triangle::SetActiveTriangleCount(unsigned short usActive)
{
	U2TriangleData* pData = SmartPtrCast(U2TriangleData, GetMeshData());

	return pData->SetActiveTriangleCount(usActive);	
}

//-------------------------------------------------------------------------------------------------
unsigned short U2Triangle::GetActiveTriangleCount() const
{
	U2TriangleData* pData = SmartPtrCast(U2TriangleData, GetMeshData());
	return pData->GetActiveTriangleCount();
}

//-------------------------------------------------------------------------------------------------
void U2Triangle::GetTriangleIndices(unsigned short i, unsigned short& i0,
						unsigned short& i1, unsigned short& i2) const
{
	U2TriangleData* pData = SmartPtrCast(U2TriangleData, GetMeshData());
	pData->GetTriangleIndices(i, i0, i1, i2);
}

//-------------------------------------------------------------------------------------------------
void U2Triangle::UpdateModelNormals()
{
	U2TriangleData* pData = SmartPtrCast(U2TriangleData, GetMeshData());
	pData->UpdateModelNormals();
}

//-------------------------------------------------------------------------------------------------
void U2Triangle::UpdateModelTangentSpace(int iTangentSpaceType)
{
	U2TriangleData* pData = SmartPtrCast(U2TriangleData, GetMeshData());
	pData->UpdateModelTangentSpace(iTangentSpaceType);
}