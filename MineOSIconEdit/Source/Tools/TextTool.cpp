#include <cwctype>

#include <Tools/TextTool.hpp>
#include <MaterialDesignIcons.hpp>
#include <Main.hpp>

#include <OCIF/Color.hpp>
#include <OCIF/IO.hpp>

//===========================================

const char* TextTool::getIcon() const
{
	return MD_ICON_TEXT_FIELDS;
}

const char* TextTool::getName() const
{
	return "Text";
}

sf::Keyboard::Key TextTool::getHotkey() const
{
	return sf::Keyboard::T;
}

void TextTool::onRenderWorkspace()
{
	auto sprite_scale = CurrentImageSprite.getScale();

	static sf::RectangleShape rect;
	rect.setSize(sf::Vector2f(sprite_scale.x * OCIF::HexFont::Glyph::DefaultWidth, sprite_scale.y * OCIF::HexFont::Glyph::DefaultHeight));
	rect.setOutlineColor(sf::Color::White);
	rect.setOutlineThickness(1.f);

	if (m_typing)
	{
		rect.setPosition(sf::Vector2f(PixelToWindowCoords(m_typing_position + sf::Vector2i(m_text.length(), 0))));
		rect.setFillColor(sf::Color::Transparent);
		RenderWindow.draw(rect);
	}

	else if (IsMouseInsideImage())
	{
		rect.setPosition(sf::Vector2f(PixelToWindowCoords(CurrentPixelCoords)));
		rect.setFillColor(sf::Color(255, 255, 255, 100));
		RenderWindow.draw(rect);
	}
}

void TextTool::processGUI()
{
	if (ImGui::Begin("Text settings"))
	{
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

void TextTool::appendCharacter(uint32_t codepoint)
{
	if (m_typing_position.x + m_text.length() < CurrentImage.getWidth())
	{
		unsigned x = m_typing_position.x + m_text.length();
		unsigned y = m_typing_position.y;

		CurrentImage.set(
			x,
			y,
			{
				codepoint,
				OCIF::NormalizeColor(OCIF::To24BitColor(m_current_background)),
				OCIF::NormalizeColor(OCIF::To24BitColor(m_current_foreground)),
				static_cast<double>(m_transparent_background)
			}
		);

		CurrentImage.rasterizePixel(CurrentRasterizedImage, OpencomputersFont, x, y);
		UpdateTexture();

		m_text.push_back(codepoint);
	}
}

void TextTool::eraseCharacter()
{
	if (!m_text.empty())
	{
		unsigned x = m_typing_position.x + m_text.length() - 1;
		unsigned y = m_typing_position.y;

		CurrentImage.set(
			x,
			y,
			{
				' ',
				OCIF::NormalizeColor(OCIF::To24BitColor(m_current_background)),
				OCIF::NormalizeColor(OCIF::To24BitColor(m_current_foreground)),
				static_cast<double>(m_transparent_background)
			}
		);

		CurrentImage.rasterizePixel(CurrentRasterizedImage, OpencomputersFont, x, y);
		UpdateTexture();

		m_text.erase(m_text.end() - 1);
	}
}

//===========================================

bool TextTool::onEvent(const sf::Event& event)
{
	switch (event.type)
	{
		case sf::Event::KeyPressed:
			switch (event.key.code)
			{
				case sf::Keyboard::Escape:
				case sf::Keyboard::Enter:
					if (m_typing)
					{
						onTypingDone();
						return true;
					}

					break;

				case sf::Keyboard::X:
					std::swap(m_current_background, m_current_foreground);
					break;
			}

			// When key is pressed, SFML will generate both KeyPressed and TextEntered events
			// We have to catch KeyPressed event and stop it's propagination to prevent 
			// other keyboard processing during typing.
			return m_typing;

			break;

		case sf::Event::TextEntered:
			if (onTextEntered(event.text.unicode))
				return true;

			break;
	}

	return Tool::onEvent(event);
}

bool TextTool::onTextEntered(uint32_t codepoint)
{
	if (!m_typing)
		return false;

	switch (codepoint)
	{
		// Backspace
		case '\b':
			eraseCharacter();
			break;

		default:
			if (std::iswprint(codepoint))
				appendCharacter(codepoint);

			break;
	}

	return true;
}

bool TextTool::onMouseButtonPressed(sf::Mouse::Button button)
{
	if (m_typing)
	{
		onTypingDone();
		return true;
	}

	return Tool::onMouseButtonPressed(button);
}

bool TextTool::onMouseButtonPressedInsideImage(sf::Mouse::Button button)
{
	switch (button)
	{
		case sf::Mouse::Left:
			onTypingStarted();

			return true;
			break;
	}
	
	return Tool::onMouseButtonPressedInsideImage(button);
}

//===========================================

void TextTool::onTypingStarted()
{
	m_typing_position = CurrentPixelCoords;
	m_text = L"";
	m_typing = true;
}

void TextTool::onTypingDone()
{
	m_typing = false;
}

//===========================================