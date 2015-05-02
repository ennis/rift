#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <common.hpp>
#include <transform.hpp>
#include <window.hpp>

struct Frustum
{
	float left;
	float right;
	float top;
	float bottom;
	// near clip plane position
	float nearPlane;
	// far clip plane position
	float farPlane;
};

// All that defines a camera (view matrix + frustum parameters)
struct Camera
{
	enum class Mode
	{
		Perspective,
		Orthographic
	};

	// TODO unproject, etc.

	// Camera mode 
	Mode mode;
	
	// Projection parameters
	// frustum (for culling)
	Frustum frustum;	
	// view matrix
	// (World -> View)
	glm::mat4 viewMat;
	// inverse view matrix
	// (View -> World)
	//glm::mat4 invViewMat;
	// projection matrix
	// (View -> clip?)
	glm::mat4 projMat;
	// Eye position in world space (camera center)
	glm::vec3 wEye;
};

class TrackballCameraControl
{
public:
	TrackballCameraControl(
		Window &window_, 
		glm::vec3 wEye_,
		float fieldOfView_,
		float nearPlane_,
		float farPlane_,
		double sensitivity_) : 
		window(window_),
		vEye(wEye_),
		fieldOfView(fieldOfView_),
		nearPlane(nearPlane_),
		farPlane(farPlane_),
		sensitivity(sensitivity_),
		sceneRotX(0.0),
		sceneRotY(0.0),
		lastWheelOffset(0.0),
		curMode(Mode::Idle),
		lastMousePosX(0.0),
		lastMousePosY(0.0)
		{}

	Camera updateCamera();

private:
	enum class Mode
	{
		Idle,
		Rotate,
		Pan
	};

	Window &window;
	// world coordinates after scene rotation
	glm::vec3 vEye;
	float fieldOfView;
	float nearPlane;
	float farPlane;
	double sensitivity;
	double sceneRotX;
	double sceneRotY;
	double lastWheelOffset;
	Mode curMode;
	double lastMousePosX;
	double lastMousePosY;
};

#endif
