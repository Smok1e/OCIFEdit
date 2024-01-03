#include <Tools/Tool.hpp>

//===========================================

sf::Keyboard::Key Tool::getHotkey() const
{
	return sf::Keyboard::Unknown;
}

bool Tool::onKeyPressed(sf::Keyboard::Key key)
{
	return false;
}

//===========================================