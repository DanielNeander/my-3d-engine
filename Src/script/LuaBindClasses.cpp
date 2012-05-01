#include <U2_3D/Src/U23DLibPCH.h>
#include "luabindclasses.h"

using namespace luabind;


static U2N2Mesh* NewMesh()
{
	U2TriListData* pData = U2_NEW U2TriListData;

	U2N2Mesh* pMesh = U2_NEW U2N2Mesh(pData);

	return pMesh;
}
   
static void SetObjectName(U2Object* pTarget, const TCHAR* szName)
{
	pTarget->SetName(szName);
}

static U2MeshLoader* NewXMeshLoader()
{
	U2MeshLoader* pLoader = U2_NEW U2XMeshLoader;
	return pLoader;
}

static void DeleteMeshLoader(U2MeshLoader* pLoader)
{
	U2_DELETE pLoader;
}

static U2D3DXEffectShader* GetShader(const TCHAR* shadername)
{
	U2FrameShader& frameShader = SceneMgr->GetFrame()->GetShader(shadername);

	return frameShader.GetShader();
}


static U2Dx9FxShaderEffect* NewFxShaderEffect(U2D3DXEffectShader* pShader)
{
	U2ASSERT(U2Dx9Renderer::GetRenderer());
	return U2_NEW U2Dx9FxShaderEffect(pShader, U2Dx9Renderer::GetRenderer());
}


static U2DX9FxMultiLayerShaderEffect* NewFxMultiLayeredEffect(U2D3DXEffectShader* pShader)
{
	U2ASSERT(U2Dx9Renderer::GetRenderer());
	return U2_NEW U2DX9FxMultiLayerShaderEffect(pShader, U2Dx9Renderer::GetRenderer());
}


void RegisterGeneralWithLua(lua_State* pLua)
{
	module(pLua)
		[
			def("ShaderParam", &U2FxShaderState::StringToParam)
		];
}


void RegisterShaderFuncsWithLua(lua_State* pLua)
{
	module(pLua)
		[
			class_<U2Shader, bases<U2Object> >("U2Shader")
		];

	module(pLua)
		[
			class_<U2D3DXEffectShader, bases<U2Shader> >("U2D3DXEffectShader")
		
		];

	module(pLua)
		[
			class_<U2FrameShader, bases<U2Object> >("U2FrameShader")			
			
		];

	module(pLua)
		[
			def("GetShader", &GetShader)		
		];	
	module(pLua)
		[
				def("NewFxShaderEffect", &NewFxShaderEffect)
		];
	module(pLua)
		[
			def("NewFxMultiLayeredEffect", &NewFxMultiLayeredEffect)
		];
}

void RegisterTriangleWithLua(lua_State* pLua)
{
	module(pLua)
		[
			class_<U2Mesh, bases<U2Spatial> >("U2Mesh")

			.def("UpdateMeshState", &U2Mesh::UpdateMeshState)
		];

	module(pLua)
		[
			class_<U2Triangle, bases<U2Mesh> >("U2Triangle")

		];
}

void RegisterMeshWithLua(lua_State* pLua)
{
	
	module(pLua)
		[
			class_<U2N2Mesh, bases<U2Triangle> >("U2N2Mesh")
			
			.def("LightOnOff", &U2N2Mesh::LightOnOff)
			.def("LoadFile", &U2N2Mesh::LoadFile)
			.def("GetNumGroups", &U2N2Mesh::GetNumGroups)
			.def("Group", &U2N2Mesh::Group)			
		];	
	module(pLua)
		[
			def("NewMesh", NewMesh)
		];
}

void AttachEffectFunc(nMeshGroup* pOwner, U2Effect* pEffect )
{
	pOwner->AttachEffect(pEffect);
}

void RegisterMeshGroupWithLua(lua_State* pLua)
{
	module(pLua)
		[
			//class_<nMeshGroup, bases<U2Mesh> >("nMeshGroup")						
			class_<nMeshGroup, U2Mesh>("nMeshGroup")						
			
			//.def("AttachEffectFunc", AttachEffectFunc)
			//.def("UpdateMeshState", &nMeshGroup::UpdateMeshState)
		];
}

// Fake member function for simplifing binding, as the real functions 
// have optional aguments, which I don't want to use in the Lua script.
// However luabind does not support optional arguments.
// Think of "obj" as "this"



void RegisterSpatialWithLua(lua_State* pLua)
{
	module(pLua)
		[
			//class_<U2Spatial, bases<U2Object> >("U2Spatial")
			class_<U2Spatial, U2Object>("U2Spatial")

			.def("AttachEffect", &U2Spatial::AttachEffect)
			.def("SetLocalTrans", (void(U2Spatial::*)(const D3DXVECTOR3&)) &U2Spatial::SetLocalTrans)
			.def("SetLocalTrans", (void(U2Spatial::*)(float, float, float)) &U2Spatial::SetLocalTrans)
			.def("SetLocalScale", (void(U2Spatial::*)(float)) &U2Spatial::SetLocalScale)
			.def("SetLocalScale", (void(U2Spatial::*)(float, float, float)) &U2Spatial::SetLocalScale)

			.def("UpdateBound", &U2Spatial::UpdateBound)
		];
}

void RegisterMeshLoaderWithLua(lua_State* pLua)
{
	module(pLua)
		[
			class_<U2MeshLoader, bases<U2RefObject> >("U2MeshLoader")

			.def("SetFilename", &U2MeshLoader::SetFilename)
		];

	module(pLua)
		[
			class_<U2XMeshLoader, bases<U2MeshLoader> >("U2XMeshLoader")			
			
		];


	module(pLua)
		[
			def("NewXMeshLoader", &NewXMeshLoader)
		];

	module(pLua)
		[
			def("DeleteMeshLoader", &DeleteMeshLoader)
		];
		
}

void RegisterNodeWithLua(lua_State* pLua)
{
	module(pLua)
		[
			class_<U2Node, bases<U2Spatial> >("U2Node")

			.def("AttachChild", &U2Node::AttachChild)

		];
}

void RegisterObjectWithLua(lua_State* pLua)
{
	module(pLua)
		[
			class_<U2RefObject>("U2RefObject")
		];			

	module(pLua)
		[
			class_<U2Object, bases<U2RefObject> >("U2Object")
			
			.def("SetName", &U2Object::SetName)
			.def("GetName", &U2Object::GetName)
			.def("GetNumControllers", &U2Object::GetNumControllers)
			.def("AttachController", &U2Object::AttachController)		
		];
	module(pLua)
		[
			def("SetObjectName", SetObjectName)
		];
}

void RegisterControllerWithLua(lua_State* pLua)
{
	module(pLua)
		[
			class_<U2Controller, bases<U2Object> >("U2Controller")

			.def("SetLoopType", &U2Controller::SetLoopType)
		];
}


void RegisterTransformInterpControllerWithLua(lua_State* pLua)
{
	module(pLua)
		[
			class_<U2TransformInterpController, bases<U2Controller> >("U2TransformInterpController")

			.def("SetAnim", &U2TransformInterpController::SetAnim)
			.def("SetAnimGroup", &U2TransformInterpController::SetAnimGroup)
		];
}


void RegisterShaderEffectWithLua(lua_State* pLua)
{
	module(pLua)
		[
			class_<U2Effect, bases<U2Object> >("U2Effect")
		];

	module(pLua)
		[
			class_<U2Dx9FxShaderEffect, bases<U2Effect> >("U2Dx9FxShaderEffect")

			.def("SetInt", (void(U2Dx9FxShaderEffect::*)(const TCHAR*, int))&U2Dx9FxShaderEffect::SetInt)
			.def("SetInt", (void(U2Dx9FxShaderEffect::*)(U2FxShaderState::Param param, int))&U2Dx9FxShaderEffect::SetInt)
			.def("SetFloat", (void(U2Dx9FxShaderEffect::*)(const TCHAR*, float))&U2Dx9FxShaderEffect::SetFloat)
			.def("SetFloat", (void(U2Dx9FxShaderEffect::*)(U2FxShaderState::Param param, float))&U2Dx9FxShaderEffect::SetFloat)
			.def("SetVector", (void(U2Dx9FxShaderEffect::*)
			(U2FxShaderState::Param param, const D3DXVECTOR4&))&U2Dx9FxShaderEffect::SetVector)
			.def("SetVector", (void(U2Dx9FxShaderEffect::*)(
			U2FxShaderState::Param param, float, float, float, float))&U2Dx9FxShaderEffect::SetVector)
			.def("SetVector", (void(U2Dx9FxShaderEffect::*)(
			const TCHAR*, const D3DXVECTOR4&))&U2Dx9FxShaderEffect::SetVector)
			.def("SetVector", (void(U2Dx9FxShaderEffect::*)(
			const TCHAR*, float, float, float, float))&U2Dx9FxShaderEffect::SetVector)
			
			.def("SetBool", (void(U2Dx9FxShaderEffect::*)(const TCHAR*, bool))&U2Dx9FxShaderEffect::SetBool)
			.def("SetBool", (void(U2Dx9FxShaderEffect::*)(U2FxShaderState::Param param, bool))&U2Dx9FxShaderEffect::SetBool)
			.def("SetTexture", (void(U2Dx9FxShaderEffect::*)(const TCHAR*, const TCHAR*)) &U2Dx9FxShaderEffect::SetTexture)
			.def("SetTexture", (void(U2Dx9FxShaderEffect::*)(U2FxShaderState::Param param, const TCHAR*)) &U2Dx9FxShaderEffect::SetTexture)
			.def("SetFrameShaderName", &U2Dx9FxShaderEffect::SetFrameShaderName)
			
		];

	module(pLua)
		[
			class_<U2DX9FxMultiLayerShaderEffect, bases<U2Dx9FxShaderEffect> >("U2Object")

			.def("SetUVStretch", &U2DX9FxMultiLayerShaderEffect::SetUVStretch)
			.def("SetTextureCnt", &U2DX9FxMultiLayerShaderEffect::SetTextureCnt)
			.def("UpdateShader", &U2DX9FxMultiLayerShaderEffect::UpdateShader)
		];
}


class UtilFunc 
{

public:
	static void  GetPath(const char* relPath, unsigned int length)
	{
		return ::GetPath((char*)relPath, length);
	}

};


void RegisterGetPathFucWithLua(lua_State* pLua)
{
	module(pLua)
		[
			class_<UtilFunc>("UtilFunc")
			.def("GetPath",  &UtilFunc::GetPath)
		];
}


void RegisterAllWithLua(lua_State* pLua)
{
	RegisterGeneralWithLua(pLua);

	RegisterObjectWithLua(pLua);
	RegisterSpatialWithLua(pLua);

	RegisterTriangleWithLua(pLua);
	RegisterMeshWithLua(pLua);
	RegisterMeshGroupWithLua(pLua);
	
	
	RegisterControllerWithLua(pLua);

	RegisterMeshLoaderWithLua(pLua);
	RegisterNodeWithLua(pLua);

	RegisterShaderEffectWithLua(pLua);
	RegisterGetPathFucWithLua(pLua);
	RegisterShaderFuncsWithLua(pLua);
}
