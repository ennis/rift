#include "cuboid.hpp"
#include "sphere.hpp"
#include "capsule.hpp"

Cuboid::Cuboid(glm::vec3 position, glm::vec3 dimensions, bool moving){
	Type() = CUBOID_TYPE;
	this->dimensions = dimensions;
	getTransform().setPosition(position);
	Speed() = glm::vec3(0, 0, 0);
	this->moving = moving;
}

glm::vec3 Cuboid::Dimensions() const{
	return dimensions;
}


inline float squared(float v) { return v * v; }

bool Cuboid::isColliding(Primitive3D* target){
	if (target->Type() == CUBOID_TYPE){
		Cuboid * c = dynamic_cast<Cuboid *>(target);
		glm::vec3 A = this->getTransform().position - 0.5f * this->Dimensions();
		glm::vec3 B = c->getTransform().position - 0.5f * c->Dimensions();

		return (((A.x + this->Dimensions().x > B.x) && (A.x  < B.x + c->Dimensions().x)) //X colliding 
			&& ((A.y + this->Dimensions().y > B.y) && (A.y  < B.y + c->Dimensions().y)) //Y colliding 
			&& ((A.z + this->Dimensions().z > B.z) && (A.z  < B.z + c->Dimensions().z))); //Z colliding
	}
	else if (target->Type() == SPHERE_TYPE){
		Sphere * s = dynamic_cast<Sphere *>(target);
		glm::vec3 C1 = this->getTransform().position - 0.5f * Dimensions(); // corner (0,0,0)
		glm::vec3 C2 = this->getTransform().position + 0.5f * Dimensions(); // corner (max_x, max_y, max_z)
		glm::vec3 s_center = s->getTransform().position; // sphere_center

		float dist_squared = squared(s->Radius());

		if (s_center.x < C1.x) dist_squared -= squared(s_center.x - C1.x);
		else if (s_center.x > C2.x) dist_squared -= squared(s_center.x - C2.x);
		if (s_center.y < C1.y) dist_squared -= squared(s_center.y - C1.y);
		else if (s_center.y > C2.y) dist_squared -= squared(s_center.y - C2.y);
		if (s_center.z < C1.z) dist_squared -= squared(s_center.z - C1.z);
		else if (s_center.z > C2.z) dist_squared -= squared(s_center.z - C2.z);

		return (dist_squared > 0);
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

// Find the closest point of the cuboid from the point "source"
glm::vec3 closestPointOnCuboid(glm::vec3 source, Cuboid *cub)
{
	glm::vec3 C1 = cub->getTransform().position - 0.5f * cub->Dimensions(); // corner (0,0,0)
	glm::vec3 C2 = cub->getTransform().position + 0.5f * cub->Dimensions(); // corner (max_x, max_y, max_z)

	glm::vec3 closestPoint;
	closestPoint.x = (source.x < C1.x) ? C1.x : (source.x > C2.x) ? C2.x : source.x;
	closestPoint.y = (source.y < C1.y) ? C1.y : (source.y > C2.y) ? C2.y : source.y;
	closestPoint.z = (source.z < C1.z) ? C1.z : (source.z > C2.z) ? C2.z : source.z;
	return closestPoint;
}

// Return +1 if f >=0
// Return -1 if f < 0
float getSign(float f){
	if (f >= 0){ return +1.f; }
	else{ return -1.f; }
}

void Cuboid::computeCollision(Primitive3D* target){
	if (target->Type() == CUBOID_TYPE){
		Cuboid * c = dynamic_cast<Cuboid *>(target);
		//TODO

		glm::vec3 toTarget = c->getTransform().position - this->getTransform().position;
		glm::vec3 signVec = glm::vec3(getSign(toTarget.x), getSign(toTarget.y), getSign(toTarget.z));

		// Multiple possible situations
		// Situation 1: only one corner is intersecting (another corner)

		// Corner of "this"
		glm::vec3 thisCorner = getTransform().position 
			+ 0.5f * glm::vec3(Dimensions().x*signVec.x, Dimensions().y*signVec.y, Dimensions().z*signVec.z);
		// Corner of "c"
		glm::vec3 cCorner = c->getTransform().position
			- 0.5f * glm::vec3(c->Dimensions().x*signVec.x, c->Dimensions().y*signVec.y, c->Dimensions().z*signVec.z);
		// We compute the diagonal of the intersection volume
		glm::vec3 penetrationDiag = cCorner - thisCorner;

		// Speed of penetration: actually we need the direction of moving
		// to know how the cuboid made contact.
		glm::vec3 penetrationSpeed;
		if (isMoving()){
			if (c->isMoving()){
				penetrationSpeed = c->Speed() - this->Speed(); // à tester...
			}
			else{
				penetrationSpeed = - this->Speed();
			}
		}
		else{
			if (c->isMoving()){
				penetrationSpeed = c->Speed();
			}
			else{
				// nothing to do
			}
		}

		// Cross product
		glm::vec3 cross = glm::cross(penetrationDiag, penetrationSpeed);

		// Comparison between the composants of the cross product
		// Which one has the lowest absolute value
		glm::vec3 response;
		if (std::abs(cross.x) <= std::abs(cross.y)){
			if (std::abs(cross.x) <= std::abs(cross.z)){
				// X is the lowest
				response = glm::vec3(-penetrationDiag.x, 0, 0);
			}
			else{ // (z < x) => z lowest
				response = glm::vec3(0, 0, -penetrationDiag.z);
			}
		}
		else{ // Y < X
			if (std::abs(cross.y) <= std::abs(cross.z)){
				// Y is the lowest
				response = glm::vec3(0, -penetrationDiag.y, 0);
			}
			else{ // (z < y) => z lowest
				response = glm::vec3(0, 0, -penetrationDiag.z);
			}
		}

		// Apply the response of the collision
		if (isMoving()){
			if (c->isMoving()){
				this->getTransform().move(-0.5f * response);
				c->getTransform().move(+0.5f * response);
			}
			else{
				this->getTransform().move(-response);
			}
		}
		else{
			if (c->isMoving()){
				c->getTransform().move(response);
			}
			else{
				// nothing to do
			}
		}

		// autres situations : a voir
		// soit deux coin penetrent => une arete
		// soit 4 coin penetrent => une face
		// plus => penetration complete

	}
	else if (target->Type() == SPHERE_TYPE){
		// /!\  Doesn't work very well if the object are colliding very fast,
		// thus intersecting each other strongly
		Sphere * s = dynamic_cast<Sphere *>(target);
		glm::vec3 sCenter = s->getTransform().position;
		
		glm::vec3 closestPoint = closestPointOnCuboid(sCenter, this);
		glm::vec3 planeNormal = glm::normalize(closestPoint - sCenter);
		float penetration_depth = std::abs(s->Radius() - glm::distance(sCenter, closestPoint));

		// Move the "moving" object along the normal
		if (isMoving()){
			if (s->isMoving()){
				// move both object half the measure
				this->getTransform().move(0.5f *penetration_depth * planeNormal);
				s->getTransform().move(-0.5f * penetration_depth * planeNormal);
			}
			else{
				// move "this"
				this->getTransform().move(penetration_depth * planeNormal);
			}
		}
		else{
			if (s->isMoving()){
				// move s
				s->getTransform().move(-penetration_depth * planeNormal);
			}
			else{
				// nothing to do, since both object can't move
			}
		}
	}
	else{
		// error
	}
}

void Cuboid::render(){
	//TODO
}