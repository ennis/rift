#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>
#include <memory>

#include <gl_common.hpp>
#include <glm/glm.hpp>
#include <array_ref.hpp>
#include <unique_resource.hpp>
#include <unordered_map>

namespace std {
	template <> struct hash<SamplerDesc>
	{
		size_t operator()(const SamplerDesc& v) const
		{
			auto h = hash<int>();
			return h(static_cast<int>(v.addrU))
				^ h(static_cast<int>(v.addrV))
				^ h(static_cast<int>(v.addrW))
				^ h(static_cast<int>(v.minFilter))
				^ h(static_cast<int>(v.magFilter));
		}
	};
}

namespace gl4
{
	// constexpr
	const int kMaxUniformBufferBindings = 16;
	const int kMaxVertexBufferBindings = 16;
	const int kMaxTextureUnits = 16;  

	class Texture2D;
	class TextureCubeMap;
	class RenderTarget;
	class RenderQueue;
	class Effect;
	class Parameter;
	class ParameterBlock;
	class ConstantBuffer;
	class Renderer;

	// WORKAROUND for vs2013
	// VS2013 does not support implicit generation of move constructors and move assignment operators
	// so the unique_resource pattern and the 'rule of zero' (http://flamingdangerzone.com/cxx11/2012/08/15/rule-of-zero.html)
	// is effectively useless

	class Texture2D
	{
		friend class Renderer;
	public:
		Texture2D() = default;

		Texture2D(
			glm::ivec2 size,
			int numMipLevels,
			ElementFormat pixelFormat,
			const void *data
			);

		// VS2013
		Texture2D(Texture2D &&rhs) : id(std::move(rhs.id)), size(rhs.size), format(rhs.format), glformat(rhs.glformat) {}
		Texture2D &operator=(Texture2D &&rhs) {
			id = std::move(rhs.id);
			size = rhs.size;
			format = rhs.format;
			glformat = rhs.glformat;
			return *this;
		}
		// -VS2013

		void update(
			int mipLevel,
			glm::ivec2 offset,
			glm::ivec2 size,
			const void *data
			);

	//protected:
		struct Deleter {
			void operator()(GLuint id) {
				gl::DeleteTextures(1, &id);
			}
		};

		util::unique_resource<GLuint, Deleter> id;
		glm::ivec2 size;
		ElementFormat format;
		GLenum glformat;
	};

	class TextureCubeMap
	{
		friend class Renderer;
	public:
		TextureCubeMap() = default;

		TextureCubeMap(
			glm::ivec2 size,
			int numMipLevels,
			ElementFormat pixelFormat,
			const void* faceData[6]
			);

		// VS2013
		TextureCubeMap(TextureCubeMap &&rhs)  {}
		TextureCubeMap &operator=(TextureCubeMap &&rhs) {}
		// -VS2013

	//protected:

	};

	class RenderTarget
	{
		friend class Renderer;
	public:
		RenderTarget() = default;

		// VS2013
		RenderTarget(RenderTarget &&rhs) : format(rhs.format), u(rhs.u), type(rhs.type), mipLevel(rhs.mipLevel), layer(rhs.layer) {}
		RenderTarget &operator=(RenderTarget &&rhs) {
			format = rhs.format;
			u = rhs.u;
			type = rhs.type;
			mipLevel = rhs.mipLevel;
			layer = rhs.layer;
			return *this;
		}
		// -VS2013


		enum Type
		{
			kRenderToTexture2D,
			kRenderToCubeMap,
			kRenderToCubeMapLayer
		};

		static RenderTarget createRenderTarget2D(
			Texture2D &texture2D,
			int mipLevel
			);

		static RenderTarget createRenderTarget2DFace(
			TextureCubeMap &cubeMap,
			int mipLevel,
			int face
			);

		static RenderTarget createRenderTargetCubeMap(
			TextureCubeMap &cubeMap,
			int mipLevel
			);


	//protected:
		ElementFormat format;
		union {
			Texture2D *texture_2d;
			TextureCubeMap *texture_cubemap;
		} u;
		Type type;
		int mipLevel;
		int layer;	// -1 if no face or texture is a cube map and is bound as a layered image
	};
	
	class Mesh
	{
		friend class Renderer;
	public:

		Mesh(PrimitiveType primitiveType,
			std::array_ref<Attribute> layout,
			int numVertices,
			const void *vertexData,
			int numIndices,
			const void *indexData,
			std::array_ref<Submesh> submeshes);

		~Mesh()
		{}

		// VS2013
		Mesh(Mesh &&rhs) :
			mode(rhs.mode),
			vb(std::move(rhs.vb)),
			ib(std::move(rhs.ib)),
			vao(std::move(rhs.vao)),
			vbsize(rhs.vbsize),
			ibsize(rhs.ibsize),
			nbvertex(rhs.nbvertex),
			nbindex(rhs.nbindex),
			stride(rhs.stride),
			index_format(rhs.index_format),
			submeshes(std::move(rhs.submeshes))
		{}
		Mesh &operator=(Mesh &&rhs) {
			mode = rhs.mode;
			vb = std::move(rhs.vb);
			ib = std::move(rhs.ib);
			vao = std::move(rhs.vao);
			vbsize = rhs.vbsize;
			ibsize = rhs.ibsize;
			nbvertex = rhs.nbvertex;
			nbindex = rhs.nbindex;
			stride = rhs.stride;
			index_format = rhs.index_format;
			submeshes = std::move(rhs.submeshes);
			return *this;
		}
		// -VS2013

	//protected:
		Mesh() = default;
		// TODO multiple buffers
		// one for each update frequency
		struct Deleter {
			void operator()(GLuint id) {
				gl::DeleteBuffers(1, &id);
			}
		};

		struct VAODeleter {
			void operator()(GLuint id) {
				gl::DeleteVertexArrays(1, &id);
			}
		};

		GLenum mode;
		util::unique_resource<GLuint, Deleter> vb;
		util::unique_resource<GLuint, Deleter> ib;
		util::unique_resource<GLuint, VAODeleter> vao;
		int vbsize;
		int ibsize;
		int nbvertex;
		int nbindex;
		int stride;
		GLenum index_format;
		// or smallvector?
		std::vector<Submesh> submeshes;
	};

	class Parameter
	{
		friend class Renderer;
		friend class ParameterBlock;
	public:

	//protected:
		Parameter() = default;

		const Effect * effect;
		GLuint location = 0;
		GLuint binding = 0;
		int size = 0;	// in bytes
	};

	class TextureParameter
	{
		friend class Renderer;
		friend class ParameterBlock;
	public:

	//protected:
		TextureParameter() = default;

		const Effect *effect = nullptr;
		GLuint location = 0;
		GLuint texunit = 0;
	};

	class ParameterBlock
	{
		friend class Renderer;
	public:

		ParameterBlock(Effect &effect);

		void setParameter(
			const Parameter &param,
			const void *data
			);

		void setConstantBuffer(
			const Parameter &param,
			const ConstantBuffer &constantBuffer
			);

		void setTextureParameter(
			const TextureParameter &param,
			const Texture2D *texture,
			const SamplerDesc &samplerDesc
			);

	//protected:
		ParameterBlock() = default;

		Effect * effect;
		GLuint ubo[kMaxUniformBufferBindings];
		GLintptr ubo_offsets[kMaxUniformBufferBindings];
		GLintptr ubo_sizes[kMaxUniformBufferBindings];
		GLuint textures[kMaxTextureUnits];
		GLuint samplers[kMaxTextureUnits];
	};

	class Effect
	{
		friend class Renderer;
		friend class Parameter;
		friend class TextureParameter;
		friend class ParameterBlock;

	public:
		Effect(
			const char *combinedSource,
			const char *includePath,
			RasterizerDesc rasterizerState,
			DepthStencilDesc depthStencilState);

		Effect(
			const char *vsSource,
			const char *psSource,
			const char *includePath,
			RasterizerDesc rasterizerState,
			DepthStencilDesc depthStencilState);

		// VS2013
		Effect(Effect &&rhs) :
			cache_id(rhs.cache_id),
			source(std::move(rhs.source)),
			program(std::move(rhs.program)),
			rs_state(rhs.rs_state),
			ds_state(rhs.ds_state)
		{}
		Effect &operator=(Effect &&rhs) {
			cache_id = rhs.cache_id;
			source = std::move(rhs.source);
			program = std::move(rhs.program);
			rs_state = rhs.rs_state;
			ds_state = rhs.ds_state;
			return *this;
		}
		// -VS2013

		Parameter createParameter(
			const char *name
			);

		// TODO named texture parameters
		TextureParameter createTextureParameter(
			const char *name
			);

		TextureParameter createTextureParameter(
			int texunit
			);

		ParameterBlock createParameterBlock();

	//private:
		Effect() = default;

		struct Deleter {
			void operator()(GLuint id) {
				gl::DeleteProgram(id);
			}
		};

		int cache_id;
		// effect source code
		std::string source;
		util::unique_resource<GLuint, Deleter> program;
		// XXX in source code?
		RasterizerDesc rs_state;
		DepthStencilDesc ds_state;
		// TODO list of passes
		// TODO list of parameters / uniforms?
	};

	// 
	class ConstantBuffer
	{
		friend class Renderer;

	public:

		void update(
			int offset,
			int size,
			const void *data
			);

		ConstantBuffer() = default;
		ConstantBuffer(
			int size,
			const void *initialData = nullptr);

		// VS2013
		ConstantBuffer(ConstantBuffer &&rhs) : ubo(std::move(rhs.ubo)), size(rhs.size) {}
		ConstantBuffer &operator=(ConstantBuffer &&rhs) {
			ubo = std::move(rhs.ubo);
			size = rhs.size;
			return *this;
		}
		// -VS2013
		
	// protected:
		struct Deleter {
			void operator()(GLuint id) {
				gl::DeleteBuffers(1, &id);
			}
		};

		util::unique_resource<GLuint, Deleter> ubo;
		int size = 0;
	};

	struct RenderItem
	{
		uint64_t sort_key;
		const Mesh *mesh;
		int submesh;
		const ParameterBlock *param_block;
		const Effect *effect;
	};

	class RenderQueue
	{
		friend class Renderer;
	public:
		void draw(
			const Mesh &mesh,
			int submeshIndex,
			const Effect &effect,
			const ParameterBlock &parameterBlock,
			uint64_t sortHint
			);

		void clear();

	//protected:
		RenderQueue() = default;

		std::vector<int> sort_list;
		std::vector<RenderItem> items;
	};

	class Renderer
	{
	public:
		//================================
		// Textures
		Texture2D createTexture2D(
			glm::ivec2 size,
			int numMipLevels,
			ElementFormat pixelFormat,
			const void *data
			);

		TextureCubeMap createTextureCubeMap(
			glm::ivec2 size,
			int numMipLevels,
			ElementFormat pixelFormat,
			const void* faceData[6]
			);

		// TODO create from file / opaque archive?
		Effect createEffect(
			const char *vsSource,
			const char *psSource,
			RasterizerDesc rasterizerState,
			DepthStencilDesc depthStencilState
			// TODO blend state
			);

		//================================
		// Render targets
		RenderTarget createRenderTarget2D(
			Texture2D &texture2D, 
			int mipLevel
			);

		RenderTarget createRenderTarget2DFace(
			TextureCubeMap &cubeMap, 
			int mipLevel, 
			int face
			);

		RenderTarget createRenderTargetCubeMap(
			TextureCubeMap &cubeMap, 
			int mipLevel
			);

		//================================
		// Render queues
		RenderQueue createRenderQueue(
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
			std::array_ref<const RenderTarget*> colorTargets,
			const RenderTarget *depthStencilTarget
			);

		void setViewports(
			std::array_ref<Viewport2> viewports
			);

		void submitRenderQueue(
			RenderQueue &renderQueue
			);

		// TODO draw instanced

		//================================
		// static
		static Renderer &getInstance();
		static void initialize();

		// do not use
		Renderer() = default;
		GLuint getSampler(SamplerDesc desc);

	private:
		

		void drawItem(const RenderItem &item);

		GLuint fbo = 0;
		RenderTarget screen_rt;
		RenderTarget screen_depth_rt;

		//-----------------------------
		// sampler state cache
		struct SamplerDeleter {
			void operator()(GLuint id) {
				gl::DeleteSamplers(1, &id);
			}
		};
		std::unordered_map<SamplerDesc, util::unique_resource<GLuint, SamplerDeleter> > sampler_cache;

		static std::unique_ptr<Renderer> instance;
	};
}

// TODO move this somewhere else (utils?)
std::string loadEffectSource(const char *fileName);

using Renderer = gl4::Renderer;
using Effect = gl4::Effect;
using Texture2D = gl4::Texture2D; 
using ParameterBlock = gl4::ParameterBlock;
using Parameter = gl4::Parameter;
using TextureParameter = gl4::TextureParameter;
using ConstantBuffer = gl4::ConstantBuffer;
using RenderQueue = gl4::RenderQueue;
using Mesh = gl4::Mesh;
 
#endif /* end of include guard: RENDERER_HPP */