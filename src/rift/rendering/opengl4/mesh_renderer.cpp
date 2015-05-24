#include <rendering/opengl4/mesh_renderer.hpp>
#include <glm/gtc/packing.hpp>

namespace gl4
{
	// packed vertex layout
	struct PackedVertex
	{
		glm::vec3 pos;
		glm::uint32 norm; // packSnorm3x10_1x2
		glm::uint32 tg; // packSnorm3x10_1x2
		glm::uint32 uv0;	// packUnorm2x16
	};

	Mesh::Ptr Mesh::create(GraphicsContext &context, const MeshData &meshData)
	{
		auto ptr = std::make_unique<Mesh>();
		auto nv = meshData.vertices.size();
		auto ni = meshData.indices.size();
		ptr->vbo = context.allocBuffer(
			BufferUsage::VertexBuffer, 
			nv*sizeof(PackedVertex));
		ptr->ibo = context.allocBuffer(
			BufferUsage::IndexBuffer,
			ni*2);
		ptr->nbvertex = nv;
		ptr->nbindex = ni;
		
		auto vbo_ptr = (PackedVertex*)ptr->vbo->ptr;
		for (auto i = 0u; i < nv; ++i)
		{
			vbo_ptr[i].pos = meshData.vertices[i];
			vbo_ptr[i].norm = glm::packSnorm3x10_1x2(glm::vec4(meshData.normals[i], 0.0f));
			vbo_ptr[i].tg = glm::packSnorm3x10_1x2(glm::vec4(meshData.tangents[i], 0.0f));
			vbo_ptr[i].uv0 = glm::packUnorm2x16(meshData.uv[0][i]);
		}

		auto ibo_ptr = (uint16_t*)ptr->ibo->ptr;
		for (auto i = 0u; i < ni; ++i)
		{
			ibo_ptr[i] = meshData.indices[i];
		}

		ptr->submeshes = meshData.submeshes;

		return std::move(ptr);
	}

	MeshRenderer::MeshRenderer(GraphicsContext& graphicsContext) : context(graphicsContext)
	{
		vao = InputLayout::create(1, { 
				{ ElementFormat::Float3, 0 },
				{ ElementFormat::Snorm10x3_1x2, 0 },
				{ ElementFormat::Snorm10x3_1x2, 0 },
				{ ElementFormat::Unorm16x2, 0 },
			});
	}

	void MeshRenderer::renderForwardPass(
		ForwardPassContext &pass,
		Mesh &mesh, 
		Material &material,
		const glm::mat4 &modelToWorld)
	{
		// bind material
		material.prepareForwardPass(context, pass, modelToWorld);
		
		pass.cmdBuf->setVertexBuffers({ mesh.vbo.get() }, *vao);
		for (auto &sm : mesh.submeshes)
		{
			pass.cmdBuf->drawIndexed(
				PrimitiveType::Triangle, 
				*mesh.ibo, 
				sm.startVertex, 
				sm.startIndex, 
				sm.numIndices, 
				0, 1);
		}

		// material.prepareForwardPass(mesh, modelToWorld, buf)
		// bind VBO & IBO
	}
}