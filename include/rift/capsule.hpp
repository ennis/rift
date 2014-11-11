#ifndef _CAPSULE_
#define _CAPSULE_

#include "primitive3D.hpp"

class Capsule : public Primitive3D{
public:
	// The position is the center
	Capsule(glm::vec3 position, float height, float radius, bool moving);

	float Height() const;
	float Radius() const;

	bool isColliding(Primitive3D* target);

	void computeCollision(Primitive3D* target);

	void render();

private:
	float height;
	float radius;
};

#endif