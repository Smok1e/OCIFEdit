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
		rect.setPosition(sf::Vector2f(PixelToWindowCoords(m_cursor_position)));
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
		if (ImGui::ColorEdit3("Foreground color", OCIF::ToFloat3Color(m_current_foreground, foreground), ImGuiColorEditFlags_NoInputs))
			m_current_foreground = OCIF::ToSFColor(foreground);

		static float background[3] = {};
		if (ImGui::ColorEdit3("Background color", OCIF::ToFloat3Color(m_current_background, background), ImGuiColorEditFlags_NoInputs))
			m_current_background = OCIF::ToSFColor(background);

		ImGui::End();
	}
}

//===========================================

void TextTool::setCharacter(const sf::Vector2i& position, uint32_t codepoint)
{
	CurrentImage.set(
		position.x,
		position.y,
		{
			codepoint,
			OCIF::NormalizeColor(OCIF::To24BitColor(m_current_background)),
			OCIF::NormalizeColor(OCIF::To24BitColor(m_current_foreground)),
			static_cast<double>(m_transparent_background)
		}
	);

	CurrentImage.rasterizePixel(
		CurrentRasterizedImage,
		OpencomputersFont,
		position.x,
		position.y
	);

	UpdateTexture();
}

void TextTool::putCharacter(uint32_t codepoint)
{
	if (m_cursor_position.x < CurrentImage.getWidth())
		setCharacter(m_cursor_position, codepoint), m_cursor_position.x++;
}

bool TextTool::moveCursor(const sf::Vector2i& offset)
{
	if (!m_typing)
		return false;

	auto new_position = m_cursor_position + offset;
	if (
		   new_position.x >= 0 && new_position.x < CurrentImage.getWidth () 
		&& new_position.y >= 0 && new_position.y < CurrentImage.getHeight()
	)
	{
		m_cursor_position = new_position;
		return true;
	}

	return false;
}

void TextTool::eraseCharacter()
{
	if (!moveCursor({ -1, 0 }))
		return;

	setCharacter(m_cursor_position, ' ');
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
					onTypingDone();
					break;

				// Navigation
				case sf::Keyboard::Left:  moveCursor({ -1,  0 }); break;
				case sf::Keyboard::Right: moveCursor({  1,  0 }); break;
				case sf::Keyboard::Up:    moveCursor({  0, -1 }); break;
				case sf::Keyboard::Down:  moveCursor({  0,  1 }); break;

				case sf::Keyboard::Backspace:
					eraseCharacter();
					break;

				case sf::Keyboard::X:
					if (!m_typing)
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

	if (std::iswprint(codepoint))
		putCharacter(codepoint);

	return true;
}

bool TextTool::onMouseButtonPressed(sf::Mouse::Button button)
{
	if (m_typing)
	{
		onTypingDone();
		return false;
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
	m_cursor_position = CurrentPixelCoords;
	m_typing = true;
}

void TextTool::onTypingDone()
{
	m_typing = false;
}

//===========================================