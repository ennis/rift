#ifndef CAMERACONTROL_HPP
#define CAMERACONTROL_HPP

#include <camera.hpp>
#include <transform.hpp>

class CameraControl
{
public:
	virtual void update(Camera &camera, Transform &transform, float dt) = 0;
};

#endif