#include "stdafx.h"
#include "Model_M2Format.h"
#include "M2Particle.h"
#include "util.h"





noVec4 fromARGB(uint32 color)
{
	const float a = ((color & 0xFF000000) >> 24) / 255.0f;
	const float r = ((color & 0x00FF0000) >> 16) / 255.0f;
	const float g = ((color & 0x0000FF00) >>  8) / 255.0f;
	const float b = ((color & 0x000000FF)      ) / 255.0f;
	return noVec4(r,g,b,a);
}

template<class T>
T lifeRamp(float life, float mid, const T &a, const T &b, const T &c)
{
	if (life<=mid) 
		return interpolate<T>(life / mid,a,b);
	else 
		return interpolate<T>((life-mid) / (1.0f-mid),b,c);
}

M2ParticleSystem::M2ParticleSystem() : mid(0), emitter(0), rem(0)	
{
	blend = 0;
	order = 0;
	ParticleType = 0;
	manim = 0;
	mtime = 0;
	rows = 0;
	cols = 0;

	model = 0;
	parent = 0;
	texture = 0;

	slowdown = 0;
	rotation = 0;
	tofs = 0;
}

void M2ParticleSystem::init(MPQFile &f, ModelParticleEmitterDef &mta, uint32 *globals)
{
	speed.init (mta.EmissionSpeed, f, globals);
	variation.init (mta.SpeedVariation, f, globals);
	spread.init (mta.VerticalRange, f, globals);
	lat.init (mta.HorizontalRange, f, globals);
	gravity.init (mta.Gravity, f, globals);
	lifespan.init (mta.Lifespan, f, globals);
	rate.init (mta.EmissionRate, f, globals);
	areal.init (mta.EmissionAreaLength, f, globals);
	areaw.init (mta.EmissionAreaWidth, f, globals);
	deacceleration.init (mta.Gravity2, f, globals);
	enabled.init (mta.en, f, globals);

	if (gameVersion >= VERSION_WOTLK) {
		noVec3 colors2[3];
		memcpy(colors2, f.getBuffer()+mta.p.colors.ofsKeys, sizeof(noVec3)*3);
		for (size_t i=0; i<3; i++) {
			float opacity = *(short*)(f.getBuffer()+mta.p.opacity.ofsKeys+i*2);
			colors[i] = noVec4(colors2[i].x/255.0f, colors2[i].y/255.0f, colors2[i].z/255.0f, opacity/32767.0f);
			sizes[i] = (*(float*)(f.getBuffer()+mta.p.sizes.ofsKeys+i*sizeof(noVec2)))*mta.p.scales[i];		
		}
		mid = 0.5; // mid can't be 0 or 1, TODO, Alfred
	} else {
		for (size_t i=0; i<3; i++) {
			// colors[i] = fromARGB(mta.p.colors[i]);
			// sizes[i] = mta.p.sizes[i] * mta.p.scales[i];
		}
		// mid = mta.p.mid;
	}

	slowdown = mta.p.slowdown;
	rotation = mta.p.rotation;
	pos = fixCoordSystem(mta.pos);
	texture = model->textureIds[mta.texture];
	blend = mta.blend;
	rows = mta.rows;
	if (rows == 0)
		rows = 1;
	cols = mta.cols;
	if (cols == 0)
		cols = 1;
	ParticleType = mta.ParticleType;
	//order = mta.s2;
	order = mta.ParticleType>0 ? -1 : 0;
	parent = model->bones + mta.bone;

	//transform = mta.flags & 1024;

	// Type 2
	// 3145 = water ele
	// 1305 = water ele
	// 1049 = water elemental
	// 1033 = water elemental
	// 281 = water ele
	// 256 = Water elemental
	// 57 = Faith halo, ring?
	// 9 = water elemental

	billboard = !(mta.flags & MODELPARTICLE_FLAGS_DONOTBILLBOARD);

	// diagnosis test info
	EmitterType = mta.EmitterType;
	flags = mta.flags; // 0x10	Do not Trail

	manim = mtime = 0;
	rem = 0;

	emitter = 0;
	switch (EmitterType) {
	case MODELPARTICLE_EMITTER_PLANE:
		emitter = new M2PlaneParticleEmitter(this);
		break;
	case MODELPARTICLE_EMITTER_SPHERE:
		emitter = new M2SphereParticleEmitter(this);
		break;
	case MODELPARTICLE_EMITTER_SPLINE: // Spline? (can't be bothered to find one)
	default:
		wxLogMessage(wxT("[Error] Unknown Emitter: %d\n"), EmitterType);
		break;
	}

	tofs = frand();

	// init tiles, slice the texture
	for (size_t i=0; i<rows*cols; i++) {
		TexCoordSet tc;
		initTile(tc.tc, (int)i);
		tiles.push_back(tc);
	}	
}

void M2ParticleSystem::initTile(noVec2 *tc, int num)
{
	noVec2 otc[4];
	noVec2 a,b;
	int x = num % cols;
	int y = num / cols;
	a.x = x * (1.0f / cols);
	b.x = (x+1) * (1.0f / cols);
	a.y = y * (1.0f / rows);
	b.y = (y+1) * (1.0f / rows);

	otc[0] = a;
	otc[2] = b;
	otc[1].x = b.x;
	otc[1].y = a.y;
	otc[3].x = a.x;
	otc[3].y = b.y;

	for (size_t i=0; i<4; i++) {
		tc[(i+4-order) & 3] = otc[i];
	}
}

void M2ParticleSystem::update(float dt)
{
	size_t l_manim = manim;
	if (bZeroParticle)
		l_manim = 0;
	float grav = gravity.getValue(l_manim, mtime);
	float deaccel = deacceleration.getValue(l_manim, mtime);

	// spawn new M2Particles
	if (emitter) {
		float frate = rate.getValue(l_manim, mtime);
		float flife = lifespan.getValue(l_manim, mtime);

		float ftospawn;
		if (flife)
			ftospawn = (dt * frate / flife) + rem;
		else
			ftospawn = rem;
		if (ftospawn < 1.0f) {
			rem = ftospawn;
			if (rem < 0) 
				rem = 0;
		} else {
			unsigned int tospawn = (int)ftospawn;
						
			if ((tospawn + particles.size()) > MAX_PARTICLES) // Error check to prevent the program from trying to load insane amounts of M2Particles.
				tospawn = (unsigned int)(MAX_PARTICLES - particles.size());

			rem = ftospawn - (float)tospawn;

			float w = areal.getValue(l_manim, mtime) * 0.5f;
			float l = areaw.getValue(l_manim, mtime) * 0.5f;
			float spd = speed.getValue(l_manim, mtime);
			float var = variation.getValue(l_manim, mtime);
			float spr = spread.getValue(l_manim, mtime);
			float spr2 = lat.getValue(l_manim, mtime);
			bool en = true;
			if (enabled.uses(manim))
				en = enabled.getValue(manim, mtime)!=0;

			//rem = 0;
			if (en) {
				for (size_t i=0; i<tospawn; i++) {
					M2Particle ptc = emitter->newParticle(manim, mtime, w, l, spd, var, spr, spr2);
					// sanity check:
					if (particles.size() < MAX_PARTICLES) // No need to check this every loop iteration. Already checked above.					
					 	particles.push_back(ptc);
						
				}
			}
		}
	}

	float mspeed = 1.0f;

	for (ParticleList::iterator it =particles.begin(); it != particles.end(); ) {
	
		M2Particle &p = *it;
		p.speed += p.down * grav * dt - p.dir * deaccel * dt;

		if (slowdown>0) {
			mspeed = expf(-1.0f * slowdown * p.life);
		}
		p.pos += p.speed * mspeed * dt;

		p.life += dt;
		float rlife = p.life / p.maxlife;
		// calculate size and color based on lifetime
		p.size = lifeRamp<float>(rlife, mid, sizes[0], sizes[1], sizes[2]);
		p.color = lifeRamp<noVec4>(rlife, mid, colors[0], colors[1], colors[2]);

		// kill off old M2Particles
		if (rlife >= 1.0f) 
		{
			particles.erase(it++);
			
		}
		else 
			++it;
	}
}

void M2ParticleSystem::setup(size_t anim, size_t time)
{
	manim = anim;
	mtime = time;
}

void M2ParticleSystem::draw()
{
	


}

//Generates the rotation matrix based on spread
static noMat4	SpreadMat;
void CalcSpreadMatrix(float Spread1,float Spread2, float w, float l)
{
	int i,j;
	float a[2],c[2],s[2];
	noMat4	Temp;
	
	SpreadMat.Identity();
	
	a[0]=randfloat(-Spread1,Spread1)/2.0f;
	a[1]=randfloat(-Spread2,Spread2)/2.0f;
	
	/*SpreadMat.m[0][0]*=l;
	SpreadMat.m[1][1]*=l;
	SpreadMat.m[2][2]*=w;*/

	for(i=0;i<2;i++)
	{		
		c[i]=cos(a[i]);
		s[i]=sin(a[i]);
	}
	Temp.Identity();
	Temp.mat[1][1]=c[0];
	Temp.mat[2][1]=s[0];
	Temp.mat[2][2]=c[0];
	Temp.mat[1][2]=-s[0];

	SpreadMat=SpreadMat*Temp;

	Temp.Identity();
	Temp.mat[0][0]=c[1];
	Temp.mat[1][0]=s[1];
	Temp.mat[1][1]=c[1];
	Temp.mat[0][1]=-s[1];

	SpreadMat=SpreadMat*Temp;

	float Size=abs(c[0])*l+abs(s[0])*w;
	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			SpreadMat.mat[i][j]*=Size;
}

M2Particle M2PlaneParticleEmitter::newParticle(size_t anim, size_t time, float w, float l, float spd, float var, float spr, float spr2)
{
	// Model Flags - *shrug* gotta write this down somewhere.
	// 0x1 =
	// 0x2 =
	// 0x4 =
	// 0x8 = 
	// 0x10 = 
	// 19 = 0x13 = blue ball in thunderfury = should be billboarded?

	// M2Particle Flags
	// 0x0	/ 0		= Basilisk has no flags?
	// 0x1	/ 1		= Pretty much everything I know of except Basilisks have this flag..  Billboard?
	// 0x2	/ 2		=
	// 0x4	/ 4		=
	// 0x8  / 8		= 
	// 0x10	/ 16	= Position Relative to bone pivot?
	// 0x20	/ 32	=
	// 0x40	/ 64	=
	// 0x80 / 128	=
	// 0x100 / 256	=
	// 0x200 / 512	=
	// 0x400 / 1024 =
	// 0x800 / 2048 =
	// 0x1000/ 4096 =
	// 0x0000/ 1593 = [1,8,16,32,512,1024]"Warp Storm" - aura type M2Particle effect
	// 0x419 / 1049 = [1,8,16,1024] Forest Wind shoulders
	// 0x411 / 1041 = [1,16,1024] Halo
	// 0x000 / 541	= [1,4,8,16,512] Staff glow
	// 0x000 / 537 = "Warp Storm"
	// 0x31 / 49 = [1,16,32] M2Particle moving up?
	// 0x00 / 41 = [1,8,32] Blood elf broom, dust spread out on the ground (X, Z axis)
	// 0x1D / 29 = [1,4,8,16] M2Particle being static
	// 0x19 / 25 = [1,8,16] flame on weapon - move up/along the weapon
	// 17 = 0x11 = [1,16] glow on weapon - static, random direction.  - Aurastone Hammer
	// 1 = 0x1 = perdition blade
	// 4121 = water ele
	// 4097 = water elemental
	// 1041 = Transcendance Halo
	// 1039 = water ele

	M2Particle p;

	//Spread Calculation
	noMat4 mrot;

	CalcSpreadMatrix(spr,spr,1.0f,1.0f);
	mrot=sys->parent->mrot*SpreadMat;
	
	if (sys->flags == 1041) { // Trans Halo
		p.pos = sys->parent->mat * (sys->pos + noVec3(randfloat(-l,l), 0, randfloat(-w,w)));

		const float t = randfloat(0.0f, float(2*noMath::PI));

		p.pos = noVec3(0.0f, sys->pos.y + 0.15f, sys->pos.z) + noVec3(cos(t)/8, 0.0f, sin(t)/8); // Need to manually correct for the halo - why?
		
		// var isn't being used, which is set to 1.0f,  whats the importance of this?
		// why does this set of values differ from other M2Particles

		noVec3 dir(0.0f, 1.0f, 0.0f);
		p.dir = dir;

		noVec3 normal_dir = dir;
		normal_dir.Normalize();
		p.speed = normal_dir * spd * randfloat(0, var);
	} else if (sys->flags == 25 && sys->parent->parent<1) { // Weapon Flame
		float dot = sys->parent->pivot * (sys->pos + noVec3(randfloat(-l,l), randfloat(-l,l), randfloat(-w,w)));
		p.pos = noVec3(dot, 0.0f, 0.0f);
		noVec3 dir = mrot * noVec3(0.0f, 1.0f, 0.0f);
		noVec3 normal_dir = dir;
		normal_dir.Normalize();
		p.dir = normal_dir;
		//noVec3 dir = sys->model->bones[sys->parent->parent].mrot * sys->parent->mrot * noVec3(0.0f, 1.0f, 0.0f);
		//p.speed = dir.Normalize() * spd;

	} else if (sys->flags == 25 && sys->parent->parent > 0) { // Weapon with built-in Flame (Avenger lightsaber!)
		p.pos = sys->parent->mat * (sys->pos + noVec3(randfloat(-l,l), randfloat(-l,l), randfloat(-w,w)));
		float dot = noVec3(sys->parent->mat.mat[1][0],sys->parent->mat.mat[1][1], sys->parent->mat.mat[1][2]) * 
			noVec3(0.0f, 1.0f, 0.0f);
		noVec3 dir = noVec3(dot, 0.0f, 0.0f);
		noVec3 normal_dir = dir;
		normal_dir.Normalize();
		p.speed = normal_dir * spd * randfloat(0, var*2);

	} else if (sys->flags == 17 && sys->parent->parent<1) { // Weapon Glow
		float dot = sys->parent->pivot * (sys->pos + noVec3(randfloat(-l,l), randfloat(-l,l), randfloat(-w,w)));
		p.pos = noVec3(dot, 0.f, 0.f);
		noVec3 dir = mrot * noVec3(0,1,0);
		noVec3 normal_dir(dir);
		normal_dir.Normalize();
		p.dir = normal_dir;
		
	} else {
		p.pos = sys->pos + noVec3(randfloat(-l,l), 0, randfloat(-w,w));
		p.pos = sys->parent->mat * p.pos;

		//noVec3 dir = mrot * noVec3(0,1,0);
		noVec3 dir = sys->parent->mrot * noVec3(0,1,0);
		
		p.dir = dir;//.Normalize();
		p.down = noVec3(0,-1.0f,0); // dir * -1.0f;
		noVec3 normal_dir(dir);
		normal_dir.Normalize();
		p.speed = normal_dir * spd * (1.0f+randfloat(-var,var));
	}

	if(!sys->billboard)	{
		p.corners[0] = mrot * noVec3(-1,0,+1);
		p.corners[1] = mrot * noVec3(+1,0,+1);
		p.corners[2] = mrot * noVec3(+1,0,-1);
		p.corners[3] = mrot * noVec3(-1,0,-1);
	}

	p.life = 0;
	size_t l_anim = anim;
	if (bZeroParticle)
		l_anim = 0;
	p.maxlife = sys->lifespan.getValue(l_anim, time);
	if (p.maxlife == 0)
		p.maxlife = 1;

	p.origin = p.pos;

	p.tile = randint(0, sys->rows*sys->cols-1);
	return p;
}

M2Particle M2SphereParticleEmitter::newParticle(size_t anim, size_t time, float w, float l, float spd, float var, float spr, float spr2)
{
    M2Particle p;
	noVec3 dir;
	float radius;

	radius = randfloat(0,1);
	
	// Old method
	//float t = randfloat(0,2*noMath::PI);

	// New
	// Spread should never be zero for sphere M2Particles ?
	float t = 0;
	if (spr == 0)
		t = randfloat((float)-noMath::PI,(float)noMath::PI);
	else
		t = randfloat(-spr,spr);

	//Spread Calculation
	noMat4 mrot;

	CalcSpreadMatrix(spr*2,spr2*2,w,l);
	mrot=sys->parent->mrot*SpreadMat;

	// New
	// Length should never technically be zero ?
	//if (l==0)
	//	l = w;

	// New method
	// noVec3 bdir(w*cosf(t), 0.0f, l*sinf(t));
	// --

	// TODO: fix shpere emitters to work properly
	/* // Old Method
	//noVec3 bdir(l*cosf(t), 0, w*sinf(t));
	//noVec3 bdir(0, w*cosf(t), l*sinf(t));

	
	float theta_range = sys->spread.getValue(anim, time);
	float theta = -0.5f* theta_range + randfloat(0, theta_range);
	noVec3 bdir(0, l*cosf(theta), w*sinf(theta));

	float phi_range = sys->lat.getValue(anim, time);
	float phi = randfloat(0, phi_range);
	rotate(0,0, &bdir.z, &bdir.x, phi);
	*/

	if (sys->flags == 57 || sys->flags == 313) { // Faith Halo
		noVec3 bdir(w*cosf(t)*1.6, 0.0f, l*sinf(t)*1.6);

		p.pos = sys->pos + bdir;
		p.pos = sys->parent->mat * p.pos;

		if (bdir.LengthSqr()==0) 
			p.speed = noVec3(0,0,0);
		else {
			noVec3 normal_bdir = bdir;
			normal_bdir.Normalize();
			dir = sys->parent->mrot * normal_bdir;//mrot * noVec3(0, 1.0f,0);

			noVec3 normal_dir = dir;
			normal_dir.Normalize();
			p.speed = normal_dir * spd * (1.0f+randfloat(-var,var));   // ?
		}

	} else {
		noVec3 bdir;
		float temp;

		bdir = mrot * noVec3(0,1,0) * radius;
		temp = bdir.z;
		bdir.z = bdir.y;
		bdir.y = temp;

		p.pos = sys->parent->mat * sys->pos + bdir;
			

		//p.pos = sys->pos + bdir;
		//p.pos = sys->parent->mat * p.pos;
		

		if ((bdir.LengthSqr()==0) && ((sys->flags&0x100)!=0x100))
		{
			p.speed = noVec3(0,0,0);
			dir = sys->parent->mrot * noVec3(0,1,0);
		}
		else {
			if(sys->flags&0x100)
				dir = sys->parent->mrot * noVec3(0,1,0);
			else
			{
				noVec3 normal_bdir = bdir;
				normal_bdir.Normalize();
				dir = normal_bdir;
			}

			noVec3 normal_dir = dir;
			normal_dir.Normalize();
			p.speed = normal_dir * spd * (1.0f+randfloat(-var,var));   // ?
		}
	}

	noVec3 normal_dir = dir;
	normal_dir.Normalize();
	p.dir =  normal_dir;//mrot * noVec3(0, 1.0f,0);
	p.down = noVec3(0,-1.0f,0);

	p.life = 0;
	size_t l_anim = anim;
	if (bZeroParticle)
		l_anim = 0;
	p.maxlife = sys->lifespan.getValue(l_anim, time);
	if (p.maxlife == 0)
		p.maxlife = 1;

	p.origin = p.pos;

	p.tile = randint(0, sys->rows*sys->cols-1);
	return p;
}




void M2RibbonEmitter::init(MPQFile &f, ModelRibbonEmitterDef &mta, uint32 *globals)
{
	color.init(mta.color, f, globals);
	opacity.init(mta.opacity, f, globals);
	above.init(mta.above, f, globals);
	below.init(mta.below, f, globals);

	parent = model->bones + mta.bone;
	int *texlist = (int*)(f.getBuffer() + mta.ofsTextures);
	// just use the first texture for now; most models I've checked only had one
	texture = model->textureIds[texlist[0]];

	tpos = pos = fixCoordSystem(mta.pos);

	// TODO: figure out actual correct way to calculate length
	// in BFD, res is 60 and len is 0.6, the trails are very short (too long here)
	// in CoT, res and len are like 10 but the trails are supposed to be much longer (too short here)
	numsegs = (int)mta.res;
	seglen = mta.length;
	length = mta.res * seglen;

	// create first segment
	M2RibbonSegment rs;
	rs.pos = tpos;
	rs.len = 0;
	segs.push_back(rs);
}

void M2RibbonEmitter::setup(size_t anim, size_t time)
{
	noVec3 ntpos = parent->mat * pos;
	noVec3 ntup = parent->mat * (pos + noVec3(0,0,1));
	ntup -= ntpos;
	ntup.Normalize();
	float dlen = (ntpos-tpos).Length();

	manim = anim;
	mtime = time;

	// move first segment
	M2RibbonSegment &first = *segs.begin();
	if (first.len > seglen) {
		// add new segment
		noVec3 normalback = tpos-ntpos;
		normalback.Normalize();
		first.back = normalback;
		first.len0 = first.len;
		M2RibbonSegment newseg;
		newseg.pos = ntpos;
		newseg.up = ntup;
		newseg.len = dlen;
		segs.push_front(newseg);
	} else {
		first.up = ntup;
		first.pos = ntpos;
		first.len += dlen;
	}

	// kill stuff from the end
	float l = 0;
	bool erasemode = false;
	for (std::list<M2RibbonSegment>::iterator it = segs.begin(); it != segs.end(); ) {
		if (!erasemode) {
			l += it->len;
			if (l > length) {
				it->len = l - length;
				erasemode = true;
			}
			++it;
		} else {
			segs.erase(it++);
		}
	}

	tpos = ntpos;	
	tcolor = noVec4(color.getValue(anim, time), opacity.getValue(anim, time));
	
	tabove = above.getValue(anim, time);
	tbelow = below.getValue(anim, time);
}

void M2RibbonEmitter::draw()
{
	
}
