#ifndef RENDERER_COMMON_HPP
#define RENDERER_COMMON_HPP
 
enum class PrimitiveType
{
    Point,
    Line,
    Triangle,
    TriangleStrip,
	Max
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
    Linear,
	Max
};

enum class TextureAddressMode : int
{
    Repeat = 0,
    Mirror,
    Clamp,
	Max
};

enum class CullMode : int
{
	None = 0,
    Front,
    Back,
    FrontAndBack,
	Max
};

enum class PolygonFillMode : int
{
    Fill = 0,
    Wireframe,
	Max
};

struct RasterizerDesc
{
    CullMode cullMode;
    PolygonFillMode fillMode;
	bool depthClipEnable;
};

struct DepthStencilDesc
{
	bool depthTestEnable = true;
	bool depthWriteEnable = true;
};

struct SamplerDesc
{
    TextureAddressMode addrU;
    TextureAddressMode addrV;
    TextureAddressMode addrW;
    TextureFilter minFilter;
    TextureFilter magFilter;
};

struct Viewport2
{
    float topLeftX;
    float topLeftY;
    float width;
    float height;
    float minDepth;
    float maxDepth;
};

enum class ResourceUsage
{
    Static,
    Dynamic
};

struct VertexElement2
{
    unsigned int inputSlot;
    unsigned int offset;
    ElementFormat format;
};

struct Attribute
{
    ElementFormat format;
    ResourceUsage usage;
};

enum class BufferUsage
{
	Unspecified,
	VertexBuffer,
	IndexBuffer,
	ConstantBuffer
};

struct Submesh {
	// Index du premier vertex 
	unsigned int startVertex;
	// Index du premier index (dans la table des indices)
	unsigned int startIndex;
	// Nombre de vertices dans la sous-mesh
	unsigned int numVertices;
	// Nombre d'indices
	unsigned int numIndices;
};

const char *getElementFormatName(ElementFormat format);
unsigned int getElementFormatSize(ElementFormat format);


template <typename ImplementationType>
class RendererObject
{
public:
    using Impl = ImplementationType;
	RendererObject() = default;

	RendererObject(Impl impl_) : impl(impl_)
	{}
    
    Impl &getImpl() {
        return impl;
    }

    const Impl &getImpl() const {
        return impl;
    }

protected:
    Impl impl;
};

#endif /* end of include guard: RENDERER_COMMON_HPP */
