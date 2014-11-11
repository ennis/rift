#ifndef FREECAMERACONTROL_HPP
#define FREECAMERACONTROL_HPP

#include <camera.hpp>
#include <entity.hpp>

class FreeCameraController : public IComponent<CID_CameraController>
{
public:
	FreeCameraController()
	{}

	void init() override;
	void update(float dt) override;

private:
	void updateViewport(Camera *camera, glm::vec2 windowSize);

	Entity *mCameraEntity;
	Camera *mCamera;
	glm::vec2 mLastMousePos = glm::vec2(0, 0);
	float mPhi = 0.f;
	float mTheta = 0.f;
};

#endif