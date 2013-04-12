#include "stdafx.h"
#include "database.h"
#include "M2ModelEdit.h"
#include "M2Loader.h"

#include <EngineCore/Util/GameLog.h>
#include <wx/txtstrm.h>
#include "CxImage/ximage.h"


int slotOrder[] = {	
	CS_SHIRT,
	CS_HEAD,
	CS_NECK,
	CS_SHOULDER,
	CS_PANTS,
	CS_BOOTS,
	CS_CHEST,
	CS_TABARD,
	CS_BELT,
	CS_BRACERS,
	CS_GLOVES,
	CS_HAND_RIGHT,
	CS_HAND_LEFT,
	CS_CAPE,
	CS_QUIVER
};

int slotOrderWithRobe[] = {
	CS_SHIRT,
	CS_HEAD,
	CS_NECK,
	CS_SHOULDER,
	CS_BOOTS,
	CS_PANTS,
	CS_BRACERS,
	CS_CHEST,
	CS_GLOVES,
	CS_TABARD,
	CS_BELT,
	CS_HAND_RIGHT,
	CS_HAND_LEFT,
	CS_CAPE,
	CS_QUIVER
};

wxString regionPaths[NUM_REGIONS] =
{
	wxEmptyString,
	wxT("Item\\TextureComponents\\ArmUpperTexture\\"),
	wxT("Item\\TextureComponents\\ArmLowerTexture\\"),
	wxT("Item\\TextureComponents\\HandTexture\\"),
	wxEmptyString,
	wxEmptyString,
	wxT("Item\\TextureComponents\\TorsoUpperTexture\\"),
	wxT("Item\\TextureComponents\\TorsoLowerTexture\\"),
	wxT("Item\\TextureComponents\\LegUpperTexture\\"),
	wxT("Item\\TextureComponents\\LegLowerTexture\\"),
	wxT("Item\\TextureComponents\\FootTexture\\")
};


static wxString makeSkinTexture(wxString texfn, wxString skin, const char* ext);


void M2ModelEdit::UpdateCharModel( M2Loader* m2 )
{
	
	hairTex = 0;
	furTex = 0;
	gobTex = 0;
	capeTex = 0;


	// Reset geosets
	for (size_t i=0; i<NUM_GEOSETS; i++) 
		cd.geosets[i] = 1;
	cd.geosets[CG_GEOSET100] = cd.geosets[CG_GEOSET200] = cd.geosets[CG_GEOSET300] = 0;

	// show ears, if toggled
	if (cd.showEars) 
		cd.geosets[CG_EARS] = 2;

	M2ActorTexture texture(cd.race);


	CharSectionsDB::Record rec = chardb.getRecord(0);

	// base character layer/texture
	try {
		rec = chardb.getByParams(cd.race, cd.gender, CharSectionsDB::SkinType, 0, cd.skinColor, cd.useNPC);
		wxString baseTexName = rec.getString(CharSectionsDB::Tex1);
		texture.AddLayer(baseTexName, CR_BASE, 0);
		//UpdateTextureList(baseTexName, TEXTURE_BODY);

		// Tauren fur
		wxString furTexName = rec.getString(CharSectionsDB::Tex2);
		if (!furTexName.IsEmpty())
		{
			//			furTex = texturemanager.add(furTexName);
			UpdateTextureList(m2, furTexName, TEXTURE_FUR);
		}

	} catch (CharSectionsDB::NotFound) {
		wxLogMessage(wxT("Assertion base character Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
	}

	if (cd.race == RACE_GOBLIN && cd.gender == GENDER_MALE && gobTex == 0 && gameVersion < VERSION_CATACLYSM) {
		//gobTex = texturemanager.add(wxT("Creature\\Goblin\\Goblin.blp"));	
	}

	// Hair related boolean flags
	bool bald = false;
	bool showHair = cd.showHair;
	bool showFacialHair = cd.showFacialHair;

	if (cd.race != RACE_GOBLIN || gameVersion >= VERSION_CATACLYSM) { // Goblin chars base texture already contains all this stuff.

		// Display underwear on the model?
		if (cd.showUnderwear) {
			try {
				rec = chardb.getByParams(cd.race, cd.gender, CharSectionsDB::UnderwearType, 0, cd.skinColor, cd.useNPC);
				texture.AddLayer(rec.getString(CharSectionsDB::Tex1), CR_PELVIS_UPPER, 1); // pants
				texture.AddLayer(rec.getString(CharSectionsDB::Tex2), CR_TORSO_UPPER, 1); // top
			} catch (CharSectionsDB::NotFound) {
				wxLogMessage(wxT("DBC underwear Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
			}
		}

		// face
		try {
			rec = chardb.getByParams(cd.race, cd.gender, CharSectionsDB::FaceType, cd.faceType, cd.skinColor, cd.useNPC);
			texture.AddLayer(rec.getString(CharSectionsDB::Tex1), CR_FACE_LOWER, 1);
			texture.AddLayer(rec.getString(CharSectionsDB::Tex2), CR_FACE_UPPER, 1);
		} catch (CharSectionsDB::NotFound) {
			wxLogMessage(wxT("DBC face Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
		}

		// facial feature geosets
		try {
			CharFacialHairDB::Record frec = facialhairdb.getByParams((unsigned int)cd.race, (unsigned int)cd.gender, (unsigned int)cd.facialHair);
			if (gameVersion >= VERSION_CATACLYSM) {
				cd.geosets[CG_GEOSET100] = frec.getUInt(CharFacialHairDB::Geoset100V400);
				cd.geosets[CG_GEOSET200] = frec.getUInt(CharFacialHairDB::Geoset200V400);
				cd.geosets[CG_GEOSET300] = frec.getUInt(CharFacialHairDB::Geoset300V400);
			} else {
				cd.geosets[CG_GEOSET100] = frec.getUInt(CharFacialHairDB::Geoset100);
				cd.geosets[CG_GEOSET200] = frec.getUInt(CharFacialHairDB::Geoset200);
				cd.geosets[CG_GEOSET300] = frec.getUInt(CharFacialHairDB::Geoset300);
			}
		} catch (CharFacialHairDB::NotFound) {
			wxLogMessage(wxT("DBC facial feature geosets Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
		}

		// facial feature
		try {
			rec = chardb.getByParams(cd.race, cd.gender, CharSectionsDB::FacialHairType, cd.facialHair, cd.facialColor, 0);
			texture.AddLayer(rec.getString(CharSectionsDB::Tex1), CR_FACE_LOWER, 2);
			texture.AddLayer(rec.getString(CharSectionsDB::Tex2), CR_FACE_UPPER, 2);
		} catch (CharSectionsDB::NotFound) {
			wxLogMessage(wxT("DBC facial feature Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
		}
	}

	// select hairstyle geoset(s)
	for (CharHairGeosetsDB::Iterator it = hairdb.begin(); it != hairdb.end(); ++it) {
		if (it->getUInt(CharHairGeosetsDB::Race)==cd.race &&
			it->getUInt(CharHairGeosetsDB::Gender)==cd.gender &&
			it->getUInt(CharHairGeosetsDB::Section)==cd.hairStyle)
		{
			unsigned int geosetId = it->getUInt(CharHairGeosetsDB::Geoset);
			bald = it->getUInt(CharHairGeosetsDB::Bald) != 0;

			for (size_t j=0; j<m2->geosets.size(); j++) {
				if (m2->geosets[j].id == geosetId)
					m2->showGeosets[j] = showHair;
				else if (m2->geosets[j].id >= 1 && m2->geosets[j].id <= cd.maxHairStyle)
					m2->showGeosets[j] = false;
			}
		}
	}

	// hair
	try {
		rec = chardb.getByParams(cd.race, cd.gender, CharSectionsDB::HairType, cd.hairStyle, cd.hairColor, 0);
		wxString hairTexfn = rec.getString(CharSectionsDB::Tex1);
		if (!hairTexfn.IsEmpty()) 
		{
			int pos = hairTexfn.find_first_of('.', 0);
			hairTexfn[pos+1] = 't';
			hairTexfn[pos+2] = 'g';
			hairTexfn[pos+3] = 'a';
			hairTex =m2->renderer->addTexture(TexPath(hairTexfn.wx_str()).c_str(), true);// texturemanager.add(hairTexfn);
			UpdateTextureList(m2, hairTexfn, TEXTURE_HAIR);
		}
		else {
			// oops, looks like we're missing a hair texture. Let's try with hair style #0.
			// (only a problem for orcs with no hair but some beard
			try {
				rec = chardb.getByParams(cd.race, cd.gender, CharSectionsDB::HairType, 0, cd.hairColor, 0);
				hairTexfn = rec.getString(CharSectionsDB::Tex1);
				if (!hairTexfn.IsEmpty()) 
				{
					int pos = hairTexfn.find_first_of('.', 0);
					hairTexfn[pos+1] = 't';
					hairTexfn[pos+2] = 'g';
					hairTexfn[pos+3] = 'a';
					hairTex =m2->renderer->addTexture(TexPath(hairTexfn.wx_str()).c_str(), true);// texturemanager.add(hairTexfn);
					UpdateTextureList(m2, hairTexfn, TEXTURE_HAIR);
				}
				else 
					hairTex = 0;
			} catch (CharSectionsDB::NotFound) {
				// oh well, give up.
				hairTex = 0; // or chartex?
			}
		}
		if (!bald) {
			texture.AddLayer(rec.getString(CharSectionsDB::Tex2), CR_FACE_LOWER, 3);
			texture.AddLayer(rec.getString(CharSectionsDB::Tex3), CR_FACE_UPPER, 3);
		}

	} catch (CharSectionsDB::NotFound) {
		wxLogMessage(wxT("DBC hair Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
		hairTex = 0;
	}

	// If they have no hair, toggle the 'bald' flag.
	if (!showHair)
		bald = true;

	// Hide facial hair if it isn't toggled and they don't have tusks, horns, etc.
	if (!showFacialHair) {		
		try {
			CharRacesDB::Record race = racedb.getById(cd.race);
			wxString tmp;
			if (gameVersion >= VERSION_CATACLYSM)
				tmp = race.getString(CharRacesDB::GeoType1V400);
			else
				tmp = race.getString(CharRacesDB::GeoType1);
			if (tmp.Lower() == wxT("normal")) {
				cd.geosets[CG_GEOSET100] = 1;
				cd.geosets[CG_GEOSET200] = 1;
				cd.geosets[CG_GEOSET300] = 1;
			}
		} catch (CharRacesDB::NotFound) {
			wxLogMessage(wxT("Assertion FacialHair Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
		}
	}

	// check if we have a robe on
	bool hadRobe = false;
	if (cd.equipment[CS_CHEST] != 0) {
		try {
			const ItemRecord &item = items.getById(cd.equipment[CS_CHEST]);
			if (item.type==IT_ROBE || item.type==IT_CHEST) {
				ItemDisplayDB::Record r = itemdisplaydb.getById(item.model);
				if (r.getUInt(ItemDisplayDB::RobeGeosetFlags)==1) 
					hadRobe = true;
			}
		} catch (ItemDisplayDB::NotFound) {
			wxLogMessage(wxT("Assertion robe Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
		}
	}

	// check if we have a kilt on, just like our robes
	if (cd.equipment[CS_PANTS] != 0) {
		try {
			const ItemRecord &item = items.getById(cd.equipment[CS_PANTS]);
			if (item.type==IT_PANTS) {
				ItemDisplayDB::Record r = itemdisplaydb.getById(item.model);
				if (r.getUInt(ItemDisplayDB::RobeGeosetFlags)==1) 
					hadRobe = true;
			}
		} catch (ItemDisplayDB::NotFound) {
			wxLogMessage(wxT("Assertion kilt Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
		}
	}

	// Default
	slotOrderWithRobe[7] = CS_CHEST;
	slotOrderWithRobe[8] = CS_GLOVES;

	// check the order of robe/gloves
	if (cd.equipment[CS_CHEST] && cd.equipment[CS_GLOVES]) {
		try {
			//const ItemRecord &item = items.getById(cd.equipment[CS_CHEST]);
			//if (item.type==IT_ROBE) {
			//	ItemDisplayDB::Record r = itemdisplaydb.getById(item.model);
			//if (r.getUInt(ItemDisplayDB::GeosetA)>0) {
			const ItemRecord &item2 = items.getById(cd.equipment[CS_GLOVES]);
			ItemDisplayDB::Record r2 = itemdisplaydb.getById(item2.model);
			if (r2.getUInt(ItemDisplayDB::GloveGeosetFlags)==0) {
				slotOrderWithRobe[7] = CS_GLOVES;
				slotOrderWithRobe[8] = CS_CHEST;
			}
			//}
			//}
		} catch (ItemDisplayDB::NotFound) {
			wxLogMessage(wxT("Assertion robe/gloves Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
		}
	}

	// dressup
	for (ssize_t i=0; i<NUM_CHAR_SLOTS; i++) {
		int sn = hadRobe ? slotOrderWithRobe[i] : slotOrder[i];
		if (cd.equipment[sn] != 0) 
			AddEquipment(sn, cd.equipment[sn], 10+i, texture);
			
	}

	// reset geosets
	for (size_t j=0; j<m2->geosets.size(); j++) {
		int id = m2->geosets[j].id;

		// hide top-of-head if we have hair.
		if (id == 1)
			m2->showGeosets[j] = bald;

		for (size_t i=1; i<NUM_GEOSETS; i++) {
			int a = (int)i*100, b = ((int)i+1) * 100;
			if (id>a && id<b) 
				m2->showGeosets[j] = (id == (a + cd.geosets[i]));
		}
	}

	if (!cachedCombosedTex.empty())
		charTex = m2->renderer->addTexture(cachedCombosedTex.c_str(), true);
	else 
		texture.Combine(m2->renderer, charTex);

	// set replacable textures
	m2->replaceTextures[TEXTURE_BODY] = charTex;
	m2->replaceTextures[TEXTURE_CAPE] = capeTex;
	m2->replaceTextures[TEXTURE_HAIR] = hairTex;
	m2->replaceTextures[TEXTURE_FUR] = furTex;
	m2->replaceTextures[TEXTURE_GAMEOBJECT1] = gobTex;	

	
}

bool M2ModelEdit::FillSkinSelector(M2Loader* m, TextureSet &skins)
{
	if (skins.size() > 0) {
		int num = 0;
		// fill our skin selector
		for (TextureSet::iterator it = skins.begin(); it != skins.end(); ++it) {
			wxString texname = it->tex[0];			
			wxString name(m->name.c_str());
			texname = name.BeforeLast(MPQ_SLASH) + MPQ_SLASH + texname + wxT(".tga");
			wxLogMessage(wxT("Info: Added %s to the TextureList[%i] via FillSkinSelector."), texname.c_str(), m->TextureList.size());
			m->TextureList.push_back(texname.wx_str());			
		}

		bool ret = (skins.size() > 0);
		//skins.clear();
		return ret;
	} else 
		return false;
}

wxString makeSkinTexture( wxString texfn, wxString skin, const char* ext )
{
	wxString res = texfn;
	size_t i = res.find_last_of("\\");
	res = res.substr(0,i+1);
	res.append(skin);
	res.append(ext);
	return res;
}


void M2ModelEdit::UpdateCreatureModel(M2Loader* m)
{
	wxString fn = m->name;
	TextureSet skins;

	try {
		CreatureModelDB::Record rec = modeldb.getByFilename(fn);
		// for character models, don't use skins
		if (rec.getUInt(CreatureModelDB::Type) != 4) {
			//TextureSet skins;
			unsigned int modelid = rec.getUInt(CreatureModelDB::ModelID);

			wxLogMessage(wxT("Found model in CreatureModelDB, id: %u"), modelid);

			for (CreatureSkinDB::Iterator it = skindb.begin();  it!=skindb.end();  ++it) {
				if (it->getUInt(CreatureSkinDB::ModelID) == modelid) {
					TextureGroup grp;
					int count = 0;
					for (size_t i=0; i<TextureGroup::num; i++) {
						wxString skin(it->getString(CreatureSkinDB::Skin1 + i));
						if (skin != wxEmptyString) {
							grp.tex[i] = skin;
							count++;
						}
					}
					grp.base = TEXTURE_GAMEOBJECT1;
					grp.count = count;
					if (grp.tex[0].length() > 0) 
						skins.insert(grp);
				}
			}
		}
	} catch (CreatureModelDB::NotFound) {
		wxLogMessage(wxT("CreatureModelDB not found !!!"));
	}

	int count = (int)skins.size();

	bool ret = false;

	if (!skins.empty()) {
		ret = FillSkinSelector(m, skins);
		if (ret) {

			// test 
			TextureSet::iterator it= skins.begin();
			if (skins.size() > 1)
				++it;
			TextureGroup grp = (*it);
			for (size_t i=0; i<grp.count; i++) {
				size_t base = grp.base + i;
				if (m->useReplaceTextures[base]) { 
					wxString skin = makeSkinTexture(m->name, grp.tex[i], ".tga");
					// refresh TextureList for further use
					for (ssize_t j=0; j<TEXTURE_MAX; j++) {
						if (base == m->specialTextures[j]) {
							m->TextureList[j] = skin;
							break;
						}
					}					
					m->replaceTextures[grp.base+i] = renderer->addTexture(TexPath(skin.wx_str()).c_str(), true);
				}
			}
		}
	}
}

void M2ModelEdit::UpdateEyeGlow(M2Loader* m2)
{
	// Eye Glows
	for(size_t i=0; i<m2->renderPasses.size(); i++) {
		ModelRenderPass &p = m2->renderPasses[i];
		wxString texName = wxString(m2->TextureList[p.tex].c_str()).AfterLast('\\').Lower();

		if (texName.Find(wxT("eyeglow")) == wxNOT_FOUND)
			continue;

		// Regular Eye Glow
		if ((texName.Find(wxT("eyeglow")) != wxNOT_FOUND)&&(texName.Find(wxT("deathknight")) == wxNOT_FOUND)){
			if (cd.eyeGlowType == EGT_NONE){					// If No EyeGlow
				m2->showGeosets[p.geoset] = false;
			}else if (cd.eyeGlowType == EGT_DEATHKNIGHT){		// If DK EyeGlow
				m2->showGeosets[p.geoset] = false;
			}else{												// Default EyeGlow, AKA cd.eyeGlowType == EGT_DEFAULT
				m2->showGeosets[p.geoset] = true;
			}
		}
		// DeathKnight Eye Glow
		if (texName.Find(wxT("deathknight")) != wxNOT_FOUND){
			if (cd.eyeGlowType == EGT_NONE){					// If No EyeGlow
				m2->showGeosets[p.geoset] = false;
			}else if (cd.eyeGlowType == EGT_DEATHKNIGHT){		// If DK EyeGlow
				m2->showGeosets[p.geoset] = true;
			}else{												// Default EyeGlow, AKA cd.eyeGlowType == EGT_DEFAULT
				m2->showGeosets[p.geoset] = false;
			}
		}
	}

}

M2ModelEdit::M2ModelEdit()
{


}


M2ModelEdit::M2ModelEdit( M2Loader* m2 )
	:m2_(m2)
{

}

void M2ModelEdit::Init(Renderer* renderer_)
{
	renderer = renderer_;

	
}

void M2ActorTexture::Combine( Renderer* renderer, TextureID& texID )
{
	// scale for pandaren race.
	size_t x_scale = race == 24 ? 2 : 1;
	size_t y_scale = 1;

	for (size_t i = 0; i < components.size(); i++)
	{
		size_t p1 = components[i].name.find_first_of(".", 0);
		components[i].name[p1+1] = 't';
		components[i].name[p1+2] = 'g';
		components[i].name[p1+3] = 'a';
	}

	// if we only have one texture then don't bother with compositing
	if (components.size()==1) {
		//Texture temp(components[0].name);
		//texturemanager.LoadBLP(texID, &temp);
		Image tex;
		tex.loadTGA(components[0].name.c_str());
		texID = renderer->addTexture(tex);
		return;
	}

	std::sort(components.begin(), components.end());

	unsigned char *destbuf, *tempbuf;
	destbuf = (unsigned char*)malloc(REGION_PX_WIDTH*x_scale*REGION_PX_HEIGHT*y_scale*4);
	memset(destbuf, 0, REGION_PX_WIDTH*x_scale*REGION_PX_HEIGHT*y_scale*4);

	int i = 0;
	for (std::vector<M2ActorTextureFragment>::iterator it = components.begin(); it != components.end(); ++it) {
		M2ActorTextureFragment &comp = *it;
		// pandaren with different regions.
		const CharRegionCoords &coords = race == 24 ? pandaren_regions[comp.region] : regions[comp.region];
		//TextureID temptex = texturemanager.add(comp.name);
		//Texture &tex = *((Texture*)texturemanager.items[temptex]);
		CxImage tex(comp.name.c_str(), CXIMAGE_FORMAT_TGA);		

		

		// Alfred 2009.07.03, tex width or height can't be zero
		if (tex.GetWidth() == 0 || tex.GetHeight() == 0) {
			//texturemanager.del(temptex);
			LOG_INFO << comp.name.c_str();
			continue;
		}		
		long size = tex.GetWidth() * tex.GetHeight() * 4;
		BYTE *buffer = NULL;
		bool bsuccess = tex.Encode2RGBA(buffer, size, true);	

		//int pos = comp.name.find_last_of("\\");
		//bsuccess = tex.Save(TexPath(comp.name.substr(pos).c_str()).c_str(), CXIMAGE_FORMAT_TGA);
		/*{
			unsigned char *buf;			
			buf = (unsigned char*)malloc(size);
			memset(buf, 0, size);
			memcpy_s(buf, size, buffer, size);
			Image img;
			img.loadFromMemory(buf, FORMAT_RGBA8, tex.GetWidth(), tex.GetHeight(), 1, 1, true);
			texID = renderer->addTexture(img);
			char texname[64];
			int pos = comp.name.find_last_of("\\");
			sprintf(texname, "data/%s",comp.name.substr(pos).c_str(), i);
			img.saveTGA(texname);
		}*/
				
		tempbuf = (unsigned char*)malloc(size);
		if (!tempbuf)
			continue;
		memset(tempbuf, 0, size);

		bool bDelBuff = true;

		if (tex.GetWidth()!=coords.xsize || tex.GetHeight()!=coords.ysize)
		{
			//tex.getPixels(tempbuf, GL_BGRA_EXT);			
			memcpy_s(tempbuf, size, buffer, size);			

			CxImage newImage(0);
			newImage.AlphaCreate();	// Create the alpha layer
			newImage.IncreaseBpp(32);	// set image to 32bit 
			newImage.CreateFromArray(tempbuf, tex.GetWidth(), tex.GetHeight(), 32, (tex.GetWidth()*4), false);
			newImage.Resample(coords.xsize, coords.ysize, 0); // 0: hight quality, 1: normal quality
			free(tempbuf);
			tempbuf = NULL;
			long size = coords.xsize * coords.ysize * 4;
			bool success = newImage.Encode2RGBA(tempbuf, size, false);

			//char texname[64];
			//sprintf(texname, "data/new%d.tga",i);
			//newImage.Save(texname, CXIMAGE_FORMAT_TGA);

			bDelBuff = false;			
			// 메모리 해제 버그 발생..
			//CxImage *newImage = new CxImage(0);
			//if (newImage) {
			//	newImage->AlphaCreate();	// Create the alpha layer
			//	newImage->IncreaseBpp(32);	// set image to 32bit 
			//	newImage->CreateFromArray(tempbuf, tex.GetWidth(), tex.GetHeight(), 32, (tex.GetWidth()*4), false);
			//	newImage->Resample(coords.xsize, coords.ysize, 0); // 0: hight quality, 1: normal quality
			//	wxDELETE(tempbuf);
			//	tempbuf = NULL;
			//	long size = coords.xsize * coords.ysize * 4;
			//	newImage->Encode2RGBA(tempbuf, size, false);
			//	wxDELETE(newImage);
			//} else {
			//	free(tempbuf);
			//	continue;
			//}
		} else
			memcpy_s(tempbuf, size, buffer, size);
		
		
		// blit the texture region over the original
		for (ssize_t y=0, dy=coords.ypos; y<coords.ysize; y++,dy++) {
			for (ssize_t x=0, dx=coords.xpos; x<coords.xsize; x++,dx++) {
				unsigned char *src = tempbuf + y*coords.xsize*4 + x*4;
				unsigned char *dest = destbuf + dy*REGION_PX_WIDTH*x_scale*4 + dx*4;

				// this is slow and ugly but I don't care
				if (src[3] == 0) src[3] = 255;
				float r = src[3] / 255.0f;
				float ir = 1.0f - r;
				// zomg RGBA?
				dest[0] = (unsigned char)(dest[0]*ir + src[0]*r);
				dest[1] = (unsigned char)(dest[1]*ir + src[1]*r);
				dest[2] = (unsigned char)(dest[2]*ir + src[2]*r);				
				dest[3] = 255;
			}
		}

#if 0
		unsigned char *buf;
		int x = REGION_PX_WIDTH*x_scale*REGION_PX_HEIGHT*y_scale*4;
		buf = (unsigned char*)malloc(REGION_PX_WIDTH*x_scale*REGION_PX_HEIGHT*y_scale*4);
		memset(buf, 0, x);
		memcpy_s(buf, x, destbuf, x);
		Image img;
		img.loadFromMemory(buf, FORMAT_RGBA8, REGION_PX_WIDTH*x_scale, REGION_PX_HEIGHT*y_scale, 1, 1, true);
		texID = renderer->addTexture(img);
		char texname[64];
		sprintf(texname, "data/texture%d.tga",i);
		img.saveTGA(texname);
#endif
		if (bDelBuff)
			free(tempbuf);		
		++i;
	}

	// good, upload this to video
	/*glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, REGION_PX_WIDTH*x_scale, REGION_PX_HEIGHT*y_scale, 0, GL_RGBA, GL_UNSIGNED_BYTE, destbuf);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);*/		
	Image img;
	img.loadFromMemory(destbuf, FORMAT_RGBA8, REGION_PX_WIDTH*x_scale, REGION_PX_HEIGHT*y_scale, 1, 1, true);
	texID = renderer->addTexture(img);
	//img.saveTGA("data/texture.tga");
	//free(destbuf);
}

void M2ActorTexture::AddLayer( std::string fn, int region, int layer )
{
	if (fn.size()==0)
		return;

	M2ActorTextureFragment ct;
	ct.name = TexPath(fn.c_str());
	ct.region = region;
	ct.layer = layer;
	components.push_back(ct);
}

void M2ActorTexture::AddLayer( wxString fn, int region, int layer )
{
	if (!fn || fn.length()==0)
		return;

	M2ActorTextureFragment ct;
	ct.name = TexPath(fn.wx_str());
	ct.region = region;
	ct.layer = layer;
	components.push_back(ct);
}

void M2ActorDetails::reset()
{
	skinColor = 0;
	faceType = 0;
	hairColor = 0;
	hairStyle = 0;
	facialHair = 0;

	showUnderwear = true;
	showHair = true;
	showFacialHair = true;
	showEars = true;
	showFeet = false;

	for (ssize_t i=0; i<NUM_CHAR_SLOTS; i++) {
		equipment[i] = 0;
	}
}

void M2ModelEdit::UpdateTextureList(M2Loader* m2, wxString texName, int special)
{
	for (size_t i=0; i< m2->header.nTextures; i++)
	{
		if (m2->specialTextures[i] == special)
		{
			wxLogMessage(wxT("Updating %s to %s"),m2->TextureList[i].c_str(),texName.c_str());
			m2->TextureList[i] = texName;
			break;
		}
	}
}

wxString M2ModelEdit::makeItemTexture( int region, const wxString name )
{
	// just return an empty filename
	if (name.Length() < 3) 
		return wxEmptyString;

	wxChar leggings = name[name.Length() - 2];

	// try prefered version first
	wxString fn = regionPaths[region];
	fn += name;
	fn += wxT("_");

	if (leggings == 'l' || leggings == 'L')
		fn += wxT("U");
	else
		fn += cd.gender ? wxT("F") : wxT("M");

	fn += wxT(".tga");
	if (MPQFile::getSize(TexPath(fn.wx_str()).c_str()) > 0)  //MPQFile::exists(fn.c_str()) && 
		return fn;

	if (fn.Length() < 5)
		return wxEmptyString;

	// if that failed try alternate texture version
	if (leggings == 'l' || leggings == 'L')
		fn[fn.Length()-5] = cd.gender ? 'F' : 'M';
	else
		fn[fn.Length()-5] = 'U';

	if (MPQFile::getSize(TexPath(fn.wx_str()).c_str()) > 0) //MPQFile::exists(fn.c_str()) && 
		return fn;

	fn = regionPaths[region];
	fn += name;	
	fn += wxT(".tga");

	// return the default name, nothing else could be found.
	return fn;
}

void M2ModelEdit::AddEquipment( ssize_t slot, ssize_t itemnum, ssize_t layer, M2ActorTexture &tex, bool lookup /*= true*/ )
{
	if (slot==CS_PANTS && cd.geosets[CG_TROUSERS]==2) 
		return; // if we are wearing a robe, no pants for us! ^_^

	try {
		const ItemRecord &item = items.getById(itemnum);
		int type = item.type;
		int itemID = 0;

		if (lookup)
			itemID = item.model;
		else
			itemID = itemnum;

		ItemDisplayDB::Record r = itemdisplaydb.getById(itemID);

		// Just a rough check to make sure textures are only being added to where they're suppose to.
		if (slot == CS_CHEST || slot == CS_SHIRT) {
			cd.geosets[CG_WRISTBANDS] = 1 + r.getUInt(ItemDisplayDB::GloveGeosetFlags);

			tex.AddLayer(makeItemTexture(CR_ARM_UPPER, r.getString(ItemDisplayDB::TexArmUpper)), CR_ARM_UPPER, layer);
			tex.AddLayer(makeItemTexture(CR_ARM_LOWER, r.getString(ItemDisplayDB::TexArmLower)), CR_ARM_LOWER, layer);

			tex.AddLayer(makeItemTexture(CR_TORSO_UPPER, r.getString(ItemDisplayDB::TexChestUpper)), CR_TORSO_UPPER, layer);
			tex.AddLayer(makeItemTexture(CR_TORSO_LOWER, r.getString(ItemDisplayDB::TexChestLower)), CR_TORSO_LOWER, layer);

			if (type == IT_ROBE || r.getUInt(ItemDisplayDB::RobeGeosetFlags)==1) {
				tex.AddLayer(makeItemTexture(CR_LEG_UPPER, r.getString(ItemDisplayDB::TexLegUpper)), CR_LEG_UPPER, layer);
				tex.AddLayer(makeItemTexture(CR_LEG_LOWER, r.getString(ItemDisplayDB::TexLegLower)), CR_LEG_LOWER, layer);
			}
		}
		else if (slot == CS_BELT) {
			// Alfred 2009.08.15 add torso_lower for Titan-Forged Waistguard of Triumph
			tex.AddLayer(makeItemTexture(CR_TORSO_LOWER, r.getString(ItemDisplayDB::TexChestLower)), CR_TORSO_LOWER, layer);
			tex.AddLayer(makeItemTexture(CR_LEG_UPPER, r.getString(ItemDisplayDB::TexLegUpper)), CR_LEG_UPPER, layer);
		} else if (slot == CS_BRACERS) {
			tex.AddLayer(makeItemTexture(CR_ARM_LOWER, r.getString(ItemDisplayDB::TexArmLower)), CR_ARM_LOWER, layer);
		} else if (slot == CS_PANTS) {
			cd.geosets[CG_KNEEPADS] = 1 + r.getUInt(ItemDisplayDB::BracerGeosetFlags);

			tex.AddLayer(makeItemTexture(CR_LEG_UPPER, r.getString(ItemDisplayDB::TexLegUpper)), CR_LEG_UPPER, layer);
			tex.AddLayer(makeItemTexture(CR_LEG_LOWER, r.getString(ItemDisplayDB::TexLegLower)), CR_LEG_LOWER, layer);
		} else if (slot == CS_GLOVES) {
			cd.geosets[CG_GLOVES] = 1 + r.getUInt(ItemDisplayDB::GloveGeosetFlags);

			tex.AddLayer(makeItemTexture(CR_HAND, r.getString(ItemDisplayDB::TexHands)), CR_HAND, layer);
			tex.AddLayer(makeItemTexture(CR_ARM_LOWER, r.getString(ItemDisplayDB::TexArmLower)), CR_ARM_LOWER, layer);
		} else if (slot == CS_BOOTS) { // && cd.showFeet==false) {
			cd.geosets[CG_BOOTS] = 1 + r.getUInt(ItemDisplayDB::GloveGeosetFlags);

			tex.AddLayer(makeItemTexture(CR_LEG_LOWER, r.getString(ItemDisplayDB::TexLegLower)), CR_LEG_LOWER, layer);
			if (!cd.showFeet)
				tex.AddLayer(makeItemTexture(CR_FOOT, r.getString(ItemDisplayDB::TexFeet)), CR_FOOT, layer);
		} else if (slot==CS_TABARD && td.showCustom) { // Display our customised tabard
			cd.geosets[CG_TARBARD] = 2;
			tex.AddLayer(td.GetBackgroundTex(CR_TORSO_UPPER), CR_TORSO_UPPER, layer);
			tex.AddLayer(td.GetBackgroundTex(CR_TORSO_LOWER), CR_TORSO_LOWER, layer);
			tex.AddLayer(td.GetIconTex(CR_TORSO_UPPER), CR_TORSO_UPPER, layer);
			tex.AddLayer(td.GetIconTex(CR_TORSO_LOWER), CR_TORSO_LOWER, layer);
			tex.AddLayer(td.GetBorderTex(CR_TORSO_UPPER), CR_TORSO_UPPER, layer);
			tex.AddLayer(td.GetBorderTex(CR_TORSO_LOWER), CR_TORSO_LOWER, layer);

		} else if (slot==CS_TABARD) { // if its just a normal tabard then do the usual
			cd.geosets[CG_TARBARD] = 2;
			tex.AddLayer(makeItemTexture(CR_TORSO_UPPER, r.getString(ItemDisplayDB::TexChestUpper)), CR_TORSO_UPPER, layer);
			tex.AddLayer(makeItemTexture(CR_TORSO_LOWER, r.getString(ItemDisplayDB::TexChestLower)), CR_TORSO_LOWER, layer);

		} else if (slot==CS_CAPE) { // capes
			cd.geosets[CG_CAPE] = 1 + r.getUInt(ItemDisplayDB::GloveGeosetFlags);

			// load the cape texture
			wxString tex = r.getString(ItemDisplayDB::Skin);
			if (!tex.IsEmpty()) 
			{
				wxString texName = makeSkinTexture(wxT("Item\\ObjectComponents\\Cape\\"),tex.wx_str(), ".tga");
				capeTex = renderer->addTexture(TexPath(texName.wx_str()).c_str(), true);
				UpdateTextureList(m2_, texName, TEXTURE_CAPE);
			}
		}

		// robe
		if (cd.geosets[CG_TROUSERS]==1) 
			cd.geosets[CG_TROUSERS] = 1 + r.getUInt(ItemDisplayDB::RobeGeosetFlags);
		if (cd.geosets[CG_TROUSERS]==2) {
			cd.geosets[CG_BOOTS] = 0;		// hide the boots
			//cd.geosets[CG_KNEEPADS] = 0;		// hide the pants
			cd.geosets[CG_TARBARD] = 0;		// also hide the tabard.
		}

		// gloves - this is so gloves have preference over shirt sleeves.
		if (cd.geosets[CG_GLOVES] > 1) 
			cd.geosets[CG_WRISTBANDS] = 0;

	} catch (ItemDisplayDB::NotFound) {}
}


void M2ModelEdit::UpdateNPCModel( M2Loader* m2 )
{
	hairTex = 0;
	furTex = 0;
	gobTex = 0;
	capeTex = 0;

	// Reset geosets
	for (size_t i=0; i<NUM_GEOSETS; i++) 
		cd.geosets[i] = 1;
	cd.geosets[CG_GEOSET100] = cd.geosets[CG_GEOSET200] = cd.geosets[CG_GEOSET300] = 0;

	// show ears, if toggled
	if (cd.showEars) 
		cd.geosets[CG_EARS] = 2;

	M2ActorTexture tex(cd.race);

	// Open first record to declare var
	CharSectionsDB::Record rec = chardb.getRecord(0);
	// It is VITAL that this record can be retrieved to display the NPC
	try {
		rec = chardb.getByParams(cd.race, cd.gender, CharSectionsDB::SkinType, 0, cd.skinColor, cd.useNPC);
	} catch (...) {
		wxLogMessage(wxT("DBC Error: %s : line #%i : %s\n\tAttempting to use character base colour."), __FILE__, __LINE__, __FUNCTION__);
		try {
			rec = chardb.getByParams(cd.race, cd.gender, CharSectionsDB::SkinType, 0, 0, cd.useNPC);
		} catch (...) {
			wxLogMessage(wxT("Exception Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
			return;
		}
	}

	// base layer texture
	try {
		if (!customSkin.IsEmpty()) {
			tex.AddLayer(customSkin, CR_BASE, 0);
			UpdateTextureList(m2, customSkin, TEXTURE_BODY);
		} else {
			wxString baseTexName = rec.getString(CharSectionsDB::Tex1);
			tex.AddLayer(baseTexName, CR_BASE, 0);
			//UpdateTextureList(baseTexName, TEXTURE_BODY);

			if (cd.showUnderwear) {
				rec = chardb.getByParams(cd.race, cd.gender, CharSectionsDB::UnderwearType, 0, cd.skinColor, cd.useNPC);
				tex.AddLayer(rec.getString(CharSectionsDB::Tex1), CR_PELVIS_UPPER, 1); // panties
				tex.AddLayer(rec.getString(CharSectionsDB::Tex1), CR_PELVIS_UPPER, 1); // panties
			}

			// face
			rec = chardb.getByParams(cd.race, cd.gender, CharSectionsDB::FaceType, cd.faceType, cd.skinColor, cd.useNPC);
			tex.AddLayer(rec.getString(CharSectionsDB::Tex1), CR_FACE_LOWER, 1);
			tex.AddLayer(rec.getString(CharSectionsDB::Tex2), CR_FACE_UPPER, 1);

			// facial hair
			rec = chardb.getByParams(cd.race, cd.gender, CharSectionsDB::FacialHairType, cd.facialHair, cd.facialColor, 0);
			tex.AddLayer(rec.getString(CharSectionsDB::Tex1), CR_FACE_LOWER, 2);
			tex.AddLayer(rec.getString(CharSectionsDB::Tex2), CR_FACE_UPPER, 2);
		} 

		// Tauren fur
		wxString furTexName = rec.getString(CharSectionsDB::Tex2);
		if (!furTexName.IsEmpty()) {
			int pos = furTexName.find_first_of('.', 0);
			furTexName[pos+1] = 't';
			furTexName[pos+2] = 'g';
			furTexName[pos+3] = 'a';
			furTex =m2->renderer->addTexture(furTexName.wx_str(), true);// texturemanager.add(hairTexfn);			
			UpdateTextureList(m2, furTexName, TEXTURE_FUR);
		}

	} catch (...) {
		wxLogMessage(wxT("Exception base layer Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
	}

	// hair
	try {
		CharSectionsDB::Record rec = chardb.getByParams(cd.race, cd.gender, CharSectionsDB::HairType, cd.hairStyle, cd.hairColor, cd.useNPC);
		wxString hairTexfn = rec.getString(CharSectionsDB::Tex1);
		if (!hairTexfn.IsEmpty()) {
			int pos = hairTexfn.find_first_of('.', 0);
			hairTexfn[pos+1] = 't';
			hairTexfn[pos+2] = 'g';
			hairTexfn[pos+3] = 'a';
			hairTex =m2->renderer->addTexture(hairTexfn.wx_str(), true);// texturemanager.add(hairTexfn);				
		} else {
			rec = chardb.getByParams(cd.race, cd.gender, CharSectionsDB::HairType, 1, cd.hairColor, cd.useNPC);
			hairTexfn = rec.getString(CharSectionsDB::Tex1);
			if (!hairTexfn.IsEmpty()) {
				int pos = hairTexfn.find_first_of('.', 0);
				hairTexfn[pos+1] = 't';
				hairTexfn[pos+2] = 'g';
				hairTexfn[pos+3] = 'a';
				hairTex =m2->renderer->addTexture(hairTexfn.wx_str(), true);// texturemanager.add(hairTexfn);				
				UpdateTextureList(m2, hairTexfn, TEXTURE_HAIR);
			} else 
				hairTex = 0;
		}

		//tex.AddLayer(rec.getString(CharSectionsDB::Tex2), CR_FACE_LOWER, 3);
		//tex.AddLayer(rec.getString(CharSectionsDB::Tex3), CR_FACE_UPPER, 3);

	} catch (CharSectionsDB::NotFound) {
		wxLogMessage(wxT("Assertion hair Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
		hairTex = 0;
	}

	bool bald = false;
	bool showHair = cd.showHair;
	bool showFacialHair = cd.showFacialHair;

	// facial hair geosets
	try {
		CharFacialHairDB::Record frec = facialhairdb.getByParams((unsigned int)cd.race, (unsigned int)cd.gender, (unsigned int)cd.facialHair);
		if (gameVersion < VERSION_CATACLYSM) {
			cd.geosets[CG_GEOSET100] = frec.getUInt(CharFacialHairDB::Geoset100);
			cd.geosets[CG_GEOSET200] = frec.getUInt(CharFacialHairDB::Geoset200);
			cd.geosets[CG_GEOSET300] = frec.getUInt(CharFacialHairDB::Geoset300);
		} else {
			cd.geosets[CG_GEOSET100] = frec.getUInt(CharFacialHairDB::Geoset100V400);
			cd.geosets[CG_GEOSET200] = frec.getUInt(CharFacialHairDB::Geoset200V400);
			cd.geosets[CG_GEOSET300] = frec.getUInt(CharFacialHairDB::Geoset300V400);
		}

		// Hide facial fair if it isn't toggled and they don't have tusks, horns, etc.
		if (showFacialHair == false) {		
			CharRacesDB::Record race = racedb.getById(cd.race);
			wxString tmp;
			if (gameVersion >= VERSION_CATACLYSM)
				tmp = race.getString(CharRacesDB::GeoType1V400);
			else
				tmp = race.getString(CharRacesDB::GeoType1);
			if (tmp.Lower() == wxT("normal")) {
				cd.geosets[CG_GEOSET100] = 1;
				cd.geosets[CG_GEOSET200] = 1;
				cd.geosets[CG_GEOSET300] = 1;
			}
		}
	} catch (CharFacialHairDB::NotFound) {
		wxLogMessage(wxT("Assertion facial hair Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
	}

	// select hairstyle geoset(s)
	for (CharHairGeosetsDB::Iterator it = hairdb.begin(); it != hairdb.end(); ++it) {
		if (it->getUInt(CharHairGeosetsDB::Race)==cd.race && it->getUInt(CharHairGeosetsDB::Gender)==cd.gender) {
			unsigned int id = it->getUInt(CharHairGeosetsDB::Geoset);
			unsigned int section = it->getUInt(CharHairGeosetsDB::Section);
			if (id!=0) {
				for (size_t j=0; j<m2->geosets.size(); j++) {
					if (m2->geosets[j].id == id)
						m2->showGeosets[j] = (cd.hairStyle==section) && showHair;
				}
			} else if (cd.hairStyle==section) 
				bald = true;
		}
	}

	// If they have no hair, toggle the 'bald' flag.
	if (!showHair)
		bald = true;

	// check if we have a robe on
	bool hadRobe = false;
	if (cd.equipment[CS_CHEST] != 0) {
		try {
			const ItemRecord &item = items.getById(cd.equipment[CS_CHEST]);
			if (item.type==IT_ROBE || item.type==IT_CHEST) {
				ItemDisplayDB::Record r = itemdisplaydb.getById(item.model);
				if (r.getUInt(ItemDisplayDB::RobeGeosetFlags)==1) 
					hadRobe = true;
			}
		} catch (...) {
			wxLogMessage(wxT("Exception robe Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
		}
	}


	// check if we have a kilt on, just like our robes
	if (cd.equipment[CS_PANTS] != 0) {
		try {
			const ItemRecord &item = items.getById(cd.equipment[CS_PANTS]);
			int type = item.type;
			if (type==IT_PANTS) {
				ItemDisplayDB::Record r = itemdisplaydb.getById(item.model);
				if (r.getUInt(ItemDisplayDB::RobeGeosetFlags)==1) 
					hadRobe = true;
			}
		} catch (...) {
			wxLogMessage(wxT("Exception Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
		}
	}

	// Default
	slotOrderWithRobe[7] = CS_CHEST;
	slotOrderWithRobe[8] = CS_GLOVES;

	// check the order of robe/gloves
	if (cd.equipment[CS_CHEST] && cd.equipment[CS_GLOVES]) {
		try {
			//const ItemRecord &item = items.getById(cd.equipment[CS_CHEST]);
			//if (item.type==IT_ROBE) {
			//	ItemDisplayDB::Record r = itemdisplaydb.getById(item.model);
			//if (r.getUInt(ItemDisplayDB::GeosetA)>0) {
			ItemDisplayDB::Record r = itemdisplaydb.getById(cd.equipment[CS_GLOVES]);
			if (r.getUInt(ItemDisplayDB::GloveGeosetFlags)==0) {
				slotOrderWithRobe[7] = CS_GLOVES;
				slotOrderWithRobe[8] = CS_CHEST;
			}
			//}
			//}
		} catch (...) {
			wxLogMessage(wxT("Exception Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
		}
	}

	// dressup
	for (ssize_t i=0; i<NUM_CHAR_SLOTS; i++) {
		int sn = hadRobe ? slotOrderWithRobe[i] : slotOrder[i];
		if (cd.equipment[sn] != 0) 
			AddEquipment(sn, cd.equipment[sn], 10+i, tex, false);
	}


	// reset geosets
	for (size_t j=0; j<m2->geosets.size(); j++) {
		int id = m2->geosets[j].id;

		// hide top-of-head if we have hair.
		if (id == 1) 
			m2->showGeosets[j] = bald;

		for (size_t i=1; i<NUM_GEOSETS; i++) {
			int a = (int)i*100, b = ((int)i+1) * 100;
			if (id>a && id<b) 
				m2->showGeosets[j] = (id == (a + cd.geosets[i]));
		}
	}

	// finalize texture
	if (!cachedCombosedTex.empty())
		charTex = m2->renderer->addTexture(cachedCombosedTex.c_str(), true);
	else 
		tex.Combine(m2->renderer, charTex);

	// set replacable textures
	m2->replaceTextures[TEXTURE_BODY] = charTex;
	m2->replaceTextures[TEXTURE_CAPE] = capeTex;
	m2->replaceTextures[TEXTURE_HAIR] = hairTex;
	m2->replaceTextures[TEXTURE_FUR] = furTex;
	m2->replaceTextures[TEXTURE_GAMEOBJECT1] = gobTex;
}

wxString TabardDetails::GetIconTex( int slot )
{
	if(slot == CR_TORSO_UPPER)
		return wxString::Format(wxT("Textures\\GuildEmblems\\Emblem_%02d_%02d_TU_U.tga"), Icon, IconColor);
	else
		return wxString::Format(wxT("Textures\\GuildEmblems\\Emblem_%02d_%02d_TL_U.tga"), Icon, IconColor);
}

wxString TabardDetails::GetBorderTex( int slot )
{
	if (slot == CR_TORSO_UPPER)
		return wxString::Format(wxT("Textures\\GuildEmblems\\Border_%02d_%02d_TU_U.tga"), Border, BorderColor);
	else
		return wxString::Format(wxT("Textures\\GuildEmblems\\Border_%02d_%02d_TL_U.tga"), Border, BorderColor);
}

wxString TabardDetails::GetBackgroundTex( int slot )
{
	if (slot == CR_TORSO_UPPER)
		return wxString::Format(wxT("Textures\\GuildEmblems\\Background_%02d_TU_U.tga"), Background);
	else
		return wxString::Format(wxT("Textures\\GuildEmblems\\Background_%02d_TL_U.tga"), Background);
}

int TabardDetails::GetMaxIcon()
{
	int i = 0;
	while(1) {
		if (!MPQFile::exists(wxString::Format(wxT("Textures\\GuildEmblems\\Emblem_%02d_%02d_TU_U.tga"), i, 0))) {
			break;
		}
		i ++;
	}
	return i;
}

int TabardDetails::GetMaxIconColor( int icon )
{
	int i = 0;
	while(1) {
		if (!MPQFile::exists(wxString::Format(wxT("Textures\\GuildEmblems\\Emblem_%02d_%02d_TU_U.tga"), icon, i))) {
			break;
		}
		i ++;
	}
	return i;
}

int TabardDetails::GetMaxBorder()
{
	int i = 0;
	while(1) {
		if (!MPQFile::exists(wxString::Format(wxT("Textures\\GuildEmblems\\Border_%02d_%02d_TU_U.tga"), i))) {
			break;
		}
		i ++;
	}
	return i;
}

int TabardDetails::GetMaxBorderColor( int border )
{
	int i = 0;
	while(1) {
		if (!MPQFile::exists(wxString::Format(wxT("Textures\\GuildEmblems\\Border_%02d_%02d_TU_U.tga"), border, i))) {
			break;
		}
		i ++;
	}
	return i;
}

int TabardDetails::GetMaxBackground()
{
	int i = 0;
	while(1) {
		if (!MPQFile::exists(wxString::Format(wxT("Textures\\GuildEmblems\\Background_%02d_TU_U.tga"), i))) {
			break;
		}
		i ++;
	}
	return i;
}


