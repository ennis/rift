#ifndef _SPHERE_
#define _SPHERE_

#include "primitive3D.hpp"

class Sphere : public Primitive3D{
public:
	// The position is at the center of the sphere
	Sphere(glm::vec3 position, float radius, bool moving);

	float Radius() const;

	bool isColliding(Primitive3D* target);

	void computeCollision(Primitive3D* target);

	void render();

private:
	float radius;
};

#endif