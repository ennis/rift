#ifndef _BOUNDING_VOLUME_
#define _BOUNDING_VOLUME_

#include <glm/glm.hpp>
#include <rendering/opengl4/mesh_renderer.hpp>
#include <rendering/opengl4/graphics_context.hpp>
#include "transform.hpp"

enum BoundingVolumeType{
	CUBOID_TYPE,
	SPHERE_TYPE,
	CAPSULE_TYPE
};

//3D volume used for collision detection
class BoundingVolume
{

protected:
	BoundingVolume(gl4::GraphicsContext &context): _graph_context(context){
		_speed = glm::vec3(0, 0, 0);
		_isFixed = false;
		_mass = 1.f;
		_mass_inverse = 1.f;
	}

public:
	~BoundingVolume()
	{
		//_shader->release();
	}

	BoundingVolumeType &VolumeType()
	{
		return _volume_type;
	}
	BoundingVolumeType VolumeType() const
	{
		return _volume_type;
	}

	Transform &getTransform()
	{
		return _transform;
	}
	glm::vec3 &Speed()
	{
		return _speed;
	}
	void setFixed(bool onOff)
	{
		_isFixed = onOff;
	}
	bool isFixed() const
	{
		return _isFixed;
	}
	float Mass() const{
		return _mass;
	}
	float MassInv() const{
		return _mass_inverse;
	}
	void setMass(float mass){
		_mass = mass;
		_mass_inverse = 1 / mass;
	}
	glm::vec3 &LastPosition(){
		return _last_position;
	}
	
	gl4::Mesh::Ptr getMesh(){
		return std::move(_mesh);
	}


	// return TRUE if this volume is colliding with "target" volume
	virtual bool isColliding(BoundingVolume* target, float & penetration_distance) = 0;

	//virtual void render(RenderContext const &renderContext, bool isColliding) = 0;

protected:
	//Shader *_shader;
	gl4::GraphicsContext &_graph_context;
	gl4::Mesh::Ptr _mesh;

	// for testing purposes...
	const glm::vec4 color_not_colliding = glm::vec4(0.0f, 1.0f, 0.0f, 0.3f);
	const glm::vec4 color_colliding = glm::vec4(1.0f, 0.0f, 0.0f, 0.3f);

	//for collision computation...
	glm::vec3 _last_position;

private:
	BoundingVolumeType _volume_type;
	Transform _transform;

	glm::vec3 _speed;
	bool _isFixed;
	float _mass;
	float _mass_inverse;

};

#endif