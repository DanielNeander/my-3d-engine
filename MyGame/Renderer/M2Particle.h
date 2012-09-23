#ifndef RENDERER_M2_PARTICLE_H
#define RENDERER_M2_PARTICLE_H

#include <Core/FreeListArray.h>
#include <Math/Vector.h>

class noM2ParticleSystem;
class noM2RibbonEmitter;

struct noM2Particle {
	noVec3 pos, speed, down, origin, dir;
	noVec3	corners[4];
	//Vec3D tpos;
	float size, life, maxlife;
	unsigned int tile;
	noVec4 color;	
};

typedef FreeListArray<noM2Particle> ParticleArray;

class noM2ParticleEmitter {
public:
	noM2ParticleEmitter(noM2ParticleSystem* sys): sys(sys) {}

protected:
	noM2ParticleSystem *sys;
	virtual noM2Particle newParticle(int anim, int time, float w, float l, float spd, float var, float spr, float spr2) = 0;	
};






#endif