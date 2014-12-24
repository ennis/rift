#include "boundingsphere.hpp"
#include <game.hpp>

BoundingSphere::BoundingSphere(glm::vec3 position, float radius)
	:_mesh(Game::renderer())
	{
		VolumeType() = SPHERE_TYPE;
		this->_radius = radius;
		getTransform().setPosition(position);

		//Initialization: Shader
		auto &renderer = Game::renderer();
		_shader = renderer.createShader(
			loadShaderSource("resources/shaders/sky/vert.glsl").c_str(),
			loadShaderSource("resources/shaders/sky/frag.glsl").c_str());

		//Initialization: mesh
		/*static const float boundingCuboidVertices[] = {
			-_dimensions.x, _dimensions.y, -_dimensions.z,
			-_dimensions.x, -_dimensions.y, -_dimensions.z,
			_dimensions.x, -_dimensions.y, -_dimensions.z,
			_dimensions.x, -_dimensions.y, -_dimensions.z,
			_dimensions.x, _dimensions.y, -_dimensions.z,
			-_dimensions.x, _dimensions.y, -_dimensions.z,

			-_dimensions.x, -_dimensions.y, _dimensions.z,
			-_dimensions.x, -_dimensions.y, -_dimensions.z,
			-_dimensions.x, _dimensions.y, -_dimensions.z,
			-_dimensions.x, _dimensions.y, -_dimensions.z,
			-_dimensions.x, _dimensions.y, _dimensions.z,
			-_dimensions.x, -_dimensions.y, _dimensions.z,

			_dimensions.x, -_dimensions.y, -_dimensions.z,
			_dimensions.x, -_dimensions.y, _dimensions.z,
			_dimensions.x, _dimensions.y, _dimensions.z,
			_dimensions.x, _dimensions.y, _dimensions.z,
			_dimensions.x, _dimensions.y, -_dimensions.z,
			_dimensions.x, -_dimensions.y, -_dimensions.z,

			-_dimensions.x, -_dimensions.y, _dimensions.z,
			-_dimensions.x, _dimensions.y, _dimensions.z,
			_dimensions.x, _dimensions.y, _dimensions.z,
			_dimensions.x, _dimensions.y, _dimensions.z,
			_dimensions.x, -_dimensions.y, _dimensions.z,
			-_dimensions.x, -_dimensions.y, _dimensions.z,

			-_dimensions.x, _dimensions.y, -_dimensions.z,
			_dimensions.x, _dimensions.y, -_dimensions.z,
			_dimensions.x, _dimensions.y, _dimensions.z,
			_dimensions.x, _dimensions.y, _dimensions.z,
			-_dimensions.x, _dimensions.y, _dimensions.z,
			-_dimensions.x, _dimensions.y, -_dimensions.z,

			-_dimensions.x, -_dimensions.y, -_dimensions.z,
			-_dimensions.x, -_dimensions.y, _dimensions.z,
			_dimensions.x, -_dimensions.y, -_dimensions.z,
			_dimensions.x, -_dimensions.y, -_dimensions.z,
			-_dimensions.x, -_dimensions.y, _dimensions.z,
			_dimensions.x, -_dimensions.y, _dimensions.z
		};*/

		//TODO
		/*Mesh::Attribute attribs[] = { { 0, ElementFormat::Float3 } };
		Mesh::Buffer buffers[] = { { ResourceUsage::Static } };
		const void *init[] = { boundingCuboidVertices };
		_mesh.allocate(PrimitiveType::Triangle, 1, attribs, 1, buffers, 36,
			init, 0, ElementFormat::Max, ResourceUsage::Static, nullptr);*/

}

float BoundingSphere::Radius() const{
	return _radius;
}

bool BoundingSphere::isColliding(BoundingVolume* target){
	//TODO
	return false;
}

void BoundingSphere::render(RenderContext const &renderContext){
	//TODO
}