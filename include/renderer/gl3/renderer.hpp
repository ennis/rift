#ifndef RENDERER_HPP
#define RENDERER_HPP
// OpenGL renderer

#include <opengl.hpp>
#include <shader.hpp>
#include <shadersource.hpp> // ShaderSource type

#include <resource.hpp>
#include <resourcemanager.hpp>

class Renderer;
class Shader;
class Texture;
class Texture2D;
class TextureCubeMap;

enum class PrimitiveType
{
    Point,
    Line,
    Triangle,
    TriangleStrip
};

enum class ElementFormat : int
{
    // 32x4
    Uint32x4,
    Sint32x4,
    Float4,
    // 32x3
    Uint32x3,
    Sint32x3,
    Float3,
    // 32x2
    Float2,
    // 16x4
    Uint16x4,
    Sint16x4,
    Unorm16x4,
    Snorm16x4,
    Float16x4,
    // 16x2
    Uint16x2,
    Sint16x2,
    Unorm16x2,
    Snorm16x2,
    Float16x2,
    // 8x4
    Uint8x4,
    Sint8x4,
    Unorm8x4,
    Snorm8x4,
    // 8x3
    Uint8x3,
    Sint8x3,
    Unorm8x3,
    Snorm8x3,
    // 8x2
    Uint8x2,
    Sint8x2,
    Unorm8x2,
    Snorm8x2,
    // 10_10_10_2
    Unorm10x3_1x2,
    Snorm10x3_1x2,
    // Compressed formats
    BC1,    // DXT1
    BC2,    // DXT3
    BC3,    // DXT5
    UnormBC4,
    SnormBC4,
    UnormBC5,
    SnormBC5,
    // Single
    Uint32,
    Sint32,
    Uint16,
    Sint16,
    Unorm16,
    Snorm16,
    //
    Uint8,
    Sint8,
    Unorm8,
    Snorm8,
    // TODO
    Depth32,
    Depth24,
    Depth16,
    Float16,
    Float,
    Max
};

enum class TextureFilter
{
    Nearest = 0,
    Linear
};

enum class TextureAddressMode
{
    Repeat = 0,
    Mirror,
    Clamp
};

enum class CullMode : int
{
    Front = 0,
    Back,
    FrontAndBack
};

enum class PolygonFillMode : int
{
    Fill = 0,
    Wireframe
};

struct RenderState
{
    CullMode cullMode;
    PolygonFillMode fillMode;
    bool depthTestEnable;
    bool depthWriteEnable;
};

const char *getElementFormatName(ElementFormat format);


// TODO simplified vertex layout creation without having to specify offset and strides
// TODO create vertex buffer from vertex layout declaration
// VertexBuffer = Buffer with VertexLayout attached
struct VertexElement
{
    VertexElement() = default;
    VertexElement(int inputSlot_, int bufferSlot_, int offset_, int stride_, ElementFormat format_) :
        inputSlot(inputSlot_), bufferSlot(bufferSlot_), offset(offset_), stride(stride_), format(format_)
    {}
    int inputSlot = 0;
    int bufferSlot = 0;
    int offset = 0;
    int stride = 0;
    ElementFormat format = ElementFormat::Float;
};

// TODO alignment issues?
struct VertexElement2
{
    VertexElement2() = default;
    VertexElement2(int bufferSlot_, ElementFormat format_) :
    bufferSlot(bufferSlot_),
    format(format_)
    {} 
    int bufferSlot = 0;
    ElementFormat format = ElementFormat::Float;
};

enum class ResourceUsage
{
    Static,
    Dynamic
};

class RenderResource : public Resource
{
public:
protected:
};

// Vertex layouts
class VertexLayout : public RenderResource
{
public:
    friend class Renderer;

    static const int kNumVertexElements = 16;

protected:
    int mNumElements;
    VertexElement mVertexElements[kNumVertexElements];
    GLuint mVAO;
};

// Texture
class Texture : public RenderResource
{
public:
    friend class Renderer;

    glm::ivec3 getSize() const
    {
        return mSize;
    }

protected:
    Texture(GLuint obj, GLenum bindingPoint, ElementFormat pixelFormat, int numMipLevels, glm::ivec3 size);

    GLuint mObj;
    GLenum mBindingPoint;
    int mNumMipLevels;
    ElementFormat mPixelFormat;
    glm::ivec3 mSize;
};

class Texture2D : public Texture
{
public:
    friend class Renderer;

protected:
    Texture2D(GLuint obj, ElementFormat pixelFormat, int numMipLevels, glm::ivec2 size);
};

class TextureCubeMap : public Texture
{
public:
    friend class Renderer;

protected:
    TextureCubeMap(GLuint obj, ElementFormat pixelFormat, int numMipLevels, glm::ivec3 size);
};

// OpenGL program
// is a resource
class Shader : public RenderResource
{
public:
    friend class Renderer;

    ~Shader();
	std::unique_ptr<uint8_t[]> getProgramBinary(int &binaryLength);

protected:
    Shader(GLuint program);

    GLuint mProgram = -1;
};

// Buffers
class Buffer : public RenderResource
{
public:
    friend class Renderer;

    std::size_t getSize() const
    {
        return mSize;
    }

    ResourceUsage getUsage() const
    {
        return mUsage;
    }

protected:
    Buffer(GLuint obj, std::size_t size, ResourceUsage usage);

    GLuint mObj;
    std::size_t mSize;
    ResourceUsage mUsage;
};

class VertexBuffer : public Buffer
{
public:
    friend class Renderer;

    int getNumVertices() const
    {
        return mNumVertices;
    }

protected:
    VertexBuffer(GLuint obj, int elementSize, int numVertices, ResourceUsage usage);

    int mElementSize;
    int mNumVertices;
};

class IndexBuffer : public Buffer
{
public:
    friend class Renderer;

    int getNumIndices() const
    {
        return mNumIndices;
    }

protected:
    IndexBuffer(GLuint obj, int elementSize, int numIndices, ResourceUsage usage);

    int mIndexSize;
    int mNumIndices;
};

class ConstantBuffer : public Buffer
{
public:
    friend class Renderer;

protected:
    ConstantBuffer(GLuint obj, std::size_t size, ResourceUsage usage);
};

class Sampler : public RenderResource
{
public:
    friend class Renderer;

protected:
    Sampler(GLuint obj);

    GLuint mObj;
};

class RenderTarget : public RenderResource
{
public:
    friend class Renderer;

private:
    RenderTarget(Texture2D *texture);

    Texture2D *mTexture;
};

struct Viewport
{
    float topLeftX;
    float topLeftY;
    float width;
    float height;
};

class Renderer
{
public:
    Renderer() = default;

    void initialize();

    // create a 2D texture object
    Texture2D *createTexture2D(
        glm::ivec2 size,
        int numMipLevels,
        ElementFormat pixelFormat,
        int nBytes,
        const void *pixels);

    // create a cube map texture object
    TextureCubeMap *createTextureCubeMap(
        glm::ivec2 size,
        int numMipLevels,
        ElementFormat pixelFormat,
        int nBytes,
        const void *facePixels[6]);

    // clear the bound render target
    void clearColor(glm::vec4 const &color);
    void clearDepth(float depth);

    // create a vertex buffer
    // if initialData is not null, fill the buffer with size bytes from initialData
    VertexBuffer *createVertexBuffer(int elemSize, int numVertices, ResourceUsage resourceUsage, const void *initialData);
    // TODO simplified
    // VertexBuffer *createVertexBuffer2(int numVertices, VertexLayout *layout, const void *initialData);

    // idem, for index buffers
    IndexBuffer *createIndexBuffer(int indexSize, int numIndices, ResourceUsage resourceUsage, const void *initialData);

    // idem for constant buffers
    ConstantBuffer *createConstantBuffer(int size, ResourceUsage usage, const void *initialData);

    // create a vertex layout for the input-assembler stage
    VertexLayout *createVertexLayout(int numElements, const VertexElement *vertexElements);
    // simplified version
    VertexLayout *createVertexLayout2(int numElements, const VertexElement2 *vertexElements);

    // create a shader from the specified sources
    Shader *createShader(const char *vertexShaderSource, const char *fragmentShaderSource);

    // create a texture sampler (TODO: params)
    Sampler *createSampler();

    // create a render target from a texture
    RenderTarget *createRenderTarget(Texture2D *texture);

    // update buffer
    void updateBuffer(Buffer *buffer, std::size_t offset, std::size_t size, const void *data);

    // update a texture
    void updateTexture2D(Texture2D *texture2D, int mipLevel, glm::ivec2 offset, glm::ivec2 size, int nBytes, const void *data);
    void updateTextureCubeMap(TextureCubeMap *textureCube, int face, int mipLevel, glm::ivec2 offset, glm::ivec2 size, int nBytes, const void *data);

    // shader binding
    void setShader(Shader *shader);

    // bind render targets
    void bindRenderTargets(
        int numRenderTargets,
        RenderTarget **colorTargets,
        RenderTarget *depthStencilTarget);

    // named interface (opengl only!)
    void setNamedConstantFloat(const char *name, float value);
    void setNamedConstantFloat2(const char *name, glm::vec2 values);
    void setNamedConstantFloat3(const char *name, glm::vec3 values);
    void setNamedConstantFloat4(const char *name, glm::vec4 values);
    void setNamedConstantMatrix4(const char *name, glm::mat4 matrix);
    void setNamedConstantInt(const char *name, int value);
    void setNamedConstantInt2(const char *name, glm::ivec2 values);
    void setNamedSampler(const char *name, Sampler *sampler);
    void setNamedConstantBuffer(const char *name, ConstantBuffer *buffer);

    // constant buffers
    void setConstantBuffer(int constantBufferSlot, ConstantBuffer *buffer);
    void setConstantBufferRange(int constantBufferSlot, ConstantBuffer *buffer, int offset, int size);

    // samplers
    void setSampler(int samplerSlot, Sampler *sampler);

    // texture binding
    void setTexture(int textureSlot, Texture *texture);
    void setVertexBuffer(int vertexBufferSlot, VertexBuffer *buffer);
    void setIndexBuffer(IndexBuffer *buffer);

    void setVertexLayout(VertexLayout *vertexLayout);


    // render states
    // cull mode, fill mode, etc.
    void setRenderStates(
        CullMode cullMode,
        PolygonFillMode polyFillMode,
        bool depthTestEnable,
        bool depthWriteEnable);

    // viewports
    void setViewports(
        int numViewports,
        const Viewport *viewports);

    // commands


    // draw direct, vertexBuffer may not be null, IndexBuffer may be null
    void draw(
        PrimitiveType primitiveType,
        int vertexOffset, int numVertices);

    void drawIndexed(
        PrimitiveType primitiveType,
        int vertexOffset, int numVertices,
        int indexOffset, int numIndices);

    static const int kNumSamplerSlots = 16;
    static const int kNumConstantBufferSlots = 16;
    static const int kNumVertexBufferSlots = 16;
    static const int kNumRenderTargetSlots = 8;

private:
    void setupVertexArrays();

    glm::vec4 mClearColor = glm::vec4(226.f / 255.f, 236.f / 255.f, 124.f / 255.f, 1.f);
    float mClearDepth = 100.f;

    Shader *mCurrentShader = nullptr;
    Sampler *mCurrentSamplers[kNumSamplerSlots];
    Texture *mCurrentTextures[kNumSamplerSlots];
    VertexLayout *mCurrentVertexLayout = nullptr;
    ConstantBuffer *mCurrentConstantBuffers[kNumConstantBufferSlots];
    VertexBuffer *mCurrentVertexBuffers[kNumVertexBufferSlots];
    IndexBuffer *mCurrentIndexBuffer = nullptr;
    RenderTarget *mCurrentRenderTargets[kNumRenderTargetSlots];
    int mNumRenderTargets = 0;

    GLuint mFBO;
};

#endif