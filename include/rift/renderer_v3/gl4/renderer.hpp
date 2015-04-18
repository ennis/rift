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
	class RenderQueue;

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
			return BufferDesc{ buffer_object, current_range * buffer_size + current_offset, current_size };
		}

		// lock the reserved space and insert fence in command stream
		// all pointers returned by reserve() are invalidated
		// (buffer descriptors are still valid)
		void fence(RenderQueue &queue);

		static Ptr create(BufferUsage usage_, size_t size_, unsigned num_buffers)
		{
			return std::make_unique<Stream>(usage_, size_, num_buffers);
		}

		static Ptr createConstantBuffer(size_t size, size_t array_size, unsigned num_buffers)
		{
			// TODO do not hardcode alignment
			auto aligned_size = (size + 256u - 1) & ~((size_t)256u - 1);
			return create(BufferUsage::ConstantBuffer, array_size * aligned_size, num_buffers);
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

	struct RenderItem2
	{

		enum class Type
		{
			DrawCommand,
			FenceSync,
		} type;

		union U {
			struct DrawCommand {
				// Thanks VS2013! (for not supporting unrestricted unions)
				GLuint vertex_buffers[kMaxVertexBufferBindings];
				GLintptr vertex_buffers_offsets[kMaxVertexBufferBindings];
				GLsizei vertex_buffers_strides[kMaxVertexBufferBindings];
				GLuint uniform_buffers[kMaxUniformBufferBindings];
				GLintptr uniform_buffers_offsets[kMaxUniformBufferBindings];
				GLsizeiptr uniform_buffers_sizes[kMaxUniformBufferBindings];
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


	class RenderQueue
	{
		friend class Renderer;
	public:
		using Ptr = std::unique_ptr<RenderQueue>;

		RenderQueue() = default;
		RenderQueue(const RenderQueue &) = delete;
		RenderQueue &operator=(const RenderQueue &) = delete;

		void beginCommand();
		
		//void setRenderTarget(RenderTarget &render_target);
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

	class RenderTarget
	{
	public:
		using Ptr = std::unique_ptr<RenderTarget>;

		RenderTarget();
		RenderTarget(
			glm::ivec2 size,
			util::array_ref<ElementFormat> colorTargetFormats);
		RenderTarget(
			glm::ivec2 size,
			util::array_ref<ElementFormat> colorTargetFormats,
			ElementFormat depthTargetFormat);

		~RenderTarget()
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

		static RenderTarget::Ptr create(
			glm::ivec2 size,
			util::array_ref<ElementFormat> colorTargetFormats,
			ElementFormat depthTargetFormat);
		static RenderTarget::Ptr createNoDepth(
			glm::ivec2 size,
			util::array_ref<ElementFormat> colorTargetFormats);
		static RenderTarget &getDefaultRenderTarget();

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

		void commit(RenderQueue &renderQueue);

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

		static RenderTarget::Ptr default_rt;
	};

	class Renderer
	{
	public:
		void commit(
			RenderQueue &renderQueue2
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
using RenderTarget = gl4::RenderTarget;
using Stream = gl4::Stream;
using RenderQueue = gl4::RenderQueue;
using BufferDesc = gl4::BufferDesc;

 
#endif /* end of include guard: RENDERER_HPP */