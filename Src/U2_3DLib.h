#pragma once 

// Enable extra D3D debugging in debug builds if using the debug DirectX runtime.  
// This makes D3D objects work well in the debugger watch window, but slows down 
// performance slightly.
#if defined(DEBUG) || defined(_DEBUG)
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif
#endif

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif  //#ifndef DIRECTINPUT_VERSION



#define WIN32_LEAN_AND_MEAN
//#define  DEBUG_SHADER 

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>
#include <DInput.h>
#include <xInput.h>



// STL File Header
#include <exception>
#include <list>
#include <vector>
#include <queue>
#include <deque>
#include <set>
#include <string>
#include <map>
#include <hash_map>
#include <algorithm>

// C/C++ Runtime File Header
#include <stdio.h>
#include <strsafe.h>
#include <sstream>


#pragma warning(disable : 4018)


#include "U2_3DLibType.h"
#include "U2_3DDefine.h"

//#include <U2_3D/src/Ui/DXUT.h>
//#include <U2_3D/src/Ui/DXUTmisc.h>
//#include <U2_3D/src/Ui/DXUTenum.h>
//#include <U2_3D/src/Ui/DXUTgui.h>
//#include <U2_3D/src/Ui/DXUTRes.h>

#include <U2_3D/Src/U2Common.h>
#include <U2_3D/Src/U2MainMacro.h>

#include <U2_3D/src/script/LuaBindFunctions.h>
#include <U2_3D/src/script/LuaBindClasses.h>


#include <U2_3D/Src/U2Main.h>

#include <U2_3D/src/Main/U2Rtti.h>
#include <U2_3D/src/Main/U2Math.h>
#include <U2_3D/src/Main/U2RttiMacro.h>
#include <U2_3D/src/Main/U2Object.h>
#include <U2_3D/src/Main/U2Math.h>
#include <U2_3D/src/Main/U2PoolAlloc.h>
#include <U2_3D/src/Main/U2TPointerList.h>
#include <U2_3D/src/Main/U2TListBase.h>

#include <U2_3D/src/main/U2Box.h>
#include <U2_3D/src/main/U2Line3D.h>
#include <U2_3D/src/main/U2Segment3D.h>
#include <U2_3D/src/main/U2Ray3D.h>
#include <U2_3D/src/Main/U2BoundingVolume.h>
#include <U2_3D/src/Main/U2BoxBV.h>
#include <U2_3D/src/Main/U2TBucket.h>
#include <U2_3D/src/Main/U2SceneMgr.h>
#include <U2_3D/src/Main/U2Scene.h>

// collision

#include <U2_3D/src/Collision/U2IntectSegmentBox3D.h>
#include <U2_3D/src/Collision/U2IntectLineBox3D.h>
#include <U2_3D/src/Collision/U2IntectLineSphere3D.h>
#include <U2_3D/src/collision/U2IntectRaySphere3D.h>
#include <U2_3D/src/Collision/U2IntectRayBox3D.h>
#include <U2_3D/src/Collision/U2IntectSegmentBox3D.h>
#include <U2_3D/src/Collision/U2ContainBox3D.h>
#include <U2_3D/src/Collision/U2ContainSphere3D.h>
#include <U2_3D/src/Collision/U2Aabb.h>
#include <U2_3D/src/Collision/U2Sphere.h>


// dx9 -specific

#include <U2_3D/src/Dx9/U2Dx9BaseTexture.h>
#include <U2_3D/src/Dx9/U2Dx9Texture.h>
#include <U2_3D/src/dx9/U2Light.h>
#include <U2_3D/src/dx9/U2RenderState.h>
#include <U2_3D/src/dx9/U2AlphaState.h>
#include <U2_3D/src/dx9/U2MaterialState.h>
#include <U2_3D/src/dx9/U2ZBufferState.h>
#include <U2_3D/src/dx9/U2StencilState.h>
#include <U2_3D/src/dx9/U2WireframeState.h>


#include <U2_3D/src/Dx9/U2RenderStateCollection.h>
#include <U2_3D/src/Dx9/U2MeshData.h>
#include <U2_3D/src/Dx9/U2PolyLineData.h>
#include <U2_3D/src/Dx9/U2TriStripData.h>

#include <U2_3D/src/Dx9/U2SkinModifier.h>

#include <U2_3D/src/dx9/U2RenderTarget.h>
#include <U2_3D/src/dx9/U2Dx9LightMgr.h>
#include <U2_3D/src/Dx9/U2Dx9FrameBuffer.h>
#include <U2_3D/src/dx9/U2Dx9RenderStateMgr.h>
#include <U2_3D/src/dx9/U2Dx9TextureMgr.h>
#include <U2_3D/src/dx9/U2Dx9VertexBufferMgr.h>
#include <U2_3D/src/dx9/U2Dx9IndexBufferMgr.h>


#include <U2_3D/src/Shader/U2VertexAttributes.h>
#include  <U2_3D/src/Shader/U2ShaderConstant.h>
#include  <U2_3D/src/Shader/U2ShaderUserConstant.h>
#include  <U2_3D/src/Shader/U2NumerIcalConstant.h>
#include  <U2_3D/src/Shader/U2ShaderSamplerInfo.h>
#include  <U2_3D/src/Shader/U2StreamInstance.h>
#include  <U2_3D/src/Shader/U2ShaderArg.h>
#include  <U2_3D/src/Shader/U2ShaderAttribute.h>
#include  <U2_3D/src/Shader/U2D3DXEffectShader.h>

#include  <U2_3D/src/Shader/U2Shader.h>

// Effect

#include <U2_3D/src/Effect/U2Effect.h>
#include <U2_3D/src/Effect/U2ShaderEffect.h>
#include <U2_3D/src/Effect/U2DefaultShaderEffect.h>
#include <U2_3D/src/Effect/U2LightEffect.h>
#include <U2_3D/src/Effect/U2TextureEffect.h>
#include <U2_3D/src/Effect/U2MaterialTextureEffect.h>
#include <U2_3D/src/Effect/U2MultiTextureEffect.h>
#include <U2_3D/src/Effect/U2VertexColor3Effect.h>
#include <U2_3D/src/Effect/U2VolumeFogEffect.h>
#include <U2_3D/src/Effect/U2Dx9FxShaderEffect.h>
#include <U2_3D/src/Effect/U2Dx9FxMultilayerShaderEffect.h>

// Varaible 

#include <U2_3D/Src/Main/U2Variable.h>
#include <U2_3D/Src/Main/U2VariableContext.h>
#include <U2_3D/Src/Main/U2VariableMgr.h>



// Scene Graph

#include <U2_3D/src/Main/U2Spatial.h>
#include <U2_3D/src/Main/U2Node.h>
#include <U2_3D/src/Main/U2CameraNode.h>
#include <U2_3D/src/Main/U2ReflectionCameraNode.h>
#include <U2_3D/src/Main/U2LightNode.h>
#include <U2_3D/src/Main/U2BillboardNode.h>
#include <U2_3D/src/Main/U2SwitchNode.h>
#include <U2_3D/src/Main/U2DLODNode.h>
#include <U2_3D/src/Main/U2BspNode.h>
//#include <U2_3D/src/Main/U2MultilayeredNode.h>

#include <U2_3D/src/Main/U2PortalCuller.h>
#include <U2_3D/src/Main/U2ConvexRegion.h>
#include <U2_3D/src/Main/U2Portal.h>
#include <U2_3D/src/Main/U2ConvexRegionMgr.h>


#include <U2_3D/src/Main/U2Frustum.h>
#include <U2_3D/src/Main/U2Rect.h>
#include <U2_3D/src/Main/U2Camera.h>
#include <U2_3D/src/main/U2Triangle3.h>

#include <U2_3D/src/collision/U2PickRecord.h>
#include <U2_3D/src/collision/U2Picker.h>

#include <U2_3D/src/dx9/U2Mesh.h>
#include <U2_3D/src/dx9/U2TriList.h>
#include <U2_3D/src/dx9/U2TriStrip.h>
#include <U2_3D/src/Dx9/U2PolyLine.h>
#include <U2_3D/src/Dx9/U2StandardMesh.h>

// nMeshGroup
#include <U2_3D/src/Dx9/nMeshGroup.h>

#include <U2_3D/src/Animation/U2Controller.h>

#include <U2_3D/src/Animation/U2Animation.h>
#include <U2_3D/src/Animation/U2MemAnimation.h>
#include <U2_3D/src/Animation/U2AnimClip.h>
#include <U2_3D/src/Animation/U2AnimData.h>
#include <U2_3D/src/Animation/U2AnimFactory.h>
#include <U2_3D/src/Animation/U2AnimKeyData.h>
#include <U2_3D/src/Animation/U2AnimKeyTrack.h>
#include <U2_3D/src/Animation/U2AnimStateArray.h>
#include <U2_3D/src/Animation/U2AnimStateInfo.h>
#include <U2_3D/src/Animation/U2AnimUtil.h>
#include <U2_3D/src/Animation/U2Bone.h>
#include <U2_3D/src/Animation/U2BonePalette.h>
#include <U2_3D/src/Animation/U2Skeleton.h>
#include <U2_3D/src/Animation/U2SkinCharSet.h>
#include <U2_3D/src/Animation/U2SkinController.h>
#include <U2_3D/src/Animation/U2TransformInterpController.h>

#include <U2_3D/src/Tools/U2MeshBuilder.h>
#include <U2_3D/src/Tools/U2AnimBuilder.h>
#include <U2_3D/src/Tools/U2SkinPartition.h>


//#include <U2_3D/src/Object/U2FileTime_Win32.h>
//#include <U2_3D/src/Object/U2MeshLoader.h>
//#include <U2_3D/src/Object/U2XMeshLoader.h>
//#include <U2_3D/src/Object/U2WinFile.h>
//#include <U2_3D/src/Object/U2FileMgr.h>
#include <U2_3D/src/dx9/U2Dx9ShadowMgr.h>
#include <U2_3D/src/dx9/U2ShadowNode.h>
#include <U2_3D/src/dx9/U2Dx9ShadowCaster.h>
#include <U2_3D/src/dx9/U2Dx9StaticShadowCaster.h>
#include <U2_3D/src/dx9/U2Dx9DynamicShadowCaster.h>



#include <U2_3D/src/Dx9/U2Dx9Renderer.h>




