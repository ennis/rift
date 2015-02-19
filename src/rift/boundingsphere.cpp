#include "boundingsphere.hpp"
#include "boundingcapsule.hpp"
#include <game.hpp>

BoundingSphere::BoundingSphere(glm::vec3 position, float radius)
{
		VolumeType() = SPHERE_TYPE;
		this->_radius = radius;
		getTransform().setPosition(position);

		//Initialization: mesh
		const int nb_lines = 10; // must be >= 1
		const int nb_vertices_per_line = nb_lines + 1;
		const int nb_vertices = 2 + nb_lines * nb_vertices_per_line;
		static float boundingSphereMeshData[nb_vertices * 3];

		// Creating south pole
		boundingSphereMeshData[0] = 0.0f;
		boundingSphereMeshData[1] = -1.0f;
		boundingSphereMeshData[2] = 0.0f;
		// Creating north pole
		boundingSphereMeshData[3] = 0.0f;
		boundingSphereMeshData[4] = +1.0f;
		boundingSphereMeshData[5] = 0.0f;
		// Other vertices: from south to north
		for (int j = 1; j <= nb_lines; j++){
			float phi = glm::pi<float>() * ((- (float)1 / 2) + ((float)j / (nb_lines+1)));
			for (int i = 0; i < nb_vertices_per_line; i++){
				float theta = 2*glm::pi<float>() * (float)i / nb_vertices_per_line;
				int vertex_start = 6 + (j-1)*nb_vertices_per_line * 3 + i * 3;
				boundingSphereMeshData[vertex_start] = glm::cos<float>(theta) * glm::cos<float>(phi);
				boundingSphereMeshData[vertex_start + 1] = glm::sin<float>(phi);
				boundingSphereMeshData[vertex_start + 2] = glm::sin<float>(theta) * glm::cos<float>(phi);
			}
		}

		const int nb_triangles = 2 * nb_vertices_per_line + 2 * (nb_lines - 1)*nb_vertices_per_line;
		static uint16_t sphereIndices[3 * nb_triangles];
		//indices de la coupole sud
		for (int k = 0; k < nb_vertices_per_line; k++){
			sphereIndices[k*3] = 2 + k;
			sphereIndices[k*3 +1] = 0;
			sphereIndices[k*3 +2] = 2 + ((k+1) % nb_vertices_per_line);
		}

		//indices de la coupole nord
		for (int k = 0; k < nb_vertices_per_line; k++){
			sphereIndices[(nb_vertices_per_line + k )* 3] = 1;
			sphereIndices[(nb_vertices_per_line + k )* 3 + 1] = 2 + (nb_lines-1)*nb_vertices_per_line + k;
			sphereIndices[(nb_vertices_per_line + k )* 3 + 2] = 2 +(nb_lines - 1)*nb_vertices_per_line + ((k + 1) % nb_vertices_per_line);
		}

		//indices du corps de la sphere
		for (int l = 1; l < nb_lines; l++){
			for (int k = 0; k < nb_vertices_per_line; k++){
				sphereIndices[(nb_vertices_per_line * 2 * l + 2 * k )* 3] = 2 + nb_vertices_per_line*(l-1) +k;
				sphereIndices[(nb_vertices_per_line * 2 * l + 2 * k) * 3 + 1] = 2 + nb_vertices_per_line*(l - 1) + ((k + 1) % nb_vertices_per_line);
				sphereIndices[(nb_vertices_per_line * 2 * l + 2 * k) * 3 + 2] = 2 + nb_vertices_per_line* l + k;

				sphereIndices[(nb_vertices_per_line * 2 * l + 2 * k) * 3 + 3] = 2 + nb_vertices_per_line*(l - 1) + ((k + 1) % nb_vertices_per_line);
				sphereIndices[(nb_vertices_per_line * 2 * l + 2 * k) * 3 + 4] = 2 + nb_vertices_per_line*l + ((k + 1) % nb_vertices_per_line);
				sphereIndices[(nb_vertices_per_line * 2 * l + 2 * k) * 3 + 5] = 2 + nb_vertices_per_line* l + k;
			}
		}

		//Mesh Allocation
		Renderer &rd = Engine::instance().getRenderer();
		Mesh::Attribute attribs[] = { { 0, ElementFormat::Float3 } };
		Mesh::Buffer buffers[] = { { ResourceUsage::Static } };
		const void *init[] = { boundingSphereMeshData };
		_mesh.allocate(
			rd,
			PrimitiveType::Triangle,
			1,  // nb d'attribut par vertex
			attribs,
			1, // nb de vertex buffer
			buffers,
			nb_vertices, // nb de vertex
			init,
			3 * nb_triangles, // nb d'indices
			ElementFormat::Uint16,
			ResourceUsage::Static,
			sphereIndices);

		//Initialization: shader
		std::string vertexShaderSource = loadShaderSource("resources/shaders/bounding_volume/vert.glsl");
		std::string fragmentShaderSource = loadShaderSource("resources/shaders/bounding_volume/frag.glsl");
		_shader = rd.createShader(vertexShaderSource.c_str(), fragmentShaderSource.c_str());
}

float BoundingSphere::Radius() const
{
	return _radius;
}

bool BoundingSphere::isColliding(BoundingVolume* target)
{
	if (target->VolumeType() == CUBOID_TYPE){
		return target->isColliding(this);
	}
	else if (target->VolumeType() == SPHERE_TYPE){ // OK
		BoundingSphere * s = dynamic_cast<BoundingSphere *>(target);
		//Compute the distance between the two centers
		float distance = glm::distance(this->getTransform().position, s->getTransform().position);
		return (distance <= (s->Radius() + this->Radius()));
	}
	else if (target->VolumeType() == CAPSULE_TYPE){
		BoundingCapsule * cap = dynamic_cast<BoundingCapsule *>(target);

		glm::vec3 s_center = this->getTransform().position;
		glm::vec3 cap_center = cap->getTransform().position;
		glm::vec3 s_center2 = s_center - cap_center;
		glm::vec3 cap_direction = cap->getTransform().rotation * glm::vec3(0, 1, 0);

		// On regarde la position du projeté du centre de la sphere 
		// dans le "repere axial" de la capsule (selon sa direction)
		float proj_abs = glm::dot(s_center2, cap_direction);
		const float cylinder_half = cap->Length()/2 - cap->Radius();
		if (proj_abs >= +cylinder_half){
			return (glm::distance(cap_center + cylinder_half * cap_direction, s_center) <= cap->Radius());
		}
		else if (proj_abs <= -cylinder_half){
			return (glm::distance(cap_center - cylinder_half * cap_direction, s_center) <= cap->Radius());
		}
		else{ // entre les deux extremités du corps du cylindre
			glm::vec3 normal_to_sphere = s_center2 - proj_abs * cap_direction;
			return normal_to_sphere.length() <= cap->Radius();
		}
	}
	else{
		// error
		return false; // pour ne pas avoir de warning
	}
}

void BoundingSphere::render(RenderContext const &renderContext, bool isColliding)
{
	Renderer &rd = Engine::instance().getRenderer();
	rd.setShader(_shader); //the shader we use here

	rd.setConstantBuffer(0, renderContext.perFrameShaderParameters);

	rd.setNamedConstantFloat("lightIntensity", 1.0f);
	if (isColliding){
		rd.setNamedConstantFloat4("volumeColor", color_colliding);
	}
	else{
		rd.setNamedConstantFloat4("volumeColor", color_not_colliding);
	}
	
	getTransform().scale(_radius); // Scaling with the radius
	rd.setNamedConstantMatrix4("modelMatrix", getTransform().toMatrix());	// transformation dand l'espace

	// draw
	_mesh.draw();
}