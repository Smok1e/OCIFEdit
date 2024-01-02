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
#include <deque>

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
	const size_t                RecentFilesListLimit    = 10;
	const std::filesystem::path RecentFilesListLocation = "recent.txt";

	bool             m_initialized { false };
	sf::RenderWindow m_window;
	sf::Clock        m_clock;
	OCIF::Image      m_ocif_image;
	sf::Image        m_image;
	sf::Texture      m_texture;
	sf::Sprite       m_sprite;
	float            m_scale { 1.f };
	OCIF::HexFont    m_font;

	bool         m_dragging { false };
	sf::Vector2i m_drag_mouse_position;
	sf::Vector2i m_drag_sprite_position;
	
	sf::Vector2i m_current_image_coords;
	sf::Vector2i m_current_pixel_coords;
	sf::Vector2i m_current_braille_coords;
	sf::Color    m_current_color { sf::Color::White };
	bool         m_drawing       { false };

	sf::Cursor m_default_cursor;
	sf::Cursor m_moving_cursor;

	bool m_show_border        { false };
	bool m_show_tools_window  { true  };
	bool m_show_pixel_frame   { false };
	bool m_show_braille_frame { true  };

	std::deque<std::filesystem::path> m_recent_files;

	void init();
	void loadFile(const std::filesystem::path& path);
	void newFile(unsigned width, unsigned height);
	void rasterizeImage();
	void updateTexture();
	void centerImage();

	void appendRecentFilesList(const std::filesystem::path& path);
	void loadRecentFilesList();
	void saveRecentFilesList();

	void processEvent(sf::Event& event);
	void onKeyboardShortcut(sf::Keyboard::Key key);
	void onMouseButtonPressed(sf::Event& event);
	void onMouseButtonReleased(sf::Event& event);
	void onDragStart();
	void onDragStop();
	void onZoom(int direction);
	void onDrawStart();
	void onDrawStop();
	void onDraw();
	void onPickColor();

	void onRenderWorkspace();
	void onRenderBorder();

	void onUpdate();

	void processGUI();
	void processGUIMainMenuBar();
	void processGUIFileMenu();
	void processGUIViewMenu();
	void processGUIToolsWindow();

	void onFileNew();
	void onFileOpen();
	void onExit();

	void setScale(float scale);

	sf::Vector2i windowToPixelCoords(const sf::Vector2f& coords);
	sf::Vector2f pixelToWindowCoords(const sf::Vector2i& coords);
	sf::Vector2i windowToBrailleCoords(const sf::Vector2f& coords);
	sf::Vector2f brailleToWindowCoords(const sf::Vector2i& coords);
	sf::Vector2i windowToImageCoords(const sf::Vector2f& coords);
	sf::Vector2f imageToWindowCoords(const sf::Vector2i& coords);
	bool isMouseInsideImage();

	OCIF::Pixel& getCurrentPixel();
};

//===========================================