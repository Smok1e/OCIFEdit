#pragma once
#include <cstdint>

#include <SFML/Graphics.hpp>

#include "OCIF/Image.hpp"

//===========================================

// Base tool class
class Tool
{
public:
	Tool() = default;

	// This method should return icon codepoint to display the tool
	virtual const char* getIcon() const = 0;

	// This method should return tool name
	virtual const char* getName() const = 0;

	// This method is called when the tool is used on pixel
	// If onDraw returns true, image texture will be updated
	virtual bool onDraw(
		sf::Mouse::Button button,
		OCIF::Image& image, 
		OCIF::HexFont& font,
		sf::Image& rasterized_image, 
		const sf::Vector2i& coords
	) = 0;

	// This method is called when tool's window is being processed
	virtual void processGUI() = 0;
};

//===========================================