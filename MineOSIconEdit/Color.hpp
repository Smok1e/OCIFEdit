#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>

//===========================================

namespace OCIF
{

using Color = uint32_t;

//===========================================

Color To24BitColor(uint8_t index);
sf::Color ToSFColor(Color color, double alpha = 0.0);

//===========================================

} // namespace OCIF

//===========================================