#include "stdafx.h"
#include <EngineCore/Util/BSP.h>
#include <EngineCore/Util/Model.h>

#include "GameApp.h"
#include "CascadedShadowsManager.h"
#include "GameApp/Mesh.h"
#include "GameApp/SceneNode.h"
#include "GameApp/M2Mesh.h"
#include "GameApp/Mesh_D3D11.h"
#include "GameApp/TestObjects.h"
#include "GameApp/GameObjectManager.h"
#include "GameApp/GameObjectUtil.h"
#include "GameApp/Common.h"
#include "ModelLib/M2Loader.h"
#include "GameApp/Actor.h"
#include "GameApp/Font/Dx11TextHelper.h"
#include "SDKMeshLoader.h"
#include "Utility.h"
#include "ShaderCompilation.h"
#include "PostProcessor.h"
#include "DeviceStates.h"

#include "Terrain/AdaptiveModelDX11Render.h"
#include "LIDRenderer.h"
#include "U2Light.h"
#include "GameApp/Light/prt.h"

#include "GameApp/Light/shlight.h"

//#include "GraphicsScene.h"

unsigned long* buffer=0;


//static GraphicsScene gScene;
//static Mesh* testMesh;
//
//static void CheckEnvInits(SHLight** lights, char** filenames, int num) {
//	for (int i = 0; i < num; i++) {
//		if (lights[i] == 0) {
//			throw("Cannot load envmap '%s'\n", filenames[i]);
//		}
//	}
//}
//
//
//static void InitEnvLights( GraphicsScene* scene )
//{
//	char** filenames = new char*[4];
//	filenames[0] = "data/grace_probe.pfm";
//	filenames[1] = "data/rnl_probe.pfm";
//	filenames[2] = "data/uffizi_probe.pfm";
//	filenames[3] = "data/galileo_probe.pfm";
//	SHLight** lights = new SHLight*[4];
//	lights[0] = LoadAndProject( filenames[0], 9 );
//	lights[1] = LoadAndProject( filenames[1], 9 );
//	lights[2] = LoadAndProject( filenames[2], 9);
//	lights[3] = LoadAndProject( filenames[3], 9);
//
//	CheckEnvInits(lights, filenames, 4);
//
//	scene->storeLcoeffs(lights[3]);
//	scene->storeLcoeffs(lights[2]);
//	scene->storeLcoeffs(lights[1]);
//	scene->storeLcoeffs(lights[0]);
//
//	const float* thecoeffs = lights[0]->GetLi();
//	DumpLi( thecoeffs, 9 );
//}
//
//static PRT* InitPRT(Mesh* pMesh, int numbands, const std::string& prtfilename,
//	U32 ourcrc,
//	bool* loadprt)
//{	
//	PRT* prtobj = new PRT(pMesh->m_iNumVertices, numbands);
//	//try to load precomputed PRT:
//	*loadprt = ReadPRTCoefficients( prtfilename.c_str(), prtobj, ourcrc );
//	if (!*loadprt) {
//		int numsamples = 100;		//# of rays per sample		
//		prtobj->projectShadow(gScene, numsamples, pMesh, ourcrc); //calc shadow coeffs
//		WritePRTCoefficients( prtfilename.c_str(), prtobj, ourcrc );
//		*loadprt = true;
//	}
//	return prtobj;
//}
//
//void App::InitPRT()
//{
//	int numsamples = 256;
//	int numbands = 9; //# of SH coeffs to use
//		
//	const char* prtfilename;
//	int ourcrc = 0;
//	
//	gScene.BuildKDTree();
//
//	InitEnvLights(&gScene);
//	bool loadprt = false;	
//	//PRT* prtobj = ::InitPRT(testMesh, numbands, prtfilename, ourcrc, &loadprt );
//
//
//}