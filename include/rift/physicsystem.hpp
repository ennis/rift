#ifndef _PHYSIC_MANAGER_
#define _PHYSIC_MANAGER_

#include "boundingvolume.hpp"
#include "boundingsphere.hpp"
#include "boundingcuboid.hpp"
#include "boundingcapsule.hpp"

#include "terrain.hpp"
#include <vector>

/*
* This class represents a physic system, with a set of Boundingvolumes.
*/
class PhysicSystem
{
private:
	//system
	std::vector<BoundingVolume *> _bVolumes;
	Terrain * _terrain;

	// System parameters (common)
	//glm::vec3 _defaultGravity;
	glm::vec3 _gravity;			// gravity used in simulation
	//double _defaultMediumViscosity;
	float _mediumViscosity;		// viscosity used in simulation

	//Parameters shared by all boundingsvolumes
	float _CoR;	// coefficient_of_restition: 
	//0 = pure absorption; 1 = pure elastic impact

	// physic enabled or not
	bool _toggleGravity;
	bool _toggleViscosity;
	bool _toggleCollisions;

public:
	PhysicSystem();
	~PhysicSystem();

	// Add BoundingVolumes to the system
	void addBoundingVolumes(std::vector<BoundingVolume*> volumes);

	void setTerrain(Terrain * t);

	// Activate/desactivate gravity during the simulation
	void switchGravity(bool onOff);
	// Activate/desactivate viscosity during the simulation
	void switchViscosity(bool onOff);
	// Activate/desactivate contacts during the simulation
	void switchCollisionsDetection(bool onOff);

	void setGravity(glm::vec3 g);
	void setViscosity(double visc);

	// Physical animation of the volumes
	void update(float dt);

private:
	void init();

	void collisionBVolumeTerrain(BoundingVolume *bv, Terrain *terr);
	void collisionBVolumeBVolume(BoundingVolume *bv1, BoundingVolume *bv2);

};

#endif