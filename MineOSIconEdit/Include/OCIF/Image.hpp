#pragma once

#include <SFML/Graphics.hpp>

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Color.hpp"
#include "HexFont.hpp"

//===========================================

namespace OCIF
{

//===========================================

// Structure that holds unrasterized OCIF pixel data
struct Pixel
{
	uint32_t character  { ' ' };
	Color    background { 0   };
	Color    foreground { 0   };
	double   alpha      { 1.0 };
};

// Class that holds unrasterized OCIF image data
class Image
{
public:
	Image() = default;
	Image(size_t width, size_t height);

	size_t getWidth() const;
	size_t getHeight() const;

	void resize(size_t new_width, size_t new_height);
	void clear(const Pixel& pixel);

	void         set(size_t x, size_t y, const Pixel& pixel);
	void         set(size_t x, size_t y, Color background, Color foreground, double alpha, uint32_t character);
	      Pixel& get(size_t x, size_t y);
	const Pixel& get(size_t x, size_t y) const;

	void loadFromStream(std::istream& stream);
	void loadFromFile(const std::filesystem::path& path);

	void saveToStream(std::ostream& stream) const;
	void saveToFile(const std::filesystem::path& path) const;

	void rasterize(sf::Image& image, HexFont& font) const;
	void rasterizePixel(sf::Image& image, HexFont& font, size_t x, size_t y) const;

protected:

	size_t m_width  { 0 };
	size_t m_height { 0 };
	std::vector<Pixel> m_data;

};

//===========================================

} // namespace OCIF

//===========================================