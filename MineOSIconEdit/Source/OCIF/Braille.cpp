#include <iostream>

#include "OCIF/Braille.hpp"

//===========================================

constexpr uint32_t BRAILLE_BEGIN           = 0x2800;
constexpr uint32_t BRAILLE_END             = 0x28FF;
constexpr uint32_t BRAILLE_EMPTY_CHARACTER = 0x2800;

std::map<uint32_t, uint32_t> BrailleAlternatives = {
	{' ',    BRAILLE_EMPTY_CHARACTER}, // Empty character
	{0x2584, 0x28E4                 }  // Lower half block
};

//===========================================

namespace OCIF
{

//===========================================

uint32_t NormalizeBrailleCharacter(uint32_t ch)
{
	auto iter = BrailleAlternatives.find(ch);
	return iter == BrailleAlternatives.end()
		? BRAILLE_BEGIN <= ch && ch <= BRAILLE_END? ch: BRAILLE_EMPTY_CHARACTER
		: iter->second;
}

unsigned BrailleBit(size_t x, size_t y)
{
	return y < 3? 3 * x + y: 6 + x;
}

bool BrailleCheckDot(uint32_t ch, size_t x, size_t y)
{
	ch = NormalizeBrailleCharacter(ch);
	return ((ch - BRAILLE_BEGIN) >> BrailleBit(x, y)) & 1;
}

uint32_t BrailleSetDot(uint32_t ch, size_t x, size_t y, bool dot)
{
	ch = NormalizeBrailleCharacter(ch);

	unsigned bit = BrailleBit(x, y);
	return BRAILLE_BEGIN + (((ch - BRAILLE_BEGIN) & ~(1 << bit)) | (dot << bit));
}

//===========================================

} // namespace OCIF

//===========================================