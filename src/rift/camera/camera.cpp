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

namespace
{
	const glm::vec3 CamFront = glm::vec3(0, 0, 1);
	const glm::vec3 CamRight = glm::vec3(1, 0, 0);
	const glm::vec3 CamUp = glm::vec3(0, 1, 0);
}

Camera TrackballCameraControl::updateCamera()
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

	if (curMode != Mode::Idle)
	{
		const double dx = (posx - lastMousePosX) * sensitivity;
		const double dy = (posy - lastMousePosY) * sensitivity;

		if (curMode == Mode::Rotate)
		{
			const double rot_speed = 1.0;
			sceneRotX = std::fmod(sceneRotX + rot_speed*dy, 2.0 * glm::pi<double>());
			sceneRotY = std::fmod(sceneRotY + rot_speed*dx, 2.0 * glm::pi<double>());
		}
		else
		{
			// pan
			const double pan_speed = 3.0;
			vEye += (float)(dx * pan_speed) * CamRight + (float)(dy * pan_speed) * CamUp;
		}
	}

	const double scroll = (window.scrollOffsetY - lastWheelOffset) * sensitivity;
	const double scroll_speed = 10.0;
	vEye += (float)(scroll * scroll_speed) * CamFront;

	lastWheelOffset = window.scrollOffsetY;
	lastMousePosX = posx;
	lastMousePosY = posy;

	Camera cam;
	cam.mode = Camera::Mode::Perspective;
	cam.viewMat =
		glm::lookAt(vEye, vEye + CamFront, CamUp) * 
		glm::rotate(glm::rotate((float)sceneRotX, CamRight), (float)sceneRotY, CamUp);
	cam.projMat = glm::perspective(fieldOfView, (float)aspect_ratio, nearPlane, farPlane);
	cam.wEye = glm::vec3(glm::inverse(cam.viewMat) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	return cam;
}