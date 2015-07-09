#ifndef BINARY_IO_HPP
#define BINARY_IO_HPP

#include <istream>
#include <ostream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include <type_traits>

namespace util {

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

void write_u8(std::ostream &streamOut, uint8_t v);
void write_u16le(std::ostream &streamOut, uint16_t v);
void write_u32le(std::ostream &streamOut, uint32_t v);
void write_i8(std::ostream &streamOut, int8_t v);
void write_i16le(std::ostream &streamOut, int16_t v);
void write_i32le(std::ostream &streamOut, int32_t v);

template <typename T>
struct Read16
{
	Read16(T &v_) : value(v_) {}
	T &value;
};

template <typename T>
Read16<T> read16(T &v) { return Read16<T>(v); }

template <typename T>
struct Read8
{
	Read8(T &v_) : value(v_) {}
	T &value;
};

template <typename T>
Read8<T> read8(T &v) { return Read8<T>(v); }

class BinaryReader
{
public:
	BinaryReader(std::istream &streamIn) : stream_in(streamIn)
	{
		stream_in.exceptions(std::ios::badbit);
	}

	operator bool()
	{
		return (bool)stream_in;
	}

	BinaryReader &operator>>(char &v) { read_i8(stream_in, v); return *this; }
	BinaryReader &operator>>(unsigned char &v) { read_u8(stream_in, v); return *this; }
	BinaryReader &operator>>(short &v) { read_i16le(stream_in, v); return *this; }
	BinaryReader &operator>>(unsigned short &v) { read_u16le(stream_in, v); return *this; }
	BinaryReader &operator>>(int &v) { read_i32le(stream_in, v); return *this; }
	BinaryReader &operator>>(unsigned int &v) { read_u32le(stream_in, v); return *this; }
	BinaryReader &operator>>(float &v)  { stream_in.read((char*)&v, sizeof(float)); return *this; }
	BinaryReader &operator>>(double &v) { stream_in.read((char*)&v, sizeof(double)); return *this; }
	BinaryReader &operator>>(std::string &v) {
		int sz = 0;
		int bit = 0;
		while (bit != 35)
		{
			uint8_t b;
			read_u8(stream_in, b);
			if (!stream_in) return *this;
			sz |= (b & 127) << bit;
			bit += 7;
			if ((b & 128) == 0)
				break;
		}
		if (sz)
		{
			std::vector<char> chars(sz);
			stream_in.read(chars.data(), sz);
			if (!stream_in) return *this;
			v.assign(chars.data(), sz);
		}
		else
			v.assign("");
		return *this;
	}
	
	// TODO put these functions out of the class

	// Read16 signed
	template <typename T>
	typename std::enable_if<
		std::is_signed<T>::value, BinaryReader>::type &
	operator>>(
		Read16<T> v
		) 
	{
		read_i16le(stream_in, v.value); return *this;
	}

	// Read16 unsigned
	template <typename T>
	typename std::enable_if<
		std::is_unsigned<T>::value, BinaryReader>::type &
	operator>>(
		Read16<T> v
		) 
	{
		read_u16le(stream_in, v.value); return *this;
	}

	// Read8 signed
	template <typename T>
	typename std::enable_if<
		std::is_signed<T>::value, BinaryReader>::type &
	operator>>(
		Read8<T> v
		)
	{
		read_i8(stream_in, v.value); return *this;
	}

	// Read8 unsigned
	template <typename T>
	typename std::enable_if<
		std::is_unsigned<T>::value, BinaryReader>::type &
	operator>>(
		Read8<T> v
		)
	{
		read_u8(stream_in, v.value); return *this;
	}

private:
	std::istream &stream_in;
};

}


#endif /* end of include guard: BINARY_IO_HPP */
