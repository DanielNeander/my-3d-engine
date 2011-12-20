#include <U2_3D/Src/U23DLibPCH.h>
#include "U2MeshBuilder.h"
//#include <U2_3D/src/Object/U2WinFile.h>

U2MeshBuilder* U2MeshBuilder::ms_pQsortData = 0;

U2MeshBuilder::U2MeshBuilder() :
m_vertexArray(32768, 32768),
m_triangleArray(32768, 32768),
m_groupEdgeArray(32768, 32768),
m_skinCtrls(64),
m_tmInterpCtrls(64)
{
	

}

U2MeshBuilder::~U2MeshBuilder()
{
	Clear();
}

//------------------------------------------------------------------------------
/**
Count the number of triangles matching a group id and material id starting
at a given triangle index. Will stop on first triangle which doesn't
match the group id.
*/
int 
U2MeshBuilder::GetNumGroupTriangles(int groupId, int materialId, 
									int usageFlags, int startTriangleIndex) const
{
	int triIdx;
	int maxTriCnt = m_triangleArray.FilledSize();
	int numTris = 0;
	for(triIdx=startTriangleIndex; triIdx < maxTriCnt; ++triIdx)
	{
		if((m_triangleArray[triIdx]->GetGroupId() == groupId) &&
			(m_triangleArray[triIdx]->GetMaterialId() == materialId) &&
			(m_triangleArray[triIdx]->GetUsageFlags() == usageFlags))
		{
			numTris++;
		}
	}
	return numTris;
}

//------------------------------------------------------------------------------
/**
Build a group map. The triangle array must be sorted for this method
to work. For each distinctive group id, a map entry will be
created which contains the group id, the first triangle and
the number of triangles in the group.
*/
void U2MeshBuilder::BuildGroupMap(U2PrimitiveVec<Group*>& groups)
{	
	int numTriangles = m_triangleArray.FilledSize();	
	if(!numTriangles)
		return;

	int triIdx=0;	

	while(triIdx < numTriangles)
	{
		Group* newGroup = U2_NEW U2MeshBuilder::Group;
		
		const Triangle& tri = GetTriangleAt(triIdx);
		int groupId			= tri.GetGroupId();
		int matId			= tri.GetMaterialId();
		int usageFlags		= tri.GetUsageFlags();
		int numTrisInGroup	= GetNumGroupTriangles(groupId, matId, usageFlags, triIdx);
		U2ASSERT(numTrisInGroup > 0);
		newGroup->SetId(groupId);
		newGroup->SetMaterialId(matId);
		newGroup->SetUsageFlags(usageFlags);
		newGroup->SetFirstTriangle(triIdx);
		newGroup->SetNumTriangles(numTrisInGroup);
		groups.AddElem(newGroup);

		U2DynString szLog;
		szLog.Format(_T("groupid : %d matid : %d usageFlags : %d, triIndex : %d numTriangle : %d\n"),
			groupId, matId, usageFlags, triIdx, numTrisInGroup);
		FILE_LOG(logDEBUG) << szLog.Str();

		triIdx += numTrisInGroup;
	}
}

//------------------------------------------------------------------------------
/**
Update the triangle group id's, material id's and usage flags
from an existing group map.
*/
void U2MeshBuilder::UpdateTriangleIds(const U2PrimitiveVec<Group*> &groupMap)
{
	int groupIdx;
	int numGroups = groupMap.Size();
	for(groupIdx=0; groupIdx < numGroups; ++groupIdx)
	{
		const Group& group = *groupMap[groupIdx];
		int triIdx = group.GetFirstTriangle();
		int maxTriIdx = triIdx + group.GetNumTriangles();
		while(triIdx < maxTriIdx)
		{
			Triangle& tri = GetTriangleAt(triIdx++);
			tri.SetGroupId(group.GetId());
			tri.SetMaterialId(group.GetMaterialId());
			tri.SetUsageFlags(group.GetUsageFlags());
		}
	}
}

//------------------------------------------------------------------------------
/**
Clear all buffers.
*/
void U2MeshBuilder::Clear()
{
	uint32 i;
	for(i=0; i < m_vertexArray.Size(); ++i)
	{
		Vertex* pVert = m_vertexArray[i];
		U2_DELETE pVert;
	}
	m_vertexArray.RemoveAll();

	for(i=0; i < m_triangleArray.Size(); ++i)
	{
		Triangle* pTri = m_triangleArray[i];
		U2_DELETE pTri;
	}
	m_triangleArray.RemoveAll();

	//for(i=0; i < m_groupEdgeArray.Size(); ++i)
	//{
	//	GroupedEdge* pGE = m_groupEdgeArray[i];
	//	U2_DELETE pGE;
	//}
	m_groupEdgeArray.RemoveAll();	
}


//------------------------------------------------------------------------------
/**
Transform vertices in mesh, affects coordinates, normals and tangents.
*/
void U2MeshBuilder::Transform(const D3DXMATRIX& m44)
{
	// Only Rotation
	D3DXMATRIX rot(m44);
	rot._41 = rot._42 = rot._43 = 0.0f;

	D3DXVECTOR3 v;
	int numVert = m_vertexArray.Size();
	int i;

	for(i=0; i < numVert; ++i)
	{
		m_vertexArray[i]->Transform(m44, rot);
	}	
}

//------------------------------------------------------------------------------
/**	
Shadow Mesh
Build triangle neighbor data - edges:

1 - create TempEdge data, 2 vertexIndex + faceIndex, where vertexIndex1 < vertexIndex2
for every face and store in array for sorting.
[tempMEM: numFaces * 3 * 3 * ushort]

2 - sort TempEdge data by: groupID vertexIndex1 vertexIndex2 (TempEdgeSorter)

3 - create GroupedEdges, divided by groupedID, merge the 2 tempEdge halfs to a complete edge, or
marked border edges with a face index of -1;

4 - sort GroupedEdges by: GroupID faceIndex1 faceIndex2 (GroupedEdgeSorter)

5 - copy all edges to the edge array, skip identical edges (groupID/face).
*/
void U2MeshBuilder::CreateEdges()
{
	U2ASSERT(0 == GetNumEdges());
	U2ASSERT(GetNumTriangles() > 0);

	// sort the triangles, to work with the same data that is written later.
	SortTriangles();

	const int numEdges = m_triangleArray.FilledSize() * 3;
	TempEdge* pTempEdges = U2_NEW TempEdge[numEdges];
	U2ASSERT(pTempEdges);

	U2FixedPrimitiveArray<TempEdge*> pTempEdgeArray(numEdges);	

	// generate edge data from all faces
	int i;
	int e0, e1, e2;
	U2DynString szLog;
	TempEdge* pNewEdge;

	int edgeIdx = 0;
	for(i=0; i < (int)m_triangleArray.FilledSize(); ++i)
	{
		Triangle& tri = GetTriangleAt(i);
		int i0, i1, i2;
		tri.GetVertexIndices(i0, i1, i2);
		int groupId = tri.GetGroupId();

		/*szLog.Format(_T("Group id : %d\n"), groupId);
		FILE_LOG(logDEBUG) << szLog.Str();*/

		//edge i0 -i1
		U2ASSERT(edgeIdx < numEdges);
		pTempEdges[edgeIdx].m_usFIdx = (short)i;
		pTempEdges[edgeIdx].m_usVIdx[0] = i0;
		pTempEdges[edgeIdx].m_usVIdx[1] = i1;
		pTempEdges[edgeIdx].m_usGroupId = (uint16)groupId;

		pNewEdge = U2_NEW TempEdge;

		memcpy(pNewEdge, &pTempEdges[edgeIdx], sizeof(TempEdge));

		pTempEdgeArray[edgeIdx] = pNewEdge;
		
		/*szLog.Format(_T("i0i1 f : %d v0 : %d v1 : %d\n"), 
			pTempEdges[edgeIdx].m_usFIdx, pTempEdges[edgeIdx].m_usVIdx[0], pTempEdges[edgeIdx].m_usVIdx[1]);
		FILE_LOG(logDEBUG) << szLog.Str();
		*/
		e0 = edgeIdx++;		

		//edge i1 - i2
		U2ASSERT(edgeIdx < numEdges);
		pTempEdges[edgeIdx].m_usFIdx = (short)i;
		pTempEdges[edgeIdx].m_usVIdx[0] = i1;
		pTempEdges[edgeIdx].m_usVIdx[1] = i2;
		pTempEdges[edgeIdx].m_usGroupId = (uint16)groupId;
		
	/*	szLog.Format(_T("i1i2 f : %d v0 : %d v1 : %d\n"), 
			pTempEdges[edgeIdx].m_usFIdx, pTempEdges[edgeIdx].m_usVIdx[0], pTempEdges[edgeIdx].m_usVIdx[1]);
		FILE_LOG(logDEBUG) << szLog.Str();*/

		pNewEdge = U2_NEW TempEdge;

		memcpy(pNewEdge, &pTempEdges[edgeIdx], sizeof(TempEdge));

		pTempEdgeArray[edgeIdx] = pNewEdge;
		
		e1 = edgeIdx++;
		
		//edge i2 - i0
		U2ASSERT(edgeIdx < numEdges);
		pTempEdges[edgeIdx].m_usFIdx = (short)i;
		pTempEdges[edgeIdx].m_usVIdx[0] = i2;
		pTempEdges[edgeIdx].m_usVIdx[1] = i0;
		pTempEdges[edgeIdx].m_usGroupId = (uint16)groupId;
		
	/*	szLog.Format(_T("i2i0 f : %d v0 : %d v1 : %d\n"), 
			pTempEdges[edgeIdx].m_usFIdx, pTempEdges[edgeIdx].m_usVIdx[0], pTempEdges[edgeIdx].m_usVIdx[1]);
		FILE_LOG(logDEBUG) << szLog.Str();*/

		pNewEdge = U2_NEW TempEdge;

		memcpy(pNewEdge, &pTempEdges[edgeIdx], sizeof(TempEdge));

		pTempEdgeArray[edgeIdx] = pNewEdge;
		
		e2 = edgeIdx++;

		
	}

	//sort the temp edges array
	std::qsort(pTempEdges, numEdges, sizeof(TempEdge), U2MeshBuilder::TempEdgeSorter);
	//std::qsort(pTempEdgeArray[0], numEdges, sizeof(TempEdge*), U2MeshBuilder::TempEdgeSorter);

//#ifdef DEBUG
//	for( int i = 0; i < numEdges; i++)
//	{
//		const TempEdge& currEdge = *pTempEdgeArray[i];
//
//		U2DynString szLog;
//		szLog.Format(_T("f : %d v0 : %d v1 : %d\n"), 
//			currEdge.m_usFIdx, currEdge.m_usVIdx[0], currEdge.m_usVIdx[1]);
//		FILE_LOG(logDEBUG) << szLog.Str();
//	}
//#endif

	// create array for the sorted edges
	GroupedEdge* pGroupedEdges = U2_NEW GroupedEdge[numEdges];
	int groupedEdgeIdx = 0;

	// create the sorted edges array
	int realEdges = 0;
	for(i=0; i < numEdges - 1; ++i)
	{
		const TempEdge& currEdge = pTempEdges[i];
		const TempEdge& nextEdge = pTempEdges[i+1];

		if(currEdge.m_usVIdx[0] == nextEdge.m_usVIdx[1] && 
			currEdge.m_usVIdx[1] == nextEdge.m_usVIdx[0] &&
			currEdge.m_usGroupId == nextEdge.m_usGroupId)
		{
			//current edge and next edge share the same vertex indices (cross compare) - copy only once
			GroupedEdge& edge = pGroupedEdges[groupedEdgeIdx++];
			edge.m_iGroupId = currEdge.m_usGroupId;
			edge.m_usVIdx[0] = currEdge.m_usVIdx[0];
			edge.m_usVIdx[1] = currEdge.m_usVIdx[1];
			edge.m_usFIdx[0] = currEdge.m_usFIdx;
			edge.m_usFIdx[1] = nextEdge.m_usFIdx;

			szLog.Format(_T("v0: %d v1: %d f0: %d f1: %d\n"), 
				edge.m_usVIdx[0] , edge.m_usVIdx[1],  edge.m_usFIdx[0] , edge.m_usFIdx[1]);
			FILE_LOG(logDEBUG) << szLog.Str();

			++realEdges;
			//skip next element because we handled it already	
			++i;
		}
		else 
		{
			//must be a edge with only used by only one face
			GroupedEdge& edge = pGroupedEdges[groupedEdgeIdx++];
			edge.m_iGroupId = currEdge.m_usGroupId;
			edge.m_usVIdx[0] = currEdge.m_usVIdx[0];
			edge.m_usVIdx[1] = currEdge.m_usVIdx[1];
			edge.m_usFIdx[0] = currEdge.m_usFIdx;
			edge.m_usFIdx[1] = 0xffff;	// InvalidIdx	
		}
	}

	//fix last element
	const TempEdge& prevEdge = pTempEdges[numEdges - 2];
	const TempEdge& currEdge = pTempEdges[numEdges - 1];
	if(!(currEdge.m_usVIdx[0] == prevEdge.m_usVIdx[1] &&
		currEdge.m_usVIdx[1] == prevEdge.m_usVIdx[0]))
	{
		// if the last and the previous are not the same than the last must be added, else this was handled before
		//must be a edge with only used by only one face
		GroupedEdge& edge = pGroupedEdges[groupedEdgeIdx++];
		edge.m_iGroupId = currEdge.m_usGroupId;
		edge.m_usVIdx[0] = currEdge.m_usVIdx[0];
		edge.m_usVIdx[1] = currEdge.m_usVIdx[1];
		edge.m_usFIdx[0] = currEdge.m_usFIdx;
		edge.m_usFIdx[1] = 0xffff;	// InvalidIdx	
		++realEdges;
	}

	szLog.Format(_T("Real Edges : %d\n"), realEdges);
	FILE_LOG(logDEBUG) << szLog.Str();


	U2_DELETE [] pTempEdges;

	// sort grouped edges array
	std::qsort(pGroupedEdges, groupedEdgeIdx, sizeof(GroupedEdge), 
		U2MeshBuilder::GroupedEdgeSorter);


//#ifdef DEBUG
//	for( int i = 0; i < numEdges; i++)
//	{
//		const GroupedEdge& currEdge = pGroupedEdges[i];
//
//		U2DynString szLog;
//		szLog.Format(_T("groupid  : %d \n"), currEdge.m_iGroupId);
//		FILE_LOG(logDEBUG) << szLog.Str();
//	}
//#endif

	// all edges that are use from triangles with the same groupID are now in continuous chunks
	// remove duplicate edges when in the same group
	// don't skip duplicate edges when they are used from triangles with different groups

	//do the 1st element
	m_groupEdgeArray.AddElem(&pGroupedEdges[0]);
	int skipped = 0;
	for(i=1; i < groupedEdgeIdx; ++i)
	{
		const GroupedEdge& e0 = pGroupedEdges[i - 1];
		const GroupedEdge& e1 = pGroupedEdges[i];

		if(e1.m_iGroupId == e0.m_iGroupId)
		{
			if(e1.m_usFIdx[0] == e0.m_usFIdx[0])
			{
				if(e1.m_usFIdx[1] == e0.m_usFIdx[1])
				{
					if(e1.m_usVIdx[0] == e0.m_usVIdx[0])
					{
						if(e1.m_usVIdx[1] == e0.m_usVIdx[1])
						{
							++skipped;
							//skip this because it's identical to the previous
							continue;							
						}
					}
				}
			}
		}

		m_groupEdgeArray.AddElem((GroupedEdge*)&e1);
	}
	
	szLog.Format(_T("skipped   : %d  real num edges : %d \n"), skipped, m_groupEdgeArray.FilledSize());
	FILE_LOG(logDEBUG) << szLog.Str();

	//U2_DELETE [] pGroupedEdges;

}


//-------------------------------------------------------------------------------------------------
/**
qsort() hook for CreateEdges() - sort tempEdges

the compare is done cross, because if 2 faces share the same edge, than
the index order for the edge from face1 is i0 - i1,
but for face2 the order is always i1 - i0.
sort by:
- groupID
- 1st vertex index (the 1st vertex index of the tempEdge is the smaller one)
- 2nd vertex index (the 2nd vertex index of the tempEdge is the greater one)
- force a definitive order: vIndex[0]
*/
int
__cdecl
U2MeshBuilder::TempEdgeSorter(const void* elm0, const void* elm1)
{
	const TempEdge& e0 = *(TempEdge*)elm0;
	const TempEdge& e1 = *(TempEdge*)elm1;

	// group ID
	if (e0.m_usGroupId < e1.m_usGroupId)        return -1;
	else if (e0.m_usGroupId > e1.m_usGroupId)   return +1;
	else
	{
		// sort by lower index
		uint16 e0i0, e0i1;
		uint16 e1i0, e1i1;
		if (e0.m_usVIdx[0] < e0.m_usVIdx[1])
		{
			e0i0 = e0.m_usVIdx[0]; e0i1 = e0.m_usVIdx[1];
		}
		else
		{
			e0i0 = e0.m_usVIdx[1]; e0i1 = e0.m_usVIdx[0];
		}

		if (e1.m_usVIdx[0] < e1.m_usVIdx[1])
		{
			e1i0 = e1.m_usVIdx[0]; e1i1 = e1.m_usVIdx[1];
		}
		else
		{
			e1i0 = e1.m_usVIdx[1]; e1i1 = e1.m_usVIdx[0];
		}

		if (e0i0 < e1i0)            return -1;
		else if (e0i0 > e1i0)       return +1;
		else
		{
			if (e0i1 < e1i1)        return -1;
			else if (e0i1 > e1i1)   return +1;
			else
			{
				// force lower index be first
				if (e0.m_usVIdx[0] < e1.m_usVIdx[0])        return -1;
				else if (e0.m_usVIdx[0] > e1.m_usVIdx[0])   return +1;
			}
		}
	}
	return 0;
}


//------------------------------------------------------------------------------
/**
qsort() hook for CreateEdges() - sort GroupedEdge
- groupID
- 1st vertex index (the 1st vertex index of the tempEdge is the smaller one)
- 2nd vertex index (the 2nd vertex index of the tempEdge is the greater one)
- 1st face index
- 2nd face index
*/
int
__cdecl
U2MeshBuilder::GroupedEdgeSorter(const void* elm0, const void* elm1)
{
	GroupedEdge e0 = *(GroupedEdge*)elm0;
	GroupedEdge e1 = *(GroupedEdge*)elm1;

	//first sort by groupID
	if (e0.m_iGroupId < e1.m_iGroupId)        return -1;
	else if (e0.m_iGroupId > e1.m_iGroupId)   return +1;
	else
	{
		uint16 e0i0, e0i1;
		uint16 e1i0, e1i1;
		if (e0.m_usVIdx[0] < e0.m_usVIdx[1])
		{
			e0i0 = e0.m_usVIdx[0]; e0i1 = e0.m_usVIdx[1];
		}
		else
		{
			e0i0 = e0.m_usVIdx[1]; e0i1 = e0.m_usVIdx[0];
		}

		if (e1.m_usVIdx[0] < e1.m_usVIdx[1])
		{
			e1i0 = e1.m_usVIdx[0]; e1i1 = e1.m_usVIdx[1];
		}
		else
		{
			e1i0 = e1.m_usVIdx[1]; e1i1 = e1.m_usVIdx[0];
		}

		if (e0i0 < e1i0)            return -1;
		else if (e0i0 > e1i0)       return +1;
		else
		{
			if (e0i1 < e1i1)        return -1;
			else if (e0i1 > e1i1)   return +1;
			else
			{
				// sort by 1st face index
				if (e0.m_usFIdx[0] < e1.m_usFIdx[0])      return -1;
				else if (e0.m_usFIdx[0] > e1.m_usFIdx[0]) return +1;
				else
				{
					// sort by 2nd face index
					if (e0.m_usFIdx[1] < e1.m_usFIdx[1])      return -1;
					else if (e0.m_usFIdx[1] > e1.m_usFIdx[1]) return +1;
				}
			}
		}
	}
	return 0;
}


int U2MeshBuilder::VertexSorter(const void* elm0, const void* elm1)
{
	U2MeshBuilder* meshBuilder = ms_pQsortData;
	int i0 = *(int*)elm0;
	int i1 = *(int*)elm1;
	Vertex& v0 = meshBuilder->GetVertexAt(i0);
	Vertex& v1 = meshBuilder->GetVertexAt(i1);
	int ret = v0.Compare(v1);

	return ret;
}


//------------------------------------------------------------------------------
/**
This returns a value suitable for sorting, -1 if the rhs is 'smaller',
0 if rhs is equal, and +1 if rhs is 'greater'.
*/
int
U2MeshBuilder::Vertex::Compare(const Vertex& rhs) const
{
	if (this->HasComponent(VERT) && rhs.HasComponent(VERT))
	{
		int res = U2Math::Compare(m_vert, rhs.m_vert, 0.0001f);
		if (0 != res)
		{
			return res;
		}
	}
	if (this->HasComponent(NORMAL) && rhs.HasComponent(NORMAL))
	{
		int res = U2Math::Compare(m_normal,rhs.m_normal, 0.001f);
		if (0 != res)
		{
			return res;
		}
	}
	if (this->HasComponent(TANGENT) && rhs.HasComponent(TANGENT))
	{
		int res = U2Math::Compare(m_tangent, rhs.m_tangent, 0.001f);
		if (0 != res)
		{
			return res;
		}
	}
	if (this->HasComponent(BINORMAL) && rhs.HasComponent(BINORMAL))
	{
		int res = U2Math::Compare(m_binormal,rhs.m_binormal, 0.001f);
		if (0 != res)
		{
			return res;
		}
	}
	if (this->HasComponent(COLOR) && rhs.HasComponent(COLOR))
	{
		int res = U2Math::Compare(m_color,rhs.m_color, 0.001f);
		if (0 != res)
		{
			return res;
		}
	}
	if (this->HasComponent(UV0) && rhs.HasComponent(UV0))
	{
		int res = U2Math::Compare(m_uv[0],rhs.m_uv[0], 0.000001f);
		if (0 != res)
		{
			return res;
		}
	}
	if (this->HasComponent(UV1) && rhs.HasComponent(UV1))
	{
		int res = U2Math::Compare(m_uv[1],rhs.m_uv[1], 0.000001f);
		if (0 != res)
		{
			return res;
		}
	}
	if (this->HasComponent(UV2) && rhs.HasComponent(UV2))
	{
		int res =  U2Math::Compare(m_uv[2],rhs.m_uv[2], 0.000001f);
		if (0 != res)
		{
			return res;
		}
	}
	if (this->HasComponent(UV3) && rhs.HasComponent(UV3))
	{
		int res =  U2Math::Compare(m_uv[3],rhs.m_uv[3], 0.000001f);
		if (0 != res)
		{
			return res;
		}
	}
	if (this->HasComponent(BONE_WEIGHTS) && rhs.HasComponent(BONE_WEIGHTS))
	{
		int res = U2Math::Compare(m_boneWeights, rhs.m_boneWeights, 0.00001f);
		if (0 != res)
		{
			return res;
		}
	}
	if (this->HasComponent(BONE_INDICES) && rhs.HasComponent(BONE_INDICES))
	{
		int res = U2Math::Compare(m_boneIndices, rhs.m_boneIndices, 0.5f);
		if (0 != res)
		{
			return res;
		}
	}

	// fallthrough: all equal
	return 0;

}


//------------------------------------------------------------------------------
/**
Cleanup the mesh. This removes redundant vertices and optionally record
the collapse history into a client-provided collapseMap. The collapse map
contains at each new vertex index the 'old' vertex indices which have
been collapsed into the new vertex.

30-Jan-03   floh    optimizations
*/
void U2MeshBuilder::Cleanup(U2PrimitiveVec< U2PrimitiveVec<int>* >* collapseMap)
{
	int numVerts = GetNumVertices();

	// generate a index remapping table and sorted vertex array
	int* pIdxMap = U2_ALLOC(int, numVerts);
	int* pSortMap = U2_ALLOC(int, numVerts);
	int* pShiftMap = U2_ALLOC(int, numVerts);
	int i;
	for(i=0; i < numVerts; ++i)
	{
		pIdxMap[i] = i;
		pSortMap[i] = i;
	}

	// generate a sorted index map (sort by X coordinate)
	ms_pQsortData = this;
	std::qsort(pSortMap, numVerts, sizeof(int), U2MeshBuilder::VertexSorter);

//#ifdef DEBUG
//	for(int i = 0; i < numVerts; i++)
//	{
//
//		U2DynString str;
//		str.Format(_T("sorted index : %d \n"), pSortMap[i]);
//		FILE_LOG(logDEBUG) << str.Str();
//	}
//#endif
	
	// search sorted array for redundant vertices	

	int baseIdx;
	for(baseIdx = 0; baseIdx < (numVerts - 1); )
	{
		int nextIdx = baseIdx + 1;
		while((nextIdx < numVerts) && 
			(*m_vertexArray[pSortMap[baseIdx]] == *m_vertexArray[pSortMap[nextIdx]]))
		{
			// mark the vertex as invalid
			U2DynString str;
			str.Format(_T(" Redudant nextdex : %d vertexid : %d, Compared Vertex : baseidx : %d, vertex id : %d \n"), 
				nextIdx, pSortMap[nextIdx], baseIdx, pSortMap[baseIdx]);

			FILE_LOG(logDEBUG) << str.Str();

			m_vertexArray[pSortMap[nextIdx]]->SetFlag(Vertex::VF_REDUNDANT);

			// put the new valid index into the index remapping table
			pIdxMap[pSortMap[nextIdx]] = pSortMap[baseIdx];
			nextIdx++;

		//	FILE_LOG(logDEBUG) << _T("Index map : ") << pIdxMap[pSortMap[nextIdx]];
		}
		baseIdx = nextIdx;
	}

	// fill the shiftMap, this contains for each vertex index the number
	// of invalid vertices in front of it
	int numInvalid = 0;
	int vertIdx;
	for(vertIdx=0; vertIdx < numVerts; ++vertIdx)
	{
		if(m_vertexArray[vertIdx]->CheckFlag(Vertex::VF_REDUNDANT))
		{
			numInvalid++;
		}
		pShiftMap[vertIdx] = numInvalid;

		U2DynString str;
		str.Format(_T("Index : %d Shift map : %d\n"), vertIdx,  numInvalid);
		FILE_LOG(logDEBUG) <<  str.Str();
	}


	// fix the triangle's vertex indices, first, remap the old index to a
	// valid index from the indexMap, then decrement by the shiftMap entry
	// at that index (which contains the number of invalid vertices in front
	// of that index)
	// fix vertex indices in triangles
	int numTris = GetNumTriangles();
	int curTri;
	for(curTri=0; curTri < numTris; curTri++)
	{
		Triangle& tri = *m_triangleArray[curTri];
		for(i=0; i < 3; ++i)
		{
			int newIdx = pIdxMap[tri.m_iVertIdx[i]];

			tri.m_iVertIdx[i] = newIdx - pShiftMap[newIdx];
		}


		FILE_LOG(logDEBUG) << _T("i0 : ") <<  tri.m_iVertIdx[0] << _T(" i1 : ") <<  tri.m_iVertIdx[1] << _T(" i2 : ") <<  tri.m_iVertIdx[2];		
	}

	// initialize the collapse map so that for each new (collapsed)
	// index it contains a list of old vertex indices which have been
	// collapsed into the new vertex
	if(collapseMap)
	{
		for(i=0; i < numVerts; ++i)
		{
			int newIdx = pIdxMap[i];
			int collapsedIdx = newIdx - pShiftMap[newIdx];			
			
			collapseMap->GetElem(collapsedIdx)->AddElem(i);
		}
	}

	// finally, remove the redundant vertices
	numVerts = GetNumVertices();
	U2PrimitiveVec<Vertex*> newArray(numVerts, numVerts);
	for(vertIdx=0; vertIdx < numVerts; ++vertIdx)
	{
		if(!m_vertexArray[vertIdx]->CheckFlag(Vertex::VF_REDUNDANT))
		{
			Vertex *pNewVert = U2_NEW Vertex;
			memcpy_s(pNewVert, sizeof(Vertex), m_vertexArray[vertIdx], sizeof(Vertex));			
			newArray.AddElem(pNewVert);
		}
	}

	for(int i=0; i < numVerts ; ++i)
	{
		Vertex* pVert = m_vertexArray[i];
		U2_DELETE pVert;
	}

	m_vertexArray.RemoveAll();
	m_vertexArray.Resize(newArray.FilledSize());

	for(i =0; i < (int)newArray.FilledSize() ; ++i)
	{
		m_vertexArray.SetElem(i, newArray[i]);
	}

	U2_FREE(pIdxMap);
	pIdxMap = NULL;
	U2_FREE(pSortMap);
	pSortMap = NULL;
	U2_FREE(pShiftMap);
	pShiftMap = NULL;
}

//bool U2MeshBuilder::RemoveDegenerateTriangles()
//{
//	int triangleIndex;
//	int numTriangles = this->GetNumTriangles();
//	D3DXVECTOR3 v0, v1;
//	
//	
//	for (triangleIndex = 0; triangleIndex < numTriangles; triangleIndex++)
//	{
//		Triangle& tri = this->GetTriangleAt(triangleIndex);
//		int index[3];
//		tri.GetVertexIndices(index[0], index[1], index[2]);
//
//		const Vertex& vertex0 = this->GetVertexAt(index[0]);
//		const Vertex& vertex1 = this->GetVertexAt(index[1]);
//		const Vertex& vertex2 = this->GetVertexAt(index[2]);
//
//	}
//	
//
//}




int U2MeshBuilder::TriangleGroupSorter(const void* elm0, const void* elm1)
{

	Triangle* t0 = *(Triangle**) elm0;
	Triangle* t1 = *(Triangle**) elm1;


	int groupDiff = t0->GetGroupId() - t1->GetGroupId();
	if (0 != groupDiff)
	{
		return groupDiff;
	}
	int materialDiff = t0->GetMaterialId() - t1->GetMaterialId();
	if (0 != materialDiff)
	{
		return materialDiff;
	}
	int usageDiff = t0->GetUsageFlags() - t1->GetUsageFlags();
	if (0 != usageDiff)
	{
		return usageDiff;
	}

	// make the sort order definitive
	int t0i0, t0i1, t0i2, t1i0, t1i1, t1i2;
	t0->GetVertexIndices(t0i0,  t0i1, t0i2);
	t1->GetVertexIndices(t1i0, t1i1, t1i2);

#ifdef DEBUG_LOG
	U2DynString szLog;
	szLog.Format(_T("t0 : i0 : %d, i1 : %d, i2 : %d "
		"t1 : i0 : %d, i1 : %d, i2 : %d\n"), 
		t0i0, t0i1, t0i2, t1i0, t1i1, t1i2);
	FILE_LOG(logDEBUG) << szLog.Str();
#endif


	if (t0i0 < t1i0)        return -1;
	else if (t0i0 > t1i0)   return +1;
	else
	{
		if (t0i1 < t1i1)        return -1;
		else if (t0i1 > t1i1)   return +1;
		else
		{
			if (t0i2 < t1i2)        return -1;
			else if (t0i2 > t1i2)   return +1;
		}
	}

	// FIXME: For some reason it can happen that 2 triangles
	// with identical indices are exported. Make sure we
	// still get a definitive sorting order.
	// (however, duplicate triangles should not be generated
	// in the first place).
	return (t1 - t0);

}


//------------------------------------------------------------------------------
/**
Sort triangles by group.
*/
void U2MeshBuilder::SortTriangles()
{	
	std::qsort(m_triangleArray.GetBase(), GetNumTriangles(), 
		sizeof(Triangle*), U2MeshBuilder::TriangleGroupSorter);
}


void U2MeshBuilder::Optimize()
{

}
//------------------------------------------------------------------------------
/**
All indices and group id's will be incremented accordingly.
*/
int
U2MeshBuilder::Append(const U2MeshBuilder& source)
{
	FILE_LOG(logDEBUG) << _T("Append Start..");

	int baseVertexIndex = this->GetNumVertices();
	U2PrimitiveVec<Group*> groupMap;
	this->BuildGroupMap(groupMap);
	int baseGroupIndex = groupMap.Size();

	// add vertices
	int numVertices = source.GetNumVertices();
	int vertexIndex;
	for (vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
	{
		this->AddVertex(source.GetVertexAt(vertexIndex));
	}

	// add triangles	
	int numTriangles = source.GetNumTriangles();
	int triangleIndex;
	

	for (triangleIndex = 0; triangleIndex < numTriangles; triangleIndex++)
	{
		Triangle* pNewTri = U2_NEW Triangle;

		Triangle& triangle = source.GetTriangleAt(triangleIndex);

		// fix vertex indices
		triangle.m_iVertIdx[0] += baseVertexIndex;
		triangle.m_iVertIdx[1] += baseVertexIndex;
		triangle.m_iVertIdx[2] += baseVertexIndex;

		// fix group id
		triangle.m_iGroupId += baseGroupIndex;

		memcpy_s(pNewTri,  sizeof(Triangle), &triangle, sizeof(Triangle));		
		this->AddTriangle(*pNewTri);
	}

	FILE_LOG(logDEBUG) << _T("Append End...");


	return baseGroupIndex;
}


//------------------------------------------------------------------------------
/**
Replace content of this mesh with source mesh.
*/
void
U2MeshBuilder::Copy(const U2MeshBuilder& src)
{
	Clear();
	
	uint32 i;	

	for(i=0; i < src.GetNumVertices(); ++i)
	{
		Vertex* pNewVert = U2_NEW Vertex;
		pNewVert->CopyComponentFromVertex(*src.m_vertexArray[i], src.m_vertexArray[i]->m_usCompMask);
		m_vertexArray.SetElem(i, pNewVert);
	}
		

	for(i=0; i < src.GetNumTriangles(); ++i)
	{
		Triangle* pNewTri = U2_NEW Triangle;
		memcpy_s(pNewTri, sizeof(Triangle), src.m_triangleArray[i], sizeof(Triangle));
		m_triangleArray.SetElem(i, pNewTri);
	}

	for(i=0; i < src.GetNumEdges(); ++i)
	{
		GroupedEdge* pNewGE = U2_NEW GroupedEdge;
		memcpy_s(pNewGE, sizeof(GroupedEdge), src.m_groupEdgeArray[i], sizeof(GroupedEdge));
		m_groupEdgeArray.SetElem(i, pNewGE);
	}
}

//------------------------------------------------------------------------------
/**
Compute the bounding box of the mesh, filtered by a triangle group id.
*/
U2Aabb U2MeshBuilder::GetGroupBBox(int groupId) const
{
	U2Aabb box ;
	box.BeginExtend();
	int numTriangles = GetNumTriangles();
	int triangleIdx;
	for(triangleIdx = 0; triangleIdx < numTriangles; ++triangleIdx)
	{
		const Triangle& triangle = GetTriangleAt(triangleIdx);
		if(triangle.GetGroupId() == groupId)
		{
			int idx[3];
			triangle.GetVertexIndices(idx[0], idx[1], idx[2]);
			int i;
			for(i=0; i < 3; ++i)
			{
				box.Extend(this->GetVertexAt(idx[i]).GetVert());
			}
		}
	}
	box.EndExtend();
	return box;	
}


//------------------------------------------------------------------------------
/**
Compute the bounding box of the complete mesh.
*/
U2Aabb U2MeshBuilder::GetBBox() const
{
	U2Aabb box;
	box.BeginExtend();
	int numVertices = GetNumVertices();
	int vertexIdx;
	for(vertexIdx = 0; vertexIdx < numVertices; ++vertexIdx)
	{
		box.Extend(GetVertexAt(vertexIdx).GetVert());
	}
	box.EndExtend();
	return box;
}


//------------------------------------------------------------------------------
/**
Get the smallest vertex index referenced by the triangles in a group.

@param  groupId             [in] group id of group to check
@param  minVertexIndex      [out] filled with minimal vertex index
@param  maxVertexIndex      [out] filled with maximal vertex index
*/
bool
U2MeshBuilder::GetGroupVertexRange(int groupId, int& minVertexIndex, int& maxVertexIndex) const
{
	minVertexIndex = this->GetNumVertices();
	maxVertexIndex = 0;
	int numCheckedTris = 0;
	int triIndex;
	int numTriangles = this->GetNumTriangles();
	for (triIndex = 0; triIndex < numTriangles; triIndex++)
	{
		const Triangle& tri = this->GetTriangleAt(triIndex);
		if (tri.GetGroupId() == groupId)
		{
			numCheckedTris++;

			int vertexIndex[3];
			tri.GetVertexIndices(vertexIndex[0], vertexIndex[1], vertexIndex[2]);
			int i;
			for (i = 0; i < 3; i++)
			{
				if (vertexIndex[i] < minVertexIndex)  minVertexIndex = vertexIndex[i];
				if (vertexIndex[i] > maxVertexIndex)  maxVertexIndex = vertexIndex[i];
			}
		}
	}
	if (0 == numCheckedTris)
	{
		minVertexIndex = 0;
		maxVertexIndex = 0;
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
/**
Get the smallest edge index with a groupID.

@param  groupId           [in] group id of group to check
@param  minEdgeIndex      [out] filled with minimal edge index
@param  maxEdgeIndex      [out] filled with maximal edge index
*/
bool
U2MeshBuilder::GetGroupEdgeRange(int groupId, int& minEdgeIndex, int& maxEdgeIndex) const
{
	minEdgeIndex = this->GetNumEdges();
	maxEdgeIndex = 0;
	bool foundGroupStart = false;
	bool foundGroupEnd = false;
	int edgeIndex;
	int numEdges = this->GetNumEdges();

	//find first edge with this groupID
	for (edgeIndex = 0; edgeIndex < numEdges; edgeIndex++)
	{
		const GroupedEdge& edge = this->GetEdgeAt(edgeIndex);
		if (edge.m_iGroupId == groupId)
		{
			foundGroupStart = true;
			minEdgeIndex = edgeIndex;
			break;
		}
	}
	//find the begin of the next group, or the end of the edgeArray
	for (;edgeIndex < numEdges; edgeIndex++)
	{
		const GroupedEdge& edge = this->GetEdgeAt(edgeIndex);
		if (edge.m_iGroupId != groupId)
		{
			foundGroupEnd = true;
			maxEdgeIndex = edgeIndex - 1;
			break;
		}
	}

	if (foundGroupStart)
	{
		if (!foundGroupEnd)
		{
			//is the group end the array end?
			const GroupedEdge& edge = this->GetEdgeAt(numEdges-1);
			if (edge.m_iGroupId == groupId)
			{
				maxEdgeIndex = numEdges-1;
			}
		}
	}
	else
	{
		minEdgeIndex = 0;
		maxEdgeIndex = 0;
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
/**
This method will erase or create empty vertex component arrays.
New vertex component arrays will always be set to zeros.
The method can be used to make sure that a mesh file has the same
vertex size as expected by a vertex shader program.
*/
void
U2MeshBuilder::ForceVertexComponents(int wantedMask)
{
	// for each vertex...
	int numVertices = this->GetNumVertices();
	int vertexIndex = 0;
	for (vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
	{
		Vertex& vertex = this->GetVertexAt(vertexIndex);
		int hasMask = vertex.GetComponentMask();
		if (wantedMask != hasMask)
		{
			int compIndex;
			for (compIndex = 0; compIndex < Vertex::MAX_VERTEX_COMPONENTS; compIndex++)
			{
				int curMask = 1 << compIndex;
				if ((hasMask & curMask) && !(wantedMask & curMask))
				{
					// delete the vertex component
					vertex.DelComponent((Vertex::Component) curMask);
				}
				else if (!(hasMask & curMask) && (wantedMask & curMask))
				{
					// add the vertex component
					vertex.ZeroComponent((Vertex::Component) curMask);
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
/**
Copy one vertex component to another if source vertex component exists.
If source vertex component does not exist, do nothing.
*/
void
U2MeshBuilder::CopyVertexComponents(Vertex::Component from, Vertex::Component to)
{
	int numVertices = this->GetNumVertices();
	int vertexIndex = 0;
	for (vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
	{
		this->GetVertexAt(vertexIndex).CopyComponentFromComponent(from, to);
	}
}

//------------------------------------------------------------------------------
/**
Does an inflated component copy from a cleaned up source mesh to
this mesh.
*/
void
U2MeshBuilder::InflateCopyComponents(const U2MeshBuilder& src, 
									 const U2PrimitiveVec< U2PrimitiveVec<int>* >& collapseMap, 
									 int compMask)
{
	int srcIndex;
	int srcNum = src.GetNumVertices();
	for (srcIndex = 0; srcIndex < srcNum; srcIndex++)
	{
		const Vertex& srcVertex = src.GetVertexAt(srcIndex);
		int dstIndex;
		int dstNum = collapseMap[srcIndex]->Size();
		for (dstIndex = 0; dstIndex < dstNum; dstIndex++)
		{
			Vertex& dstVertex = this->GetVertexAt(collapseMap[srcIndex]->GetElem(dstIndex));
			dstVertex.CopyComponentFromVertex(srcVertex, compMask);

		}
	}
}

//------------------------------------------------------------------------------
// BSP 
/**
Clip a triangle group by a plane. All positive side triangles will
remain in the group, a new group will be created which contains
all negative side triangles.
Please make sure that all triangles have a correct group index set
before calling this method!

NOTE: new triangles will be created at the end of the triangle array.
Although this method does not depend on correct triangle ordering,
other will. It is recommended to do a PackTrianglesByGroup() and
to update all existing group triangle ranges with GetNumTrianglesInGroup()
and GetFirstTriangleInGroup().

Please note also that groups can become empty! At the end of the split
you generally want to clean up the group array and remove empty groups!

@param  clipPlane           [in] a clip plane
@param  groupId             [in] defines triangle group to split
@param  posGroupId          [in] group id to use for the positive group
@param  negGroupId          [in] group id to use for the negative group
@param  numPosTriangles     [out] resulting num of triangles in positive group
@param  numNegTriangles     [out] resulting num of triangles in negative group
*/
void
U2MeshBuilder::Split(const U2Plane& clipPlane,
					int groupId,
					int posGroupId,
					int negGroupId,
					int& numPosTriangles,
					int& numNegTriangles)
{
	

}

//------------------------------------------------------------------------------
/**
Build a vertex/triangle map. Lookup the map with the vertex index,
and find an array of indices of all triangles using that vertex.
You want to make sure to clean up the mesh before to ensure
correct vertex sharing behavior.

@param  vertexTriangleMap   2D-array to be filled with resulting map
*/
void U2MeshBuilder::BuildVertexTriangleMap(U2PrimitiveVec< U2PrimitiveVec<int>* >& vertexTriangleMap) const
{
	vertexTriangleMap.Resize(GetNumVertices());

	// iterate over triangle and record vertex/triangle mapping
	int triIdx;
	int numTris = GetNumTriangles();
	for(triIdx=0; triIdx < numTris; ++triIdx)
	{
		const Triangle& tri = GetTriangleAt(triIdx);
		int i[3];
		tri.GetVertexIndices(i[0], i[1], i[2]);
		vertexTriangleMap[i[0]]->AddElem(triIdx);
		vertexTriangleMap[i[1]]->AddElem(triIdx);
		vertexTriangleMap[i[2]]->AddElem(triIdx);
	}

}

//------------------------------------------------------------------------------
/**
Flip vertical texture coordinates.
*/
void
U2MeshBuilder::FlipUvs()
{
	int numVertices = this->GetNumVertices();
	int vertexIndex;
	D3DXVECTOR2 uv;
	for (vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
	{
		Vertex& v = this->GetVertexAt(vertexIndex);
		int layer;
		for (layer = 0; layer < 4; layer++)
		{
			if (v.HasComponent(Vertex::Component(Vertex::UV0 << layer)))
			{
				uv = v.GetUv(layer);
				uv.y = 1.0f - uv.y;
				v.SetUv(layer, uv);
			}
		}
	}
}


//------------------------------------------------------------------------------
/**
Or's the components of all vertices, and forces the whole
vertex pool to that mask. This ensures that all vertices
in the mesh builder have the same format.
*/
void
U2MeshBuilder::ExtendVertexComponents()
{
	// get or'ed mask of all vertex components
	int numVertices = this->GetNumVertices();
	int mask = 0;
	int vertexIndex;
	for (vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
	{
		Vertex& v = this->GetVertexAt(vertexIndex);
		mask |= v.GetComponentMask();
	}

	// extend all vertices to the or'ed vertex component mask
	this->ForceVertexComponents(mask);
}

//------------------------------------------------------------------------------
/**
searches the mesh for duplicated faces
*/
void U2MeshBuilder::SearchDuplicatedFaces(U2PrimitiveVec<int>& result)
{	
	int i;

	// an array which holds 1 list for each vertex. 
	// 1 lists contains references to all faces that the vertice  belongs to

	U2PrimitiveVec< U2PrimitiveVec<Triangle*>* > triangleRefs;

	U2PrimitiveVec<Triangle*> emptyDummy;
	triangleRefs.Resize(m_vertexArray.Size());

	for(i=0; i < (int)m_triangleArray.Size(); ++i)
	{
		int idx0 = m_triangleArray[i]->m_iVertIdx[0];
		int idx1 = m_triangleArray[i]->m_iVertIdx[1];
		int idx2 = m_triangleArray[i]->m_iVertIdx[2];

		// now we check if there is a triangle in the list at index0 that equals the one we are currently working on
		// the size of the list at index0 should be low for the average case
		int k;
		bool bFound = false;
		for(k=0; k < (int)triangleRefs[idx0]->Size(); ++k)
		{
			if(triangleRefs[idx0]->GetElem(k)->Equals(*m_triangleArray[i]))
			{
				bFound = true;
			}
		}

		if(bFound)
		{
			// ok, we found a triangle that is equal to the current
			result.AddElem(i);
		}
		else 
		{
			// otherwise we store a reference to this triangle in the lists at the indices
			triangleRefs[idx0]->AddElem(m_triangleArray[i]);
			triangleRefs[idx1]->AddElem(m_triangleArray[i]);
			triangleRefs[idx2]->AddElem(m_triangleArray[i]);
		}

	}	
}

//------------------------------------------------------------------------------
/**
This creates 3 unique vertices for each triangle in the mesh, generating
redundant vertices. This is the opposite operation to Cleanup(). This will
throw away any generated edges!
*/
void U2MeshBuilder::Inflate()
{
	U2PrimitiveVec<Vertex*> newVerts(GetNumTriangles() * 3, 0);
	int numTris = GetNumTriangles();
	int triIdx;
	for(triIdx=0; triIdx < numTris; ++triIdx)
	{
		// build new vertex array and fix triangle vertex indices
		Triangle& tri = GetTriangleAt(triIdx);
		int i;
		for(i=0; i < 3; ++i)
		{
			newVerts.AddElem(&GetVertexAt(tri.m_iVertIdx[i]));
			tri.m_iVertIdx[i] = triIdx * 3 + i;
		}
	}

	uint32 numVerts = m_vertexArray.Size();
	uint32 i;
	for(i=0; i < numVerts; ++i)
	{
		Vertex* pVert = m_vertexArray[i];
		U2_DELETE pVert;
	}
	m_vertexArray.RemoveAll();

	m_vertexArray.Resize(newVerts.Size());

	for(i=0; i < newVerts.FilledSize(); ++i)
	{
		m_vertexArray.SetElem(i, newVerts[i]);
	}

	// dump edge array
	for(i=0; i < m_groupEdgeArray.FilledSize(); ++i)
	{
		GroupedEdge *pE = m_groupEdgeArray[i];
		U2_DELETE pE;
	}
	m_groupEdgeArray.RemoveAll();
}

//-------------------------------------------------------------------------------------------------
bool U2MeshBuilder::Load(const TCHAR* szFilename)
{
	U2StackString fullPath(szFilename);

	if(fullPath.Contains(_T("xmesh")))
	{
		return LoadXMesh(fullPath.Str());
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
bool U2MeshBuilder::LoadFile(U2MeshLoader* pMeshLoader, const TCHAR* szFilename)
{
	U2ASSERT(pMeshLoader);
	U2ASSERT(szFilename);

	pMeshLoader->SetFilename(szFilename);
	pMeshLoader->SetIndexType(U2MeshLoader::Index16);
	if (pMeshLoader->Open())
	{
		int numGroups = pMeshLoader->GetNumGroups();
		int numVerts = pMeshLoader->GetNumVertices();
		int numIndices = pMeshLoader->GetNumIndices();
		int vertexWidth = pMeshLoader->GetVertexChannelCnt();
		int vertexComponents = pMeshLoader->GetVertexComponents();

		int vertBuffSize = numVerts * vertexWidth * sizeof(float);
		int idxBuffSize = numIndices * sizeof(USHORT);
		float* pVertBuff = U2_ALLOC(float, vertBuffSize);
		USHORT* pIdxBuff = U2_ALLOC(USHORT, idxBuffSize);

		if (!pMeshLoader->ReadVertices(pVertBuff, vertBuffSize))
		{
			return false;
		}

		if (!pMeshLoader->ReadIndices(pIdxBuff, idxBuffSize))
		{
			return false;
		}

		int vertIdx;
		float* pVert = pVertBuff;
		for (vertIdx = 0; vertIdx < numVerts; vertIdx++)
		{
			Vertex* pVertex= U2_NEW Vertex;
			Vertex& vert = *pVertex;
			if (vertexComponents & U2N2Mesh::Coord)
			{
				vert.SetVert(D3DXVECTOR3(pVert[0], pVert[1], pVert[2]));
				pVert += 3;
			}
			if (vertexComponents & U2N2Mesh::Normal)
			{
				vert.SetNormal(D3DXVECTOR3(pVert[0], pVert[1], pVert[2]));
				pVert += 3;
			}			
			if (vertexComponents & U2N2Mesh::Uv0)
			{
				vert.SetUv(0, D3DXVECTOR2(pVert[0], pVert[1]));
				pVert += 2;
			}
			if (vertexComponents & U2N2Mesh::Uv1)
			{
				vert.SetUv(1, D3DXVECTOR2(pVert[0], pVert[1]));
				pVert += 2;
			}
			if (vertexComponents & U2N2Mesh::Uv2)
			{
				vert.SetUv(2, D3DXVECTOR2(pVert[0], pVert[1]));
				pVert += 2;
			}
			if (vertexComponents & U2N2Mesh::Uv3)
			{
				vert.SetUv(3, D3DXVECTOR2(pVert[0], pVert[1]));
				pVert += 2;
			}
			if (vertexComponents & U2N2Mesh::Color)
			{
				vert.SetColor(D3DXVECTOR4(pVert[0], pVert[1], pVert[2], pVert[3]));
				pVert += 4;				
			}
			if (vertexComponents & U2N2Mesh::Tangent)
			{
				vert.SetTangent(D3DXVECTOR3(pVert[0], pVert[1], pVert[2]));
				pVert += 3;
			}
			if (vertexComponents & U2N2Mesh::Binormal)
			{
				vert.SetBinormal(D3DXVECTOR3(pVert[0], pVert[1], pVert[2]));
				pVert += 3;
			}
			if (vertexComponents & U2N2Mesh::Weights)
			{
				vert.SetWeights(D3DXVECTOR4(pVert[0], pVert[1], pVert[2], pVert[3]));
				pVert += 4;
			}
			if (vertexComponents & U2N2Mesh::JIndices)
			{
				vert.SetBoneIndices(D3DXVECTOR4(pVert[0], pVert[1], pVert[2], pVert[3]));
				pVert += 4;
			}
			AddVertex(vert);
		}
		int groupIdx;
		USHORT* pIdx = pIdxBuff;
		for (groupIdx = 0; groupIdx < numGroups; groupIdx++)
		{
			const nMeshGroup& meshGroup = pMeshLoader->GetMeshGroup(groupIdx);
			int groupFirstIdx = meshGroup.GetFirstIndex();
			int groupNumIndices = meshGroup.GetNumIndices();
			int triIdx = 0;
			int numTris = groupNumIndices / 3;
			for (triIdx = 0; triIdx < numTris; triIdx++)
			{
				Triangle *pTri = U2_NEW Triangle;
				Triangle& tri = *pTri;
				tri.SetVertexIndices(pIdx[groupFirstIdx + (triIdx * 3) + 0],
					pIdx[groupFirstIdx + (triIdx * 3) + 1], 
					pIdx[groupFirstIdx + (triIdx * 3) + 2]);
				tri.SetGroupId(groupIdx);
				AddTriangle(tri);
			}
		}
		U2_FREE(pIdxBuff);
		U2_FREE(pVertBuff);
		pMeshLoader->Close();
		return true;
	}
	return false;	
}


//-------------------------------------------------------------------------------------------------
bool U2MeshBuilder::Save(const TCHAR* szFilename)
{
	U2StackString fullPath(szFilename);

	if(fullPath.Contains(_T("xmesh")))
	{
		return SaveXMesh(fullPath.Str());
	}
	else 
	{		
		FILE_LOG(logDEBUG) << _T("U2MeshBuilder::Save(): unsupported file extension in");
		return false;
	}	
}

//-------------------------------------------------------------------------------------------------
bool U2MeshBuilder::LoadXMesh(const TCHAR* szFilename)
{
	U2MeshLoader* pMeshLoader = U2_NEW U2XMeshLoader();

	if (pMeshLoader)
	{
		bool retVal = this->LoadFile(pMeshLoader, szFilename);
		U2_DELETE pMeshLoader;
		return retVal;			
	}
	else 
	{
		return false;
	}	
}

//-------------------------------------------------------------------------------------------------
bool U2MeshBuilder::SaveXMesh(const TCHAR* szFilename)
{
	U2ASSERT(szFilename);
	
	// SkinModifer 정보를 Group에 삽입하기 위해 수정..
	// sort triangles by group id and create a group map
		
	SortTriangles();
	U2PrimitiveVec<Group*> groupMap;
	BuildGroupMap(groupMap);

	bool retval = false;
	U2WinFile* file = U2_NEW U2WinFile;
	U2ASSERT(file);

#define LINE_SIZE 1024

	TCHAR line[LINE_SIZE];
	if(file->Open(szFilename, _T("w")))
	{
		// write header
		file->PutS(_T("type xmesh\n"));
		U2Aabb meshBBox = GetBBox();
		_stprintf_s(line, LINE_SIZE, _T("boundingbox %f, %f, %f %f, %f, %f\n"),
			meshBBox.m_vMin.x, meshBBox.m_vMin.y, meshBBox.m_vMin.z, 
			meshBBox.m_vMax.y, meshBBox.m_vMax.z, meshBBox.m_vMax.z);
		file->PutS(line);

		// Write Controller 
		int skelIdx;		
		U2SkinController* pSkinCtrl = m_skinCtrls.FirstVal(&skelIdx);
		while(pSkinCtrl)
		{
			_stprintf_s(line, LINE_SIZE, _T("skincontroller %s\n"), pSkinCtrl->GetName().Str());
			file->PutS(line);			
			_stprintf_s(line, LINE_SIZE, _T("setchannel %s\n"), pSkinCtrl->GetChannel());
			file->PutS(line);
			_stprintf_s(line, LINE_SIZE, _T("setlooptype %s\n"), 
				U2AnimLoopType::ToString(pSkinCtrl->GetLoopType()).Str());
			file->PutS(line);
			_stprintf_s(line, LINE_SIZE, _T("setanim %s\n"), pSkinCtrl->GetAnim().Str());
			file->PutS(line);
			_stprintf_s(line, LINE_SIZE, _T("beginjoints %d\n"), pSkinCtrl->GetNumJoints());
			file->PutS(line);
			for(int boneIdx=0; boneIdx < pSkinCtrl->GetNumJoints(); ++boneIdx)
			{
				int parentBoneIdx;
				D3DXVECTOR3 poseTranslate;
				D3DXQUATERNION poseRot;
				D3DXVECTOR3 poseScale;
				U2DynString boneName;
				pSkinCtrl->GetJoint(boneIdx, parentBoneIdx, poseTranslate, 
					poseRot, poseScale, boneName);
				_stprintf_s(line, LINE_SIZE, _T("setjoint "
					"%d %d %f %f %f %f %f %f %f %f %f %f \"%s\"\n"), 
					boneIdx, 
					parentBoneIdx, 
					poseTranslate.x, poseTranslate.y, poseTranslate.z,
					poseRot.x, poseRot.y, poseRot.z, poseRot.w, 
					poseScale.x,poseScale.y, poseScale.z, 
					boneName.Str());

				file->PutS(line);
			}
			_stprintf_s(line, LINE_SIZE, _T("endjoints\n"));
			file->PutS(line);
			_stprintf_s(line, LINE_SIZE, _T("beginclips %d\n"), pSkinCtrl->GetNumClips());
			file->PutS(line);
			for(int clipIdx=0; clipIdx < pSkinCtrl->GetNumClips(); ++clipIdx)
			{
				const U2AnimClip& clip = pSkinCtrl->GetClip(clipIdx);
				_stprintf_s(line, LINE_SIZE, _T("setclips %d %d %s\n"), 
					clipIdx, 
					clip.GetAnimGroupIdx(),
					clip.GetClipname().IsEmpty() ? _T("\"\"") : clip.GetClipname().Str());
				file->PutS(line);
			}
			_stprintf_s(line, LINE_SIZE, _T("endclips\n"));
			file->PutS(line);

			pSkinCtrl = m_skinCtrls.NextVal(&skelIdx);
		}

		_stprintf_s(line, LINE_SIZE, _T("numGroups %d\n"), groupMap.FilledSize());
		file->PutS(line);
		_stprintf_s(line, LINE_SIZE, _T("numvertices %d\n"), GetNumVertices());
		file->PutS(line);
		_stprintf_s(line, LINE_SIZE, _T("vertexstride %d\n"), GetVertexAt(0).GetVertexStride());
		file->PutS(line);
		_stprintf_s(line, LINE_SIZE, _T("numtris %d\n"), GetNumTriangles());
		file->PutS(line);
		_stprintf_s(line, LINE_SIZE, _T("numedges %d\n"), GetNumEdges());
		file->PutS(line);

		
		// write vertex components
		_stprintf_s(line, LINE_SIZE, _T("vertexcomponent "));
		const Vertex& v = GetVertexAt(0);
		if(v.HasComponent(Vertex::VERT))
		{
			_tcscat_s(line, LINE_SIZE, _T("vert "));
		}
		if(v.HasComponent(Vertex::NORMAL))
		{
			_tcscat_s(line, LINE_SIZE, _T("normal "));
		}
		if(v.HasComponent(Vertex::UV0))
		{
			_tcscat_s(line, LINE_SIZE, _T("uv0 "));
		}
		if(v.HasComponent(Vertex::UV1))
		{
			_tcscat_s(line, LINE_SIZE, _T("uv1 "));
		}
		if(v.HasComponent(Vertex::UV2))
		{
			_tcscat_s(line, LINE_SIZE, _T("uv2 "));
		}
		if(v.HasComponent(Vertex::UV3))
		{
			_tcscat_s(line, LINE_SIZE, _T("uv3 "));
		}
		if(v.HasComponent(Vertex::COLOR))
		{
			_tcscat_s(line, LINE_SIZE, _T("color "));
		}
		if(v.HasComponent(Vertex::TANGENT))
		{
			_tcscat_s(line, LINE_SIZE, _T("tangent "));
		}
		if(v.HasComponent(Vertex::BINORMAL))
		{
			_tcscat_s(line, LINE_SIZE, _T("binormal "));
		}
		if(v.HasComponent(Vertex::BONE_WEIGHTS))
		{
			_tcscat_s(line, LINE_SIZE, _T("bone_wieghts "));
		}
		if(v.HasComponent(Vertex::BONE_INDICES))
		{
			_tcscat_s(line, LINE_SIZE, _T("bone_indices"));
		}
		_tcscat_s(line, LINE_SIZE, _T("\n"));
		file->PutS(line);

		// write groups
		uint32 i;
		for(i=0; i < groupMap.FilledSize(); ++i)
		{
			const Group& group = *groupMap[i];
			int firstTriangle = group.GetFirstTriangle();
			int numTriangles = group.GetNumTriangles();

			int minVertexIdx, maxVertexIdx;
			GetGroupVertexRange(group.GetId(), minVertexIdx, maxVertexIdx);
			// Edges
			int minEdgeIdx, maxEdgeIdx;
			GetGroupEdgeRange(group.GetId(), minEdgeIdx, maxEdgeIdx);

			_stprintf_s(line, LINE_SIZE, _T("group %d %d %d %d %d %d\n"), 
				minVertexIdx, (maxVertexIdx - minVertexIdx) + 1, 
				firstTriangle, numTriangles, minEdgeIdx, 
				(maxEdgeIdx - minEdgeIdx) + 1);
			file->PutS(line);

			// Group마다 하나의 SkinModifier를 지정한다. 
			// 만약 존재하면 groupname은 SkinModifier 내부안에 SkinController의 SetTarget 대상이 된다.
			
			if(group.GetSkinModifier())
			{
				_stprintf_s(line, LINE_SIZE, _T("groupname %s\n"), 
				group.GetSkinModifier()->GetName().Str());
				file->PutS(line);

				U2SkinModifier* pSkinMod =group.GetSkinModifier();
				_stprintf_s(line, LINE_SIZE, _T("beginfragments %d\n"), 
					pSkinMod->GetNumFragments());
				file->PutS(line);

				for(int fragIndex=0; fragIndex < pSkinMod->GetNumFragments(); ++fragIndex)
				{
					_stprintf_s(line, LINE_SIZE, _T("setfraggroupindex %d %d\n"),
						fragIndex, pSkinMod->GetFragGroupIndex(fragIndex));
					file->PutS(line);

					_stprintf_s(line, LINE_SIZE, _T("beginjointpalette %d %d\n"),
						fragIndex, pSkinMod->GetJointPaletteSize(fragIndex));
					file->PutS(line);										

					int palIndex;					
					const int numBones =  pSkinMod->GetJointPaletteSize(fragIndex);
					for (palIndex = 0; palIndex < numBones;	)
					{
						_stprintf_s(line, LINE_SIZE, _T("setjointindices %d %d "),
							fragIndex, palIndex);
						file->PutS(line);		

						int boneIndices[8];
						int lastIndex = palIndex + 8;
						for(int i = 0; palIndex < lastIndex; ++palIndex, i++)
						{
							if(palIndex < numBones)
							{
								boneIndices[i] = pSkinMod->GetJointIndex(fragIndex, palIndex);
							}
							else 
							{
								boneIndices[i] = 0;
							}

						}

						_stprintf_s(line, LINE_SIZE, _T("%d %d %d %d %d %d %d %d\n"),
							boneIndices[0], boneIndices[1], boneIndices[2], boneIndices[3],
							boneIndices[4], boneIndices[5], boneIndices[6], boneIndices[7]);
						file->PutS(line);													

					}

					_stprintf_s(line, LINE_SIZE, _T("endjointpalette %d\n"), fragIndex);
					file->PutS(line);									
				} // end for fragment		

				_stprintf_s(line, LINE_SIZE, _T("endfragments\n"));
				file->PutS(line);
			}			
		}

		// write vertices
		int numVerts = GetNumVertices();
		for(i = 0; i < (int)numVerts; ++i)
		{
			TCHAR buff[128];
			_stprintf_s(line, LINE_SIZE, _T("vertex "));
			Vertex& curVertex = GetVertexAt(i);
			if(curVertex.HasComponent(Vertex::VERT))
			{
				_stprintf_s(buff, 128, _T("%f %f %f "), 
					curVertex.m_vert.x, curVertex.m_vert.y, curVertex.m_vert.z);
				_tcscat_s(line, LINE_SIZE, buff);
			}
			if(curVertex.HasComponent(Vertex::NORMAL))
			{
				_stprintf_s(buff, 128, _T("%f %f %f "), 
					curVertex.m_normal.x, curVertex.m_normal.y, curVertex.m_normal.z);
				_tcscat_s(line, LINE_SIZE, buff);

			}
			if(curVertex.HasComponent(Vertex::UV0))
			{
				_stprintf_s(buff, 128, _T("%f %f "), 
					curVertex.m_uv[0].x, curVertex.m_uv[0].y);
				_tcscat_s(line, LINE_SIZE, buff);				
			}
			if(curVertex.HasComponent(Vertex::UV1))
			{
				_stprintf_s(buff, 128, _T("%f %f "), 
					curVertex.m_uv[1].x, curVertex.m_uv[1].y);
				_tcscat_s(line, LINE_SIZE, buff);				
			}
			if(curVertex.HasComponent(Vertex::UV2))
			{
				_stprintf_s(buff, 128, _T("%f %f "), 
					curVertex.m_uv[2].x, curVertex.m_uv[2].y);
				_tcscat_s(line, LINE_SIZE, buff);				
			}
			if(curVertex.HasComponent(Vertex::UV3))
			{
				_stprintf_s(buff, 128, _T("%f %f "), 
					curVertex.m_uv[3].x, curVertex.m_uv[3].y);
				_tcscat_s(line, LINE_SIZE, buff);				
			}
			if(curVertex.HasComponent(Vertex::COLOR))
			{
				_stprintf_s(buff, 128, _T("%f %f %f %f "), 
					curVertex.m_color.x, curVertex.m_color.y, curVertex.m_color.z, curVertex.m_color.w);
				_tcscat_s(line, LINE_SIZE, buff);
			}
			if(curVertex.HasComponent(Vertex::TANGENT))
			{
				_stprintf_s(buff, 128, _T("%f %f %f "), 
					curVertex.m_tangent.x, curVertex.m_tangent.y, curVertex.m_tangent.z);
				_tcscat_s(line, LINE_SIZE, buff);
			}
			if(curVertex.HasComponent(Vertex::BINORMAL))
			{
				_stprintf_s(buff, 128, _T("%f %f %f "), 
					curVertex.m_binormal.x, curVertex.m_binormal.y, curVertex.m_binormal.z);
				_tcscat_s(line, LINE_SIZE, buff);
			}
			if(curVertex.HasComponent(Vertex::BONE_WEIGHTS))
			{
				_stprintf_s(buff, 128, _T("%f %f %f %f "), 
					curVertex.m_boneWeights.x, curVertex.m_boneWeights.y, 
					curVertex.m_boneWeights.z, curVertex.m_boneWeights.w);
				_tcscat_s(line, LINE_SIZE, buff);
			}
			if(curVertex.HasComponent(Vertex::BONE_INDICES))
			{
				_stprintf_s(buff, 128, _T("%f %f %f %f "), 
					curVertex.m_boneIndices.x, curVertex.m_boneIndices.y, 
					curVertex.m_boneIndices.z, curVertex.m_boneIndices.w);
				_tcscat_s(line, LINE_SIZE, buff);
			}
			_tcscat_s(line, LINE_SIZE, _T("\n"));
			file->PutS(line);
		}

		// write triangles
		int numTris = GetNumTriangles();
		for(i = 0; i < numTris; ++i)
		{
			Triangle& curTri = GetTriangleAt(i);
			int i0, i1, i2;
			curTri.GetVertexIndices(i0, i1, i2);
			_stprintf_s(line, LINE_SIZE, _T("triangle %d %d %d\n"), i0, i1, i2);
			file->PutS(line);
		}

		int numEdges = GetNumEdges();
		for(i = 0; i < numEdges; ++i)
		{
			GroupedEdge& curEdge = GetEdgeAt(i);
			_stprintf_s(line, LINE_SIZE, _T("edge %d %d %d %d\n"), 
				curEdge.m_usFIdx[0], curEdge.m_usFIdx[1], curEdge.m_usVIdx[0], curEdge.m_usVIdx[1]);
			file->PutS(line);
		}

		file->Close();
		retval = true;
	}	

	U2_DELETE file;
	return retval;
}

//------------------------------------------------------------------------------
/**
Build triangle normals and tangents. The tangents require a valid
uv-mapping in texcoord layer 0.

02-Sep-03   floh    no longer generates Binormals
*/
void U2MeshBuilder::BuildTriangleNormals()
{
	// compute face normals and tangents
	int triangleIndex;
	int numTriangles = this->GetNumTriangles();
	D3DXVECTOR3 v0, v1;
	D3DXVECTOR3 uv0, uv1;
	D3DXVECTOR3 n, t, b;
	for (triangleIndex = 0; triangleIndex < numTriangles; triangleIndex++)
	{
		Triangle& tri = this->GetTriangleAt(triangleIndex);
		int index[3];
		tri.GetVertexIndices(index[0], index[1], index[2]);

		const Vertex& vertex0 = this->GetVertexAt(index[0]);
		const Vertex& vertex1 = this->GetVertexAt(index[1]);
		const Vertex& vertex2 = this->GetVertexAt(index[2]);

		// compute the face normal
		v0 = vertex1.GetVert() - vertex0.GetVert();
		v1 = vertex2.GetVert() - vertex0.GetVert();
		
		D3DXVec3Cross(&n, &v0, &v1);		
		D3DXVECTOR3 origN = n;
		D3DXVec3Normalize(&n, &origN);
		tri.SetNormal(n);

		// compute the tangents
		float x1 = vertex1.GetVert().x - vertex0.GetVert().x;
		float x2 = vertex2.GetVert().x - vertex0.GetVert().x;
		float y1 = vertex1.GetVert().y - vertex0.GetVert().y;
		float y2 = vertex2.GetVert().y - vertex0.GetVert().y;
		float z1 = vertex1.GetVert().z - vertex0.GetVert().z;
		float z2 = vertex2.GetVert().z - vertex0.GetVert().z;

		float s1 = vertex1.GetUv(0).x - vertex0.GetUv(0).x;
		float s2 = vertex2.GetUv(0).x - vertex0.GetUv(0).x;
		float t1 = vertex1.GetUv(0).y - vertex0.GetUv(0).y;
		float t2 = vertex2.GetUv(0).y - vertex0.GetUv(0).y;

		float l = (s1 * t2 - s2 * t1);
		// catch singularity
		if (l == 0.0f)
		{
			l = 0.0001f;
		}
		float r = 1.0f / l;
		D3DXVECTOR3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		D3DXVECTOR3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		// Gram-Schmidt orthogonalize		
		D3DXVECTOR3 orthoN;

		float dot;
		dot = D3DXVec3Dot(&n, &sdir);
		
		t = sdir - n * dot;
		D3DXVECTOR3 origT = t;
		D3DXVec3Normalize(&t, &origT);

		// calculate handedness
		float h;
		D3DXVECTOR3 cross;
		D3DXVec3Cross(&cross, &n, &sdir);
		dot = D3DXVec3Dot(&cross, &tdir);

		if (dot < 0.0f)
		{
			h = -1.0f;
		}
		else
		{
			h = +1.0f;
		}
		
		D3DXVec3Cross(&cross, &n, &t);	
		b = cross * h;

		// FIXME: invert binormal for Nebula2 (also need to invert green channel in normalmaps)
		b = -b;

		tri.SetTangent(t);
		tri.SetBinormal(b);
	}

}


void U2MeshBuilder::BuildVertexNormals()
{
	 unsigned int vertexIndex = 0;
	 const int numVertices = this->GetNumVertices();

	 U2PrimitiveVec< U2PrimitiveVec<int>* > triangleMap;
	 this->BuildVertexTriangleMap(triangleMap);

	 D3DXVECTOR3 avgNormal;
	 for (vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
	 {
		 avgNormal = VECTOR3_ZERO;

		 // for each triangle sharing this vertex
		 int numTris = triangleMap[vertexIndex]->Size();
		 int trisIndex;
		 for ( trisIndex = 0; trisIndex < numTris; trisIndex++)
		 {
			 const Triangle& tri = this->GetTriangleAt(triangleMap[vertexIndex]->GetElem(trisIndex));
			 avgNormal += tri.GetNormal();
		 }

		 D3DXVECTOR3 origNorm = avgNormal;
		 D3DXVec3Normalize(&avgNormal, &origNorm);

		 GetVertexAt(vertexIndex).SetNormal(avgNormal);
	 }
}

void U2MeshBuilder::BuildVertexTangents(bool allowVertexSplits)
{
	int numTris = GetNumTriangles();
	for (unsigned int i = 0; i < numTris; ++i)
	{
		Triangle& curTri = GetTriangleAt(i);
		int i0, i1, i2;
		curTri.GetVertexIndices(i0, i1, i2);

		Vertex& v0 = GetVertexAt(i0);
		Vertex& v1 = GetVertexAt(i1);
		Vertex& v2 = GetVertexAt(i2);

		//D3DXVECTOR3 e1 = v1.m_vert - v0.m_vert;
		//D3DXVECTOR3 e2 = v2.m_vert - v0.m_vert;
		//D3DXVECTOR2 e1_uv = v1.m_uv[0] - v0.m_uv[0];
		//D3DXVECTOR2 e2_uv = v2.m_uv[0] - v0.m_uv[0];

		//float cp = e1_uv.y * e2_uv.x - e1_uv.x * e2_uv.y;

		//if ( cp != 0.0f )
		//{
		//	float mul = 1.0f / cp;
		//	curTri.m_tangent = (e1 * -e2_uv.y + e2 * e1_uv.y) * mul;
		//	curTri.m_binormal = (e1 * -e2_uv.x + e2 * e1_uv.x) * mul;

		//	// Normalize tangent

		//	// Normalize binormal
		//}
		D3DXVECTOR3 tangent;
		if ( !ComputeTangent( v0.m_vert, v0.m_uv[0], v1.m_vert, v1.m_uv[0], 
			v2.m_vert, v2.m_uv[0], tangent) )
		{
			v0.SetTangent(VECTOR3_ZERO);
			v0.SetBinormal(VECTOR3_ZERO);

			v1.SetTangent(VECTOR3_ZERO);
			v1.SetBinormal(VECTOR3_ZERO);

			v2.SetTangent(VECTOR3_ZERO);
			v2.SetBinormal(VECTOR3_ZERO);

			continue;
		}

		// Project T into the tangent plane by projecting out the surface
		// normal N, and then make it unit length.
		tangent -= D3DXVec3Dot(&v0.m_normal, &tangent) * v0.m_normal;
		D3DXVECTOR3 tempTangent = tangent;
		D3DXVec3Normalize(&tangent, &tempTangent);
		v0.SetTangent(tangent);

		// Compute the bitangent B, another tangent perpendicular to T.
		D3DXVECTOR3 binormal;
		D3DXVec3Cross(&binormal, &v0.m_normal, &tangent);
		D3DXVECTOR3 tempBinormal = binormal;
		D3DXVec3Normalize(&binormal, &tempBinormal);
		v0.SetBinormal(binormal);

		// Project T into the tangent plane by projecting out the surface
		// normal N, and then make it unit length.
		tangent -= D3DXVec3Dot(&v1.m_normal, &tangent) * v1.m_normal;
		tempTangent = tangent;
		D3DXVec3Normalize(&tangent, &tempTangent);
		v1.SetTangent(tangent);

		// Compute the bitangent B, another tangent perpendicular to T.		
		D3DXVec3Cross(&binormal, &v1.m_normal, &tangent);
		tempBinormal = binormal;
		D3DXVec3Normalize(&binormal, &tempBinormal);
		v1.SetBinormal(binormal);

		// Project T into the tangent plane by projecting out the surface
		// normal N, and then make it unit length.
		tangent -= D3DXVec3Dot(&v2.m_normal, &tangent) * v2.m_normal;
		tempTangent = tangent;
		D3DXVec3Normalize(&tangent, &tempTangent);
		v2.SetTangent(tangent);


		// Compute the bitangent B, another tangent perpendicular to T.		
		D3DXVec3Cross(&binormal, &v2.m_normal, &tangent);
		tempBinormal = binormal;
		D3DXVec3Normalize(&binormal, &tempBinormal);
		v2.SetBinormal(binormal);
		
	}

	/*if (allowVertexSplits)
		this->BuildVertexTangentsWithSplits();
	else 
		this->BuildVertexTangentsWithoutSplits();*/
}


// Compute a tangent at the vertex P0.  The triangle is counterclockwise
// ordered, <P0,P1,P2>.
bool U2MeshBuilder::ComputeTangent(
	const D3DXVECTOR3& pos0, const D3DXVECTOR2& tcoord0,
	const D3DXVECTOR3& pos1, const D3DXVECTOR2& tcoord1,
	const D3DXVECTOR3& pos2, const D3DXVECTOR2& tcoord2,
	D3DXVECTOR3& tangent)
{
	// Compute the change in positions at the vertex P0.
	D3DXVECTOR3 diffP1 = pos1 - pos0;
	D3DXVECTOR3 diffP2 = pos2 - pos0;

	if(U2Math::FAbs(D3DXVec3Length(&diffP1)) < ZERO_TOLERANCE 
		|| U2Math::FAbs(D3DXVec3Length(&diffP1)) < ZERO_TOLERANCE)
	{
		// The triangle is very small, call it degenerate.
		return false;
	}

	// Compute the change in texture coordinates at the vertex P0 in the
	// direction of edge P1-P0.
	float fDiffU1 = tcoord1.x - tcoord0.x;
	float fDiffV1 = tcoord1.y - tcoord0.y;
	if(U2Math::FAbs(fDiffV1) < ZERO_TOLERANCE)
	{
		if(U2Math::FAbs(fDiffU1) < ZERO_TOLERANCE)
		{
			// The triangle effectively has no variation in the u coordinate.
			// Since the texture coordinates do not vary on this triangle,
			// treat it as a degenerate parametric surface.
			return false;
		}

		// The variation is effectively all in u, so set the tangent vector
		// to be T = dP/du.
		tangent = diffP1 / fDiffU1;
		return true;
	}
	// Compute the change in texture coordinates at the vertex P0 in the
	// direction of edge P2-P0.
	float fDiffU2 = tcoord2.x - tcoord0.x;
	float fDiffV2 = tcoord2.y - tcoord0.y;
	float fDet = fDiffV1 * fDiffU2 - fDiffV2 * fDiffU1;
	if(U2Math::FAbs(fDet) < ZERO_TOLERANCE)
	{
		// The triangle vertices are collinear in parameter space, so treat
		// this as a degenerate parametric surface.
		return false;
	}

	// The triangle vertices are not collinear in parameter space, so choose
	// the tangent to be dP/du = (dv1*dP2-dv2*dP1)/(dv1*du2-dv2*du1)
	tangent = (fDiffV1 * diffP2 - fDiffV2 * diffP1) / fDet;
	return true;
}


void U2MeshBuilder::BuildVertexTangentsWithoutSplits()
{

}

void U2MeshBuilder::BuildVertexTangentsWithSplits()
{

}










