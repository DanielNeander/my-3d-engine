#include "DXUT.h"

#include <cassert>
#include <algorithm>

#include <Math/Vector.h>
#include "M2Loader.h"
#include <Renderer/noRenderSystemDX9.h>
#include <Renderer/noD3DResources.h>


noVec3 fixCoordSystem(noVec3 v)
{
	return noVec3(v.x, v.z, -v.y);
}

noVec3 fixCoordSystem2(noVec3 v)
{
	return noVec3(v.x, v.z, v.y);
}

void M2Loader::Load(const char* fname)
{
	rad = 1.0f;
	MPQFile f(fname);
	
	
	bool ok = true;

	memcpy(&header, f.getBuffer(), sizeof(ModelHeader));
	bool bAnim = isAnimated(f);

	// Error check
	if (header.id[0] != 'M' && header.id[1] != 'D' && header.id[2] != '2' && header.id[3] != '0') {
		LOG_WARN << _T("Error:\t\tInvalid model!  May be corrupted.");
		ok = false;
		f.close();
		return;
	}

#ifdef WotLK
	modelname = fname;

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

	uint32 *globalSequences;
	if (header.nGlobalSequences) {
		globalSequences = new uint32[header.nGlobalSequences];
		memcpy(globalSequences, (f.getBuffer() + header.ofsGlobalSequences), header.nGlobalSequences * sizeof(uint32));
	}

	if (bAnim) initAnimated(f);
	else initStatic(f);

	f.close();
}

void M2Loader::initCommon(MPQFile &f) {

	LoadVertices( f );
	// textures
	ModelTextureDef *texdef = (ModelTextureDef*)(f.getBuffer() + header.ofsTextures);
	if (header.nTextures) {
		textureIds = new uint32[header.nTextures];	// 게임 상의 데이터
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
				//textureIds[i] = texturemanager.add(texname);
				noTexture2D* pTex = new noTexture2D();
	//			pTex->SetTextureFile(&texname[0]);
				TextureList.push_back(texname);
				LOG_INFO << "Info: Added %s to the TextureList." << texname;
			} else {
				// special texture - only on characters and such...
				textureIds[i] = 0;
				//while (texdef[i].type < TEXTURE_MAX && specialTextures[texdef[i].type]!=-1) texdef[i].type++;
				//if (texdef[i].type < TEXTURE_MAX)specialTextures[texdef[i].type] = (int)i;
				specialTextures[i] = texdef[i].type;

				if (modelType == MT_CHAR || modelType == MT_NPC){
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
				}else if (modelType == MT_NORMAL){
					if (texdef[i].type == TEXTURE_CAPE){
						// This is a replacable Texture!

						//g_charControl
						strncpy(texname,(const char*)"Cape.blp",8);
						TextureList.push_back(texname);
						LOG_INFO << "Info: Added %s to the TextureList via specialTextures."<< texname;
					}
				}

				if (texdef[i].type < TEXTURE_MAX)
					useReplaceTextures[texdef[i].type] = true;

				if (texdef[i].type == TEXTURE_ARMORREFLECT) {
					// a fix for weapons with type-3 textures.
					//replaceTextures[texdef[i].type] = texturemanager.add("Item\\ObjectComponents\\Weapon\\ArmorReflect4.BLP");
				}
			}
		}
	}
	
	// replacable textures - it seems to be better to get this info from the texture types
	if (header.nTexReplace) {
		size_t m = header.nTexReplace;
		if (m>16) m = 16;
		int16 *texrep = (int16*)(f.getBuffer() + header.ofsTexReplace);
		for (size_t i=0; i<m; i++) specialTextures[i] = texrep[i];
	}
	

		// attachments
	// debug code here
	if (header.nAttachments) {
		ModelAttachmentDef *attachments = (ModelAttachmentDef*)(f.getBuffer() + header.ofsAttachments);
		for (size_t i=0; i<header.nAttachments; i++) {
			/*ModelAttachment att;
			att.model = this;
			att.init(f, attachments[i], globalSequences);
			atts.push_back(att);*/
		}
	}

	if (header.nAttachLookup) {
		int16 *p = (int16*)(f.getBuffer() + header.ofsAttachLookup);
		/*if (header.nAttachLookup > ATT_MAX)
			LOG_INFO << _T("Critical Error: Model AttachLookup %d over %d"), header.nAttachLookup, ATT_MAX);
		for (size_t i=0; i<header.nAttachLookup; i++) {
			if (i>ATT_MAX-1)
				break;
			attLookup[i] = p[i];
		}*/
	}


	// init colors
	if (header.nColors) {
		ModelColorDef *colorDefs = (ModelColorDef*)(f.getBuffer() + header.ofsColors);
		colors = new ModelColor[header.nColors];
		for (size_t i=0; i<header.nColors; i++) 
			colors[i].init(f, colorDefs[i], globalSequences);
	}

	// init transparency
	int16 *transLookup = (int16*)(f.getBuffer() + header.ofsTransparencyLookup);
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

		// Old method - use this to try to determine a bug.
		// just use the first LOD/view

		// indices - allocate space, too
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
			
			pass.order = tex[j].shading; //pass.order = 0;
			
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
			pass.cull = (rf.flags & RENDERFLAGS_TWOSIDED)==0 && rf.blend==0;

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
			
			if (animTextures) {
				// tex[j].flags: Usually 16 for static textures, and 0 for animated textures.	
				if (tex[j].flags & TEXTUREUNIT_STATIC) {
					pass.texanim = -1; // no texture animation
				} else {
					pass.texanim = texanimlookup[tex[j].texanimid];
				}
			} else {
				pass.texanim = -1; // no texture animation
			}

			passes.push_back(pass);
		}

#ifdef WotLK
		g.close();
#endif
		// transparent parts come later
		std::sort(passes.begin(), passes.end());
	}

	// zomg done
}



void M2Loader::initAnimated( MPQFile &f ) {
	if (vertDatas) {
		delete [] vertDatas;
		vertDatas = NULL;
	}

	vertDatas = new ModelVertex[header.nVertices];
	memcpy(vertDatas, f.getBuffer() + header.ofsVertices, header.nVertices * sizeof(ModelVertex));

	initCommon(f);

}

bool M2Loader::isAnimated( MPQFile& f )
{
	animGeom = false;

	if (header.nAnimations > 0) {
		anims = new ModelAnimation[header.nAnimations];

		#ifndef WotLK
		memcpy(anims, f.getBuffer() + header.ofsAnimations, header.nAnimations * sizeof(ModelAnimation));
		#else
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
		#endif
	}

	if (animBones) {
		// init bones...
		bones = new Bone[header.nBones];
		ModelBoneDef *mb = (ModelBoneDef*)(f.getBuffer() + header.ofsBones);
		for (size_t i=0; i<header.nBones; i++) {
			//if (i==0) mb[i].rotation.ofsRanges = 1.0f;
#ifdef WotLK
		//	bones[i].model = this;
			//bones[i].init(f, mb[i], globalSequences, animfiles);
#else
			//bones[i].init(f, mb[i], globalSequences);
#endif
		}

		// Block keyBoneLookup is a lookup table for Key Skeletal Bones, hands, arms, legs, etc.
		if (header.nKeyBoneLookup < BONE_MAX) {
			memcpy(keyBoneLookup, f.getBuffer() + header.ofsKeyBoneLookup, sizeof(int16)*header.nKeyBoneLookup);
		} else {
			memcpy(keyBoneLookup, f.getBuffer() + header.ofsKeyBoneLookup, sizeof(int16)*BONE_MAX);
			LOG_ERR << _T("Error: keyBone number ") <<  header.nKeyBoneLookup << _T(" over ") <<  BONE_MAX;
		}
	}

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

	// Index at ofsAnimations which represents the animation in AnimationData.dbc. -1 if none.
	if (header.nAnimationLookup > 0) {
		animLookups = new int16[header.nAnimationLookup];
		memcpy(animLookups, f.getBuffer() + header.ofsAnimationLookup, sizeof(int16)*header.nAnimationLookup);
	}

	const size_t size = (header.nVertices * sizeof(float));
	vbufsize = (3 * size); // we multiple by 3 for the x, y, z positions of the vertex

	texCoords = new noVec2[header.nVertices];
	for (size_t i=0; i<header.nVertices; i++) 
		texCoords[i] = vertDatas[i].texcoords;


	if (animTextures) {
		texAnims = new TextureAnim[header.nTexAnims];
		ModelTexAnimDef *ta = (ModelTexAnimDef*)(f.getBuffer() + header.ofsTexAnims);
		//for (size_t i=0; i<header.nTexAnims; i++)
			//texAnims[i].init(f, ta[i], globalSequences);
	}

	return true;
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

M2Loader::M2Loader()
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
void ModelTransparency::init( MPQFile &f, ModelTransDef &mtd, uint32 *global )
{

}

void ModelColor::init( MPQFile &f, ModelColorDef &mcd, uint32 *global )
{

}