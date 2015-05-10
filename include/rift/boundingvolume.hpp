#ifndef _BOUNDING_VOLUME_
#define _BOUNDING_VOLUME_

#include <glm/glm.hpp>
//#include <renderer.hpp>
//#include <renderable.hpp>
#include <mesh.hpp>
//#include <effect.hpp>
#include "transform.hpp"

enum BoundingVolumeType{
	CUBOID_TYPE,
	SPHERE_TYPE,
	CAPSULE_TYPE
};

//3D volume used for collision detection
class BoundingVolume
{
public:
	~BoundingVolume()
	{
		_shader->release();
	}

	BoundingVolumeType &VolumeType()
	{
		return _volume_type;
	}
	BoundingVolumeType VolumeType() const
	{
		return _volume_type;
	}

	Transform &getTransform()
	{
		return _transform;
	}

	// return TRUE if this volume is colliding with "target" volume
	virtual bool isColliding(BoundingVolume* target) = 0;

	virtual void render(RenderContext const &renderContext, bool isColliding) = 0;

protected:
	Shader *_shader;

	const glm::vec4 color_not_colliding = glm::vec4(0.0f, 1.0f, 0.0f, 0.3f);
	const glm::vec4 color_colliding = glm::vec4(1.0f, 0.0f, 0.0f, 0.3f);

private:
	BoundingVolumeType _volume_type;
	Transform _transform;

};

#endif