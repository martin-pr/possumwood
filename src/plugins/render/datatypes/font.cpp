#include "font.h"

#include <nlohmann/json.hpp>

#include <fstream>

namespace possumwood {

bool Font::Glyph::operator==(const Glyph& g) const {
	return x == g.x && y == g.y && width == g.width && height == g.height && originX == g.originX &&
	       originY == g.originY && advance == g.advance;
}

bool Font::Glyph::operator!=(const Glyph& g) const {
	return x != g.x || y != g.y || width != g.width || height != g.height || originX != g.originX ||
	       originY != g.originY || advance != g.advance;
}

void Font::load(const boost::filesystem::path& filename) {
	// read the JSON with glyphs
	nlohmann::json json;
	{
		std::ifstream in(filename.string());
		in >> json;
	}

	for(nlohmann::json::const_iterator i = json["characters"].begin(); i != json["characters"].end(); ++i) {
		assert(i.key().length() == 1);

		Glyph g;
		g.x = (*i)["x"].get<int>();
		g.y = (*i)["y"].get<int>();
		g.width = (*i)["width"].get<int>();
		g.height = (*i)["height"].get<int>();
		g.originX = (*i)["originX"].get<int>();
		g.originY = (*i)["originY"].get<int>();
		g.advance = (*i)["advance"].get<int>();

		m_glyphs[i.key()[0]] = g;
	}

	m_width = json["width"].get<float>();
	m_height = json["height"].get<float>();
	m_size = json["size"].get<float>();
}

const Font::Glyph& Font::glyph(char c) const {
	static const Glyph s_nullGlyph{0, 0, 0, 0, 0, 0, 0};

	auto it = m_glyphs.find(c);
	if(it == m_glyphs.end())
		return s_nullGlyph;
	return it->second;
}

float Font::size() const {
	return m_size;
}
float Font::height() const {
	return m_height;
}
float Font::width() const {
	return m_width;
}

bool Font::operator==(const Font& f) const {
	return m_glyphs == f.m_glyphs && m_width == f.m_width && m_height == f.m_height && m_size == f.m_size;
}

bool Font::operator!=(const Font& f) const {
	return m_glyphs != f.m_glyphs || m_width != f.m_width || m_height != f.m_height || m_size != f.m_size;
}

std::ostream& operator<<(std::ostream& out, const Font& f) {
	out << "(font)";

	return out;
}

}  // namespace possumwood
