#include <gl_common.hpp>
	
namespace
{
	ElementFormatInfoGL element_format_info_gl[static_cast<int>(ElementFormat::Max)] =
	{
		/*Uint32x4*/     {GL_UNSIGNED_INT,                4, GL_RGBA32UI,                      GL_RGBA_INTEGER,    false, false},
		/*Sint32x4*/     {GL_INT,                         4, GL_RGBA32I,                       GL_RGBA_INTEGER,    false, false},
		/*Float4*/       {GL_FLOAT,                       4, GL_RGBA32F,                       GL_RGBA,            false, false},
		/*Uint32x3*/     {GL_UNSIGNED_INT,                3, GL_RGB32UI,                       GL_RGB_INTEGER,     false, false}, 
		/*Sint32x3*/     {GL_INT,                         3, GL_RGB32I,                        GL_RGB_INTEGER,     false, false},
		/*Float3*/	     {GL_FLOAT,                       3, GL_RGB32F,                        GL_RGB,             false, false},
		/*Float2*/       {GL_FLOAT,                       2, GL_RG16F,                         GL_RG,              false, false},
		/*Uint16x4*/     {GL_UNSIGNED_SHORT,              4, GL_RGBA16UI,                      GL_RGBA_INTEGER,    false, false}, 
		/*Sint16x4*/     {GL_SHORT,                       4, GL_RGBA16I,                       GL_RGBA_INTEGER,    false, false},
		/*Unorm16x4*/    {GL_UNSIGNED_SHORT,              4, GL_RGBA16,                        GL_RGBA,            true, false},
		/*Snorm16x4*/    {GL_SHORT,                       4, GL_RGBA16_SNORM,                  GL_RGBA,            true, false},
		/*Float16x4*/    {GL_HALF_FLOAT,                  4, GL_RGBA16F,                       GL_RGBA,            false, false},
		/*Uint16x2*/     {GL_UNSIGNED_SHORT,              2, GL_RG16UI,                        GL_RG_INTEGER,      false, false},
		/*Sint16x2*/     {GL_SHORT,                       2, GL_RG16I,                         GL_RG_INTEGER,      false, false},
		/*Unorm16x2*/    {GL_UNSIGNED_SHORT,              2, GL_RG16,                          GL_RG,              true, false},
		/*Snorm16x2*/    {GL_SHORT,                       2, GL_RG16_SNORM,                    GL_RG,              true, false},
		/*Float16x2*/    {GL_HALF_FLOAT,                  2, GL_RG16F,                         GL_RG,              false, false},
		/*Uint8x4*/	     {GL_UNSIGNED_BYTE,               4, GL_RGBA8UI,                       GL_RGBA_INTEGER,    false, false},
		/*Sint8x4*/      {GL_BYTE,                        4, GL_RGBA8I,                        GL_RGBA_INTEGER,    false, false},
		/*Unorm8x4*/     {GL_UNSIGNED_BYTE,               4, GL_RGBA8,                         GL_RGBA,            true, false},
		/*Snorm8x4*/     {GL_BYTE,                        4, GL_RGBA8_SNORM,                   GL_RGBA,            true, false},
		/*Uint8x3*/      {GL_UNSIGNED_BYTE,               3, GL_RGB8UI,                        GL_RGB_INTEGER,     false, false},
		/*Sint8x3*/      {GL_BYTE,                        3, GL_RGB8I,                         GL_RGB_INTEGER,     false, false},
		/*Unorm8x3*/     {GL_UNSIGNED_BYTE,               3, GL_RGB8,                          GL_RGB,             true, false},
		/*Snorm8x3*/     {GL_BYTE,                        3, GL_RGB8_SNORM,                    GL_RGB,             true, false},
		/*Uint8x2*/      {GL_UNSIGNED_BYTE,               2, GL_RG8UI,                         GL_RG_INTEGER,      false, false},
		/*Sint8x2*/      {GL_BYTE,                        2, GL_RG8I,                          GL_RG_INTEGER,      false, false},
		/*Unorm8x2*/     {GL_UNSIGNED_BYTE,               2, GL_RG8,                           GL_RG,              true, false},
		/*Snorm8x2*/     {GL_BYTE,                        2, GL_RG8_SNORM,                     GL_RG,              true, false},
		/*Unorm10x3_1x2*/{GL_UNSIGNED_INT_2_10_10_10_REV, 4, GL_RGB10_A2,                      GL_RGBA,            true, false },
		/*Snorm10x3_1x2*/{GL_INT_2_10_10_10_REV,          4, GL_RGB10_A2,                      GL_RGBA,            true, true },
		/*BC1*/          {0,                              0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_RGBA,            false, true},
		/*BC2*/          {0,                              0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_RGBA,            false, true},
		/*BC3*/          {0,                              0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_RGBA,            false, true},
		/*UnormBC4*/     {0,                              0, 0,                                0,                  false, true},
		/*SnormBC4*/     {0,                              0, 0,                                0,                  false, true},
		/*UnormBC5*/     {0,                              0, 0,                                0,                  false, true},
		/*SnormBC5*/     {0,                              0, 0,                                0,                  false, true},
		/*Uint32*/       {GL_UNSIGNED_INT,                1, GL_R32UI,                         GL_RED,             false, false},
		/*Sint32*/       {GL_INT,                         1, GL_R32I,                          GL_RED,             false, false},
		/*Uint16*/       {GL_UNSIGNED_SHORT,              1, GL_R16UI,                         GL_RED,             false, false},
		/*Sint16*/       {GL_SHORT,                       1, GL_R16I,                          GL_RED,             false, false},
		/*Unorm16*/      {GL_UNSIGNED_SHORT,              1, GL_R16,                           GL_RED,             true, false},
		/*Snorm16*/      {GL_SHORT,                       1, GL_R16_SNORM,                     GL_RED,             true, false},
		/*Uint8*/        {GL_UNSIGNED_BYTE,               1, GL_R8UI,                          GL_RED,             false, false},
		/*Sint8*/        {GL_BYTE,                        1, GL_R8I,                           GL_RED,             false, false},
		/*Unorm8*/       {GL_UNSIGNED_BYTE,               1, GL_R8,                            GL_RED,             true, false},
		/*Snorm8*/       {GL_BYTE,                        1, GL_R8_SNORM,                      GL_RED,             true, false},
		/*Depth32*/      {GL_UNSIGNED_INT,                0, GL_DEPTH_COMPONENT32,             GL_DEPTH_COMPONENT, false, false},
		/*Depth24*/      {GL_UNSIGNED_INT,                0, GL_DEPTH_COMPONENT24,             GL_DEPTH_COMPONENT, false, false},
		/*Depth16*/      {GL_UNSIGNED_SHORT,              0, GL_DEPTH_COMPONENT16,             GL_DEPTH_COMPONENT, false, false},
		/*Float16*/      {GL_HALF_FLOAT,                  1, GL_R16F,                          GL_RED,             false, false},
		/*Float*/        {GL_FLOAT,                       1, GL_R32F,                          GL_RED,             false, false}
	};
}

const ElementFormatInfoGL &getElementFormatInfoGL(ElementFormat format)
{
	return element_format_info_gl[static_cast<int>(format)];
}
