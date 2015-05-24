#ifndef MESH_RENDERER_HPP
#define MESH_RENDERER_HPP

#include <rendering/opengl4/material.hpp>
#include <rendering/opengl4/graphics_context.hpp>
#include <rendering/opengl4/pass.hpp>
#include <rendering/opengl4/light.hpp>
#include <transform.hpp>
#include <mesh_data.hpp>
#include <scene/entity.hpp>
#include <asset.hpp>

namespace gl4
{
	struct Mesh : public Asset
	{
		using Ptr = std::unique_ptr<Mesh>;

		static Ptr create(GraphicsContext &context, const MeshData &meshData);

		std::vector<Submesh> submeshes;
		Buffer::Ptr vbo;
		Buffer::Ptr ibo;
		unsigned nbvertex;
		unsigned nbindex;
	};

	class MeshRenderer
	{
	public:
		MeshRenderer(GraphicsContext& graphicsContext);

		void renderForwardPass(
			ForwardPassContext &pass,
			Mesh &mesh, 
			Material &material,
			const glm::mat4 &modelToWorld);

	private:
		GraphicsContext& context;
		InputLayout::Ptr vao;
	};
}

#endif /* end of include guard: MESHRENDERER_HPP */