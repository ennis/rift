#ifndef _BOUNDING_SPHERE_
#define _BOUNDING_SPHERE_

#include "boundingvolume.hpp"
#include <rendering/opengl4.hpp>


class BoundingSphere : public BoundingVolume
{
public:
	// The position is the center of the sphere
	BoundingSphere(glm::vec3 position, float radius, GraphicsContext &context);

	float Radius() const;

	bool isColliding(BoundingVolume* target, float & penetration_distance);

	void MakeCollision(BoundingVolume* target,
		const float penetration_distance, float CoR);

	//void render(RenderContext const &renderContext, bool isColliding);

private:
	float _radius;
	
};

#endif