#include <U2_3D/src/U23DLibPCH.h>
#include "U2XMeshLoader.h"

//-------------------------------------------------------------------------------------------------
U2XMeshLoader::U2XMeshLoader()
{

}

//-------------------------------------------------------------------------------------------------
U2XMeshLoader::~U2XMeshLoader()
{

}




//-------------------------------------------------------------------------------------------------
bool U2XMeshLoader::Open()
{
	U2ASSERT(!m_pFile);

	m_pFile = U2_NEW U2WinFile;
	U2ASSERT(m_pFile);

	if(!m_pFile->Open(m_szFilename.Str(), _T("r")))
	{
		FDebug("U2XMeshLoader: could not open file %s", m_szFilename.Str());
		Close();
		return false;
	}

	U2Controller* pCurrCtrl = 0;

	bool bHeaderDone = false;
	TCHAR line[1024];
	TCHAR* szNextToken;
	while(!bHeaderDone && m_pFile->GetS(line, sizeof(line)))
	{
		TCHAR* szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);
		if(0 == szKeyword)
			continue;

		if(0 == _tcscmp(szKeyword, _T("type")))
		{
			const TCHAR* szType = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			U2ASSERT(szType);
			U2DynString ext(_T("xmesh"));
			ext.ToLower();
			if(0 != _tcscmp(szType, ext.Str()))
			{
				FDebug("U2XMeshLoader: In file '%s' invalid type '%s', must be 'xmesh'\n",
					m_szFilename.Str(), szType);
				Close();
				return false;
			}
		}
		else if(0 == _tcscmp(szKeyword, _T("boundingbox")))
		{
			const TCHAR* szMinX = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			const TCHAR* szMinY = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			const TCHAR* szMinZ = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			const TCHAR* szMaxX = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			const TCHAR* szMaxY = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			const TCHAR* szMaxZ = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
								
			m_bbox.m_vMin = D3DXVECTOR3((float)atof(szMinX), 
				(float)atof(szMinY), (float)atof(szMinZ));
			m_bbox.m_vMax = D3DXVECTOR3((float)atof(szMaxX), 
				(float)atof(szMaxY), (float)atof(szMaxZ));
		}
		else if(0 == _tcscmp(szKeyword, _T("skincontroller")))
		{
			const TCHAR* szSkinCtrl = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			U2ASSERT(szSkinCtrl);
			U2SkinController *pSkinCtrl = U2_NEW U2SkinController();
			pSkinCtrl->SetName(szSkinCtrl);			
			
			m_pFile->GetS(line, sizeof(line));
			szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

			if(0 == _tcscmp(szKeyword, _T("setchannel")))
			{
				const TCHAR* szCahnnel = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				pSkinCtrl->SetChannel(szCahnnel);
			}

			m_pFile->GetS(line, sizeof(line));
			szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

			if(0 == _tcscmp(szKeyword, _T("setlooptype")))
			{
				const TCHAR* szLoopType = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
				pSkinCtrl->SetLoopType(U2AnimLoopType::FromString(szLoopType));
			}

			m_pFile->GetS(line, sizeof(line));
			szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

			if(0 == _tcscmp(szKeyword, _T("setanim")))
			{
				// fullpath anim filename
				const TCHAR* szAnimname = _tcstok_s(0, U2_WHITESPACE, &szNextToken);				
				pSkinCtrl->SetAnim(szAnimname);

			}

			m_pFile->GetS(line, sizeof(line));
			szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

			if(0 == _tcscmp(szKeyword, _T("beginjoints")))
			{
				const TCHAR* szNumBones = _tcstok_s(0, U2_WHITESPACE, &szNextToken);

				pSkinCtrl->BeginJoints(atoi(szNumBones));

				for(int i=0; i < pSkinCtrl->GetNumJoints(); ++i)
				{
					m_pFile->GetS(line, sizeof(line));
					szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

					if(0 == _tcscmp(szKeyword, _T("setjoint")))
					{
						const TCHAR* szBoneIdx		 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						const TCHAR* szParentBoneIdx = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						const TCHAR* szPoseTransX	 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						const TCHAR* szPoseTransY	 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						const TCHAR* szPoseTransZ	 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						const TCHAR* szPoseRotX		 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						const TCHAR* szPoseRotY 	 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						const TCHAR* szPoseRotZ		 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						const TCHAR* szPoseRotW		 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						const TCHAR* szPoseScaleX	 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						const TCHAR* szPoseScaleY	 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						const TCHAR* szPoseScaleZ	 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);					
						
						TCHAR* token;
						token = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						U2DynString szBonename;
						do 
						{
							szBonename = szBonename + token + _T(" ");							
							token = _tcstok_s(0, U2_WHITESPACE, &szNextToken);

						} while(token &&_tcscmp(token, _T("\"")));
						szBonename.TrimRight(_T(' '));
						szBonename.Trim("\"");
						
						pSkinCtrl->SetJoint(atoi(szBoneIdx), atoi(szParentBoneIdx), 
							D3DXVECTOR3(atof(szPoseTransX), atof(szPoseTransY), atof(szPoseTransZ)),
							D3DXQUATERNION(atof(szPoseRotX), atof(szPoseRotY), atof(szPoseRotZ), atof(szPoseRotW)),
							D3DXVECTOR3(atof(szPoseScaleX), atof(szPoseScaleY), atof(szPoseScaleZ)),
							szBonename);
					}					
				}			
			}

			m_pFile->GetS(line, sizeof(line));
			szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

			if(0 == _tcscmp(szKeyword, _T("endjoints")))
			{
				pSkinCtrl->EndJoints();
			}

			m_pFile->GetS(line, sizeof(line));
			szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

			if(0 == _tcscmp(szKeyword, _T("beginclips")))
			{
				const TCHAR* szNumClips = _tcstok_s(0, U2_WHITESPACE, &szNextToken);

				pSkinCtrl->BeginClips(atoi(szNumClips));

				for(int clipidx = 0; clipidx < pSkinCtrl->GetNumClips(); ++clipidx)
				{	
					m_pFile->GetS(line, sizeof(line));
					szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

					if(0 == _tcscmp(szKeyword, _T("setclips")))
					{
						const TCHAR* szClipIdx		=  _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						const TCHAR* szAnimGroupIdx =  _tcstok_s(0, U2_WHITESPACE, &szNextToken);	
						const TCHAR* szClipname		=  _tcstok_s(0, U2_WHITESPACE, &szNextToken);	 

						pSkinCtrl->SetClip(atoi(szClipIdx), atoi(szAnimGroupIdx), szClipname);
					}
				}			
			}

			m_pFile->GetS(line, sizeof(line));
			szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

			if(0 == _tcscmp(szKeyword, _T("endclips")))
			{
				pSkinCtrl->EndClips();
			}

			pCurrCtrl = pSkinCtrl;			

			m_controllers.AddElem(pSkinCtrl);						
		}
		// NebulaImport : CreateBounceBall ÂüÁ¶..
		//else if(0 == _tcscmp(szKeyword, _T("transforminterpcontroller")))
		//{
		//	const TCHAR* szTransformController = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
		//	U2ASSERT(szTransformController);
		//	U2TransformInterpController *pTransformController = U2_NEW U2TransformInterpController();
		//	pTransformController->SetName(szTransformController);

		//	m_controllers.AddElem(pTransformController);

		//	m_pFile->GetS(line, sizeof(line));
		//	szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

		//	if(0 == _tcscmp(szKeyword, _T("setchannel")))
		//	{
		//		const TCHAR* szCahnnel = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
		//		pTransformController->SetChannel(szCahnnel);
		//	}

		//	m_pFile->GetS(line, sizeof(line));
		//	szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

		//	if(0 == _tcscmp(szKeyword, _T("setlooptype")))
		//	{
		//		const TCHAR* szLoopType = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
		//		pTransformController->SetLoopType(U2AnimLoopType::FromString(szLoopType));
		//	}

		//	m_pFile->GetS(line, sizeof(line));
		//	szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

		//	if(0 == _tcscmp(szKeyword, _T("setanim")))
		//	{
		//		// fullpath anim filename
		//		const TCHAR* szAnimname = _tcstok_s(0, U2_WHITESPACE, &szNextToken);				
		//		pTransformController->SetAnim(szAnimname);
		//	}

		//	m_pFile->GetS(line, sizeof(line));
		//	szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

		//	if(0 == _tcscmp(szKeyword, _T("setanimgroup")))
		//	{
		//		const TCHAR* szAnimGroup = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
		//		pTransformController->SetAnimGroup(atoi(szAnimGroup));
		//	}

		//	m_controllers.AddElem(pTransformController);			
		//}
		else if(0 == _tcscmp(szKeyword, _T("numGroups")))
		{
			const TCHAR* szNumGroups = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			U2ASSERT(szNumGroups);
			m_uNumGroups = atoi(szNumGroups);				
		}
		else if(0 == _tcscmp(szKeyword, _T("numvertices")))
		{
			const TCHAR* szNumVertices = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			U2ASSERT(szNumVertices);
			m_uNumVertices = atoi(szNumVertices);				
		}
		else if(0 == _tcscmp(szKeyword, _T("numedges")))
		{
			const TCHAR* szNumEdge = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			U2ASSERT(szNumEdge);
			m_uNumEdges = atoi(szNumEdge);
		}
		else if(0 == _tcscmp(szKeyword, _T("vertexcomponent")))
		{
			TCHAR* szVertexComp;
			m_uFileVertexComponents = 0;
			while(szVertexComp = _tcstok_s(0, U2_WHITESPACE, &szNextToken))
			{
				if(0 == _tcscmp(szVertexComp, _T("vert")))
				{
					m_uFileVertexComponents |= Coord;
				}
				if(0 == _tcscmp(szVertexComp, _T("normal")))
				{
					m_uFileVertexComponents |= Normal;
				}
				if(0 == _tcscmp(szVertexComp, _T("uv0")))
				{
					m_uFileVertexComponents |= Uv0;
				}
				if(0 == _tcscmp(szVertexComp, _T("uv1"))) 
				{
					m_uFileVertexComponents |= Uv1;
				}
				if(0 == _tcscmp(szVertexComp, _T("uv2")))
				{
					m_uFileVertexComponents |= Uv2;
				}
				if(0 == _tcscmp(szVertexComp, _T("uv3")))
				{
					m_uFileVertexComponents |= Uv3;
				}
				if(0 == _tcscmp(szVertexComp, _T("color")))
				{
					m_uFileVertexComponents |= Color;
				}
				if(0 == _tcscmp(szVertexComp, _T("tangent"))) 
				{
					m_uFileVertexComponents |= Tangent;
				}
				if(0 == _tcscmp(szVertexComp, _T("binormal"))) 
				{
					m_uFileVertexComponents |= Binormal;
				}
				if(0 == _tcscmp(szVertexComp, _T("bone_wieghts")))
				{
					m_uFileVertexComponents |= Weights;
				}
				if(0 == _tcscmp(szVertexComp, _T("bone_indices")))
				{
					m_uFileVertexComponents |= JIndices;
				}				
			}
		}
		else if(0 == _tcscmp(szKeyword, _T("numtris")))
		{
			const TCHAR* szNumTris = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			U2ASSERT(szNumTris);
			m_uNumTriangles = atoi(szNumTris);
			m_uNumIndices = m_uNumTriangles * 3;
		}
		else if(0 == _tcscmp(szKeyword, _T("vertexstride")))
		{
			const TCHAR* szVertexStride = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			U2ASSERT(szVertexStride);
			m_uFileVertexStride = atoi(szVertexStride);
		}
		else if(0 == _tcscmp(szKeyword, _T("group")))
		{
			const TCHAR* szFirstVert = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			const TCHAR* szNumVert	 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			const TCHAR* szFirstTris = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			const TCHAR* szNumTris	 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			const TCHAR* szFirstEdge = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
			const TCHAR* szNumEdge	 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);

			//U2ASSERT(szFirstVert && szNumVert);
			//U2ASSERT(szFirstTris && szNumTris);
			//U2ASSERT(szFirstEdge && szNumEdge);

			nMeshGroup* meshGroup = U2_NEW nMeshGroup;
			meshGroup->SetFirstVertex(atoi(szFirstVert));
			meshGroup->SetNumVertices(atoi(szNumVert));
			meshGroup->SetFirstIndex(atoi(szFirstTris) * 3);
			meshGroup->SetNumIndices(atoi(szNumTris) * 3);
			meshGroup->SetFirstEdge(atoi(szFirstEdge));
			meshGroup->SetNumEdges(atoi(szNumEdge));

			U2SkinController* pSkinCtrl = 0;

			for(unsigned int i=0; i < m_controllers.FilledSize(); ++i)
			{
				if(DynamicCast<U2SkinController>(SmartPtrCast(U2Controller, m_controllers[i])))
					pSkinCtrl = SmartPtrCast(U2SkinController,m_controllers[i]);				
				
				meshGroup->AttachController(m_controllers[i]);				
			}
						
			if(pSkinCtrl)
			{	
				U2SkinModifier* pSkinMod = U2_NEW U2SkinModifier;

				m_skinMods.AddElem(pSkinMod);

				//pSkinMod->SetSkinController((U2SkinController*)pCurrCtrl);

				meshGroup->SetSkinModifier(pSkinMod);

				m_pFile->GetS(line, sizeof(line));
				szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);


				if(0 == _tcscmp(szKeyword, _T("groupname")))
				{
					const TCHAR* szGroupname = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
					meshGroup->SetName(szGroupname);
				}


				m_pFile->GetS(line, sizeof(line));
				szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

				if(0 == _tcscmp(szKeyword, _T("beginfragments")))
				{
					const TCHAR* szNumFragments = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
					pSkinMod->BeginFragments(atoi(szNumFragments));
				}

				for(int fragIndex=0; fragIndex < pSkinMod->GetNumFragments(); ++fragIndex)
				{
					m_pFile->GetS(line, sizeof(line));
					szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

					if(0 == _tcscmp(szKeyword, _T("setfraggroupindex")))
					{
						const TCHAR* szFragmentIdx = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						const TCHAR* szFragmentGroupIdx = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						pSkinMod->SetFragGroupIndex(atoi(szFragmentIdx), atoi(szFragmentGroupIdx));					
					}

					m_pFile->GetS(line, sizeof(line));
					szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

					if(0 == _tcscmp(szKeyword, _T("beginjointpalette")))
					{
						const TCHAR* szFragmentIdx = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
						const TCHAR* szNumBonePalettesSize = _tcstok_s(0, U2_WHITESPACE, &szNextToken);

						const int numBones = atoi(szNumBonePalettesSize);
						pSkinMod->BeginJointPalette(atoi(szFragmentIdx), numBones);
					
						int palIndex;					
						for (palIndex = 0; palIndex < numBones; palIndex+=8)
						{
							m_pFile->GetS(line, sizeof(line));
							szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

							if(0 == _tcscmp(szKeyword, _T("setjointindices")))
							{
								const TCHAR* szFragmentIdx = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
								const TCHAR* szBonePaletteIdx = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
								const TCHAR* szBoneIdx0 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
								const TCHAR* szBoneIdx1 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
								const TCHAR* szBoneIdx2 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
								const TCHAR* szBoneIdx3 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
								const TCHAR* szBoneIdx4 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
								const TCHAR* szBoneIdx5 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
								const TCHAR* szBoneIdx6 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
								const TCHAR* szBoneIdx7 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);

								pSkinMod->SetJointIndices(atoi(szFragmentIdx), atoi(szBonePaletteIdx),
									atoi(szBoneIdx0), atoi(szBoneIdx1), atoi(szBoneIdx2), atoi(szBoneIdx3),
									atoi(szBoneIdx4), atoi(szBoneIdx5), atoi(szBoneIdx6), atoi(szBoneIdx7));
							}				

						} // End for bonePalette	

						m_pFile->GetS(line, sizeof(line));
						szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

						if(0 == _tcscmp(szKeyword, _T("endjointpalette")))
						{
							pSkinMod->EndJointPalette(fragIndex);
						}
					}
				} // End for fragment	

				m_pFile->GetS(line, sizeof(line));
				szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);

				if(0 == _tcscmp(szKeyword, _T("endfragments")))
				{				
					pSkinMod->EndFragments();
				}
			}


			m_meshGroups.AddElem(meshGroup);

			if(m_meshGroups.FilledSize() == m_uNumGroups)
			{
				bHeaderDone = true;
			}			
		}
	}

	// Reset 
	pCurrCtrl = 0;

	return U2MeshLoader::Open();
}

void U2XMeshLoader::Close()
{
	if(m_pFile)
	{
		if(m_pFile->IsOpen())
		{
			m_pFile->Close();
		}
		U2_DELETE m_pFile;
		m_pFile = NULL;
	}
}


bool U2XMeshLoader::ReadVertices(void *pvBuffer, int buffSize)
{
	U2ASSERT(pvBuffer);
	U2ASSERT(m_pFile);

	U2ASSERT(m_uNumVertices * m_uVertexStride * sizeof(float) == buffSize);
	float* pFloatBuffer = ((float*)pvBuffer) + (m_uNumVertices * m_uVertexStride);
	TCHAR* szNextToken;

	if(m_uVertexComponents == m_uFileVertexComponents)
	{
		U2ASSERT(m_uVertexStride == m_uFileVertexStride);
		int vertIdx;
		for(vertIdx=0; vertIdx < m_uNumVertices; ++vertIdx)
		{
			TCHAR line[1024];
			bool res = m_pFile->GetS(line, sizeof(line));
			U2ASSERT(res);

			TCHAR* szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);
			U2ASSERT(0 == _tcscmp(szKeyword, _T("vertex")));

			float* pFloat = ((float*)pvBuffer) + (vertIdx * m_uVertexStride);
			unsigned int i;
			for(i=0; i < m_uVertexStride; ++i)
			{
				const TCHAR* szCurFloat = _tcstok_s(NULL, U2_WHITESPACE, &szNextToken);
				U2ASSERT(szCurFloat);
				float curFloat = (float)atof(szCurFloat);
				pFloat[i] = curFloat;
				U2ASSERT(&(pFloat[i]) < pFloatBuffer);
			}
		}
	}
	else 
	{
		unsigned int vertIdx;
		for(vertIdx = 0; vertIdx < m_uNumVertices; ++vertIdx)
		{
			TCHAR line[1024];
			bool res = m_pFile->GetS(line, sizeof(line));
			U2ASSERT(res);

			TCHAR* szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);
			U2ASSERT(0 == _tcscmp(szKeyword, _T("vertex")));

			float* pFloat = ((float*)pvBuffer) + (vertIdx * m_uVertexStride);

			unsigned int bitIdx;
			for(bitIdx=0; bitIdx < NumVertexComponents; ++bitIdx)
			{
				int mask = (1 << bitIdx);

				if(0 == (m_uFileVertexComponents & mask))
				{
					continue;
				}

				unsigned int stride = GetVertexChannelCntFromMask(mask);
				U2ASSERT(stride > 0);
				if(m_uVertexComponents & mask)
				{
					for(unsigned int i=0; i < stride; ++i)
					{
						const TCHAR* szCurFloat = _tcstok_s(line, U2_WHITESPACE, &szNextToken);
						U2ASSERT(szCurFloat);
						float curFloat = (float)atof(szCurFloat);
						*(pFloat++) = curFloat;
						U2ASSERT(pFloat <= pFloatBuffer);

					}
				}
				else 
				{
					for(unsigned i=0; i < stride; ++i)
					{
						const TCHAR* szCurFloat = _tcstok_s(line, U2_WHITESPACE, 
							&szNextToken);
						U2ASSERT(szCurFloat);
					}
				}
			}
		}
	}
	return true;
}

bool U2XMeshLoader::ReadIndices(void* pvBuffer, int buffSize)
{
	U2ASSERT(pvBuffer);
	U2ASSERT(m_pFile);

	void* pIndicesBuffer = 0;
	if(Index16 == m_eIndexType)
	{
		U2ASSERT(buffSize == (m_uNumIndices * (int)sizeof(unsigned short)));
		pIndicesBuffer = ((unsigned short*)pvBuffer) + m_uNumIndices;
	}
	else	// 32bit type
	{
		U2ASSERT(buffSize == (m_uNumIndices * (int)sizeof(unsigned int)));
		pIndicesBuffer = ((unsigned int*)pvBuffer) + m_uNumIndices;
	}

	unsigned short* pIndices16 = (unsigned short*)pvBuffer;
	unsigned int* pIndices32 = (unsigned int*)pvBuffer;
	TCHAR* szNextToken;

	int triIdx;
	int idx = 0;
	for(triIdx = 0; triIdx < m_uNumTriangles; ++triIdx)
	{		
		TCHAR line[1024];
		bool res = m_pFile->GetS(line, sizeof(line));
		U2ASSERT(res);

		TCHAR* szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);
		U2ASSERT(0 == _tcscmp(szKeyword, _T("triangle")));

		const TCHAR* szI0 =  _tcstok_s(0, U2_WHITESPACE, &szNextToken);
		const TCHAR* szI1 =  _tcstok_s(0, U2_WHITESPACE, &szNextToken);
		const TCHAR* szI2 =  _tcstok_s(0, U2_WHITESPACE, &szNextToken);
		U2ASSERT(szI0 && szI1 && szI2);
		unsigned int i0 = atoi(szI0);
		unsigned int i1 = atoi(szI1);
		unsigned int i2 = atoi(szI2);

		if(Index16 == m_eIndexType)
		{
			pIndices16[idx++] = (unsigned short)i0;
			pIndices16[idx++] = (unsigned short)i1;
			pIndices16[idx++] = (unsigned short)i2;
			U2ASSERT(&pIndices16[idx] <= pIndicesBuffer);
		}
		else 
		{
			pIndices32[idx++] = (unsigned short)i0;
			pIndices32[idx++] = (unsigned short)i1;
			pIndices32[idx++] = (unsigned short)i2;
			U2ASSERT(&pIndices32[idx] <= pIndicesBuffer);
		}
	}

	return true;	
}

//------------------------------------------------------------------------------
/**
A edge has the size of 4 * ushort, so you have to provide a buffer with the
size numEdges * 4 * sizeof(ushort).
The edge data is: ushort vertexIndex1, vertexIndex2, faceIndex1, faceIndex2;
If a face Indicie is invalid (a border edge with only on face connected)
the value is nMeshBuilder::InvalidIndex (== -1).
*/
bool U2XMeshLoader::ReadEdges(void *pvBuffer, int buffSize)
{
	void* pEdgeBufferEnd = 0;
	U2ASSERT(buffSize == (m_uNumEdges * 4 * (int)sizeof(unsigned short)));

	pEdgeBufferEnd = ((unsigned short*)pvBuffer) + m_uNumEdges * 4;

	U2N2Mesh::Edge* pEdgeBuffer = (U2N2Mesh::Edge*)pvBuffer;

	TCHAR* szNextToken;

	uint32 edgeIdx;
	uint32 idx = 0;
	for(edgeIdx = 0; edgeIdx < m_uNumEdges; ++edgeIdx)
	{
		TCHAR line[1024];
		bool res = m_pFile->GetS(line, sizeof(line));
		U2ASSERT(res);

		TCHAR* szKeyword = _tcstok_s(line, U2_WHITESPACE, &szNextToken);
		U2ASSERT(0 == _tcscmp(szKeyword, _T("edge")));

		const TCHAR* szFace0 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
		const TCHAR* szFace1 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
		const TCHAR* szVertex0 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);
		const TCHAR* szVertex1 = _tcstok_s(0, U2_WHITESPACE, &szNextToken);

		uint16 face0 = atoi(szFace0);
		uint16 face1 = atoi(szFace1);
		uint16 vertex0 = atoi(szVertex0);
		uint16 vertex1 = atoi(szVertex1);

		pEdgeBuffer[idx].faceIdx[0] = face0;
		pEdgeBuffer[idx].faceIdx[1] = face1;
		pEdgeBuffer[idx].vertexIdx[0] = vertex0;
		pEdgeBuffer[idx].vertexIdx[1] = vertex1;
		++idx;

		U2ASSERT(&(pEdgeBuffer[idx]) <= pEdgeBufferEnd);

	}

	return true;
	
}