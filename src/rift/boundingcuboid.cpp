#include "boundingcuboid.hpp"
#include "boundingsphere.hpp"
#include "boundingcapsule.hpp"
#include <mesh_data.hpp>

BoundingCuboid::BoundingCuboid(glm::vec3 position, glm::vec3 dimensions, GraphicsContext &context)
	: BoundingVolume(context)
{
	VolumeType() = CUBOID_TYPE;
	this->_dimensions = dimensions;
	getTransform().setPosition(position);
	_last_position = position;

	//Initialization: mesh
	static float boundingCuboidMeshData[] = {
		/* pos */ -_dimensions.x / 2, -_dimensions.y / 2, +_dimensions.z / 2,
		/* pos */ +_dimensions.x / 2, -_dimensions.y / 2, +_dimensions.z / 2,
		/* pos */ +_dimensions.x / 2, +_dimensions.y / 2, +_dimensions.z / 2,
		/* pos */ -_dimensions.x / 2, +_dimensions.y / 2, +_dimensions.z / 2,
		/* pos */ -_dimensions.x / 2, -_dimensions.y / 2, -_dimensions.z / 2,
		/* pos */ +_dimensions.x / 2, -_dimensions.y / 2, -_dimensions.z / 2,
		/* pos */ +_dimensions.x / 2, +_dimensions.y / 2, -_dimensions.z / 2,
		/* pos */ -_dimensions.x / 2, +_dimensions.y / 2, -_dimensions.z / 2,
	};
	static uint16_t cubeIndices[] = {
		0, 1, 2, 2, 3, 0,
		3, 2, 6, 6, 7, 3,
		7, 6, 5, 5, 4, 7,
		4, 0, 3, 3, 7, 4,
		0, 5, 1, 5, 0, 4,
		1, 5, 6, 6, 2, 1
	};

	////Mesh Allocation
	//Renderer &rd = Engine::instance().getRenderer();
	//Mesh::Attribute attribs[] = { { 0, ElementFormat::Float3 } };
	//Mesh::Buffer buffers[] = { { ResourceUsage::Static } };
	//const void *init[] = { boundingCuboidMeshData };
	//_mesh.allocate(
	//	rd,
	//	PrimitiveType::Triangle,
	//	1,  // nb d'attribut par vertex
	//	attribs,
	//	1, // nb de vertex buffer
	//	buffers,
	//	8, // nb de vertex
	//	init,
	//	36, // nb d'indices (ie 12 triangles)
	//	ElementFormat::Uint16,
	//	ResourceUsage::Static,
	//	cubeIndices);

	////Initialization: shader
	//// on charge les sources des shaders
	//std::string vertexShaderSource = loadShaderSource("resources/shaders/bounding_volume/vert.glsl");
	//std::string fragmentShaderSource = loadShaderSource("resources/shaders/bounding_volume/frag.glsl");

	//_shader = rd.createShader(vertexShaderSource.c_str(), fragmentShaderSource.c_str());
}

glm::vec3 BoundingCuboid::Dimensions() const
{
	return _dimensions;
}

bool BoundingCuboid::isColliding(BoundingVolume* target, float & penetration_distance)
{
	if (target->VolumeType() == CUBOID_TYPE){ // TO REDO
		BoundingCuboid * c = static_cast<BoundingCuboid *>(target);
		// A and B are corner of the two cuboids
		glm::vec3 A = this->getTransform().position - 0.5f * this->Dimensions();
		glm::vec3 B = c->getTransform().position - 0.5f * c->Dimensions();

		return (((A.x + this->Dimensions().x >= B.x) && (A.x <= B.x + c->Dimensions().x)) //X colliding 
			&& ((A.y + this->Dimensions().y >= B.y) && (A.y <= B.y + c->Dimensions().y)) //Y colliding 
			&& ((A.z + this->Dimensions().z >= B.z) && (A.z <= B.z + c->Dimensions().z))); //Z colliding
	}
	else if (target->VolumeType() == SPHERE_TYPE){
		BoundingSphere * s = static_cast<BoundingSphere *>(target);

		glm::vec3 closestP = closestPoint(s->getTransform().position);
		return (glm::distance(closestP, s->getTransform().position) <= s->Radius());
	}
	else if (target->VolumeType() == CAPSULE_TYPE){
		// TODO
		return false;
	}
	else{
		// error
		return false; // pour ne pas avoir de warning
	}
}

//void BoundingCuboid::render(RenderContext const &renderContext, bool isColliding)
//{
//	Renderer &rd = Engine::instance().getRenderer();
//	rd.setShader(_shader); //the shader we use here
//
//	rd.setConstantBuffer(0, renderContext.perFrameShaderParameters);
//
//	rd.setNamedConstantFloat("lightIntensity", 1.0f);
//	if (isColliding){
//		rd.setNamedConstantFloat4("volumeColor", color_colliding);
//	}
//	else{
//		rd.setNamedConstantFloat4("volumeColor", color_not_colliding);
//	}
//
//	rd.setNamedConstantMatrix4("modelMatrix", getTransform().toMatrix());	// transformation dand l'espace
//
//	// draw
//	_mesh.draw();
//}

glm::vec3 BoundingCuboid::closestPoint(glm::vec3 origin){
	// On se place dans le repere du cuboid
	glm::vec3 origin2 = origin - getTransform().position;
	glm::vec3 proj = glm::vec3(
		glm::dot(origin2, getTransform().rotation * glm::vec3(1, 0, 0)),
		glm::dot(origin2, getTransform().rotation * glm::vec3(0, 1, 0)),
		glm::dot(origin2, getTransform().rotation * glm::vec3(0, 0, 1)));

	// On recherche le point le plus proche du point origin
	glm::vec3 res_proj;
	glm::vec3 dim = Dimensions();
	res_proj.x = (proj.x >= dim.x / 2) ? dim.x / 2 : ((proj.x <= -dim.x / 2) ? -dim.x / 2 : proj.x);
	res_proj.z = (proj.y >= dim.y / 2) ? dim.y / 2 : ((proj.y <= -dim.y / 2) ? -dim.y / 2 : proj.y);
	res_proj.x = (proj.z >= dim.z / 2) ? dim.z / 2 : ((proj.z <= -dim.z / 2) ? -dim.z / 2 : proj.z);

	// On retourne dans le repere de base
	glm::vec3 res = (-getTransform().rotation * res_proj) + getTransform().position;
	return res;
}