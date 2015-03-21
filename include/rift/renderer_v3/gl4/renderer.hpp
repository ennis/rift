#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>
#include <memory>

#include <gl_common.hpp>
#include <glm/glm.hpp>
#include <array_ref.hpp>
#include <unordered_map>
#include <serialization.hpp>
#include <unique_resource.hpp>
#include <small_vector.hpp>

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
	constexpr auto kMaxUniformBufferBindings = 16u;
	constexpr auto kMaxVertexBufferBindings = 16u;
	constexpr auto kMaxTextureUnits = 16u;

	class Texture2D;
	class TextureCubeMap;
	class RenderTarget;
	class RenderQueue;
	class Shader;
	class Parameter;
	class ParameterBlock;
	class ConstantBuffer;
	class Renderer;
	class InputLayout;

	// WORKAROUND for vs2013
	// VS2013 does not support implicit generation of move constructors and move assignment operators
	// so the unique_resource pattern and the 'rule of zero' (http://flamingdangerzone.com/cxx11/2012/08/15/rule-of-zero.html)
	// is effectively useless


	class InputLayout
	{
	public:
		using Ptr = std::unique_ptr<InputLayout>;

		InputLayout(util::array_ref<Attribute> attribs);

		static Ptr create(util::array_ref<Attribute> attribs)
		{
			return std::make_unique<InputLayout>(attribs);
		}

	// protected:
		GLuint vao;
		int stride;
	};

	class Buffer
	{
	public:
		using Ptr = std::unique_ptr < Buffer > ;

		Buffer() = default;
		Buffer(
			int size,
			ResourceUsage resourceUsage,
			BufferUsage usage,
			void *initialData
			);

		~Buffer()
		{
			gl::DeleteBuffers(1, &id);
		}

		void update(
			int offset,
			int size,
			const void *data);

		static Ptr create(
			int size,
			ResourceUsage resourceUsage,
			BufferUsage usage,
			void *initialData
			);

	//protected:
		int size;
		GLbitfield flags;
		GLenum target;
		GLuint id;
	};

	class Texture2D
	{
		friend class Renderer;
	public:
		using Ptr = std::unique_ptr < Texture2D >;

		Texture2D() = default;

		Texture2D(
			glm::ivec2 size,
			int numMipLevels,
			ElementFormat pixelFormat,
			const void *data
			);

		~Texture2D()
		{
			gl::DeleteTextures(1, &id);
		}

		// VS2013
		/*Texture2D(Texture2D &&rhs) : id(std::move(rhs.id)), size(rhs.size), format(rhs.format), glformat(rhs.glformat) {}
		Texture2D &operator=(Texture2D &&rhs) {
			id = std::move(rhs.id);
			size = rhs.size;
			format = rhs.format;
			glformat = rhs.glformat;
			return *this;
		}*/
		// -VS2013

		void update(
			int mipLevel,
			glm::ivec2 offset,
			glm::ivec2 size,
			const void *data
			);

	//protected:
		static Ptr create(
			glm::ivec2 size,
			int numMipLevels,
			ElementFormat pixelFormat,
			const void *data)
		{
			return std::make_unique<Texture2D>(size, numMipLevels, pixelFormat, data);
		}

		GLuint id;
		glm::ivec2 size;
		ElementFormat format;
		GLenum glformat;
	};

	class TextureCubeMap
	{
		friend class Renderer;
	public:
		using Ptr = std::unique_ptr < TextureCubeMap >;

		TextureCubeMap() = default;

		TextureCubeMap(
			glm::ivec2 size,
			int numMipLevels,
			ElementFormat pixelFormat,
			const void* faceData[6]
			);

		void update(
			int face,
			int mipLevel,
			glm::ivec2 offset,
			glm::ivec2 size,
			const void *data
			);

		// VS2013
		/*TextureCubeMap(TextureCubeMap &&rhs)  {}
		TextureCubeMap &operator=(TextureCubeMap &&rhs) {}*/
		// -VS2013

		static Ptr create(
			glm::ivec2 size,
			int numMipLevels,
			ElementFormat pixelFormat,
			const void* faceData[6])
		{
			return std::make_unique<TextureCubeMap>(
				size,
				numMipLevels,
				pixelFormat,
				faceData);
		}

	//protected:
		GLuint id;
		glm::ivec2 size;
		ElementFormat format;
		GLenum glformat;
	};

	class RenderTarget
	{
		friend class Renderer;
	public:
		using Ptr = std::unique_ptr < RenderTarget > ;
		RenderTarget() = default;

		// VS2013
		/*RenderTarget(RenderTarget &&rhs) : format(rhs.format), u(rhs.u), type(rhs.type), mipLevel(rhs.mipLevel), layer(rhs.layer) {}
		RenderTarget &operator=(RenderTarget &&rhs) {
			format = rhs.format;
			u = rhs.u;
			type = rhs.type;
			mipLevel = rhs.mipLevel;
			layer = rhs.layer;
			return *this;
		}*/
		// -VS2013

		enum Type
		{
			kRenderToTexture2D,
			kRenderToCubeMap,
			kRenderToCubeMapLayer
		};

		static RenderTarget::Ptr createRenderTarget2D(
			Texture2D &texture2D,
			int mipLevel
			);

		static RenderTarget::Ptr createRenderTarget2DFace(
			TextureCubeMap &cubeMap,
			int mipLevel,
			int face
			);

		static RenderTarget::Ptr createRenderTargetCubeMap(
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

		using Ptr = std::unique_ptr < Mesh > ;

		Mesh(util::array_ref<Attribute> layout,
			int numVertices,
			const void *vertexData,
			int numIndices,
			const void *indexData,
			util::array_ref<Submesh> submeshes,
			ResourceUsage usage);

		// VS2013
		/*Mesh(Mesh &&rhs) :
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
		}*/
		// -VS2013

	//protected:
		Mesh() = default;

		~Mesh() {
			gl::DeleteVertexArrays(1, &vao);
			gl::DeleteBuffers(1, &vb);
			gl::DeleteBuffers(1, &ib);
		}

		void setSubmesh(int index, const Submesh &submesh);

		void updateVertices(int offset, int size, const void *data);
		void updateIndices(int offset, int size, const uint16_t *data);

		static Ptr create(
			util::array_ref<Attribute> layout,
			int numVertices,
			const void *vertexData,
			int numIndices,
			const void *indexData,
			util::array_ref<Submesh> submeshes,
			ResourceUsage usage)
		{
			return std::make_unique<Mesh>(
				layout, 
				numVertices, 
				vertexData, 
				numIndices, 
				indexData, 
				submeshes,
				usage);
		}

		static Ptr loadFromArchive(serialization::IArchive &ar);

		static const auto kMaxVertexBuffers = 16u;

		// TODO multiple buffers?
		GLbitfield vb_flags;
		GLbitfield ib_flags;
		GLuint vb;
		GLuint ib;
		GLuint vao;
		int nbvb;
		int vbsize;
		int ibsize;
		int nbvertex;
		int nbindex;
		int stride;
		GLenum index_format;
		std::vector<Submesh> submeshes;
	};

	class Parameter
	{
		friend class Renderer;
		friend class ParameterBlock;
	public:
		using Ptr = std::unique_ptr < Parameter >;

	//protected:
		Parameter() = default;

		const Shader *shader;
		GLuint location = 0;
		GLuint binding = 0;
		int size = 0;	// in bytes
	};

	class TextureParameter
	{
		friend class Renderer;
		friend class ParameterBlock;
	public:

		using Ptr = std::unique_ptr < TextureParameter > ;

	//protected:
		TextureParameter() = default;

		const Shader *shader = nullptr;
		GLuint location = 0;
		GLuint texunit = 0;
	};

	class ParameterBlock
	{
		friend class Renderer;
	public:

		using Ptr = std::unique_ptr < ParameterBlock > ;

		ParameterBlock(Shader &shader);

		void setParameter(
			const Parameter &param,
			const void *data
			);

		void setConstantBuffer(
			const Parameter &param,
			const ConstantBuffer &constantBuffer
			);

		void setConstantBuffer(
			int binding,
			const ConstantBuffer &constantBuffer
			);

		void setTextureParameter(
			const TextureParameter &param,
			const Texture2D *texture,
			const SamplerDesc &samplerDesc
			);

		void setTextureParameter(
			int texunit,
			const Texture2D *texture,
			const SamplerDesc &samplerDesc
			);

		void setTextureParameter(
			int texunit,
			const TextureCubeMap *texture,
			const SamplerDesc &samplerDesc
			);

		static Ptr create(Shader &shader) {
			return std::make_unique<ParameterBlock>(shader);
		}

	//protected:
		ParameterBlock() = default;

		Shader * shader;
		GLuint ubo[kMaxUniformBufferBindings];
		GLintptr ubo_offsets[kMaxUniformBufferBindings];
		GLintptr ubo_sizes[kMaxUniformBufferBindings];
		GLuint textures[kMaxTextureUnits];
		GLuint samplers[kMaxTextureUnits];
	};

	class Shader
	{
		friend class Renderer;
		friend class Parameter;
		friend class TextureParameter;
		friend class ParameterBlock;

	public:
		using Ptr = std::unique_ptr < Shader >;
		
		Shader(
			const char *vsSource,
			const char *psSource,
			const RasterizerDesc &rasterizerState,
			const DepthStencilDesc &depthStencilState,
			const BlendDesc &blendState);

		// VS2013
		/*Effect(Effect &&rhs) :
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
		}*/
		// -VS2013

		static Ptr create(
			const char *vsSource,
			const char *psSource,
			const RasterizerDesc &rasterizerState,
			const DepthStencilDesc &depthStencilState,
			const BlendDesc &blendState)
		{
			return std::make_unique<Shader>(
				vsSource,
				psSource, 
				rasterizerState, 
				depthStencilState, 
				blendState);
		}

	//private:
		Shader() = default;
		~Shader()
		{
			gl::DeleteProgram(program);
		}

		int cache_id;
		// shader source code
		std::string source;
		GLuint program = 0;
		// XXX in source code?
		RasterizerDesc rs_state;
		DepthStencilDesc ds_state; 
		BlendDesc om_state;
		// TODO list of passes
		// TODO list of parameters / uniforms?
	};

	// 
	class ConstantBuffer
	{
		friend class Renderer;

	public:

		using Ptr = std::unique_ptr < ConstantBuffer > ;

		void update(
			int offset,
			int size,
			const void *data
			);

		ConstantBuffer() = default;
		ConstantBuffer(
			int size,
			const void *initialData = nullptr);

		~ConstantBuffer()
		{
			gl::DeleteBuffers(1, &ubo);
		}

		// VS2013
		/*ConstantBuffer(ConstantBuffer &&rhs) : ubo(std::move(rhs.ubo)), size(rhs.size) {}
		ConstantBuffer &operator=(ConstantBuffer &&rhs) {
			ubo = std::move(rhs.ubo);
			size = rhs.size;
			return *this;
		}*/
		// -VS2013

		static Ptr create(
			int size,
			const void *initialData) 
		{
			return std::make_unique<ConstantBuffer>(size, initialData);
		}

	// protected:

		GLuint ubo;
		int size = 0;
	};

	struct RenderItem
	{
		uint64_t sort_key;
		const Mesh *mesh;
		int submesh_index;
		const ParameterBlock *param_block;
		const Shader *shader;
		int procedural_count;	// == 0 if mesh == nullptr
		GLenum procedural_mode;
	};

	class RenderQueue
	{
		friend class Renderer;
	public:
		using Ptr = std::unique_ptr < RenderQueue > ;

		void draw(
			const Mesh &mesh,
			int submesh_index,
			const Shader &shader,
			const ParameterBlock &parameterBlock,
			uint64_t sortHint
			);

		void drawProcedural(
			PrimitiveType primitiveType,
			int count,
			const Shader &shader,
			const ParameterBlock &parameterBlock,
			uint64_t sortHint);


		void clear();

	//protected:
		RenderQueue() = default;

		std::vector<int> sort_list;
		std::vector<RenderItem> items;

		static Ptr create() { return std::make_unique<RenderQueue>(); }
	};

	class Renderer
	{
	public:

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
			util::array_ref<const RenderTarget*> colorTargets,
			const RenderTarget *depthStencilTarget
			);

		void setViewports(
			util::array_ref<Viewport2> viewports
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
		Renderer();
		~Renderer();
		GLuint getSampler(SamplerDesc desc);

	private:
		void drawItem(const RenderItem &item);

		GLuint dummy_vao;
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

using InputLayout = gl4::InputLayout;
using Renderer = gl4::Renderer;
using Buffer = gl4::Buffer;
using Shader = gl4::Shader;
using Texture2D = gl4::Texture2D; 
using TextureCubeMap = gl4::TextureCubeMap;
using ParameterBlock = gl4::ParameterBlock;
using Parameter = gl4::Parameter;
using TextureParameter = gl4::TextureParameter;
using ConstantBuffer = gl4::ConstantBuffer;
using RenderQueue = gl4::RenderQueue;
using Mesh = gl4::Mesh;
using RenderTarget = gl4::RenderTarget;
 
#endif /* end of include guard: RENDERER_HPP */