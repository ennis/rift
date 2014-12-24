#ifndef _BOUNDING_SPHERE_
#define _BOUNDING_SPHERE_

#include "boundingvolume.hpp"

class BoundingSphere : public BoundingVolume
{
public:
	// The position is the center of the sphere
	BoundingSphere(glm::vec3 position, float radius);

	float Radius() const;

	bool isColliding(BoundingVolume* target);

	void render(RenderContext const &renderContext);

private:
	float _radius;
	Mesh _mesh;
};

#endif