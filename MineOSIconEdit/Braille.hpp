#pragma once
#include <cstdint>
#include <SFML/Graphics.hpp>

//===========================================

namespace OCIF
{

//===========================================

// Converts some characters such as space and lower half blocks to 
// same braille alternatives
uint32_t NormalizeBrailleCharacter(uint32_t ch);
unsigned BrailleBit(size_t x, size_t y);
bool     BrailleCheckDot(uint32_t ch, size_t x, size_t y);
uint32_t BrailleSetDot(uint32_t ch, size_t x, size_t y, bool dot);
	
//===========================================

} // namespace OCIF

//===========================================