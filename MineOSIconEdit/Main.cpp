#include "Main.hpp"
#include "Braille.hpp"
#include "IconsFontAwesome6.hpp"

#include <Windows.h>

//===========================================

int main(int argc, char* argv[])
{
	Main main;
	main.start();
}

//===========================================

Main::Main()
{
	init();
}

//===========================================

void Main::init()
{
	try
	{
		m_font.loadFromFile("font.hex");
	}

	catch (std::exception exc)
	{
		std::cerr << "Unable to load font: " << exc.what() << std::endl;
		return;
	}

	loadRecentFilesList();

	m_default_cursor.loadFromSystem(sf::Cursor::Arrow);
	m_moving_cursor.loadFromSystem(sf::Cursor::SizeAll);

	m_window.create(sf::VideoMode(1000, 1000), "Icon edit");
	ImGui::SFML::Init(m_window);

	auto& io = ImGui::GetIO();
	io.Fonts->Clear();
	io.Fonts->AddFontDefault();
	float base_font_size = 13.f;
	float icon_font_size = base_font_size * 2.f / 3.f;

	// Merge in icons from fontawesome
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig icons_config = {};
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = icon_font_size;
	io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, icon_font_size, &icons_config, icons_ranges);
	io.Fonts->Build();
	ImGui::SFML::UpdateFontTexture();

	m_initialized = true;
}

void Main::start()
{
	if (!m_initialized)
		return;

	while (m_window.isOpen())
	{
		sf::Event event;
		while (m_window.pollEvent(event))
			processEvent(event);

		ImGui::SFML::Update(m_window, m_clock.restart());

		m_window.clear(sf::Color(16, 16, 16));
		onUpdate();
		onRenderWorkspace();
		processGUI();

		m_window.display();
	}
}

void Main::loadFile(const std::filesystem::path& path)
{
	try
	{
		m_ocif_image.loadFromFile(path);
		rasterizeImage();
		centerImage();
		appendRecentFilesList(path);
	}

	catch (std::exception exc)
	{
		printf("%s\n", exc.what());
	}
}

void Main::newFile(unsigned width, unsigned height)
{
	m_ocif_image.resize(width, height);
	m_ocif_image.clear({' ', 0xFFFFFF, 0x000000, 0.0});
	rasterizeImage();
	centerImage();
}

void Main::rasterizeImage()
{
	m_ocif_image.rasterize(m_image, m_font);
	updateTexture();
}

void Main::updateTexture()
{
	m_texture.loadFromImage(m_image);
	m_sprite.setTexture(m_texture, true /* CYKA */);
}

void Main::centerImage()
{
	setScale(1.f);

	auto window_size = m_window.getSize();
	auto bounds = m_sprite.getGlobalBounds();
	m_sprite.setPosition(window_size.x / 2 - bounds.width / 2, window_size.y / 2 - bounds.height / 2);
}

//===========================================

void Main::appendRecentFilesList(const std::filesystem::path& path)
{
	auto iter = std::find(m_recent_files.begin(), m_recent_files.end(), path);
	if (iter == m_recent_files.end())
	{
		m_recent_files.push_front(path);
		if (m_recent_files.size() > RecentFilesListLimit)
			m_recent_files.pop_back();
	}

	else
	{
		// Move found path to top of the list
		std::rotate(m_recent_files.begin(), iter, iter+1);
	}

	saveRecentFilesList();
}

void Main::loadRecentFilesList()
{
	std::ifstream stream(RecentFilesListLocation);
	if (stream.good())
	{
		m_recent_files.clear();

		std::string line;
		while (std::getline(stream, line))
			m_recent_files.push_back(line);
	}
}

void Main::saveRecentFilesList()
{
	std::ofstream stream(RecentFilesListLocation);
	if (!stream)
	{
		std::cerr << "Failed to save recent files list" << std::endl;
		return;
	}

	for (const auto& path: m_recent_files)
		stream << path.string() << std::endl;
}

//===========================================

void Main::processEvent(sf::Event& event)
{
	ImGui::SFML::ProcessEvent(event);

	switch (event.type)
	{
		case sf::Event::Closed:
			onExit();
			break;

		case sf::Event::KeyPressed:
			if (!ImGui::GetIO().WantCaptureKeyboard)
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
					onKeyboardShortcut(event.key.code);

			break;

		case sf::Event::MouseButtonPressed:
			if (!ImGui::GetIO().WantCaptureMouse)
				onMouseButtonPressed(event);

			break;

		case sf::Event::MouseButtonReleased:
			if (!ImGui::GetIO().WantCaptureMouse)
				onMouseButtonReleased(event);

			break;

		case sf::Event::MouseWheelMoved:
			onZoom(event.mouseWheel.delta);
			break;

		case sf::Event::Resized:
			m_window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
			break;

		default:
			break;
	}
}

void Main::onKeyboardShortcut(sf::Keyboard::Key key)
{
	switch (key)
	{
		case sf::Keyboard::W:
			onExit();
			break;

		case sf::Keyboard::O:
			onFileOpen();
			break;
	}
}

void Main::onMouseButtonPressed(sf::Event& event)
{
	switch (event.mouseButton.button)
	{
		case sf::Mouse::Left:
			if (isMouseInsideImage())
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt))
					onPickColor();

				else
					onDrawStart();
			}

			break;

		case sf::Mouse::Middle:
			onDragStart();
	}	
}

void Main::onMouseButtonReleased(sf::Event& event)
{
	switch (event.mouseButton.button)
	{
		case sf::Mouse::Middle:
			onDragStop();
			break;

		case sf::Mouse::Left:
			onDrawStop();
			break;
	}
}

void Main::onDragStart()
{
	m_dragging = true;
	m_drag_mouse_position = sf::Mouse::getPosition(m_window);
	m_drag_sprite_position = sf::Vector2i(m_sprite.getPosition());

	m_window.setMouseCursor(m_moving_cursor);
}

void Main::onDragStop()
{
	m_dragging = false;

	m_window.setMouseCursor(m_default_cursor);
}

void Main::onZoom(int direction)
{
	setScale(m_scale + .1f * direction);
}

void Main::onDrawStart()
{
	m_drawing = true;
	onDraw();
}

void Main::onDrawStop()
{
	m_drawing = false;
}

void Main::onDraw()
{
	auto& current_pixel = getCurrentPixel();

	bool dot = OCIF::BrailleCheckDot(
		current_pixel.character,
		m_current_braille_coords.x % 2,
		m_current_braille_coords.y % 4
	);

	(dot? current_pixel.foreground: current_pixel.background) = OCIF::To24BitColor(OCIF::To8BitColor(m_current_color));

	m_ocif_image.rasterizePixel(m_image, m_font, m_current_pixel_coords.x, m_current_pixel_coords.y);
	updateTexture();
}

void Main::onPickColor()
{
	m_current_color = m_image.getPixel(m_current_image_coords.x, m_current_image_coords.y);
}

//===========================================

void Main::onRenderWorkspace()
{
	m_window.draw(m_sprite);

	if (m_show_border)
		onRenderBorder();

	if (isMouseInsideImage())
	{
		const auto& pixel = getCurrentPixel();
		const auto& glyph = m_font[pixel.character];
		auto scale = m_sprite.getScale();

		static sf::RectangleShape rect;
		rect.setOutlineThickness(1);

		if (m_show_pixel_frame)
		{
			rect.setOutlineColor(sf::Color::White);
			rect.setFillColor(sf::Color(255, 255, 255, 100));
			rect.setPosition(pixelToWindowCoords(m_current_pixel_coords));
			rect.setSize(sf::Vector2f(glyph.getWidth() * scale.x, glyph.getHeight() * scale.y));

			m_window.draw(rect);
		}

		if (m_show_braille_frame && !sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt))
		{
			auto color = m_current_color;
			color.a = 100;

			rect.setOutlineColor(m_current_color);
			rect.setFillColor(color);
			rect.setPosition(brailleToWindowCoords(m_current_braille_coords));
			rect.setSize(sf::Vector2f(4 * scale.x, 4 * scale.y));

			m_window.draw(rect);
		}
	}
}

void Main::onRenderBorder()
{
	static sf::RectangleShape rect;
	rect.setOutlineColor(sf::Color::White);
	rect.setOutlineThickness(1);
	rect.setFillColor(sf::Color::Transparent);

	auto bounds = m_sprite.getGlobalBounds();
	rect.setPosition(bounds.left, bounds.top);
	rect.setSize(sf::Vector2f(bounds.width, bounds.height));

	m_window.draw(rect);
}

void Main::onUpdate()
{
	auto last_braille_coords = m_current_braille_coords;

	sf::Vector2f mouse(sf::Mouse::getPosition(m_window));
	m_current_pixel_coords   = windowToPixelCoords  (mouse);
	m_current_braille_coords = windowToBrailleCoords(mouse);
	m_current_image_coords   = windowToImageCoords  (mouse);

	if (m_drawing && isMouseInsideImage() && last_braille_coords != m_current_braille_coords)
		onDraw();

	if (m_dragging)
		m_sprite.setPosition(sf::Vector2f(m_drag_sprite_position - m_drag_mouse_position) + mouse);
}

//===========================================

void Main::processGUI()
{
	processGUIMainMenuBar();

	if (m_show_tools_window)
		processGUIToolsWindow();

	ImGui::SFML::Render(m_window);
}

void Main::processGUIMainMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			processGUIFileMenu();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			processGUIViewMenu();
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void Main::processGUIFileMenu()
{
	if (ImGui::MenuItem(ICON_FA_FILE " New", "^N"))
		onFileNew();

	if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open", "^O"))
		onFileOpen();

	if (ImGui::BeginMenu(ICON_FA_CUBE " Open recent", !m_recent_files.empty()))
	{
		for (const auto& path: m_recent_files)
			if (ImGui::MenuItem(path.filename().string().c_str()))
				loadFile(path);

		ImGui::EndMenu();
	}

	if (ImGui::MenuItem(ICON_FA_XMARK " Exit", "^W"))
		onExit();
}

void Main::processGUIViewMenu()
{
	ImGui::MenuItem("Tools window",   nullptr, &m_show_tools_window );
	ImGui::MenuItem("Image border",   nullptr, &m_show_border       );
	ImGui::MenuItem("Pixel border",   nullptr, &m_show_pixel_frame  );
	ImGui::MenuItem("Braille border", nullptr, &m_show_braille_frame);
}

void Main::processGUIToolsWindow()
{
	static float color[3] = {};

	if (ImGui::Begin("Tools", &m_show_tools_window))
	{
		if (ImGui::ColorEdit3("Color", OCIF::ToFloat3Color(m_current_color, color)))
			m_current_color = OCIF::ToSFColor(color);
	}

	ImGui::End();
}

void Main::onExit()
{
	m_window.close();
}

void Main::onFileNew()
{
	newFile(8, 4);
}

void Main::onFileOpen()
{
	wchar_t filename[BUFFSIZE] = L"";

	OPENFILENAMEW ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_window.getSystemHandle();
	ofn.lpstrFile = filename;
	ofn.nMaxFile = BUFFSIZE;
	ofn.lpstrFilter = L"All\0*.*\0OCIF (.pic)\0*.pic\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameW(&ofn))
	{
		char8_t filename_utf8[BUFFSIZE] = u8"";

		WideCharToMultiByte(
			CP_UTF8, 
			0, 
			filename, 
			BUFFSIZE, 
			reinterpret_cast<char*>(filename_utf8),
			BUFFSIZE,
			nullptr,
			nullptr
		);

		loadFile(filename_utf8);
	}
}

//===========================================

void Main::setScale(float scale)
{
	m_scale = std::clamp(scale, .5f, 10.f);
	auto scale_factor = m_scale*m_scale;
	m_sprite.setScale(scale_factor, scale_factor);
}

sf::Vector2i Main::windowToPixelCoords(const sf::Vector2f& coords)
{
	auto sprite_bounds = m_sprite.getGlobalBounds();
	return sf::Vector2i(
		std::floor(((coords.x - sprite_bounds.left) / sprite_bounds.width ) * m_ocif_image.getWidth ()),
		std::floor(((coords.y - sprite_bounds.top ) / sprite_bounds.height) * m_ocif_image.getHeight())
	);
}

sf::Vector2f Main::pixelToWindowCoords(const sf::Vector2i& coords)
{
	auto sprite_bounds = m_sprite.getGlobalBounds();
	return sf::Vector2f(
		sprite_bounds.left + (static_cast<float>(coords.x) / m_ocif_image.getWidth ()) * sprite_bounds.width,
		sprite_bounds.top  + (static_cast<float>(coords.y) / m_ocif_image.getHeight()) * sprite_bounds.height
	);
}

sf::Vector2i Main::windowToBrailleCoords(const sf::Vector2f& coords)
{
	auto sprite_bounds = m_sprite.getGlobalBounds();
	return sf::Vector2i(
		std::floor(((coords.x - sprite_bounds.left) / sprite_bounds.width ) * (m_ocif_image.getWidth ()) * 2),
		std::floor(((coords.y - sprite_bounds.top ) / sprite_bounds.height) * (m_ocif_image.getHeight()) * 4)
	);
}

sf::Vector2f Main::brailleToWindowCoords(const sf::Vector2i& coords)
{
	auto sprite_bounds = m_sprite.getGlobalBounds();
	return sf::Vector2f(
		sprite_bounds.left + (static_cast<float>(coords.x) / (m_ocif_image.getWidth () * 2)) * sprite_bounds.width,
		sprite_bounds.top  + (static_cast<float>(coords.y) / (m_ocif_image.getHeight() * 4)) * sprite_bounds.height
	);
}

sf::Vector2i Main::windowToImageCoords(const sf::Vector2f& coords)
{
	auto sprite_bounds = m_sprite.getGlobalBounds();
	auto image_size = m_image.getSize();

	return sf::Vector2i(
		((coords.x - sprite_bounds.left) / sprite_bounds.width ) * image_size.x,
		((coords.y - sprite_bounds.top ) / sprite_bounds.height) * image_size.y
	);
}

sf::Vector2f Main::imageToWindowCoords(const sf::Vector2i& coords)
{
	auto sprite_bounds = m_sprite.getGlobalBounds();
	auto image_size = m_image.getSize();

	return sf::Vector2f(
		sprite_bounds.left + (static_cast<float>(coords.x) / image_size.x) * sprite_bounds.width,
		sprite_bounds.top  + (static_cast<float>(coords.y) / image_size.y) * sprite_bounds.height
	);
}

bool Main::isMouseInsideImage()
{
	return    m_current_pixel_coords.x >= 0 && m_current_pixel_coords.x < m_ocif_image.getWidth ()
		   && m_current_pixel_coords.y >= 0 && m_current_pixel_coords.y < m_ocif_image.getHeight();
}

OCIF::Pixel& Main::getCurrentPixel()
{
	return m_ocif_image.get(m_current_pixel_coords.x, m_current_pixel_coords.y);
}

//===========================================