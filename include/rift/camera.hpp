#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <common.hpp>
#include <transform.hpp>
#include <entity.hpp>

enum class CameraMode
{
	Perspective,
	Orthographic
};

class Camera : public IComponent<CID_Camera>
{
public:
	Camera() = default;
	void init() override;
	void update(float dt) override;
	glm::mat4 getProjectionMatrix() const;
	glm::mat4 getViewMatrix() const;
	void setAspectRatio(float aspectRatio);
	static const glm::vec3 Front;
	static const glm::vec3 Right;
	static const glm::vec3 Up;
private:
	CameraMode mode = CameraMode::Perspective;
	// if mode == Perspective
	float aspectRatio = 1.f;
	float fov = 75.f;
	// if mode == Ortho
	float left;
	float right;
	float top;
	float bottom;
	float nearPlane = 0.1f;
	float farPlane = 100000.f;
};

#endif
