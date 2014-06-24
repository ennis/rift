#ifndef FREECAMERACONTROL_HPP
#define FREECAMERACONTROL_HPP

#include <cameracontrol.hpp>

class FreeCameraControl : public CameraControl
{
public:
	FreeCameraControl() = default;
	void update(Camera &camera, Transform &transform, float dt);

private:
	void updateViewport(Camera &camera, glm::vec2 windowSize);

	glm::vec2 mLastMousePos = glm::vec2(0, 0);
	float mPhi = 0.f;
	float mTheta = 0.f;
};

#endif