#include <Main.hpp>
#include <Version>

#pragma warning(disable: 4996 )
#pragma warning(disable: 28159)

//===========================================

const std::filesystem::path RESOURCES_BASE_DIR      = "Resources";                       // Resources directory path
const std::filesystem::path GUI_FONT_PATH           = "Fonts/CascadiaCode.ttf";          // Unicode font used to display text in GUI
const std::filesystem::path GUI_ICONS_FONT_PATH     = "Fonts/MaterialIcons-Regular.ttf"; // Font used to display icons in GUI
const float                 GUI_FONT_SIZE           = 18.f;                              // GUI text size
const std::filesystem::path OC_FONT_PATH            = "Fonts/font.hex";                  // Opencomputers font in their own hex format
const std::filesystem::path RECENT_FILES_LIST_PATH  = "recent.txt";                      // Cached list of recently open files
const size_t                RECENT_FILES_LIST_LIMIT = 10;                                // Limit of recently open files
const sf::Color             BACKGROUND_COLOR        = sf::Color(24, 24, 24);             // Window background color
const sf::Vector2u          WINDOW_INITIAL_SIZE     = sf::Vector2u(1000, 1000);          // Render window size when it is initially opened
const char*                 WINDOW_TITLE            = "OCIF edit";                       // Render window title
const std::filesystem::path WINDOW_ICON_PATH        = "Icon.png";                        // Render window icon

OCIF::HexFont                     OpencomputersFont;
std::deque<std::filesystem::path> RecentFilesList;
sf::RenderWindow                  RenderWindow;
BOOL                              UseDarkWindowMode = true;
sf::Cursor                        DefaultCursor;
sf::Cursor                        MovingCursor;
sf::Cursor                        LoadingCursor;
std::stack<sf::Cursor*>           MouseCursorStack;
sf::Clock                         DeltaClock;
sf::Shader                        BackgroundGridShader;
DebugLog                          Log;

bool                              ImageLoaded = false;
bool                              ImageLoadedFromFile = false;
OCIF::Image                       CurrentImage;
sf::Image                         CurrentRasterizedImage;
sf::Texture                       CurrentRasterizedTexture;
sf::Sprite                        CurrentImageSprite;
std::filesystem::path             CurrentImagePath;
float                             CurrentImageScale = 1.f;

bool                              ShowImageBorder     = false;
bool                              ShowBackgroundGrid  = true;
bool                              ShowImGuiDemoWindow = false;
bool                              ShowDebugLog        = false;

bool                              Dragging = false;
sf::Vector2i                      DragStartMousePosition;
sf::Vector2i                      DragStartSpritePosition;

bool                              ShowToolsWindow = true;
std::vector<Tool*>                Tools;
Tool*                             CurrentTool = nullptr;
bool                              Drawing = false;
sf::Mouse::Button                 DrawingButton;
sf::Vector2i                      CurrentMouseCoords;
sf::Vector2i                      CurrentImageCoords;
sf::Vector2i                      CurrentPixelCoords;
sf::Vector2i                      CurrentBrailleCoords;

bool                              NewFilePopupOpened     = false;
bool                              MessageBoxPopupOpened  = false;
std::string                       MessageBoxPopupTitle   = "";
std::string                       MessageBoxPopupMessage = "";
bool                              ExportPopupOpened      = false;
std::filesystem::path             ExportPath             = "";
extern bool                       ResizeImagePopupOpened = false;

//===========================================

int main()
{
	if (Initialize())
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
	DefaultCursor.loadFromSystem(sf::Cursor::Arrow  );
	MovingCursor.loadFromSystem (sf::Cursor::SizeAll);
	LoadingCursor.loadFromSystem(sf::Cursor::Wait   );

	// Shaders
	if (!LoadShaders())
		return false;

	// Initializing window
	RenderWindow.create(sf::VideoMode(WINDOW_INITIAL_SIZE.x, WINDOW_INITIAL_SIZE.y), WINDOW_TITLE);
	RenderWindow.setVerticalSyncEnabled(true);

	sf::Image icon;
	if (icon.loadFromFile((RESOURCES_BASE_DIR/WINDOW_ICON_PATH).string()))
		RenderWindow.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

	PushMouseCursor(DefaultCursor);
	// MaximizeWindow();

	TryEnableDarkTitlebar();

	// ImGui
	ImGui::SFML::Init(RenderWindow);

	// Loading fonts
	auto& io = ImGui::GetIO();
	io.Fonts->Clear();

	// Loading font for displaying UTF-8 encoded text
	static const ImWchar utf8_ranges[] = { 0x20, 0xFFFF, 0 };
	io.Fonts->AddFontFromFileTTF((RESOURCES_BASE_DIR/GUI_FONT_PATH).string().c_str(), GUI_FONT_SIZE, nullptr, utf8_ranges);

	// Loading MaterialDesign and merging icons glyphs to previous font
	static const ImWchar icons_ranges[] = { MD_RANGE_MIN, MD_RANGE_MAX, 0 };
	ImFontConfig icons_config = {};
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphOffset.y = 5;
	icons_config.GlyphMinAdvanceX = GUI_FONT_SIZE;
	io.Fonts->AddFontFromFileTTF((RESOURCES_BASE_DIR/GUI_ICONS_FONT_PATH).string().c_str(), GUI_FONT_SIZE * 1.5f, &icons_config, icons_ranges);
	io.Fonts->Build();

	ImGui::SFML::UpdateFontTexture();

	// Rounded corners
	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding  = 5.f;
	style.WindowRounding = 5.f;
	style.PopupRounding  = 5.f;

	// Initializing tools
	Tools.push_back(new BrushTool);
	Tools.push_back(new BrailleTool);
	Tools.push_back(new EraserTool);
	Tools.push_back(new TextTool);
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

bool LoadShaders()
{
	if (!BackgroundGridShader.loadFromFile((RESOURCES_BASE_DIR/"Shaders/BackgroundGrid.frag").string(), sf::Shader::Fragment))
	{
		Log.warn("Background grid shader loading failed");
		return false;
	}

	return true;
}

bool TryEnableDarkTitlebar()
{
	// The DWMWA_USE_IMMERSIVE_DARK_MODE option can only be used
	// starting from Windows 11 Build 22000, so we should check
	// OS version before trying to set this attribute
	Log.info("Checking the OS version...");

	OSVERSIONINFOA version = {};
	version.dwOSVersionInfoSize = sizeof(version);

	GetVersionExA(&version);
	Log.info("Windows build number: {}", version.dwBuildNumber);

	if (version.dwBuildNumber < 22000)
		return false;

	Log.info("Trying to enable immersive dark mode...");

	// In Windows 11 dwmapi imports function called 'NtDCompositionSetBlurredWallpaperSurface'
	// from win32u.dll, but in windows build < 22000 win32u.dll does not have such function.
	// This is why i am loading dwmapi.dll dynamically in case that os build number >= 22000.
	// Otherwise this will cause an error when running on Windows 10.
	HMODULE dwmapi = LoadLibraryA("dwmapi.dll");
	if (!dwmapi)
	{
		Log.error("Unable to dynamically load dwmapi.dll, error code 0x{:08X}", GetLastError());
		return false;
	}

	auto __DwmSetWindowAttribute = reinterpret_cast<HRESULT (__stdcall*)(HWND, DWORD, LPCVOID, DWORD)>(
		GetProcAddress(dwmapi, "DwmSetWindowAttribute")
	);

	if (__DwmSetWindowAttribute)
	{
		__DwmSetWindowAttribute
		(
	 		RenderWindow.getSystemHandle(),
	 		DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE, 
	 		&UseDarkWindowMode, 
	 		sizeof(UseDarkWindowMode)
		);

		Log.info("Immersive dark mode enabled");
	}
		
	else Log.error("Dwmapi.dll was successfully loaded, but procedure 'DwmSetWindowAttribute' not found in the DLL");

	FreeLibrary(dwmapi);
	return __DwmSetWindowAttribute;
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

	auto last_braille_coords = CurrentPixelCoords;
	auto sprite_bounds = CurrentImageSprite.getGlobalBounds();
					     
	CurrentMouseCoords   = sf::Mouse::getPosition(RenderWindow);
	CurrentImageCoords   = WindowToImageCoords  (CurrentMouseCoords);
	CurrentPixelCoords   = WindowToPixelCoords  (CurrentMouseCoords);
	CurrentBrailleCoords = WindowToBrailleCoords(CurrentMouseCoords);

	CurrentTool->onUpdate();
}

void NewFile(int width, int height, OCIF::Color color, bool create_transparent)
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
		static_cast<double>(create_transparent)
	});

	CurrentImage.rasterize(CurrentRasterizedImage, OpencomputersFont);
	UpdateTexture();
	ResetImage();

	CurrentImagePath = "Untitled.pic";
	ImageLoaded = true;
	ImageLoadedFromFile = false;
}

void LoadFile(const std::filesystem::path& path)
{
	PushMouseCursor(LoadingCursor);

	try
	{
		CurrentImage.loadFromFile(path);

		CurrentImage.rasterize(CurrentRasterizedImage, OpencomputersFont);
		UpdateTexture();
		ResetImage();

		AddToRecentFilesList(path);

		ImageLoaded = true;
		ImageLoadedFromFile = true;
		CurrentImagePath = path;
	}

	catch (std::exception exc)
	{
		std::stringstream stream;
		stream << "Unable to load file '" << reinterpret_cast<const char*>(path.filename().u8string().c_str()) << "': " << exc.what() << std::endl;

		ShowMessageBox("Unable to load file", stream.str());
		RemoveFromRecentFilesList(path);
	}

	PopMouseCursor();
}

void SaveFile(const std::filesystem::path& path)
{
	PushMouseCursor(LoadingCursor);

	try
	{
		CurrentImage.saveToFile(path);
		CurrentImagePath = path;
		ImageLoadedFromFile = true;
		AddToRecentFilesList(path);

		Log.info("{} was successfully saved", path.filename().string());
	}

	catch (std::exception exc)
	{
		std::stringstream stream;
		stream << "Unable to save file '" << reinterpret_cast<const char*>(path.filename().u8string().c_str()) << "': " << exc.what() << std::endl;

		ShowMessageBox("Unable to save file", stream.str());
	}

	PopMouseCursor();
}

void ExportFile(const std::filesystem::path& path, float scale)
{
	PushMouseCursor(LoadingCursor);

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

	if (render_texture.getTexture().copyToImage().saveToFile(path.string()))
		Log.info("{} was successfully saved", path.filename().string());

	PopMouseCursor();
}

void ResizeImage(size_t new_width, size_t new_height, OCIF::Color fill_color, bool fill_transparent)
{
	CurrentImage.resizeAndKeepContent(new_width, new_height, {
		' ',
		fill_color,
		0,
		static_cast<double>(fill_transparent)
	});

	CurrentImage.rasterize(CurrentRasterizedImage, OpencomputersFont);
	UpdateTexture();
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

void PushMouseCursor(sf::Cursor& cursor)
{
	MouseCursorStack.push(&cursor);
	RenderWindow.setMouseCursor(cursor);
}

void PopMouseCursor()
{
	MouseCursorStack.pop();
	RenderWindow.setMouseCursor(*MouseCursorStack.top());
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

	if (CurrentTool->onEvent(event))
		return;

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
			if (!io.WantCaptureMouse)
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

	PushMouseCursor(MovingCursor);
}

void OnDragStop()
{
	Dragging = false;

	PopMouseCursor();
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
	SaveFile(CurrentImagePath);
}

void OnFileSaveAs()
{
	static wchar_t buffer[BUFFSIZE] = L"";
	CurrentImagePath.wstring().copy(buffer, BUFFSIZE);

	OPENFILENAMEW ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = RenderWindow.getSystemHandle();
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = BUFFSIZE;
	ofn.lpstrFileTitle = nullptr;
	ofn.lpstrDefExt = L"pic";
	ofn.lpstrFilter = L"All\0*.*\0" "OCIF (.pic)\0*.pic\0";
	ofn.nFilterIndex = 2;
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

void OnImageResize()
{
	ResizeImagePopupOpened = true;
}

void OnKeyPressed(sf::Keyboard::Key key)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
	{
		OnKeyboardShortcut(key);
		return;
	}

	for (const auto& tool: Tools)
	{
		if (tool->getHotkey() == key)
		{
			CurrentTool = tool;
			return;
		}
	}

	switch (key)
	{
		case sf::Keyboard::Escape:
			NewFilePopupOpened     = false;
			MessageBoxPopupOpened  = false;
			ExportPopupOpened      = false;
			ResizeImagePopupOpened = false;
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

		// File -> Save / Save as
		case sf::Keyboard::S:
			if (ImageLoaded)
			{
				if (!ImageLoadedFromFile || sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
					OnFileSaveAs();

				else
					OnFileSave();
			}

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
	static sf::RectangleShape rect;
	auto sprite_bounds = CurrentImageSprite.getGlobalBounds();
	rect.setPosition(sprite_bounds.left, sprite_bounds.top);
	rect.setSize(sf::Vector2f(sprite_bounds.width, sprite_bounds.height));

	if (ShowImageBorder)
	{
		rect.setFillColor(sf::Color::Transparent);
		rect.setOutlineColor(sf::Color::White);
		rect.setOutlineThickness(1.f);

		RenderWindow.draw(rect);
	}

	if (ShowBackgroundGrid)
	{
		rect.setOutlineThickness(0.f);
		BackgroundGridShader.setUniform("rect_position", rect.getPosition());
		RenderWindow.draw(rect, &BackgroundGridShader);
	}

	RenderWindow.draw(CurrentImageSprite);

	CurrentTool->onRenderWorkspace();
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

	if (ShowDebugLog)
		Log.draw(&ShowDebugLog);

	CurrentTool->processGUI();
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

		if (ImGui::BeginMenu("Image"))
		{
			ProcessGUIImageMenu();
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
	if (ImGui::MenuItem(MD_ICON_CREATE_NEW_FOLDER " New", "CTRL + N"))
		OnFileNew();
		
	if (ImGui::MenuItem(MD_ICON_FOLDER_OPEN " Open", "CTRL + O"))
		OnFileOpen();

	if (ImGui::BeginMenu(MD_ICON_SCHEDULE " Open recent", !RecentFilesList.empty()))
	{
		ProcessGUIFileOpenRecentMenu();
		ImGui::EndMenu();
	}

	// Show tooltip if open recent item is disabled
	if (RecentFilesList.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("No recent files");

	if (ImGui::MenuItem(MD_ICON_SAVE " Save", "CTRL + S", nullptr, ImageLoadedFromFile))
		OnFileSave();

	// Show tooltip if the save item is disabled
	if (!ImageLoaded && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("Nothing to save");

	if (ImGui::MenuItem(MD_ICON_SAVE_AS " Save as...", "CTRL + SHIFT + S", nullptr, ImageLoaded))
		OnFileSaveAs();

	// Show tooltip if the save as item is disabled
	if (!ImageLoaded && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("Nothing to save");

	if (ImGui::MenuItem(MD_ICON_DRIVE_FILE_MOVE " Export", "CTRL + E", nullptr, ImageLoaded))
		OnFileExport();

	// Show tooltip if the export item is disabled
	if (!ImageLoaded && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("Nothing to export");

	ImGui::Separator();
	if (ImGui::MenuItem(MD_ICON_CLOSE " Exit", "CTRL + W"))
		OnExit();
}

void ProcessGUIFileOpenRecentMenu()
{
	for (const auto& path: RecentFilesList)
	{
		if (ImGui::MenuItem(reinterpret_cast<const char*>(path.filename().u8string().c_str())))
		{
			LoadFile(path);
			break;
		}
	}

	ImGui::Separator();

	if (ImGui::MenuItem(MD_ICON_DELETE " Clear"))
		ClearRecentFilesList();
}

void ProcessGUIViewMenu()
{
	ImGui::MenuItem("Tools window",    nullptr, &ShowToolsWindow   );
	ImGui::MenuItem("Background grid", nullptr, &ShowBackgroundGrid);
	ImGui::MenuItem("Image border",    nullptr, &ShowImageBorder   );
	ImGui::MenuItem("Debug log",       nullptr, &ShowDebugLog      );
}

void ProcessGUIImageMenu()
{
	if (ImGui::MenuItem(MD_ICON_RESIZE " Resize", nullptr, nullptr, ImageLoaded))
		OnImageResize();

	// Show tooltip if the resize as item is disabled
	if (!ImageLoaded && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("Nothing to resize");
}

void ProcessGUIDebugMenu()
{
	if (ImGui::MenuItem("Show ImGui demo"))
		ShowImGuiDemoWindow = true;

	if (ImGui::MenuItem("Reload shaders"))
		LoadShaders();
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
		ProcessGUIFileExportPopup();
		ImGui::EndPopup();
	}

	if (ResizeImagePopupOpened)
		ImGui::OpenPopup("Resize image");

	if (ImGui::BeginPopupModal("Resize image", &ResizeImagePopupOpened, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ProcessGUIImageResizePopup();
		ImGui::EndPopup();
	}
}

void ProcessGUIFileNewPopup()
{
	static int size[2] = { 8, 4 };
	ImGui::InputInt2("Image size", size);

	static float color[3] = { 1.f, 1.f, 1.f };
	static bool create_transparent = true;

	if (!create_transparent)
		ImGui::ColorPicker3("Fill color", color);

	ImGui::Checkbox("Create transparent image", &create_transparent);

	if (ImGui::Button("Ok"))
	{
		NewFile(size[0], size[1], OCIF::NormalizeColor(OCIF::To24BitColor(color)), create_transparent);
		NewFilePopupOpened = false;
	}
}

void ProcessGUIMessageBoxPopup()
{
	ImGui::Text(MessageBoxPopupMessage.c_str());

	if (ImGui::Button("Ok"))
		MessageBoxPopupOpened = false;
}

void ProcessGUIFileExportPopup()
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

void ProcessGUIImageResizePopup()
{
	static int size[2] = { 8, 4 };
	ImGui::InputInt2("Image size", size);

	static float color[3] = { 1.f, 1.f, 1.f };
	static bool create_transparent = true;

	if (!create_transparent)
		ImGui::ColorPicker3("Fill color", color);

	ImGui::Checkbox("No fill", &create_transparent);

	if (ImGui::Button("Ok"))
	{
		ResizeImage(size[0], size[1], OCIF::NormalizeColor(OCIF::To24BitColor(color)), create_transparent);
		ResizeImagePopupOpened = false;
	}
}

//===========================================

void ProcessGUIToolsWindow()
{
	static char tool_name[64] = "";
	if (ImGui::Begin("Tools", &ShowToolsWindow))
	{
		for (Tool* tool: Tools)
		{
			if (tool->getHotkey() != sf::Keyboard::Unknown)
				sprintf_s(tool_name, "[%c] %s %s", 'A' + static_cast<char>(tool->getHotkey() - sf::Keyboard::A), tool->getIcon(), tool->getName());

			else
				sprintf_s(tool_name, "     %s %s", tool->getIcon(), tool->getName());

			if (ImGui::Selectable(tool_name, tool == CurrentTool))
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

void ClearRecentFilesList()
{
	RecentFilesList.clear();
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
	if (RecentFilesList.empty())
	{
		std::filesystem::remove(RECENT_FILES_LIST_PATH);
		return true;
	}

	std::ofstream stream(RECENT_FILES_LIST_PATH, std::ios::out);
	if (!stream)
	{
		Log.warn("Unable to save recent files list");
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
		sprite_bounds.left + std::ceil((static_cast<float>(pixel.x) / CurrentImage.getWidth ()) * sprite_bounds.width),
		sprite_bounds.top  + std::ceil((static_cast<float>(pixel.y) / CurrentImage.getHeight()) * sprite_bounds.height)
	);
}

sf::Vector2i WindowToImageCoords(const sf::Vector2i& mouse)
{
	auto sprite_bounds = CurrentImageSprite.getGlobalBounds();
	auto image_size = CurrentRasterizedImage.getSize();

	return sf::Vector2i(
		((mouse.x - sprite_bounds.left) / sprite_bounds.width ) * image_size.x,
		((mouse.y - sprite_bounds.top ) / sprite_bounds.height) * image_size.y
	);
}

sf::Vector2i ImageToWindowCoords(const sf::Vector2i& coords)
{
	auto sprite_bounds = CurrentImageSprite.getGlobalBounds();
	auto image_size = CurrentRasterizedImage.getSize();

	return sf::Vector2i(
		sprite_bounds.left + (static_cast<float>(coords.x) / image_size.x) * sprite_bounds.width,
		sprite_bounds.top  + (static_cast<float>(coords.y) / image_size.y) * sprite_bounds.height
	);
}

sf::Vector2i WindowToBrailleCoords(const sf::Vector2i& window)
{
	auto sprite_bounds = CurrentImageSprite.getGlobalBounds();

	return sf::Vector2i(
		((window.x - sprite_bounds.left) / sprite_bounds.width ) * CurrentImage.getWidth () * 2,
		((window.y - sprite_bounds.top ) / sprite_bounds.height) * CurrentImage.getHeight() * 4
	);
}

sf::Vector2i BrailleToWindowCoords(const sf::Vector2i& braille)
{
	auto sprite_bounds = CurrentImageSprite.getGlobalBounds();

	return sf::Vector2i(
		sprite_bounds.left + (static_cast<float>(braille.x) / (CurrentImage.getWidth () * 2)) * sprite_bounds.width,
		sprite_bounds.top  + (static_cast<float>(braille.y) / (CurrentImage.getHeight() * 4)) * sprite_bounds.height
	);
}

//===========================================
