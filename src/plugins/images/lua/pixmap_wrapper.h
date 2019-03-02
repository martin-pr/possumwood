#pragma once

#include <iostream>
#include <iomanip>

#include "datatypes/pixmap.h"

namespace possumwood { namespace images {

template<typename PIXMAP>
class PixmapWrapper {
	public:
		PixmapWrapper(std::size_t width, std::size_t height) : m_pixmap(new PIXMAP(width, height)) {
		}

		void setPixel(std::size_t x, std::size_t y, const luabind::object& value) {
			if(x >= width() || y >= height()) {
				std::stringstream ss;
				ss << "Coordinates " << x << ", " << y << " out of bounds of " << width() << ", " << height() << ".";
				throw std::runtime_error(ss.str().c_str());
			}

			typename PIXMAP::pixel_t::value_t tmp;
			for(std::size_t i=0;i<tmp.size(); ++i)
				tmp[i] = luabind::object_cast<typename PIXMAP::pixel_t::value_t::value_type>(value[i+1]);

			(*m_pixmap)(x, y).setValue(tmp);
		}

		luabind::object pixel(std::size_t x, std::size_t y, lua_State* L) {
			luabind::object result = luabind::newtable(L);

			auto& pixel = (*m_pixmap)(x, y).value();
			for(std::size_t i=0;i<pixel.size(); ++i)
				result[i+1] = pixel[i]; // 1-indexed arrays!
			return result;
		}

		std::size_t width() const {
			return m_pixmap->width();
		}

		std::size_t height() const {
			return m_pixmap->height();
		}

		const PIXMAP& pixmap() const {
			return *m_pixmap;
		}

	private:
		std::shared_ptr<PIXMAP> m_pixmap;
};

} }
