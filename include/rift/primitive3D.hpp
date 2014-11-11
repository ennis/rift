#ifndef _PRIMITIVE3D_
#define _PRIMITIVE3D_

#include <glm/glm.hpp>
#include "transform.hpp"

enum PrimitiveType{
	CUBOID_TYPE,
	SPHERE_TYPE,
	CAPSULE_TYPE
};

// 3D primitive : cuboid, sphere & capsule
class Primitive3D
{
public:
	PrimitiveType &Type()
		{return type;}
	PrimitiveType Type() const
		{return type;}

	Transform &getTransform()
		{return transform;}

	bool isMoving() const
		{return moving;}

	glm::vec3 &Speed()
		{return speed;}
	glm::vec3 Speed() const
		{return speed;}


	// return TRUE if this primitive is colliding the "target" primitive
	virtual bool isColliding(Primitive3D* target) = 0;

	// compute the collision with the target
	// makes sence only if the collision has been detected
	virtual void computeCollision(Primitive3D* target) = 0;

	virtual void render() = 0;

protected:
	bool moving; // TRUE if the primitive is expected to be mobile

private:
	PrimitiveType type;
	Transform transform;
	glm::vec3 speed; // speed vector
};

#endif