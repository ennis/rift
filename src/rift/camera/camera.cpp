#include <camera.hpp>
#include <log.hpp>
#include <cmath>

/*void Camera::init()
{
}

void Camera::update(float dt)
{
}

glm::mat4 Camera::getProjectionMatrix() const
{
	if (mode == CameraMode::Perspective) {
		return glm::perspective(fov, aspectRatio, nearPlane, farPlane);
	}
	else {
		return glm::ortho(left, right, bottom, top, nearPlane, farPlane);
	}
}

glm::mat4 Camera::getViewMatrix() const 
{
	auto &transform = getEntity()->getTransform();
	return glm::lookAt(
		transform.position,
		transform.position + transform.rotation * Camera::Front,
		transform.rotation * Camera::Up);
}

void Camera::setAspectRatio(float aspectRatio)
{
	this->aspectRatio = aspectRatio;
}

const glm::vec3 Camera::Front = glm::vec3(0, 0, 1);
const glm::vec3 Camera::Right = glm::vec3(1, 0, 0);
const glm::vec3 Camera::Up = glm::vec3(0, 1, 0);
*/


glm::vec3 TrackballCameraControl::getRotationCenter() const
{
	return glm::vec3();
}

Camera TrackballCameraControl::getCamera()
{
	auto w = window.getHandle();
	int w_width, w_height;
	glfwGetWindowSize(w, &w_width, &w_height);
	const double aspect_ratio = (double)w_width / (double)w_height;

	// get mouse button state
	auto leftmb = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	auto middlemb = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
	
	double posx, posy;
	glfwGetCursorPos(w, &posx, &posy);

	if (!leftmb && !middlemb) {
		// no button pressed: idle mode
		curMode = Mode::Idle;
	} 
	else if (curMode != Mode::Rotate && leftmb) {
		// going to rotate mode
		curMode = Mode::Rotate;
		LOG << "Camera: Rotate mode";
		lastMousePosX = posx;
		lastMousePosY = posy;
	}
	else if (curMode != Mode::Pan && middlemb) {
		// going to panning mode
		curMode = Mode::Pan;
		LOG << "Camera: Panning mode";
		lastMousePosX = posx;
		lastMousePosY = posy;
	}

	const double dx = (posx - lastMousePosX) * sensitivity;
	const double dy = (posy - lastMousePosY) * sensitivity;

	// Update scene rotation
	if (curMode != Mode::Idle)
	{
		if (curMode == Mode::Rotate)
		{
			const double rot_speed = 1.0;
			sceneRotX = std::fmod(sceneRotX + rot_speed*dy, 2.0 * glm::pi<double>());
			sceneRotY = std::fmod(sceneRotY + rot_speed*dx, 2.0 * glm::pi<double>());
		}
	}

	auto lookAt = glm::lookAt(glm::vec3(0, 0, -2), glm::vec3(0, 0, 0), CamUp)
		* glm::rotate(glm::rotate((float)sceneRotX, CamRight), (float)sceneRotY, CamUp);

	// no rotation: move view center
	if (curMode != Mode::Rotate)
	{
		auto invLookAt = glm::inverse(lookAt);
		auto wCamRight = glm::vec3(invLookAt * glm::vec4(CamRight, 0.0f));
		auto wCamUp = glm::vec3(invLookAt * glm::vec4(-CamUp, 0.0f));
		auto wCamFront = glm::vec3(invLookAt * glm::vec4(CamFront, 0.0f));

		if (curMode == Mode::Pan)
		{
			const double pan_speed = 3.0;
			vEye += (float)(dx * pan_speed) * wCamRight + (float)(dy * pan_speed) * wCamUp;
		}
		else 
		{
			const double scroll = (window.scrollOffsetY - lastWheelOffset) * sensitivity;
			const double scroll_speed = 10.0;
			vEye += (float)(scroll * scroll_speed) * wCamFront;
		}
	}

	lastWheelOffset = window.scrollOffsetY;
	lastMousePosX = posx;
	lastMousePosY = posy;

	Camera cam;
	cam.mode = Camera::Mode::Perspective;
	cam.viewMat = lookAt * glm::translate(vEye);

	cam.projMat = glm::perspective(fieldOfView, (float)aspect_ratio, nearPlane, farPlane);
	cam.wEye = glm::vec3(glm::inverse(cam.viewMat) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	return cam;
}