#pragma once

#include <SFML/Graphics.hpp>
#include <cstdint>

//===========================================

namespace OCIF
{

using Color = uint32_t;

//===========================================

Color     To24BitColor(uint8_t index);
Color     To24BitColor(sf::Color color);
Color     To24BitColor(float* arr);
sf::Color ToSFColor(Color color, double alpha = 0.0);
sf::Color ToSFColor(float* arr);
float*    ToFloat3Color(sf::Color color, float* arr);
uint8_t   To8BitColor(sf::Color color);
uint8_t   To8BitColor(Color color);
double    ColorDiffAvg(sf::Color a, sf::Color b);

Color NormalizeColor(Color color);

//===========================================

} // namespace OCIF

//===========================================