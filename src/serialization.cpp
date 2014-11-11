#include <serialization.hpp>
#include <log.hpp>
#include <cassert>

void write_u8(std::ostream &streamOut, uint8_t v)
{
	streamOut.write((char*)&v, 1);
}

void write_u16le(std::ostream &streamOut, uint16_t v)
{
	streamOut.write((char*)&v, 2);
}

void write_u32le(std::ostream &streamOut, uint32_t v)
{
	streamOut.write((char*)&v, 4);
}

void write_i8(std::ostream &streamOut, int8_t v)
{
	streamOut.write((char*)&v, 1);
}

void write_i16le(std::ostream &streamOut, int16_t v)
{
	streamOut.write((char*)&v, 2);
}

void write_i32le(std::ostream &streamOut, int32_t v)
{
	streamOut.write((char*)&v, 4);
}

namespace BinaryTag 
{
	Reader::Reader(std::istream &streamIn) : mStreamIn(streamIn)
	{
	}

	bool Reader::nextTag()
	{
		read_u8(mStreamIn, mCurTagNameSize);
		assert(mCurTagNameSize < kMaxTagNameSize);
		mStreamIn.read(mCurTagName, mCurTagNameSize);
		mCurTagName[mCurTagNameSize] = 0;
		read_u8(mStreamIn, mCurTagType);
		read_u32le(mStreamIn, mCurPayloadSize);
		if (mStreamIn.eof()) {
			return false;
		}
		LOG << "Tag: \""
			<< mCurTagName
			<< "\", type "
			<< mCurTagName
			<< ", payload "
			<< mCurPayloadSize;
		return true;
	}

	bool Reader::match(const char *name, TagType tagType)
	{
		return !std::strncmp(name, mCurTagName, mCurTagNameSize+1) 
				&& (unsigned int)tagType == mCurTagType;
	}

	void Reader::readUint(unsigned int &v) 
	{
		read_u32le(mStreamIn, v);
	}

	void Reader::readInt(int &v)
	{
		read_i32le(mStreamIn, v);
	}

	void Reader::readFloat(float &v) 
	{
		mStreamIn.read((char*)&v, sizeof(float));
	}

	void Reader::readBlob(void *out)
	{
		mStreamIn.read((char*)out, mCurPayloadSize);
	}

	void Reader::readString(std::string &v)
	{
		// TODO decode utf8?
		auto buf = new char[mCurPayloadSize+1];
		mStreamIn.read(buf, mCurPayloadSize);
		buf[mCurPayloadSize] = 0;
		v.assign(buf, mCurPayloadSize+1);
	}

	Writer::Writer(std::ostream &streamOut) : mStreamOut(streamOut)
	{
	}

	void Writer::writeTagHeader(
		const char *name, 
		TagType type, 
		unsigned int payloadSize)
	{
		auto nlen = std::strlen(name);
		assert(nlen < kMaxTagNameSize);
		write_u8(mStreamOut, static_cast<uint8_t>(nlen));
		mStreamOut.write(name, nlen);
		write_u8(mStreamOut, (uint8_t)type);
		write_u32le(mStreamOut, payloadSize);
	}

	void Writer::writeTag(
		const char *name, 
		TagType type, 
		unsigned int payloadSize, 
		void *payload)
	{
		writeTagHeader(name, type, payloadSize);
		mStreamOut.write((char*)payload, payloadSize);
	}

	void Writer::writeInt(const char *name, int v) 
	{
		writeTagHeader(name, TagType::Int, 4);
		write_i32le(mStreamOut, v);
	}

	void Writer::writeUint(const char *name, unsigned int v)
	{
		writeTagHeader(name, TagType::Uint, 4);
		write_u32le(mStreamOut, v);
	}

	void Writer::writeFloat(const char *name, float v)
	{
		writeTagHeader(name, TagType::Float, 4);
		mStreamOut.write((char*)&v, 4);
	}

	void Writer::writeString(const char *name, const char *string)
	{
		auto mlen = std::strlen(string);
		writeTagHeader(name, TagType::String, mlen);
		mStreamOut.write(string, mlen);
	}

	void Writer::beginCompound(const char *name)
	{
		writeTagHeader(name, TagType::Compound, 0);
	}

	void Writer::endCompound()
	{
		writeTagHeader("", TagType::End, 0);
	}
}
