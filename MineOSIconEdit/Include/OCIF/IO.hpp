#pragma once
#include <iostream>
#include <cstdint>

//===========================================

uint32_t ReadBytes(std::istream& stream, uint8_t count = 1);
uint32_t ReadUnicodeCharacter(std::istream& stream);

//===========================================