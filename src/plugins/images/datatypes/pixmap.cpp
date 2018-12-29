#include "pixmap.h"

namespace possumwood {

Pixel::Pixel(const value_t& value) : m_value(value) {
}

const Pixel::value_t& Pixel::value() const {
	return m_value;
}

void Pixel::setValue(const value_t& rgb) {
	m_value = rgb;
}

Pixel& Pixel::operator = (const value_t& rgb) {
	m_value = rgb;
	return *this;
}

const Pixel::value_t& Pixel::operator*() const {
	return *this;
}

Pixel::operator const value_t&() const {
	return m_value;
}

Pixmap::Pixmap(std::size_t width, std::size_t height, const Pixel& defaultValue) : m_data(width*height, defaultValue), m_width(width), m_height(height) {
}

const Pixel& Pixmap::operator()(std::size_t x, std::size_t y) const {
	assert(x < m_width);
	assert(y < m_height);
	return m_data[x + y*m_width];
}

Pixel& Pixmap::operator()(std::size_t x, std::size_t y) {
	assert(x < m_width);
	assert(y < m_height);
	return m_data[x + y*m_width];
}

std::size_t Pixmap::width() const {
	return m_width;
}

std::size_t Pixmap::height() const {
	return m_height;
}

bool Pixmap::empty() const {
	return m_width == 0 && m_height == 0;
}

}
