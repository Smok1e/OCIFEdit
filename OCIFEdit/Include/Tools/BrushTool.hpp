#pragma once
#include <Tools/DrawingTool.hpp>

//===========================================

// The brush
class BrushTool: public DrawingTool
{
public:
	using DrawingTool::DrawingTool;

	virtual const char* getIcon() const override;
	virtual const char* getName() const override;
	virtual sf::Keyboard::Key getHotkey() const;

	virtual bool onDraw() override;

	virtual void onRenderWorkspace() override;
	virtual void processGUI() override;

protected:
	sf::Color m_current_color { sf::Color::White };
	bool     m_show_crosshair { true };

};

//===========================================