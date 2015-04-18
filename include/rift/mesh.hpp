#ifndef MESH_HPP
#define MESH_HPP

#include <gl4/renderer.hpp>
#include <serialization.hpp>

class Mesh
{
public:
	using Ptr = std::unique_ptr < Mesh > ;

	Mesh(util::array_ref<Attribute> layout,
		int numVertices,
		const void *vertexData,
		int numIndices,
		const void *indexData,
		util::array_ref<Submesh> submeshes);

	Mesh() = default;

	static Ptr create(
		util::array_ref<Attribute> layout,
		int numVertices,
		const void *vertexData,
		int numIndices,
		const void *indexData,
		util::array_ref<Submesh> submeshes)
	{
		return std::make_unique<Mesh>(
			layout, 
			numVertices, 
			vertexData, 
			numIndices, 
			indexData, 
			submeshes);
	}

	void draw(RenderQueue &renderQueue, unsigned submesh);
	void drawInstanced(RenderQueue &renderQueue, unsigned submesh, unsigned baseInstance, unsigned numInstances);

	static Ptr loadFromArchive(serialization::IArchive &ar);

	InputLayout::Ptr layout;
	Buffer::Ptr vbo;
	Buffer::Ptr ibo;
	unsigned nbvertex;
	unsigned nbindex;
	unsigned stride;
	GLenum index_format;
	std::vector<Submesh> submeshes;
};


class SkinnedMesh
{
public:
	using Ptr = std::unique_ptr < SkinnedMesh >;

	SkinnedMesh() = default;
	void update(util::array_ref<glm::mat4> pose);
	void draw(RenderQueue &renderQueue, unsigned submesh);
	void drawInstanced(RenderQueue &renderQueue, unsigned submesh, unsigned baseInstance, unsigned numInstances);
	static Ptr loadFromArchive(serialization::IArchive &ar);

private:
	// part of the vertex that does not change
	struct StaticVertex
	{
		glm::vec2 tex;
	};

	// vertex data updated on each frame
	struct DynamicVertex
	{
		glm::vec3 pos;
		glm::vec3 norm;
		glm::vec3 tg;
		glm::vec3 bitg;
	};

	// positions (updated each frame)
	// TODO update normals...
	std::vector<glm::vec3> base_pos;
	std::vector<glm::vec3> base_norm;
	std::vector<glm::vec3> base_tg;
	std::vector<glm::vec3> base_bitg;
	std::vector<glm::u8vec4> bone_ids;
	std::vector<glm::vec4> bone_weights;
	InputLayout::Ptr layout;
	Stream::Ptr dynamic_attribs;
	// other attributes (static)
	Buffer::Ptr static_attribs;	
	Buffer::Ptr ibo;
	unsigned nbvertex;
	unsigned nbindex;
	unsigned stride;
	GLenum index_format;
	std::vector<Submesh> submeshes;

};

 
#endif /* end of include guard: MESH_HPP */