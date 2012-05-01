/************************************************************************
module	:	U2Scene
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma  once
#ifndef	U2_SCENE_H
#define	U2_SCENE_H


//#include <windows.h>
//#include <assert.h>
//#include <tchar.h>
//#include <commctrl.h> // for InitCommonControls() 
//
//#include <U2Lib/Src/U2Lib.h>
//#include <U2_3D/Src/U2_3DLib.h>
//#include <U2lib/Src/U2DataType.h>
//
//#include <U2_3D/Src/Main/U2Object.h>
//#include <U2_3D/Src/main/U2Node.h>
//#include <U2_3D/Src/main/U2VariableMgr.h>
//#include <U2_3D/Src/main/U2RenderContext.h>

class U2_3D U2Scene : public U2Object 
{
public:
	
	virtual ~U2Scene();

	static U2Scene* Create();

	bool Initialize();


	void Trigger(float fAppTime, unsigned int frameId);
	void TransferGlobalVars(U2RenderContext& context, float time, unsigned int frameId);

	void AddDefaultLight();

	inline U2LightNode* GetDefaultLight() const { return m_pDefaultLitNode; }
	inline U2Node*	GetSceneNode() const { return m_spRootNode; }

	U2RenderContext*  AddRenderContext(const U2DynString& szName);

	void AddObject(U2Spatial* pSpatial);

	void LoadObject(const U2DynString& szName);

	void UpdateRenderContext();

private:
	U2Scene();

	void AddDefaultVariables(U2RenderContext& context);
	
	U2ObjVec<U2RenderContextPtr> m_aspRenderContexts;	
	U2ObjVec<U2NodePtr> m_aspNodes;
	U2NodePtr m_spRootNode;
	U2Variable::Handle m_timeHandle;

	U2LightNode* m_pDefaultLitNode;
};

typedef U2SmartPtr<U2Scene> U2ScenePtr;

#endif 







