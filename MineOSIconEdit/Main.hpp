#pragma once

#define NOMINMAX

#include <Windows.h>

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include <fstream>
#include <vector>
#include <string>
#include <codecvt>
#include <filesystem>

#include "Image.hpp"
#include "HexFont.hpp"

//===========================================

constexpr size_t BUFFSIZE = 1024;

//===========================================

class Main
{
public:
	Main();

	void start();

protected:
	bool              m_initialized { false };
	sf::RenderWindow  m_window;
	sf::Clock         m_clock;
	OCIF::Image       m_ocif_image;
	sf::Image         m_image;
	sf::Texture       m_texture;
	sf::Sprite        m_sprite;
	bool              m_dragging { false };
	sf::Vector2i      m_drag_mouse_position;
	sf::Vector2i      m_drag_sprite_position;
	float             m_scale { 1.f };
	OCIF::HexFont     m_font;

	bool m_show_border { false };

	void init();
	void loadFile(const std::filesystem::path& path);

	void processEvent(sf::Event event);
	void onKeyboardShortcut(sf::Keyboard::Key key);
	void onDragStart();
	void onDragStop();
	void onZoom(int direction);

	void onRenderWorkspace();
	void onRenderBorder();
	void onRenderPixelInfo();

	void onUpdate();

	void processGUIMainMenuBar();
	void processGUIFileMenu();
	void processGUIViewMenu();

	void onExit();
	void onFileOpen();

	void setScale(float scale);

	sf::Vector2i windowToImageCoords(const sf::Vector2f& coords);
	sf::Vector2f imageToWindowCoords(const sf::Vector2i& coords);
};

//===========================================