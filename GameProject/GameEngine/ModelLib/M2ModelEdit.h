#pragma once 
#ifndef __M2MODELEDIT_H__
#define __M2MODELEDIT_H__

#include "M2Enums.h"

class M2Loader;
class Renderer;

#define	REGION_FAC_X	2
#define REGION_FAC_Y  2
#define	REGION_PX_WIDTH	(256*REGION_FAC_X)
#define REGION_PX_HEIGHT (256*REGION_FAC_Y)

struct CharRegionCoords {
	int xpos, ypos, xsize, ysize;
};

const CharRegionCoords regions[NUM_REGIONS] =
{
	{0, 0, 256*REGION_FAC_X, 256*REGION_FAC_Y},	// base
	{0, 0, 128*REGION_FAC_X, 64*REGION_FAC_Y},	// arm upper
	{0, 64*REGION_FAC_Y, 128*REGION_FAC_X, 64*REGION_FAC_Y},	// arm lower
	{0, 128*REGION_FAC_Y, 128*REGION_FAC_X, 32*REGION_FAC_Y},	// hand
	{0, 160*REGION_FAC_Y, 128*REGION_FAC_X, 32*REGION_FAC_Y},	// face upper
	{0, 192*REGION_FAC_Y, 128*REGION_FAC_X, 64*REGION_FAC_Y},	// face lower
	{128*REGION_FAC_X, 0, 128*REGION_FAC_X, 64*REGION_FAC_Y},	// torso upper
	{128*REGION_FAC_X, 64*REGION_FAC_Y, 128*REGION_FAC_X, 32*REGION_FAC_Y},	// torso lower
	{128*REGION_FAC_X, 96*REGION_FAC_Y, 128*REGION_FAC_X, 64*REGION_FAC_Y}, // pelvis upper
	{128*REGION_FAC_X, 160*REGION_FAC_Y, 128*REGION_FAC_X, 64*REGION_FAC_Y},// pelvis lower
	{128*REGION_FAC_X, 224*REGION_FAC_Y, 128*REGION_FAC_X, 32*REGION_FAC_Y}	// foot
};

const CharRegionCoords pandaren_regions[NUM_REGIONS] =
{
	{0, 0, 256*REGION_FAC_X*2, 256*REGION_FAC_Y},	// base
	{0, 0, 128*REGION_FAC_X, 64*REGION_FAC_Y},	// arm upper
	{0, 64*REGION_FAC_Y, 128*REGION_FAC_X, 64*REGION_FAC_Y},	// arm lower
	{0, 128*REGION_FAC_Y, 128*REGION_FAC_X, 32*REGION_FAC_Y},	// hand
	{128*REGION_FAC_X*2, 0, 256*REGION_FAC_X, 256*REGION_FAC_Y},	// face upper
	{0, 192*REGION_FAC_Y, 128*REGION_FAC_X, 64*REGION_FAC_Y},	// face lower
	{128*REGION_FAC_X, 0, 128*REGION_FAC_X, 64*REGION_FAC_Y},	// torso upper
	{128*REGION_FAC_X, 64*REGION_FAC_Y, 128*REGION_FAC_X, 32*REGION_FAC_Y},	// torso lower
	{128*REGION_FAC_X, 96*REGION_FAC_Y, 128*REGION_FAC_X, 64*REGION_FAC_Y}, // pelvis upper
	{128*REGION_FAC_X, 160*REGION_FAC_Y, 128*REGION_FAC_X, 64*REGION_FAC_Y},// pelvis lower
	{128*REGION_FAC_X, 224*REGION_FAC_Y, 128*REGION_FAC_X, 32*REGION_FAC_Y}	// foot
};


struct M2ActorTextureFragment
{
	std::string name;
	int region;
	int layer;

	const bool operator<(const M2ActorTextureFragment& c) const
	{
		return layer < c.layer;
	}
};

struct M2ActorTexture
{
	std::vector<M2ActorTextureFragment>	components;
	size_t race;

	M2ActorTexture(size_t _race) : race(_race) {}

	void AddLayer(std::string fn, int region, int layer);
	

	void AddLayer(wxString fn, int region, int layer);
	
	void Combine(Renderer* renderer, TextureID& texID);
};

struct TabardDetails
{
	int Icon;
	int IconColor;
	int Border;
	int BorderColor;
	int Background;

	int maxIcon;
	int maxIconColor;
	int maxBorder;
	int maxBorderColor;
	int maxBackground;

	bool showCustom;

	wxString GetIconTex(int slot);
	wxString GetBorderTex(int slot);
	wxString GetBackgroundTex(int slot);

	int GetMaxIcon();
	int GetMaxIconColor(int icon);
	int GetMaxBorder();
	int GetMaxBorderColor(int border);
	int GetMaxBackground();
};

struct M2ActorDetails
{
	size_t skinColor, faceType, hairColor, hairStyle, facialHair;
	size_t facialColor, maxFacialColor;
	size_t maxHairStyle, maxHairColor, maxSkinColor, maxFaceType, maxFacialHair;

	size_t race, gender;

	size_t useNPC;
	size_t eyeGlowType;

	bool showUnderwear, showEars, showHair, showFacialHair, showFeet;

	int equipment[NUM_CHAR_SLOTS];
	int geosets[NUM_GEOSETS];

	// save + load equipment
	void save(std::string fn, TabardDetails *td);
	bool load(std::string fn, TabardDetails *td);

	//void loadSet(ItemSetDB &sets, ItemDatabase &items, int setid);
	//void loadStart(StartOutfitDB &start, ItemDatabase &items, int cls);

	void reset();
};

struct TextureGroup {
	static const size_t num = 3;
	int base, count;
	wxString tex[num];
	TextureGroup()
	{
		for (size_t i=0; i<num; i++) {
			tex[i] = wxT("");
		}
	}

	// default copy constr
	TextureGroup(const TextureGroup &grp)
	{
		for (size_t i=0; i<num; i++) {
			tex[i] = grp.tex[i];
		}
		base = grp.base;
		count = grp.count;
	}
	const bool operator<(const TextureGroup &grp) const
	{
		for (size_t i=0; i<num; i++) {
			if (tex[i]<grp.tex[i]) return true;
			if (tex[i]>grp.tex[i]) return false;
		}
		return false;
	}
};

typedef std::set<TextureGroup> TextureSet;

class M2ModelEdit 
{
public:
	M2ModelEdit();
	M2ModelEdit(M2Loader* m2);

	virtual void Init(Renderer* renderer_);

	M2ActorDetails cd;
	TabardDetails td;

	TextureID charTex, hairTex, furTex, capeTex, gobTex;

	M2Loader* m2_;
	Renderer* renderer;

	wxString customSkin;
	std::string cachedCombosedTex;

	bool bSheathe;

	void AddEquipment(ssize_t slot, ssize_t itemnum, ssize_t layer, M2ActorTexture &tex, bool lookup = true);

	wxString makeItemTexture(int region, const wxString name);

	void UpdateCharModel(M2Loader* m2);
	void UpdateNPCModel( M2Loader* m2 );
	void UpdateCreatureModel(M2Loader* m);
	
	void UpdateEyeGlow(M2Loader* m2);
	void UpdateTextureList(M2Loader* m2, wxString texName, int special);
	bool FillSkinSelector(M2Loader* m, TextureSet &skins);
};

#endif