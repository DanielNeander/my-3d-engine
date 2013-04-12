#include "stdafx.h"
#include "EngineCore/Math/MathTools.h"

#include "GameApp/GameApp.h"

#include "TestObjects.h"
#include "M2Object.h"
#include "Mesh.h"
#include "M2Mesh.h"
#include "DemoSetup.h"
#include "PSSMShadowMapDx11.h"
#include "U2Light.h"
#include "LIDRenderer.h"
#include "./Light/SSAORenderer.h"

extern ShaderID gPssm;
extern ShaderID gPssmSkinned;
extern ShaderID gColorPssm;
extern ShaderID gSkinnedColorPssm;
extern ShaderID gColor;
extern ShaderID gSkinned;
extern U2Light gSun;


struct RenderVertexPNUV
{
	noVec3 position;
	noVec3 normal;
	noVec2 texcoord0;
};

struct RenderVertexPNUVTB
{
	noVec3 position;
	noVec3 normal;
	noVec3 tangent;
	noVec3 binormal;
	noVec2 texcoord0;
	
};

struct SkinnedVertexPNT
{
	noVec3 position;
	noVec3 normal;
	noVec2 texcoord0;
	float bones[4];
	float weights[4];
};

struct SkinnedVertexPNTUVTB
{
	noVec3 position;
	noVec3 normal;
	noVec3 tangent;
	noVec3 binormal;	
	noVec2 texcoord0;	
	float bones[4];
	float weights[4];
	
};


void M2Object::ComputeTangetSpace()
{
	float *vertexArrays[2];
					
	Tangents_ = new noVec3[this->numVertices];
	Binormals_ = new noVec3[this->numVertices];
	memset(Tangents_,  0, numVertices * sizeof(noVec3));
	memset(Binormals_, 0, numVertices * sizeof(noVec3));
	
	for (uint i = 0; i < this->nIndices; i += 3) 	{
		
		noVec3 v0 = vertDatas[indices[i]].pos;
		noVec3 v1 = vertDatas[indices[i+1]].pos;
		noVec3 v2 = vertDatas[indices[i+2]].pos;

		noVec2 t0 = vertDatas[indices[i]].texcoords;
		noVec2 t1 = vertDatas[indices[i+1]].texcoords;
		noVec2 t2 = vertDatas[indices[i+2]].texcoords;

		noVec3 sdir, tdir, normal;
		ComputeTangentVectors(v0, v1, v2, t0, t1, t2, sdir, tdir, normal);

		sdir.Normalize();
		tdir.Normalize();

		Tangents_[indices[i    ]] += sdir;
		Tangents_[indices[i + 1]] += sdir;
		Tangents_[indices[i + 2]] += sdir;

		Binormals_[indices[i    ]] += tdir;
		Binormals_[indices[i + 1]] += tdir;
		Binormals_[indices[i + 2]] += tdir;

		/*normals[indices[i    ]] += normal;
		normals[indices[i + 1]] += normal;
		normals[indices[i + 2]] += normal;*/
	}

	for (uint j = 0; j < numVertices; j++){
		Tangents_ [j].Normalize();
		Binormals_[j].Normalize();			
	}

	/*uint *indicesS = new uint[nIndices];
	uint *indicesT = new uint[nIndices];
	memcpy(indicesS, indices, nIndices * sizeof(uint));
	memcpy(indicesT, indices, nIndices * sizeof(uint));*/		
}

void M2Object::LoadModelData()
{
	if (animGeom) 
	{
		/*char def[256];		
		sprintf(def, "#define MATRIX_PALETTE_SIZE_DEFAULT %d\n", header.nBones);

		render_data[0].shader = renderer->addShader("Data/Shaders/skinnedShader.hlsl", def);*/
		//render_data[0].shader = renderer->addShader("Data/Shaders/SkinColorPSSM.hlsl", def);

		switch (AppSettings::ShadowModeEnum)
		{		
		
		case SHADOW_PSSM:
			render_data[0].shader = gSkinnedColorPssm;			
			break;
		case SHADOW_NONE:
			render_data[0].shader = gSkinned;
			break;
		case SHADOW_SSAO:			
			render_data[0].shader = gSkinnedColorSsao;
			break;
		}		

		FormatDesc vbFmt[] = {
			0, TYPE_VERTEX, FORMAT_FLOAT,  3,
			0, TYPE_NORMAL, FORMAT_FLOAT,   3,
			0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,
			0, TYPE_BONES,	 FORMAT_FLOAT,   4,
			0, TYPE_WEIGHTS, FORMAT_FLOAT,  4,
		};

		for (int i = 0; i < elementsOf(vbFmt); ++i)
		{
			vertexStride +=vbFmt[i].size * renderer->getFormatSize(vbFmt[i].format);
		}
		
		if ((render_data[0].vf = renderer->addVertexFormat(vbFmt, elementsOf(vbFmt), render_data[0].shader)) == VF_NONE) return;

		if ((render_data[0].diffuseSampler = renderer->addSamplerState(LINEAR, WRAP, WRAP, WRAP)) == SS_NONE) return;

		int32 numVerts = this->numVertices;
		SkinnedVertexPNT* pVerts = new SkinnedVertexPNT[numVerts];

		for (uint32 i = 0; i < this->numVertices; ++i)
		{
			pVerts[i].position = vertDatas[i].pos;

			float y = vertDatas[i].texcoords.y;
			if (modelType != MT_CHAR)
				y = 1.0f - vertDatas[i].texcoords.y;;
			pVerts[i].texcoord0 = noVec2(vertDatas[i].texcoords.x, y);
			pVerts[i].normal = vertDatas[i].normal;
			for (uint32 x = 0; x < 4; ++x)
			{			
				pVerts[i].bones[x] = vertDatas[i].bones[x];
				pVerts[i].weights[x] = vertDatas[i].weights[x] / 255.f;
			}
		}
		render_data[0].vb = renderer->addVertexBuffer(sizeof(SkinnedVertexPNT) * this->numVertices  , STATIC, pVerts);
		render_data[0].ib = renderer->addIndexBuffer(this->nIndices, sizeof(WORD), STATIC, this->indices);

		//SkinnedVertex
		delete [] pVerts;
	}
	else 
	{
		switch (AppSettings::ShadowModeEnum)		
		{		
		case SHADOW_PSSM:
			render_data[0].shader = gColorPssm;			
			break;
		case SHADOW_NONE:
			render_data[0].shader = gColor;
			break;
		case SHADOW_SSAO:			
			render_data[0].shader = gColorSsao;
			break;
		}		

		FormatDesc vbFmt[] = {
			0, TYPE_VERTEX, FORMAT_FLOAT, 3,
			0, TYPE_NORMAL, FORMAT_FLOAT, 3,
			0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,
		};

		for (int i = 0; i < elementsOf(vbFmt); ++i)
		{
			vertexStride +=vbFmt[i].size * renderer->getFormatSize(vbFmt[i].format);
		}
		if ((render_data[0].vf = renderer->addVertexFormat(vbFmt, elementsOf(vbFmt), render_data[0].shader)) == VF_NONE) return;



		if ((render_data[0].diffuseSampler = renderer->addSamplerState(LINEAR, WRAP, WRAP, WRAP)) == SS_NONE) return;

		/*for (size_t i=0; i < this->TextureList.size(); i++)
		{
			std::string texturename = this->TextureList.at(i) + ".tga";
			render_data[0].baseTex.push_back(renderer->addTexture(texturename.c_str(), true, render_data[0].linear));
		}*/
	
		int32 numVerts = this->numVertices;
		RenderVertexPNUV* pVerts = new RenderVertexPNUV[numVerts];

		for (uint32 i = 0; i < this->numVertices; ++i)
		{
			pVerts[i].position = vertDatas[i].pos;//this->vertices[i];
			pVerts[i].normal = vertDatas[i].normal;
			pVerts[i].texcoord0 = noVec2(this->texCoords[i].x, 1.f - this->texCoords[i].y);
		}

		render_data[0].vb = renderer->addVertexBuffer(sizeof(RenderVertexPNUV) * this->numVertices  , STATIC, pVerts);
		render_data[0].ib = renderer->addIndexBuffer(this->nIndices, sizeof(WORD), STATIC, this->indices);

		render_data[0].vertexStart = 0;
		render_data[0].vertexEnd = numVerts;
		render_data[0].startIndex = 0;
		render_data[0].endIndex = this->nIndices;

		delete [] pVerts;
	}

}

void M2Object::CreateRenderPass()

{
	//LoadModelData();
	if (header.nVertices == 0)
		return;

	LoadModelData2();
	

	size_t count = renderPasses.size();
	int currPass = 0;
		

	for (size_t i=0; i < count; ++i)
	{
		ModelRenderPass pass = renderPasses.at(i);

		if (pass.init(this))
		{
			RenderData& data = render_data[currPass++];

			GetRenderData(data);

			data.colorIndex = pass.color;
			data.opacity = pass.opacity;			
			data.startIndex = pass.indexStart;
			data.endIndex = pass.indexCount;
			data.vertexStart = pass.vertexStart;
			data.vertexEnd = pass.vertexEnd;

			uint32 bindtex = 0;

			if (specialTextures[pass.tex]==-1) 
				bindtex = textureIds[pass.tex];
			else 
				bindtex = replaceTextures[specialTextures[pass.tex]];
						
			data.baseTex = bindtex;

			if (replaceTextures[TEXTURE_BUMP] > 0)
			{
				
				data.bumpTex = replaceTextures[TEXTURE_BUMP];
				//data.normalSampler = 
			}

			// Texture
			// ALPHA BLENDING
			// blend mode
			BlendStateID blend = -1;
			switch (pass.blendmode) {
			case BM_OPAQUE:	// 0							
				break;
			case BM_TRANSPARENT: // 1
				//glEnable(GL_ALPHA_TEST);
				//glAlphaFunc(GL_GEQUAL,0.7f); // Dx10 ÀÌÈÄ Alpha Test ¾ø¾îÁü. 				
				break;
			case BM_ALPHA_BLEND:
				blend = GetApp()->GetBlendMode(0);
				break;
			case BM_ADDITIVE: // 3
				blend = GetApp()->GetBlendMode(1);
				break;
			case BM_ADDITIVE_ALPHA:
				blend = GetApp()->GetBlendMode(2);
				break;
			case BM_MODULATE:
				blend = GetApp()->GetBlendMode(3);
				break;
			case BM_MODULATEX2:	// 6, not sure if this is right
				blend = GetApp()->GetBlendMode(4);
				break;
			default:
				blend = GetApp()->GetBlendMode(5);
				break;
			}


			data.blendMode = blend;


			if (pass.cull)
				data.cull = renderer->addRasterizerState(D3D11_CULL_FRONT);
			else 
				data.cull = renderer->addRasterizerState(D3D11_CULL_NONE);

			DepthStateID depthid = -1;
			if (pass.noZWrite) // depth write == false
				depthid = renderer->addDepthState(true, false);
			else 				
				depthid = renderer->addDepthState(true, true);

			data.depthMode = depthid;
			data.unlit = pass.unlit;			
		}
	}
	numPass = currPass;
}

void M2Object::ToggleBlendState(int index, int blendIndex)
{
	 render_data[index].blendMode = GetApp()->GetBlendMode(blendIndex);
}

void M2Object::DrawModel( const mat4& world, const mat4& view, const mat4& proj )
{
	view_ = view;

	if (!animated) {


	}
	else {		
						
		static int i = 0;
		static int currAnim = 3;
		/*if (currAnim >= header.nAnimations)
		{
			currAnim = 0;			
		}*/

		/*animManager->SetAnim(0, currAnim, 0);
		animManager->Play();					*/

		static size_t fps = 0;
		mat4* frameMatrices = NULL;

#if 0
		if (animManager->GetCurrAnim())
		{		


			if (fps++ >= 3)
			{
				i++;
				fps = 0;
			}

			if (i >= animManager->GetCurrAnim()->GetNumFrames())		
			{
				i = 0;
				//++currAnim;
			}
											
			frameMatrices = animManager->GetCurrAnim()->GetFrame2(i);
		}
#endif

		renderer->resetToDefaults();
		if (animGeom)	
		{
			std::vector<mat4> temp;
			
			temp.resize(header.nBones);
			for (size_t i=0; i<header.nBones; i++) 
				temp[i] = bones[i].rdmat;
			
			
						
			//float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
			//renderer->clear(true, true, false, ClearColor, 1.f);
			int num =  numPass;				
			noVec4 ocolor, ecolor;
			for (int i = 0; i < num; i++)						
			{
				const RenderData& rd = render_data[i];

				if (!UpdateColors(rd.colorIndex, rd.opacity, rd.unlit, ocolor, ecolor) || !rd.visible)
					continue;

				LIDRenderer::GetSingletonPtr()->UpdateMaterial(D3D11Context(), ecolor, ocolor, noVec3(0.5f, 0.5f, 0.5f), 0);				
				
				
				renderer->reset();	
				renderer->setVertexFormat(rd.vf);
				renderer->setVertexBuffer(0, rd.vb);
				renderer->setIndexBuffer(rd.ib);
				renderer->setRasterizerState(rd.cull);
				renderer->setShader(rd.shader);	
				//renderer->setShaderConstant3f("g_localLightPos", vec3(gLight.m_mInvView.rows[3].x, gLight.m_mInvView.rows[3].y, gLight.m_mInvView.rows[3].z));
				renderer->setSamplerState("g_samLinear", render_data[0].diffuseSampler);
				renderer->setTexture("g_txDiffuse", rd.baseTex);	
				//if (rd.bumpTex != -1) renderer->setTexture("g_txBump", rd.bumpTex);
				if (rd.blendMode != -1) renderer->setBlendState(rd.blendMode);
				else 
				{
					float blendFactor[4] = {1, 1, 1, 1};
					D3D11Context()->OMSetBlendState(GetApp()->blendStates_->BlendDisabled(), blendFactor, 0xFFFFFFF);			
				}
				if (rd.depthMode != -1) renderer->setDepthState(rd.depthMode);				
				else 
				{
					D3D11Context()->OMSetDepthStencilState(GetApp()->depthStencilStates_->DepthDisabled(), 0.f);
					
				}
				
				renderer->setShaderConstant4x4f("g_mWorld", world);
				//renderer->setShaderConstant4x4f("g_mViewProj", view * proj);
				renderer->setShaderConstant4x4f("g_mView", view);

				//mat4 invView = transpose(view);
				//renderer->setShaderConstant4x4f("InvView", invView);
				//renderer->setShaderConstant3f("g_localEye", invView.getTrans());
				renderer->setShaderConstant4x4f("g_mWorldViewProjection",  world *view * proj);
				//renderer->setShaderConstant4x4f("View", view);
				//renderer->setShaderConstant4x4f("Projection", proj);	
				//renderer->setShaderConstantArray4x4f("g_matrices", frameMatrices, header.nBones);
				renderer->setShaderConstantArray4x4f("g_matrices", &temp[0], header.nBones);		
				
				renderer->apply();
				
				renderer->drawElements(PRIM_TRIANGLES, rd.startIndex, rd.endIndex, rd.vertexStart, rd.vertexEnd);

			}
		}
		else 
		{
			noVec4 ocolor, ecolor;
			for (int i = 0; i < numPass; i++)
			{
				const RenderData& rd = render_data[i];

				if (!UpdateColors(rd.colorIndex, rd.opacity, rd.unlit, ocolor, ecolor))
					continue;

				LIDRenderer::GetSingletonPtr()->UpdateMaterial(D3D11Context(), ecolor, ocolor);

				renderer->reset();	
				renderer->setVertexFormat(rd.vf);
				renderer->setVertexBuffer(0, rd.vb);
				renderer->setIndexBuffer(rd.ib);
				renderer->setRasterizerState(rd.cull);
				renderer->setShader(rd.shader);					
				//renderer->setSamplerState("g_samShadowMap", gPassmSampler);				
				renderer->setSamplerState("g_samLinear", render_data[0].diffuseSampler);
				renderer->setTexture("g_txDiffuse", rd.baseTex);			
				if (rd.blendMode != -1) renderer->setBlendState(rd.blendMode);
				else 
				{
					float blendFactor[4] = {1, 1, 1, 1};
					D3D11Context()->OMSetBlendState(GetApp()->blendStates_->BlendDisabled(), blendFactor, 0xFFFFFFF);			
				}
				if (rd.depthMode != -1) renderer->setDepthState(rd.depthMode);				
				else 
				{
					D3D11Context()->OMSetDepthStencilState(GetApp()->depthStencilStates_->DepthDisabled(), 0.f);
				}
				
				renderer->setShaderConstant4x4f("World", world);
				renderer->setShaderConstant4x4f("View", view);
				renderer->setShaderConstant4x4f("Projection", proj);	
				vec3 eye;
				GetApp()->ActiveCam_->getFrom((float*)eye);
				renderer->setShaderConstant3f("vecEye", eye);
				vec3 Lightdir(gSun.GetDir().x, gSun.GetDir().y, gSun.GetDir().z);	
				renderer->setShaderConstant3f("vecLightDir", Lightdir);
				//vec4 diffuse(0.25098f, 0.435294f, 0.901961f, 1.f);
				renderer->setShaderConstant4f("vDIC", vec4(gSun.m_diffuseColor.x , gSun.m_diffuseColor.y, gSun.m_diffuseColor.z, 1));
				vec4 specular(0.135f, 0.135f, 0.135f, 10.f);
				renderer->setShaderConstant4f("vSpecIC", specular);	

				renderer->apply();				
				renderer->drawElements(PRIM_TRIANGLES, rd.startIndex, rd.endIndex, rd.vertexStart, rd.vertexEnd);
			}
		}
	}
}

M2Object::M2Object( Renderer* _renderer, WowActor* owner)
	: M2Loader(_renderer),Owner_(owner)
{
	
}

void M2Object::update(size_t deltaTime, size_t accumTime) {

	deltaTime_ = deltaTime;
	globalTime = accumTime;

	if (animManager->IsPaused())
		deltaTime = 0;
	if (!animManager->IsParticlePaused())
		deltaTime = animManager->GetTimeDiff();			

	if (animated)
		animManager->Tick(deltaTime);

	if (animated) {

		if (ind) {
			animating(animManager->GetAnim());
		} else {
			if (!animcalc) {
				animating(animManager->GetAnim());				
			}
		}	
	}


	UpdateParticles(deltaTime/1000.0f);
}

extern std::vector<mat4> mCropMatrix;
extern std::vector<mat4> mTextureMatrix;

void M2Object::DrawShadowModel( const mat4& world, const mat4& view, const mat4& proj )
{
	view_ = view;

	if (!animated) {


	}
	else {		
						
		static int i = 0;
		static int currAnim = 3;
		/*if (currAnim >= header.nAnimations)
		{
			currAnim = 0;			
		}*/

		/*animManager->SetAnim(0, currAnim, 0);
		animManager->Play();					*/

		static size_t fps = 0;
		mat4* frameMatrices = NULL;

#if 0
		if (animManager->GetCurrAnim())
		{		


			if (fps++ >= 3)
			{
				i++;
				fps = 0;
			}

			if (i >= animManager->GetCurrAnim()->GetNumFrames())		
			{
				i = 0;
				//++currAnim;
			}
											
			frameMatrices = animManager->GetCurrAnim()->GetFrame2(i);
		}
#endif
		renderer->resetToDefaults();	
		M2Mesh* mesh = Owner_->mesh_;
				
		if (animGeom)	
		{
			std::vector<mat4> temp;
			
			temp.resize(header.nBones);
			for (size_t i=0; i<header.nBones; i++) 
				temp[i] = bones[i].rdmat;
			
			
			
			//float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
			//renderer->clear(true, true, false, ClearColor, 1.f);
			int num =  numPass;	
			//num = 2;
			noVec4 ocolor, ecolor;
			for (int i = 0; i < num; i++)						
			{
				const RenderData& rd = render_data[i];

				if (!UpdateColors(rd.colorIndex, rd.opacity, rd.unlit, ocolor, ecolor))
					continue;

				gSun.Apply(1);
				LIDRenderer::GetSingletonPtr()->UpdateMaterial(D3D11Context(), ecolor, ocolor);				

				renderer->reset();	
				renderer->setVertexFormat(rd.vf);
				renderer->setVertexBuffer(0, rd.vb);
				renderer->setIndexBuffer(rd.ib);
				//renderer->setRasterizerState(rd.cull);
				renderer->setShader(rd.shader);		

				ID3D11SamplerState* ss[] = { LIDRenderer::GetSingletonPtr()->GetSamplerState().ShadowMap() };
				D3D11Context()->PSSetSamplers(0, 1, ss);				
				D3D11Context()->PSSetShaderResources(0, 1, &GetShadowMap<PSSMShadowMapDx11>()->m_pSRV);		
				renderer->setSamplerState("g_samLinear", render_data[0].diffuseSampler);
				renderer->setTexture("g_txDiffuse", rd.baseTex);		
				if (rd.bumpTex != -1) 
				{					
					//renderer->setTexture("g_txBump", rd.bumpTex);
				}

				if (rd.blendMode != -1) renderer->setBlendState(rd.blendMode);
				else 
				{
					float blendFactor[4] = {1, 1, 1, 1};
					D3D11Context()->OMSetBlendState(GetApp()->blendStates_->BlendDisabled(), blendFactor, 0xFFFFFFF);			
				}
				if (rd.depthMode != -1) renderer->setDepthState(rd.depthMode);				
				else 
				{
					D3D11Context()->OMSetDepthStencilState(GetApp()->depthStencilStates_->DepthDisabled(), 0.f);
				}

				// set split range												
				renderer->setShaderConstant4x4f("g_mWorld", world);
				renderer->setShaderConstant4x4f("g_mViewProj", view * proj);				
				renderer->setShaderConstant4x4f("g_mView", view);				
				mat4 invView = transpose(view);
				//renderer->setShaderConstant4x4f("InvView", invView);
				renderer->setShaderConstant3f("g_localEye", invView.getTrans());

				renderer->setShaderConstant3f("g_vLightDir", gSun.GetDir().ToFloatPtr());
				renderer->setShaderConstant3f("g_vLightColor", vec3(gSun.m_diffuseColor.ToFloatPtr()));
				renderer->setShaderConstant3f("g_vAmbient", vec3(gSun.m_ambientColor.ToFloatPtr()));	
				renderer->setShaderConstantArray1f("g_fSplitPlane", &g_fSplitPos[1], g_iNumSplits);	
				renderer->setShaderConstant4x4f("g_mWorldViewProjection",  world * view * proj);	
				renderer->setShaderConstantArray4x4f("g_matrices", &temp[0], header.nBones);				
				renderer->setShaderConstantArray4x4f("g_mTextureMatrix", &mTextureMatrix[0], g_iNumSplits);

				renderer->apply();				

				
				renderer->drawElements(rd.prim, rd.startIndex, rd.endIndex, 0, rd.vertexEnd);			
			}
		}
		else 
		{
			noVec4 ocolor, ecolor;
			for (int i = 0; i < numPass; i++)
			{
				const RenderData& rd = render_data[i];

				if (!UpdateColors(rd.colorIndex, rd.opacity, rd.unlit, ocolor, ecolor))
					continue;
				
				renderer->reset();	
				renderer->setVertexFormat(rd.vf);
				renderer->setVertexBuffer(0, rd.vb);
				renderer->setIndexBuffer(rd.ib);
				//renderer->setRasterizerState(rd.cull);
				renderer->setShader(rd.shader);					
				renderer->setSamplerState("g_samLinear", render_data[0].diffuseSampler);
				renderer->setTexture("g_txDiffuse", rd.baseTex);			
				//if (rd.blendMode != -1) renderer->setBlendState(GetApp()->GetBlendMode(5));
				//if (rd.depthMode != -1) renderer->setDepthState(rd.depthMode);
				if (rd.blendMode != -1) renderer->setBlendState(rd.blendMode);
				else 
				{
					float blendFactor[4] = {1, 1, 1, 1};
					D3D11Context()->OMSetBlendState(GetApp()->blendStates_->BlendDisabled(), blendFactor, 0xFFFFFFF);			
				}
				if (rd.depthMode != -1) renderer->setDepthState(rd.depthMode);				
				else 
				{
					D3D11Context()->OMSetDepthStencilState(GetApp()->depthStencilStates_->DepthDisabled(), 0.f);
				}
				
				// set split range												
				renderer->setShaderConstant4x4f("g_mWorld", world);
				renderer->setShaderConstant4x4f("g_mViewProj", view * proj);				
				renderer->setShaderConstant4x4f("g_mView", view);				
				renderer->setShaderConstant3f("g_vLightDir", gSun.GetDir().ToFloatPtr());
				renderer->setShaderConstant3f("g_vLightColor", vec3(gSun.m_diffuseColor.ToFloatPtr()));
				renderer->setShaderConstant3f("g_vAmbient", vec3(gSun.m_ambientColor.ToFloatPtr()));	
				renderer->setShaderConstantArray1f("g_fSplitPlane", &g_fSplitPos[1], g_iNumSplits);	
				renderer->setShaderConstant4x4f("g_mWorldViewProjection",  world * view * proj);	
				renderer->setShaderConstantArray4x4f("g_mTextureMatrix", &mTextureMatrix[0], g_iNumSplits);
				
				renderer->apply();				

				ID3D11SamplerState* ss[] = { LIDRenderer::GetSingletonPtr()->GetSamplerState().ShadowMap() };
				D3D11Context()->PSSetSamplers(1, 1, ss);				
				D3D11Context()->PSSetShaderResources(1, 1, &GetShadowMap<PSSMShadowMapDx11>()->m_pSRV);		
				renderer->drawElements(rd.prim, rd.startIndex, rd.endIndex, 0, rd.vertexEnd);				
			}
		}
	}
	
	// Unbind shader resources
	ID3D11ShaderResourceView* nullViews[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	GetApp()->GetContext()->PSSetShaderResources(0, 8, nullViews);
}

void M2Object::DrawShadow( const mat4& world, const mat4& view, const mat4& proj )
{
	view_ = view;

	if (!animated) {


	}
	else {		
						
	
		static size_t fps = 0;
		mat4* frameMatrices = NULL;

		M2Mesh* mesh = Owner_->mesh_;
		renderer->resetToDefaults();
				
		if (animGeom)	
		{
			std::vector<mat4> temp;
			
			temp.resize(header.nBones);
			for (size_t i=0; i<header.nBones; i++) 
				temp[i] = bones[i].rdmat;
			
			
			
			//float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
			//renderer->clear(true, true, false, ClearColor, 1.f);
			int num =  numPass;	
			//num = 2;
			noVec4 ocolor, ecolor;
			for (int i = 0; i < num; i++)						
			{
				const RenderData& rd = render_data[i];

				if (!UpdateColors(rd.colorIndex, rd.opacity, rd.unlit, ocolor, ecolor))
					continue;

				renderer->reset();	
				renderer->setVertexFormat(rd.vf);
				renderer->setVertexBuffer(0, rd.vb);
				renderer->setIndexBuffer(rd.ib);
				renderer->setRasterizerState(rd.cull);
				renderer->setShader(gPssmSkinned);					
				if (rd.blendMode != -1) renderer->setBlendState(rd.blendMode);
				if (rd.depthMode != -1) renderer->setDepthState(rd.depthMode);

				// set split range				
				renderer->setShaderConstant1i("g_iFirstSplit", mesh->m_iFirstSplit);
				renderer->setShaderConstant1i("g_iLastSplit", mesh->m_iLastSplit);								
				renderer->setShaderConstant4x4f("g_mWorld", world);
				renderer->setShaderConstant4x4f("g_mViewProj", view * proj);				
				renderer->setShaderConstantArray4x4f("g_mCropMatrix", &mCropMatrix[0], 4);				
				//renderer->setShaderConstant4x4f("View", view);
				//renderer->setShaderConstant4x4f("Projection", proj);	
				renderer->setShaderConstantArray4x4f("g_matrices", &temp[0], header.nBones);

				renderer->apply();
				GetApp()->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				int iNumInstances = mesh->m_iLastSplit - mesh->m_iFirstSplit + 1;
				D3D11Context()->DrawIndexedInstanced(rd.endIndex, iNumInstances, rd.startIndex, 0, 0);
			}
		}
		else 
		{
			noVec4 ocolor, ecolor;
			for (int i = 0; i < numPass; i++)
			{
				const RenderData& rd = render_data[i];

				if (!UpdateColors(rd.colorIndex, rd.opacity, rd.unlit, ocolor, ecolor))
					continue;

				renderer->reset();	
				renderer->setVertexFormat(rd.vf);
				renderer->setVertexBuffer(0, rd.vb);
				renderer->setIndexBuffer(rd.ib);
				renderer->setRasterizerState(rd.cull);
				renderer->setShader(gPssm);	
				//renderer->setSamplerState("samLinear", render_data[0].linear);
				//renderer->setTexture("txDiffuse", rd.baseTex);			
				if (rd.blendMode != -1) renderer->setBlendState(rd.blendMode);
				if (rd.depthMode != -1) renderer->setDepthState(rd.depthMode);
				
				renderer->setShaderConstant1i("g_iFirstSplit", mesh->m_iFirstSplit);
				renderer->setShaderConstant1i("g_iLastSplit", mesh->m_iLastSplit);								
				renderer->setShaderConstantArray4x4f("g_mCropMatrix", &mCropMatrix[0], 4);
				renderer->setShaderConstant4x4f("g_mWorld", world);
				renderer->setShaderConstant4x4f("g_mViewProj", view * proj);
				
				renderer->apply();				
				GetApp()->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				int iNumInstances = mesh->m_iLastSplit - mesh->m_iFirstSplit + 1;
				D3D11Context()->DrawIndexedInstanced(rd.endIndex, iNumInstances, rd.startIndex, 0, 0);				
			}
		}
	}
}

void M2Object::DrawSSAO( const mat4& world, const mat4& view, const mat4& proj ) {		
	/*if (!animated) {


	}
	else*/ {		
		static size_t fps = 0;
		mat4* frameMatrices = NULL;

		M2Mesh* mesh = Owner_->mesh_;

		renderer->resetToDefaults();
		Direct3D11Renderer* Dx11R = (Direct3D11Renderer*)renderer;

		if (animGeom)	
		{
			std::vector<mat4> temp;

			temp.resize(header.nBones);
			for (size_t i=0; i<header.nBones; i++) 
				temp[i] = bones[i].rdmat;

			//float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
			//renderer->clear(true, true, false, ClearColor, 1.f);
			int num =  numPass;	
			//num = 2;
			noVec4 ocolor, ecolor;
			for (int i = 0; i < num; i++)						
			{
				const RenderData& rd = render_data[i];

				if (!UpdateColors(rd.colorIndex, rd.opacity, rd.unlit, ocolor, ecolor))
					continue;

				renderer->reset();	
				renderer->setShader(rd.shader);
				renderer->setVertexFormat(rd.vf);
				renderer->setVertexBuffer(0, rd.vb);
				renderer->setIndexBuffer(rd.ib);								
				
				renderer->setShaderConstant4x4f("g_mWorld", world);
				//renderer->setShaderConstant4x4f("g_mViewProj", view * proj);
				renderer->setShaderConstant4x4f("g_mView", view);

				//mat4 invView = transpose(view);
				//renderer->setShaderConstant4x4f("InvView", invView);
				//renderer->setShaderConstant3f("g_localEye", invView.getTrans());
				renderer->setShaderConstant4x4f("g_mWorldViewProjection",  world *view * proj);
				//renderer->setShaderConstant4x4f("View", view);
				//renderer->setShaderConstant4x4f("Projection", proj);	
				renderer->setShaderConstantArray4x4f("g_matrices", &temp[0], header.nBones);				

				renderer->setTexture("g_txDiffuse", rd.baseTex);
				renderer->setSamplerState("g_samLinear", render_data[0].diffuseSampler);

				renderer->apply();
								

				//ID3D11ShaderResourceView* srv = Dx11R->getTextureSRV(rd.baseTex);
				//D3D11Context()->PSSetShaderResources(0, 1, &srv);				
								
				ID3D11ShaderResourceView* aoSRV[] ={ GetApp()->SSAORender_->GetAOSRV() };
				D3D11Context()->PSSetShaderResources(1, 1, aoSRV);				
				
				//renderer->setShaderConstantArray4x4f("g_matrices", &temp[0], header.nBones);				
				renderer->drawElements(rd.prim, rd.startIndex, rd.endIndex, 0, rd.vertexEnd);															
				
				
			}
		}
		else 
		{
			noVec4 ocolor, ecolor;
			for (int i = 0; i < numPass; i++)
			{
				const RenderData& rd = render_data[i];

				if (!UpdateColors(rd.colorIndex, rd.opacity, rd.unlit, ocolor, ecolor))
					continue;
				
				renderer->reset();	
				renderer->setShader(rd.shader);
				renderer->setVertexFormat(rd.vf);
				renderer->setVertexBuffer(0, rd.vb);
				renderer->setIndexBuffer(rd.ib);								

				renderer->setShaderConstant4x4f("World", world);
				//renderer->setShaderConstant4x4f("g_mViewProj", view * proj);
				renderer->setShaderConstant4x4f("View", view);

				//mat4 invView = transpose(view);
				//renderer->setShaderConstant4x4f("InvView", invView);
				//renderer->setShaderConstant3f("g_localEye", invView.getTrans());
				renderer->setShaderConstant4x4f("Projection",  proj);

				renderer->setTexture("g_txDiffuse", rd.baseTex);
				renderer->setSamplerState("g_samLinear", render_data[0].diffuseSampler);

				renderer->apply();
				
				//ID3D11ShaderResourceView* srv = Dx11R->getTextureSRV(rd.baseTex);
				//D3D11Context()->PSSetShaderResources(0, 1, &srv);										

				ID3D11ShaderResourceView* aoSRV[] ={ GetApp()->SSAORender_->GetAOSRV() };
				D3D11Context()->PSSetShaderResources(1, 1, aoSRV);				

				renderer->drawElements(rd.prim, rd.startIndex, rd.endIndex, 0, rd.vertexEnd);											
			}
		}
	}
}


void M2Object::DrawCascaded()
{
	/*if (!animated) {


	}
	else*/ {		
		static size_t fps = 0;
		mat4* frameMatrices = NULL;

		M2Mesh* mesh = Owner_->mesh_;

		renderer->resetToDefaults();
		Direct3D11Renderer* Dx11R = (Direct3D11Renderer*)renderer;

		if (animGeom)	
		{
			std::vector<mat4> temp;

			temp.resize(header.nBones);
			for (size_t i=0; i<header.nBones; i++) 
				temp[i] = bones[i].rdmat;

			//float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
			//renderer->clear(true, true, false, ClearColor, 1.f);
			int num =  numPass;	
			//num = 2;
			noVec4 ocolor, ecolor;
			for (int i = 0; i < num; i++)						
			{
				const RenderData& rd = render_data[i];

				if (!UpdateColors(rd.colorIndex, rd.opacity, rd.unlit, ocolor, ecolor))
					continue;

				renderer->reset();	
				renderer->changeVertexFormat(rd.vf);
				renderer->changeVertexBuffer(0, rd.vb);
				renderer->changeIndexBuffer(rd.ib);		
								
				//renderer->setShaderConstantArray4x4f("g_matrices", &temp[0], header.nBones);				
				renderer->drawElements(rd.prim, rd.startIndex, rd.endIndex, 0, rd.vertexEnd);															
				
				ID3D11ShaderResourceView* srv = Dx11R->getTextureSRV(rd.baseTex);
				D3D11Context()->PSSetShaderResources(0, 1, &srv);				
			}
		}
		else 
		{
			noVec4 ocolor, ecolor;
			for (int i = 0; i < numPass; i++)
			{
				const RenderData& rd = render_data[i];

				if (!UpdateColors(rd.colorIndex, rd.opacity, rd.unlit, ocolor, ecolor))
					continue;

				renderer->reset();	
				renderer->changeVertexFormat(rd.vf);
				renderer->changeVertexBuffer(0, rd.vb);
				renderer->changeIndexBuffer(rd.ib);								

				ID3D11ShaderResourceView* srv = Dx11R->getTextureSRV(rd.baseTex);
				D3D11Context()->PSSetShaderResources(0, 1, &srv);				
				renderer->drawElements(rd.prim, rd.startIndex, rd.endIndex, 0, rd.vertexEnd);											
			}
		}
	}

}

void M2Object::DrawZOnly( const noMat4& world, const mat4& view, const mat4& proj )
{
	view_ = view;

	if (!animated) {


	}
	else {		


		static size_t fps = 0;
		mat4* frameMatrices = NULL;

		M2Mesh* mesh = Owner_->mesh_;

		renderer->resetToDefaults();	
		if (animGeom)	
		{
			std::vector<noMat4> temp;

			temp.resize(header.nBones);
			for (size_t i=0; i<header.nBones; i++) 
				temp[i] = bones[i].mat;



			//float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
			//renderer->clear(true, true, false, ClearColor, 1.f);
			int num =  numPass;	
			//num = 2;
			noVec4 ocolor, ecolor;
			for (int i = 0; i < num; i++)						
			{
				const RenderData& rd = render_data[i];

				if (!UpdateColors(rd.colorIndex, rd.opacity, rd.unlit, ocolor, ecolor))
					continue;

				renderer->reset();
							
				renderer->setVertexFormat(rd.vf);				
				renderer->setVertexBuffer(0, rd.vb);				
				renderer->setIndexBuffer(rd.ib);				
				renderer->apply();
				
				if (AppSettings::ShadowModeEnum == SHADOW_SSAO)
					GetApp()->SSAORender_->SetSkinDepthShader((noMat4*)&world, &temp[0], header.nBones);
				else 
					LIDRenderer::GetSingletonPtr()->BeginDepthSkinRender(GetApp()->ActiveCam_, D3D11Context(), world.ToFloatPtr(), &temp[0], header.nBones);		
				
				renderer->drawElements(rd.prim, rd.startIndex, rd.endIndex, 0, rd.vertexEnd);											
			}
		}
		else 
		{
			noVec4 ocolor, ecolor;
			for (int i = 0; i < numPass; i++)
			{
				const RenderData& rd = render_data[i];

				if (!UpdateColors(rd.colorIndex, rd.opacity, rd.unlit, ocolor, ecolor))
					continue;

				renderer->reset();
												
				renderer->setVertexFormat(rd.vf);				
				renderer->setVertexBuffer(0, rd.vb);				
				renderer->setIndexBuffer(rd.ib);				
				renderer->apply();
				
				if (AppSettings::ShadowModeEnum == SHADOW_SSAO)
					GetApp()->SSAORender_->SetDepthShader((noMat4*)&world);
				else 
					LIDRenderer::GetSingletonPtr()->BeginDepthRender(GetApp()->ActiveCam_, D3D11Context(), world.ToFloatPtr());									
				
				renderer->drawElements(rd.prim, rd.startIndex, rd.endIndex, 0, rd.vertexEnd);											
			}
		}
	}
}


void M2Object::LoadModelData2()
{
	
	ComputeTangetSpace();
	
	if (animGeom) 
	{
		char def[256];		
		sprintf(def, "#define MATRIX_PALETTE_SIZE_DEFAULT %d\n", header.nBones);

		//render_data[0].shader = renderer->addShader("Data/Shaders/skinnedShader.hlsl", def);
		switch (AppSettings::ShadowModeEnum)
		{
		//case SHADOW_NONE:			
		case SHADOW_PSSM:
			render_data[0].shader = gSkinnedColorPssm;			
			break;
		case SHADOW_NONE:
			render_data[0].shader = gSkinned;
			break;
		case SHADOW_SSAO:			
			render_data[0].shader = gSkinnedColorSsao;
			break;
		}		

		FormatDesc vbFmt[] = {
			0, TYPE_VERTEX, FORMAT_FLOAT,  3,
			0, TYPE_NORMAL, FORMAT_FLOAT,   3,
			0, TYPE_TANGENT,FORMAT_FLOAT, 3,
			0, TYPE_BINORMAL,FORMAT_FLOAT, 3,		
			0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,
			0, TYPE_BONES,	 FORMAT_FLOAT,   4,
			0, TYPE_WEIGHTS, FORMAT_FLOAT,  4,			
		};
		
		for (int i = 0; i < elementsOf(vbFmt); ++i)
		{
			vertexStride +=vbFmt[i].size * renderer->getFormatSize(vbFmt[i].format);
		}

		if ((render_data[0].vf = renderer->addVertexFormat(vbFmt, elementsOf(vbFmt), render_data[0].shader)) == VF_NONE) return;

		if ((render_data[0].diffuseSampler = renderer->addSamplerState(LINEAR, WRAP, WRAP, WRAP)) == SS_NONE) return;

		int32 numVerts = this->numVertices;
		SkinnedVertexPNTUVTB* pVerts = new SkinnedVertexPNTUVTB[numVerts];

		for (uint32 i = 0; i < this->numVertices; ++i)
		{
			pVerts[i].position = vertDatas[i].pos;
			
			float y = vertDatas[i].texcoords.y;
			if (modelType != MT_CHAR)
				y = 1.0f - vertDatas[i].texcoords.y;;
			pVerts[i].texcoord0 = noVec2(vertDatas[i].texcoords.x, y);
			pVerts[i].normal = vertDatas[i].normal;
			for (uint32 x = 0; x < 4; ++x)
			{			
				pVerts[i].bones[x] = vertDatas[i].bones[x];
				pVerts[i].weights[x] = vertDatas[i].weights[x] / 255.f;
			}
			pVerts[i].tangent = Tangents_[i];
			pVerts[i].binormal = Binormals_[i];
		}
		render_data[0].vb = renderer->addVertexBuffer(sizeof(SkinnedVertexPNTUVTB) * this->numVertices  , STATIC, pVerts);
		render_data[0].ib = renderer->addIndexBuffer(this->nIndices, sizeof(WORD), STATIC, this->indices);

		//SkinnedVertex
	}
	else 
	{
		switch (AppSettings::ShadowModeEnum)
		{				
		case SHADOW_PSSM:
			render_data[0].shader = gColorPssm;			
			break;
		case SHADOW_NONE:
			render_data[0].shader = gColor;
			break;
		case SHADOW_SSAO:			
			render_data[0].shader = gColorSsao;
			break;
		}		
		
		
		FormatDesc vbFmt[] = {
			0, TYPE_VERTEX, FORMAT_FLOAT, 3,
			0, TYPE_NORMAL, FORMAT_FLOAT, 3,
			0, TYPE_TANGENT,FORMAT_FLOAT, 3,
			0, TYPE_BINORMAL,FORMAT_FLOAT, 3,			
			0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,			
		};
		
		for (int i = 0; i < elementsOf(vbFmt); ++i)
		{
			vertexStride +=vbFmt[i].size * renderer->getFormatSize(vbFmt[i].format);
		}
		
		if ((render_data[0].vf = renderer->addVertexFormat(vbFmt, elementsOf(vbFmt), render_data[0].shader)) == VF_NONE) return;



		if ((render_data[0].diffuseSampler = renderer->addSamplerState(LINEAR, WRAP, WRAP, WRAP)) == SS_NONE) return;

		/*for (size_t i=0; i < this->TextureList.size(); i++)
		{
			std::string texturename = this->TextureList.at(i) + ".tga";
			render_data[0].baseTex.push_back(renderer->addTexture(texturename.c_str(), true, render_data[0].linear));
		}*/
	
		int32 numVerts = this->numVertices;
		RenderVertexPNUVTB* pVerts = new RenderVertexPNUVTB[numVerts];

		for (uint32 i = 0; i < this->numVertices; ++i)
		{
			pVerts[i].position = vertDatas[i].pos;//this->vertices[i];
			pVerts[i].normal = vertDatas[i].normal;
			pVerts[i].texcoord0 = noVec2(this->texCoords[i].x, 1.f - this->texCoords[i].y);
			pVerts[i].tangent = Tangents_[i];
			pVerts[i].binormal = Binormals_[i];
		}

		render_data[0].vb = renderer->addVertexBuffer(sizeof(RenderVertexPNUVTB) * this->numVertices  , STATIC, pVerts);
		render_data[0].ib = renderer->addIndexBuffer(this->nIndices, sizeof(WORD), STATIC, this->indices);

		render_data[0].vertexStart = 0;
		render_data[0].vertexEnd = numVerts;
		render_data[0].startIndex = 0;
		render_data[0].endIndex = this->nIndices;
	}


}

M2Object::~M2Object() {
	delete [] Tangents_;
	delete [] Binormals_;	
}


