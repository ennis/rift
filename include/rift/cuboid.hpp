#ifndef _CUBOID_
#define _CUBOID_

#include "primitive3D.hpp"

class Cuboid : public Primitive3D{
public:
	// The position is the center of the cube
	// The cube is alligned with the axis of the frame
	Cuboid(glm::vec3 position, glm::vec3 dimensions, bool moving);

	glm::vec3 Dimensions() const;

	bool isColliding(Primitive3D* target);

	void computeCollision(Primitive3D* target);

	void render();

private:
	glm::vec3 dimensions;
};

#endif