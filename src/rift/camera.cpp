#include <camera.hpp>

void Camera::init()
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
