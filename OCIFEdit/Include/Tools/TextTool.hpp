#pragma once
#include <Tools/Tool.hpp>

//===========================================

// Text tool
class TextTool: public Tool
{
public:
	using Tool::Tool;

	virtual const char* getIcon() const override;
	virtual const char* getName() const override;
	virtual sf::Keyboard::Key getHotkey() const;

	virtual void onRenderWorkspace() override;
	virtual void processGUI() override;

	virtual void setCharacter(const sf::Vector2i& position, uint32_t codepoint);
	virtual void putCharacter(uint32_t codepoint);
	virtual bool moveCursor(const sf::Vector2i& offset);
	virtual void eraseCharacter();

	virtual bool onEvent(const sf::Event& event);
	virtual bool onTextEntered(uint32_t codepoint);
	virtual bool onMouseButtonPressed(sf::Mouse::Button button) override;
	virtual bool onMouseButtonPressedInsideImage(sf::Mouse::Button button) override;
	
	virtual void onTypingStarted();
	virtual void onTypingDone();

protected:
	sf::Color m_current_background { sf::Color::Black };
	sf::Color m_current_foreground { sf::Color::White };
	bool m_transparent_background { false };

	bool m_typing { false };
	sf::Vector2i m_cursor_position;

};

//===========================================