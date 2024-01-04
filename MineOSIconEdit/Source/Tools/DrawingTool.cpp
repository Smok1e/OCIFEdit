#include <Tools/DrawingTool.hpp>
#include <Main.hpp>

//===========================================

void DrawingTool::onUpdate()
{
	if (m_drawing && IsMouseInsideImage())
		if (onDraw()) UpdateTexture();
}

bool DrawingTool::onMouseButtonPressedInsideImage(sf::Mouse::Button button)
{
	switch (button)
	{
		case sf::Mouse::Left:
		case sf::Mouse::Right:
			m_drawing = true;
			m_drawing_button = button;

			return true;
			break;
	}

	return false;
}

bool DrawingTool::onMouseButtonReleased(sf::Mouse::Button button)
{
	switch (button)
	{
		case sf::Mouse::Left:
		case sf::Mouse::Right:
			m_drawing = false;

			return true;
			break;
	}

	return false;
}

//===========================================