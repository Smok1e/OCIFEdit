#pragma once
#include <cstdint>
#include <SFML/Graphics.hpp>

//===========================================

namespace OCIF
{

//===========================================

bool IsBrailleCharacter(uint32_t ch);
bool BrailleCheckDot(uint32_t ch, size_t x, size_t y);
void RasterizeBraille(sf::Image& image, unsigned x, unsigned y, uint32_t ch, sf::Color background, sf::Color foreground);

//===========================================

} // namespace OCIF

//===========================================