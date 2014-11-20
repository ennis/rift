#ifndef SERIALIZATION_HPP
#define SERIALIZATION_HPP

#include <istream>
#include <cstdint>
#include <memory>
#include <sstream>
#include <vector>
#include <cstring>

namespace rift {
namespace serialization {

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


template <typename T> struct pack_traits;

class Packer
{
public:
	Packer(std::ostream &streamOut) : mStreamOut(streamOut) {}
	Packer &pack(int v) { write_i32le(mStreamOut, v); return *this; }
	Packer &pack(unsigned int v) { write_u32le(mStreamOut, v); return *this; }
	Packer &pack(float v) { mStreamOut.write((char*)&v, sizeof(float)); return *this; }
	Packer &pack(double v) { mStreamOut.write((char*)&v, sizeof(double)); return *this; }
	template <std::size_t size>
	Packer &pack(const char (&v)[size]) {
		pack(v, size);
		return *this;
	}
	Packer &pack(const char *str) {
		pack(str, std::strlen(str));
		return *this;
	}
	Packer &pack(const char *str, unsigned int size) { write_u32le(mStreamOut, size); mStreamOut.write(str, size); return *this; }
	Packer &pack(std::string const &str) { pack(str.c_str(), str.size()); return *this; }
	Packer &packArray(unsigned int size) { write_u32le(mStreamOut, size); return *this; }
	template <typename T>
	Packer &pack(std::vector<T> const &vec) {
		packArray(vec.size());
		for (auto &&v : vec) {
			pack(v);
		}
		return *this;
	}
	template <typename T, typename Fn>
	Packer &pack(std::vector<T> const &v, Fn f) {
		write_u32le(mStreamOut, v.size());
		for (const auto &e : v) {
			f(*this, e);
		}
		return *this;
	}
	template <typename T>
	Packer &pack(T const &v) {
		pack_traits<T>::pack(*this, v);
		return *this;
	}

	Packer &pack_bin(void const *ptr, unsigned int size) {
		write_u32le(mStreamOut, size);
		mStreamOut.write((const char*)ptr, size);
		return *this;
	}

private:
	std::ostream &mStreamOut;
};

class Unpacker
{
public:
	Unpacker(std::istream &streamIn) : mStreamIn(streamIn) {}

	Unpacker &unpack_array(unsigned int &v) { read_u32le(mStreamIn, v); return *this; }
	Unpacker &unpack(int &v) { read_i32le(mStreamIn, v); return *this; }
	Unpacker &unpack(unsigned int &v) { read_u32le(mStreamIn, v); return *this; }
	Unpacker &unpack(float &v)  { mStreamIn.read((char*)&v, sizeof(float)); return *this; }
	Unpacker &unpack(double &v) { mStreamIn.read((char*)&v, sizeof(double)); return *this; }

	template <typename T>
	Unpacker &skip() { T t; return unpack(t); }

	template <std::size_t size>
	Unpacker &unpack(char (&v)[size]) {
		unsigned int rsize;
		unpack(v, size, rsize);
		assert(rsize == size);
		return *this;
	}
	Unpacker &unpack(char *v, unsigned int maxSize, unsigned int &size) {
		read_u32le(mStreamIn, size);
		assert(size < maxSize);
		mStreamIn.read(v, size);
		return *this;
	}
	Unpacker &unpack(std::string &str) {
		unsigned int size;
		read_u32le(mStreamIn, size);
		auto ch = std::unique_ptr<char[]>(new char[size]);
		mStreamIn.read(ch.get(), size);
		str.assign(ch.get(), size);
		return *this;
	}
	template <typename T>
	Unpacker &unpack(std::vector<T> &v) {
		unsigned int size;
		unpack(size);
		v.resize(size);
		for (unsigned int i = 0; i < size; ++i) {
			unpack(v[i]);
		}
		return *this;
	}
	template <typename T, typename Fn>
	Unpacker &unpack(std::vector<T> &v, Fn f) {
		unsigned int size;
		unpack(size);
		v.reserve(size);
		for (unsigned int i = 0; i < size; ++i) {
			v.emplace_back(f(*this));
		}
		return *this;
	}
	template <typename T>
	Unpacker &unpack(T &v) {
		pack_traits<T>::unpack(*this, v);
		return *this;
	}
	template <typename Fn>
	Unpacker &skip_array(Fn f) {
		unsigned int size;
		unpack(size);
		for (unsigned int i = 0; i < size; ++i) {
			f(*this);
		}
		return *this;
	}
	Unpacker &unpack_bin(std::unique_ptr<uint8_t[]> &data, unsigned int &size) {
		unpack(size);
		auto p = new uint8_t[size];
		mStreamIn.read((char*)p, size);
		data = std::unique_ptr<uint8_t[]>(p);
		return *this;
	}

private:
	std::istream &mStreamIn;
};

}}


#endif /* end of include guard: SERIALIZATION_HPP */
