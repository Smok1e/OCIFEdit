#include <cstdio>
#include "HexFont.hpp"

//===========================================

int main()
{
	OCIF::HexFont font;
	font.loadFromFile("font.hex");

	const auto& glyph = font.getGlyph(0x25E3);
	for (size_t y = 0; y < glyph.getHeight(); y++)
	{
		for (size_t x = 0; x < glyph.getWidth(); x++)
			printf("%s", glyph.dot(x, y)? "**": "  ");

		printf("\n");
	}
}

//===========================================