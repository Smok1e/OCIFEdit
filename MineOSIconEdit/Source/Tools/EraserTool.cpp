#include <Tools/EraserTool.hpp>
#include <MaterialDesignIcons.hpp>
#include <Main.hpp>

#include <OCIF/Color.hpp>

//===========================================

const char* EraserTool::getIcon() const
{
	return MD_ICON_INK_ERASER;
}

const char* EraserTool::getName() const
{
	return "Eraser";
}

sf::Keyboard::Key EraserTool::getHotkey() const
{
	return sf::Keyboard::E;
}

bool EraserTool::onDraw()
{
	if (m_drawing_button == sf::Mouse::Left)
	{
		auto& pixel = CurrentImage.get(CurrentPixelCoords.x, CurrentPixelCoords.y);
		pixel.character = ' ';
		pixel.alpha = 1.0;

		CurrentImage.rasterizePixel(
			CurrentRasterizedImage, 
			OpencomputersFont, 
			CurrentPixelCoords.x, 
			CurrentPixelCoords.y
		);

		return true;
	}

	return false;
}

void EraserTool::processGUI()
{

}

//===========================================