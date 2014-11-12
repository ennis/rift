#ifndef SERIALIZATION_HPP
#define SERIALIZATION_HPP

#include <istream>
#include <cstdint>
#include <memory>

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

enum class TagType : unsigned int 
{
	Int = 0,
	Uint,
	IntArray, 
	UintArray,
	String,
	Float,
	Blob,
	Array,
	Compound,
	End,
	Max
};

namespace BinaryTag 
{
	static const unsigned int kMaxTagNameSize = 64;
	class Reader
	{
	public:
		Reader(std::istream &streamIn);
		bool nextTag();
		void skip();
		// zero-terminated tag name
		const char *tagName() {
			return mCurTagName;
		}
		TagType tagType() {
			return static_cast<TagType>(mCurTagType);
		}
		unsigned int payloadSize() {
			return mCurPayloadSize;
		}
		void readUint(unsigned int &v);
		void readInt(int &v);
		void readFloat(float &v);
		void readBlob(void *out);
		void readString(std::string &v);
		void readArraySize(int &arraySize);
		std::unique_ptr<int[]> readIntArray();
		std::unique_ptr<unsigned int[]> readUintArray();
		bool match(const char *name, TagType tagType);
	private:
		std::istream &mStreamIn;
		unsigned int mCurTagNameSize;
		char mCurTagName[kMaxTagNameSize+1];
		unsigned int mCurTagType;
		unsigned int mCurPayloadSize;
	};

	class Writer
	{
	public:
		Writer(std::ostream &streamOut);
		void writeTag(
			const char *name, 
			TagType type, 
			unsigned int payloadSize, 
			void *payload);
		void writeTagHeader(
			const char *name, 
			TagType type, 
			unsigned int payloadSize);
		void writeInt(const char *name, int v);
		void writeUint(const char *name, unsigned int v);
		void writeFloat(const char *name, float v);
		void writeString(const char *name, const char *string);
		template <typename Iter>
		void writeUintArray(const char *name,Iter begin,Iter end) {
			// XXX integer overflow?
			auto numElements=end-begin;
			writeTagHeader(name,TagType::UintArray,numElements*4);
			while (begin!=end) {
				write_u32le(mStreamOut,*begin++);
			}
		}
		template <typename Iter>
		void writeIntArray(const char *name,Iter begin,Iter end) {
			auto numElements=end-begin;
			writeTagHeader(name,TagType::IntArray,numElements*4);
			while (begin!=end) {
				write_i32le(mStreamOut,*begin++);
			}
		}
		void writeUintArray(const char *name,std::initializer_list<unsigned int> values) {
			writeUintArray(name,values.begin(),values.end());
		}
		void writeIntArray(const char *name,std::initializer_list<int> values) {
			writeIntArray(name,values.begin(),values.end());
		}

		void beginCompound(const char *name);
		void endCompound();
	private:
		std::ostream &mStreamOut;
	};
}


#endif /* end of include guard: SERIALIZATION_HPP */
