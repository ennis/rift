#include "boundingcuboid.hpp"
#include <game.hpp>

BoundingCuboid::BoundingCuboid(glm::vec3 position, glm::vec3 dimensions)
	:_mesh(Game::renderer())
{
	VolumeType() = CUBOID_TYPE;
	this->_dimensions = dimensions;
	getTransform().setPosition(position);

	//Initialization: Shader
	auto &renderer = Game::renderer();
	_shader = renderer.createShader(
		loadShaderSource("resources/shaders/sky/vert.glsl").c_str(),
		loadShaderSource("resources/shaders/sky/frag.glsl").c_str());

	//Initialization: mesh
	static const float boundingCuboidVertices[] = {
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
	};

	Mesh::Attribute attribs[] = { { 0, ElementFormat::Float3 } };
	Mesh::Buffer buffers[] = { { ResourceUsage::Static } };
	const void *init[] = { boundingCuboidVertices };
	_mesh.allocate(PrimitiveType::Triangle, 1, attribs, 1, buffers, 36,
			init, 0, ElementFormat::Max, ResourceUsage::Static, nullptr);

}

glm::vec3 BoundingCuboid::Dimensions() const
{
	return _dimensions;
}

bool BoundingCuboid::isColliding(BoundingVolume* target)
{
	if (target->VolumeType() == CUBOID_TYPE){
		BoundingCuboid * c = dynamic_cast<BoundingCuboid *>(target);
		// A and B are corner of the two cuboids
		glm::vec3 A = this->getTransform().position - 0.5f * this->Dimensions();
		glm::vec3 B = c->getTransform().position - 0.5f * c->Dimensions();

		return (((A.x + this->Dimensions().x > B.x) && (A.x  < B.x + c->Dimensions().x)) //X colliding 
			&& ((A.y + this->Dimensions().y > B.y) && (A.y  < B.y + c->Dimensions().y)) //Y colliding 
			&& ((A.z + this->Dimensions().z > B.z) && (A.z  < B.z + c->Dimensions().z))); //Z colliding
	}
	else{
		// error
		return false; // pour ne pas avoir de warning
	}
}

void BoundingCuboid::render(RenderContext const &renderContext)
{
	using namespace glm;
	renderContext.renderer->setShader(_shader);
	renderContext.renderer->setConstantBuffer(0, renderContext.perFrameShaderParameters);
	_mesh.draw();
}