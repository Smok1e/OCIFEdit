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

	virtual bool onDraw(
		sf::Mouse::Button button,
		OCIF::Image& image, 
		OCIF::HexFont& font,
		sf::Image& rasterized_image, 
		const sf::Vector2i& coords
	);

	virtual void processGUI() override;

protected:
	sf::Color m_current_color { sf::Color::White };

};

//===========================================