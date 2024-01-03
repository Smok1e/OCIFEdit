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

	// This method should return hotkey which will activate the tool
	// If tool does not needs hotkey, this method should return sf::Key::Unknown
	virtual sf::Keyboard::Key getHotkey() const;

	// This method is called when the tool is used on pixel
	// If onDraw returns true, image texture will be updated
	virtual bool onDraw(sf::Mouse::Button button) = 0;

	// This method is called after workspace is rendered
	virtual void onRenderWorkspace() = 0;

	// This method is called when tool's window is being processed
	virtual void processGUI() = 0;

	// This method is called on key press event
	// If onKeyPressed returns true, then event 
	// propagination will be stopped
	virtual bool onKeyPressed(sf::Keyboard::Key);
};

//===========================================