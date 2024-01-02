#include <fstream>

#include "OCIF/HexFont.hpp"

//===========================================

#define HEX2QUAD(byte) ((byte) > '9'? 10 + (byte) - 'A': (byte) - '0')
#define HEX2BYTE(byte0, byte1) (HEX2QUAD(byte0) << 4) | HEX2QUAD(byte1)
#define QUAD2HEX(byte) static_cast<char>(((byte) > 9? 'A' - 10: '0') + (byte))

//===========================================

namespace OCIF
{
	
//=========================================== Glyph

HexFont::Glyph::Glyph(const uint8_t* data, size_t size)
{
	setData(data, size);
}

HexFont::Glyph::Glyph(const Glyph& copy)
{
	setData(copy.m_data, copy.m_size);
}

HexFont::Glyph::Glyph(Glyph&& move) noexcept
{
	std::swap(m_data, move.m_data);
	std::swap(m_size, move.m_size);
}

HexFont::Glyph::~Glyph()
{
	if (m_data)
		delete[](m_data);

	m_data = nullptr;
	m_size = 0;
}

void HexFont::Glyph::setData(const uint8_t* data, size_t size)
{
	uint8_t* tmp = new uint8_t[size];
	std::memcpy(tmp, data, size);
	std::swap(m_data, tmp);

	if (tmp)
		delete[](tmp);

	m_size = size;
}

size_t HexFont::Glyph::getDotBitIndex(unsigned x, unsigned y) const
{
	return y * getWidth() + x;
}

bool HexFont::Glyph::get(unsigned x, unsigned y) const
{
	size_t index = getDotBitIndex(x, y);

	// Each byte representing a row of 8 or 16 pixels
	return ((m_data[index / 8] >> (7 - (index % 8)))) & 1;
}

void HexFont::Glyph::set(unsigned x, unsigned y, bool dot)
{
	size_t index = getDotBitIndex(x, y);
	size_t bit = 7 - (index % 8);

	(m_data[index / 8] &= ~(1 << bit)) |= (dot << bit);
}

void HexFont::Glyph::rasterize(sf::Image& image, unsigned x, unsigned y, sf::Color background, sf::Color foreground) const
{
	const auto& image_size = image.getSize();
	for (size_t dot_x = 0; dot_x < getWidth() && x + dot_x < image_size.x; dot_x++)
		for (size_t dot_y = 0; dot_y < getHeight() && y + dot_y < image_size.y; dot_y++)
			image.setPixel(x + dot_x, y + dot_y, get(dot_x, dot_y)? foreground: background);
}

size_t HexFont::Glyph::getSize() const
{
	return m_size;
}

const uint8_t* HexFont::Glyph::getData() const
{
	return m_data;
}

//=========================================== Font

void HexFont::loadFromStream(std::istream& stream)
{
	constexpr size_t buffsize = 1024;
	char line[buffsize];

	while (stream.peek() != EOF && stream.getline(line, buffsize))
	{
		const char* delimiter = std::strchr(line, ':');
		if (!delimiter)
		{
			std::cerr << "Delimiter not found" << std::endl;
			continue;
		}

		size_t delimiter_offset = delimiter - line;

		size_t line_length = std::strlen(line);
		size_t data_size = (line_length - delimiter_offset - 1) / 2;

		uint8_t* data = new uint8_t[data_size];
		for (size_t i = 0, offset = delimiter_offset + 1; i < data_size; i++, offset += 2)
			data[i] = HEX2BYTE(line[offset], line[offset+1]);

		uint32_t code = std::strtoul(line, nullptr, 16);
		m_data.insert({code, Glyph(data, data_size)});
	}
}

void HexFont::loadFromFile(const std::filesystem::path& path)
{
	std::ifstream stream;
	stream.exceptions(stream.exceptions() | std::ios::failbit);
	stream.open(path);

	loadFromStream(stream);
}

void HexFont::saveToStream(std::ostream& stream) const
{
	static char buffer[128] = "";
	for (const auto& [codepoint, glyph]: m_data)
	{
		stream.write(buffer, sprintf_s(buffer, "%04X:", codepoint));

		const uint8_t* data = glyph.getData();
		for (size_t i = 0; i < glyph.getSize(); i++)
		{
			stream.put(QUAD2HEX(data[i] >> 4));
			stream.put(QUAD2HEX(data[i] & 0xF));
		}

		stream.put('\n');
	}
}

void HexFont::saveToFile(const std::filesystem::path& path) const
{
	std::ofstream stream;
	stream.exceptions(stream.exceptions() | std::ios::failbit);
	stream.open(path);

	saveToStream(stream);
}

const HexFont::Glyph& HexFont::getGlyph(uint32_t ch) const
{
	return m_data.at(ch);
}

HexFont::Glyph& HexFont::getGlyph(uint32_t ch)
{
	return m_data.at(ch);
}

const HexFont::Glyph& HexFont::operator[](uint32_t ch) const
{
	return getGlyph(ch);
}

HexFont::Glyph& HexFont::operator[](uint32_t ch)
{
	return getGlyph(ch);
}

//===========================================

}; // namespace OCIF

//===========================================