#ifndef _BOUNDING_CUBOID_
#define _BOUNDING_CUBOID_

#include "boundingvolume.hpp"

class BoundingCuboid: public BoundingVolume
{
public:
	// The position is the center of the cube
	// The cube is alligned with the axis of the frame
	BoundingCuboid(glm::vec3 position, glm::vec3 dimensions, GraphicsContext &context);

	glm::vec3 Dimensions() const;

	bool isColliding(BoundingVolume* target, float & penetration_distance);

	//void render(RenderContext const &renderContext, bool isColliding);

	//Return the closest point of the cube to the point origin
	glm::vec3 closestPoint(glm::vec3 origin);

private:
	glm::vec3 _dimensions;

	//Mesh _mesh;
};

#endif