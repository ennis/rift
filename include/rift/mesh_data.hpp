#ifndef MESH_DATA_HPP
#define MESH_DATA_HPP

#include <utils/small_vector.hpp>
#include <renderer_common.hpp>
#include <iosfwd>

struct MeshData
{
	// vertices
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	// max 8 uv sets
	util::small_vector<std::vector<glm::vec2>, 8> uv;
	std::vector<uint16_t> indices;
	std::vector<Submesh> submeshes;

	void loadFromStream(std::istream &in_stream);
};

#endif