#include "hkgTest.h"

#include <Common/Base/hkBase.h>
#include <Common/Base/Math/hkMath.h>

// Scene Data
#include <Common/SceneData/Scene/hkxScene.h>
#include <Common/SceneData/Mesh/hkxMesh.h>
#include <Common/SceneData/Mesh/hkxMeshSection.h>
#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>
#include <Common/Base/Math/Util/hkMathUtil.h>

// Graphics Stuff
#include <Graphics/Common/Window/hkgWindow.h>
#include <Graphics/Common/Shader/hkgShaderCollection.h>
#include <Graphics/Common/DisplayObject/hkgDisplayObject.h>
#include <Graphics/Common/Geometry/hkgGeometry.h>
#include <Graphics/Common/Geometry/hkgMaterialFaceSet.h>
#include <Graphics/Bridge/SceneData/hkgVertexBufferWrapper.h>


//hkg
#include <Graphics/Common/DisplayWorld/hkgDisplayWorld.h>
#include <Graphics/Common/Geometry/hkgGeometry.h>
#include <Graphics/Common/DisplayObject/hkgDisplayObject.h>
#include <Graphics/Common/Light/hkgLightManager.h>
#include <Graphics/Common/Material/hkgMaterialManager.h>
#include <Graphics/Common/Shader/hkgShaderContext.h>
#include <Graphics/Common/Shader/hkgShaderCollection.h>
#include <Graphics/Common/Texture/SkyBox/hkgSkyBox.h>


//dx9
#include <Graphics/Dx9/Shared/Camera/hkgCameraDX9.h>
#include <Graphics/Dx9/Shared/DisplayContext/hkgDisplayContextDX9.h>
#include <Graphics/Dx9/Shared/Geometry/FaceSet/hkgFaceSetDX9.h>
#include <Graphics/Dx9/Shared/Font/hkgFontDX9.h>
#include <Graphics/Dx9/Shared/Light/hkgLightDX9.h>
#include <Graphics/Dx9/Shared/Material/hkgMaterialDX9.h>
#include <Graphics/Dx9/Shared/Texture/hkgTextureDX9.h>
#include <Graphics/Dx9/Shared/Texture/hkgCubemapTextureDX9.h>
#include <Graphics/Dx9/Shared/Geometry/VertexSet/hkgVertexSetDX9.h>
#include <Graphics/Dx9/Shared/Window/hkgViewportDX9.h>
#include <Graphics/Dx9/Shared/Shader/hkgVertexShaderDX9.h>
#include <Graphics/Dx9/Shared/Shader/hkgPixelShaderDX9.h>
#include <Graphics/Dx9/Shared/Geometry/BlendMatrixSet/hkgBlendMatrixSetDX9.h>
#include <Graphics/Dx9/Shared/DisplayObject/hkgInstancedDisplayObjectDX9.h>
#include <Graphics/Dx9/Shared/DisplayObject/hkgParticleDisplayObjectDX9.h>
#include <Graphics/Dx9/Shared/DisplayObject/hkgBillboardDisplayObjectDX9.h>


//#include <Graphics/Bridge/System/hkgSystem.h> // to figure put if we should hardware skin
void hkgTest::TestCamera()
{
	hkVector4 from( 0.3f, -1, 1 );
	hkVector4 to  ( 0, 0, 0.5f );
	hkVector4 up  ( 0.0f, 0.0f, 1.0f );
		
}

void hkgTest::setupDefaultCamera()
{

}

void hkgTest::loadSceneData()
{
	hkxScene* scene = new hkxScene();		
}

void hkgTest::init()
{
	//hkg
	//hkgDisplayWorld::create = hkgDisplayWorld::defaultCreateInternal;
	//hkgGeometry::create = hkgGeometry::defaultCreateInternal;
	//hkgDisplayObject::create = hkgDisplayObject::defaultCreateInternal; 
	//hkgLightManager::create = hkgLightManager::defaultCreateInternal;
	//hkgMaterialFaceSet::create = hkgMaterialFaceSet::defaultCreateInternal;
	//hkgMaterialManager::create = hkgMaterialManager::defaultCreateInternal;
	//hkgShaderCollection::create = hkgShaderCollection::defaultCreateInternal;
	//hkgShaderContext::create = hkgShaderContext::defaultCreateInternal;
	//hkgSkyBox::create = hkgSkyBox::defaultCreateInternal; 

	////dx9
	//hkgCamera::create = hkgCameraDX9::createCameraDX9;
	//hkgDisplayContext::create = hkgDisplayContextDX9::createDisplayContextDX9;
	//hkgDisplayContext::s_maxNumLights = 8;
	//hkgFaceSet::create = hkgFaceSetDX9::createFaceSetDX9;
	//hkgFont::create = hkgFontDX9::createFontDX9;
	//hkgLight::create = hkgLightDX9::createLightDX9;
	//hkgMaterial::create = hkgMaterialDX9::createMaterialDX9;
	//hkgTexture::create = hkgTextureDX9::createTextureDX9;
	//hkgCubemapTexture::create = hkgCubemapTextureDX9::createCubemapTextureDX9;
	//hkgVertexSet::create = hkgVertexSetDX9::createVertexSetDX9;
	//hkgViewport::create = hkgViewportDX9::createViewportDX9;
	//hkgShader::createVertexShader =  hkgVertexShaderDX9::createVertexShaderDX9;
	//hkgShader::createPixelShader =  hkgPixelShaderDX9::createPixelShaderDX9;
	//hkgBlendMatrixSet::create = hkgBlendMatrixSetDX9::createBlendMatrixSetDX9;
	//hkgInstancedDisplayObject::create = hkgInstancedDisplayObjectDX9::createInstancedDisplayObjectDX9;
	//hkgParticleDisplayObject::create = hkgParticleDisplayObjectDX9::createParticleDisplayObjectDX9;
	//hkgBillboardDisplayObject::create = hkgBillboardDisplayObjectDX9::createBillboardDisplayObjectDX9;

}

void hkgTest::createMesh()
{
	// See if it has scale
	// As Havok rigid bodies do not have scale, when we use
	// meshes with rigidbodies the scale must be either bakes in
	// of set as an extra transform in the display object
	// For now I will just bake it in as instancing is not the norm
	//hkVector4 scale;
	hkMatrix4 worldTM;
	hkRotation skewMatrix;
	hkMathUtil::Decomposition decomposition;
	hkMathUtil::decomposeMatrix(worldTM, decomposition);
	hkTransform decomposedWorldTransform (decomposition.m_rotation, decomposition.m_translation);
	// Set the transform
	HK_ALIGN(float colMaj[16], 16);
	decomposedWorldTransform.get4x4ColumnMajor( colMaj );

	hkMatrix3 scaleSkewInverseTranspose;
	{
		scaleSkewInverseTranspose = decomposition.m_scaleAndSkew;
		scaleSkewInverseTranspose.invert( HK_REAL_EPSILON );
		scaleSkewInverseTranspose.transpose();
	}

}