/**************************************************************************************************
module	:	U2MaxSceneExport
Author	:	Yun sangyong
Desc	:	ExportSkinAnimation ¹Ì±¸Çö
*************************************************************************************************/
#pragma once
#ifndef U2_MAXSCENEEXPORT_H
#define U2_MAXSCENEEXPORT_H

#include "U2MaxLib.h"


class U2MaxAnimExport;
class U2MaxMeshExport;

class U2MaxSceneExport 
{
public:
	U2MaxSceneExport();
	~U2MaxSceneExport();


	bool ExportScene(Interface* pIf, const TCHAR* szFilename);


	unsigned int CalculateTotalNodeCount(INode* pkNode);


	static Matrix3 GetLocalTM(INode* pMaxNode, TimeValue t);

	static Matrix3 UniformMatrix(Matrix3 orig_cur_mat);

	bool ExportSkinAnimation(bool bAnimOnly = false);

	void ExportGeomObj(INode* pMaxNode, U2Node* pCurNode,  Object* pObj);

	void AddMeshByType(U2MaxMeshExport* pMesh);

	
	bool ExportNodes(Interface* pIf, INode* pMaxNode, U2NodePtr* pspResult);

	void ProcessMeshBuilder(U2MaxMeshExport* pMeshExport, 
		U2MeshBuilder& meshBlder, bool bIsShadowMesh, const U2DynString& szMeshname);

		
	TimeValue m_animStart, m_animEnd;

	U2MaxAnimExport* m_pAnimExport;
	U2MeshBuilder* m_nonSkinnedMeshBuilder;
	U2MeshBuilder* m_skinnedMeshBuider;

	U2PrimitiveVec<U2MaxMeshExport*> m_nonSkinnnedMeshs;
	U2PrimitiveVec<U2MaxMeshExport*> m_skinnedMeshes;

	U2PrimitiveVec<U2MaxMeshExport*> m_nonSkinnedShadowMeshs;
	U2PrimitiveVec<U2MaxMeshExport*> m_skinnedShadowMeshs;

	

	U2NodePtr m_spScene;

	U2ObjVec<U2NodePtr>	m_aspScenes;

	static U2Filename ms_szFullPath;

private:
	bool BuildMeshTangentNormals(U2MeshBuilder& meshBuilder);


};




#endif