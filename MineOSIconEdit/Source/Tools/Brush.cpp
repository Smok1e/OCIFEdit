#include "IconsMaterialDesign.hpp"
#include "Tools/Brush.hpp"
#include "OCIF/Color.hpp"

#include <imgui.h>
#include <imgui-SFML.h>

//===========================================

const char* BrushTool::getIcon() const
{
	return ICON_MD_DRAW;
}

const char* BrushTool::getName() const
{
	return "Brush";
}

bool BrushTool::onDraw(
	sf::Mouse::Button button,
	OCIF::Image& image, 
	OCIF::HexFont& font,
	sf::Image& rasterized_image, 
	const sf::Vector2i& coords
)
{
	if (button == sf::Mouse::Left)
	{
		auto& pixel = image.get(coords.x, coords.y);
		pixel.character = ' ';
		pixel.background = OCIF::To24BitColor(OCIF::To8BitColor(m_current_color));
		pixel.foreground = 0x000000;
		pixel.alpha = 0.0;

		image.rasterizePixel(rasterized_image, font, coords.x, coords.y);
		return true;
	}

	return false;
}

void BrushTool::processGUI()
{
	static float color[3] = {};
	if (ImGui::ColorPicker3("Color", OCIF::ToFloat3Color(m_current_color, color)))
		m_current_color = OCIF::ToSFColor(color);
}

//===========================================