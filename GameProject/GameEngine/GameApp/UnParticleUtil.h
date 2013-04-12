#ifndef RENDERER_PARTICLEHELPER_H
#define RENDERER_PARTICLEHELPER_H

#include <Math/Vector.h>
#include <Renderer/Color.h>

class noParticleEmitter;
class noParticleSpriteEmitter;
class noParticleEvent;

class noParticleEventDataBeam;
class noParticleEventDataBeam2;

class noParticleEventBeamSource;
class noParticleEventBeamTarget;
class noParticleEventBeamNoise;
class noParticleEventBeamModifier;

struct noBaseParticle {
	// 16 bytes
	noVec3			OldLocation;			// Last frame's location, used for collision
	FLOAT			RelativeTime;			// Relative time, range is 0 (==spawn) to 1 (==death)

	// 16 bytes
	noVec3			Location;				// Current location
	FLOAT			OneOverMaxLifetime;		// Reciprocal of lifetime

	// 16 bytes
	noVec3			BaseVelocity;			// Velocity = BaseVelocity at the start of each frame.
	FLOAT			Rotation;				// Rotation of particle (in Radians)

	// 16 bytes
	noVec3			Velocity;				// Current velocity, gets reset to BaseVelocity each frame to allow 
	FLOAT			BaseRotationRate;		// Initial angular velocity of particle (in Radians per second)

	// 16 bytes
	noVec3			BaseSize;				// Size = BaseSize at the start of each frame
	FLOAT			RotationRate;			// Current rotation rate, gets reset to BaseRotationRate each frame

	// 16 bytes
	noVec3			Size;					// Current size, gets reset to BaseSize each frame
	INT				Flags;					// Flags indicating various particle states

	// 16 bytes
	noLinearColor	Color;					// Current color of particle.

	// 16 bytes
	noLinearColor	BaseColor;				// Base color of the particle
};

#endif