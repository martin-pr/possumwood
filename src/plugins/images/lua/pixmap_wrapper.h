#pragma once

#include <iostream>
#include <iomanip>

#include "datatypes/pixmap.h"

namespace possumwood { namespace images {

template<typename PIXMAP>
class PixmapWrapper {
	public:
		PixmapWrapper(std::size_t width, std::size_t height) : m_pixmap(new PIXMAP(width, height)) {
			m_constPixmap = m_pixmap;
		}

		PixmapWrapper(std::shared_ptr<const PIXMAP> ptr) : m_constPixmap(ptr) {
		}

		void setPixel(std::size_t x, std::size_t y, const luabind::object& value) {
			if(x >= width() || y >= height()) {
				std::stringstream ss;
				ss << "Coordinates " << x << ", " << y << " out of bounds of " << width() << ", " << height() << ".";
				throw std::runtime_error(ss.str().c_str());
			}

			// copy on first write - to be used in "injection" to allow to seamlessly
			//   "overwrite" the original image, copying only when necessary
			if(m_pixmap == nullptr) {
				m_pixmap = std::shared_ptr<PIXMAP>(new PIXMAP(*m_constPixmap));
				m_constPixmap = m_pixmap;
			}

			typename PIXMAP::pixel_t::value_t tmp;
			for(std::size_t i=0;i<tmp.size(); ++i)
				tmp[i] = luabind::object_cast<typename PIXMAP::pixel_t::value_t::value_type>(value[i+1]);

			(*m_pixmap)(x, y).setValue(tmp);
		}

		luabind::object pixel(std::size_t x, std::size_t y, lua_State* L) {
			luabind::object result = luabind::newtable(L);

			auto& pixel = (*m_constPixmap)(x, y).value();
			for(std::size_t i=0;i<pixel.size(); ++i)
				result[i+1] = pixel[i]; // 1-indexed arrays!
			return result;
		}

		std::size_t width() const {
			return m_constPixmap->width();
		}

		std::size_t height() const {
			return m_constPixmap->height();
		}

		const PIXMAP& pixmap() const {
			return *m_constPixmap;
		}

		operator std::shared_ptr<const PIXMAP>() const {
			return m_constPixmap;
		}

		bool operator == (const PixmapWrapper<PIXMAP>& p) const {
			return m_constPixmap == p.m_constPixmap;
		}

	private:
		std::shared_ptr<const PIXMAP> m_constPixmap;
		std::shared_ptr<PIXMAP> m_pixmap;
};

template<typename PIXMAP>
std::string to_string(const PixmapWrapper<PIXMAP>& p) {
	return "(pixmap " + std::to_string(p.width()) + "x" + std::to_string(p.height()) + ")";
}

} }
