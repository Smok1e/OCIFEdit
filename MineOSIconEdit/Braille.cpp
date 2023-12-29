#include "Braille.hpp"

//===========================================

constexpr uint32_t BRAILLE_BEGIN    = 0x2800;
constexpr uint32_t BRAILLE_END      = 0x28FF;
constexpr uint32_t LOWER_HALF_BLOCK = 0x2584;

//===========================================

namespace OCIF
{

//===========================================

bool IsBrailleCharacter(uint32_t ch)
{
	return BRAILLE_BEGIN <= ch && ch <= BRAILLE_END || ch == LOWER_HALF_BLOCK || ch == ' ';
}

bool BrailleCheckDot(uint32_t ch, size_t x, size_t y)
{
	if (ch == LOWER_HALF_BLOCK)
		return y >= 2;

	if (ch == ' ')
		return false;

	size_t index = ch - BRAILLE_BEGIN;

	// Pizda
	return y < 3? (index >> (3*x + y)) & 1: (index >= 0x40) * ((index >> (x + 6)) & 1);
}

void RasterizeBraille(sf::Image& image, unsigned x, unsigned y, uint32_t ch, sf::Color background, sf::Color foreground)
{
	constexpr size_t braille_dots_x = 2;
	constexpr size_t braille_dots_y = 4;

	for (size_t dot_x = 0; dot_x < braille_dots_x; dot_x++)
	{
		for (size_t dot_y = 0; dot_y < braille_dots_y; dot_y++)
		{
			image.setPixel(
				x * braille_dots_x + dot_x, 
				y * braille_dots_y + dot_y,
				BrailleCheckDot(ch, dot_x, dot_y)
					? foreground
					: background
			);
		}
	}
}

//===========================================

} // namespace OCIF

//===========================================