#include <string>
#include <stdexcept>
#include <cuchar>

#include "OCIF/IO.hpp"

//===========================================

namespace OCIF
{

//===========================================

uint32_t ReadBytes(std::istream& stream, uint8_t count /*= 1*/)
{
	uint32_t value = 0;
	for (uint8_t i = 0; i < count; i++)
		value = (value << 8) | stream.get();

	return value;
}

// CYKA
uint32_t ReadUnicodeCharacter(std::istream& stream)
{
	uint8_t first_byte = stream.get();

	size_t size = 0;
	for (size_t bit = 0; bit < 8; bit++)
	{
		if (((first_byte >> (7-bit)) & 1) == 0)
		{
			if (bit == 1)
				break;

			size = bit + (bit == 0);
			break;
		}
	}

	if (!size)
		throw std::runtime_error(std::string("Invalid UTF-8 character at position ") + std::to_string(stream.tellg()));	

	uint32_t ch = first_byte & (0xFF >> (size + (size > 1)));
	for (size_t i = 0; i < size-1; i++)
		ch = (ch << 6) | (stream.get() & 0b00111111);

	return ch;
}

void WriteUnicodeCharacter(std::ostream& stream, uint32_t codepoint)
{
	std::mbstate_t state;

	char buffer[MB_LEN_MAX];
	stream.write(buffer, std::c32rtomb(buffer, codepoint, &state));
}

//===========================================

} // namespace OCIF

//===========================================