#ifndef _BOUNDING_CAPSULE_
#define _BOUNDING_CAPSULE_

#include "boundingvolume.hpp"

// Warning: when creating a BoundingCapsule,
// make sure the length is at least twice the radius.
class BoundingCapsule : public BoundingVolume
{
public:
	// The position is the center of the capsule
	// The capsule is considered alligned with the z axis
	BoundingCapsule(glm::vec3 position, float radius, float length);

	float Radius() const;
	float Length() const;

	bool isColliding(BoundingVolume* target);

	void render(RenderContext const &renderContext, bool isColliding);

private:
	float _radius;
	float _length;

	Mesh _mesh;
};

#endif