#pragma once

#include <map>
#include <iostream>
#include <filesystem>

#include <SFML/Graphics.hpp>

//===========================================
		   
namespace OCIF
{

//===========================================

// Class for loading and storing opencomputers's hex font
class HexFont
{
public:
	// Class that holds and rasterizes hex font glyph
	class Glyph
	{
	public:
		static constexpr unsigned DefaultWidth  = 8; // 16 for wide characters
		static constexpr unsigned DefaultHeight = 16;

		Glyph() = default;
		Glyph(const uint8_t* data, size_t size);
		Glyph(const Glyph& copy);
		Glyph(Glyph&& move) noexcept;
		~Glyph();

		size_t getSize() const;
		const uint8_t* getData() const;
		void setData(const uint8_t* data, size_t size);

		// Returns true if dot is set in 8x16 glyph by x and y
		bool dot(unsigned x, unsigned y) const;

		// Draws the glyph at x and y with specified background/foreground colors
		void rasterize(sf::Image& image, unsigned x, unsigned y, sf::Color background, sf::Color foreground) const;

		constexpr bool isWide() const
		{
			return m_size > 16;
		}

		constexpr unsigned getWidth() const
		{
			return DefaultWidth * (1 + isWide());
		}

		constexpr unsigned getHeight() const
		{
			return DefaultHeight;
		}

	protected:
		uint8_t* m_data { nullptr };
		size_t m_size { 0 };

	};

	HexFont() = default;

	void loadFromStream(std::istream& stream);
	void loadFromFile(const std::filesystem::path& path);

	const Glyph& getGlyph(uint32_t code);
	const Glyph& operator[](uint32_t code);

protected:
	std::map<uint32_t, Glyph> m_data;

};

//===========================================

} // namespace OCIF

//===========================================