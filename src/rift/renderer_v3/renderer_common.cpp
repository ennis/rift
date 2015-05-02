#include <renderer_common.hpp>

//=============================================================================
namespace
{
	struct ElementFormatInfo
	{
		unsigned int byteSize;
		const char *name;
	};

	ElementFormatInfo element_format_info[static_cast<int>(ElementFormat::Max)] =
	{
		/*Uint32x4*/      {16, "Uint32x4"},
		/*Sint32x4*/      {16, "Sint32x4"},
		/*Float4*/        {16, "Float4"},
		/*Uint32x3*/      {12, "Uint32x3"},
		/*Sint32x3*/      {12, "Sint32x3"},	
		/*Float3*/	      {12, "Float3"},
		/*Float2*/        {8, "Float2"},
		/*Uint16x4*/      {8, "Uint16x4"},
		/*Sint16x4*/      {8, "Sint16x4"}, 
		/*Unorm16x4*/     {8, "Unorm16x4"},
		/*Snorm16x4*/     {8, "Snorm16x4"},
		/*Float16x4*/     {8, "Float16x4"},
		/*Uint16x2*/      {4, "Uint16x2"},
		/*Sint16x2*/      {4, "Sint16x2"},
		/*Unorm16x2*/     {4, "Unorm16x2"},
		/*Snorm16x2*/     {4, "Snorm16x2"},
		/*Float16x2*/     {4, "Float16x2"},
		/*Uint8x4*/	      {4, "Uint8x4"},
		/*Sint8x4*/       {4, "Sint8x4"},
		/*Unorm8x4*/      {4, "Unorm8x4"},
		/*Snorm8x4*/      {4, "Snorm8x4"},
		/*Uint8x3*/       {3, "Uint8x3"},
		/*Sint8x3*/       {3, "Sint8x3"},
		/*Unorm8x3*/      {3, "Unorm8x3"},
		/*Snorm8x3*/      {3, "Snorm8x3"},
		/*Uint8x2*/       {2, "Uint8x2"},
		/*Sint8x2*/       {2, "Sint8x2"},
		/*Unorm8x2*/      {2, "Unorm8x2"},
		/*Snorm8x2*/      {2, "Snorm8x2"},
		/*Unorm10x3_1x2*/ {4, "Unorm10x3_1x2"},
		/*Snorm10x3_1x2*/ {4, "Snorm10x3_1x2"},
		/*BC1*/           {0, "BC1"},
		/*BC2*/           {0, "BC2"},
		/*BC3*/           {0, "BC3"},
		/*UnormBC4*/      {0, "UnormBC4"},
		/*SnormBC4*/      {0, "SnormBC4"},
		/*UnormBC5*/      {0, "UnormBC5"},
		/*SnormBC5*/      {0, "SnormBC5"},
		/*Uint32*/        {4, "Uint32"},
		/*Sint32*/        {4, "Sint32"},
		/*Uint16*/        {2, "Uint16"},
		/*Sint16*/        {2, "Sint16"},
		/*Unorm16*/       {2, "Unorm16"},
		/*Snorm16*/       {2, "Snorm16"},
		/*Uint8*/         {1, "Uint8"},
		/*Sint8*/         {1, "Sint8"},
		/*Unorm8*/        {1, "Unorm8"},
		/*Snorm8*/        {1, "Snorm8"},
		/*Depth32*/       {4, "Depth32"},
		/*Depth24*/       {3, "Depth24"},
		/*Depth16*/       {2, "Depth16"},
		/*Float16*/       {2, "Float16"},
		/*Float*/         {4, "Float"}
	};
}

const char *getElementFormatName(ElementFormat format)
{
	return element_format_info[static_cast<int>(format)].name;
}

unsigned int getElementFormatSize(ElementFormat format)
{
	return element_format_info[static_cast<int>(format)].byteSize;
}
