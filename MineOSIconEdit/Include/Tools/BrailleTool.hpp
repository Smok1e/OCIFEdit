#pragma once

#include "Tool.hpp"

//===========================================

// Braille subpixel brush
class BrailleTool: public Tool
{
public:
	using Tool::Tool;

	virtual const char* getIcon() const override;
	virtual const char* getName() const override;
	virtual sf::Keyboard::Key getHotkey() const;

	virtual bool onDraw(sf::Mouse::Button button) override;
	virtual void onRenderWorkspace() override;
	virtual void processGUI() override;
	virtual bool onKeyPressed(sf::Keyboard::Key key) override;

protected:
	sf::Color m_current_background { sf::Color::White };
	sf::Color m_current_foreground { sf::Color::Black };
	bool m_transparent_background { false };
	bool m_show_crosshair { true };

};

//===========================================