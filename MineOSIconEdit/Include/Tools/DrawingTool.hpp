#pragma once
#include <Tools/Tool.hpp>

//===========================================

// Base class for thools with brush behaiveour
class DrawingTool: public Tool
{
public:
	using Tool::Tool;

	virtual void onUpdate() override;

	virtual bool onMouseButtonPressedInsideImage(sf::Mouse::Button button) override;
	virtual bool onMouseButtonReleased(sf::Mouse::Button button) override;

	// This method is called when the tool is currently being used
	// If onDraw returns true, then the texture will be updated
	virtual bool onDraw() = 0;

protected:
	bool              m_drawing { false };
	sf::Mouse::Button m_drawing_button;

};

//===========================================