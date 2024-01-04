#include <Tools/Tool.hpp>
#include <Main.hpp>

//===========================================

sf::Keyboard::Key Tool::getHotkey() const
{
	return sf::Keyboard::Unknown;
}

void Tool::onUpdate()
{
	
}

bool Tool::onEvent(const sf::Event& event) 
{
	auto& io = ImGui::GetIO();

	switch (event.type)
	{
		case sf::Event::MouseButtonPressed:
			if (!io.WantCaptureMouse)
				return onMouseButtonPressed(event.mouseButton.button);
			
			break;

		case sf::Event::MouseButtonReleased:
			return onMouseButtonReleased(event.mouseButton.button);
			break;
	}

	return false;
}

bool Tool::onMouseButtonPressedInsideImage(sf::Mouse::Button button)
{
	return false;
}

bool Tool::onMouseButtonPressed(sf::Mouse::Button button)
{
	if (IsMouseInsideImage())
		return onMouseButtonPressedInsideImage(button);

	return false;
}

bool Tool::onMouseButtonReleased(sf::Mouse::Button button)
{
	return false;
}

//===========================================