#include <Tools/BrailleTool.hpp>
#include <MaterialDesignIcons.hpp>
#include <Main.hpp>

#include <OCIF/Color.hpp>
#include <OCIF/Braille.hpp>

//===========================================

const char* BrailleTool::getIcon() const
{
	return MD_ICON_GRID_ON;
}

const char* BrailleTool::getName() const
{
	return "Braille";
}

sf::Keyboard::Key BrailleTool::getHotkey() const
{
	return sf::Keyboard::F;
}

bool BrailleTool::onDraw(sf::Mouse::Button button)
{
	auto& pixel = CurrentImage.get(CurrentPixelCoords.x, CurrentPixelCoords.y);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt))
	{
		if (button == sf::Mouse::Left)
		{
			m_current_background = OCIF::ToSFColor(pixel.background);
			m_current_foreground = OCIF::ToSFColor(pixel.foreground);
			m_transparent_background = (pixel.alpha > 0.0);
		}
	}

	else
	{
		pixel.character = OCIF::BrailleSetDot(
			pixel.character, 
			CurrentBrailleCoords.x % 2,
			CurrentBrailleCoords.y % 4,
			button == sf::Mouse::Left
		);

		pixel.foreground = OCIF::NormalizeColor(OCIF::To24BitColor(m_current_foreground));
		pixel.background = OCIF::NormalizeColor(OCIF::To24BitColor(m_current_background));
		pixel.alpha = m_transparent_background;

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

void BrailleTool::onRenderWorkspace()
{
	static sf::RectangleShape rect;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) && sf::Mouse::isButtonPressed(sf::Mouse::Left))
	{
		rect.setPosition(sf::Vector2f(CurrentMouseCoords) + sf::Vector2f(20, 20));
		rect.setFillColor(sf::Color::Transparent);
		rect.setSize(sf::Vector2f(50, 50));
		rect.setOutlineColor(sf::Color::White);
		rect.setOutlineThickness(1.f);
		RenderWindow.draw(rect);

		rect.setOutlineThickness(0.f);
		rect.setFillColor(m_current_foreground);
		rect.setSize(sf::Vector2f(50, 25));
		RenderWindow.draw(rect);

		if (!m_transparent_background)
		{
			rect.setFillColor(m_current_background);
			rect.setPosition(sf::Vector2f(CurrentMouseCoords) + sf::Vector2f(20, 45));
			RenderWindow.draw(rect);
		}
	}

	else if (IsMouseInsideImage())
	{	
		auto sprite_scale  = CurrentImageSprite.getScale();
		auto sprite_bounds = CurrentImageSprite.getGlobalBounds();

		if (m_show_crosshair)
		{
			rect.setFillColor(sf::Color(255, 255, 255, 100));
			rect.setPosition(sf::Vector2f(BrailleToWindowCoords(CurrentBrailleCoords) + sf::Vector2i(1, 1)));
			rect.setSize(sf::Vector2f(sprite_scale.x * 4, sprite_scale.y * 4));
			RenderWindow.draw(rect);

			auto src_coords = BrailleToWindowCoords(CurrentBrailleCoords);
			auto dst_coords = BrailleToWindowCoords(CurrentBrailleCoords + sf::Vector2i(1, 1));

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

		if (m_show_hovered_pixel)
		{
			rect.setFillColor(sf::Color::Transparent);
			rect.setOutlineColor(sf::Color::White);
			rect.setOutlineThickness(1.f);
			rect.setSize(sf::Vector2f(sprite_scale.x * OCIF::HexFont::Glyph::DefaultWidth, sprite_scale.y * OCIF::HexFont::Glyph::DefaultHeight));
			rect.setPosition(sf::Vector2f(PixelToWindowCoords(CurrentPixelCoords)));
			RenderWindow.draw(rect);
		}
	}
}

void BrailleTool::processGUI()
{
	if (ImGui::Begin("Braille settings"))
	{
		ImGui::Checkbox("Show crosshair", &m_show_crosshair);
		ImGui::Checkbox("Show hovered pixel", &m_show_hovered_pixel);
		ImGui::Separator();

		ImGui::Checkbox("Transparent background", &m_transparent_background);

		static float foreground[3] = {};
		if (ImGui::ColorPicker3("Foreground", OCIF::ToFloat3Color(m_current_foreground, foreground)))
			m_current_foreground = OCIF::ToSFColor(foreground);

		static float background[3] = {};
		if (ImGui::ColorPicker3("Background", OCIF::ToFloat3Color(m_current_background, background)))
			m_current_background = OCIF::ToSFColor(background);

		ImGui::End();
	}
}

bool BrailleTool::onKeyPressed(sf::Keyboard::Key key)
{
	switch (key)
	{
		case sf::Keyboard::X:
			std::swap(m_current_background, m_current_foreground);
			return true;
			break;
	}

	return false;
}

//===========================================