#include "stdafx.h"

#include <cassert>
#include <algorithm>
#include <map>

#include <EngineCore/Math/Vector.h>
#include <EngineCore/Util/GameLog.h>

#include "database.h"
#include "M2_Manager.h"
#include "M2Particle.h"


#include "M2Loader.h"


//#include <Renderer/noRenderSystemDX9.h>
//#include <Renderer/noD3DResources.h>





extern TextureManager texturemanager;
extern int gameVersion;

struct RenderVertexPNUV
{
	noVec3 position;
	noVec3 normal;
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


noVec3 fixCoordSystem(noVec3 v)
{
	//return noVec3(v.x, v.z, -v.y);
	return noVec3(v.x, v.z, v.y);
}

noVec3 fixCoordSystem2(noVec3 v)
{	
	return noVec3(v.x, v.z, v.y);
}


noQuat fixCoordSystemQuat(noQuat v)
{
	//return noQuat(-v.x, -v.z, v.y, v.w);
	return noQuat(v.x, v.z, v.y, v.w);
}

M2AnimationManager* (* M2Loader::createAnimMgr) (ModelAnimation *anim, int numAnims);


// Sets up the models attachments
void M2Loader::setupAtt(int id)
{
	int l = attLookup[id];
	if (l>-1)
		atts[l].setup();
}

// Sets up the models attachments
void M2Loader::setupAtt2(int id)
{
	int l = attLookup[id];
	if (l>=0)
		atts[l].setupParticle();
}



void M2Loader::SetViewMatrix(const mat4& view)
{
	view_ = view;
}

void M2Loader::Load( const char* fname, bool bForceAnim )
{

	// replace .MDX with .M2
	wxString tempname(fname);
	tempname = tempname.BeforeLast(wxT('.')) + wxT(".m2");

	vertexStride = 0;
	trans = 1.0f;
	rad = 1.0f;
		
	for (size_t i=0; i<TEXTURE_MAX; i++) {
		specialTextures[i] = -1;
		replaceTextures[i] = 0;
		useReplaceTextures[i] = false;
	}

	for (size_t i=0; i<ATT_MAX; i++) 
		attLookup[i] = -1;

	for (size_t i=0; i<BONE_MAX; i++) 
		keyBoneLookup[i] = -1;


	hasCamera = false;
	hasParticles = false;
	isWMO = false;
	isMount = false;

	vertices = 0;
	normals = 0;
	texCoords = 0;
	indices = 0;

	animtime = 0;
	anim = 0;
	anims = 0;
	animLookups = 0;
	animated = false;

	bones = 0;
	bounds = 0;
	boundIndices = 0;
	currAnim = 0;
	colors = 0;
	globalSequences = 0;
	
	texAnims = 0;
	transparency = 0;

	modelType = MT_NORMAL;
	indiceToVerts = 0;
	forceAnim = bForceAnim;

	particleSystems = NULL;
	ribbons = NULL;

	charModelDetails.Reset();

	wxString fn(fname);
	charModelDetails.isChar = ((fn.Lower().Find(wxT("char"))  != wxNOT_FOUND )|| 
		(fn.Lower().Find(wxT("alternate\\char")) != wxNOT_FOUND));

	if (charModelDetails.isChar) modelType = MT_CHAR;

	bool isItem = (fn.Lower().Find(wxT("item"))  != wxNOT_FOUND);


	wxString str = tempname.substr(0, tempname.length()-3);
	name = str.wx_str();
	modelname = ModelPath(tempname.wx_str());
		
	MPQFile f(modelname.c_str());

		
	bool ok = true;

	memcpy(&header, f.getBuffer(), sizeof(ModelHeader));
	animated = isAnimated(f) || forceAnim;

	// Error check
	if (header.id[0] != 'M' && header.id[1] != 'D' && header.id[2] != '2' && header.id[3] != '0') {
		LOG_WARN << _T("Error:\t\tInvalid model!  May be corrupted.");
		ok = false;
		f.close();
		return;
	}

#ifdef WotLK
	//modelname = fname;

	if (header.nameOfs != 304 && header.nameOfs != 320) {
		LOG_INFO << _T("Error:\t\tInvalid model nameOfs=%d/%d!  May be corrupted.") <<  header.nameOfs << _T(" ") <<  sizeof(ModelHeader);
		//ok = false;
		f.close();
		return;
	}
#else
	if (header.nameOfs != 336) {
		LOG_INFO << _T("Error:\t\tInvalid model nameOfs=%d/%d!  May be corrupted.") << header.nameOfs << _T(" ") << sizeof(ModelHeader);
		//ok = false;
		f.close();
		return;
	}
#endif

	// Error check
	// 8 1 0 0 = WoW 3.0 models
	// 4 1 0 0 = WoW 2.0 models
	// 0 1 0 0 = WoW 1.0 models
	if (header.version[0] != 4 && header.version[1] != 1 && header.version[2] != 0 && header.version[3] != 0) {
		LOG_INFO << _T("Error:\t\tModel version is incorrect!\n\t\tMake sure you are loading models from World of Warcraft 2.0.1 or newer client.");
		ok = false;
		f.close();

		if (header.version[0] == 0)
			//wxMessageBox(wxString::Format(_T("An error occured while trying to load the model %s.\nWoW Model Viewer 0.5.x only supports loading WoW 2.0 models\nModels from WoW 1.12 or earlier are not supported"), fname), _T("Error: Unable to load model"));

		return;
	}

	if (f.getSize() < header.ofsParticleEmitters) {
		LOG_INFO << _T("Error: Unable to load the Model \"%s\", appears to be corrupted. ") << fname;
	}

	if (f.getSize() < header.ofsParticleEmitters) {
		LOG_INFO << _T("Error: Unable to load the Model \"%s\", appears to be corrupted. ") << fname;
	}
		
	if (header.nGlobalSequences) {
		globalSequences = new uint32[header.nGlobalSequences];
		memcpy(globalSequences, (f.getBuffer() + header.ofsGlobalSequences), header.nGlobalSequences * sizeof(uint32));
	}

	//if (forceAnim)
	//	animBones = true;
	

	if (animated) initAnimated(f);
	else initStatic(f);

	
	//if (animGeom) 
	//{
	//	char def[256];		
	//	sprintf(def, "#define MATRIX_PALETTE_SIZE_DEFAULT %d\n", header.nBones);

	//	//render_data[0].shader = renderer->addShader("Data/Shaders/skinnedShader.shd", def);
	//	render_data[0].shader = renderer->addShader("Data/Shaders/SkinColorPSSM.shd", def);

	//	FormatDesc vbFmt[] = {
	//		0, TYPE_VERTEX, FORMAT_FLOAT,  3,
	//		0, TYPE_NORMAL, FORMAT_FLOAT,   3,
	//		0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,
	//		0, TYPE_BONES,	 FORMAT_FLOAT,   4,
	//		0, TYPE_WEIGHTS, FORMAT_FLOAT,  4,
	//	};
	//	

	//	if ((render_data[0].vf = renderer->addVertexFormat(vbFmt, elementsOf(vbFmt), render_data[0].shader)) == VF_NONE) return;

	//	if ((render_data[0].linear = renderer->addSamplerState(LINEAR, WRAP, WRAP, WRAP)) == SS_NONE) return;

	//	int32 numVerts = this->numVertices;
	//	SkinnedVertex* pVerts = new SkinnedVertex[numVerts];

	//	for (uint32 i = 0; i < this->numVertices; ++i)
	//	{
	//		pVerts[i].position = vertDatas[i].pos;

	//		float y = vertDatas[i].texcoords.y;
	//		if (modelType != MT_CHAR)
	//			y = 1.0f - vertDatas[i].texcoords.y;;
	//		pVerts[i].texcoord0 = noVec2(vertDatas[i].texcoords.x, y);
	//		pVerts[i].normal = vertDatas[i].normal;
	//		for (uint32 x = 0; x < 4; ++x)
	//		{			
	//			pVerts[i].bones[x] = vertDatas[i].bones[x];
	//			pVerts[i].weights[x] = vertDatas[i].weights[x] / 255.f;
	//		}
	//	}
	//	render_data[0].vb = renderer->addVertexBuffer(sizeof(SkinnedVertex) * this->numVertices  , STATIC, pVerts);
	//	render_data[0].ib = renderer->addIndexBuffer(this->nIndices, sizeof(WORD), STATIC, this->indices);

	//	//SkinnedVertex
	//}
	//else 
	//{
	//	render_data[0].shader = renderer->addShader("Data/Shaders/ColorPSSM.shd");
	//	
	//	FormatDesc vbFmt[] = {
	//		0, TYPE_VERTEX, FORMAT_FLOAT, 3,
	//		0, TYPE_NORMAL, FORMAT_FLOAT, 3,
	//		0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,
	//	};
	//	if ((render_data[0].vf = renderer->addVertexFormat(vbFmt, elementsOf(vbFmt), render_data[0].shader)) == VF_NONE) return;



	//	if ((render_data[0].linear = renderer->addSamplerState(LINEAR, WRAP, WRAP, WRAP)) == SS_NONE) return;

	//	/*for (size_t i=0; i < this->TextureList.size(); i++)
	//	{
	//		std::string texturename = this->TextureList.at(i) + ".tga";
	//		render_data[0].baseTex.push_back(renderer->addTexture(texturename.c_str(), true, render_data[0].linear));
	//	}*/
	//
	//	int32 numVerts = this->numVertices;
	//	RenderVertex* pVerts = new RenderVertex[numVerts];

	//	for (uint32 i = 0; i < this->numVertices; ++i)
	//	{
	//		pVerts[i].position = vertDatas[i].pos;//this->vertices[i];
	//		pVerts[i].normal = vertDatas[i].normal;
	//		pVerts[i].texcoord0 = noVec2(this->texCoords[i].x, 1.f - this->texCoords[i].y);
	//	}

	//	render_data[0].vb = renderer->addVertexBuffer(sizeof(RenderVertex) * this->numVertices  , STATIC, pVerts);
	//	render_data[0].ib = renderer->addIndexBuffer(this->nIndices, sizeof(WORD), STATIC, this->indices);

	//	render_data[0].vertexStart = 0;
	//	render_data[0].vertexEnd = numVerts;
	//	render_data[0].startIndex = 0;
	//	render_data[0].endIndex = this->nIndices;
	//}

	//
	f.close();
}

void M2Loader::initCommon(MPQFile &f) {

	LoadVertices( f );
	// textures
	ModelTextureDef *texdef = (ModelTextureDef*)(f.getBuffer() + header.ofsTextures);
	if (header.nTextures) {
		textureIds = new M2::TextureID[header.nTextures];	// 게임 상의 데이터
		for (size_t i=0; i<header.nTextures; i++) {
			// Error check
			if (i > TEXTURE_MAX-1) {
				LOG_INFO << _T("Critical Error: Model Texture %d over %d") << header.nTextures << _T(" ") << TEXTURE_MAX;
				break;
			}
			/*
			Texture Types
			Texture type is 0 for regular textures, nonzero for skinned textures (filename not referenced in the M2 file!) 
			For instance, in the NightElfFemale model, her eye glow is a type 0 texture and has a file name, 
			the other 3 textures have types of 1, 2 and 6. The texture filenames for these come from client database files:

			DBFilesClient\CharSections.dbc
			DBFilesClient\CreatureDisplayInfo.dbc
			DBFilesClient\ItemDisplayInfo.dbc
			(possibly more)
				
			0	 Texture given in filename
			1	 Body + clothes
			2	Cape
			6	Hair, beard
			8	Tauren fur
			11	Skin for creatures #1
			12	Skin for creatures #2
			13	Skin for creatures #3

			Texture Flags
			Value	 Meaning
			1	Texture wrap X
			2	Texture wrap Y
			*/

			char texname[256] = "";
			if (texdef[i].type == TEXTURE_FILENAME) {
				strncpy(texname, (const char*)f.getBuffer() + texdef[i].nameOfs, texdef[i].nameLen);
				texname[texdef[i].nameLen] = 0;
				texname[texdef[i].nameLen - 4] = 't';
				texname[texdef[i].nameLen - 3] = 'g';
				texname[texdef[i].nameLen - 2] = 'a';
				textureIds[i] = renderer->addTexture(TexPath(texname).c_str(), true);
				//texturemanager.add(texname);
//				noTexture2D* pTex = new noTexture2D();
	//			pTex->SetTextureFile(&texname[0]);
				TextureList.push_back(texname);
				TextureMap.insert(std::make_pair(textureIds[i], TexPath(texname)));
				LOG_INFO << "Info: Added %s to the TextureList." << texname;
			} else {
				// special texture - only on characters and such...
				textureIds[i] = 0;
				//while (texdef[i].type < TEXTURE_MAX && specialTextures[texdef[i].type]!=-1) texdef[i].type++;
				//if (texdef[i].type < TEXTURE_MAX)specialTextures[texdef[i].type] = (int)i;
				specialTextures[i] = texdef[i].type;

				wxString tex = wxT("Special_");
				tex << texdef[i].type;
								
				if (modelType == MT_NORMAL){
					
					if (texdef[i].type == TEXTURE_HAIR)
						tex = wxT("Hair.blp");
					else if(texdef[i].type == TEXTURE_BODY)
						tex = wxT("Body.blp");
					else if(texdef[i].type == TEXTURE_FUR)
						tex = wxT("Fur.blp");
				}
				/*else if (modelType == MT_CHAR || modelType == MT_NPC){
					if (texdef[i].type == TEXTURE_HAIR){
						strncpy(texname,(const char*)"Hair.blp",8);
						TextureList.push_back(texname);
						LOG_INFO << "Info: Added %s to the TextureList via specialTextures."<< texname;
					}else if(texdef[i].type == TEXTURE_BODY){
						strncpy(texname,(const char*)"Body.blp",8);
						TextureList.push_back(texname);
						LOG_INFO << "Info: Added %s to the TextureList via specialTextures."<< texname;
					}else if(texdef[i].type == TEXTURE_CAPE){
						strncpy(texname,(const char*)"Cape.blp",8);
						TextureList.push_back(texname);
						LOG_INFO << "Info: Added %s to the TextureList via specialTextures."<< texname;
					}else if(texdef[i].type == TEXTURE_FUR){
						strncpy(texname,(const char*)"Fur.blp",7);
						TextureList.push_back(texname);
						LOG_INFO << "Info: Added %s to the TextureList via specialTextures."<< texname;
					}else if(texdef[i].type == TEXTURE_ARMORREFLECT){
						strncpy(texname,(const char*)"Reflection.blp",14);
						TextureList.push_back(texname);
						LOG_INFO << "Info: Added %s to the TextureList via specialTextures."<< texname;
					}else if(texdef[i].type == TEXTURE_GAMEOBJECT1){
						strncpy(texname,(const char*)"ChangableTexture 1.blp",22);
						TextureList.push_back(texname);
						LOG_INFO << "Info: Added %s to the TextureList via specialTextures."<< texname;
					}else if(texdef[i].type == TEXTURE_GAMEOBJECT2){
						strncpy(texname,(const char*)"ChangableTexture 2.blp",22);
						TextureList.push_back(texname);
						LOG_INFO << "Info: Added %s to the TextureList via specialTextures."<< texname;
					}else if(texdef[i].type == TEXTURE_GAMEOBJECT3){
						strncpy(texname,(const char*)"ChangableTexture 3.blp",22);
						TextureList.push_back(texname);
						LOG_INFO << "Info: Added %s to the TextureList via specialTextures."<< texname;
					}
				}*/
					
					wxLogMessage(wxT("Info: Added %s to the TextureList[%i] via specialTextures. Type: %i"), tex.c_str(), TextureList.size(), texdef[i].type);
					TextureList.push_back(tex.wx_str());

					if (texdef[i].type < TEXTURE_MAX)
						useReplaceTextures[texdef[i].type] = true;

					if (texdef[i].type == TEXTURE_ARMORREFLECT) {
						// a fix for weapons with type-3 textures.
						replaceTextures[texdef[i].type] = renderer->addTexture(wxT(TexPath("Item\\ObjectComponents\\Weapon\\ArmorReflect4.TGA")).c_str(), true);
					}								
			}
		}
	}
	
	// replacable textures - it seems to be better to get this info from the texture types
	/*if (header.nTexReplace) {
		size_t m = header.nTexReplace;
		if (m>16) m = 16;
		int16 *texrep = (int16*)(f.getBuffer() + header.ofsTexReplace);
		for (size_t i=0; i<m; i++) specialTextures[i] = texrep[i];
	}*/
	

		// attachments
	// debug code here
	if (header.nAttachments) {
		ModelAttachmentDef *attachments = (ModelAttachmentDef*)(f.getBuffer() + header.ofsAttachments);
		for (size_t i=0; i<header.nAttachments; i++) {
			ModelAttachment att;
			att.model = this;
			att.init(f, attachments[i], globalSequences);
			atts.push_back(att);
		}
	}

	if (header.nAttachLookup) {
		int16 *p = (int16*)(f.getBuffer() + header.ofsAttachLookup);
		if (header.nAttachLookup > ATT_MAX)
			LOG_INFO << _T("Critical Error: Model AttachLookup ") <<  header.nAttachLookup << " over " << ATT_MAX;
		for (size_t i=0; i<header.nAttachLookup; i++) {
			if (i>ATT_MAX-1)
				break;
			attLookup[i] = p[i];
		}
	}


	// init colors
	if (header.nColors) {
		ModelColorDef *colorDefs = (ModelColorDef*)(f.getBuffer() + header.ofsColors);
		colors = new ModelColor[header.nColors];
		for (size_t i=0; i<header.nColors; i++) 
			colors[i].init(f, colorDefs[i], globalSequences);
	}

	// init transparency
	if (header.nTransparency) {
		ModelTransDef *trDefs = (ModelTransDef*)(f.getBuffer() + header.ofsTransparency);
		transparency = new ModelTransparency[header.nTransparency];		
		for (size_t i=0; i<header.nTransparency; i++) 
			transparency[i].init(f, trDefs[i], globalSequences);
	}

	if (header.nViews) {
		// just use the first LOD/view
		// First LOD/View being the worst?
		// TODO: Add support for selecting the LOD.
		// indices - allocate space, too
		// header.nViews;
		// int viewLOD = 0; // sets LOD to worst
		// int viewLOD = header.nViews - 1; // sets LOD to best
		//setLOD(f, header.nViews - 1); // Set the default Level of Detail to the best possible. 
		setLOD(f, 0);
	}

	if (nIndices) {
		indiceToVerts = new size_t[nIndices*2];
		for (size_t i=0; i <nIndices; ++i) {
			size_t a = indices[i];
			for (size_t j=0; j < header.nVertices; ++j) {
				if (a < header.nVertices && vertDatas[a].pos == vertDatas[j].pos) {
					indiceToVerts[i] = j;
					break;
				}
			}
		}
	}	
}



void M2Loader::initAnimated( MPQFile &f ) {
	if (vertDatas) {
		delete [] vertDatas;
		vertDatas = NULL;
	}

	if (header.nVertices > 0)
	{	
		vertDatas = new ModelVertex[header.nVertices];
		memcpy(vertDatas, f.getBuffer() + header.ofsVertices, header.nVertices * sizeof(ModelVertex));
	}

	initCommon(f);

	

	if (header.nAnimations > 0) {
		anims = new ModelAnimation[header.nAnimations];

		if (gameVersion < VERSION_WOTLK) {
			memcpy(anims, f.getBuffer() + header.ofsAnimations, header.nAnimations * sizeof(ModelAnimation));
		} else {
			ModelAnimationWotLK animsWotLK;
			char tempname[256];
			animfiles = new MPQFile[header.nAnimations];
			for(size_t i=0; i<header.nAnimations; i++) {
				memcpy(&animsWotLK, f.getBuffer() + header.ofsAnimations + i*sizeof(ModelAnimationWotLK), sizeof(ModelAnimationWotLK));
				anims[i].animID = animsWotLK.animID;
				anims[i].timeStart = 0;
				anims[i].timeEnd = animsWotLK.length;
				anims[i].moveSpeed = animsWotLK.moveSpeed;
				anims[i].flags = animsWotLK.flags;
				anims[i].probability = animsWotLK.probability;
				anims[i].d1 = animsWotLK.d1;
				anims[i].d2 = animsWotLK.d2;
				anims[i].playSpeed = animsWotLK.playSpeed;
				anims[i].rad = animsWotLK.rad;
				anims[i].NextAnimation = animsWotLK.NextAnimation;
				anims[i].Index = animsWotLK.Index;
								
				sprintf(tempname, "%s%04d-%02d.anim", fullname.c_str(), anims[i].animID, animsWotLK.subAnimID);
				if (MPQFile::getSize(tempname) > 0) {
					animfiles[i].openFile(tempname);
					
				}				
			}
		}
		animManager =  M2Loader::createAnimMgr(anims, header.nAnimations);
		//new M2AnimationManager(anims, header.nAnimations);
	}

	if (animBones) {

		// init bones...		
		bones = new Bone[header.nBones];
		ModelBoneDef *mb = (ModelBoneDef*)(f.getBuffer() + header.ofsBones);
		for (size_t i=0; i<header.nBones; i++) {
			//if (i==0) mb[i].rotation.ofsRanges = 1.0f;
			//if (gameVersion >= VERSION_WOTLK) {
			bones[i].model = this;			
			bones[i].initV3(f, mb[i], globalSequences, animfiles);
			//} else {
			//	bones[i].initV2(f, mb[i], globalSequences);
			//}

		}
		
		// Block keyBoneLookup is a lookup table for Key Skeletal Bones, hands, arms, legs, etc.
		if (header.nKeyBoneLookup < BONE_MAX) {
			memcpy(keyBoneLookup, f.getBuffer() + header.ofsKeyBoneLookup, sizeof(int16)*header.nKeyBoneLookup);
		} else {
			memcpy(keyBoneLookup, f.getBuffer() + header.ofsKeyBoneLookup, sizeof(int16)*BONE_MAX);
			LOG_ERR << _T("Error: keyBone number ") <<  header.nKeyBoneLookup << _T(" over ") <<  BONE_MAX;
		}	
		
	}



	// Index at ofsAnimations which represents the animation in AnimationData.dbc. -1 if none.
	if (header.nAnimationLookup > 0) {
		animLookups = new int16[header.nAnimationLookup];
		memcpy(animLookups, f.getBuffer() + header.ofsAnimationLookup, sizeof(int16)*header.nAnimationLookup);
	}

	if (header.nVertices > 0)
	{	
		const size_t size = (header.nVertices * sizeof(float));
		vbufsize = (3 * size); // we multiple by 3 for the x, y, z positions of the vertex

		texCoords = new noVec2[header.nVertices];
		for (size_t i=0; i<header.nVertices; i++) 
			texCoords[i] = vertDatas[i].texcoords;
	}

	if (animTextures) {
		texAnims = new TextureAnim[header.nTexAnims];
		ModelTexAnimDef *ta = (ModelTexAnimDef*)(f.getBuffer() + header.ofsTexAnims);
		for (size_t i=0; i<header.nTexAnims; i++)
			texAnims[i].init(f, ta[i], globalSequences);
	}

	if (header.nEvents) {
		ModelEventDef *edefs = (ModelEventDef *)(f.getBuffer()+header.ofsEvents);
		events = new M2AnimationEvent[header.nEvents];
		for (size_t i=0; i<header.nEvents; i++) {
			events[i].init(f, edefs[i], globalSequences);
		}
	}

	// particle systems
	if (header.nParticleEmitters) {
		if (header.version[0] >= 0x10) {
			ModelParticleEmitterDefV10 *pdefsV10 = (ModelParticleEmitterDefV10 *)(f.getBuffer() + header.ofsParticleEmitters);
			ModelParticleEmitterDef *pdefs;
			particleSystems = new M2ParticleSystem[header.nParticleEmitters];
			hasParticles = true;
			for (size_t i=0; i<header.nParticleEmitters; i++) {
				pdefs = (ModelParticleEmitterDef *) &pdefsV10[i];
				particleSystems[i].model = this;
				particleSystems[i].init(f, *pdefs, globalSequences);
			}
		} else {
			ModelParticleEmitterDef *pdefs = (ModelParticleEmitterDef *)(f.getBuffer() + header.ofsParticleEmitters);
			particleSystems = new M2ParticleSystem[header.nParticleEmitters];
			hasParticles = true;
			for (size_t i=0; i<header.nParticleEmitters; i++) {
				particleSystems[i].model = this;
				particleSystems[i].init(f, pdefs[i], globalSequences);
			}
		}
	}

	// ribbons
	if (header.nRibbonEmitters) {
		ModelRibbonEmitterDef *rdefs = (ModelRibbonEmitterDef *)(f.getBuffer() + header.ofsRibbonEmitters);
		ribbons = new M2RibbonEmitter[header.nRibbonEmitters];
		for (size_t i=0; i<header.nRibbonEmitters; i++) {
			ribbons[i].model = this;
			ribbons[i].init(f, rdefs[i], globalSequences);
		}
	}

	// Cameras
	if (header.nCameras>0) {
		if (header.version[0] <= 9){
			ModelCameraDef *camDefs = (ModelCameraDef*)(f.getBuffer() + header.ofsCameras);
			for (size_t i=0;i<header.nCameras;i++){
				ModelCamera a;
				a.init(f, camDefs[i], globalSequences, modelname);
				cameras.push_back(a);
			}
		}else if (header.version[0] <= 16){
			ModelCameraDefV10 *camDefs = (ModelCameraDefV10*)(f.getBuffer() + header.ofsCameras);
			for (size_t i=0;i<header.nCameras;i++){
				ModelCamera a;
				a.initv10(f, camDefs[i], globalSequences, modelname);
				cameras.push_back(a);
			}
		}
		if (cameras.size() > 0){
			hasCamera = true;
		}
	}

	// init lights
	if (header.nLights) {
		lights = new ModelLight[header.nLights];
		ModelLightDef *lDefs = (ModelLightDef*)(f.getBuffer() + header.ofsLights);
		for (size_t i=0; i<header.nLights; i++) {
			lights[i].init(f, lDefs[i], globalSequences);
		}
	}
		
	//currAnim = 3;
	//animManager->SetAnim(0, 3, 0);
	//animManager->ProcessAnimations(this);

#ifdef WotLK
	// free MPQFile
	if (header.nAnimations > 0) {
		for(size_t i=0; i<header.nAnimations; i++) {
			if(animfiles[i].getSize() > 0)
				animfiles[i].close();
		}
		delete [] animfiles;
	}
#endif
	
	animcalc = false;
	
}

bool M2Loader::isAnimated( MPQFile& f )
{	
	// see if we have any animated bones
	ModelBoneDef *bo = (ModelBoneDef*)(f.getBuffer() + header.ofsBones);

	animGeom = false;
	animBones = false;
	ind = false;

	ModelVertex *verts = (ModelVertex*)(f.getBuffer() + header.ofsVertices);
	for (size_t i=0; i<header.nVertices && !animGeom; i++) {
		for (size_t b=0; b<4; b++) {
			if (verts[i].weights[b]>0) {
				ModelBoneDef &bb = bo[verts[i].bones[b]];
				if (bb.translation.type || bb.rotation.type || bb.scaling.type || (bb.flags & MODELBONE_BILLBOARD)) {
					if (bb.flags & MODELBONE_BILLBOARD) {
						// if we have billboarding, the model will need per-instance animation
						ind = true;
					}
					animGeom = true;
					break;
				}
			}
		}
	}

	if (animGeom) 
		animBones = true;
	else {
		for (size_t i=0; i<header.nBones; i++) {
			ModelBoneDef &bb = bo[i];
			if (bb.translation.type || bb.rotation.type || bb.scaling.type) {
				animBones = true;
				animGeom = true;
				break;
			}
		}
	}

	animTextures = header.nTexAnims > 0;

	bool animMisc = header.nCameras>0 || // why waste time, pretty much all models with cameras need animation
		header.nLights>0 || // same here
		header.nParticleEmitters>0 ||
		header.nRibbonEmitters>0;

	if (animMisc) 
		animBones = true;

	// animated colors
	if (header.nColors) {
		ModelColorDef *cols = (ModelColorDef*)(f.getBuffer() + header.ofsColors);
		for (size_t i=0; i<header.nColors; i++) {
			if (cols[i].color.type!=0 || cols[i].opacity.type!=0) {
				animMisc = true;
				break;
			}
		}
	}

	// animated opacity
	if (header.nTransparency && !animMisc) {
		ModelTransDef *trs = (ModelTransDef*)(f.getBuffer() + header.ofsTransparency);
		for (size_t i=0; i<header.nTransparency; i++) {
			if (trs[i].trans.type!=0) {
				animMisc = true;
				break;
			}
		}
	}

	// guess not...
	return animGeom || animTextures || animMisc;	
}

bool M2Loader::initStatic( MPQFile &f )
{
	vertDatas = (ModelVertex*)(f.getBuffer() + header.ofsVertices);

	initCommon(f);

	return true;
}

void M2Loader::LoadVertices( MPQFile &f )
{
	// assume: vertDatas already set

	numVertices = header.nVertices;

	if (header.nVertices == 0)
		return;		

	// This data is needed for both VBO and non-VBO cards.
	vertices = new noVec3[header.nVertices];
	normals = new noVec3[header.nVertices];

	// Correct the data from the model, so that its using the Y-Up axis mode.
	for (size_t i=0; i<header.nVertices; i++) {
		vertDatas[i].pos = fixCoordSystem(vertDatas[i].pos);
		vertDatas[i].normal = fixCoordSystem(vertDatas[i].normal);

		// Set the data for our vertices, normals from the model data
		if (!animGeom) {
			vertices[i] = vertDatas[i].pos;
			normals[i] = vertDatas[i].normal;
			normals[i].Normalize();
		}

		float len = vertDatas[i].pos.LengthSqr();
		if (len > rad)	{ 
			rad = len;
		}
	}

	// model vertex radius
	rad = sqrtf(rad);

	// bounds
	if (header.nBoundingVertices > 0) {
		bounds = new noVec3[header.nBoundingVertices];
		noVec3 *b = (noVec3*)(f.getBuffer() + header.ofsBoundingVertices);
		for (size_t i=0; i<header.nBoundingVertices; i++) {
			bounds[i] = fixCoordSystem(b[i]);
		}
	}
	if (header.nBoundingTriangles > 0) {
		boundIndices = new uint16[header.nBoundingTriangles];
		memcpy(boundIndices, f.getBuffer() + header.ofsBoundingTriangles, header.nBoundingTriangles*sizeof(uint16));
	}
}

//M2Loader::M2Loader()
//{
//
//}

M2Loader::M2Loader( Renderer* _renderer )
	:renderer(_renderer)
{

	anims = 0;
	vertDatas = 0;
	vertices  = normals = 0;
	texCoords = 0;

	indices = 0;
	nIndices = 0;

	bounds = 0;
	boundIndices = 0;
	textureIds = 0;


	texAnims = 0;
	globalSequences = 0;
	colors = 0;
	transparency = 0;
	lights = 0;
}

void M2Loader::setLOD(MPQFile &f, int index)
{
	// Texture definitions
	ModelTextureDef *texdef = (ModelTextureDef*)(f.getBuffer() + header.ofsTextures);
	
	int16 *transLookup = (int16*)(f.getBuffer() + header.ofsTransparencyLookup);

	// I thought the view controlled the Level of detail,  but that doesn't seem to be the case.
	// Seems to only control the render order.  Which makes this function useless and not needed :(
	#ifdef WotLK
		// remove suffix .M2
		lodname = modelname.substr(0, modelname.length()-3);
		fullname = lodname;
		lodname.append("00.skin"); // Lods: 00, 01, 02, 03
		MPQFile g(lodname.c_str());
		//g_modelViewer->modelOpened->Add(wxString(lodname.c_str(), wxConvUTF8));
		if (g.isEof()) {
			LOG_INFO << _T("Error: Unable to load Lods: [%s]") <<  lodname.c_str();
			g.close();
			return;
		}
		
		ModelView *view = (ModelView*)(g.getBuffer());

		if (view->id[0] != 'S' || view->id[1] != 'K' || view->id[2] != 'I' || view->id[3] != 'N') {
			LOG_INFO << _T("Error: Unable to load Lods: [%s]") << lodname.c_str();
			g.close();
			return;
		}

		// Indices,  Triangles
		uint16 *indexLookup = (uint16*)(g.getBuffer() + view->ofsIndex);
		uint16 *triangles = (uint16*)(g.getBuffer() + view->ofsTris);
		nIndices = view->nTris;
		if (indices ) 
		{
			delete indices;
			indices = 0;
		}
		indices = new uint16[nIndices];
		for (size_t i = 0; i<nIndices; i++) {
	        indices[i] = indexLookup[triangles[i]];
		}

		// render ops
		ModelGeoset *ops = (ModelGeoset*)(g.getBuffer() + view->ofsSub);
		ModelTexUnit *tex = (ModelTexUnit*)(g.getBuffer() + view->ofsTex);
#else // not WotLK
		ModelView *view = (ModelView*)(f.getBuffer() + header.ofsViews);

		// Indices,  Triangles
		uint16 *indexLookup = (uint16*)(f.getBuffer() + view->ofsIndex);
		uint16 *triangles = (uint16*)(f.getBuffer() + view->ofsTris);
		nIndices = view->nTris;
		if (indices ) 
		{
			delete indices;
			indices = 0;
		}
		indices = new uint16[nIndices];
		for (size_t i = 0; i<nIndices; i++) {
	        indices[i] = indexLookup[triangles[i]];
		}

		// render ops
		ModelGeoset *ops = (ModelGeoset*)(f.getBuffer() + view->ofsSub);
		ModelTexUnit *tex = (ModelTexUnit*)(f.getBuffer() + view->ofsTex);
#endif // WotLK

		ModelRenderFlags *renderFlags = (ModelRenderFlags*)(f.getBuffer() + header.ofsTexFlags);
		uint16 *texlookup = (uint16*)(f.getBuffer() + header.ofsTexLookup);
		uint16 *texanimlookup = (uint16*)(f.getBuffer() + header.ofsTexAnimLookup);
		int16 *texunitlookup = (int16*)(f.getBuffer() + header.ofsTexUnitLookup);

		
		showGeosets = new bool[view->nSub];
		for (size_t i=0; i<view->nSub; i++) {
			geosets.push_back(ops[i]);
			showGeosets[i] = true;
		}

		renderPasses.clear();
		for (size_t j = 0; j<view->nTex; j++) {
			ModelRenderPass pass;

			pass.useTex2 = false;
			pass.useEnvMap = false;
			pass.cull = false;
			pass.trans = false;
			pass.unlit = false;
			pass.noZWrite = false;
			pass.billboard = false;

			//pass.texture2 = 0;
			size_t geoset = tex[j].op;
			
			pass.geoset = (int)geoset;

			pass.indexStart = ops[geoset].istart;
			pass.indexCount = ops[geoset].icount;
			pass.vertexStart = ops[geoset].vstart;
			pass.vertexEnd = pass.vertexStart + ops[geoset].vcount;
			
			//pass.order = tex[j].shading; //pass.order = 0;
			
			//TextureID texid = textures[texlookup[tex[j].textureid]];
			//pass.texture = texid;
			pass.tex = texlookup[tex[j].textureid];
			
			/*
			// Render Flags
			flags:
			0x01 = Unlit
			0x02 = ? glow effects ? no zwrite?
			0x04 = Two-sided (no backface culling if set)
			0x08 = (probably billboarded)
			0x10 = Disable z-buffer?

			blend:
			Value	 Mapped to	 Meaning
			0	 	0	 		Combiners_Opaque
			1	 	1	 		Combiners_Mod
			2	 	1	 		Combiners_Decal
			3	 	1	 		Combiners_Add
			4	 	1	 		Combiners_Mod2x
			5	 	4	 		Combiners_Fade
			6	 	4	 		Used in the Deeprun Tram subway glass, supposedly (src=dest_color, dest=src_color) (?)
			*/
			// TODO: figure out these flags properly -_-
			ModelRenderFlags &rf = renderFlags[tex[j].flagsIndex];
			
			pass.blendmode = rf.blend;
			//if (rf.blend == 0) // Test to disable/hide different blend types
			//	continue;

			pass.color = tex[j].colorIndex;
			pass.opacity = transLookup[tex[j].transid];

			pass.unlit = (rf.flags & RENDERFLAGS_UNLIT)!= 0;

			// This is wrong but meh.. best I could get it so far.
			//pass.cull = (rf.flags & 0x04)==0 && pass.blendmode!=1 && pass.blendmode!=4 && (rf.flags & 17)!=17;
			//pass.cull = false; // quick test
			pass.cull = (rf.flags & RENDERFLAGS_TWOSIDED)==0; //&& rf.blend==0;

			pass.billboard = (rf.flags & RENDERFLAGS_BILLBOARD) != 0;

			pass.useEnvMap = (texunitlookup[tex[j].texunit] == -1) && pass.billboard && rf.blend>2; //&& rf.blend<5; // Use environmental reflection effects?

			// Disable environmental mapping if its been unchecked.
			//if (pass.useEnvMap && !video.useEnvMapping)
			//	pass.useEnvMap = false;


			//pass.noZWrite = (texdef[pass.tex].flags & 3)!=0;
			/*
			if (name == "Creature\\Turkey\\turkey.m2") // manual fix as I just bloody give up.
				pass.noZWrite = false;
			else
				pass.noZWrite = (pass.blendmode>1);
			*/
				//pass.noZWrite = (pass.blendmode>1) && !(rf.blend==4 && rf.flags==17);
			pass.noZWrite = (rf.flags & RENDERFLAGS_ZBUFFERED) != 0;

			// ToDo: Work out the correct way to get the true/false of transparency
			pass.trans = (pass.blendmode>0) && (pass.opacity>0);	// Transparency - not the correct way to get transparency

			pass.p = ops[geoset].BoundingBox[0].z;

			// Texture flags
			pass.swrap = (texdef[pass.tex].flags & TEXTURE_WRAPX) != 0; // Texture wrap X
			pass.twrap = (texdef[pass.tex].flags & TEXTURE_WRAPY) != 0; // Texture wrap Y
			
			if (animTextures && (tex[j].flags & TEXTUREUNIT_STATIC) == 0) {
				// tex[j].flags: Usually 16 for static textures, and 0 for animated textures.					
					pass.texanim = texanimlookup[tex[j].texanimid];				
			} else {
				pass.texanim = -1; // no texture animation
			}
			renderPasses.push_back(pass);
		}

#ifdef WotLK
		g.close();
#endif
		// transparent parts come later
		//std::sort(passes.begin(), passes.end());
}

void M2Loader::GetRenderData( RenderData& inout )
{
	inout.vf = render_data[0].vf;
	inout.ib = render_data[0].ib;
	inout.vb = render_data[0].vb;
	inout.shader = render_data[0].shader;
	inout.diffuseSampler = render_data[0].diffuseSampler;
	inout.baseTex = render_data[0].baseTex;
	inout.startIndex = render_data[0].startIndex;
	inout.endIndex = render_data[0].endIndex;
	inout.vertexEnd = render_data[0].vertexEnd;

}

void ModelTransparency::init( MPQFile &f, ModelTransDef &mtd, uint32 *global )
{
	trans.init(mtd.trans, f, global);
}

void ModelColor::init( MPQFile &f, ModelColorDef &mcd, uint32 *global )
{
	color.init(mcd.color, f, global);
	opacity.init(mcd.opacity, f, global);
}

//
void M2Loader::CreateRenderPass()
{
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
			// Texture
			// ALPHA BLENDING
			// blend mode
			BlendStateID blend = -1;
			switch (pass.blendmode) {
			case BM_OPAQUE:	// 0							
				break;
			case BM_TRANSPARENT: // 1
				//glEnable(GL_ALPHA_TEST);
				//glAlphaFunc(GL_GEQUAL,0.7f); // Dx10 이후 Alpha Test 없어짐. 				
				break;
			case BM_ALPHA_BLEND:
				blend = renderer->addBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA,
					D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, 0x0F, D3D11_BLEND_OP_ADD);
				break;
			case BM_ADDITIVE: // 3
				blend = renderer->addBlendState(D3D11_BLEND_SRC_COLOR, D3D11_BLEND_ONE, 
					D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, 0x0F, D3D11_BLEND_OP_ADD);
				break;
			case BM_ADDITIVE_ALPHA:
				blend = renderer->addBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE,
					D3D11_BLEND_ZERO, D3D11_BLEND_ZERO,  0x0F, D3D11_BLEND_OP_ADD);
				break;
			case BM_MODULATE:
				blend = renderer->addBlendState(D3D11_BLEND_DEST_COLOR, D3D11_BLEND_SRC_COLOR, 
					D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, 0x0F, D3D11_BLEND_OP_ADD);
				break;
			case BM_MODULATEX2:	// 6, not sure if this is right
				blend = renderer->addBlendState(D3D11_BLEND_DEST_COLOR, D3D11_BLEND_SRC_COLOR, 
					D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, 0x0F, D3D11_BLEND_OP_ADD);
				break;
			default:
				blend = renderer->addBlendState(D3D11_BLEND_DEST_COLOR, D3D11_BLEND_SRC_COLOR, 
					D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, 0x0F, D3D11_BLEND_OP_ADD);
				break;
			}

			
			data.blendMode = blend;
				

			if (pass.cull)
				data.cull = renderer->addRasterizerState(D3D11_CULL_FRONT);
			else 
				data.cull = renderer->addRasterizerState(D3D11_CULL_NONE);

			DepthStateID depthid = -1;
			if (pass.noZWrite) // depth write == false
				depthid = renderer->addDepthState(false, false);
			else 				
				depthid = renderer->addDepthState(true, true);
				
			data.depthMode = depthid;
			data.unlit = pass.unlit;			
		}
	}
	numPass = currPass;
}

void M2Loader::HideGeosets()
{
	for (size_t i=0; i< geosets.size(); i++) {
		showGeosets[i] = (geosets[i].id==0);
	}
}

void M2Loader::GetRaceAndGenderName(wxString& raceName, wxString& genderName)
{
	size_t p1 = name.find_first_of('\\', 0);
	size_t p2 = name.find_first_of('\\', p1+1);
	size_t p3 = name.find_first_of('\\', p2+1);

	raceName = name.substr(p1+1,p2-p1-1);
	genderName = name.substr(p2+1,p3-p2-1);
};

bool M2Loader::UpdateColors( int color, int opacity, bool unlit, noVec4& opacitycolor, noVec4& emissiveColor )
{
	noVec4 ocol = noVec4(1.0f, 1.0f, 1.0f, trans);
	noVec4 ecol = noVec4(0.0f, 0.0f, 0.0f, 0.0f);

	//if (m->trans == 1.0f)
	//	return false;

	// emissive colors
	if (color!=-1 && colors && colors[color].color.uses(0)) {
		noVec3 c;
		if (gameVersion >= VERSION_WOTLK) {
			/* Alfred 2008.10.02 buggy opacity make model invisable, TODO */
			c = colors[color].color.getValue(0,animtime);
			if (colors[color].opacity.uses(anim)) {
				ocol.w = colors[color].opacity.getValue(anim,animtime);
			}
		} else {
			c = colors[color].color.getValue(anim,animtime);
			ocol.w = colors[color].opacity.getValue(anim,animtime);
		}

		if (unlit) {
			ocol.x = c.x; ocol.y = c.y; ocol.z = c.z;
		} else {
			ocol.x = ocol.y = ocol.z = 0;
		}

		ecol = noVec4(c.x, c.y, c.z, ocol.w);			
	}

	// opacity
	if (opacity!=-1) {
		if (gameVersion >= VERSION_WOTLK) {
			/* Alfred 2008.10.02 buggy opacity make model invisable, TODO */
			if (transparency && transparency[opacity].trans.uses(0))
				ocol.w *= transparency[opacity].trans.getValue(0, animtime);
		} else
			ocol.w *= transparency[opacity].trans.getValue(anim, animtime);
	}

	emissiveColor = ecol;
	opacitycolor.SetAll(ocol.w);

	// exit and return false before affecting the opengl render state
	if (!((ocol.w > 0) && (color==-1 || ecol.w > 0)))
		return false;


	return true;
}

void M2Loader::DrawModel(const mat4& world, const mat4& view, const mat4& proj)
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
			for (int i = 0; i <= num; i++)						
			{
				const RenderData& rd = render_data[i];

				if (!UpdateColors(rd.colorIndex, rd.opacity, rd.unlit, ocolor, ecolor))
					continue;

				renderer->reset();	
				renderer->setVertexFormat(rd.vf);
				renderer->setVertexBuffer(0, rd.vb);
				renderer->setIndexBuffer(rd.ib);
				renderer->setRasterizerState(rd.cull);
				renderer->setShader(rd.shader);	
				renderer->setSamplerState("g_samLinear", render_data[0].diffuseSampler);
				renderer->setTexture("g_txDiffuse", rd.baseTex);			
				if (rd.blendMode != -1) renderer->setBlendState(rd.blendMode);
				if (rd.depthMode != -1) renderer->setDepthState(rd.depthMode);
				renderer->setShaderConstant4x4f("g_mWorld", world);
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

				renderer->reset();	
				renderer->setVertexFormat(rd.vf);
				renderer->setVertexBuffer(0, rd.vb);
				renderer->setIndexBuffer(rd.ib);
				renderer->setRasterizerState(rd.cull);
				renderer->setShader(rd.shader);	
				renderer->setSamplerState("samLinear", render_data[0].diffuseSampler);
				renderer->setTexture("txDiffuse", rd.baseTex);			
				if (rd.blendMode != -1) renderer->setBlendState(rd.blendMode);
				if (rd.depthMode != -1) renderer->setDepthState(rd.depthMode);
				renderer->setShaderConstant4x4f("World", world);
				renderer->setShaderConstant4x4f("View", view);
				renderer->setShaderConstant4x4f("Projection", proj);	
				//renderer->setShaderConstant4x4f("g_mWorldViewProjection",  world *view * proj);								
				renderer->apply();
				renderer->drawElements(PRIM_TRIANGLES, rd.startIndex, rd.endIndex, rd.vertexStart, rd.vertexEnd);
			}
		}
	}
}

void M2Loader::calcBones(ssize_t anim, size_t time)
{	
	// Reset all bones to 'false' which means they haven't been animated yet.
	for (size_t i=0; i<header.nBones; i++) {
		bones[i].calc = false;
	}

	// Character specific bone animation calculations.
	if (charModelDetails.isChar) {	

		// Animate the "core" rotations and transformations for the rest of the model to adopt into their transformations
		if (keyBoneLookup[BONE_ROOT] > -1)	{
			for (size_t i=0; i<=keyBoneLookup[BONE_ROOT]; i++) {				
				bones[i].calcMatrix(bones, anim, time);
			}
		}

		// Find the close hands animation id
		int closeFistID = 0;
		/*
		for (size_t i=0; i<header.nAnimations; i++) {
			if (anims[i].animID==15) {  // closed fist
				closeFistID = i;
				break;
			}
		}
		*/
		// Alfred 2009.07.23 use animLookups to speedup
		if (header.nAnimationLookup >= ANIMATION_HANDSCLOSED && animLookups[ANIMATION_HANDSCLOSED] > 0) // closed fist
			closeFistID = animLookups[ANIMATION_HANDSCLOSED];

		// Animate key skeletal bones except the fingers which we do later.
		// -----
		size_t a, t;

		// if we have a "secondary animation" selected,  animate upper body using that.
		if (animManager->GetSecondaryID() > -1) {
			a = animManager->GetSecondaryID();
			t = animManager->GetSecondaryFrame();
		} else {
			a = anim;
			t = time;
		}

		for (size_t i=0; i<animManager->GetSecondaryCount(); i++) { // only goto 5, otherwise it affects the hip/waist rotation for the lower-body.
			if (keyBoneLookup[i] > -1)
				bones[keyBoneLookup[i]].calcMatrix(bones, a, t);
		}

		if (animManager->GetMouthID() > -1) {
			// Animate the head and jaw
			if (keyBoneLookup[BONE_HEAD] > -1)
					bones[keyBoneLookup[BONE_HEAD]].calcMatrix(bones, animManager->GetMouthID(), animManager->GetMouthFrame());
			if (keyBoneLookup[BONE_JAW] > -1)
					bones[keyBoneLookup[BONE_JAW]].calcMatrix(bones, animManager->GetMouthID(), animManager->GetMouthFrame());
		} else {
			// Animate the head and jaw
			if (keyBoneLookup[BONE_HEAD] > -1)
					bones[keyBoneLookup[BONE_HEAD]].calcMatrix(bones, a, t);
			if (keyBoneLookup[BONE_JAW] > -1)
					bones[keyBoneLookup[BONE_JAW]].calcMatrix(bones, a, t);
		}

		// still not sure what 18-26 bone lookups are but I think its more for things like wrist, etc which are not as visually obvious.
		for (size_t i=BONE_BTH; i<BONE_MAX; i++) {
			if (keyBoneLookup[i] > -1)
				bones[keyBoneLookup[i]].calcMatrix(bones, a, t);
		}
		// =====

		if (charModelDetails.closeRHand) {
			a = closeFistID;
			t = anims[closeFistID].timeStart+1;
		} else {
			a = anim;
			t = time;
		}

		for (size_t i=0; i<5; i++) {
			if (keyBoneLookup[BONE_RFINGER1 + i] > -1) 
				bones[keyBoneLookup[BONE_RFINGER1 + i]].calcMatrix(bones, a, t);
		}

		if (charModelDetails.closeLHand) {
			a = closeFistID;
			t = anims[closeFistID].timeStart+1;
		} else {
			a = anim;
			t = time;
		}

		for (size_t i=0; i<5; i++) {
			if (keyBoneLookup[BONE_LFINGER1 + i] > -1)
				bones[keyBoneLookup[BONE_LFINGER1 + i]].calcMatrix(bones, a, t);
		}
	} else {
		for (ssize_t i=0; i<keyBoneLookup[BONE_ROOT]; i++) {
			bones[i].calcMatrix(bones, anim, time);
		}

		// The following line fixes 'mounts' in that the character doesn't get rotated, but it also screws up the rotation for the entire model :(
		//bones[18].calcMatrix(bones, anim, time, false);

		// Animate key skeletal bones except the fingers which we do later.
		// -----
		size_t a, t;

		// if we have a "secondary animation" selected,  animate upper body using that.
		if (animManager->GetSecondaryID() > -1) {
			a = animManager->GetSecondaryID();
			t = animManager->GetSecondaryFrame();
		} else {
			a = anim;
			t = time;
		}

		for (size_t i=0; i<animManager->GetSecondaryCount(); i++) { // only goto 5, otherwise it affects the hip/waist rotation for the lower-body.
			if (keyBoneLookup[i] > -1)
				bones[keyBoneLookup[i]].calcMatrix(bones, a, t);
		}

		if (animManager->GetMouthID() > -1) {
			// Animate the head and jaw
			if (keyBoneLookup[BONE_HEAD] > -1)
					bones[keyBoneLookup[BONE_HEAD]].calcMatrix(bones, animManager->GetMouthID(), animManager->GetMouthFrame());
			if (keyBoneLookup[BONE_JAW] > -1)
					bones[keyBoneLookup[BONE_JAW]].calcMatrix(bones, animManager->GetMouthID(), animManager->GetMouthFrame());
		} else {
			// Animate the head and jaw
			if (keyBoneLookup[BONE_HEAD] > -1)
					bones[keyBoneLookup[BONE_HEAD]].calcMatrix(bones, a, t);
			if (keyBoneLookup[BONE_JAW] > -1)
					bones[keyBoneLookup[BONE_JAW]].calcMatrix(bones, a, t);
		}

		// still not sure what 18-26 bone lookups are but I think its more for things like wrist, etc which are not as visually obvious.
		for (size_t i=BONE_ROOT; i<BONE_MAX; i++) {
			if (keyBoneLookup[i] > -1)
				bones[keyBoneLookup[i]].calcMatrix(bones, a, t);
		}
	}

	// Animate everything thats left with the 'default' animation
	for (size_t i=0; i<header.nBones; i++) {
		bones[i].calcMatrix(bones, anim, time);
	}		
}

void M2Loader::animating(ssize_t anim)
{
	size_t t=0;
	
	ModelAnimation &a = anims[anim];
	int tmax = (a.timeEnd-a.timeStart);
	if (tmax==0) 
		tmax = 1;

	if (isWMO == true) {
		t = globalTime;
		t %= tmax;
		t += a.timeStart;
	} else		
	{
		if (animManager->IsLoop())	// Loop
			t = animManager->GetFrame();
		else  
			t = Min(a.timeEnd-1, animManager->GetFrame());	// 애니메이션 튀는 거 방지.	
			
	}
	
	this->animtime = t;
	this->anim = anim;

	// 미리 계산 필요..
	if (animBones) // && (!animManager->IsPaused() || !animManager->IsParticlePaused()))
		calcBones(anim, t);

	for (size_t i=0; i<header.nLights; i++) {
		if (lights[i].parent>=0) {
			lights[i].tpos = bones[lights[i].parent].mat * lights[i].pos;
			lights[i].tdir = bones[lights[i].parent].mrot * lights[i].dir;
		}
	}

	for (size_t i=0; i<header.nParticleEmitters; i++) {
		// random time distribution for teh win ..?
		//int pt = a.timeStart + (t + (int)(tmax*particleSystems[i].tofs)) % tmax;
		particleSystems[i].setup(anim, t);
	}

	for (size_t i=0; i<header.nRibbonEmitters; i++) {
		ribbons[i].setup(anim, t);
	}

	if (animTextures) {
		for (size_t i=0; i<header.nTexAnims; i++) {
			texAnims[i].calc(anim, t);
		}
	}
}

void Bone::calcMatrix( Bone* allbones, int anim, int time, bool rotate/*=true*/ )
{
	if (calc)
		return;

	noMat4 m;
	noQuat q;

	bool tr = rot.uses(anim) || scale.uses(anim) || trans.uses(anim) || billboard;
	if (tr) {
		//m.translation(pivot);
		m.Translation(pivot);		

		if (trans.uses(anim)) {
			noVec3 tr = trans.getValue(anim, time);
			noMat4 transMat;
			transMat.Translation(tr);
			m *= transMat;
		}

		if (rot.uses(anim) && rotate) {
			q = rot.getValue(anim, time);
			noMat4 rotMat;
			rotMat = q.ToMat4();
			m *= rotMat;
		}

		if (scale.uses(anim)) {
			noVec3 sc = scale.getValue(anim, time);
			noMat4 scaleMat;
			scaleMat.Scale(sc);
			m *= scaleMat;
		}

		if (billboard) {			
			float modelview[16];
			//glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
						
			modelview[0] = model->view_.rows[0].z ;
			modelview[4] = model->view_.rows[1].z ;
			modelview[8] = model->view_.rows[2].z ;

			modelview[1] = model->view_.rows[0].y ;
			modelview[5] = model->view_.rows[1].y ;
			modelview[9] = model->view_.rows[2].y ;

			noVec3 vRight = noVec3(modelview[0], modelview[4], modelview[8]);
			noVec3 vUp = noVec3(modelview[1], modelview[5], modelview[9]); // Spherical billboarding
			//Vec3D vUp = Vec3D(0,1,0); // Cylindrical billboarding
			vRight = vRight * -1;
			m.mat[0][2] = vRight.x;
			m.mat[1][2] = vRight.y;
			m.mat[2][2] = vRight.z;
			m.mat[0][1] = vUp.x;
			m.mat[1][1] = vUp.y;
			m.mat[2][1] = vUp.z;
		}

		noMat4 transMat;
		transMat.Translation(pivot*-1.0f);
		m *= transMat;

	} else m.Identity();

	if (parent > -1) {
		allbones[parent].calcMatrix(allbones, anim, time, rotate);
		mat = allbones[parent].mat * m;
	} else mat = m;

	// transform matrix for normal vectors ... ??
	if (rot.uses(anim) && rotate) {
		noMat4 rotMat;
		rotMat = q.ToMat4();
		if (parent>=0)					
			mrot = allbones[parent].mrot * rotMat;
		else
			mrot = rotMat;
	} else mrot.Identity();

	// 잘못된 수정.
	memcpy(&rdmat, &mat.Transpose(), sizeof(float) * 16);
	//mrot.TransposeSelf(); 
	transPivot = mat * pivot;

	calc = true;

}

void Bone::initV3( MPQFile &f, ModelBoneDef &b, uint32 *global, MPQFile *animfiles )
{
	calc = false;

	parent = b.parent;
	pivot = fixCoordSystem(b.pivot);
	billboard = (b.flags & MODELBONE_BILLBOARD) != 0;
	//billboard = false;

	boneDef = b;

	trans.init(b.translation, f, global, animfiles);
	rot.init(b.rotation, f, global, animfiles);
	scale.init(b.scaling, f, global, animfiles);
	trans.fix(fixCoordSystem);
	rot.fix(fixCoordSystemQuat);
	scale.fix(fixCoordSystem2);
}

void Bone::initV2( MPQFile &f, ModelBoneDef &b, uint32 *global )
{

}

void TextureAnim::init( MPQFile &f, ModelTexAnimDef &mta, uint32 *global )
{
	trans.init(mta.trans, f, global);
	rot.init(mta.rot, f, global);
	scale.init(mta.scale, f, global);
}

void TextureAnim::setup( int anim )
{
	//glLoadIdentity();
	//if (trans.uses(anim)) {
	//	glTranslatef(tval.x, tval.y, tval.z);
	//}
	//if (rot.uses(anim)) {
	//	glRotatef(rval.x, 0, 0, 1); // this is wrong, I have no idea what I'm doing here ;)
	//}
	//if (scale.uses(anim)) {
	//	glScalef(sval.x, sval.y, sval.z);
	//}
}

void TextureAnim::calc( int anim, int time )
{
	if (trans.uses(anim)) {
		tval = trans.getValue(anim, time);
	}
	if (rot.uses(anim)) {
		rval = rot.getValue(anim, time);
	}
	if (scale.uses(anim)) {
		sval = scale.getValue(anim, time);
	}	
}

void ModelCamera::init( MPQFile &f, ModelCameraDef &mcd, uint32 *global, wxString modelname )
{
	wxLogMessage(wxT("Using original Camera Model Definitions."));
	ok = true;
	nearclip = mcd.nearclip;
	farclip = mcd.farclip;
	fov = mcd.fov;
	pos = fixCoordSystem(mcd.pos);
	target = fixCoordSystem(mcd.target);
	tPos.init(mcd.transPos, f, global);
	tTarget.init(mcd.transTarget, f, global);
	rot.init(mcd.rot, f, global);
	tPos.fix(fixCoordSystem);
	tTarget.fix(fixCoordSystem);

	noVec3 wopos = noVec3(0,0,0);
	float worot = 0.0f;
	if (modelname.Find(wxT("Cameras\\"))>-1) {
		try {
			wxLogMessage(wxT("Trying Camera DB..."));
			wxString mn = modelname.BeforeLast(wxT('.')) + wxT(".mdx");
			wxLogMessage(wxT("ModelName: %s"), mn.c_str());
			CamCinematicDB::Record r = camcinemadb.getByCamModel(mn.c_str());
			wxLogMessage(wxT("Setting variables.."));
			wopos = fixCoordSystem(noVec3(r.getFloat(camcinemadb.PosX),r.getFloat(camcinemadb.PosY),r.getFloat(camcinemadb.PosZ)));
			worot = r.getFloat(camcinemadb.Rot);
		}
		catch (CamCinematicDB::NotFound) {
			wxLogMessage(wxT("DBFilesClient/CinematicCamera.dbc not found."));
			wopos = noVec3(0,0,0);
			worot = 0.0f;
		}
		wxLogMessage(wxT("WorldPos: %f, %f, %f"), wopos.x, wopos.y, wopos.z);
		wxLogMessage(wxT("WorldRot: %f"), worot);
	}
	WorldOffset = fixCoordSystem(wopos);
	WorldRotation = worot;
}

void ModelCamera::initv10( MPQFile &f, ModelCameraDefV10 &mcd, uint32 *global, wxString modelname )
{
	wxLogMessage(wxT("Using version 10 Camera Model Definitions."));
	ok = true;
	nearclip = mcd.nearclip;
	farclip = mcd.farclip;
	pos = fixCoordSystem(mcd.pos);
	target = fixCoordSystem(mcd.target);
	tPos.init(mcd.transPos, f, global);
	tTarget.init(mcd.transTarget, f, global);
	rot.init(mcd.rot, f, global);
	tPos.fix(fixCoordSystem);
	tTarget.fix(fixCoordSystem);
	fov = 0.95f;

	noVec3 wopos = noVec3(0,0,0);
	float worot = 0.0f;
	if (modelname.Find(wxT("Cameras\\"))>-1) {
		try {
			wxLogMessage(wxT("Trying Camera DB..."));
			wxString mn = modelname.BeforeLast(wxT('.')) + wxT(".mdx");
			wxLogMessage(wxT("ModelName: %s"), mn.c_str());
			CamCinematicDB::Record r = camcinemadb.getByCamModel(mn.c_str());
			wxLogMessage(wxT("Setting variables.."));
			wopos = fixCoordSystem(noVec3(r.getFloat(camcinemadb.PosX),r.getFloat(camcinemadb.PosY),r.getFloat(camcinemadb.PosZ)));
			worot = r.getFloat(camcinemadb.Rot);
		}
		catch (CamCinematicDB::NotFound) {
			wxLogMessage(wxT("DBFilesClient/CinematicCamera.dbc not found."));
			wopos = noVec3(0,0,0);
			worot = 0.0f;
		}
		wxLogMessage(wxT("WorldPos: %f, %f, %f"), wopos.x, wopos.y, wopos.z);
		wxLogMessage(wxT("WorldRot: %f"), worot);
	}
	WorldOffset = fixCoordSystem(wopos);
	WorldRotation = worot;
}

void ModelCamera::setup( size_t time/*=0*/ )
{
	//if (!ok) return;

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluPerspective(fov * 34.5f, (GLfloat)video.xRes/(GLfloat)video.yRes, nearclip, farclip*5);

	//Vec3D p = pos + tPos.getValue(0, time);
	//Vec3D t = target + tTarget.getValue(0, time);

	//Vec3D u(0,1,0);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	//gluLookAt(p.x, p.y, p.z, t.x, t.y, t.z, u.x, u.y, u.z);
}

void ModelLight::init( MPQFile &f, ModelLightDef &mld, uint32 *global )
{
	tpos = pos = fixCoordSystem(mld.pos);
	tdir = dir = noVec3(0,1,0); // no idea
	type = mld.type;
	parent = mld.bone;
	ambColor.init(mld.ambientColor, f, global);
	ambIntensity.init(mld.ambientIntensity, f, global);
	diffColor.init(mld.diffuseColor, f, global);
	diffIntensity.init(mld.diffuseIntensity, f, global);
	AttenStart.init(mld.attenuationStart, f, global);
	AttenEnd.init(mld.attenuationEnd, f, global);
	UseAttenuation.init(mld.useAttenuation, f, global);
}

void ModelLight::setup( size_t time, uint32 l )
{
	noVec4 ambcol(ambColor.getValue(0, time) * ambIntensity.getValue(0, time), 1.0f);
	noVec4 diffcol(diffColor.getValue(0, time) * diffIntensity.getValue(0, time), 1.0f);
	noVec4 p;
	if (type==MODELLIGHT_DIRECTIONAL) {
		// directional
		p = noVec4(tdir, 0.0f);
	} else if (type==MODELLIGHT_POINT) {
		// point
		p = noVec4(tpos, 1.0f);
	} else {
		p = noVec4(tpos, 1.0f);
		wxLogMessage(wxT("Error: Light type %d is unknown."), type);
	}
	//gLog("Light %d (%f,%f,%f) (%f,%f,%f) [%f,%f,%f]\n", l-GL_LIGHT4, ambcol.x, ambcol.y, ambcol.z, diffcol.x, diffcol.y, diffcol.z, p.x, p.y, p.z);
	/*glLightfv(l, GL_POSITION, p);
	glLightfv(l, GL_DIFFUSE, diffcol);
	glLightfv(l, GL_AMBIENT, ambcol);
	glEnable(l);*/
}

void ModelAttachment::init( MPQFile &f, ModelAttachmentDef &mad, uint32 *global )
{
	pos = fixCoordSystem(mad.pos);
	bone = mad.bone;
	id = mad.id;
}

void ModelAttachment::setup()
{
	noMat4 m = model->bones[bone].mat;
	/*m.transpose();
	glMultMatrixf(m);
	glTranslatef(pos.x, pos.y, pos.z);*/
}

void ModelAttachment::setupParticle()
{
	noMat4 m = model->bones[bone].mat;
	/*m.transpose();
	glMultMatrixf(m);
	glTranslatef(pos.x, pos.y, pos.z);*/
}


void M2Loader::update(size_t deltaTime, size_t accumTime) {

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


void M2Loader::calcBones(M2Animation& animation, size_t time)
{	

	ssize_t anim = animation.id;
	// Reset all bones to 'false' which means they haven't been animated yet.
	for (size_t i=0; i<header.nBones; i++) {
		bones[i].calc = false;
	}
	
	noMat4* frame = new noMat4[header.nBones];
	//for (int i = 0; i < header.nBones; ++i)
		//frame[i].Identity();

	// Character specific bone animation calculations.
	if (charModelDetails.isChar) {	

		// Animate the "core" rotations and transformations for the rest of the model to adopt into their transformations
		if (keyBoneLookup[BONE_ROOT] > -1)	{
			for (size_t i=0; i<=keyBoneLookup[BONE_ROOT]; i++) {				
				bones[i].calcMatrix(bones, anim, time);				
				frame[i] = bones[i].mat;
			}
		}

		// Find the close hands animation id
		int closeFistID = 0;
		/*
		for (size_t i=0; i<header.nAnimations; i++) {
			if (anims[i].animID==15) {  // closed fist
				closeFistID = i;
				break;
			}
		}
		*/
		// Alfred 2009.07.23 use animLookups to speedup
		if (header.nAnimationLookup >= ANIMATION_HANDSCLOSED && animLookups[ANIMATION_HANDSCLOSED] > 0) // closed fist
			closeFistID = animLookups[ANIMATION_HANDSCLOSED];

		// Animate key skeletal bones except the fingers which we do later.
		// -----
		size_t a, t;

		// if we have a "secondary animation" selected,  animate upper body using that.
		if (animManager->GetSecondaryID() > -1) {
			a = animManager->GetSecondaryID();
			t = animManager->GetSecondaryFrame();
		} else {
			a = anim;
			t = time;
		}

		for (size_t i=0; i<animManager->GetSecondaryCount(); i++) { // only goto 5, otherwise it affects the hip/waist rotation for the lower-body.
			if (keyBoneLookup[i] > -1) {
				bones[keyBoneLookup[i]].calcMatrix(bones, a, t);
				frame[keyBoneLookup[i]] = bones[keyBoneLookup[i]].mat;
			}
		}

		if (animManager->GetMouthID() > -1) {
			// Animate the head and jaw
			if (keyBoneLookup[BONE_HEAD] > -1) {
					bones[keyBoneLookup[BONE_HEAD]].calcMatrix(bones, animManager->GetMouthID(), animManager->GetMouthFrame());
					frame[keyBoneLookup[BONE_HEAD]] = bones[keyBoneLookup[BONE_HEAD]].mat;
			}
			
			if (keyBoneLookup[BONE_JAW] > -1) {
					bones[keyBoneLookup[BONE_JAW]].calcMatrix(bones, animManager->GetMouthID(), animManager->GetMouthFrame());
					frame[keyBoneLookup[BONE_JAW]] = bones[keyBoneLookup[BONE_JAW]].mat;
			}
		} else {
			// Animate the head and jaw
			if (keyBoneLookup[BONE_HEAD] > -1) {
					bones[keyBoneLookup[BONE_HEAD]].calcMatrix(bones, a, t);
					frame[keyBoneLookup[BONE_HEAD]] = bones[keyBoneLookup[BONE_HEAD]].mat;
			}
			if (keyBoneLookup[BONE_JAW] > -1) {
					bones[keyBoneLookup[BONE_JAW]].calcMatrix(bones, a, t);
					frame[keyBoneLookup[BONE_JAW]] = bones[keyBoneLookup[BONE_JAW]].mat;
			}
		}

		// still not sure what 18-26 bone lookups are but I think its more for things like wrist, etc which are not as visually obvious.
		for (size_t i=BONE_BTH; i<BONE_MAX; i++) {
			if (keyBoneLookup[i] > -1) {
				bones[keyBoneLookup[i]].calcMatrix(bones, a, t);
				frame[keyBoneLookup[i]] = bones[keyBoneLookup[i]].mat;
			}
		}
		// =====

		if (charModelDetails.closeRHand) {
			a = closeFistID;
			t = anims[closeFistID].timeStart+1;
		} else {
			a = anim;
			t = time;
		}

		for (size_t i=0; i<5; i++) {
			if (keyBoneLookup[BONE_RFINGER1 + i] > -1) {
				bones[keyBoneLookup[BONE_RFINGER1 + i]].calcMatrix(bones, a, t);
				frame[keyBoneLookup[BONE_RFINGER1 + i]] = bones[keyBoneLookup[BONE_RFINGER1 + i]].mat;
			}
		}

		if (charModelDetails.closeLHand) {
			a = closeFistID;
			t = anims[closeFistID].timeStart+1;
		} else {
			a = anim;
			t = time;
		}

		for (size_t i=0; i<5; i++) {
			if (keyBoneLookup[BONE_LFINGER1 + i] > -1) {
				bones[keyBoneLookup[BONE_LFINGER1 + i]].calcMatrix(bones, a, t);
				frame[keyBoneLookup[BONE_LFINGER1]] = bones[keyBoneLookup[BONE_LFINGER1]].mat;
			}
		}
	} else {
		for (ssize_t i=0; i<keyBoneLookup[BONE_ROOT]; i++) {
			bones[i].calcMatrix(bones, anim, time);
			frame[i] = bones[i].mat;
		}

		// The following line fixes 'mounts' in that the character doesn't get rotated, but it also screws up the rotation for the entire model :(
		//bones[18].calcMatrix(bones, anim, time, false);

		// Animate key skeletal bones except the fingers which we do later.
		// -----
		size_t a, t;

		// if we have a "secondary animation" selected,  animate upper body using that.
		if (animManager->GetSecondaryID() > -1) {
			a = animManager->GetSecondaryID();
			t = animManager->GetSecondaryFrame();
		} else {
			a = anim;
			t = time;
		}

		for (size_t i=0; i<animManager->GetSecondaryCount(); i++) { // only goto 5, otherwise it affects the hip/waist rotation for the lower-body.
			if (keyBoneLookup[i] > -1) {
				bones[keyBoneLookup[i]].calcMatrix(bones, a, t);
				frame[keyBoneLookup[i]] = bones[keyBoneLookup[i]].mat;
			}

		}

		if (animManager->GetMouthID() > -1) {
			// Animate the head and jaw
			if (keyBoneLookup[BONE_HEAD] > -1) {
					bones[keyBoneLookup[BONE_HEAD]].calcMatrix(bones, animManager->GetMouthID(), animManager->GetMouthFrame());
					frame[keyBoneLookup[BONE_HEAD]] = bones[keyBoneLookup[BONE_HEAD]].mat;
			}
			if (keyBoneLookup[BONE_JAW] > -1) {
					bones[keyBoneLookup[BONE_JAW]].calcMatrix(bones, animManager->GetMouthID(), animManager->GetMouthFrame());
					frame[keyBoneLookup[BONE_JAW]] = bones[keyBoneLookup[BONE_JAW]].mat;
			}
		} else {
			// Animate the head and jaw
			if (keyBoneLookup[BONE_HEAD] > -1) {
					bones[keyBoneLookup[BONE_HEAD]].calcMatrix(bones, a, t);
					frame[keyBoneLookup[BONE_HEAD]] = bones[keyBoneLookup[BONE_HEAD]].mat;
			}
			if (keyBoneLookup[BONE_JAW] > -1) {
					bones[keyBoneLookup[BONE_JAW]].calcMatrix(bones, a, t);
					frame[keyBoneLookup[BONE_JAW]] = bones[keyBoneLookup[BONE_JAW]].mat;
			}
		}

		// still not sure what 18-26 bone lookups are but I think its more for things like wrist, etc which are not as visually obvious.
		for (size_t i=BONE_ROOT; i<BONE_MAX; i++) {
			if (keyBoneLookup[i] > -1) {
				bones[keyBoneLookup[i]].calcMatrix(bones, a, t);
				frame[keyBoneLookup[i]] = bones[keyBoneLookup[i]].mat;
			}
		}
	}

	// Animate everything thats left with the 'default' animation
	for (size_t i=0; i<header.nBones; i++) { 
		//if (!bones[i].calc) {
			bones[i].calcMatrix(bones, anim, time);
			frame[i] = bones[i].mat;
		//}
	}

#if 0
	if (time == 48)
	{
		ModelVertex *ov = vertDatas;
		for (size_t i=0; i<header.nVertices; ++i,++ov) { //,k=0
			noVec3 v(0,0,0), n(0,0,0);

			
			
				for (size_t b=0; b<4; b++) {
					if (ov->weights[b]>0) {
						noVec3 tv = bones[ov->bones[b]].mat * ov->pos;
						noVec3 tn = bones[ov->bones[b]].mrot * ov->normal;
						v += tv * ((float)ov->weights[b] / 255.0f);
						n += tn * ((float)ov->weights[b] / 255.0f);
					}
				}			
				/*char msg[128];
				sprintf(msg, "Sinned vertices : %f %f %f\n", v.x, v.y, v.z);
				OutputDebugStringA(msg);*/
			
		}
	}
#endif

	animation.AddFrame(frame);
}

void M2Loader::DrawParticles()
{


}

void M2Loader::UpdateParticles( float dt )
{
	if (!bShowParticle) 
		return;

	for (size_t i=0; i<header.nParticleEmitters; i++) {
		particleSystems[i].update(dt);
	}
}

M2Loader::~M2Loader()
{
	delete [] globalSequences;
	delete [] textureIds;
	delete [] colors;
	delete [] transparency;
	delete [] indiceToVerts;
	delete [] vertDatas;
	delete [] anims;
	delete [] bones;
	delete [] animLookups;
	delete [] texCoords;
	delete [] texAnims;
	delete [] events;
	delete [] particleSystems;
	delete [] ribbons;
	delete [] lights;
	delete [] vertices;
	delete [] normals;
	delete [] bounds;
	delete [] boundIndices;
	delete [] indices;
	delete [] showGeosets;
}



void M2AnimationEvent::init( MPQFile &f, ModelEventDef &med, uint32 *global )
{
	def = med;
}


std::string ModelPath( const char* name )
{
	std::string path("model\\");
	path.append(name);
	return path;
}

std::string TexPath( const char* name )
{
	std::string path("textures\\");
	path.append(name);
	return path;
}

std::string ItemTexPath( const char* name )
{
	std::string path("itemtexture\\");
	path.append(name);
	return path;
}