#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>
#include <memory>

#include <gl_common.hpp>
#include <effect.hpp>
#include <mesh.hpp>
#include <renderqueue.hpp>
#include <constant_buffer.hpp>
#include <array_ref.hpp>
#include <glm/glm.hpp>
#include <texture.hpp>

namespace gl4
{
	// constexpr
	const int kMaxUniformBufferBindings = 16;
	const int kMaxVertexBufferBindings = 16;
	const int kMaxTextureUnits = 16;  

	// TODO Rename impl -> handle

	struct Texture2DImpl
	{
		GLuint id;
		glm::ivec2 size;
		ElementFormat format;
		GLenum glformat;
	};

	struct TextureCubeMapImpl
	{

	};

	struct RenderTargetImpl
	{
		enum Type
		{
			kRenderToTexture2D,
			kRenderToCubeMap,
			kRenderToCubeMapLayer
		};

		ElementFormat format;
		union {
			Texture2DImpl *texture_2d;
			TextureCubeMapImpl *texture_cubemap;
		} u;
		Type type;
		int mipLevel;
		int layer;	// -1 if no face or texture is a cube map and is bound as a layered image
	};
	
	struct MeshImpl
	{
		// TODO multiple buffers
		// one for each update frequency
		GLenum mode;
		GLuint vb;
		GLuint ib;
		GLuint vao;
		int vbsize;
		int ibsize;
		int nbvertex;
		int nbindex;
		int stride;
		GLenum index_format;
		// or smallvector?
		std::vector<Submesh> submeshes;
	};

	struct EffectImpl
	{
		int cache_id;
		// shader source code
		std::string source;
		GLuint program;
		// XXX in source code?
		RasterizerDesc rs_state;
		DepthStencilDesc ds_state;
		// TODO list of passes
		// TODO list of parameters / uniforms?
	};

	struct ParameterImpl
	{
		const EffectImpl *effect;
		GLuint location;
		GLuint binding;
		int size;	// in bytes
	};

	struct TextureParameterImpl
	{
		const EffectImpl *effect;
		GLuint location;
		GLuint texunit;
	};

	struct ParameterBlockImpl
	{
		GLuint ubo[kMaxUniformBufferBindings];
		GLintptr ubo_offsets[kMaxUniformBufferBindings];
		GLintptr ubo_sizes[kMaxUniformBufferBindings];
		GLuint textures[kMaxTextureUnits];
		GLuint samplers[kMaxTextureUnits];
	};

	// 
	struct ConstantBufferImpl
	{
		GLuint ubo;
		int size;
	};

	struct RenderItem
	{
		uint64_t sort_key;
		const MeshImpl *mesh;
		int submesh;
		const ParameterBlockImpl *param_block;
		const EffectImpl *effect;
	};

	struct RenderQueueImpl
	{
		std::vector<int> sort_list;
		std::vector<RenderItem> items;
	};

	class Renderer
	{
	public:
		//================================
		// Implementation-defined types
		using Texture2DImpl = gl4::Texture2DImpl;
		using TextureCubeMapImpl = gl4::TextureCubeMapImpl;
		using RenderTargetImpl = gl4::RenderTargetImpl;
		using ParameterImpl = gl4::ParameterImpl;
		using TextureParameterImpl = gl4::TextureParameterImpl;
		using EffectImpl = gl4::EffectImpl;
		using ConstantBufferImpl = gl4::ConstantBufferImpl;
		using RenderTargetImpl = gl4::RenderTargetImpl;
		using MeshImpl = gl4::MeshImpl;
		using ParameterBlockImpl = gl4::ParameterBlockImpl;
		using RenderQueueImpl = gl4::RenderQueueImpl;

		//================================
		// Textures
		Texture2DImpl createTexture2D(
			glm::ivec2 size,
			int numMipLevels,
			ElementFormat pixelFormat,
			const void *data
			);

		void updateTexture2D(
			Texture2DImpl &impl,
			int mipLevel,
			glm::ivec2 offset,
			glm::ivec2 size,
			const void *data
			);

		void deleteTexture2D(
			Texture2DImpl &impl
			);

		TextureCubeMapImpl createTextureCubeMap(
			glm::ivec2 size,
			int numMipLevels,
			ElementFormat pixelFormat,
			const void* faceData[6]
			);

		void deleteTextureCubeMap(
			TextureCubeMapImpl &impl
			);

		//================================
		// Meshes
		MeshImpl createMesh(
			PrimitiveType primitiveType,
			std::array_ref<Attribute> layout,
			int numVertices,
			const void *vertexData,
			int numIndices,
			const void *indexData,
			std::array_ref<Submesh> submeshes
			);

		void deleteMesh(
			MeshImpl &mesh
			);

		//================================
		// Effects

		// returns an object describing an effect parameter
		ParameterImpl createEffectParameter(
			const EffectImpl &effect, 
			const char *name
			);

		TextureParameterImpl createEffectTextureParameter(
			const EffectImpl &effect,
			const char *name
			);

		void deleteEffectParameter(
			ParameterImpl &impl
			);

		void deleteEffectTextureParameter(
			TextureParameterImpl &impl
			);

		ConstantBufferImpl createConstantBuffer(
			int size,
			const void *initialData
			);

		void updateConstantBuffer(
			ConstantBufferImpl &impl,
			int offset,
			int size,
			const void *data
			);

		void deleteConstantBuffer(
			ConstantBufferImpl &impl
			);

		// TODO create from file / opaque archive?
		EffectImpl createEffect(
			const char *combinedShaderSource,
			const char *includePath,
			RasterizerDesc rasterizerState,
			DepthStencilDesc depthStencilState
			);

		void deleteEffect(
			EffectImpl &effect
			);

		//================================
		// Parameter blocks
		// TODO
		ParameterBlockImpl createParameterBlock(
			const EffectImpl &effect
			//const PassImpl &pass
			// TechniqueID / PassID
			);

		void setParameter(
			ParameterBlockImpl &paramBlock,
			const ParameterImpl &param,
			const void *data
			);

		void setConstantBuffer(
			ParameterBlockImpl &paramBlock,
			const ParameterImpl &param,
			const ConstantBufferImpl &constantBuffer
			);

		void setTextureParameter(
			ParameterBlockImpl &paramBlock,
			const TextureParameterImpl &param,
			const Texture2DImpl *texture,
			const SamplerDesc &samplerDesc
			);

		void deleteParameterBlock(
			ParameterBlockImpl &impl
			);

		//================================
		// Render targets
		RenderTargetImpl createRenderTarget2D(
			Texture2DImpl *texture2D, 
			int mipLevel
			);

		RenderTargetImpl createRenderTarget2DFace(
			TextureCubeMapImpl *cubeMap, 
			int mipLevel, 
			int face
			);

		RenderTargetImpl createRenderTargetCubeMap(
			TextureCubeMapImpl *cubeMap, 
			int mipLevel
			);

		void deleteRenderTarget(
			RenderTargetImpl &impl
			);

		//================================
		// Render queues
		RenderQueueImpl createRenderQueue(
			// void
			// TODO?
			);

		void clearRenderQueue(
			RenderQueueImpl &impl
			);

		void deleteRenderQueue(
			RenderQueueImpl &impl
			);

		void draw(
			RenderQueueImpl &renderQueue,
			const MeshImpl &mesh,
			int submeshIndex,
			const EffectImpl &effect,
			const ParameterBlockImpl &parameterBlock,
			uint64_t sortHint
			);

		//================================
		// Submission

		// issue a clear color command
		void clearColor(
			float r,
			float g,
			float b,
			float a
			);

		// issue a clear depth command
		void clearDepth(
			float z
			);

		// set the color & depth render targets
		void setRenderTargets(
			std::array_ref<const RenderTargetImpl*> colorTargets,
			const RenderTargetImpl *depthStencilTarget
			);

		void setViewports(
			std::array_ref<Viewport2> viewports
			);

		void submitRenderQueue(
			RenderQueueImpl &renderQueue
			);

		// TODO draw instanced

		//================================
		// static
		static Renderer &getInstance();
		static void initialize();

		// do not use
		Renderer() = default;

	private:
		void drawItem(const RenderItem &item);

		GLuint fbo = 0;
		RenderTargetImpl screen_rt;
		RenderTargetImpl screen_depth_rt;

		static std::unique_ptr<Renderer> instance;
	};
}

// TODO move this somewhere else (utils?)
std::string loadShaderSource(const char *fileName);

// instantiate RAII wrapper types
using Renderer = gl4::Renderer;
using Effect = EffectBase < gl4::Renderer >;
using Texture2D = Texture2DBase < gl4::Renderer >;
using BaseParameter = ParameterBase < gl4::Renderer >;
using ConstantBuffer = ConstantBufferBase < gl4::Renderer >;
using RenderQueue = RenderQueueBase < gl4::Renderer >;
using Mesh = MeshBase < gl4::Renderer >;
using Parameter = ParameterBase < gl4::Renderer >;
using ParameterBlock = ParameterBlockBase < gl4::Renderer >;
 
#endif /* end of include guard: RENDERER_HPP */