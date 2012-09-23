#ifndef FRAMEWORK_M2LOADER_H
#define FRAMEWORK_M2LOADER_H

#define WotLK 

#include <Framework/mpq.h>
#include <Renderer/Model_M2Format.h>
#include <Renderer/Model_M2Interpolator.h>
typedef uint32 TextureID;

enum ModelType {
	MT_NORMAL,
	MT_CHAR,
	MT_WMO,
	MT_NPC
};

enum KeyBoneTable {
	//Block F - Key Bone lookup table.
	//---------------------------------
	BONE_LARM = 0,		// 0, ArmL: Left upper arm
	BONE_RARM,			// 1, ArmR: Right upper arm
	BONE_LSHOULDER,		// 2, ShoulderL: Left Shoulder / deltoid area
	BONE_RSHOULDER,		// 3, ShoulderR: Right Shoulder / deltoid area
	BONE_STOMACH,		// 4, SpineLow: (upper?) abdomen
	BONE_WAIST,			// 5, Waist: (lower abdomen?) waist
	BONE_HEAD,			// 6, Head
	BONE_JAW,			// 7, Jaw: jaw/mouth
	BONE_RFINGER1,		// 8, IndexFingerR: (Trolls have 3 "fingers", this points to the 2nd one.
	BONE_RFINGER2,		// 9, MiddleFingerR: center finger - only used by dwarfs.. don't know why
	BONE_RFINGER3,		// 10, PinkyFingerR: (Trolls have 3 "fingers", this points to the 3rd one.
	BONE_RFINGERS,		// 11, RingFingerR: Right fingers -- this is -1 for trolls, they have no fingers, only the 3 thumb like thingys
	BONE_RTHUMB,		// 12, ThumbR: Right Thumb
	BONE_LFINGER1,		// 13, IndexFingerL: (Trolls have 3 "fingers", this points to the 2nd one.
	BONE_LFINGER2,		// 14, MiddleFingerL: Center finger - only used by dwarfs.
	BONE_LFINGER3,		// 15, PinkyFingerL: (Trolls have 3 "fingers", this points to the 3rd one.
	BONE_LFINGERS,		// 16, RingFingerL: Left fingers
	BONE_LTHUMB,		// 17, ThubbL: Left Thumb
	BONE_BTH,			// 18, $BTH: In front of head
	BONE_CSR,			// 19, $CSR: Left hand
	BONE_CSL,			// 20, $CSL: Left hand
	BONE_BREATH,		// 21, _Breath
	BONE_NAME,			// 22, _Name
	BONE_NAMEMOUNT,		// 23, _NameMount
	BONE_CHD,			// 24, $CHD: Head
	BONE_CCH,			// 25, $CCH: Bust
	BONE_ROOT,			// 26, Root: The "Root" bone,  this controls rotations, transformations, etc of the whole model and all subsequent bones.
	BONE_WHEEL1,		// 27, Wheel1
	BONE_WHEEL2,		// 28, Wheel2
	BONE_WHEEL3,		// 29, Wheel3
	BONE_WHEEL4,		// 30, Wheel4
	BONE_WHEEL5,		// 31, Wheel5
	BONE_WHEEL6,		// 32, Wheel6
	BONE_WHEEL7,		// 33, Wheel7
	BONE_WHEEL8,		// 34, Wheel8
	BONE_MAX
};

/*
Texture Types
Texture type is 0 for regular textures, nonzero for skinned textures (filename not referenced in the M2 file!) For instance, in the NightElfFemale model, her eye glow is a type 0 texture and has a file name, the other 3 textures have types of 1, 2 and 6. The texture filenames for these come from client database files:
DBFilesClient\CharSections.dbc
DBFilesClient\CreatureDisplayInfo.dbc
DBFilesClient\ItemDisplayInfo.dbc
(possibly more)
*/
enum TextureTypes {
	TEXTURE_FILENAME=0,			// Texture given in filename
	TEXTURE_BODY,				// Body + clothes
	TEXTURE_CAPE,				// Item, Capes ("Item\ObjectComponents\Cape\*.blp")
	TEXTURE_ARMORREFLECT,		// 
	TEXTURE_HAIR=6,				// Hair, bear
	TEXTURE_FUR=8,				// Tauren fur
	TEXTURE_INVENTORY_ART1,		// Used on inventory art M2s (1): inventoryartgeometry.m2 and inventoryartgeometryold.m2
	TEXTURE_QUILL,				// Only used in quillboarpinata.m2. I can't even find something referencing that file. Oo Is it used?
	TEXTURE_GAMEOBJECT1,		// Skin for creatures or gameobjects #1
	TEXTURE_GAMEOBJECT2,		// Skin for creatures or gameobjects #2
	TEXTURE_GAMEOBJECT3,		// Skin for creatures or gameobjects #3
	TEXTURE_INVENTORY_ART2		// Used on inventory art M2s (2): ui-buffon.m2 and forcedbackpackitem.m2 (LUA::Model:ReplaceIconTexture("texture"))
};

enum TextureFlags {
	TEXTURE_WRAPX=1,
	TEXTURE_WRAPY
};

enum ModelLightTypes {
	MODELLIGHT_DIRECTIONAL=0,
	MODELLIGHT_POINT
};



struct AnimInfo {
	short Loops;
	unsigned int AnimID;
};


class Bone {
public:
	Animated<noVec3> trans;
	//Animated<Quaternion> rot;
	Animated<noQuat, PACK_QUATERNION, Quat16ToQuat32> rot;
	Animated<noVec3> scale;

	noVec3 pivot, transPivot;
	int16 parent;

	bool billboard;
	//Matrix mat;
	//Matrix mrot;

	ModelBoneDef boneDef;

	bool calc;
	//Model *model;
	void calcMatrix(Bone* allbones, int anim, int time, bool rotate=true);
#ifdef WotLK
	void init(MPQFile &f, ModelBoneDef &b, uint32 *global, MPQFile *animfiles);
#else
	void init(MPQFile &f, ModelBoneDef &b, uint32 *global);
#endif
};


class TextureAnim {
public:
	Animated<noVec3> trans, rot, scale;

	noVec3 tval, rval, sval;

	void calc(int anim, int time);
	void init(MPQFile &f, ModelTexAnimDef &mta, uint32 *global);
	void setup(int anim);
};

struct ModelColor {
	Animated<noVec3> color;
	AnimatedShort opacity;

	void init(MPQFile &f, ModelColorDef &mcd, uint32 *global);
};

struct ModelTransparency {
	AnimatedShort trans;

	void init(MPQFile &f, ModelTransDef &mtd, uint32 *global);
};

struct ModelCamera {
	bool ok;

	noVec3 pos, target;
	float nearclip, farclip, fov;
	Animated<noVec3> tPos, tTarget;
	Animated<float> rot;

	void init(MPQFile &f, ModelCameraDef &mcd, uint32 *global);
	void setup(int time=0);

	ModelCamera():ok(false) {}
};

struct ModelLight {
	int type, parent;
	noVec3 pos, tpos, dir, tdir;
	Animated<noVec3> diffColor, ambColor;
	Animated<float> diffIntensity, ambIntensity, AttenStart, AttenEnd;
	Animated<int> UseAttenuation;

	void init(MPQFile &f, ModelLightDef &mld, uint32 *global);
	void setup(int time, uint32 l);
};

class M2Loader 
{
public:
	M2Loader();

	void	Load(const char* fname);
	
	void	initAnimated(MPQFile &f);
	bool	initStatic(MPQFile &f);
	
	void	initCommon(MPQFile &f);
	bool	isAnimated(MPQFile& f);

	ModelHeader header;
	std::vector<ModelRenderPass> passes;
	std::vector<ModelGeoset> geosets;
	ModelAnimation *anims;
	int16 *animLookups;
	Bone *bones;
	MPQFile *animfiles;

#ifdef WotLK
	std::string fullname;
	std::string modelname;
	std::string lodname;
#endif


private:
	uint32 vbuf, nbuf, tbuf;
	size_t vbufsize;

	ModelVertex*					vertDatas;
	noVec3							*vertices, *normals;
	noVec2							*texCoords;

	uint16							*indices;
	size_t							nIndices;
	std::vector<std::string>		TextureList;

	float							rad;
	bool							animGeom, animTextures, animBones;

	noVec3		*bounds;
	uint16		*boundIndices;

	TextureID*						textureIds;
	int specialTextures[TEXTURE_MAX];
	uint32 replaceTextures[TEXTURE_MAX];
	bool useReplaceTextures[TEXTURE_MAX];

	ModelType modelType;	// Load 하기전 세팅 필요.
	bool	*showGeosets;

	//std::vector<ModelAttachment> atts;
	static const size_t ATT_MAX = 50;
	int16 attLookup[ATT_MAX];
	int16 keyBoneLookup[BONE_MAX];

public:
	TextureAnim		*texAnims;
	uint32			*globalSequences;
	ModelColor		*colors;
	ModelTransparency *transparency;
	ModelLight		*lights;

	

private:
	void LoadVertices( MPQFile &f );
	

};


#endif