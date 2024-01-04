#pragma once
#include <Tools/DrawingTool.hpp>

//===========================================

// Braille subpixel brush
class BrailleTool: public DrawingTool
{
public:
	using DrawingTool::DrawingTool;

	virtual const char* getIcon() const override;
	virtual const char* getName() const override;
	virtual sf::Keyboard::Key getHotkey() const;

	virtual bool onDraw() override;
	virtual void onRenderWorkspace() override;
	virtual void processGUI() override;
	
	virtual bool onEvent(const sf::Event& event) override;

protected:
	sf::Color m_current_background { sf::Color::Black };
	sf::Color m_current_foreground { sf::Color::White };
	bool m_transparent_background { false };
	bool m_show_crosshair { true };
	bool m_show_hovered_pixel { true };

};

//===========================================