// Tell Windows.h to not define MIN and MAX macros to be compatiable with SFML
#define NOMINMAX

#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <Windows.h>

#include <SFML/Graphics.hpp>

#include <imgui.h>
#include <imgui-SFML.h>

#include "IconsFontAwesome6.hpp"
#include "OCIF/HexFont.hpp"
#include "OCIF/Image.hpp"

//===========================================

// Constants
constexpr size_t                  BUFFSIZE                = 1024;
const std::filesystem::path       OC_FONT_PATH            = "font.hex";               // Opencomputers font in their own hex format
const std::filesystem::path       RECENT_FILES_LIST_PATH  = "recent.txt";             // Cached list of recently open files
const size_t                      RECENT_FILES_LIST_LIMIT = 10;                       // Limit of recently open files
const sf::Color                   BACKGROUND_COLOR        = sf::Color(24, 24, 24);    // Window background color
sf::Vector2u                      WINDOW_INITIAL_SIZE     = sf::Vector2u(1000, 1000); // Render window size when it is initially opened
const char*                       WINDOW_TITLE            = "OCIF edit";              // Render window title

OCIF::HexFont                     OpencomputersFont;
std::deque<std::filesystem::path> RecentFilesList;
sf::RenderWindow                  RenderWindow;
sf::Cursor                        DefaultCursor;
sf::Cursor                        MovingCursor;
sf::Clock                         DeltaClock;

bool                              ImageLoaded = false;
OCIF::Image                       CurrentImage;
sf::Image                         CurrentRasterizedImage;
sf::Texture                       CurrentRasterizedTexture;
sf::Sprite                        CurrentImageSprite;
std::filesystem::path             CurrentImagePath;
float                             CurrentImageScale = 1.f;

bool                              ShowImageBorder = false;

bool                              Dragging = false;
sf::Vector2i                      DragStartMousePosition;
sf::Vector2i                      DragStartSpritePosition;

//===========================================

bool Initialize();
void StartLoop();
void Update();
void NewFile(unsigned width, unsigned height);
void LoadFile(const std::filesystem::path& path);
void ExportFile(const std::filesystem::path& path);
void MaximizeWindow();

void UpdateTexture();
void ResetImage();
void SetImageScale(float scale);

void OnEvent(const sf::Event& event);
void OnMouseButtonPressed(sf::Mouse::Button button);
void OnMouseButtonReleased(sf::Mouse::Button button);
void OnZoom(int direction);
void OnDragStart();
void OnDragStop();
void OnKeyboardShortcut(sf::Keyboard::Key key);
void OnExit();
void OnFileNew();
void OnFileOpen();
void OnFileExport();

void RenderWorkspace();

void ProcessGUI();
void ProcessGUIMainMenuBar();
void ProcessGUIFileMenu();
void ProcessGUIFileOpenRecentMenu();
void ProcessGUIViewMenu();

void AppendRecentFilesList(const std::filesystem::path& path);
void RemoveFromRecentFilesList(const std::filesystem::path& path);
bool LoadRecentFilesList();
bool SaveRecentFilesList();

//===========================================

int main()
{
	if (!Initialize())
		return 0;

	StartLoop();
}

//===========================================

bool Initialize()
{
	SetConsoleOutputCP(CP_UTF8);

	// Loading opencomputers font from .hex file
	OpencomputersFont.loadFromFile(OC_FONT_PATH);

	// Loading cached recent files list
	LoadRecentFilesList();

	// Cursors
	DefaultCursor.loadFromSystem(sf::Cursor::Arrow);
	MovingCursor.loadFromSystem(sf::Cursor::SizeAll);

	// Initializing window
	RenderWindow.create(sf::VideoMode(WINDOW_INITIAL_SIZE.x, WINDOW_INITIAL_SIZE.y), WINDOW_TITLE);
	MaximizeWindow();

	// ImGui
	ImGui::SFML::Init(RenderWindow);
	
	// Loading icons font
	auto& io = ImGui::GetIO();
	io.Fonts->Clear();
	io.Fonts->AddFontDefault();

	float base_font_size = 18.f;
	float icon_font_size = base_font_size * (2.f / 3.f);

	// Merge icons from fontawesome into default font
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig icons_config = {};
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = icon_font_size;
	io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, icon_font_size, &icons_config, icons_ranges);
	io.Fonts->Build();

	ImGui::SFML::UpdateFontTexture();

	return true;
}

//===========================================

void StartLoop()
{
	while (RenderWindow.isOpen())
	{
		static sf::Event event;
		while (RenderWindow.pollEvent(event))
			OnEvent(event);

		Update();

		RenderWindow.clear(BACKGROUND_COLOR);

		RenderWorkspace();
		ProcessGUI();

		RenderWindow.display();
	}
}

void Update()
{
	ImGui::SFML::Update(RenderWindow, DeltaClock.restart());

	// Update image position if it is currently being dragged
	if (Dragging)
	{
		auto delta = sf::Mouse::getPosition(RenderWindow) - DragStartMousePosition;
		CurrentImageSprite.setPosition(sf::Vector2f(DragStartSpritePosition + delta));
	}
}

void NewFile(unsigned width, unsigned height)
{
	CurrentImage.resize(width, height);
	CurrentImage.clear({
		' ',
		0xFFFFFF,
		0x000000,
		0.0
	});

	CurrentImage.rasterize(CurrentRasterizedImage, OpencomputersFont);
	UpdateTexture();
	ResetImage();

	CurrentImagePath = "Untitled";
}

void LoadFile(const std::filesystem::path& path)
{
	try
	{
		CurrentImage.loadFromFile(path);
		CurrentImage.rasterize(CurrentRasterizedImage, OpencomputersFont);
		UpdateTexture();
		ResetImage();

		AppendRecentFilesList(path);

		ImageLoaded = true;
		CurrentImagePath = path;
	}

	catch (std::exception exc)
	{
		std::wstringstream stream;
		stream << "Unable to load file '" << path.filename().wstring() << "': " << exc.what() << std::endl;

		MessageBoxW(RenderWindow.getSystemHandle(), stream.str().c_str(), L"Unable to load file", MB_ICONERROR | MB_OK);
		RemoveFromRecentFilesList(path);
	}
}

void ExportFile(const std::filesystem::path& path)
{
	CurrentRasterizedImage.saveToFile(path.string());
}

void MaximizeWindow()
{
	// There is no way to maximize window in SFML 
	// so we have to do it by using native Windows API
	PostMessageA(RenderWindow.getSystemHandle(), WM_SYSCOMMAND, SC_MAXIMIZE, 0);
}

//===========================================

void UpdateTexture()
{
	CurrentRasterizedTexture.loadFromImage(CurrentRasterizedImage);
	CurrentImageSprite.setTexture(CurrentRasterizedTexture, true);
}

void ResetImage()
{
	auto sprite_bounds = CurrentImageSprite.getGlobalBounds();
	auto window_size   = RenderWindow.getSize();

	CurrentImageSprite.setPosition(
		window_size.x / 2 - sprite_bounds.width  / 2, 
		window_size.y / 2 - sprite_bounds.height / 2
	);

	SetImageScale(1.f);
}

void SetImageScale(float scale)
{
	CurrentImageScale = std::clamp(scale, .5f, 10.f);

	float scale_factor = std::pow(CurrentImageScale, 2);
	CurrentImageSprite.setScale(scale_factor, scale_factor);
}

//===========================================

void OnEvent(const sf::Event& event)
{
	auto& io = ImGui::GetIO();

	ImGui::SFML::ProcessEvent(event);
	switch (event.type)
	{
		case sf::Event::Closed:
			OnExit();
			break;

		case sf::Event::Resized:
			RenderWindow.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
			break;

		case sf::Event::KeyPressed:
			if (!io.WantCaptureKeyboard && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
				OnKeyboardShortcut(event.key.code);

			break;

		case sf::Event::MouseButtonPressed:
			if (!io.WantCaptureMouse)
				OnMouseButtonPressed(event.mouseButton.button);

			break;

		case sf::Event::MouseButtonReleased:
			if (!io.WantCaptureMouse)
				OnMouseButtonReleased(event.mouseButton.button);

			break;

		case sf::Event::MouseWheelMoved:
			OnZoom(event.mouseWheel.delta);
			break;
	}
}

void OnMouseButtonPressed(sf::Mouse::Button button)
{
	switch (button)
	{
		case sf::Mouse::Middle:
			OnDragStart();
			break;
	}
}

void OnMouseButtonReleased(sf::Mouse::Button button)
{
	switch (button)
	{
		case sf::Mouse::Middle:
			OnDragStop();
			break;
	}
}

void OnZoom(int direction)
{
	SetImageScale(CurrentImageScale + .1f * direction);
}

void OnDragStart()
{
	Dragging = true;
	DragStartMousePosition = sf::Mouse::getPosition(RenderWindow);
	DragStartSpritePosition = sf::Vector2i(CurrentImageSprite.getPosition());

	RenderWindow.setMouseCursor(MovingCursor);
}

void OnDragStop()
{
	Dragging = false;

	RenderWindow.setMouseCursor(DefaultCursor);
}

void OnExit()
{
	RenderWindow.close();
}

void OnFileNew()
{
	NewFile(8, 4);
}

void OnFileOpen()
{
	static wchar_t buffer[BUFFSIZE] = L"";

	OPENFILENAMEW ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = RenderWindow.getSystemHandle();
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = BUFFSIZE;
	ofn.lpstrFilter = L"All\0*.*\0" "OCIF (.pic)\0*.pic\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameW(&ofn))
		LoadFile(ofn.lpstrFile);
}

void OnFileExport()
{
	static wchar_t buffer[BUFFSIZE] = L"";
	CurrentImagePath.stem().wstring().copy(buffer, BUFFSIZE);

	OPENFILENAMEW ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = RenderWindow.getSystemHandle();
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = BUFFSIZE;
	ofn.lpstrFileTitle = nullptr;
	ofn.lpstrDefExt = L"png";
	ofn.lpstrFilter = L"PNG (*.png)\0*.png*\0" "JPEG (*.jpg)\0.jpg\0" "BMP (*.bmp)\0*.bmp\0" "Targa (*.tga)\0*.tga\0";
	ofn.nFilterIndex = 1;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;

	if (GetSaveFileNameW(&ofn))
		ExportFile(ofn.lpstrFile);
}

void OnKeyboardShortcut(sf::Keyboard::Key key)
{
	switch (key)
	{
		case sf::Keyboard::N:
			OnFileNew();
			break;

		case sf::Keyboard::O:
			OnFileOpen();
			break;

		case sf::Keyboard::E:
			if (ImageLoaded)
				OnFileExport();

			break;

		case sf::Keyboard::W:
			OnExit();
			break;
	}
}

//===========================================

void RenderWorkspace()
{
	if (ShowImageBorder)
	{
		auto sprite_bounds = CurrentImageSprite.getGlobalBounds();

		static sf::RectangleShape rect;
		rect.setFillColor(sf::Color::Transparent);
		rect.setOutlineColor(sf::Color::White);
		rect.setOutlineThickness(1.f);
		rect.setPosition(sprite_bounds.left, sprite_bounds.top);
		rect.setSize(sf::Vector2f(sprite_bounds.width, sprite_bounds.height));

		RenderWindow.draw(rect);
	}

	RenderWindow.draw(CurrentImageSprite);
}

void ProcessGUI()
{
	ProcessGUIMainMenuBar();
	ImGui::SFML::Render(RenderWindow);
}

void ProcessGUIMainMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ProcessGUIFileMenu();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			ProcessGUIViewMenu();
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void ProcessGUIFileMenu()
{
	if (ImGui::MenuItem(ICON_FA_FILE " New", "^N"))
		OnFileNew();
		
	if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open", "^O"))
		OnFileOpen();

	if (ImGui::BeginMenu(ICON_FA_CUBE " Open recent", !RecentFilesList.empty()))
	{
		ProcessGUIFileOpenRecentMenu();
		ImGui::EndMenu();
	}

	// Show tooltip if open recent item is disabled
	if (RecentFilesList.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("You have not opened any file yet");

	if (ImGui::MenuItem(ICON_FA_FILE_EXPORT " Export", "^E", nullptr, ImageLoaded))
		OnFileExport();

	// Show tooltip if the export item is disabled
	if (!ImageLoaded && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("You have not loaded image yet");

	if (ImGui::MenuItem(ICON_FA_XMARK " Exit", "^W"))
		OnExit();
}

void ProcessGUIFileOpenRecentMenu()
{
	for (const auto& path: RecentFilesList)
	{
		// c++20 utf-8 strings are represented in char8_t instead of char
		if (ImGui::MenuItem(reinterpret_cast<const char*>(path.filename().u8string().c_str())))
		{
			LoadFile(path);
			break;
		}
	}
}

void ProcessGUIViewMenu()
{
	ImGui::MenuItem(ICON_FA_SQUARE " Image border", nullptr, &ShowImageBorder);
}

//===========================================

void AppendRecentFilesList(const std::filesystem::path& path)
{
	auto iter = std::find(RecentFilesList.begin(), RecentFilesList.end(), path);
	if (iter == RecentFilesList.end())
	{
		RecentFilesList.push_front(path);
		if (RecentFilesList.size() > RECENT_FILES_LIST_LIMIT)
			RecentFilesList.pop_back();
	}

	else if (iter != RecentFilesList.begin())
		// Move found path to top of the list
		std::rotate(RecentFilesList.begin(), iter, iter+1);

	SaveRecentFilesList();
}

void RemoveFromRecentFilesList(const std::filesystem::path& path)
{
	auto iter = std::find(RecentFilesList.begin(), RecentFilesList.end(), path);
	if (iter != RecentFilesList.end())
		RecentFilesList.erase(iter);

	SaveRecentFilesList();
}

bool LoadRecentFilesList()
{
	std::ifstream stream(RECENT_FILES_LIST_PATH, std::ios::in);
	if (stream)
	{
		RecentFilesList.clear();

		std::string line;
		while (std::getline(stream, line))
			RecentFilesList.push_back(reinterpret_cast<const char8_t*>(line.c_str()));

		return true;
	}

	return false;
}

bool SaveRecentFilesList()
{
	std::ofstream stream(RECENT_FILES_LIST_PATH, std::ios::out);
	if (!stream)
	{
		std::cerr << "Unable to save recent files list" << std::endl;
		return false;
	}

	for (const auto& path: RecentFilesList)
		stream << reinterpret_cast<const char*>(path.u8string().c_str()) << std::endl;

	return true;
}

//===========================================