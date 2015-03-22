#include <gl_common.hpp>
	
namespace
{
	ElementFormatInfoGL element_format_info_gl[static_cast<int>(ElementFormat::Max)] =
	{
		/*Uint32x4*/     {gl::UNSIGNED_INT,                4, gl::RGBA32UI,                      gl::RGBA_INTEGER,    false, false},
		/*Sint32x4*/     {gl::INT,                         4, gl::RGBA32I,                       gl::RGBA_INTEGER,    false, false},
		/*Float4*/       {gl::FLOAT,                       4, gl::RGBA32F,                       gl::RGBA,            false, false},
		/*Uint32x3*/     {gl::UNSIGNED_INT,                3, gl::RGB32UI,                       gl::RGB_INTEGER,     false, false}, 
		/*Sint32x3*/     {gl::INT,                         3, gl::RGB32I,                        gl::RGB_INTEGER,     false, false},
		/*Float3*/	     {gl::FLOAT,                       3, gl::RGB32F,                        gl::RGB,             false, false},
		/*Float2*/       {gl::FLOAT,                       2, gl::RG16F,                         gl::RG,              false, false},
		/*Uint16x4*/     {gl::UNSIGNED_SHORT,              4, gl::RGBA16UI,                      gl::RGBA_INTEGER,    false, false}, 
		/*Sint16x4*/     {gl::SHORT,                       4, gl::RGBA16I,                       gl::RGBA_INTEGER,    false, false},
		/*Unorm16x4*/    {gl::UNSIGNED_SHORT,              4, gl::RGBA16,                        gl::RGBA,            true, false},
		/*Snorm16x4*/    {gl::SHORT,                       4, gl::RGBA16_SNORM,                  gl::RGBA,            true, false},
		/*Float16x4*/    {gl::HALF_FLOAT,                  4, gl::RGBA16F,                       gl::RGBA,            false, false},
		/*Uint16x2*/     {gl::UNSIGNED_SHORT,              2, gl::RG16UI,                        gl::RG_INTEGER,      false, false},
		/*Sint16x2*/     {gl::SHORT,                       2, gl::RG16I,                         gl::RG_INTEGER,      false, false},
		/*Unorm16x2*/    {gl::UNSIGNED_SHORT,              2, gl::RG16,                          gl::RG,              true, false},
		/*Snorm16x2*/    {gl::SHORT,                       2, gl::RG16_SNORM,                    gl::RG,              true, false},
		/*Float16x2*/    {gl::HALF_FLOAT,                  2, gl::RG16F,                         gl::RG,              false, false},
		/*Uint8x4*/	     {gl::UNSIGNED_BYTE,               4, gl::RGBA8UI,                       gl::RGBA_INTEGER,    false, false},
		/*Sint8x4*/      {gl::BYTE,                        4, gl::RGBA8I,                        gl::RGBA_INTEGER,    false, false},
		/*Unorm8x4*/     {gl::UNSIGNED_BYTE,               4, gl::RGBA8,                         gl::RGBA,            true, false},
		/*Snorm8x4*/     {gl::BYTE,                        4, gl::RGBA8_SNORM,                   gl::RGBA,            true, false},
		/*Uint8x3*/      {gl::UNSIGNED_BYTE,               3, gl::RGB8UI,                        gl::RGB_INTEGER,     false, false},
		/*Sint8x3*/      {gl::BYTE,                        3, gl::RGB8I,                         gl::RGB_INTEGER,     false, false},
		/*Unorm8x3*/     {gl::UNSIGNED_BYTE,               3, gl::RGB8,                          gl::RGB,             true, false},
		/*Snorm8x3*/     {gl::BYTE,                        3, gl::RGB8_SNORM,                    gl::RGB,             true, false},
		/*Uint8x2*/      {gl::UNSIGNED_BYTE,               2, gl::RG8UI,                         gl::RG_INTEGER,      false, false},
		/*Sint8x2*/      {gl::BYTE,                        2, gl::RG8I,                          gl::RG_INTEGER,      false, false},
		/*Unorm8x2*/     {gl::UNSIGNED_BYTE,               2, gl::RG8,                           gl::RG,              true, false},
		/*Snorm8x2*/     {gl::BYTE,                        2, gl::RG8_SNORM,                     gl::RG,              true, false},
		/*Unorm10x3_1x2*/{gl::UNSIGNED_INT_2_10_10_10_REV, 4, gl::RGB10_A2,                      gl::RGBA,            true, false },
		/*Snorm10x3_1x2*/{gl::INT_2_10_10_10_REV,          4, gl::RGB10_A2,                      gl::RGBA,            true, true },
		/*BC1*/          {0,                               0, gl::COMPRESSED_RGBA_S3TC_DXT1_EXT, gl::RGBA,            false, true},
		/*BC2*/          {0,                               0, gl::COMPRESSED_RGBA_S3TC_DXT3_EXT, gl::RGBA,            false, true},
		/*BC3*/          {0,                               0, gl::COMPRESSED_RGBA_S3TC_DXT5_EXT, gl::RGBA,            false, true},
		/*UnormBC4*/     {0,                               0, 0,                                 0,                  false, true},
		/*SnormBC4*/     {0,                               0, 0,                                 0,                  false, true},
		/*UnormBC5*/     {0,                               0, 0,                                 0,                  false, true},
		/*SnormBC5*/     {0,                               0, 0,                                 0,                  false, true},
		/*Uint32*/       {gl::UNSIGNED_INT,                1, gl::R32UI,                         gl::RED,             false, false},
		/*Sint32*/       {gl::INT,                         1, gl::R32I,                          gl::RED,             false, false},
		/*Uint16*/       {gl::UNSIGNED_SHORT,              1, gl::R16UI,                         gl::RED,             false, false},
		/*Sint16*/       {gl::SHORT,                       1, gl::R16I,                          gl::RED,             false, false},
		/*Unorm16*/      {gl::UNSIGNED_SHORT,              1, gl::R16,                           gl::RED,             true, false},
		/*Snorm16*/      {gl::SHORT,                       1, gl::R16_SNORM,                     gl::RED,             true, false},
		/*Uint8*/        {gl::UNSIGNED_BYTE,               1, gl::R8UI,                          gl::RED,             false, false},
		/*Sint8*/        {gl::BYTE,                        1, gl::R8I,                           gl::RED,             false, false},
		/*Unorm8*/       {gl::UNSIGNED_BYTE,               1, gl::R8,                            gl::RED,             true, false},
		/*Snorm8*/       {gl::BYTE,                        1, gl::R8_SNORM,                      gl::RED,             true, false},
		/*Depth32*/      {gl::UNSIGNED_INT,                0, gl::DEPTH_COMPONENT32,             gl::DEPTH_COMPONENT, false, false},
		/*Depth24*/      {gl::UNSIGNED_INT,                0, gl::DEPTH_COMPONENT24,             gl::DEPTH_COMPONENT, false, false},
		/*Depth16*/      {gl::UNSIGNED_SHORT,              0, gl::DEPTH_COMPONENT16,             gl::DEPTH_COMPONENT, false, false},
		/*Float16*/      {gl::HALF_FLOAT,                  1, gl::R16F,                          gl::RED,             false, false},
		/*Float*/        {gl::FLOAT,                       1, gl::R32F,                          gl::RED,             false, false}
	};
}

const ElementFormatInfoGL &getElementFormatInfoGL(ElementFormat format)
{
	return element_format_info_gl[static_cast<int>(format)];
}