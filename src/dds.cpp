#include <image.hpp>
#include <fstream>
#include <cstring>
#include <log.hpp>
#include <algorithm>

//=============================================================================
// DDS enums & structs
enum DXGI_FORMAT { 
	DXGI_FORMAT_UNKNOWN                     = 0,
	DXGI_FORMAT_R32G32B32A32_TYPELESS       = 1,
	DXGI_FORMAT_R32G32B32A32_FLOAT          = 2,
	DXGI_FORMAT_R32G32B32A32_UINT           = 3,
	DXGI_FORMAT_R32G32B32A32_SINT           = 4,
	DXGI_FORMAT_R32G32B32_TYPELESS          = 5,
	DXGI_FORMAT_R32G32B32_FLOAT             = 6,
	DXGI_FORMAT_R32G32B32_UINT              = 7,
	DXGI_FORMAT_R32G32B32_SINT              = 8,
	DXGI_FORMAT_R16G16B16A16_TYPELESS       = 9,
	DXGI_FORMAT_R16G16B16A16_FLOAT          = 10,
	DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
	DXGI_FORMAT_R16G16B16A16_UINT           = 12,
	DXGI_FORMAT_R16G16B16A16_SNORM          = 13,
	DXGI_FORMAT_R16G16B16A16_SINT           = 14,
	DXGI_FORMAT_R32G32_TYPELESS             = 15,
	DXGI_FORMAT_R32G32_FLOAT                = 16,
	DXGI_FORMAT_R32G32_UINT                 = 17,
	DXGI_FORMAT_R32G32_SINT                 = 18,
	DXGI_FORMAT_R32G8X24_TYPELESS           = 19,
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
	DXGI_FORMAT_R10G10B10A2_TYPELESS        = 23,
	DXGI_FORMAT_R10G10B10A2_UNORM           = 24,
	DXGI_FORMAT_R10G10B10A2_UINT            = 25,
	DXGI_FORMAT_R11G11B10_FLOAT             = 26,
	DXGI_FORMAT_R8G8B8A8_TYPELESS           = 27,
	DXGI_FORMAT_R8G8B8A8_UNORM              = 28,
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
	DXGI_FORMAT_R8G8B8A8_UINT               = 30,
	DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
	DXGI_FORMAT_R8G8B8A8_SINT               = 32,
	DXGI_FORMAT_R16G16_TYPELESS             = 33,
	DXGI_FORMAT_R16G16_FLOAT                = 34,
	DXGI_FORMAT_R16G16_UNORM                = 35,
	DXGI_FORMAT_R16G16_UINT                 = 36,
	DXGI_FORMAT_R16G16_SNORM                = 37,
	DXGI_FORMAT_R16G16_SINT                 = 38,
	DXGI_FORMAT_R32_TYPELESS                = 39,
	DXGI_FORMAT_D32_FLOAT                   = 40,
	DXGI_FORMAT_R32_FLOAT                   = 41,
	DXGI_FORMAT_R32_UINT                    = 42,
	DXGI_FORMAT_R32_SINT                    = 43,
	DXGI_FORMAT_R24G8_TYPELESS              = 44,
	DXGI_FORMAT_D24_UNORM_S8_UINT           = 45,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS       = 46,
	DXGI_FORMAT_X24_TYPELESS_G8_UINT        = 47,
	DXGI_FORMAT_R8G8_TYPELESS               = 48,
	DXGI_FORMAT_R8G8_UNORM                  = 49,
	DXGI_FORMAT_R8G8_UINT                   = 50,
	DXGI_FORMAT_R8G8_SNORM                  = 51,
	DXGI_FORMAT_R8G8_SINT                   = 52,
	DXGI_FORMAT_R16_TYPELESS                = 53,
	DXGI_FORMAT_R16_FLOAT                   = 54,
	DXGI_FORMAT_D16_UNORM                   = 55,
	DXGI_FORMAT_R16_UNORM                   = 56,
	DXGI_FORMAT_R16_UINT                    = 57,
	DXGI_FORMAT_R16_SNORM                   = 58,
	DXGI_FORMAT_R16_SINT                    = 59,
	DXGI_FORMAT_R8_TYPELESS                 = 60,
	DXGI_FORMAT_R8_UNORM                    = 61,
	DXGI_FORMAT_R8_UINT                     = 62,
	DXGI_FORMAT_R8_SNORM                    = 63,
	DXGI_FORMAT_R8_SINT                     = 64,
	DXGI_FORMAT_A8_UNORM                    = 65,
	DXGI_FORMAT_R1_UNORM                    = 66,
	DXGI_FORMAT_R9G9B9E5_SHAREDEXP          = 67,
	DXGI_FORMAT_R8G8_B8G8_UNORM             = 68,
	DXGI_FORMAT_G8R8_G8B8_UNORM             = 69,
	DXGI_FORMAT_BC1_TYPELESS                = 70,
	DXGI_FORMAT_BC1_UNORM                   = 71,
	DXGI_FORMAT_BC1_UNORM_SRGB              = 72,
	DXGI_FORMAT_BC2_TYPELESS                = 73,
	DXGI_FORMAT_BC2_UNORM                   = 74,
	DXGI_FORMAT_BC2_UNORM_SRGB              = 75,
	DXGI_FORMAT_BC3_TYPELESS                = 76,
	DXGI_FORMAT_BC3_UNORM                   = 77,
	DXGI_FORMAT_BC3_UNORM_SRGB              = 78,
	DXGI_FORMAT_BC4_TYPELESS                = 79,
	DXGI_FORMAT_BC4_UNORM                   = 80,
	DXGI_FORMAT_BC4_SNORM                   = 81,
	DXGI_FORMAT_BC5_TYPELESS                = 82,
	DXGI_FORMAT_BC5_UNORM                   = 83,
	DXGI_FORMAT_BC5_SNORM                   = 84,
	DXGI_FORMAT_B5G6R5_UNORM                = 85,
	DXGI_FORMAT_B5G5R5A1_UNORM              = 86,
	DXGI_FORMAT_B8G8R8A8_UNORM              = 87,
	DXGI_FORMAT_B8G8R8X8_UNORM              = 88,
	DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  = 89,
	DXGI_FORMAT_B8G8R8A8_TYPELESS           = 90,
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
	DXGI_FORMAT_B8G8R8X8_TYPELESS           = 92,
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
	DXGI_FORMAT_BC6H_TYPELESS               = 94,
	DXGI_FORMAT_BC6H_UF16                   = 95,
	DXGI_FORMAT_BC6H_SF16                   = 96,
	DXGI_FORMAT_BC7_TYPELESS                = 97,
	DXGI_FORMAT_BC7_UNORM                   = 98,
	DXGI_FORMAT_BC7_UNORM_SRGB              = 99,
	DXGI_FORMAT_AYUV                        = 100,
	DXGI_FORMAT_Y410                        = 101,
	DXGI_FORMAT_Y416                        = 102,
	DXGI_FORMAT_NV12                        = 103,
	DXGI_FORMAT_P010                        = 104,
	DXGI_FORMAT_P016                        = 105,
	DXGI_FORMAT_420_OPAQUE                  = 106,
	DXGI_FORMAT_YUY2                        = 107,
	DXGI_FORMAT_Y210                        = 108,
	DXGI_FORMAT_Y216                        = 109,
	DXGI_FORMAT_NV11                        = 110,
	DXGI_FORMAT_AI44                        = 111,
	DXGI_FORMAT_IA44                        = 112,
	DXGI_FORMAT_P8                          = 113,
	DXGI_FORMAT_A8P8                        = 114,
	DXGI_FORMAT_B4G4R4A4_UNORM              = 115,
	DXGI_FORMAT_FORCE_UINT                  = 0xffffffffUL
};


#define MAKEFOURCC2(ch0, ch1, ch2, ch3) \
		(uint32_t)( \
		(((uint32_t)(uint8_t)(ch3) << 24) & 0xFF000000) | \
		(((uint32_t)(uint8_t)(ch2) << 16) & 0x00FF0000) | \
		(((uint32_t)(uint8_t)(ch1) <<  8) & 0x0000FF00) | \
		((uint32_t)(uint8_t)(ch0) & 0x000000FF) )

enum D3DFORMAT {
		D3DFMT_UNKNOWN              =  0,

		D3DFMT_R8G8B8               = 20,
		D3DFMT_A8R8G8B8             = 21,
		D3DFMT_X8R8G8B8             = 22,
		D3DFMT_R5G6B5               = 23,
		D3DFMT_X1R5G5B5             = 24,
		D3DFMT_A1R5G5B5             = 25,
		D3DFMT_A4R4G4B4             = 26,
		D3DFMT_R3G3B2               = 27,
		D3DFMT_A8                   = 28,
		D3DFMT_A8R3G3B2             = 29,
		D3DFMT_X4R4G4B4             = 30,
		D3DFMT_A2B10G10R10          = 31,
		D3DFMT_A8B8G8R8             = 32,
		D3DFMT_X8B8G8R8             = 33,
		D3DFMT_G16R16               = 34,
		D3DFMT_A2R10G10B10          = 35,
		D3DFMT_A16B16G16R16         = 36,

		D3DFMT_A8P8                 = 40,
		D3DFMT_P8                   = 41,

		D3DFMT_L8                   = 50,
		D3DFMT_A8L8                 = 51,
		D3DFMT_A4L4                 = 52,

		D3DFMT_V8U8                 = 60,
		D3DFMT_L6V5U5               = 61,
		D3DFMT_X8L8V8U8             = 62,
		D3DFMT_Q8W8V8U8             = 63,
		D3DFMT_V16U16               = 64,
		D3DFMT_A2W10V10U10          = 67,

		D3DFMT_UYVY                 = MAKEFOURCC2('U', 'Y', 'V', 'Y'),
		D3DFMT_R8G8_B8G8            = MAKEFOURCC2('R', 'G', 'B', 'G'),
		D3DFMT_YUY2                 = MAKEFOURCC2('Y', 'U', 'Y', '2'),
		D3DFMT_G8R8_G8B8            = MAKEFOURCC2('G', 'R', 'G', 'B'),
		D3DFMT_DXT1                 = MAKEFOURCC2('D', 'X', 'T', '1'),
		D3DFMT_DXT2                 = MAKEFOURCC2('D', 'X', 'T', '2'),
		D3DFMT_DXT3                 = MAKEFOURCC2('D', 'X', 'T', '3'),
		D3DFMT_DXT4                 = MAKEFOURCC2('D', 'X', 'T', '4'),
		D3DFMT_DXT5                 = MAKEFOURCC2('D', 'X', 'T', '5'),

		D3DFMT_D16_LOCKABLE         = 70,
		D3DFMT_D32                  = 71,
		D3DFMT_D15S1                = 73,
		D3DFMT_D24S8                = 75,
		D3DFMT_D24X8                = 77,
		D3DFMT_D24X4S4              = 79,
		D3DFMT_D16                  = 80,

		D3DFMT_D32F_LOCKABLE        = 82,
		D3DFMT_D24FS8               = 83,

		D3DFMT_D32_LOCKABLE         = 84,
		D3DFMT_S8_LOCKABLE          = 85,

		D3DFMT_L16                  = 81,

		D3DFMT_VERTEXDATA           =100,
		D3DFMT_INDEX16              =101,
		D3DFMT_INDEX32              =102,

		D3DFMT_Q16W16V16U16         =110,

		D3DFMT_MULTI2_ARGB8         = MAKEFOURCC2('M','E','T','1'),

		D3DFMT_R16F                 = 111,
		D3DFMT_G16R16F              = 112,
		D3DFMT_A16B16G16R16F        = 113,

		D3DFMT_R32F                 = 114,
		D3DFMT_G32R32F              = 115,
		D3DFMT_A32B32G32R32F        = 116,

		D3DFMT_CxV8U8               = 117,

		D3DFMT_A1                   = 118,
		D3DFMT_A2B10G10R10_XR_BIAS  = 119,
		D3DFMT_BINARYBUFFER         = 199,

		D3DFMT_DX10                 = MAKEFOURCC2('D', 'X', '1', '0'),

		D3DFMT_FORCE_DWORD          =0x7fffffff
};


enum ddsCubemapflag
{
	DDSCAPS2_CUBEMAP				= 0x00000200,
	DDSCAPS2_CUBEMAP_POSITIVEX		= 0x00000400,
	DDSCAPS2_CUBEMAP_NEGATIVEX		= 0x00000800,
	DDSCAPS2_CUBEMAP_POSITIVEY		= 0x00001000,
	DDSCAPS2_CUBEMAP_NEGATIVEY		= 0x00002000,
	DDSCAPS2_CUBEMAP_POSITIVEZ		= 0x00004000,
	DDSCAPS2_CUBEMAP_NEGATIVEZ		= 0x00008000,
	DDSCAPS2_VOLUME					= 0x00200000
};

const uint32_t DDSCAPS2_CUBEMAP_ALLFACES = (
	DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_NEGATIVEX |
	DDSCAPS2_CUBEMAP_POSITIVEY | DDSCAPS2_CUBEMAP_NEGATIVEY |
	DDSCAPS2_CUBEMAP_POSITIVEZ | DDSCAPS2_CUBEMAP_NEGATIVEZ);

enum ddsFlag
{
	DDSD_CAPS			= 0x00000001,
	DDSD_HEIGHT			= 0x00000002,
	DDSD_WIDTH			= 0x00000004,
	DDSD_PITCH			= 0x00000008,
	DDSD_PIXELFORMAT	= 0x00001000,
	DDSD_MIPMAPCOUNT	= 0x00020000,
	DDSD_LINEARSIZE		= 0x00080000,
	DDSD_DEPTH			= 0x00800000
};

enum ddsSurfaceflag
{
	DDSCAPS_COMPLEX				= 0x00000008,
	DDSCAPS_MIPMAP				= 0x00400000,
	DDSCAPS_TEXTURE				= 0x00001000
};

enum ddsPixelFormatFlags
{
	DDPF_ALPHAPIXELS = 0x1,
	DDPF_ALPHA = 0x2,
	DDPF_FOURCC = 0x4,
	DDPF_RGB = 0x40,
	DDPF_YUV = 0x200,
	DDPF_LUMINANCE = 0x20000
};

struct DDS_PIXELFORMAT
{
	uint32_t size; // 32
	uint32_t flags;
	uint32_t fourCC;
	uint32_t bpp;
	uint32_t redMask;
	uint32_t greenMask;
	uint32_t blueMask;
	uint32_t alphaMask;
};

struct DDS_HEADER
{
	uint32_t size;
	uint32_t flags;
	uint32_t height;
	uint32_t width;
	uint32_t pitch;
	uint32_t depth;
	uint32_t mipMapLevels;
	uint32_t reserved1[11];
	DDS_PIXELFORMAT format;
	uint32_t surfaceFlags;
	uint32_t cubemapFlags;
	uint32_t reserved2[3];
};

enum D3D10_RESOURCE_DIMENSION 
{
	D3D10_RESOURCE_DIMENSION_UNKNOWN     = 0,
	D3D10_RESOURCE_DIMENSION_BUFFER      = 1,
	D3D10_RESOURCE_DIMENSION_TEXTURE1D   = 2,
	D3D10_RESOURCE_DIMENSION_TEXTURE2D   = 3,
	D3D10_RESOURCE_DIMENSION_TEXTURE3D   = 4 
};

enum D3D10_RESOURCE_MISC_FLAG 
{
	D3D10_RESOURCE_MISC_GENERATE_MIPS       = 0x1L,
	D3D10_RESOURCE_MISC_SHARED              = 0x2L,
	D3D10_RESOURCE_MISC_TEXTURECUBE         = 0x4L,
	D3D10_RESOURCE_MISC_SHARED_KEYEDMUTEX   = 0x10L,
	D3D10_RESOURCE_MISC_GDI_COMPATIBLE      = 0x20L 
};

struct DDS_HEADER_DXT10
{
	DDS_HEADER_DXT10() :
		format(DXGI_FORMAT_UNKNOWN),
		resourceDimension(D3D10_RESOURCE_DIMENSION_UNKNOWN),
		miscFlag(0),
		arraySize(1),
		reserved(0)
	{}

	DXGI_FORMAT format;
	D3D10_RESOURCE_DIMENSION resourceDimension;
	uint32_t miscFlag; // D3D10_RESOURCE_MISC_GENERATE_MIPS
	uint32_t arraySize;
	uint32_t reserved;
};

//=============================================================================
static ElementFormat FourCCToPixelFormat(uint32_t flags, uint32_t FourCC)
{
	switch(FourCC)
	{
	case D3DFMT_DXT1:
		return ElementFormat::BC1;
	case D3DFMT_DXT2:
	case D3DFMT_DXT3:
		return ElementFormat::BC2;
	case D3DFMT_DXT4:
	case D3DFMT_DXT5:
		return ElementFormat::BC3;
	/*case D3DFMT_ATI1:
	case D3DFMT_AT1N:
		return ElementFormat::Max;		// TODO
	case D3DFMT_ATI2:
	case D3DFMT_AT2N:*/
	case D3DFMT_R16F:
		return ElementFormat::Float16;
	case D3DFMT_G16R16F:
		return ElementFormat::Float16x2;
	case D3DFMT_A16B16G16R16F:
		return ElementFormat::Float16x4;
	case D3DFMT_R32F:
		return ElementFormat::Float;
	case D3DFMT_G32R32F:
		return ElementFormat::Float2;
	case D3DFMT_A32B32G32R32F:
		return ElementFormat::Float4;
	case D3DFMT_R8G8B8:
		return ElementFormat::Uint8x3;
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
	case D3DFMT_A8B8G8R8:
	case D3DFMT_X8B8G8R8:
		return ElementFormat::Uint8x4;
	case D3DFMT_R5G6B5:
		return ElementFormat::Max;		// XXX
	case D3DFMT_A4R4G4B4:
	case D3DFMT_X4R4G4B4:
		return ElementFormat::Max;		// XXX blah
	case D3DFMT_G16R16:
		return ElementFormat::Uint16x2;
	case D3DFMT_A16B16G16R16:
		return ElementFormat::Uint16x4;
	case D3DFMT_A2R10G10B10:
	case D3DFMT_A2B10G10R10:
		return ElementFormat::Unorm10x3_1x2;
	default:
		assert(0);
		return ElementFormat::Max;
	}
}

//=============================================================================
ElementFormat DXGIToPixelFormat(DXGI_FORMAT dxgi_format)
{
	static const ElementFormat cast[] = 
	{
		ElementFormat::Max,	//DXGI_FORMAT_UNKNOWN                      = 0,
		ElementFormat::Uint32x4,		//DXGI_FORMAT_R32G32B32A32_TYPELESS        = 1,
		ElementFormat::Float4,		//DXGI_FORMAT_R32G32B32A32_FLOAT           = 2,
		ElementFormat::Uint32x4,		//DXGI_FORMAT_R32G32B32A32_UINT            = 3,
		ElementFormat::Sint32x4,		//DXGI_FORMAT_R32G32B32A32_SINT            = 4,
		ElementFormat::Uint32x3,			//DXGI_FORMAT_R32G32B32_TYPELESS           = 5,
		ElementFormat::Float3,			//DXGI_FORMAT_R32G32B32_FLOAT              = 6,
		ElementFormat::Uint32x3,			//DXGI_FORMAT_R32G32B32_UINT               = 7,
		ElementFormat::Sint32x3,			//DXGI_FORMAT_R32G32B32_SINT               = 8,
		ElementFormat::Uint16x4,		//DXGI_FORMAT_R16G16B16A16_TYPELESS        = 9,
		ElementFormat::Float16x4,		//DXGI_FORMAT_R16G16B16A16_FLOAT           = 10,
		ElementFormat::Unorm16x4,		//DXGI_FORMAT_R16G16B16A16_UNORM           = 11,
		ElementFormat::Uint16x4,		//DXGI_FORMAT_R16G16B16A16_UINT            = 12,
		ElementFormat::Snorm16x4,		//DXGI_FORMAT_R16G16B16A16_SNORM           = 13,
		ElementFormat::Sint16x4,		//DXGI_FORMAT_R16G16B16A16_SINT            = 14,
		ElementFormat::Max,			//DXGI_FORMAT_R32G32_TYPELESS              = 15,
		ElementFormat::Float2,			//DXGI_FORMAT_R32G32_FLOAT                 = 16,
		ElementFormat::Max,			//DXGI_FORMAT_R32G32_UINT                  = 17,
		ElementFormat::Max,			//DXGI_FORMAT_R32G32_SINT                  = 18,
		ElementFormat::Max,      //DXGI_FORMAT_R32G8X24_TYPELESS            = 19,
		ElementFormat::Max,      //DXGI_FORMAT_D32_FLOAT_S8X24_UINT         = 20,
		ElementFormat::Max,      //DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS     = 21,
		ElementFormat::Max,      //DXGI_FORMAT_X32_TYPELESS_G8X24_UINT      = 22,
		ElementFormat::Unorm10x3_1x2,		//DXGI_FORMAT_R10G10B10A2_TYPELESS         = 23,
		ElementFormat::Unorm10x3_1x2,		//DXGI_FORMAT_R10G10B10A2_UNORM            = 24,
		ElementFormat::Max,		//DXGI_FORMAT_R10G10B10A2_UINT             = 25,
		ElementFormat::Max,		//DXGI_FORMAT_R11G11B10_FLOAT              = 26,
		ElementFormat::Uint8x4,			//DXGI_FORMAT_R8G8B8A8_TYPELESS            = 27,
		ElementFormat::Unorm8x4,			//DXGI_FORMAT_R8G8B8A8_UNORM               = 28,
		ElementFormat::Unorm8x4,			//DXGI_FORMAT_R8G8B8A8_UNORM_SRGB          = 29,
		ElementFormat::Uint8x4,			//DXGI_FORMAT_R8G8B8A8_UINT                = 30,
		ElementFormat::Snorm8x4,			//DXGI_FORMAT_R8G8B8A8_SNORM               = 31,
		ElementFormat::Sint8x4,			//DXGI_FORMAT_R8G8B8A8_SINT                = 32,
		ElementFormat::Max,			//DXGI_FORMAT_R16G16_TYPELESS              = 33,
		ElementFormat::Max,			//DXGI_FORMAT_R16G16_FLOAT                 = 34,
		ElementFormat::Max,			//DXGI_FORMAT_R16G16_UNORM                 = 35,
		ElementFormat::Max,			//DXGI_FORMAT_R16G16_UINT                  = 36,
		ElementFormat::Max,			//DXGI_FORMAT_R16G16_SNORM                 = 37,
		ElementFormat::Max,			//DXGI_FORMAT_R16G16_SINT                  = 38,
		ElementFormat::Max,			//DXGI_FORMAT_R32_TYPELESS                 = 39,
		ElementFormat::Depth32,			//DXGI_FORMAT_D32_FLOAT                    = 40,
		ElementFormat::Max,			//DXGI_FORMAT_R32_FLOAT                    = 41,
		ElementFormat::Max,			//DXGI_FORMAT_R32_UINT                     = 42,
		ElementFormat::Max,			//DXGI_FORMAT_R32_SINT                     = 43,
		ElementFormat::Max,	//DXGI_FORMAT_R24G8_TYPELESS               = 44,
		ElementFormat::Max,	//DXGI_FORMAT_D24_UNORM_S8_UINT            = 45,
		ElementFormat::Max,	//DXGI_FORMAT_R24_UNORM_X8_TYPELESS        = 46,
		ElementFormat::Max,	//DXGI_FORMAT_X24_TYPELESS_G8_UINT         = 47,
		ElementFormat::Max,			//DXGI_FORMAT_R8G8_TYPELESS                = 48,
		ElementFormat::Max,			//DXGI_FORMAT_R8G8_UNORM                   = 49,
		ElementFormat::Max,			//DXGI_FORMAT_R8G8_UINT                    = 50,
		ElementFormat::Max,			//DXGI_FORMAT_R8G8_SNORM                   = 51,
		ElementFormat::Max,			//DXGI_FORMAT_R8G8_SINT                    = 52,
		ElementFormat::Max,			//DXGI_FORMAT_R16_TYPELESS                 = 53,
		ElementFormat::Max,			//DXGI_FORMAT_R16_FLOAT                    = 54,
		ElementFormat::Depth16,			//DXGI_FORMAT_D16_UNORM                    = 55,
		ElementFormat::Unorm16,			//DXGI_FORMAT_R16_UNORM                    = 56,
		ElementFormat::Uint16,		//DXGI_FORMAT_R16_UINT                     = 57,
		ElementFormat::Max,			//DXGI_FORMAT_R16_SNORM                    = 58,
		ElementFormat::Max,			//DXGI_FORMAT_R16_SINT                     = 59,
		ElementFormat::Max,			//DXGI_FORMAT_R8_TYPELESS                  = 60,
		ElementFormat::Max,			//DXGI_FORMAT_R8_UNORM                     = 61,
		ElementFormat::Max,			//DXGI_FORMAT_R8_UINT                      = 62,
		ElementFormat::Max,			//DXGI_FORMAT_R8_SNORM                     = 63,
		ElementFormat::Max,			//DXGI_FORMAT_R8_SINT                      = 64,
		ElementFormat::Max,			//DXGI_FORMAT_A8_UNORM                     = 65,
		ElementFormat::Max,	//DXGI_FORMAT_R1_UNORM                     = 66,
		ElementFormat::Max,			//DXGI_FORMAT_R9G9B9E5_SHAREDEXP           = 67,
		ElementFormat::Max,		//DXGI_FORMAT_R8G8_B8G8_UNORM              = 68,
		ElementFormat::Max,		//DXGI_FORMAT_G8R8_G8B8_UNORM              = 69,
		ElementFormat::Max,			//DXGI_FORMAT_BC1_TYPELESS                 = 70,
		ElementFormat::Max,			//DXGI_FORMAT_BC1_UNORM                    = 71,
		ElementFormat::Max,			//DXGI_FORMAT_BC1_UNORM_SRGB               = 72,
		ElementFormat::Max,			//DXGI_FORMAT_BC2_TYPELESS                 = 73,
		ElementFormat::Max,			//DXGI_FORMAT_BC2_UNORM                    = 74,
		ElementFormat::Max,			//DXGI_FORMAT_BC2_UNORM_SRGB               = 75,
		ElementFormat::Max,			//DXGI_FORMAT_BC3_TYPELESS                 = 76,
		ElementFormat::Max,			//DXGI_FORMAT_BC3_UNORM                    = 77,
		ElementFormat::Max,			//DXGI_FORMAT_BC3_UNORM_SRGB               = 78,
		ElementFormat::Max,		//DXGI_FORMAT_BC4_TYPELESS                 = 79,
		ElementFormat::Max,		//DXGI_FORMAT_BC4_UNORM                    = 80,
		ElementFormat::Max,		//DXGI_FORMAT_BC4_SNORM                    = 81,
		ElementFormat::Max,	//DXGI_FORMAT_BC5_TYPELESS                 = 82,
		ElementFormat::Max,	//DXGI_FORMAT_BC5_UNORM                    = 83,
		ElementFormat::Max,	//DXGI_FORMAT_BC5_SNORM                    = 84,
		ElementFormat::Max,		//DXGI_FORMAT_B5G6R5_UNORM                 = 85,
		ElementFormat::Max,		//DXGI_FORMAT_B5G5R5A1_UNORM               = 86,
		ElementFormat::Max,			//DXGI_FORMAT_B8G8R8A8_UNORM               = 87,
		ElementFormat::Max,				//DXGI_FORMAT_B8G8R8X8_UNORM               = 88,
		ElementFormat::Max,		//DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM   = 89,
		ElementFormat::Max,			//DXGI_FORMAT_B8G8R8A8_TYPELESS            = 90,
		ElementFormat::Max,			//DXGI_FORMAT_B8G8R8A8_UNORM_SRGB          = 91,
		ElementFormat::Max,				//DXGI_FORMAT_B8G8R8X8_TYPELESS            = 92,
		ElementFormat::Max,						//DXGI_FORMAT_B8G8R8X8_UNORM_SRGB          = 93,
		ElementFormat::Max,		//DXGI_FORMAT_BC6H_TYPELESS                = 94,
		ElementFormat::Max,		//DXGI_FORMAT_BC6H_UF16                    = 95,
		ElementFormat::Max,		//DXGI_FORMAT_BC6H_SF16                    = 96,
		ElementFormat::Max,				//DXGI_FORMAT_BC7_TYPELESS                 = 97,
		ElementFormat::Max,				//DXGI_FORMAT_BC7_UNORM                    = 98,
		ElementFormat::Max,				//DXGI_FORMAT_BC7_UNORM_SRGB               = 99,
		ElementFormat::Max						//DXGI_FORMAT_FORCE_UINT                   = 0xffffffffUL 
	};

	return cast[(int)dxgi_format];
}

//=============================================================================
static const int sElementFormatBits[(int)ElementFormat::Max] = 
{
		/*Uint32x4*/   128,
		/*Sint32x4*/   128,
		/*Float4*/     128,
		/*Uint32x3*/   96, 
		/*Sint32x3*/   96, 
		/*Float3*/     96,
		/*Float2*/     64,
		/*Uint16x4*/   64, 
		/*Sint16x4*/   64, 
		/*Unorm16x4*/  64,
		/*Snorm16x4*/  64,
		/*Float16x4*/  64,
		/*Uint16x2*/   32,
		/*Sint16x2*/   32,
		/*Unorm16x2*/  32,
		/*Snorm16x2*/  32,
		/*Float16x2*/  32,    // XXX GL_HALF_FLOAT_ARB 
		/*Uint8x4*/    32,
		/*Sint8x4*/    32,
		/*Unorm8x4*/   32,
		/*Snorm8x4*/   32,
		/*Uint8x3*/    24,
		/*Sint8x3*/    24,
		/*Unorm8x3*/   24,
		/*Snorm8x3*/   24,
		/*Uint8x2*/    16,
		/*Sint8x2*/    16,
		/*Unorm8x2*/   16,
		/*Snorm8x2*/   16,
		/*Unorm10x3_1x2*/ 32,
		/*Snorm10x3_1x2*/ 32,
		/*BC1*/        0,
		/*BC2*/        0,
		/*BC3*/        0,
		/*UnormBC4*/   0,   // TODO kkk
		/*SnormBC4*/   0,
		/*UnormBC5*/   0,
		/*SnormBC5*/   0,
		/*Uint32*/     32,
		/*Sint32*/     32,
		/*Uint16*/     16,
		/*Sint16*/     16,
		/*Unorm16*/    16,
		/*Snorm16*/    16,
		/*Uint8*/      8,
		/*Sint8*/      8,
		/*Unorm8*/     8,
		/*Snorm8*/     8,
		/*Float16*/    16,
		/*Float*/      32
};

//=============================================================================
static inline int mipSize(ElementFormat fmt, int width, int height)
{
	int size;
	if (fmt == ElementFormat::BC1) {
		int ww = std::max(1, ((width + 3) / 4));
		int hh = std::max(1, ((height + 3) / 4));
		size = ww * hh * 8;
	} else if (fmt == ElementFormat::BC2 || fmt == ElementFormat::BC3) {
		int ww = std::max(1, ((width + 3) / 4));
		int hh = std::max(1, ((height + 3) / 4));
		size = ww * hh * 16;
	} else {
		size = height * ((width * sElementFormatBits[(int)fmt] + 7) / 8);
	}
	return size;
}

//=============================================================================
void Image::loadDDS(std::istream &streamIn)
{
	// read magic ('DDS ')
	char magic[4];
	streamIn.read(magic, 4);
	assert(!strncmp(magic, "DDS ", 4));
	// read header
	DDS_HEADER header;
	DDS_HEADER_DXT10 header10;
	bool hasDX10Header = false;
	streamIn.read((char*)&header, sizeof(header));
	// DX10 header?
	if (header.format.flags & DDPF_FOURCC && header.format.fourCC == D3DFMT_DX10) {
		streamIn.read((char*)&header10, sizeof(header10));
		hasDX10Header = true;
	}
	mFormat = ElementFormat::Max;
	if (header.format.fourCC == D3DFMT_DX10) {
		// pixel format from DXGI format
		LOG << "DDS: DX10";
		mFormat = DXGIToPixelFormat(header10.format);
	}
	else if(header.format.flags & DDPF_FOURCC) {
		// pixel format from fourCC
		LOG << "DDS: FOURCC";
		mFormat = FourCCToPixelFormat(header.flags, header.format.fourCC);
	}
	else if(header.format.flags & DDPF_RGB) {
		LOG << "DDS: RGB desc";
		// TODO pixel format from RGB descriptor
	}
	// verify that the format is supported
	assert(mFormat != ElementFormat::Max);
	LOG << "DDS: format = " << getElementFormatName(mFormat);
	mMainSize.x = header.width;
	mMainSize.y = header.height;
	mNumMipLevels = (header.mipMapLevels == 0) ? 1 : header.mipMapLevels;
	if (hasDX10Header) {
		if (header10.arraySize > 1) {
			LOG << "DDS: DX10 texture array";
		}
	}
	bool isCubemap = false;
	if (header.cubemapFlags & DDSCAPS2_CUBEMAP) {
		isCubemap = true;
		LOG << "DDS: texture is cube map";
		// TODO
		mNumFaces = 1;
	} else {
		mNumFaces = 1;
	}
	// TODO cube maps
	// calculate total data size
	mDataSize = 0;
	int cw = mMainSize.x;
	int ch = mMainSize.y;
	int iFace = 0;
	for (int iMip = 0; iMip < mNumMipLevels; iMip++) {
		int nBytes = mipSize(mFormat, cw, ch);
		LOG << "DDS: mip " << iMip << " " << cw << 'x' << ch << " nBytes = " << nBytes;
		mMipMaps[iMip][iFace].offset = mDataSize;
		mMipMaps[iMip][iFace].bytes = nBytes;
		mMipMaps[iMip][iFace].size = glm::ivec2(cw, ch);
		mDataSize += nBytes;
		cw /= 2;
		ch /= 2;
	}
	// load data
	mMipData = std::unique_ptr<PtrWrap>(new DefaultPtrWrap(new unsigned char[mDataSize]));
	streamIn.read((char*)mMipData.get()->ptr, mDataSize);
}

//=============================================================================
Texture2D *loadTexture2D_DDS(Renderer &renderer, const char *ddsFilePath)
{
	Image ddsFile;
	ddsFile.loadFromFile(ddsFilePath);
	return ddsFile.convertToTexture2D(renderer);
}
