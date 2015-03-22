#include "stdafx.h"
#include "PhysicSystem.h"

#include <assert.h>
#include <map>

PhysicSystem::PhysicSystem()
{
	init();
}

void PhysicSystem::init()
{
	_bVolumes.clear();

	_toggleGravity = true;
	_toggleViscosity = true;
	_toggleCollisions = true;

	_gravity = glm::vec3(0, -9.81, 0);
	_mediumViscosity = 0.05f;

	_CoR = 1.f;
}


PhysicSystem::~PhysicSystem()
{
}


void PhysicSystem::addBoundingVolumes(std::vector<BoundingVolume*> volumes)
{
	_bVolumes.insert(_bVolumes.begin(),
		volumes.begin(), volumes.end());
}
void PhysicSystem::setTerrain(Terrain * t){
	_terrain = t;
}

void PhysicSystem::switchGravity(bool onOff){
	_toggleGravity = onOff;
}
void PhysicSystem::switchViscosity(bool onOff){
	_toggleViscosity = onOff;
}
void PhysicSystem::switchCollisionsDetection(bool onOff){
	_toggleCollisions = onOff;
}

void PhysicSystem::setGravity(glm::vec3 g){
	_gravity = g;
}
void PhysicSystem::setViscosity(double visc){
	_mediumViscosity = visc;
}

void PhysicSystem::update(float dt){
	//======== 0. Save the last coordinates
	std::vector<BoundingVolume *>::iterator itBV;
	for (itBV = _bVolumes.begin(); itBV != _bVolumes.end(); ++itBV) {
		BoundingVolume *bv = *itBV;
		bv->LastPosition() = bv->getTransform().position;
	}

	//======== 1. Compute all forces
	// map to accumulate the forces to apply on each boundingvolumes
	std::map<const BoundingVolume *, glm::vec3> forces;

	// weights
	if (_toggleGravity){
		for (itBV = _bVolumes.begin(); itBV != _bVolumes.end(); ++itBV) {
			BoundingVolume *bv = *itBV;
			forces[bv] = _gravity * bv->Mass();
		}
	}

	// global viscosity
	if (_toggleViscosity){
		for (itBV = _bVolumes.begin(); itBV != _bVolumes.end(); ++itBV) {
			BoundingVolume *bv = *itBV;
			forces[bv] -= _mediumViscosity * bv->Speed();
		}
	}

	//======== 2. Integration scheme
	// update boundingVolumes velocity
	for (itBV = _bVolumes.begin(); itBV != _bVolumes.end(); ++itBV) {
		BoundingVolume *bv = *itBV;
		bv->Speed() += dt * forces[bv] * bv->MassInv();
	}

	// update boundingVolumes positions
	for (itBV = _bVolumes.begin(); itBV != _bVolumes.end(); ++itBV) {
		BoundingVolume *bv = *itBV;
		// q = q + dt * v
		bv->getTransform().position += dt * bv->Speed();
	}

	//======== 3. Collisions
	if (_toggleCollisions){
		// TODO: ordre?

		// collision with the terrain
		for (itBV = _bVolumes.begin(); itBV != _bVolumes.end(); ++itBV) {
			collisionBVolumeTerrain(*itBV, _terrain);
		}

		// collision between boundingVolumes
		std::vector<BoundingVolume *>::iterator itBV2;
		for (itBV = _bVolumes.begin(); itBV != _bVolumes.end(); ++itBV) {
			for (itBV2 = std::next(itBV); itBV2 != _bVolumes.end(); ++itBV2) {
				collisionBVolumeBVolume(*itBV, *itBV2);
			}
		}
	}

}

// IMPORTANT: We suppose we use only spheres for now
void PhysicSystem::collisionBVolumeTerrain(BoundingVolume *bv, Terrain *terr){
	// We do not compute the collision if the BoundingVolume is fixed
	if (bv->isFixed()){
		return;
	}

	//TODO: calculer la distance de penetration par rapport à la normale ... ?

	//Test if the volume is penetrating the terrain
	glm::vec2 pos2D(bv->getTransform().position.x, bv->getTransform().position.z);
	float terrain_height;
	assert(_terrain->getHeight(pos2D, terrain_height) == false); // TODO : better error handle

	float penetration_distance;
	if (bv->VolumeType() == SPHERE_TYPE){
		BoundingSphere *s = static_cast<BoundingSphere *>(bv);
		penetration_distance = terrain_height - (s->getTransform().position.y - s->Radius());
	}

	// No penetration => stop here
	if (penetration_distance <= 0){
		return;
	}

	//We compute the normal to the terrain...
	//(direction : from the terrain to the boundingVolume)
	glm::vec3 normal;
	_terrain->getNormal(pos2D, normal);

	//We watch the direction of the normal
	float vertical_value = glm::dot(normal, glm::vec3(0, 1, 0));


	// We move the sphere so that we do not penetrate the terrain
	if (vertical_value >= 0.8){
		bv->getTransform().move(penetration_distance * normal);
	}
	else{
		bv->getTransform().position = bv->LastPosition();
	}

	//We compute the collision (slide or rebound...)
	float norm_speed = glm::dot(bv->Speed(), normal);
	bv->Speed() -= (1 + _CoR)*norm_speed * normal;
}

// For now: only spheres on spheres
void PhysicSystem::collisionBVolumeBVolume(BoundingVolume *bv1, BoundingVolume *bv2){

	// We do not compute the collision if both the BoundingVolumes are fixed
	if (bv1->isFixed() && bv2->isFixed()){
		return;
	}

	// If there is no collision, we stop
	float penetration_distance = 0;
	if (!bv1->isColliding(bv2, penetration_distance)){
		return;
	}

	// We compute the collision (slide or rebound...)
	if (bv1->VolumeType() == SPHERE_TYPE && bv2->VolumeType() == SPHERE_TYPE){
		BoundingSphere *s1 = static_cast<BoundingSphere *>(bv1);
		BoundingSphere *s2 = static_cast<BoundingSphere *>(bv2);

		s1->MakeCollision(s2, penetration_distance, _CoR);
	}

}