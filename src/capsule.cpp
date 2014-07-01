#include "capsule.hpp"
#include "cuboid.hpp"
#include "sphere.hpp"

Capsule::Capsule(glm::vec3 position, float height, float radius, bool moving){
	Type() = CAPSULE_TYPE;
	this->height = height;
	this->radius = radius;
	getTransform().setPosition(position);
	Speed() = glm::vec3(0, 0, 0);
	this->moving = moving;
}

float Capsule::Height() const{
	return height;
}
float Capsule::Radius() const{
	return radius;
}

bool Capsule::isColliding(Primitive3D* target){
	return false;
	// TODO
}

void Capsule::computeCollision(Primitive3D* target){
	// TODO
}

void Capsule::render(){
	// TODO
}