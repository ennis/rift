#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>
#include <memory>

#include <renderer_common.hpp>
#include <gl_core_4_4.hpp>
#include <glm/glm.hpp>
#include <array_ref.hpp>
#include <unordered_map>
#include <image.hpp>
#include <asset.hpp>
#include <asset_database.hpp>
#include <mesh_data.hpp>

struct Buffer;
class Texture;
class Texture2D;
class TextureCubeMap;
class RenderTarget;
class VAO;
class GraphicsContext;

// constants 
// TODO do we need them?
constexpr auto kMaxUniformBufferBindings = 16u;
constexpr auto kMaxVertexBufferBindings = 16u;
constexpr auto kMaxTextureUnits = 16u;

//---------------------------
// misc.
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
void checkForUnusualColorFormats(ElementFormat f);
bool isSamplerType(GLenum type);

//---------------------------
// buffers
struct PoolBlockIndex
{
	unsigned page_index;
	unsigned block_index;
};
GLuint allocBufferRaw(GLenum target, size_t size, GLbitfield flags, const void *initialData);	
void updateBufferRaw(GLuint buf, GLenum target, int offset, int size, const void *data);

//---------------------------
// shader utils
struct ShaderKeyword
{
	std::string define;
	std::string value;
};
GLuint compileShader(
	const char *source,
	const char *include_path,
	GLenum stage,
	util::array_ref<ShaderKeyword> keywords);
GLuint compileProgram(GLuint vs, GLuint gs, GLuint ps);
std::string loadShaderSource(const char *path);

//---------------------------
// debug
void setDebugCallback();

//---------------------------
// binding helpers
void bindVertexBuffers(util::array_ref<const Buffer*> vbs, const VAO &vao);	
void bindBuffersRangeHelper(unsigned first, util::array_ref<const Buffer*> buffers);
void bindRenderTarget(RenderTarget *render_target);
void bindScreenRenderTarget();

//---------------------------
// draw helpers
void drawIndexed(
	GLenum mode,
	const Buffer &ib,
	unsigned firstVertex,
	unsigned firstIndex,
	unsigned indexCount,
	unsigned firstInstance,
	unsigned instanceCount);

//---------------------------
// VAO helper class
class VAO
{
public:
	VAO() = default;
	~VAO() {
		if (obj)
			gl::DeleteVertexArrays(1, &obj);
	}
	void create(unsigned num_buffers, util::array_ref<Attribute> attribs);
	GLuint obj = 0;
	util::small_vector<int, kMaxVertexBufferBindings> strides;
};

//---------------------------
// buffer helper
struct Buffer {
	using Ptr = std::unique_ptr < Buffer > ;
	Buffer(GraphicsContext &gc_) : gc(gc_)
	{}
	Buffer(const Buffer &) = delete;
	Buffer &operator=(const Buffer &) = delete; 
	~Buffer();
	
	GraphicsContext &gc;
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
struct TBuffer {
	T *map() {
		return static_cast<T*>(buf->ptr);
	}
	Buffer::Ptr buf;
};


template <typename T>
struct TTransientBuffer {
	T *map() {
		return static_cast<T*>(buf->ptr);
	}
	Buffer *buf;
};

// Texture assets
class Texture : public Asset {
public:
	enum Type
	{
		Tex2D,
		TexCubeMap
	};

	Texture(Type type_ = Tex2D) : type(type_)
	{}

	GLuint getGL() const {
		return id;
	}

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
	
// render targets
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

class GraphicsContext
{
public:
	void initialize();
	void beginFrame();
	void endFrame();
	void tearDown();

	// samplers
	GLuint getSamplerLinearClamp();
	GLuint getSamplerNearestClamp();
	GLuint getSamplerLinearRepeat();
	GLuint getSamplerNearestRepeat();

	// buffer_pool.cpp
	Buffer::Ptr createBuffer(GLenum target, size_t size, const void *initialData = nullptr);
	Buffer *createTransientBuffer(GLenum target, size_t size, const void *initialData = nullptr);
	void deleteBuffer(Buffer &buf);

	template <typename T>
	TTransientBuffer<T> createTransientBuffer() {
		return TTransientBuffer<T> { createTransientBuffer(gl::UNIFORM_BUFFER, sizeof(T)) };
	}

	// helper
	template <typename T>
	Buffer *createTransientBuffer(GLenum target, const T &data)
	{
		auto buf = createTransientBuffer(target, sizeof(T), &data);
		return buf;
	}

	unsigned getFrameCounter() const {
		return frame_counter;
	}

protected:
	// pools
	// 256 x 4096, 512 x 2048, 1024 x 1024, 2048 x 512, 4096 x 256, 8192 x 128,
	// 16384 x 64, 32768 x 32, 65536 x 16
	static const unsigned kNumPools = 9u;
	static const unsigned kMinBlockSizeLog = 8u;
	static const unsigned kMinBlockSize = 256u;
	static const unsigned kMaxBlockSize = kMinBlockSize << (kNumPools - 1);
	static const unsigned kPoolPageSize = 1024 * 1024ul; 
	static size_t blockSizeFromPoolIndex(unsigned index);
	// buffer pools
	Buffer::Ptr allocLargeBuffer(GLenum target, size_t size, GLbitfield flags_, const void *initialData);
	void createPool(unsigned pool_index);
	PoolBlockIndex allocFromPool(unsigned pool_index);
	void showPoolDebugInfo();
	struct BufferPool
	{
		BufferPool() = default;
		GLenum target = 0;
		GLuint obj = 0;
		// block size (multiple of GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT)
		size_t block_size = 0;
		// number of blocks
		size_t num_blocks = 0;
		// persistently mapped pointer
		void *mapped_ptr = nullptr;
	};
	std::vector<BufferPool> pools[kNumPools];
	std::vector<PoolBlockIndex> free_list[kNumPools];
	unsigned sync_cycle;
	std::vector<Buffer::Ptr> transient_buffers[3];
	GLsync transient_syncs[3];
	std::vector<Buffer::Ptr> static_buffers;
	void reclaimTransientBuffers();
	void syncTransientBuffers();
	// samplers
	GLuint samLinearClamp;
	GLuint samNearestClamp;
	GLuint samLinearRepeat;
	GLuint samNearestRepeat;
	GLuint dummy_vao;
	unsigned frame_counter;
};

enum class LightMode
{
	Directional,
	Spot,
	Point
};

struct Light
{
	LightMode mode;
	glm::vec3 intensity;
};

struct Mesh : public Asset
{
	using Ptr = std::unique_ptr<Mesh>;
	std::vector<Submesh> submeshes;
	Buffer::Ptr vbo;
	Buffer::Ptr ibo;
	unsigned nbvertex;
	unsigned nbindex;
};

std::unique_ptr<Mesh> createMesh(GraphicsContext &gc, MeshData &data);

enum class MaterialType
{
	Lambertian,
	Reflective,
	Glass
};

// A shader (collection of shader variants)
struct Shader : public Asset
{
	using Ptr = std::unique_ptr<Shader>;
	MaterialType materialType = MaterialType::Lambertian;
	GLuint programShadowStandard = 0;
	GLuint programForwardPointLight = 0;
	GLuint programForwardSpotLight = 0;
	GLuint programForwardDirectionalLight = 0;
	GLuint programDeferred = 0;
};

struct Material : public Asset
{
	using Ptr = std::unique_ptr<Material>;
	Material() = default;
	Shader *shader = nullptr;
	Texture2D *diffuseMap = nullptr;
	Texture2D *normalMap = nullptr;
	Buffer *userParams = nullptr;
};

Mesh *loadMeshAsset(AssetDatabase &assetDb, GraphicsContext &gc, std::string assetId);
Texture2D *loadTexture2DAsset(AssetDatabase &assetDb, GraphicsContext &gc, std::string assetId);
Shader *loadShaderAsset(AssetDatabase &assetDb, GraphicsContext &gc, std::string assetId);

#endif /* end of include guard: RENDERER_HPP */