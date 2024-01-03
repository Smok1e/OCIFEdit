#include <fstream>
#include <map>

#include "OCIF/IO.hpp"
#include "OCIF/Image.hpp"
#include "OCIF/Color.hpp"
#include "OCIF/Braille.hpp"

//===========================================

namespace OCIF
{

//===========================================

Pixel Pixel::Transparent = {
	' ',
	0,
	0,
	1.0
};

//===========================================

Image::Image(size_t width, size_t height)
{
	resize(width, height);
}

size_t Image::getWidth() const
{
	return m_width;
}

size_t Image::getHeight() const
{
	return m_height;
}

void Image::resize(size_t new_width, size_t new_height)
{
	m_data.clear();
	m_data.resize(new_width * new_height);
	m_width  = new_width;
	m_height = new_height;
}

void Image::resizeAndKeepContent(size_t new_width, size_t new_height, const Pixel& fill /*= Pixel::Transparent*/)
{
	auto old_data = m_data;
	m_data.resize(new_width * new_height);

	for (size_t x = 0; x < new_width; x++)
		for (size_t y = 0; y < new_height; y++)
			m_data[y*new_width + x] = (x < m_width && y < m_height)
				? old_data[y*m_width + x]
				: fill;

	m_width  = new_width;
	m_height = new_height;
}

void Image::clear(const Pixel& pixel)
{
	std::fill(m_data.begin(), m_data.end(), pixel);
}

void Image::set(size_t x, size_t y, const Pixel& pixel)
{
	get(x, y) = pixel;
}

void Image::set(size_t x, size_t y, Color background, Color foreground, double alpha, uint32_t character)
{
	auto& pixel = get(x, y);
	pixel.background = background;
	pixel.foreground = foreground;
	pixel.alpha = alpha;
	pixel.character = character;
}

Pixel& Image::get(size_t x, size_t y)
{
	return m_data[y * m_width + x];
}

const Pixel& Image::get(size_t x, size_t y) const
{
	return m_data[y * m_width + x];
}

//===========================================

void Image::loadFromStream(std::istream& stream)
{
	char signature[4] = "";
	stream.read(signature, sizeof(signature));
	if (strncmp(signature, "OCIF", sizeof(signature)))
		throw std::runtime_error("Invalid OCIF signature");

	auto encoding_method = ReadBytes(stream);
	if (!(6 <= encoding_method && encoding_method <= 8))
		throw std::runtime_error(std::string("Unsupported encoding method: ") + std::to_string(encoding_method));

	bool is7 = encoding_method >= 7;
	bool is8 = encoding_method >= 8;

	auto width  = ReadBytes(stream) + is8;
	auto height = ReadBytes(stream) + is8;
	resize(width, height);

	auto alpha_count = ReadBytes(stream) + is7;
	for (size_t alpha = 0; alpha < alpha_count; alpha++)
	{
		auto current_alpha = static_cast<double>(ReadBytes(stream)) / 0xFF;

		auto character_count = ReadBytes(stream, 2) + is7;
		for (size_t character = 0; character < character_count; character++)
		{
			auto current_character = ReadUnicodeCharacter(stream);

			auto background_count = ReadBytes(stream) + is7;
			for (size_t background = 0; background < background_count; background++)
			{
				auto current_background = To24BitColor(ReadBytes(stream));

				auto foreground_count = ReadBytes(stream) + is7;
				for (size_t foreground = 0; foreground < foreground_count; foreground++)
				{
					auto current_foreground = To24BitColor(ReadBytes(stream));

					auto y_count = ReadBytes(stream) + is7;
					for (size_t y = 0; y < y_count; y++)
					{
						auto current_y = ReadBytes(stream) - 1; // Lua's indexing starting from 1

						auto x_count = ReadBytes(stream) + is7;
						for (size_t x = 0; x < x_count; x++)
						{
							auto current_x = ReadBytes(stream) - 1;

							set(
								current_x + is8, 
								current_y + is8,
								current_background,
								current_foreground,
								current_alpha,
								current_character
							);
						}
					}
				}
			}
		}
	}
}

void Image::loadFromFile(const std::filesystem::path& path)
{
	std::ifstream stream;
	stream.exceptions(stream.exceptions() | std::ios::failbit);
	stream.open(path, std::ios::binary);

	loadFromStream(stream);
}

//===========================================

void Image::saveToStream(std::ostream& stream) const
{
	//       Alpha            Character          Background        Foreground        Y                     X
	std::map<double, std::map<uint32_t, std::map<uint8_t, std::map<uint8_t, std::map<unsigned, std::vector<unsigned>>>>>> grouped_image;

	for (size_t i = 0; i < m_data.size(); i++)
	{
		auto& pixel = m_data[i];

		if (!grouped_image.contains(pixel.alpha))
			grouped_image.insert({pixel.alpha, {}});

		if (!grouped_image[pixel.alpha].contains(pixel.character))
			grouped_image[pixel.alpha].insert({pixel.character, {}});

		uint8_t background = To8BitColor(pixel.background);
		if (!grouped_image[pixel.alpha][pixel.character].contains(background))
			grouped_image[pixel.alpha][pixel.character].insert({background, {}});

		uint8_t foreground = To8BitColor(pixel.foreground);
		if (!grouped_image[pixel.alpha][pixel.character][background].contains(foreground))
			grouped_image[pixel.alpha][pixel.character][background].insert({foreground, {}});

		unsigned y = i / m_width;
		if (!grouped_image[pixel.alpha][pixel.character][background][foreground].contains(y))
			grouped_image[pixel.alpha][pixel.character][background][foreground].insert({y, {}});

		unsigned x = i % m_width;
		grouped_image[pixel.alpha][pixel.character][background][foreground][y].push_back(x);
	}

	// Signature
	stream << "OCIF";

	// Encoding method
	WriteBytes<uint8_t>(stream, 8);

	// Width/height
	WriteBytes<uint8_t>(stream, m_width  - 1);
	WriteBytes<uint8_t>(stream, m_height - 1);

	// Alpha count
	WriteBytes<uint8_t>(stream, grouped_image.size() - 1);
	for (const auto& [alpha, characters]: grouped_image)
	{
		// Current alpha
		WriteBytes<uint8_t>(stream, std::floor(alpha * 0xFF));

		// 2 bytes for character count
		WriteBytes<uint16_t>(stream, characters.size() - 1);
		for (const auto& [character, backgrounds]: characters)
		{
			// Current character encoded in UTF-8
			WriteUnicodeCharacter(stream, character);

			// Background count
			WriteBytes<uint8_t>(stream, backgrounds.size() - 1);
			for (const auto& [background, foregrounds]: backgrounds)
			{
				// Current background
				WriteBytes<uint8_t>(stream, background);

				// Foreground count
				WriteBytes<uint8_t>(stream, foregrounds.size() - 1);
				for (const auto& [foreground, y_map]: foregrounds)
				{
					// Current foreground
					WriteBytes<uint8_t>(stream, foreground);

					// Y count
					WriteBytes<uint8_t>(stream, y_map.size() - 1);
					for (const auto& [y, x_array]: y_map)
					{
						// Current Y
						WriteBytes<uint8_t>(stream, y);

						// X count
						WriteBytes<uint8_t>(stream, x_array.size() - 1);
						for (const auto& x: x_array)
							// Current X
							WriteBytes<uint8_t>(stream, x);
					}
				}
			}
		}
	}
}

void Image::saveToFile(const std::filesystem::path& path) const
{
	std::ofstream stream;
	stream.exceptions(stream.exceptions() | std::ios::failbit);
	stream.open(path, std::ios::binary);

	saveToStream(stream);
}

//===========================================

void Image::rasterize(sf::Image& image, HexFont& font) const
{
	image.create(m_width * HexFont::Glyph::DefaultWidth, m_height * HexFont::Glyph::DefaultHeight);
	for (size_t x = 0; x < m_width; x++)
		for (size_t y = 0; y < m_height; y++)
			rasterizePixel(image, font, x, y);
}

void Image::rasterizePixel(sf::Image& image, HexFont& font, size_t x, size_t y) const
{
	const auto& pixel = get(x, y);
	font[pixel.character].rasterize(
		image,
		x * HexFont::Glyph::DefaultWidth,
		y * HexFont::Glyph::DefaultHeight,
		ToSFColor(pixel.background, pixel.alpha),
		ToSFColor(pixel.foreground)
	);	
}

//===========================================

} // namespace OCIF

//===========================================