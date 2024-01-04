#pragma once
#include <SFML/Graphics.hpp>

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

	// This method is called after workspace is rendered
	virtual void onRenderWorkspace() = 0;

	// This method is called every frame before rendering anything
	virtual void onUpdate();

	// This method is called when tool's window is being processed
	virtual void processGUI() = 0;

	// This method is called when window got event
	// If onEvent returns true, then event propagination will be stopped
	virtual bool onEvent(const sf::Event& event);

	// This method is called if mouse button was pressed
	virtual bool onMouseButtonPressed(sf::Mouse::Button button);

	// This method is called if mouse button was released
	virtual bool onMouseButtonReleased(sf::Mouse::Button button);

	// This method is called if mouse button was pressed while
	// mouse cursor was intersecting visible image area
	virtual bool onMouseButtonPressedInsideImage(sf::Mouse::Button button);
};

//===========================================