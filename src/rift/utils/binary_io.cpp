#include <utils/binary_io.hpp>
#include <log.hpp>
#include <cassert>

namespace util
{
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

}