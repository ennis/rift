#include "boundingcapsule.hpp"
#include <mesh_data.hpp>

BoundingCapsule::BoundingCapsule(glm::vec3 position, float radius, float length, GraphicsContext &context)
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
	boundingCapsuleMeshData.resize(nb_vertices);

	const float cylinder_length = _length - 2 * _radius;
	// Creating south pole
	boundingCapsuleMeshData[0] = glm::vec3(0.0f, -(_length) / 2 - radius, 0.0f);
	// Creating north pole
	boundingCapsuleMeshData[1] = glm::vec3(0.0f, (_length) / 2 + radius, 0.0f);
	// Vertices: South hemisphere
	for (int j = 1; j <= nb_lines_hemisphere; j++){
		float phi = glm::pi<float>() * ((-(float)1 / 2) + ((float)j / (nb_lines_sphere + 1)));
		for (int i = 0; i < nb_vertices_per_line; i++){
			float theta = 2 * glm::pi<float>() * (float)i / nb_vertices_per_line;
			int vertex_start = 2 + (j - 1)*nb_vertices_per_line + i;
			boundingCapsuleMeshData[vertex_start] = glm::vec3(
				_radius * glm::cos<float>(theta) * glm::cos<float>(phi),
				-_length / 2 + radius * glm::sin<float>(phi),
				_radius * glm::sin<float>(theta) * glm::cos<float>(phi));
		}
	}
	// Vertices: North hemisphere
	for (int j = nb_lines_hemisphere; j <= nb_lines_sphere; j++){
		float phi = glm::pi<float>() * ((-(float)1 / 2) + ((float)j / (nb_lines_sphere + 1)));
		for (int i = 0; i < nb_vertices_per_line; i++){
			float theta = 2 * glm::pi<float>() * (float)i / nb_vertices_per_line;
			int vertex_start = 2 + j*nb_vertices_per_line + i;
			boundingCapsuleMeshData[vertex_start] = glm::vec3(
				_radius * glm::cos<float>(theta) * glm::cos<float>(phi),
				+_length / 2 + radius * glm::sin<float>(phi),
				_radius * glm::sin<float>(theta) * glm::cos<float>(phi));
		}
	}

	const int nb_triangles = 2 * nb_vertices_per_line //coupoles (autour des poles)
		+ 4 * (nb_lines_hemisphere - 1)*nb_vertices_per_line //hemispheres
		+ 2 * nb_vertices_per_line; //cylindre
	capsuleIndices.resize(3 * nb_triangles);
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

void BoundingCapsule::render(SceneRenderer &sceneRenderer)
{
	// is that simple enough?
	sceneRenderer.drawWireMesh(
		getTransform(),
		gl::TRIANGLES,
		boundingCapsuleMeshData,
		capsuleIndices,
		glm::vec4(0.4, 1.0, 0.4, 1.0),
		false);
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