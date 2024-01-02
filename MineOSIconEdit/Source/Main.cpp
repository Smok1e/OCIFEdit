// Tell Windows.h to not define MIN and MAX macros to be compatiable with SFML
#define NOMINMAX

#include <deque>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <Windows.h>

#include <SFML/Graphics.hpp>

#include <imgui.h>
#include <imgui-SFML.h>

#include "IconsMaterialDesign.hpp"

#include "OCIF/HexFont.hpp"
#include "OCIF/Image.hpp"

#include "Tools/Brush.hpp"

//===========================================

// Constants
constexpr size_t            BUFFSIZE                = 1024;
const std::filesystem::path RESOURCES_BASE_DIR      = "Resources";              // Resources directory path
const std::filesystem::path GUI_FONT_PATH           = "CascadiaCode.ttf";       // Unicode font used to display text in GUI
constexpr float             GUI_FONT_SIZE           = 18.f;                     // GUI text size
const std::filesystem::path OC_FONT_PATH            = "font.hex";               // Opencomputers font in their own hex format
const std::filesystem::path RECENT_FILES_LIST_PATH  = "recent.txt";             // Cached list of recently open files
constexpr size_t            RECENT_FILES_LIST_LIMIT = 10;                       // Limit of recently open files
const sf::Color             BACKGROUND_COLOR        = sf::Color(24, 24, 24);    // Window background color
sf::Vector2u                WINDOW_INITIAL_SIZE     = sf::Vector2u(1000, 1000); // Render window size when it is initially opened
const char*                 WINDOW_TITLE            = "OCIF edit";              // Render window title
const std::filesystem::path WINDOW_ICON_PATH        = "Icon.png";               // Render window icon

// Context variables
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

bool                              ShowImageBorder        = false;
bool                              ShowImGuiDemoWindow    = false;
bool                              ShowCurrentPixelBorder = false;
bool                              ShowCurrentPixelFrame  = false;

bool                              Dragging = false;
sf::Vector2i                      DragStartMousePosition;
sf::Vector2i                      DragStartSpritePosition;

// Tools
bool               ShowToolsWindow = true;
std::vector<Tool*> Tools;
Tool*              CurrentTool = nullptr;
bool               Drawing = false;
sf::Mouse::Button  DrawingButton;
sf::Vector2i       CurrentPixelCoords;

// Popups state
bool                  NewFilePopupOpened     = false;
bool                  MessageBoxPopupOpened  = false;
std::string           MessageBoxPopupTitle   = "";
std::string           MessageBoxPopupMessage = "";
bool                  ExportPopupOpened      = false;
std::filesystem::path ExportPath             = "";

//===========================================

bool Initialize();
void StartLoop();
void Cleanup();

void Update();
void NewFile(int width, int height, OCIF::Color color);
void LoadFile(const std::filesystem::path& path);
void SaveFile(const std::filesystem::path& path);
void ExportFile(const std::filesystem::path& path, float scale);
void MaximizeWindow();
void ShowMessageBox(std::string_view title, std::string_view message);

bool IsMouseInsideImage();

void UpdateTexture();
void ResetImage();
void SetImageScale(float scale);

void OnEvent(const sf::Event& event);
void OnMouseButtonPressed(sf::Mouse::Button button);
void OnMouseButtonReleased(sf::Mouse::Button button);
void OnZoom(int direction);
void OnDragStart();
void OnDragStop();
void OnKeyPressed(sf::Keyboard::Key key);
void OnKeyboardShortcut(sf::Keyboard::Key key);
void OnExit();
void OnFileNew();
void OnFileOpen();
void OnFileSave();
void OnFileExport();
void OnDrawStart(sf::Mouse::Button button);
void OnDrawStop();
void OnDraw(sf::Mouse::Button button);

void RenderWorkspace();

void CenterNextWindow();
void ProcessGUI();
void ProcessGUIMainMenuBar();
void ProcessGUIFileMenu();
void ProcessGUIFileOpenRecentMenu();
void ProcessGUIViewMenu();
void ProcessGUIDebugMenu();

void ProcessGUIPopups();
void ProcessGUIFileNewPopup();
void ProcessGUIMessageBoxPopup();
void ProcessGUIExportPopup();

void ProcessGUIToolsWindow();

void AddToRecentFilesList(const std::filesystem::path& path);
void RemoveFromRecentFilesList(const std::filesystem::path& path);
bool LoadRecentFilesList();
bool SaveRecentFilesList();

sf::Vector2i WindowToPixelCoords(const sf::Vector2i& mouse);
sf::Vector2i PixelToWindowCoords(const sf::Vector2i& pixel);

//===========================================

int main()
{
	if (!Initialize())
		return 0;

	StartLoop();
	Cleanup();
}

//===========================================

bool Initialize()
{
	SetConsoleOutputCP(CP_UTF8);

	// Loading opencomputers font from .hex file
	OpencomputersFont.loadFromFile(RESOURCES_BASE_DIR/OC_FONT_PATH);

	// Loading cached recent files list
	LoadRecentFilesList();

	// Cursors
	DefaultCursor.loadFromSystem(sf::Cursor::Arrow);
	MovingCursor.loadFromSystem(sf::Cursor::SizeAll);

	// Initializing window
	RenderWindow.create(sf::VideoMode(WINDOW_INITIAL_SIZE.x, WINDOW_INITIAL_SIZE.y), WINDOW_TITLE);
	RenderWindow.setVerticalSyncEnabled(true);

	sf::Image icon;
	if (icon.loadFromFile((RESOURCES_BASE_DIR/WINDOW_ICON_PATH).string()))
		RenderWindow.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

	MaximizeWindow();

	// ImGui
	ImGui::SFML::Init(RenderWindow);

	// Loading fonts
	auto& io = ImGui::GetIO();
	io.Fonts->Clear();

	// Loading font for displaying UTF-8 encoded text
	static const ImWchar utf8_ranges[] = { 0x20, 0xFFFF, 0 };
	io.Fonts->AddFontFromFileTTF((RESOURCES_BASE_DIR/GUI_FONT_PATH).string().c_str(), GUI_FONT_SIZE, nullptr, utf8_ranges);

	// Loading MaterialDesign and merging icons glyphs to previous font
	static const ImWchar icons_ranges[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
	ImFontConfig icons_config = {};
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphOffset.y = 5;
	icons_config.GlyphMinAdvanceX = GUI_FONT_SIZE;
	io.Fonts->AddFontFromFileTTF((RESOURCES_BASE_DIR/FONT_ICON_FILE_NAME_MD).string().c_str(), GUI_FONT_SIZE * 1.5f, &icons_config, icons_ranges);
	io.Fonts->Build();

	ImGui::SFML::UpdateFontTexture();

	// Rounded corners
	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding  = 5.f;
	style.WindowRounding = 5.f;
	style.PopupRounding  = 5.f;

	// Initializing tools
	Tools.push_back(new BrushTool);
	CurrentTool = Tools[0];

	return true;
}

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

void Cleanup()
{
	for (auto& instance: Tools)
		delete instance;
}

//===========================================

void Update()
{
	ImGui::SFML::Update(RenderWindow, DeltaClock.restart());

	// Update image position if it is currently being dragged
	if (Dragging)
	{
		auto delta = sf::Mouse::getPosition(RenderWindow) - DragStartMousePosition;
		CurrentImageSprite.setPosition(sf::Vector2f(DragStartSpritePosition + delta));
	}

	auto last_pixel_coords = CurrentPixelCoords;
	auto sprite_bounds = CurrentImageSprite.getGlobalBounds();
	CurrentPixelCoords = WindowToPixelCoords(sf::Mouse::getPosition(RenderWindow));
	if (Drawing && last_pixel_coords != CurrentPixelCoords && IsMouseInsideImage())
		OnDraw(DrawingButton);
}

void NewFile(int width, int height, OCIF::Color color)
{
	if (width < 1 || height < 1)
	{
		ShowMessageBox("Unable to create image", "Invalid image size");
		return;
	}

	CurrentImage.resize(width, height);
	CurrentImage.clear({
		' ',
		color,
		0x000000,
		0.0
	});

	CurrentImage.rasterize(CurrentRasterizedImage, OpencomputersFont);
	UpdateTexture();
	ResetImage();

	CurrentImagePath = "Untitled";
	ImageLoaded = true;
}

void LoadFile(const std::filesystem::path& path)
{
	try
	{
		CurrentImage.loadFromFile(path);
		CurrentImage.rasterize(CurrentRasterizedImage, OpencomputersFont);
		UpdateTexture();
		ResetImage();

		AddToRecentFilesList(path);

		ImageLoaded = true;
		CurrentImagePath = path;
	}

	catch (std::exception exc)
	{
		std::stringstream stream;
		stream << "Unable to load file '" << reinterpret_cast<const char*>(path.filename().u8string().c_str()) << "': " << exc.what() << std::endl;

		ShowMessageBox("Unable to load file", stream.str());
		RemoveFromRecentFilesList(path);
	}
}

void SaveFile(const std::filesystem::path& path)
{
	try
	{
		CurrentImage.saveToFile(path);
		AddToRecentFilesList(path);
	}

	catch (std::exception exc)
	{
		std::stringstream stream;
		stream << "Unable to save file '" << reinterpret_cast<const char*>(path.filename().u8string().c_str()) << "': " << exc.what() << std::endl;

		ShowMessageBox("Unable to save file", stream.str());
	}
}

void ExportFile(const std::filesystem::path& path, float scale)
{
	sf::Texture texture;
	texture.loadFromImage(CurrentRasterizedImage);

	sf::Sprite sprite;
	sprite.setTexture(CurrentRasterizedTexture);
	sprite.setScale(scale, scale);

	auto bounds = sprite.getGlobalBounds();
	sf::RenderTexture render_texture;
	render_texture.create(bounds.width, bounds.height);
	render_texture.draw(sprite);
	render_texture.display();
	render_texture.getTexture().copyToImage().saveToFile(path.string());
}

void MaximizeWindow()
{
	// There is no way to maximize window in SFML 
	// so we have to do it by using native Windows API
	PostMessageA(RenderWindow.getSystemHandle(), WM_SYSCOMMAND, SC_MAXIMIZE, 0);
}

void ShowMessageBox(std::string_view title, std::string_view message)
{
	MessageBoxPopupTitle   = std::string(title) + "##msgbox";
	MessageBoxPopupMessage = message;
	MessageBoxPopupOpened  = true;
}

//===========================================

bool IsMouseInsideImage()
{
	return CurrentPixelCoords.x >= 0 && CurrentPixelCoords.x < CurrentImage.getWidth() && CurrentPixelCoords.y >= 0 && CurrentPixelCoords.y < CurrentImage.getHeight();
}

//===========================================

void UpdateTexture()
{
	CurrentRasterizedTexture.loadFromImage(CurrentRasterizedImage);
	CurrentImageSprite.setTexture(CurrentRasterizedTexture, true);
}

void ResetImage()
{
	SetImageScale(1.f);

	auto sprite_bounds = CurrentImageSprite.getGlobalBounds();
	auto window_size   = RenderWindow.getSize();

	CurrentImageSprite.setPosition(
		window_size.x / 2 - sprite_bounds.width  / 2, 
		window_size.y / 2 - sprite_bounds.height / 2
	);
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
			OnKeyPressed(event.key.code);
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

		case sf::Mouse::Right:
		case sf::Mouse::Left:
			if (IsMouseInsideImage())
				OnDrawStart(button);

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

		case sf::Mouse::Left:
			OnDrawStop();
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
	NewFilePopupOpened = true;
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

void OnFileSave()
{
	static wchar_t buffer[BUFFSIZE] = L"";
	CurrentImagePath.stem().wstring().copy(buffer, BUFFSIZE);

	OPENFILENAMEW ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = RenderWindow.getSystemHandle();
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = BUFFSIZE;
	ofn.lpstrFileTitle = nullptr;
	ofn.lpstrDefExt = L"pic";
	ofn.lpstrFilter = L"All\0*.*\0" "OCIF (.pic)\0*.pic\0";
	ofn.nFilterIndex = 1;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;

	if (GetSaveFileNameW(&ofn))
		SaveFile(ofn.lpstrFile);
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
	{
		ExportPath = ofn.lpstrFile;
		ExportPopupOpened = true;
	}
}

void OnDrawStart(sf::Mouse::Button button)
{
	Drawing = true;
	DrawingButton = button;
	OnDraw(button);
}

void OnDrawStop()
{
	Drawing = false;
}

void OnDraw(sf::Mouse::Button button)
{
	if (CurrentTool->onDraw(button, CurrentImage, OpencomputersFont, CurrentRasterizedImage, CurrentPixelCoords))
		UpdateTexture();
}

void OnKeyPressed(sf::Keyboard::Key key)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
	{
		OnKeyboardShortcut(key);
		return;
	}

	switch (key)
	{
		case sf::Keyboard::Escape:
			NewFilePopupOpened = false;
			MessageBoxPopupOpened = false;
			break;
	}
}

void OnKeyboardShortcut(sf::Keyboard::Key key)
{
	switch (key)
	{
		// File -> New
		case sf::Keyboard::N:
			OnFileNew();
			break;

		// File -> Open
		case sf::Keyboard::O:
			OnFileOpen();
			break;

		// File -> Save
		case sf::Keyboard::S:
			if (ImageLoaded)
				OnFileSave();

			break;

		// File -> Export
		case sf::Keyboard::E:
			if (ImageLoaded)
				OnFileExport();

			break;

		// File -> Exit
		case sf::Keyboard::W:
			OnExit();
			break;
	}
}

//===========================================

void RenderWorkspace()
{
	RenderWindow.draw(CurrentImageSprite);

	static sf::RectangleShape rect;
	if (ShowImageBorder)
	{
		auto sprite_bounds = CurrentImageSprite.getGlobalBounds();

		rect.setFillColor(sf::Color::Transparent);
		rect.setOutlineColor(sf::Color::White);
		rect.setOutlineThickness(1.f);
		rect.setPosition(sprite_bounds.left, sprite_bounds.top);
		rect.setSize(sf::Vector2f(sprite_bounds.width, sprite_bounds.height));

		RenderWindow.draw(rect);
	}

	if (IsMouseInsideImage())
	{
		rect.setFillColor(sf::Color(255, 255, 255, 100));

		if (ShowCurrentPixelBorder)
		{
			auto sprite_scale = CurrentImageSprite.getScale();

			rect.setOutlineColor(sf::Color::White);
			rect.setOutlineThickness(1.f);
			rect.setPosition(sf::Vector2f(PixelToWindowCoords(CurrentPixelCoords)));
			rect.setSize(sf::Vector2f(sprite_scale.x * OCIF::HexFont::Glyph::DefaultWidth, sprite_scale.y * OCIF::HexFont::Glyph::DefaultHeight));

			RenderWindow.draw(rect);
		}

		if (ShowCurrentPixelFrame)
		{
			auto sprite_bounds = CurrentImageSprite.getGlobalBounds();

			auto src_coords = PixelToWindowCoords(CurrentPixelCoords);
			auto dst_coords = PixelToWindowCoords(CurrentPixelCoords + sf::Vector2i(1, 1));

			rect.setOutlineThickness(0);

			rect.setPosition(sprite_bounds.left, src_coords.y);
			rect.setSize(sf::Vector2f(sprite_bounds.width, 1));
			RenderWindow.draw(rect);

			rect.setPosition(sprite_bounds.left, dst_coords.y);
			RenderWindow.draw(rect);

			rect.setPosition(src_coords.x, sprite_bounds.top);
			rect.setSize(sf::Vector2f(1, sprite_bounds.height));
			RenderWindow.draw(rect);

			rect.setPosition(dst_coords.x, sprite_bounds.top);
			RenderWindow.draw(rect);
		}
	}
}

//===========================================

void CenterNextWindow()
{
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(.5f, .5f));
}

void ProcessGUI()
{
	ProcessGUIMainMenuBar();
	ProcessGUIPopups();

	if (ShowToolsWindow)
		ProcessGUIToolsWindow();

	if (ShowImGuiDemoWindow)
		ImGui::ShowDemoWindow(&ShowImGuiDemoWindow);

	static char buffer[BUFFSIZE] = "";
	sprintf_s(buffer, "%s settings", CurrentTool->getName());
	if (ImGui::Begin(buffer))
	{
		CurrentTool->processGUI();
		ImGui::End();
	}

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

		if (ImGui::BeginMenu("Debug"))
		{
			ProcessGUIDebugMenu();
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void ProcessGUIFileMenu()
{
	if (ImGui::MenuItem(ICON_MD_CREATE_NEW_FOLDER " New", "^N"))
		OnFileNew();
		
	if (ImGui::MenuItem(ICON_MD_FOLDER_OPEN " Open", "^O"))
		OnFileOpen();

	if (ImGui::BeginMenu(ICON_MD_SCHEDULE " Open recent", !RecentFilesList.empty()))
	{
		ProcessGUIFileOpenRecentMenu();
		ImGui::EndMenu();
	}

	// Show tooltip if open recent item is disabled
	if (RecentFilesList.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("You have not opened any file yet");

	if (ImGui::MenuItem(ICON_MD_SAVE " Save", "^S", nullptr, ImageLoaded))
		OnFileSave();

	// Show tooltip if the save item is disabled
	if (!ImageLoaded && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("You have not loaded image yet");

	if (ImGui::MenuItem(ICON_MD_DRIVE_FILE_MOVE " Export", "^E", nullptr, ImageLoaded))
		OnFileExport();

	// Show tooltip if the export item is disabled
	if (!ImageLoaded && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("You have not loaded image yet");

	ImGui::Separator();
	if (ImGui::MenuItem(ICON_MD_CLOSE " Exit", "^W"))
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
	ImGui::MenuItem("Tools window", nullptr, &ShowToolsWindow);
	ImGui::MenuItem("Image border", nullptr, &ShowImageBorder);
	ImGui::MenuItem("Pixel border", nullptr, &ShowCurrentPixelBorder);
	ImGui::MenuItem("Pixel frame",  nullptr, &ShowCurrentPixelFrame);
}

void ProcessGUIDebugMenu()
{
	if (ImGui::MenuItem("Show ImGui demo"))
		ShowImGuiDemoWindow = true;
}

//===========================================

void ProcessGUIPopups()
{
	if (NewFilePopupOpened)
		ImGui::OpenPopup("Create new image");

	CenterNextWindow();
	if (ImGui::BeginPopupModal("Create new image", &NewFilePopupOpened, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ProcessGUIFileNewPopup();
		ImGui::EndPopup();
	}

	if (MessageBoxPopupOpened)
		ImGui::OpenPopup(MessageBoxPopupTitle.c_str());

	CenterNextWindow();
	if (ImGui::BeginPopupModal(MessageBoxPopupTitle.c_str(), &MessageBoxPopupOpened, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ProcessGUIMessageBoxPopup();
		ImGui::EndPopup();
	}

	if (ExportPopupOpened)
		ImGui::OpenPopup("Export image");

	CenterNextWindow();
	if (ImGui::BeginPopupModal("Export image", &ExportPopupOpened, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ProcessGUIExportPopup();
		ImGui::EndPopup();
	}
}

void ProcessGUIFileNewPopup()
{
	static int size[2] = { 8, 4 };
	ImGui::InputInt2("Image size", size);

	static float color[3] = { 1.f, 1.f, 1.f };
	ImGui::ColorPicker3("Fill color", color);

	if (ImGui::Button("Ok"))
	{
		NewFile(size[0], size[1], OCIF::NormalizeColor(OCIF::To24BitColor(color)));
		NewFilePopupOpened = false;
	}
}

void ProcessGUIMessageBoxPopup()
{
	ImGui::Text(MessageBoxPopupMessage.c_str());

	if (ImGui::Button("Ok"))
		MessageBoxPopupOpened = false;
}

void ProcessGUIExportPopup()
{
	static float scale = 1.f;
	ImGui::SliderFloat("Output scale", &scale, .1f, 10.f);

	auto image_size = CurrentRasterizedImage.getSize();
	ImGui::Text("Output (scaled) resolution: %.0fx%.0f", image_size.x * scale, image_size.y * scale);

	if (ImGui::Button("Ok"))
	{
		ExportFile(ExportPath, scale);
		ExportPopupOpened = false;
	}
}

//===========================================

void ProcessGUIToolsWindow()
{
	if (ImGui::Begin("Tools", &ShowToolsWindow))
	{
		for (Tool* tool: Tools)
		{
			static char buffer[BUFFSIZE] = "";
			sprintf_s(buffer, "%s %s", tool->getIcon(), tool->getName());
			if (ImGui::Selectable(buffer, tool == CurrentTool))
				CurrentTool = tool;
		}
	}

	ImGui::End();
}

//===========================================

void AddToRecentFilesList(const std::filesystem::path& path)
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

sf::Vector2i WindowToPixelCoords(const sf::Vector2i& mouse)
{
	auto sprite_bounds = CurrentImageSprite.getGlobalBounds();

	return sf::Vector2i(
		std::floor(((mouse.x - sprite_bounds.left) / sprite_bounds.width ) * CurrentImage.getWidth ()),
		std::floor(((mouse.y - sprite_bounds.top ) / sprite_bounds.height) * CurrentImage.getHeight())
	);
}

sf::Vector2i PixelToWindowCoords(const sf::Vector2i& pixel)
{
	auto sprite_bounds = CurrentImageSprite.getGlobalBounds();

	return sf::Vector2i(
		sprite_bounds.left + (static_cast<float>(pixel.x) / CurrentImage.getWidth ()) * sprite_bounds.width,
		sprite_bounds.top  + (static_cast<float>(pixel.y) / CurrentImage.getHeight()) * sprite_bounds.height
	);
}

//===========================================