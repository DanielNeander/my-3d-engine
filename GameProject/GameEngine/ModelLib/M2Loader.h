#ifndef FRAMEWORK_M2LOADER_H
#define FRAMEWORK_M2LOADER_H


#define WotLK 

#include <extlib/libmpq/mpq.h>
#include "M2Enums.h"
#include "Model_M2Format.h"
#include "Model_M2Interpolator.h"
#include "M2AnimationManager.h"
#include "M2Skeleton.h"

class Renderer;
class M2Loader;
class M2ParticleSystem;
class M2RibbonEmitter;

noVec3 fixCoordSystem(noVec3 v);


namespace M2 {
	typedef uint32 TextureID;
}


struct RenderData;


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
	noVec3 WorldOffset;
	float WorldRotation;

	void init(MPQFile &f, ModelCameraDef &mcd, uint32 *global, wxString modelname);
	void initv10(MPQFile &f, ModelCameraDefV10 &mcd, uint32 *global, wxString modelname);
	void setup(size_t time=0);

	ModelCamera():ok(false) {}
};

struct ModelLight {
	int type, parent;
	noVec3 pos, tpos, dir, tdir;
	Animated<noVec3> diffColor, ambColor;
	Animated<float> diffIntensity, ambIntensity, AttenStart, AttenEnd;
	Animated<int> UseAttenuation;

	void init(MPQFile &f, ModelLightDef &mld, uint32 *global);
	void setup(size_t time, uint32 l);
};

struct ModelAttachment {
	int id;
	noVec3 pos;
	int bone;
	M2Loader *model;

	void init(MPQFile &f, ModelAttachmentDef &mad, uint32 *global);
	void setup();
	void setupParticle();
};



class M2AnimationEvent {
	ModelEventDef def;
public:
	void init(MPQFile &f, ModelEventDef &mad, uint32 *global);
};


// ModelViewer의 LoadModel, LoadNPC,  LoadWMO
class M2Loader 
{
public:
	friend class M2AnimationManager;

	M2Loader(Renderer* _renderer);
	~M2Loader();

	static M2AnimationManager* (*createAnimMgr)(ModelAnimation *anim, int numAnims);

	void Load(const char* fname, bool bForceAnim);
	
	void	initAnimated(MPQFile &f);
	bool	initStatic(MPQFile &f);
	
	void	initCommon(MPQFile &f);
	bool	isAnimated(MPQFile& f);

	void HideGeosets();
	void GetRaceAndGenderName(wxString& raceName, wxString& genderName);

	void GetRenderData(RenderData& inout);

	virtual void CreateRenderPass();	

	virtual void DrawShadow(const mat4& world, const mat4& view, const mat4& proj) {}
	virtual void DrawZOnly(const noMat4& world, const mat4& view, const mat4& proj) {}
	virtual void DrawCascaded() {}
	virtual void DrawModel(const mat4& world, const mat4& view, const mat4& proj);
	virtual void DrawShadowModel(const mat4& world, const mat4& view, const mat4& proj) {}
	virtual void DrawParticles();
	virtual void DrawSSAO( const mat4& world, const mat4& view, const mat4& proj ) {}

	
	ModelHeader header;
	std::vector<ModelCamera> cameras;
	std::vector<ModelRenderPass> renderPasses;
	std::vector<ModelGeoset> geosets;
	
	
	// ===============================
	// Bone & Animation data
	// ===============================
	ModelAnimation *anims;
	M2AnimationManager* animManager;
	int16 *animLookups;
	Bone *bones;
	MPQFile *animfiles;

	size_t currAnim;
	bool   animcalc;
	size_t	anim, animtime;

	void reset() {
		animcalc = false;
	}

	virtual void update(size_t deltaTime, size_t accumTime); /* {

		size_t deltaTime = 0;
		if (animManager->IsPaused())
			deltaTime = 0;
		if (!animManager->IsParticlePaused())
			deltaTime = animManager->GetTimeDiff();			

		if (animated)
			animManager->Tick(deltaTime);
	}*/

	inline const Bone&	GetBone(int bone) const { return bones[keyBoneLookup[bone]]; }
	inline Bone&	GetBone(int bone)			{ return bones[keyBoneLookup[bone]]; }
	inline int16	GetBoneId(int bone)			{ return keyBoneLookup[bone];}
	inline std::string GetTextureName(TextureID id) { return TextureMap.find(id)->second; }
	
protected:
	virtual void calcBones(ssize_t anim, size_t time);
	virtual void calcBones(M2Animation& animation, size_t time);

public:

	void SetViewMatrix(const mat4& view);

	// Attachment
	void setupAtt(int id);
	void setupAtt2(int id);

#ifdef WotLK
	std::string name;
	std::string fullname;
	std::string modelname;
	std::string lodname;
#endif

	ModelVertex*					vertDatas;

	size_t							*indiceToVerts;
	noVec3							*vertices, *normals;
	noVec2							*texCoords;
	uint16							*indices;
	size_t							nIndices;
	uint32							numVertices;
	std::vector<std::string>		TextureList;	
	std::map<M2::TextureID, std::string>	TextureMap;

	// Misc
	float							rad;
	float		trans;
	bool	*showGeosets;


	int specialTextures[TEXTURE_MAX];
	uint32 replaceTextures[TEXTURE_MAX];
	bool useReplaceTextures[TEXTURE_MAX];

	M2::TextureID*						textureIds;

	ModelType modelType;	// Load 하기전 세팅 필요.

	CharModelDetails charModelDetails;

	bool IsSKinned() const { return animBones; }


	uint16		*boundIndices;
	noVec3		*bounds;
	int			vertexStride;

protected:
	uint32 vbuf, nbuf, tbuf;
	size_t vbufsize;

	
	
	
	
	bool							animGeom, animTextures, animBones;


	

	


	

	bool ind;
	bool hasCamera;
	bool hasParticles;
	bool isWMO;
	bool isMount;
	bool forceAnim;

public:
	bool animated;

	TextureAnim		*texAnims;
	uint32			*globalSequences;
	ModelColor		*colors;
	ModelTransparency *transparency;
	ModelLight		*lights;
	M2AnimationEvent *events;
	M2ParticleSystem* particleSystems;
	M2RibbonEmitter* ribbons;

	std::vector<ModelAttachment> atts;
	static const size_t ATT_MAX = 50;
	int16 attLookup[ATT_MAX];
	int16 keyBoneLookup[BONE_MAX];


	RenderData render_data[15];
	int numPass;

	Renderer* renderer;

	mat4 view_;

	size_t deltaTime_;

protected:
	void LoadVertices( MPQFile &f );
	void setLOD(MPQFile &f, int index);
	bool UpdateColors(int color, int opacity, bool unlit, noVec4& opacitycolor,  noVec4& emissiveColor);	
	void animating(ssize_t anim);
	void UpdateParticles(float dt);
	
};


std::string ModelPath(const char* name);
std::string TexPath(const char* name);
std::string ItemTexPath(const char* name);

#endif