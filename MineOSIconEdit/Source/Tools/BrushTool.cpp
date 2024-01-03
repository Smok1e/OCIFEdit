#include <Tools/BrushTool.hpp>
#include <MaterialDesignIcons.hpp>
#include <Main.hpp>

#include <OCIF/Color.hpp>

//===========================================

const char* BrushTool::getIcon() const
{
	return MD_ICON_DRAW;
}

const char* BrushTool::getName() const
{
	return "Brush";
}

sf::Keyboard::Key BrushTool::getHotkey() const
{
	return sf::Keyboard::B;
}

bool BrushTool::onDraw(sf::Mouse::Button button)
{
	if (button == sf::Mouse::Left)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt))
			m_current_color = CurrentRasterizedImage.getPixel(CurrentImageCoords.x, CurrentImageCoords.y);

		else
		{
			auto& pixel = CurrentImage.get(CurrentPixelCoords.x, CurrentPixelCoords.y);
			pixel.character = ' ';
			pixel.background = OCIF::To24BitColor(OCIF::To8BitColor(m_current_color));
			pixel.foreground = 0x000000;
			pixel.alpha = 0.0;

			CurrentImage.rasterizePixel(
				CurrentRasterizedImage, 
				OpencomputersFont, 
				CurrentPixelCoords.x, 
				CurrentPixelCoords.y
			);

			return true;
		}
	}

	return false;
}

void BrushTool::onRenderWorkspace()
{
	static sf::RectangleShape rect;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) && sf::Mouse::isButtonPressed(sf::Mouse::Left))
	{
		rect.setPosition(sf::Vector2f(CurrentMouseCoords) + sf::Vector2f(20, 20));
		rect.setFillColor(m_current_color);
		rect.setSize(sf::Vector2f(50, 50));
		rect.setOutlineColor(sf::Color::White);
		rect.setOutlineThickness(1.f);
		RenderWindow.draw(rect);
	}

	else if (m_show_crosshair && IsMouseInsideImage())
	{	
		auto sprite_scale  = CurrentImageSprite.getScale();
		auto sprite_bounds = CurrentImageSprite.getGlobalBounds();

		rect.setFillColor(sf::Color(255, 255, 255, 100));
		rect.setPosition(sf::Vector2f(PixelToWindowCoords(CurrentPixelCoords) + sf::Vector2i(1, 1)));
		rect.setSize(sf::Vector2f(sprite_scale.x * OCIF::HexFont::Glyph::DefaultWidth, sprite_scale.y * OCIF::HexFont::Glyph::DefaultHeight));
		RenderWindow.draw(rect);

		auto src_coords = PixelToWindowCoords(CurrentPixelCoords);
		auto dst_coords = PixelToWindowCoords(CurrentPixelCoords + sf::Vector2i(1, 1));

		rect.setOutlineThickness(0);

		rect.setPosition(sprite_bounds.left, src_coords.y);
		rect.setSize(sf::Vector2f(sprite_bounds.width, 1));
		RenderWindow.draw(rect);

		rect.setPosition(sprite_bounds.left, dst_coords.y);
		RenderWindow.draw(rect);

		rect.setPosition(src_coords.x, sprite_bounds.top);
		rect.setSize(sf::Vector2f(1, sprite_bounds.height));
		RenderWindow.draw(rect);

		rect.setPosition(dst_coords.x, sprite_bounds.top);
		RenderWindow.draw(rect);
	}
}

void BrushTool::processGUI()
{
	if (ImGui::Begin("Brush settings"))
	{
		ImGui::Checkbox("Show crosshair", &m_show_crosshair);
		ImGui::Separator();

		static float color[3] = {};
		if (ImGui::ColorPicker3("Color", OCIF::ToFloat3Color(m_current_color, color)))
			m_current_color = OCIF::ToSFColor(color);

		ImGui::End();
	}
}

//===========================================