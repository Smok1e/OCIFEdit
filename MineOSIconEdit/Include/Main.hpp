#pragma once

//===========================================

// Tell Windows.h to not define MIN and MAX macros to be compatiable with SFML
#define NOMINMAX

#include <deque>
#include <vector>
#include <stack>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <Windows.h>

#include <SFML/Graphics.hpp>

#include <imgui.h>
#include <imgui-SFML.h>

#include <MaterialDesignIcons.hpp>

#include <OCIF/HexFont.hpp>
#include <OCIF/Image.hpp>

#include <Tools/BrushTool.hpp>
#include <Tools/BrailleTool.hpp>
#include <Tools/EraserTool.hpp>
#include <Tools/TextTool.hpp>

//===========================================

constexpr size_t BUFFSIZE = 1024;

// Constants
extern const std::filesystem::path RESOURCES_BASE_DIR;
extern const std::filesystem::path GUI_FONT_PATH;
extern const std::filesystem::path GUI_ICONS_FONT_PATH;
extern const float                 GUI_FONT_SIZE;
extern const std::filesystem::path OC_FONT_PATH;
extern const std::filesystem::path RECENT_FILES_LIST_PATH;
extern const size_t                RECENT_FILES_LIST_LIMIT;
extern const sf::Color             BACKGROUND_COLOR;
extern const sf::Vector2u          WINDOW_INITIAL_SIZE;
extern const char*                 WINDOW_TITLE;
extern const std::filesystem::path WINDOW_ICON_PATH;

// Context variables
extern OCIF::HexFont                     OpencomputersFont;
extern std::deque<std::filesystem::path> RecentFilesList;
extern sf::RenderWindow                  RenderWindow;
extern sf::Cursor                        DefaultCursor;
extern sf::Cursor                        MovingCursor;
extern sf::Cursor                        LoadingCursor;
extern std::stack<sf::Cursor*>           MouseCursorStack;
extern sf::Clock                         DeltaClock;

extern bool                              ImageLoaded;
extern bool                              ImageLoadedFromFile;
extern OCIF::Image                       CurrentImage;
extern sf::Image                         CurrentRasterizedImage;
extern sf::Texture                       CurrentRasterizedTexture;
extern sf::Sprite                        CurrentImageSprite;
extern std::filesystem::path             CurrentImagePath;
extern float                             CurrentImageScale;

extern bool                              ShowImageBorder;
extern bool                              ShowImGuiDemoWindow;

extern bool                              Dragging;
extern sf::Vector2i                      DragStartMousePosition;
extern sf::Vector2i                      DragStartSpritePosition;

// Tools
extern bool								 ShowToolsWindow;
extern std::vector<Tool*>				 Tools;
extern Tool*							 CurrentTool;
extern sf::Vector2i                      CurrentMouseCoords;
extern sf::Vector2i                      CurrentImageCoords; 
extern sf::Vector2i						 CurrentPixelCoords;
extern sf::Vector2i                      CurrentBrailleCoords;

// Popups state
extern bool                              NewFilePopupOpened;
extern bool                              MessageBoxPopupOpened;
extern std::string                       MessageBoxPopupTitle;
extern std::string                       MessageBoxPopupMessage;
extern bool                              ExportPopupOpened;
extern std::filesystem::path             ExportPath;
extern bool                              ResizeImagePopupOpened;

//===========================================

bool Initialize();
void StartLoop();
void Cleanup();

void Update();
void NewFile(int width, int height, OCIF::Color color, bool create_transparent);
void LoadFile(const std::filesystem::path& path);
void SaveFile(const std::filesystem::path& path);
void ExportFile(const std::filesystem::path& path, float scale);
void ResizeImage(size_t new_width, size_t new_height, OCIF::Color fill_color, bool fill_transparent);
void MaximizeWindow();
void ShowMessageBox(std::string_view title, std::string_view message);
void PushMouseCursor(sf::Cursor& cursor);
void PopMouseCursor();

bool IsMouseInsideImage();

void UpdateTexture();
void ResetImage();
void SetImageScale(float scale);

void OnEvent(const sf::Event& event);
void OnMouseButtonPressed(sf::Mouse::Button button);
void OnMouseButtonReleased(sf::Mouse::Button button);
void OnKeyPressed(sf::Keyboard::Key key);
void OnKeyboardShortcut(sf::Keyboard::Key key);
void OnZoom(int direction);
void OnDragStart();
void OnDragStop();
void OnExit();
void OnFileNew();
void OnFileOpen();
void OnFileSave();
void OnFileSaveAs();
void OnFileExport();
void OnImageResize();

void RenderWorkspace();

void CenterNextWindow();
void ProcessGUI();
void ProcessGUIMainMenuBar();
void ProcessGUIFileMenu();
void ProcessGUIFileOpenRecentMenu();
void ProcessGUIViewMenu();
void ProcessGUIImageMenu();
void ProcessGUIDebugMenu();

void ProcessGUIPopups();
void ProcessGUIFileNewPopup();
void ProcessGUIMessageBoxPopup();
void ProcessGUIFileExportPopup();
void ProcessGUIImageResizePopup();

void ProcessGUIToolsWindow();

void AddToRecentFilesList(const std::filesystem::path& path);
void RemoveFromRecentFilesList(const std::filesystem::path& path);
void ClearRecentFilesList();
bool LoadRecentFilesList();
bool SaveRecentFilesList();

sf::Vector2i WindowToPixelCoords  (const sf::Vector2i& window );
sf::Vector2i PixelToWindowCoords  (const sf::Vector2i& pixel  );
sf::Vector2i WindowToImageCoords  (const sf::Vector2i& window );
sf::Vector2i ImageToWindowCoords  (const sf::Vector2i& image  );
sf::Vector2i WindowToBrailleCoords(const sf::Vector2i& window );
sf::Vector2i BrailleToWindowCoords(const sf::Vector2i& braille);

//===========================================