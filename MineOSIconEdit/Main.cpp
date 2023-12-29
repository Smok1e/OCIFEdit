#include "Main.hpp"

//===========================================

int main()
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

	m_window.create(sf::VideoMode(1000, 1000), "Icon edit");
	ImGui::SFML::Init(m_window);

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
		processGUIMainMenuBar();

		m_window.display();
	}
}

void Main::loadFile(const std::filesystem::path& path)
{
	try
	{
		m_ocif_image.loadFromFile(path);
		m_ocif_image.rasterize(m_image, m_font);
		m_texture.loadFromImage(m_image);
		m_sprite.setTexture(m_texture, true /* CYKA */);

		setScale(1.f);

		auto window_size = m_window.getSize();
		auto bounds = m_sprite.getGlobalBounds();
		m_sprite.setPosition(window_size.x / 2 - bounds.width / 2, window_size.y / 2 - bounds.height / 2);
	}

	catch (std::exception exc)
	{
		printf("%s\n", exc.what());
	}
}

//===========================================

void Main::processEvent(sf::Event event)
{
	ImGui::SFML::ProcessEvent(event);

	switch (event.type)
	{
		case sf::Event::Closed:
			onExit();
			break;

		case sf::Event::KeyPressed:
			if (!ImGui::GetIO().WantCaptureKeyboard)
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
					onKeyboardShortcut(event.key.code);
			}

			break;

		case sf::Event::MouseButtonPressed:
			if (!ImGui::GetIO().WantCaptureMouse)
			{
				switch (event.mouseButton.button)
				{
					case sf::Mouse::Left:
						onDragStart();
						break;
				}
			}

			break;

		case sf::Event::MouseButtonReleased:
			if (!ImGui::GetIO().WantCaptureMouse)
			{
				switch (event.mouseButton.button)
				{
					case sf::Mouse::Left:
						onDragStop();
						break;
				}
			}

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

void Main::onDragStart()
{
	m_dragging = true;
	m_drag_mouse_position = sf::Mouse::getPosition(m_window);
	m_drag_sprite_position = sf::Vector2i(m_sprite.getPosition());
}

void Main::onDragStop()
{
	m_dragging = false;
}

void Main::onZoom(int direction)
{
	setScale(m_scale + .1f * direction);
}

//===========================================

void Main::onRenderWorkspace()
{
	m_window.draw(m_sprite);

	if (m_show_border)
		onRenderBorder();

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt))
		onRenderPixelInfo();
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

void Main::onRenderPixelInfo()
{
	sf::Vector2f mouse_coords(sf::Mouse::getPosition(m_window));
	auto image_coords = windowToImageCoords(mouse_coords);

	if (
		   0 <= image_coords.x && image_coords.x < m_ocif_image.getWidth () 
		&& 0 <= image_coords.y && image_coords.y < m_ocif_image.getHeight()
	)
	{
		static sf::RectangleShape rect;
		rect.setOutlineColor(sf::Color::White);
		rect.setOutlineThickness(1);
		rect.setFillColor(sf::Color(255, 255, 255, 100));
		rect.setPosition(imageToWindowCoords(image_coords));

		const auto& pixel = m_ocif_image.get(image_coords.x, image_coords.y);
		const auto& glyph = m_font[pixel.character];

		auto scale = m_sprite.getScale();
		rect.setSize(sf::Vector2f(glyph.getWidth() * scale.x, glyph.getHeight() * scale.y));

		m_window.draw(rect);

		ImGui::SetNextWindowPos(ImVec2(mouse_coords.x + 10, mouse_coords.y + 10));
		if (ImGui::Begin("Pixel info", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
		{
			ImGui::Text("Position: %d, %d", image_coords.x, image_coords.y);

			// Display only ASCII characters for now, too lazy to make it work with unicode
			if (pixel.character <= 128)
				ImGui::Text("Character: U+%04X (%c)", pixel.character, pixel.character);

			else
				ImGui::Text("Character: U+%04X", pixel.character);

			ImGui::Text("Background: 0x%06X", pixel.background);
			ImGui::Text("Foreground: 0x%06X", pixel.foreground);
			ImGui::Text("Alpha: %lf", pixel.alpha);

			ImGui::End();
		}
	}
}

void Main::onUpdate()
{
	if (m_dragging)
	{
		auto delta = sf::Mouse::getPosition(m_window) - m_drag_mouse_position;
		m_sprite.setPosition(sf::Vector2f(m_drag_sprite_position + delta));
	}
}

//===========================================

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

	ImGui::SFML::Render(m_window);
}

void Main::processGUIFileMenu()
{
	if (ImGui::MenuItem("Exit", "^W"))
		onExit();

	if (ImGui::MenuItem("Open", "^O"))
		onFileOpen();
}

void Main::processGUIViewMenu()
{
	ImGui::MenuItem("Image border", nullptr, &m_show_border);
}

void Main::onExit()
{
	m_window.close();
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
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

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

sf::Vector2i Main::windowToImageCoords(const sf::Vector2f& coords)
{
	auto sprite_bounds = m_sprite.getGlobalBounds();
	return sf::Vector2i(
		std::floor(((coords.x - sprite_bounds.left) / sprite_bounds.width ) * m_ocif_image.getWidth ()),
		std::floor(((coords.y - sprite_bounds.top ) / sprite_bounds.height) * m_ocif_image.getHeight())
	);
}

sf::Vector2f Main::imageToWindowCoords(const sf::Vector2i& coords)
{
	auto sprite_bounds = m_sprite.getGlobalBounds();
	return sf::Vector2f(
		sprite_bounds.left + (static_cast<float>(coords.x) / m_ocif_image.getWidth ()) * sprite_bounds.width,
		sprite_bounds.top  + (static_cast<float>(coords.y) / m_ocif_image.getHeight()) * sprite_bounds.height
	);
}

//===========================================