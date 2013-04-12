#include "stdafx.h"
#include "cyclone_physics.h"

using namespace cyclone;

/*
 * --------------------------------------------------------------------------
 * FUNCTIONS DECLARED IN HEADER:
 * --------------------------------------------------------------------------
 */

void Particle::integrate(float duration)
{
    // We don't integrate things with zero mass.
    if (inverseMass <= 0.0f) return;

    assert(duration > 0.0);

    // Update linear position.    
	position = position + velocity * duration;

    // Work out the acceleration from the force
    noVec3 resultingAcc = acceleration;    
	resultingAcc = resultingAcc + forceAccum * inverseMass;

    // Update linear velocity from the acceleration.    
	velocity = velocity + resultingAcc * duration;

    // Impose drag.
    velocity *= pow(damping, duration);

    // Clear the forces.
    clearAccumulator();
}



void Particle::setMass(const float mass)
{
    assert(mass != 0);
    Particle::inverseMass = ((float)1.0)/mass;
}

float Particle::getMass() const
{
    if (inverseMass == 0) {
        return REAL_MAX;
    } else {
        return ((float)1.0)/inverseMass;
    }
}

void Particle::setInverseMass(const float inverseMass)
{
    Particle::inverseMass = inverseMass;
}

float Particle::getInverseMass() const
{
    return inverseMass;
}

bool Particle::hasFiniteMass() const
{
    return inverseMass >= 0.0f;
}

void Particle::setDamping(const float damping)
{
    Particle::damping = damping;
}

float Particle::getDamping() const
{
    return damping;
}

void Particle::setPosition(const noVec3 &position)
{
    Particle::position = position;
}

void Particle::setPosition(const float x, const float y, const float z)
{
    position.x = x;
    position.y = y;
    position.z = z;
}

void Particle::getPosition(noVec3 *position) const
{
    *position = Particle::position;
}

noVec3 Particle::getPosition() const
{
    return position;
}

void Particle::setVelocity(const noVec3 &velocity)
{
    Particle::velocity = velocity;
}

void Particle::setVelocity(const float x, const float y, const float z)
{
    velocity.x = x;
    velocity.y = y;
    velocity.z = z;
}

void Particle::getVelocity(noVec3 *velocity) const
{
    *velocity = Particle::velocity;
}

noVec3 Particle::getVelocity() const
{
    return velocity;
}

void Particle::setAcceleration(const noVec3 &acceleration)
{
    Particle::acceleration = acceleration;
}

void Particle::setAcceleration(const float x, const float y, const float z)
{
    acceleration.x = x;
    acceleration.y = y;
    acceleration.z = z;
}

void Particle::getAcceleration(noVec3 *acceleration) const
{
    *acceleration = Particle::acceleration;
}

noVec3 Particle::getAcceleration() const
{
    return acceleration;
}

void Particle::clearAccumulator()
{
    forceAccum = vec3_zero;
}

void Particle::addForce(const noVec3 &force)
{
    forceAccum += force;
}
