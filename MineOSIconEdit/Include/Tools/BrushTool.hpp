#pragma once

#include "Tool.hpp"

//===========================================

// The brush
class BrushTool: public Tool
{
public:
	using Tool::Tool;

	virtual const char* getIcon() const override;
	virtual const char* getName() const override;
	virtual sf::Keyboard::Key getHotkey() const;

	virtual bool onDraw(sf::Mouse::Button button) override;
	virtual void onRenderWorkspace() override;
	virtual void processGUI() override;

protected:
	sf::Color m_current_color { sf::Color::White };
	bool m_show_crosshair { true };

};

//===========================================