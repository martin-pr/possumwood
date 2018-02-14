#pragma once

#include <map>

#include <boost/filesystem/path.hpp>

#include <possumwood_sdk/traits.h>

namespace possumwood {

class Font {
  public:
	// a description of a single glyph
	struct Glyph {
		int x, y, width, height, originX, originY, advance;

		bool operator ==(const Glyph& g) const;
		bool operator !=(const Glyph& g) const;
	};

	// loads the font definition from a file
	void load(const boost::filesystem::path& filename);

	const Glyph& glyph(char c) const;

	bool operator==(const Font& f) const;
	bool operator!=(const Font& f) const;

	float size() const;
	float height() const;
	float width() const;

  private:
	std::map<char, Glyph> m_glyphs;

	float m_width, m_height, m_size;
};

template<>
struct Traits<Font> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 0.5, 0}};
	}
};

}
