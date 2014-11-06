#ifndef ENDIAN_HPP
#define ENDIAN_HPP

#include <istream>

template <typename T>
void read_u8(std::istream &streamIn, T &v)
{
	uint8_t tmp;
	streamIn.read((char*)&tmp, 1);
	v = tmp;
}

// read a byte from a stream
template <typename T>
void read_i8(std::istream &streamIn, T &v)
{
	int8_t tmp;
	streamIn.read((char*)&tmp, 1);
	v = tmp;
}

// TODO host endianness
template <typename T>
void read_u16le(std::istream &streamIn, T &v)
{
	uint16_t tmp;
	streamIn.read((char*)&tmp, 2);
	v = tmp;
}

// TODO host endianness
template <typename T>
void read_i16le(std::istream &streamIn, T &v)
{
	int16_t tmp;
	streamIn.read((char*)&tmp, 2);
	v = tmp;
}

// TODO host endianness
template <typename T>
void read_u32le(std::istream &streamIn, T &v)
{
	uint32_t tmp;
	streamIn.read((char*)&tmp, 4);
	v = tmp;
}

// TODO host endianness
template <typename T>
void read_i32le(std::istream &streamIn, T &v)
{
	int32_t tmp;
	streamIn.read((char*)&tmp, 4);
	v = tmp;
}

// TODO write
 
#endif /* end of include guard: ENDIAN_HPP */