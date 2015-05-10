#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>
#include <memory>

#include <renderer_common.hpp>
#include <gl_core_4_4.hpp>
#include <glm/glm.hpp>
#include <array_ref.hpp>
#include <unordered_map>
#include <unique_resource.hpp>
#include <small_vector.hpp>
#include <image.hpp>

namespace gl4
{
	struct ElementFormatInfoGL {
		// vertex element
		GLenum type;
		unsigned int size;
		// pixel element
		GLenum internalFormat;
		GLenum externalFormat;
		// 
		bool normalize;
		bool compressed;
	};

	const ElementFormatInfoGL &getElementFormatInfoGL(ElementFormat format);
	GLint textureFilterToGL(TextureFilter filter);
	GLint textureAddressModeToGL(TextureAddressMode addr);
	GLenum bufferUsageToBindingPoint(BufferUsage bufferUsage);
	GLenum blendOpToGL(BlendOp bo);
	GLenum blendFactorToGL(BlendFactor bf);
	GLenum cullModeToGLenum(CullMode mode);
	GLenum fillModeToGLenum(PolygonFillMode fillMode);
	GLenum primitiveTypeToGLenum(PrimitiveType type);
	GLenum stencilOpToGLenum(StencilOp op);
	GLenum stencilFuncToGLenum(StencilFunc func);
	GLenum shaderStageToGLenum(ShaderStage stage);

	class Texture;
	class Texture2D;
	class TextureCubeMap;
	class RenderTarget;
	struct Buffer;
	class ShaderProgram;
	class InputLayout;
	class CommandBuffer;
	class PipelineState;

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

	struct PoolBlockIndex
	{
		unsigned page_index;
		unsigned block_index;
	};

	// 
	void reclaimTransientBuffers();
	void syncTransientBuffers();
	void deleteBuffer(Buffer &buf);
	void setDebugCallback();
	void checkForUnusualColorFormats(ElementFormat f);
	bool isSamplerType(GLenum type);

	// constexpr
	constexpr auto kMaxUniformBufferBindings = 16u;
	constexpr auto kMaxVertexBufferBindings = 16u;
	constexpr auto kMaxTextureUnits = 16u;

	// WORKAROUND for vs2013
	// VS2013 does not support implicit generation of move constructors and move assignment operators
	// so the unique_resource pattern and the 'rule of zero' (http://flamingdangerzone.com/cxx11/2012/08/15/rule-of-zero.html)
	// is effectively useless

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
		GLuint vao;
		util::small_vector<int, kMaxVertexBufferBindings> strides;
	};

	struct Buffer
	{
		using Ptr = std::unique_ptr < Buffer > ;

		Buffer() = default;

		Buffer(const Buffer &) = delete;
		Buffer &operator=(const Buffer &) = delete;

		~Buffer()
		{
			deleteBuffer(*this);
		}
		
		GLenum target;
		GLuint obj;
		// -1 if not part of a pool
		unsigned pool_index;
		// (-1, -1) if not part of a pool
		PoolBlockIndex block_index;
		size_t offset;
		size_t size;
		void *ptr;
	};

	template <typename T>
	struct TBuffer
	{
		T *map() {
			return static_cast<T*>(buf->ptr);
		}

		Buffer::Ptr buf;
	};


	template <typename T>
	struct TTransientBuffer
	{
		T *map() {
			return static_cast<T*>(buf->ptr);
		}

		Buffer *buf;
	};

	class Texture
	{
	public:
		enum Type
		{
			Tex2D,
			TexCubeMap
		};

		Texture(Type type_ = Tex2D) : type(type_)
		{}

		Type type;
		GLuint id;
	};

	class Texture2D : public Texture
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

		static Ptr createFromImage(
			const Image &image
			);

		glm::ivec2 size;
		ElementFormat format;
		GLenum glformat;
	};

	class TextureCubeMap : public Texture
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

		static Ptr createFromImage(
			const Image &image
			);

		//protected:
		glm::ivec2 size;
		ElementFormat format;
		GLenum glformat;
	};

	class Sampler
	{
	public:
		using Ptr = std::unique_ptr<Sampler>;

		static Ptr create(
			TextureAddressMode addrU = TextureAddressMode::Clamp,
			TextureAddressMode addrV = TextureAddressMode::Clamp,
			TextureAddressMode addrW = TextureAddressMode::Clamp,
			TextureFilter minFilter = TextureFilter::Linear,
			TextureFilter magFilter = TextureFilter::Linear
			)
		{
			auto ptr = std::make_unique<Sampler>();
			gl::GenSamplers(1, &ptr->id);
			gl::SamplerParameteri(ptr->id, gl::TEXTURE_MIN_FILTER, textureFilterToGL(minFilter));
			gl::SamplerParameteri(ptr->id, gl::TEXTURE_MAG_FILTER, textureFilterToGL(magFilter));
			gl::SamplerParameteri(ptr->id, gl::TEXTURE_WRAP_R, textureAddressModeToGL(addrU));
			gl::SamplerParameteri(ptr->id, gl::TEXTURE_WRAP_S, textureAddressModeToGL(addrV));
			gl::SamplerParameteri(ptr->id, gl::TEXTURE_WRAP_T, textureAddressModeToGL(addrW));
			return std::move(ptr);
		}

	// protected:
		GLuint id;
	};

	class CommandBuffer
	{
	public:
		CommandBuffer() : writePtr(0)
		{
			cmdBuf.reserve(4096);
		}

		void setRenderTarget(const RenderTarget &rt);
		void setScreenRenderTarget();
		void clearColor(float color[4]);
		void clearDepth(float depth);

		void setVertexBuffers(
			util::array_ref<const Buffer*> vertexBuffers,
			const InputLayout &layout);

		void setConstantBuffers(
			util::array_ref<const Buffer*> constantBuffers);

		void setTextures(
			util::array_ref<const Texture*> textures,
			util::array_ref<const Sampler*> samplers);

		void setPipelineState(
			const PipelineState *pipelineState);	
		
		void setStencilRef(int8_t ref);

		void draw(
			PrimitiveType primitiveType,
			unsigned firstVertex,
			unsigned vertexCount,
			unsigned firstInstance,
			unsigned instanceCount
			);

		void drawIndexed(
			PrimitiveType primitiveType,
			const Buffer &indexBuffer,
			unsigned firstVertex,
			unsigned firstIndex,
			unsigned indexCount,
			unsigned firstInstance,
			unsigned instanceCount
			);

		void drawProcedural(
			PrimitiveType primitiveType,
			unsigned firstVertex,
			unsigned vertexCount,
			unsigned firstInstance,
			unsigned instanceCount
			);

	// protected:
		void write(const void *data, unsigned size);

		std::vector<char> cmdBuf;
		unsigned writePtr;
	};


	class ShaderProgram
	{
	public:
		using Ptr = std::unique_ptr<ShaderProgram>;

	// protected:
		// shader object
		GLuint shader;
		std::string source;
		ShaderStage stage;
	};

	class PipelineState
	{
	public:
		using Ptr = std::unique_ptr < PipelineState >;
		
		PipelineState(
			const ShaderProgram *vertexShader,
			const ShaderProgram *geometryShader,
			const ShaderProgram *pixelShader,
			const RasterizerDesc &rasterizerState,
			const DepthStencilDesc &depthStencilState,
			const BlendStateRenderTargetDesc &blendState);

		// the shader objects are not needed once the pipeline state is created
		static Ptr create(
			const ShaderProgram *vertexShader,
			const ShaderProgram *geometryShader,
			const ShaderProgram *pixelShader,
			const RasterizerDesc &rasterizerState,
			const DepthStencilDesc &depthStencilState,
			const BlendStateRenderTargetDesc &blendState)
		{
			return std::make_unique<PipelineState>(
				vertexShader,
				geometryShader,
				pixelShader,
				rasterizerState, 
				depthStencilState, 
				blendState);
		}

	//private:
		PipelineState() = default;
		~PipelineState()
		{
			if (program)
				gl::DeleteProgram(program);
		}

		unsigned cache_id;
		// shader source code
		//std::string source;
		GLuint program = 0;
		// XXX in source code?
		RasterizerDesc rs_state;
		DepthStencilDesc ds_state; 
		BlendStateRenderTargetDesc om_state;
		// TODO list of passes
		// TODO list of parameters / uniforms?
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

		Texture2D &getColorTexture(int index) {
			assert(index < color_targets.size());
			return *color_targets[index];
		}
		Texture2D &getDepthTexture() {
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

		//private:
		void init();

		GLuint fbo;
		glm::ivec2 size;
		util::small_vector<Texture2D::Ptr, kMaxColorRenderTargets> color_targets;
		Texture2D::Ptr depth_target;
	};
}

#endif /* end of include guard: RENDERER_HPP */