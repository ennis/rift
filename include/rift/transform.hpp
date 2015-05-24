#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

//
// représente une transformation (position, scaling, rotation)
struct Transform
{

	//
	// set scaling
	Transform &scale(float v) {
		scaling = glm::vec3(v);
		return *this;
	}

	//
	// set position
	Transform &setPosition(glm::vec3 const &v) {
		position = v;
		return *this;
	}

	//
	// add to position
	Transform &move(glm::vec3 const &v) {
		position += v;
		return *this;
	}

	//
	// set rotation
	Transform &rotate(float angle, glm::vec3 const &axis) {
		rotation = glm::angleAxis(angle, axis);
		return *this;
	}

	//
	// matrice 4x4 correspondante
	glm::mat4 toMatrix() const {
		return glm::translate(position) * glm::scale(scaling) * glm::toMat4(rotation);
	}

	//
	// matrice de transformation des normales
	glm::mat3 normalMatrix() {
		return glm::inverseTranspose(glm::mat3(toMatrix()));
	}

	glm::vec3 scaling = glm::vec3(1.f);
	glm::vec3 position = glm::vec3(0.f);
	glm::quat rotation;

	//
	// application à un point
	glm::vec3 applyPoint(glm::vec3 const &pos) {
		return position + rotation * scaling * pos;
	}

	//
	// application à un vecteur
	glm::vec3 applyVec(glm::vec3 const &vector) {
		return rotation * scaling * vector;
	}

	//
	// transformation d'une normale
	glm::vec3 applyNormal(glm::vec3 const &n) {
		glm::mat3 mat = glm::inverseTranspose(glm::mat3(toMatrix()));
		return mat * n;
	}

};

#endif