#include "boundingsphere.hpp"
#include "boundingcapsule.hpp"
#include <game.hpp>

BoundingSphere::BoundingSphere(glm::vec3 position, float radius)
{
		VolumeType() = SPHERE_TYPE;
		this->_radius = radius;
		getTransform().setPosition(position);
		_last_position = position;

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

bool BoundingSphere::isColliding(BoundingVolume* target, float & penetration_distance)
{
	if (target->VolumeType() == CUBOID_TYPE){
		return target->isColliding(this, penetration_distance);
	}
	else if (target->VolumeType() == SPHERE_TYPE){ // OK
		BoundingSphere * s = static_cast<BoundingSphere *>(target);
		//Compute the distance between the two centers
		float distance = glm::distance(this->getTransform().position, s->getTransform().position);
		penetration_distance = 0.5f * ((s->Radius() + this->Radius()) - distance);
		return (penetration_distance > 0);
	}
	else if (target->VolumeType() == CAPSULE_TYPE){
		BoundingCapsule * cap = static_cast<BoundingCapsule *>(target);

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

void BoundingSphere::MakeCollision(BoundingVolume* target, const float penetration_distance, float CoR){
	if (target->VolumeType() == CUBOID_TYPE){
		//return target->MakeCollision(this, penetration_distance, rebound);
		//TODO
		return;
	}
	else if (target->VolumeType() == SPHERE_TYPE){
		BoundingSphere * s = static_cast<BoundingSphere *>(target);

		// The normal goes from this volume to the target volume
		glm::vec3 normal = glm::normalize(target->getTransform().position - this->getTransform().position);

		if (target->isFixed()){
			this->getTransform().move(-penetration_distance * normal);

			float norm_speed = glm::dot(this->Speed(), normal);
			this->Speed() -=  (1+CoR)*norm_speed * normal;
		}
		else if (this->isFixed()){
			target->getTransform().move(+penetration_distance * normal);

			float norm_speed_target = glm::dot(target->Speed(), normal);
			target->Speed() -= (1+CoR) * norm_speed_target * normal;
		}
		else{ // no spheres are fixed
			float m1 = this->Mass(), m2 = target->Mass();
			float total_mass = m1 + m2;
			float mr1 = m1 / total_mass; // mass_ratios for faster computation
			float mr2 = m2 / total_mass;

			this->getTransform().move(-0.5f* penetration_distance * mr2 * normal);
			target->getTransform().move(+0.5f* penetration_distance * mr1 * normal);

			// compute the normal components of the speed
			float ns1 = glm::dot(this->Speed(), normal);
			float ns2 = glm::dot(target->Speed(), normal);

			//float new_ns1 = ((m1 - m2)*ns1 + 2 * m2 * ns2) / total_mass;
			//float new_ns2 = ((m2 - m1)*ns2 + 2 * m1 * ns1) / total_mass;
			float new_ns1 = CoR * mr2 * (ns2 - ns1) + mr1*ns1 + mr2 * ns2;
			float new_ns2 = CoR * mr1 * (ns1 - ns2) + mr1*ns1 + mr2 * ns2;

			//change the final speeds
			this->Speed() += (new_ns1 - ns1) * normal;
			target->Speed() += (new_ns2 - ns2) * normal;
		}
	}
	else if (target->VolumeType() == CAPSULE_TYPE){
		BoundingCapsule * cap = static_cast<BoundingCapsule *>(target);
		//TODO
		return;
	}
	else{
		// error
		return;
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