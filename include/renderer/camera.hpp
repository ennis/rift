#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <common.hpp>
#include <transform.hpp>

enum class CameraMode
{
	Perspective,
	Orthographic
};

struct Camera
{
	Camera() = default;

	void updateProjectionMatrix(Transform &transform);
	void updateViewMatrix(Transform &transform);

	CameraMode mode = CameraMode::Perspective;

	// if mode == Perspective
	float aspectRatio = 1.f;
	float fov = 75.f;

	// if mode == Ortho
	float left;
	float right;
	float top;
	float bottom;

	float nearPlane = 0.001f;
	float farPlane = 1000.f;

	// cached matrices
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;

	static const glm::vec3 Front;
	static const glm::vec3 Right;
	static const glm::vec3 Up;
};

#endif