#ifndef RENDERER_M2_PARTICLE_H
#define RENDERER_M2_PARTICLE_H

//#include <EngineCore/TFreeList.h>
#include <EngineCore/Math/Vector.h>
#include "Model_M2Interpolator.h"
#include "M2Loader.h"
#include "mpq_libmpq.h"
#include <list>

#define MAX_PARTICLES 2000


class M2ParticleSystem;
class M2RibbonEmitter;

struct M2Particle {
	noVec3 pos, speed, down, origin, dir;
	noVec3	corners[4];
	//noVec3 tpos;
	float size, life, maxlife;
	unsigned int tile;
	noVec4 color;	
};


typedef std::list<M2Particle> ParticleList;

class M2ParticleEmitter {
public:
	M2ParticleEmitter(M2ParticleSystem* sys): sys(sys) {}

	M2ParticleSystem *sys;
	virtual M2Particle newParticle(size_t anim, size_t time, float w, float l, float spd, float var, float spr, float spr2) = 0;	
};

class M2PlaneParticleEmitter: public M2ParticleEmitter {
public:
	M2PlaneParticleEmitter(M2ParticleSystem *sys): M2ParticleEmitter(sys) {}
	M2Particle newParticle(size_t anim, size_t time, float w, float l, float spd, float var, float spr, float spr2);
};


class M2SphereParticleEmitter: public M2ParticleEmitter {
public:
	M2SphereParticleEmitter(M2ParticleSystem *sys): M2ParticleEmitter(sys) {}
	M2Particle newParticle(size_t anim, size_t time, float w, float l, float spd, float var, float spr, float spr2);
};

struct TexCoordSet {
	noVec2 tc[4];
};

class M2ParticleSystem {

public:
	float mid, slowdown, rotation;
	noVec3 pos;
	//GLuint texture;
	TextureID texture;
	M2ParticleEmitter *emitter;
	ParticleList particles;
	
	int blend, order, ParticleType;
	size_t manim, mtime;
	int rows, cols;
	std::vector<TexCoordSet> tiles;
	void initTile(noVec2 *tc, int num);
	bool billboard;

	float rem;
	//bool transform;

	// unknown parameters omitted for now ...
	int32 flags;
	int16 EmitterType;

	Bone *parent;

public:
	M2Loader *model;
	float tofs;

	Animated<uint16> enabled;
	Animated<float> speed, variation, spread, lat, gravity, lifespan, rate, areal, areaw, deacceleration;
	noVec4 colors[3];
	float sizes[3];

	M2ParticleSystem();
	~M2ParticleSystem() { delete emitter; }

	void init(MPQFile &f, ModelParticleEmitterDef &mta, uint32 *globals);
	void update(float dt);

	void setup(size_t anim, size_t time);
	void draw();

	friend class M2PlaneParticleEmitter;
	friend class M2SphereParticleEmitter;


};

struct M2RibbonSegment {
	noVec3 pos, up, back;
	float len,len0;
};

class M2RibbonEmitter {

public:
	Animated<noVec3> color;
	AnimatedShort opacity;
	Animated<float> above, below;

	Bone *parent;
	float f1, f2;

	noVec3 pos;

	size_t manim, mtime;
	float length, seglen;
	int numsegs;

	noVec3 tpos;
	noVec4 tcolor;
	float tabove, tbelow;

	TextureID texture;

	std::list<M2RibbonSegment> segs;	
public:
	M2Loader *model;

	void init(MPQFile &f, ModelRibbonEmitterDef &mta, uint32 *globals);
	void setup(size_t anim, size_t time);
	void draw();
};


#endif