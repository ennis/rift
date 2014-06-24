#include <camera.hpp>

void Camera::updateProjectionMatrix(Transform &transform)
{
	if (mode == CameraMode::Perspective) {
		projMatrix = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
	}
	else {
		projMatrix = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
	}
}

void Camera::updateViewMatrix(Transform &transform)
{

	viewMatrix = glm::lookAt(
		transform.position,
		transform.position + transform.rotation * Camera::Front,
		transform.rotation * Camera::Up);
}

const glm::vec3 Camera::Front = glm::vec3(0, 0, 1);
const glm::vec3 Camera::Right = glm::vec3(1, 0, 0);
const glm::vec3 Camera::Up = glm::vec3(0, 1, 0);
