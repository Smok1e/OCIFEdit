#pragma once
#include <iostream>
#include <cstdint>

//===========================================

namespace OCIF
{

//===========================================

uint32_t ReadBytes(std::istream& stream, uint8_t count = 1);

template<typename T>
void WriteBytes(std::ostream& stream, const T& data);

uint32_t ReadUnicodeCharacter(std::istream& stream);
void WriteUnicodeCharacter(std::ostream& stream, uint32_t codepoint);

//===========================================

template<typename T>
void WriteBytes(std::ostream& stream, const T& data)
{
	for (size_t i = 0; i < sizeof(T); i++)
		stream.put(reinterpret_cast<const uint8_t*>(&data)[sizeof(T) - i - 1]);
}

//===========================================

} // namespace OCIF

//===========================================