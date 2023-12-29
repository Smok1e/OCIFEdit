#include <fstream>

#include "IO.hpp"
#include "Image.hpp"
#include "Color.hpp"
#include "Braille.hpp"

//===========================================

namespace OCIF
{

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
	m_data.resize(new_width * new_height);
	m_width = new_width;
	m_height = new_height;
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
		auto current_alpha = static_cast<double>(ReadBytes(stream) + is7) / 0xFF;

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
	stream.open(path);

	loadFromStream(stream);
}

//===========================================

void Image::rasterize(sf::Image& image, HexFont& font) const
{
	image.create(m_width * HexFont::Glyph::DefaultWidth, m_height * HexFont::Glyph::DefaultHeight);
	for (size_t x = 0; x < m_width; x++)
	{
		for (size_t y = 0; y < m_height; y++)
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
	}
}

//===========================================

} // namespace OCIF

//===========================================