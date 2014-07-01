#include "sphere.hpp"
#include "cuboid.hpp"
#include "capsule.hpp"

Sphere::Sphere(glm::vec3 position, float radius, bool moving){
	Type() = SPHERE_TYPE;
	this->radius = radius;
	getTransform().setPosition(position);
	Speed() = glm::vec3(0, 0, 0);
	this->moving = moving;
}

float Sphere::Radius() const{
	return radius;
}

bool Sphere::isColliding(Primitive3D* target){
	if (target->Type() == CUBOID_TYPE){
		Cuboid * c = dynamic_cast<Cuboid *>(target);
		return c->isColliding(this);
	}
	else if (target->Type() == SPHERE_TYPE){
		Sphere * s = dynamic_cast<Sphere *>(target);
		float distance = glm::distance(getTransform().position,
			s->getTransform().position);
		return distance < (this->Radius() + s->Radius());
	}
	else if (target->Type() == CAPSULE_TYPE){
		//TODO
		return false;
	}
	else{
		// error
		return false; // pour ne pas avoir de warning
	}
}

void Sphere::computeCollision(Primitive3D* target){
	if (target->Type() == CUBOID_TYPE){
		Cuboid * c = dynamic_cast<Cuboid *>(target);
		c->computeCollision(this);
	}
	else if (target->Type() == SPHERE_TYPE){
		Sphere * s = dynamic_cast<Sphere *>(target);

		// profondeur of penetration ( >0, since there is penetration )
		float penetration_depth = (this->Radius() + s->Radius())
			- glm::distance(getTransform().position, s->getTransform().position);
		glm::vec3 normal = glm::normalize(getTransform().position - s->getTransform().position);

		// Move the "moving" spheres along the normal
		if (isMoving()){
			if (s->isMoving()){
				// move both sphere half the measure
				this->getTransform().move(0.5f *penetration_depth * normal);
				s->getTransform().move(-0.5f * penetration_depth * normal);
			}
			else{
				// move "this"
				this->getTransform().move(penetration_depth * normal);
			}
		}
		else{
			if (s->isMoving()){
				// move s
				s->getTransform().move(-penetration_depth * normal);
			}
			else{
				// nothing to do, since both spheres can't move
			}
		}
	}
	else{
		// error
	}
}


void Sphere::render(){
	//TODO
}