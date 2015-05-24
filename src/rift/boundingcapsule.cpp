#include "boundingcapsule.hpp"
#include <game.hpp>

BoundingCapsule::BoundingCapsule(glm::vec3 position, float radius, float length, gl4::GraphicsContext &context)
	:BoundingVolume(context)
{
	VolumeType() = CAPSULE_TYPE;
	this->_radius = radius;
	this->_length = length; // must be > 2 * radius
	getTransform().setPosition(position);
	_last_position = position;

	//Initialization: mesh
	const int nb_lines_sphere = 9; // must be >= 1, and of odd parity
	const int nb_lines_hemisphere = (nb_lines_sphere / 2) + 1;
	const int nb_vertices_per_line = nb_lines_sphere + 1;
	const int nb_vertices = 2 + nb_lines_hemisphere * 2 * nb_vertices_per_line;
	static float boundingCapsuleMeshData[nb_vertices * 3];

	const float cylinder_length = _length - 2 * _radius;
	// Creating south pole
	boundingCapsuleMeshData[0] = 0.0f;
	boundingCapsuleMeshData[1] = -_length / 2;
	boundingCapsuleMeshData[2] = 0.0f;
	// Creating north pole
	boundingCapsuleMeshData[3] = 0.0f;
	boundingCapsuleMeshData[4] = +_length / 2;
	boundingCapsuleMeshData[5] = 0.0f;
	// Vertices: South hemisphere
	for (int j = 1; j <= nb_lines_hemisphere; j++){
		float phi = glm::pi<float>() * ((-(float)1 / 2) + ((float)j / (nb_lines_sphere + 1)));
		for (int i = 0; i < nb_vertices_per_line; i++){
			float theta = 2 * glm::pi<float>() * (float)i / nb_vertices_per_line;
			int vertex_start = 6 + (j - 1)*nb_vertices_per_line * 3 + i * 3;
			boundingCapsuleMeshData[vertex_start] = _radius * glm::cos<float>(theta) * glm::cos<float>(phi);
			boundingCapsuleMeshData[vertex_start + 1] = -cylinder_length / 2 + glm::sin<float>(phi);
			boundingCapsuleMeshData[vertex_start + 2] = _radius * glm::sin<float>(theta) * glm::cos<float>(phi);
		}
	}
	// Vertices: North hemisphere
	for (int j = nb_lines_hemisphere; j <= nb_lines_sphere; j++){
		float phi = glm::pi<float>() * ((-(float)1 / 2) + ((float)j / (nb_lines_sphere + 1)));
		for (int i = 0; i < nb_vertices_per_line; i++){
			float theta = 2 * glm::pi<float>() * (float)i / nb_vertices_per_line;
			int vertex_start = 6 + j*nb_vertices_per_line * 3 + i * 3;
			boundingCapsuleMeshData[vertex_start] = _radius * glm::cos<float>(theta) * glm::cos<float>(phi);
			boundingCapsuleMeshData[vertex_start + 1] = +cylinder_length / 2 + glm::sin<float>(phi);
			boundingCapsuleMeshData[vertex_start + 2] = _radius * glm::sin<float>(theta) * glm::cos<float>(phi);
		}
	}

	const int nb_triangles = 2 * nb_vertices_per_line //coupoles (autour des poles)
		+ 4 * (nb_lines_hemisphere - 1)*nb_vertices_per_line //hemispheres
		+ 2 * nb_vertices_per_line; //cylindre
	static uint16_t capsuleIndices[3 * nb_triangles];
	//indices de la coupole sud
	for (int k = 0; k < nb_vertices_per_line; k++){
		capsuleIndices[k * 3] = 2 + k;
		capsuleIndices[k * 3 + 1] = 0;
		capsuleIndices[k * 3 + 2] = 2 + ((k + 1) % nb_vertices_per_line);
	}
	//indices de la coupole nord
	int base_indice = nb_vertices_per_line;
	for (int k = 0; k < nb_vertices_per_line; k++){
		capsuleIndices[(base_indice + k) * 3] = 1;
		capsuleIndices[(base_indice + k) * 3 + 1] = 2 + (nb_lines_hemisphere * 2 - 1)*nb_vertices_per_line + k;
		capsuleIndices[(base_indice + k) * 3 + 2] = 2 + (nb_lines_hemisphere * 2 - 1)*nb_vertices_per_line + ((k + 1) % nb_vertices_per_line);
	}
	//indices du reste de la demi-sphere sud
	for (int l = 1; l < nb_lines_hemisphere; l++){
		for (int k = 0; k < nb_vertices_per_line; k++){
			capsuleIndices[(nb_vertices_per_line * 2 * l + 2 * k) * 3] = 2 + nb_vertices_per_line*(l - 1) + k;
			capsuleIndices[(nb_vertices_per_line * 2 * l + 2 * k) * 3 + 1] = 2 + nb_vertices_per_line*(l - 1) + ((k + 1) % nb_vertices_per_line);
			capsuleIndices[(nb_vertices_per_line * 2 * l + 2 * k) * 3 + 2] = 2 + nb_vertices_per_line* l + k;

			capsuleIndices[(nb_vertices_per_line * 2 * l + 2 * k) * 3 + 3] = 2 + nb_vertices_per_line*(l - 1) + ((k + 1) % nb_vertices_per_line);
			capsuleIndices[(nb_vertices_per_line * 2 * l + 2 * k) * 3 + 4] = 2 + nb_vertices_per_line*l + ((k + 1) % nb_vertices_per_line);
			capsuleIndices[(nb_vertices_per_line * 2 * l + 2 * k) * 3 + 5] = 2 + nb_vertices_per_line* l + k;
		}
	}
	//indices du corps du cylindre
	base_indice = nb_vertices_per_line * 2 * nb_lines_hemisphere;
	for (int k = 0; k < nb_vertices_per_line; k++){
		capsuleIndices[(base_indice + 2 * k) * 3] = 2 + nb_vertices_per_line*(nb_lines_hemisphere - 1) + k;
		capsuleIndices[(base_indice + 2 * k) * 3 + 1] = 2 + nb_vertices_per_line*(nb_lines_hemisphere - 1) + ((k + 1) % nb_vertices_per_line);
		capsuleIndices[(base_indice + 2 * k) * 3 + 2] = 2 + nb_vertices_per_line* nb_lines_hemisphere + k;

		capsuleIndices[(base_indice + 2 * k) * 3 + 3] = 2 + nb_vertices_per_line*(nb_lines_hemisphere - 1) + ((k + 1) % nb_vertices_per_line);
		capsuleIndices[(base_indice + 2 * k) * 3 + 4] = 2 + nb_vertices_per_line*nb_lines_hemisphere + ((k + 1) % nb_vertices_per_line);
		capsuleIndices[(base_indice + 2 * k) * 3 + 5] = 2 + nb_vertices_per_line* nb_lines_hemisphere + k;
	}
	//indices du reste de la demi-sphere nord
	for (int l = 1; l < nb_lines_hemisphere; l++){
		for (int k = 0; k < nb_vertices_per_line; k++){
			capsuleIndices[(base_indice + nb_vertices_per_line * 2 * l + 2 * k) * 3] = 2 + nb_vertices_per_line*(nb_lines_hemisphere + l - 1) + k;
			capsuleIndices[(base_indice + nb_vertices_per_line * 2 * l + 2 * k) * 3 + 1] = 2 + nb_vertices_per_line*(nb_lines_hemisphere + l - 1) + ((k + 1) % nb_vertices_per_line);
			capsuleIndices[(base_indice + nb_vertices_per_line * 2 * l + 2 * k) * 3 + 2] = 2 + nb_vertices_per_line* (nb_lines_hemisphere + l) + k;

			capsuleIndices[(base_indice + nb_vertices_per_line * 2 * l + 2 * k) * 3 + 3] = 2 + nb_vertices_per_line*(l + nb_lines_hemisphere - 1) + ((k + 1) % nb_vertices_per_line);
			capsuleIndices[(base_indice + nb_vertices_per_line * 2 * l + 2 * k) * 3 + 4] = 2 + nb_vertices_per_line*(l + nb_lines_hemisphere) + ((k + 1) % nb_vertices_per_line);
			capsuleIndices[(base_indice + nb_vertices_per_line * 2 * l + 2 * k) * 3 + 5] = 2 + nb_vertices_per_line* (l + nb_lines_hemisphere) + k;
		}
	}
	
	////Mesh Allocation
	//Renderer &rd = Engine::instance().getRenderer();
	//Mesh::Attribute attribs[] = { { 0, ElementFormat::Float3 } };
	//Mesh::Buffer buffers[] = { { ResourceUsage::Static } };
	//const void *init[] = { boundingCapsuleMeshData };
	//_mesh.allocate(
	//	rd,
	//	PrimitiveType::Triangle,
	//	1,  // nb d'attribut par vertex
	//	attribs,
	//	1, // nb de vertex buffer
	//	buffers,
	//	nb_vertices, // nb de vertex
	//	init,
	//	3 * nb_triangles, // nb d'indices
	//	ElementFormat::Uint16,
	//	ResourceUsage::Static,
	//	capsuleIndices);

	////Initialization: shader
	//std::string vertexShaderSource = loadShaderSource("resources/shaders/bounding_volume/vert.glsl");
	//std::string fragmentShaderSource = loadShaderSource("resources/shaders/bounding_volume/frag.glsl");
	//_shader = rd.createShader(vertexShaderSource.c_str(), fragmentShaderSource.c_str());
}

float BoundingCapsule::Radius() const
{
	return _radius;
}
float BoundingCapsule::Length() const
{
	return _length;
}

bool BoundingCapsule::isColliding(BoundingVolume* target, float & penetration_distance){
	if (target->VolumeType() == CUBOID_TYPE){
		return target->isColliding(this,penetration_distance);
	}
	else if (target->VolumeType() == SPHERE_TYPE){
		return target->isColliding(this, penetration_distance);
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

//void BoundingCapsule::render(RenderContext const &renderContext, bool isColliding)
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