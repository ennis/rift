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
#include <boost/circular_buffer.hpp>

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
	namespace detail
	{
		GLint textureFilterToGL(TextureFilter filter);
		GLint textureAddressModeToGL(TextureAddressMode addr);
		GLenum bufferUsageToBindingPoint(BufferUsage bufferUsage);
		GLenum blendOpToGL(BlendOp bo);
		GLenum blendFactorToGL(BlendFactor bf);
		GLenum cullModeToGLenum(CullMode mode);
		GLenum fillModeToGLenum(PolygonFillMode fillMode);
		GLenum primitiveTypeToGLenum(PrimitiveType type);
		GLuint createBuffer(
			GLenum bindingPoint,
			int size,
			GLuint flags,
			const void *initialData
			);
		void updateBuffer(
			GLuint buf,
			GLenum target,
			int offset,
			int size,
			const void *data
			);
		GLuint glslCompileShader(const char *shaderSource, GLenum type);
		void glslLinkProgram(GLuint program);
		GLuint glslCreateProgram(const char *vertexShaderSource, const char *fragmentShaderSource);
	}

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
	class RenderQueue2;

	// WORKAROUND for vs2013
	// VS2013 does not support implicit generation of move constructors and move assignment operators
	// so the unique_resource pattern and the 'rule of zero' (http://flamingdangerzone.com/cxx11/2012/08/15/rule-of-zero.html)
	// is effectively useless

	struct BufferDesc
	{
		BufferDesc() = default;

		BufferDesc(GLuint buffer_, size_t offset_, size_t size_) :
			buffer(buffer_),
			offset(offset_),
			size(size_)
		{}

		GLuint buffer = 0;
		size_t offset = 0;
		size_t size = 0;
	};

	class InputLayout
	{
	public:
		using Ptr = std::unique_ptr<InputLayout>;

		InputLayout(unsigned num_buffers, util::array_ref<Attribute> attribs);

		static Ptr create(unsigned num_buffers, util::array_ref<Attribute> attribs)
		{
			return std::make_unique<InputLayout>(num_buffers, attribs);
		}

	// protected:
		static constexpr auto kMaxVertexBuffers = 8u;
		GLuint vao;
		util::small_vector<int, kMaxVertexBuffers> strides;
	};

	// write-only GPU buffer for pushing data to the GPU
	struct Stream
	{
		using Ptr = std::unique_ptr<Stream>;
		// usage ...
		// size: size reserved for data
	
		Stream(BufferUsage usage_, size_t size_, unsigned num_buffers);

		~Stream()
		{
			gl::DeleteBuffers(1, &buffer_object);
		}

		// reserve storage for an element of type T and copy arg in it 
		template <typename T>
		void write(const T& t)
		{
			T *ptr = reserve_one<T>();
			*ptr = t;
		}

		// reserve some suitably aligned storage for an element of type T
		template <typename T>
		T *reserve_one()
		{
			// TODO aligned storage (std::align)
			return static_cast<T*>(reserve(sizeof(T)));
		}

		// reserve some suitably aligned storage for an array of n elements of type T
		template <typename T>
		T *reserve_many(int array_size)
		{
			// TODO aligned storage
			return static_cast<T*>(reserve(sizeof(T)*array_size));
		}

		// reserve some space 
		// TODO suitably aligned storage
		void *reserve(size_t size);
		// get descriptor for active storage
		BufferDesc getDescriptor() const
		{
			return BufferDesc{ buffer_object, current_offset, current_size };
		}

		// lock the reserved space and insert fence in command stream
		// all pointers returned by reserve() are invalidated
		// (buffer descriptors are still valid)
		void fence(RenderQueue2 &queue);

		static Ptr create(BufferUsage usage_, size_t size_, unsigned num_buffers)
		{
			return std::make_unique<Stream>(usage_, size_, num_buffers);
		}

		// max triple-buffering
		static const unsigned kMaxSyncRingSize = 3;

		struct Range
		{
			GLsync sync;
		};

		size_t current_offset;
		size_t current_size;
		unsigned current_range;
		size_t buffer_size;
		GLuint buffer_object;
		util::small_vector<Range, kMaxSyncRingSize> ranges;
		GLenum buffer_target;
		// mapped memory pointer
		// GL_MAP_WRITE, GL_MAP_PERSISTENT_BIT, etc.
		void *mapped_ptr;
	};

	class Buffer
	{
	public:
		using Ptr = std::unique_ptr < Buffer > ;

		Buffer() = default;
		Buffer(
			size_t size,
			ResourceUsage resourceUsage,
			BufferUsage usage,
			const void *initialData
			);

		~Buffer()
		{
			gl::DeleteBuffers(1, &id);
		}

		void update(
			size_t offset,
			size_t size,
			const void *data);

		static Ptr create(
			size_t size,
			ResourceUsage resourceUsage,
			BufferUsage usage,
			const void *initialData
			);

		BufferDesc getDescriptor() const
		{
			return BufferDesc{ id, 0, size };
		}

	//protected:
		size_t size;
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

	class ParameterBlock
	{
		friend class Renderer;
	public:

		using Ptr = std::unique_ptr < ParameterBlock > ;

		ParameterBlock(Shader &shader);
		
		void setConstantBuffer(
			int binding,
			const ConstantBuffer &constantBuffer
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
		int num_ubo;
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

	struct RenderItem2
	{
		static const unsigned kMaxVertexStreams = 8u;
		static const unsigned kMaxUniformStreams = 8u;

		enum class Type
		{
			DrawCommand,
			FenceSync,
		} type;

		union U {
			struct DrawCommand {
				// Thanks VS2013! (for not supporting unrestricted unions)
				GLuint vertex_buffers[kMaxVertexStreams];
				GLintptr vertex_buffers_offsets[kMaxVertexStreams];
				GLsizei vertex_buffers_strides[kMaxVertexStreams];
				GLuint uniform_buffers[kMaxUniformStreams];
				GLintptr uniform_buffers_offsets[kMaxUniformStreams];
				GLsizeiptr uniform_buffers_sizes[kMaxUniformStreams];
				GLuint textures[kMaxTextureUnits];
				GLuint samplers[kMaxTextureUnits];
				unsigned num_vertex_buffers;
				unsigned num_uniform_buffers;
				unsigned num_textures;
				const Shader *shader;
				const InputLayout *input_layout;
				GLuint index_buffer;
				GLintptr index_buffer_offset;
				unsigned first_vertex;
				unsigned first_index;
				unsigned vertex_count;
				unsigned index_count;
				unsigned first_instance;
				unsigned instance_count;
				GLenum mode;
			} drawCommand;
			struct Fence {
				GLsync *sync;
			} fence;
		} u;
	};


	// RenderQueue V2
	class RenderQueue2
	{
		friend class Renderer;
	public:
		using Ptr = std::unique_ptr<RenderQueue2>;

		void beginCommand();

		void setInputLayout(const InputLayout &layout);
		void setVertexBuffers(util::array_ref<BufferDesc> vertex_buffers, const InputLayout &layout);
		void setIndexBuffer(const BufferDesc &index_buffer);
		void setUniformBuffers(util::array_ref<BufferDesc> uniform_buffers);
		void setTexture2D(int unit, const Texture2D &tex, const SamplerDesc &samplerDesc);
		void setTextureCubeMap(int unit, const TextureCubeMap &tex, const SamplerDesc &samplerDesc);

		// TODO rename shader -> pipeline state
		void setShader(const Shader &shader);

		void draw(
			PrimitiveType primitiveType,
			unsigned firstVertex,
			unsigned vertexCount,
			unsigned firstInstance,
			unsigned instanceCount);

		void drawIndexed(
			PrimitiveType primitiveType,
			unsigned firstIndex,
			unsigned indexCount,
			int vertexOffset,
			unsigned firstInstance,
			unsigned instanceCount);

		// clumsy
		void fenceSync(GLsync &out_sync);


	private:
		RenderItem2 state;
		std::vector<RenderItem2> render_items;
	};

	/*class RenderQueue
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

		void drawInstanced(
			const Mesh &mesh,
			int submesh_index,
			const Shader &shader,
			const ParameterBlock &parameterBlock,
			int num_instances, 
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
	};*/

	class RenderTarget2
	{
	public:
		using Ptr = std::unique_ptr<RenderTarget2>;

		RenderTarget2();
		RenderTarget2(
			glm::ivec2 size,
			util::array_ref<ElementFormat> colorTargetFormats);
		RenderTarget2(
			glm::ivec2 size,
			util::array_ref<ElementFormat> colorTargetFormats,
			ElementFormat depthTargetFormat);

		~RenderTarget2()
		{
			if (fbo)
				gl::DeleteFramebuffers(1, &fbo);
		}

		bool hasDepthTexture() const {
			return depth_target != nullptr;
		}

		const Texture2D &getColorTexture(int index) const {
			assert(index < color_targets.size());
			return *color_targets[index];
		}
		const Texture2D &getDepthTexture() const {
			assert(hasDepthTexture());
			return *depth_target;
		}

		static RenderTarget2::Ptr create(
			glm::ivec2 size,
			util::array_ref<ElementFormat> colorTargetFormats,
			ElementFormat depthTargetFormat);
		static RenderTarget2::Ptr createNoDepth(
			glm::ivec2 size,
			util::array_ref<ElementFormat> colorTargetFormats);
		static RenderTarget2 &getDefaultRenderTarget();

		// issue a clear color command
		void clearColor(
			float r,
			float g,
			float b,
			float a
			)
		{
			gl::BindFramebuffer(gl::FRAMEBUFFER, fbo);
			gl::ClearColor(r, g, b, a);
			gl::Clear(gl::COLOR_BUFFER_BIT);
		}

		// issue a clear depth command
		void clearDepth(
			float z
			)
		{
			gl::BindFramebuffer(gl::FRAMEBUFFER, fbo);
			gl::DepthMask(gl::TRUE_);
			gl::ClearDepth(z);
			gl::Clear(gl::DEPTH_BUFFER_BIT);
		}

		void commit(RenderQueue2 &renderQueue);

		//private:
		void init();

		GLuint fbo;
		glm::ivec2 size;
		// TODO viewport arrays
		Viewport2 viewport;
		util::small_vector<Texture2D::Ptr, 8> color_targets;
		Texture2D::Ptr depth_target;
		glm::vec4 clear_color;
		float clear_depth;

		static RenderTarget2::Ptr default_rt;
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
		/*void setRenderTargets(
			util::array_ref<const RenderTarget*> colorTargets,
			const RenderTarget *depthStencilTarget
			);

		void setViewports(
			util::array_ref<Viewport2> viewports
			);

		void submitRenderQueue(
			RenderQueue &renderQueue
			);*/

		void commit(
			RenderQueue2 &renderQueue2
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
		//void drawItem(const RenderItem &item);
		void drawItem2(const RenderItem2 &item);

		GLuint dummy_vao;
		GLuint fbo = 0;
		//RenderTarget screen_rt;
		//RenderTarget screen_depth_rt;

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
using RenderTarget = gl4::RenderTarget;
using RenderTarget2 = gl4::RenderTarget2;
using Stream = gl4::Stream;
using RenderQueue2 = gl4::RenderQueue2;
using BufferDesc = gl4::BufferDesc;

 
#endif /* end of include guard: RENDERER_HPP */